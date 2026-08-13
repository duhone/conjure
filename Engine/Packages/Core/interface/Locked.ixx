module;

#include <core/Defer.h>

export module CR.Engine.Core.Locked;

import CR.Engine.Core.Defer;

import std;

export namespace CR::Engine::Core {
	template<std::semiregular... T>
	class MultiLock;

	template<std::semiregular... T>
	class Locked final {
		template<std::semiregular... T>
		friend class MultiLock;

	  public:
		Locked()  = default;
		~Locked() = default;
		Locked(const Locked& other) {
			std::shared_lock lockOther(other.m_mutex);
			m_instances(other.m_instances);
		}
		Locked& operator=(const Locked& other) {
			std::unique_lock lock(m_mutex);
			std::shared_lock lockOther(other.m_mutex);
			m_instances = other.m_instances;
		}
		Locked(Locked&& other) noexcept {
			std::unique_lock lockOther(other.m_mutex);
			m_instances = std::move(other.m_instances);
		}
		Locked& operator=(Locked&& other) noexcept {
			std::unique_lock lock1(m_mutex, std::defer_lock);
			std::unique_lock lock2(other.m_mutex, std::defer_lock);
			std::lock(lock1, lock2);
			m_instances = std::move(other.m_instances);
		}

		Locked(const T&... args) { m_instances = std::make_tuple(args...); }
		Locked(T&&... args) { m_instances = std::make_tuple(args...); }
		Locked& operator=(const std::tuple<T...>& arg) {
			std::unique_lock lock(m_mutex);
			m_instances = arg;
		}
		Locked& operator=(std::tuple<T...>&& arg) {
			std::unique_lock lock(m_mutex);
			m_instances = std::move(arg);
		}

		friend bool operator==(const Locked& a_arg1, const Locked& a_arg2) {
			std::shared_lock lock1(a_arg1.m_mutex, std::defer_lock);
			std::shared_lock lock2(a_arg2.m_mutex, std::defer_lock);
			std::lock(lock1, lock2);
			return a_arg1.m_instances == a_arg2.m_instances;
		}

		friend bool operator<(const Locked& a_arg1, const Locked& a_arg2) {
			std::shared_lock lock1(a_arg1.m_mutex, std::defer_lock);
			std::shared_lock lock2(a_arg2.m_mutex, std::defer_lock);
			std::lock(lock1, lock2);
			return a_arg1.m_instances < a_arg2.m_instances;
		}

		auto operator()(std::invocable<const T&...> auto a_operation) const {
			std::shared_lock lock(m_mutex);
			return std::apply(a_operation, m_instances);
		}

		auto operator()(std::invocable<T&...> auto a_operation) {
			std::unique_lock lock(m_mutex);
			return std::apply(a_operation, m_instances);
		}

		template<std::invocable<const T&...> OperationType>
		auto Try(OperationType a_operation) const {
			constexpr bool voidFunc = std::is_void_v<decltype(std::apply(a_operation, m_instances))>;
			if(!m_mutex.try_lock()) {
				if constexpr(voidFunc) {
					return false;
				} else {
					return std::nullopt;
				}
			}
			std::shared_lock lock(m_mutex, std::adopt_lock);

			if constexpr(voidFunc) {
				std::apply(a_operation, m_instances);
				return true;
			} else {
				return std::apply(a_operation, m_instances);
			}
		}

		template<std::invocable<T&...> OperationType>
		auto Try(OperationType a_operation) {
			constexpr bool voidFunc = std::is_void_v<decltype(std::apply(a_operation, m_instances))>;
			if(!m_mutex.try_lock()) {
				if constexpr(voidFunc) {
					return false;
				} else {
					return std::nullopt;
				}
			}
			std::unique_lock lock(m_mutex, std::adopt_lock);

			if constexpr(voidFunc) {
				std::apply(a_operation, m_instances);
				return true;
			} else {
				return std::apply(a_operation, m_instances);
			}
		}

	  private:
		std::tuple<T...> m_instances;
		mutable std::shared_mutex m_mutex;
	};

	template<std::semiregular... T>
	class MultiLock final {
	  public:
		MultiLock() = delete;    // doesn't make sense to create one of these with no child locks
		MultiLock(Locked<T>&... a_arg) : m_locks(a_arg...) {}
		~MultiLock()                               = default;
		MultiLock(MultiLock&)                      = delete;
		MultiLock& operator=(MultiLock&)           = delete;
		MultiLock(MultiLock&&) noexcept            = default;
		MultiLock& operator=(MultiLock&&) noexcept = default;

