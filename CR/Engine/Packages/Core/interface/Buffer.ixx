module;

// needed for windows only
#include <malloc.h>

export module CR.Engine.Core.Buffer;

import std;
import std.compat;

export namespace CR::Engine::Core {
	// A raw buffer. Mostly to get around stl insistence to default initialize data.
	// Actual buffer will always be a multiple of 64 bytes regardless of its reported size.
	class Buffer final {
	  public:
		Buffer() = default;
		~Buffer();

		Buffer(const Buffer&)            = delete;
		Buffer& operator=(const Buffer&) = delete;
		Buffer(Buffer&& a_other) noexcept { *this = std::move(a_other); }
		Buffer& operator=(Buffer&& a_other) noexcept {
			m_data = a_other.m_data;
			m_size = a_other.m_size;

			a_other.m_data = nullptr;
			a_other.m_size = 0;

			return *this;
		}

		const std::byte* data() const noexcept { return m_data; }
		std::byte* data() noexcept { return m_data; }
		uint32_t size() const noexcept { return m_size; }

		template<typename T>
		    requires std::is_standard_layout_v<T>
		const T* data() const noexcept {
			return (const T*)m_data;
		}

		template<typename T>
		    requires std::is_standard_layout_v<T>
		T* data() noexcept {
			return (T*)m_data;
		}

		template<typename T>
		    requires std::is_standard_layout_v<T>
		uint32_t size() const noexcept {
			return m_size / sizeof(T);
		}

		// If shrinking, actual buffer won't be shrunk, and resize is "free". use shrinkToFit if you want
		// to actually shrink in that case.
		void resize(uint32_t a_newSize);
		template<typename T>
		    requires std::is_standard_layout_v<T>
		void resize(uint32_t a_newSize) {
			return resize(m_size * sizeof(T));
		}

		void shrinkToFit();

	  private:
		std::byte* m_data{nullptr};
		uint32_t m_size{};
		uint32_t m_capacity{};
	};
}    // namespace CR::Engine::Core

module :private;

namespace crcore = CR::Engine::Core;

crcore::Buffer::~Buffer() {
	// std::free(m_data);
	// visual studio doesn't support std::aligned_alloc(and std::free with an aligned pointer)
	_aligned_free(m_data);
}

void crcore::Buffer::resize(uint32_t a_newSize) {
	if(a_newSize > m_capacity) {
		m_capacity = (a_newSize + 63) & 0xffffffc0;

		// windows doesn't support std::aligned_alloc(and std::free with an aligned pointer)
		// m_data == std::aligned_alloc(64, roundedSize);
		std::byte* newData = (std::byte*)_aligned_malloc(m_capacity, 64);
		memcpy(newData, m_data, std::min(m_size, a_newSize));
		_aligned_free(m_data);

		m_data = newData;
		m_size = a_newSize;
	} else {
		// buffer itself won't change
		m_size = a_newSize;
	}
}

void crcore::Buffer::shrinkToFit() {
	uint32_t newCapacity = (m_size + 63) & 0xffffffc0;
	if(newCapacity < m_capacity) {
		// windows doesn't support std::aligned_alloc(and std::free with an aligned pointer)
		// m_data == std::aligned_alloc(64, roundedSize);
		std::byte* newData = (std::byte*)_aligned_malloc(newCapacity, 64);
		memcpy(newData, m_data, m_size);
		_aligned_free(m_data);

		m_data     = newData;
		m_capacity = newCapacity;
	}
}