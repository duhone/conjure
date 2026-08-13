module;

#include "CRWindows.h"
#include "ioringapi.h"

#include <core/Core.h>

module CR.Engine.Platform.FileRequest;

import CR.Engine.Core;

namespace cecore = CR::Engine::Core;
namespace cep    = CR::Engine::Platform;

namespace {
	constexpr uint32_t c_maxFiles   = 512;
	constexpr uint32_t c_maxBuffers = 1024;
	constexpr uint32_t c_maxReads   = 4096;

	HIORING m_ioring{};
	cecore::HandlePool<cep::Handles::File, c_maxFiles> m_fileHandles;
	cecore::HandlePool<cep::Handles::Buffer, c_maxBuffers> m_bufferHandles;
	cecore::HandlePool<cep::Handles::Read, c_maxReads> m_readHandles;

	std::mutex m_dataMutex;
	cecore::BitSet<c_maxFiles> m_filesToRegister;
	cecore::BitSet<c_maxFiles> m_filesToUnregister;
	std::array<std::string, c_maxFiles> m_filePaths;
	std::array<HANDLE, c_maxFiles> m_winFileHandles;

	cecore::BitSet<c_maxBuffers> m_buffersToRegister;
	cecore::BitSet<c_maxBuffers> m_buffersToUnregister;
	std::array<IORING_BUFFER_INFO, c_maxBuffers> m_buffers;

	cecore::BitSet<c_maxReads> m_readRequests;
	std::array<cep::FileRequest::ReadArgs, c_maxReads> m_readArgs;
	std::array<std::atomic_bool, c_maxReads> m_readCompletions;

	std::jthread m_submitThread;
	std::jthread m_completionThread;
	std::mutex m_requestMutex;
	std::condition_variable m_notify;

	HANDLE m_ioEvent;

	bool anyWork() {
		return !m_filesToRegister.empty() || !m_filesToUnregister.empty() || !m_buffersToRegister.empty() ||
		       !m_buffersToUnregister.empty() || !m_readRequests.empty();
	}

	void processFiles() {
		cecore::BitSet<c_maxFiles> filesToRegister;
		cecore::BitSet<c_maxFiles> filesToUnregister;
		{
			std::scoped_lock lock(m_dataMutex);
			if(m_filesToRegister.empty() && m_filesToUnregister.empty()) { return; }
			filesToRegister = m_filesToRegister;
			m_filesToRegister.clear();
			filesToUnregister = m_filesToUnregister;
			m_filesToUnregister.clear();
		}

		for(auto index : filesToRegister) {
			CR_ASSERT(m_winFileHandles[index] == INVALID_HANDLE_VALUE, "Logic error, file already open");
			m_winFileHandles[index] =
			    CreateFile(m_filePaths[index].c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
			               FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, nullptr);
		}
		for(auto index : filesToUnregister) {
			CR_ASSERT(m_winFileHandles[index] != INVALID_HANDLE_VALUE, "Logic error, file not open");
			CloseHandle(m_winFileHandles[index]);
			m_winFileHandles[index] = INVALID_HANDLE_VALUE;
		}

		HRESULT winResult =
		    BuildIoRingRegisterFileHandles(m_ioring, m_winFileHandles.size(), m_winFileHandles.data(), 0);
		CR_ASSERT(winResult != IORING_E_SUBMISSION_QUEUE_FULL,
		          "queue should be empty, should be impossible to fail");

		winResult = SubmitIoRing(m_ioring, 0, 0, nullptr);
		CR_ASSERT(winResult == S_OK, "failed to submit io ring");

		{
			std::scoped_lock lock(m_dataMutex);
			m_fileHandles.release(filesToUnregister);
		}
	}

