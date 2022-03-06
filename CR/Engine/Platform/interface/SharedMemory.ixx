export module CR.Engine.Platform.SharedMemory;

import<cstdint>;
import<memory>;

namespace CR::Engine::Platform {
	export class SharedMemory final {
	  public:
		struct CreateNew {};
		struct OpenExisting {};

		SharedMemory() = default;
		// This will create a new shared memory object
		SharedMemory(const char* a_name, size_t a_size, CreateNew);
		// This will open an existing shared memory space, it must already exist
		SharedMemory(const char* a_name, size_t a_size, OpenExisting);
		~SharedMemory();

		SharedMemory(const SharedMemory&) = delete;
		SharedMemory& operator=(const SharedMemory&) = delete;
		SharedMemory(SharedMemory&& a_other) noexcept;
		SharedMemory& operator=(SharedMemory&& a_other) noexcept;

		// follow stl naming convention for compatibility with non member data/size
		[[nodiscard]] std::size_t size() const;
		[[nodiscard]] const uint8_t* data() const;
		[[nodiscard]] uint8_t* data();

	  private:
		std::unique_ptr<struct SharedMemoryData> m_data;
	};
}    // namespace CR::Engine::Platform
