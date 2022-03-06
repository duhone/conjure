module;

#include "core/Log.h"
#include <platform/windows/CRWindows.h>

module CR.Engine.Platform.MemoryMappedFile;

namespace CR::Engine::Platform {
	struct MemoryMappedFileData {
		HANDLE m_fileHandle{INVALID_HANDLE_VALUE};
		HANDLE m_fileMapping{INVALID_HANDLE_VALUE};
		std::byte* m_data{nullptr};
		std::size_t m_fileSize{0};
	};
}    // namespace CR::Engine::Platform

namespace cep = CR::Engine::Platform;

cep::MemoryMappedFile::MemoryMappedFile() {}

cep::MemoryMappedFile::MemoryMappedFile(const std::filesystem::path& a_filePath) {
	auto handle =
	    CreateFileW(a_filePath.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	CR_ASSERT(handle != INVALID_HANDLE_VALUE, "Could not open file {}", a_filePath.string());

	LARGE_INTEGER fileSize;
	if(!GetFileSizeEx(handle, &fileSize)) {
		CR_ASSERT(false, "Could not get file size for {}", a_filePath.string());
	}
	CR_ASSERT(fileSize.QuadPart != 0, "File {} was 0 sized", a_filePath.string());

	m_fileData               = std::make_unique<MemoryMappedFileData>();
	m_fileData->m_fileHandle = handle;
	m_fileData->m_fileSize   = fileSize.QuadPart;

	m_fileData->m_fileMapping =
	    CreateFileMapping(m_fileData->m_fileHandle, nullptr, PAGE_READONLY, 0, 0, nullptr);
	CR_ASSERT(m_fileData->m_fileMapping != INVALID_HANDLE_VALUE, "Could not map file {}",
	          a_filePath.string());
	m_fileData->m_data = (std::byte*)MapViewOfFile(m_fileData->m_fileMapping, FILE_MAP_READ, 0, 0, 0);
}

cep::MemoryMappedFile::~MemoryMappedFile() {
	if(m_fileData) {
		UnmapViewOfFile(m_fileData->m_data);
		CloseHandle(m_fileData->m_fileMapping);
		CloseHandle(m_fileData->m_fileHandle);
	}
}

cep::MemoryMappedFile::MemoryMappedFile(MemoryMappedFile&& a_other) noexcept {
	*this = std::move(a_other);
}

cep::MemoryMappedFile& cep::MemoryMappedFile::operator=(MemoryMappedFile&& a_other) noexcept {
	if(this == &a_other) { return *this; }
	m_fileData = std::move(a_other.m_fileData);
	return *this;
}

std::size_t cep::MemoryMappedFile::size() const noexcept {
	if(!m_fileData) { return 0; }
	return m_fileData->m_fileSize;
}

std::byte* cep::MemoryMappedFile::data() noexcept {
	if(!m_fileData) { return nullptr; }
	return m_fileData->m_data;
}

const std::byte* cep::MemoryMappedFile::data() const noexcept {
	if(!m_fileData) { return nullptr; }
	return m_fileData->m_data;
}