	void processBuffers() {
		cecore::BitSet<c_maxBuffers> buffersToRegister;
		cecore::BitSet<c_maxBuffers> buffersToUnregister;
		{
			std::scoped_lock lock(m_dataMutex);
			if(m_buffersToRegister.empty() && m_buffersToUnregister.empty()) { return; }

			buffersToRegister = m_buffersToRegister;
			m_buffersToRegister.clear();
			buffersToUnregister = m_buffersToUnregister;
			m_buffersToUnregister.clear();
		}

		for(auto index : buffersToUnregister) {
			m_buffers[index].Address = nullptr;
			m_buffers[index].Length  = 0;
		}

		HRESULT winResult = BuildIoRingRegisterBuffers(m_ioring, m_buffers.size(), m_buffers.data(), 0);
		CR_ASSERT(winResult != IORING_E_SUBMISSION_QUEUE_FULL,
		          "queue should be empty, should be impossible to fail");

		winResult = SubmitIoRing(m_ioring, 0, 0, nullptr);
		CR_ASSERT(winResult == S_OK, "failed to submit io ring");

		{
			std::scoped_lock lock(m_dataMutex);
			m_bufferHandles.release(buffersToUnregister);
		}
	}

	void processReads() {
		cecore::BitSet<c_maxReads> readRequests;
		{
			std::scoped_lock lock(m_dataMutex);
			if(m_readRequests.empty()) { return; }

			readRequests = m_readRequests;
			m_readRequests.clear();
		}

		for(auto request : readRequests) {
			uint32_t fileIndex        = m_readArgs[request].fileHandle;
			uint32_t bufferIndex      = m_readArgs[request].bufferHandle;
			IORING_HANDLE_REF fileRef = IoRingHandleRefFromIndex(fileIndex);
			IORING_BUFFER_REF bufferRef =
			    IoRingBufferRefFromIndexAndOffset(bufferIndex, m_readArgs[request].bufferOffset);

			HRESULT winResult =
			    BuildIoRingReadFile(m_ioring, fileRef, bufferRef, m_readArgs[request].readSize,
			                        m_readArgs[request].fileOffset, 0, IOSQE_FLAGS_NONE);
			CR_ASSERT(winResult != IORING_E_SUBMISSION_QUEUE_FULL,
			          "ran out of queue space, shouldn't happen for smallish apps");
		}

		uint32_t submitted{};
		HRESULT winResult = SubmitIoRing(m_ioring, 0, 0, &submitted);
		CR_ASSERT(winResult == S_OK, "failed to submit io ring");
		CR_ASSERT(submitted == readRequests.size(), "not all were submitted");

		for(auto request : readRequests) {
			m_readCompletions[request].store(true, std::memory_order_release);
		}
	}

	void submitThreadMain(std::stop_token a_stoken) {
		while(!a_stoken.stop_requested()) {
			std::unique_lock<std::mutex> lock(m_requestMutex);
			if(!anyWork()) { m_notify.wait(lock); }
			if(a_stoken.stop_requested()) { break; }

			processFiles();

			if(a_stoken.stop_requested()) { break; }

			processBuffers();

			if(a_stoken.stop_requested()) { break; }

			processReads();
		}

		// need to close any open files, mostly only matters for unit tests.
		for(uint32_t i = 0; i < c_maxFiles; ++i) {
			if(m_winFileHandles[i] != INVALID_HANDLE_VALUE) { m_filesToUnregister.insert(i); }
		}
		processFiles();
	}

	void completionThreadMain(std::stop_token a_stoken) {
		while(!a_stoken.stop_requested()) {
			WaitForSingleObject(m_ioEvent, INFINITE);
			if(a_stoken.stop_requested()) { break; }

			IORING_CQE result;
			while(PopIoRingCompletion(m_ioring, &result) == S_OK) {
				// we don't use the user data or result code for anything right now, but we might want to in
				// the future. if we do, we'll need to store the user data with the request and look it up
				// here.

				if(a_stoken.stop_requested()) { break; }
			}
		}
	}
}    // namespace

