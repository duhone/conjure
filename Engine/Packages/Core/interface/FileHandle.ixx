export module CR.Engine.Core.FileHandle;

import std;

export namespace CR::Engine::Core {
	// Currently only for binary files
	class FileHandle final {
	  public:
		FileHandle(const std::filesystem::path& a_path, bool a_forWriting);
		~FileHandle();

		FileHandle(const FileHandle&)            = delete;
		FileHandle& operator=(const FileHandle&) = delete;
		FileHandle(FileHandle&& a_other) noexcept;
		FileHandle& operator=(FileHandle&& a_other) noexcept;

		std::FILE* asFile() const noexcept { return m_file; }

	  private:
		std::FILE* m_file{nullptr};
	};
}    // namespace CR::Engine::Core

namespace crec = CR::Engine::Core;

inline crec::FileHandle::FileHandle(const std::filesystem::path& a_path, bool a_forWriting) {
	m_file = std::fopen(a_path.string().c_str(), a_forWriting ? "wb" : "rb");
}

inline crec::FileHandle::~FileHandle() {
	if(m_file) { std::fclose(m_file); }
}

inline crec::FileHandle::FileHandle(FileHandle&& a_other) noexcept {
	*this = std::move(a_other);
}

inline crec::FileHandle& crec::FileHandle::operator=(FileHandle&& a_other) noexcept {
	if(m_file) { std::fclose(m_file); }
	m_file         = a_other.m_file;
	a_other.m_file = nullptr;

	return *this;
}