		template<std::invocable<const T&...> OperationType>
		auto operator()(OperationType a_operation) const {
			std::apply(AcquireLock, m_locks);
			auto release = make_scope_exit([this]() { std::apply(ReleaseLocks, m_locks); });
			auto data    = std::apply(BuildTuple, m_locks);
			return std::apply(a_operation, data);
		}

		template<std::invocable<T&...> OperationType>
		auto operator()(OperationType a_operation) {
			std::apply(AcquireLock, m_locks);
			defer({ std::apply(ReleaseLocks, m_locks); });
			auto data = std::apply(BuildTuple, m_locks);
			return std::apply(a_operation, data);
		}

		template<std::invocable<const T&...> OperationType>
		auto Try(OperationType a_operation) const
		    -> std::enable_if_t<!std::is_void<std::invoke_result_t<OperationType(T&...)>>::value,
		                        std::optional<std::invoke_result_t<OperationType(T&...)>>> {
			if(!std::apply(TryAcquireLock, m_locks)) return std::nullopt;
			auto release = make_scope_exit([this]() { std::apply(ReleaseLocks, m_locks); });
			auto data    = std::apply(BuildTuple, m_locks);
			return std::apply(a_operation, data);
		}

		template<std::invocable<T&...> OperationType>
		auto Try(OperationType a_operation)
		    -> std::enable_if_t<!std::is_void<std::invoke_result_t<OperationType(T&...)>>::value,
		                        std::optional<std::invoke_result_t<OperationType(T&...)>>> {
			if(!std::apply(TryAcquireLock, m_locks)) return std::nullopt;
			auto release = make_scope_exit([this]() { std::apply(ReleaseLocks, m_locks); });
			auto data    = std::apply(BuildTuple, m_locks);
			return std::apply(a_operation, data);
		}

		template<std::invocable<const T&...> OperationType>
		auto Try(OperationType a_operation) const
		    -> std::enable_if_t<std::is_void<std::invoke_result_t<OperationType(T&...)>>::value, bool> {
			if(!std::apply(TryAcquireLock, m_locks)) return false;
			auto release = make_scope_exit([this]() { std::apply(ReleaseLocks, m_locks); });
			auto data    = std::apply(BuildTuple, m_locks);
			std::apply(a_operation, data);
			return true;
		}

		template<std::invocable<T&...> OperationType>
		auto Try(OperationType a_operation)
		    -> std::enable_if_t<std::is_void<std::invoke_result_t<OperationType(T&...)>>::value, bool> {
			if(!std::apply(TryAcquireLock, m_locks)) return false;
			auto release = make_scope_exit([this]() { std::apply(ReleaseLocks, m_locks); });
			auto data    = std::apply(BuildTuple, m_locks);
			std::apply(a_operation, data);
			return true;
		}

	  private:
		static void AcquireLock(Locked<T>&... a_locks) { std::lock(a_locks.m_mutex...); }

		static bool TryAcquireLock(Locked<T>&... a_locks) { return std::try_lock(a_locks.m_mutex...) != -1; }

		template<typename FirstT>
		static void ReleaseMutex(FirstT& a_first) {
			a_first.unlock();
		}

		template<typename FirstT, typename... RestT>
		static void ReleaseMutex(FirstT& a_first, RestT&... a_rest) {
			a_first.unlock();
			ReleaseMutex(a_rest...);
		}

		static void ReleaseLocks(Locked<T>&... a_locks) { ReleaseMutex(a_locks.m_mutex...); }

		template<typename FirstType>
		static void ReleaseLock(Locked<FirstType>& a_first) {
			a_first.m_mutex.unlock();
		}

		template<typename FirstType, typename... RestType>
		static void ReleaseLock(Locked<FirstType>& a_first, Locked<RestType>&... a_rest) {
			a_first.m_mutex.unlock();
			ReleaseLock(a_rest);
		}

		static auto BuildTuple(Locked<T>&... a_locks) { return std::tuple_cat(a_locks.m_instances...); }

		std::tuple<Locked<T>&...> m_locks;
	};

	template<std::semiregular... T>
	MultiLock<T...> MakeMultiLock(Locked<T>&... a_arg) {
		return MultiLock<T...>{a_arg...};
	}
}    // namespace CR::Engine::Core