void cep::FileRequest::Internal::initialize() {
	IORING_CAPABILITIES iocaps{};
	HRESULT result = QueryIoRingCapabilities(&iocaps);
	CR_ASSERT(result == S_OK, "failed to get ioring caps");
	// I haven't looked into whats different between version 1-4. my PC returns 4, so thats the min version
	// I've tested.
	CR_ASSERT(iocaps.MaxVersion >= IORING_VERSION_4, "ioring version 4 not supported");

	IORING_CREATE_FLAGS createFlags{};
#if CR_PROFILE || CR_FINAL
	createFlags.Advisory = IORING_CREATE_SKIP_BUILDER_PARAM_CHECKS;
#endif

	// we don't need a max size queue, but until it causes trouble... last time i looked it was 64K for
	// submission.
	result = CreateIoRing(iocaps.MaxVersion, createFlags, iocaps.MaxSubmissionQueueSize,
	                      iocaps.MaxCompletionQueueSize, &m_ioring);
	CR_ASSERT(result == S_OK, "failed to create io ring");

	for(auto& handle : m_winFileHandles) { handle = INVALID_HANDLE_VALUE; }

	m_ioEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	result = SetIoRingCompletionEvent(m_ioring, m_ioEvent);
	CR_ASSERT(result == S_OK, "failed to set completion event");

	m_submitThread = std::jthread([](std::stop_token a_token) { submitThreadMain(std::move(a_token)); });

	m_completionThread =
	    std::jthread([](std::stop_token a_token) { completionThreadMain(std::move(a_token)); });
}

void cep::FileRequest::Internal::shutdown() {
	m_submitThread.request_stop();
	m_completionThread.request_stop();
	m_notify.notify_one();
	SetEvent(m_ioEvent);
	m_submitThread.join();
	m_completionThread.join();
	CloseIoRing(m_ioring);
	CloseHandle(m_ioEvent);
}

void cep::FileRequest::Internal::update() {}

cep::Handles::File cep::FileRequest::registerFile(const std::filesystem::path& a_filePath) {
	std::scoped_lock lock(m_dataMutex);

	CR_ASSERT(!m_fileHandles.exhausted(), "Ran out of file handles");
	auto handle         = m_fileHandles.acquire();
	m_filePaths[handle] = a_filePath.string();
	m_filesToRegister.insert(handle.id());

	m_notify.notify_one();

	return handle;
}

void cep::FileRequest::unregisterFile(Handles::File a_file) {
	std::scoped_lock lock(m_dataMutex);
	CR_ASSERT(m_fileHandles.isValid(a_file), "tried to unregister an invalid file");
	m_filesToUnregister.insert(a_file.id());

	m_notify.notify_one();
}

cep::Handles::Buffer cep::FileRequest::registerBuffer(std::span<std::byte> a_buffer) {
	std::scoped_lock lock(m_dataMutex);

	CR_ASSERT(!m_bufferHandles.exhausted(), "Ran out of buffer handles");
	auto handle = m_bufferHandles.acquire();

	m_buffersToRegister.insert(handle);
	m_buffers[handle].Address = a_buffer.data();
	m_buffers[handle].Length  = a_buffer.size();

	m_notify.notify_one();

	return handle;
}

void cep::FileRequest::unregisterBuffer(Handles::Buffer a_buffer) {
	std::scoped_lock lock(m_dataMutex);
	m_buffersToUnregister.insert(a_buffer);

	m_notify.notify_one();
}

cep::Handles::Read cep::FileRequest::read(const ReadArgs& a_args) {
	CR_ASSERT(!m_readHandles.exhausted(), "Ran out of file read handles");
	auto handle = m_readHandles.acquire();

	m_readArgs[handle] = a_args;

	m_readCompletions[handle].store(false, std::memory_order_release);

	std::scoped_lock lock(m_dataMutex);
	m_readRequests.insert(handle);

	m_notify.notify_one();

	return handle;
}

bool cep::FileRequest::hasReadCompleted(Handles::Read a_handle) {
	CR_ASSERT(
	    m_readHandles.isValid(a_handle),
	    "invalid read handle, they are invalid after the 1st call to hasReadCompleted that returs true.");

	bool completed = m_readCompletions[a_handle].load(std::memory_order_acquire);

	if(completed) { m_readHandles.release(a_handle); }

	return completed;
}