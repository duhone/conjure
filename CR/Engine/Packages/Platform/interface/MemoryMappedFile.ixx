export module CR.Engine.Platform.MemoryMappedFile;

import <cstddef>;
import <cstdint>;
import <filesystem>;
import <memory>;
import <span>;

namespace CR::Engine::Platform {
	export class MemoryMappedFile final {
	  public:
		MemoryMappedFile();
		MemoryMappedFile(const std::filesystem::path& a_filePath);
		~MemoryMappedFile();
		MemoryMappedFile(const MemoryMappedFile&) = delete;
		MemoryMappedFile(MemoryMappedFile&& a_other) noexcept;
		MemoryMappedFile& operator=(const MemoryMappedFile&) = delete;
		MemoryMappedFile& operator=(MemoryMappedFile&& a_other) noexcept;

		// follow stl naming convention for compatibility with non member data/size
		[[nodiscard]] bool empty() const noexcept;
		[[nodiscard]] std::size_t size() const noexcept;
		[[nodiscard]] std::byte* data() noexcept;
		[[nodiscard]] const std::byte* data() const noexcept;

		[[nodiscard]] std::byte* begin() noexcept;
		[[nodiscard]] const std::byte* begin() const noexcept;
		[[nodiscard]] std::byte* end() noexcept;
		[[nodiscard]] const std::byte* end() const noexcept;

		[[nodiscard]] const std::byte* cbegin() const noexcept;
		[[nodiscard]] const std::byte* cend() const noexcept;

		// element access
		[[nodiscard]] std::byte& operator[](size_t n);
		[[nodiscard]] const std::byte& operator[](size_t n) const;

		[[nodiscard]] std::span<std::byte> GetData() { return {data(), size()}; }
		[[nodiscard]] const std::span<const std::byte> GetData() const { return {data(), size()}; }

		[[nodiscard]] bool isValid() const { return m_fileData.get() != nullptr; }

	  private:
		std::unique_ptr<struct MemoryMappedFileData> m_fileData;
	};

	inline bool MemoryMappedFile::empty() const noexcept {
		return size() == 0;
	}

	inline std::byte* MemoryMappedFile::begin() noexcept {
		return data();
	}
	inline const std::byte* MemoryMappedFile::begin() const noexcept {
		return data();
	}
	inline std::byte* MemoryMappedFile::end() noexcept {
		return data() + size();
	}
	inline const std::byte* MemoryMappedFile::end() const noexcept {
		return data() + size();
	}

	inline const std::byte* MemoryMappedFile::cbegin() const noexcept {
		return data();
	}
	inline const std::byte* MemoryMappedFile::cend() const noexcept {
		return data() + size();
	}

	inline std::byte& MemoryMappedFile::operator[](size_t a_element) {
		return data()[a_element];
	}
	inline const std::byte& MemoryMappedFile::operator[](size_t a_element) const {
		return data()[a_element];
	}
}    // namespace CR::Engine::Platform
