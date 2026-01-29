module;

#include "CRWindows.h"
#include "ioringapi.h"

#include <core/Core.h>

module CR.Engine.Platform.FileRequest;

import CR.Engine.Core;

namespace cecore = CR::Engine::Core;
namespace cep    = CR::Engine::Platform;

namespace {
	constexpr uint32_t c_maxFiles   = 1024;
	constexpr uint32_t c_maxBuffers = 1024;

	HIORING m_ioring{};
	cecore::HandlePool<cep::Handles::File, c_maxFiles> m_fileHandles;
	cecore::HandlePool<cep::Handles::Buffer, c_maxBuffers> m_bufferHandles;

	std::mutex m_dataMutex;
	cecore::BitSet<c_maxFiles> m_filesToRegister;
	cecore::BitSet<c_maxFiles> m_filesToUnregister;
	std::array<std::string, c_maxFiles> m_filePaths;
	std::array<HANDLE, c_maxFiles> m_winFileHandles;

	std::jthread m_thread;
	std::mutex m_requestMutex;
	std::condition_variable m_notify;

	bool anyWork() {
		return !m_filesToRegister.empty() || !m_filesToUnregister.empty();
	}
	void threadMain(std::stop_token a_stoken) {
		while(!a_stoken.stop_requested()) {
			std::unique_lock<std::mutex> lock(m_requestMutex);
			if(!anyWork()) { m_notify.wait(lock); }
			if(a_stoken.stop_requested()) { return; }

			cecore::BitSet<c_maxFiles> filesToRegister;
			cecore::BitSet<c_maxFiles> filesToUnregister;
			{
				std::scoped_lock lock(m_dataMutex);
				filesToRegister = m_filesToRegister;
				m_filesToRegister.clear();
				filesToUnregister = m_filesToUnregister;
				m_filesToUnregister.clear();
			}

			for(auto index : filesToRegister) {
				CR_ASSERT(m_winFileHandles[index] == INVALID_HANDLE_VALUE, "Logic error, file already open");
				m_winFileHandles[index] = CreateFile(
				    m_filePaths[index].c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
				    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, nullptr);
			}
			for(auto index : filesToUnregister) {
				CR_ASSERT(m_winFileHandles[index] != INVALID_HANDLE_VALUE, "Logic error, file not open");
				CloseHandle(m_winFileHandles[index]);
			}

			uint32_t winFileHandleCount{};
			std::array<HANDLE, c_maxFiles> winFileHandles;
			for(uint32_t index = 0; index < c_maxFiles; ++index) {
				if(m_winFileHandles[index] != INVALID_HANDLE_VALUE) {
					winFileHandles[winFileHandleCount] = m_winFileHandles[index];
					++winFileHandleCount;
				}
			}

			HRESULT winResult =
			    BuildIoRingRegisterFileHandles(m_ioring, winFileHandleCount, winFileHandles.data(), 0);
			CR_ASSERT(winResult != IORING_E_SUBMISSION_QUEUE_FULL, "Don't open an insane number of files!");

			if(a_stoken.stop_requested()) { return; }

			SubmitIoRing(m_ioring, 1, INFINITE, nullptr);
			{
				std::scoped_lock lock(m_dataMutex);
				m_fileHandles.release(filesToUnregister);
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
#if !CR_DEBUG
	createFlags.Advisory = IORING_CREATE_SKIP_BUILDER_PARAM_CHECKS;
#endif

	// we don't need a max size queue, but until it causes trouble... last time i looked it was 64K for
	// submission.
	result = CreateIoRing(iocaps.MaxVersion, createFlags, iocaps.MaxSubmissionQueueSize,
	                      iocaps.MaxCompletionQueueSize, &m_ioring);
	CR_ASSERT(result == S_OK, "failed to create io ring");

	for(auto& handle : m_winFileHandles) { handle = INVALID_HANDLE_VALUE; }

	m_thread = std::jthread([](std::stop_token a_token) { threadMain(std::move(a_token)); });
}

void cep::FileRequest::Internal::shutdown() {
	m_thread.request_stop();
	m_notify.notify_one();
	m_thread.join();
	CloseIoRing(m_ioring);
}

void cep::FileRequest::Internal::update() {}

cep::Handles::File cep::FileRequest::registerFile(const std::filesystem::path& a_filePath) {
	std::scoped_lock lock(m_dataMutex);

	CR_ASSERT(!m_fileHandles.exhausted(), "Ran out of file handles");
	auto handle         = m_fileHandles.aquire();
	m_filePaths[handle] = a_filePath.string();
	m_filesToRegister.insert(handle.id());

	return handle;
}

void cep::FileRequest::unregisterFile(Handles::File a_file) {
	std::scoped_lock lock(m_dataMutex);
	CR_ASSERT(m_fileHandles.isValid(a_file), "tried to unregister an invalid file");
	m_filesToUnregister.insert(a_file.id());
}

cep::Handles::Buffer cep::FileRequest::registerBuffer(std::span<std::byte> a_buffer) {
	std::scoped_lock lock(m_dataMutex);

	CR_ASSERT(!m_bufferHandles.exhausted(), "Ran out of buffer handles");
	auto handle = m_bufferHandles.aquire();

	return handle;
}

void cep::FileRequest::unregisterBuffer(Handles::Buffer a_buffer) {
	std::scoped_lock lock(m_dataMutex);

	m_bufferHandles.release(a_buffer);
}
