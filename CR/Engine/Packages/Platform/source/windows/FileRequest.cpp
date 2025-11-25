module;

#include "CRWindows.h"
#include "ioringapi.h"

#include <core/Core.h>

module CR.Engine.Platform.FileRequest;

import CR.Engine.Core;

namespace cecore = CR::Engine::Core;
namespace cep    = CR::Engine::Platform;

namespace {
	constexpr uint32_t c_queueSize = 1024;

	HIORING m_ioring{};
	cecore::BitSet<c_queueSize> m_used;

}    // namespace

void cep::FileRequest::Internal::Initialize() {
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
	const uint32_t submissionQueueSize = std::min(c_queueSize, iocaps.MaxSubmissionQueueSize);
	const uint32_t completionQueueSize = 2 * submissionQueueSize;

	result =
	    CreateIoRing(iocaps.MaxVersion, createFlags, submissionQueueSize, completionQueueSize, &m_ioring);
	CR_ASSERT(result == S_OK, "failed to create io ring");
}

void cep::FileRequest::Internal::Shutdown() {
	CloseIoRing(m_ioring);
}

void cep::FileRequest::Internal::Update() {}

cep::Handles::File cep::FileRequest::RegisterFile(const std::filesystem::path& a_filePath) {
	return {};
}

void cep::FileRequest::UnregisterFile(Handles::File a_file) {}
