export module CR.Engine.Core.FileHandle;

import<cstdio>;
import<filesystem>;

export namespace CR::Engine::Core {
	// Currently only for writing files, for reading use memory mapped files.
	class FileHandle final {
	  public:
		FileHandle(const std::filesystem::path& a_path);
		~FileHandle();

		FileHandle(const FileHandle&) = delete;
		FileHandle& operator=(const FileHandle&) = delete;
		FileHandle(FileHandle&& a_other) noexcept;
		FileHandle& operator=(FileHandle&& a_other) noexcept;

		operator FILE*() const noexcept { return m_file; }

	  private:
		FILE* m_file{nullptr};
	};
}    // namespace CR::Core

namespace crec = CR::Engine::Core;

inline crec::FileHandle::FileHandle(const std::filesystem::path& a_path) {
	fopen_s(&m_file, a_path.string().c_str(), "wb");
}

inline crec::FileHandle::~FileHandle() {
	if(m_file) { fclose(m_file); }
}

inline crec::FileHandle::FileHandle(FileHandle&& a_other) noexcept {
	*this = std::move(a_other);
}

inline crec::FileHandle& crec::FileHandle::operator=(FileHandle&& a_other) noexcept {
	if(m_file) { fclose(m_file); }
	m_file         = a_other.m_file;
	a_other.m_file = nullptr;

	return *this;
}
