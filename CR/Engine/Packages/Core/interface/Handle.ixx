module;

#include <core/Log.h>

export module CR.Engine.Core.Handle;

import CR.Engine.Core.Log;
import CR.Engine.Core.BitSet;

import std;
import std.compat;

export namespace CR::Engine::Core {
	// TAG is just there to force a unique type, can just make a new type inline if needed. i.e.
	// Handle<class HandleTag>
	template<typename TAG>
	class Handle {
	  public:
		constexpr Handle() noexcept = default;
		// always take uint64_t, convenience, avoid some casting,
		constexpr explicit Handle(uint64_t a_id, uint64_t a_generation) noexcept {
			CR_ASSERT(a_id < std::numeric_limits<uint16_t>::max(), "handle id's must be less than 64K");
			CR_ASSERT(m_generation < std::numeric_limits<uint16_t>::max(),
			          "handle generation's must be less than 64K");
			m_id         = (uint16_t)a_id;
			m_generation = a_generation;
		}
		// always take uint64_t, convenience, avoid some casting, defaults to generation 0.
		constexpr explicit Handle(uint64_t a_id) noexcept {
			CR_ASSERT(a_id < std::numeric_limits<uint16_t>::max(), "handle id's must be less than 64K");
			m_id         = (uint16_t)a_id;
			m_generation = 0;
		}

		constexpr ~Handle() noexcept              = default;
		Handle(const Handle&) noexcept            = default;
		Handle(Handle&&) noexcept                 = default;
		Handle& operator=(const Handle&) noexcept = default;
		Handle& operator=(Handle&&) noexcept      = default;

		constexpr bool operator==(const Handle&) const noexcept = default;

		// allow implicit cast to int, for indexing into containers tersely.
		constexpr operator uint16_t() const noexcept { return m_id; }
		constexpr bool isValid() const noexcept { return m_id != c_unused; }

		constexpr uint16_t getGeneration() const noexcept { return m_generation; }
		constexpr void incGeneration() noexcept { ++m_generation; }

	  protected:
		inline static constexpr uint16_t c_unused{std::numeric_limits<uint16_t>::max()};

		uint16_t m_id{c_unused};
		uint16_t m_generation{};
	};

	template<typename HandleType, uint16_t c_poolSize>
	class HandlePool {
	  public:
		HandlePool() {
			for(uint32_t i = 0; i < c_poolSize; ++i) { m_handles[i] = HandleType{i}; }
		}
		~HandlePool() = default;

		constexpr HandleType aquire() {
			CR_ASSERT(m_used.size() != c_poolSize, "ran out of handles");
			auto avail = m_used.FindNotInSet();
			m_used.insert(avail);
			m_handles[avail].incGeneration();
			return m_handles[avail];
		}

		// will mark passed in handle as invalid. You have to ask the pool about any other duplicate handles
		// though.
		constexpr void release(HandleType& a_handle) {
			CR_ASSERT(a_handle.isValid(), "tried to release handle that isn't valid");
			CR_ASSERT(m_used.contains(a_handle), "tried to release handle that is no longer valid");
			CR_ASSERT(a_handle.getGeneration() == m_handles[a_handle].getGeneration(),
			          "tried to release handle that is no longer valid, incorrect generation");
			m_used.erase(a_handle);

			a_handle = HandleType{};
		}

		constexpr HandleType tryGetHandle(uint16_t index) {
			if(!m_used.contains(index)) { return {}; }
			return m_handles[index];
		}

		// checks if handle itself is valid, and it is the currently valid(correct generation.
		//  handle itself being valid only means its was valid now or in the past. check the pool here to
		//  verify if its still valid.
		constexpr bool isValid(HandleType a_handle) {
			if(!a_handle.isValid()) { return false; }
			if(!m_used.contains(a_handle)) { return false; }
			return m_handles[a_handle] == a_handle;
		}

		constexpr bool exhausted() const { return m_used.size() == c_poolSize; }

		template<typename Callable>
		void iterate(Callable&& callable) {
			for(const auto index : m_used) {
				if(!callable(m_handles[index])) { break; }
			}
		}

	  private:
		BitSet<c_poolSize> m_used{};
		std::array<HandleType, c_poolSize> m_handles{};
	};
}    // namespace CR::Engine::Core

template<typename TAG>
struct std::formatter<CR::Engine::Core::Handle<TAG>> : std::formatter<uint16_t> {
	using std::formatter<uint16_t>::parse;

	auto format(const CR::Engine::Core::Handle<TAG>& a_handle, auto& a_ctx) const {
		return std::formatter<uint16_t>::format((uint16_t)a_handle, a_ctx);
	}
};