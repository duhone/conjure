/*
similar to std::function
both hold a list of functions. MultiFunction will call every assigned function when invoked.
SelectableFunction will invoke a single one of its functions which can be independently selected.
*/
module;

// replace in c++23, move_only_function I think it will be called.
#include <function2/function2.hpp>

export module CR.Engine.Core.Function;

import CR.Engine.Core.Algorithm;

import<algorithm>;
import<concepts>;
import<functional>;
import<vector>;

export namespace CR::Engine::Core {
	template<typename CallableT>
	class MultiFunction {};

	// no return type for multifunction, who would win?
	template<typename ReturnType, std::semiregular... ArgTypes>
	class MultiFunction<ReturnType(ArgTypes...)> final {
		static_assert(std::is_same_v<ReturnType, void>, "MultiFunction only works with void return type");

	  public:
		using OperationT = fu2::unique_function<ReturnType(ArgTypes...)>;

		MultiFunction()                         = default;
		~MultiFunction()                        = default;
		MultiFunction(const MultiFunction&)     = delete;
		MultiFunction(MultiFunction&&) noexcept = default;
		MultiFunction& operator=(const MultiFunction&) = delete;
		MultiFunction& operator=(MultiFunction&&) noexcept = default;

		[[nodiscard]] std::size_t size() const noexcept { return m_operations.size(); }

		MultiFunction& operator=(std::nullptr_t) {
			m_operations.clear();
			return *this;
		}

		MultiFunction& operator=(OperationT a_operation) {
			*this = nullptr;
			*this += std::move(a_operation);
			return *this;
		}

		inline void operator+=(OperationT a_operation) { m_operations.push_back(std::move(a_operation)); }

		template<std::semiregular... FArgTypes>
		void operator()(FArgTypes&&... a_params) {
			for(auto& op : m_operations) { op(std::forward<FArgTypes>(a_params)...); }
		}

		explicit operator bool() const {
			return std::ranges::all_of(m_operations, [](const OperationT& a_op) { return static_cast<bool>(a_op); });
		}

	  private:
		std::vector<OperationT> m_operations;
	};

	template<typename CallableT>
	class SelectableFunction {};

	template<typename ReturnType, std::semiregular... ArgTypes>
	class SelectableFunction<ReturnType(ArgTypes...)> final {
	  public:
		using OperationT = fu2::unique_function<ReturnType(ArgTypes...)>;

		SelectableFunction()                              = default;
		~SelectableFunction()                             = default;
		SelectableFunction(const SelectableFunction&)     = delete;
		SelectableFunction(SelectableFunction&&) noexcept = default;
		SelectableFunction& operator=(const SelectableFunction&) = delete;
		SelectableFunction& operator=(SelectableFunction&&) noexcept = default;

		[[nodiscard]] std::size_t size() const { return m_operations.size(); }

		SelectableFunction& operator=(std::nullptr_t) {
			m_operations.clear();
			return *this;
		}

		SelectableFunction& operator=(std::size_t a_op) {
			SetOperation(a_op);
			return *this;
		}

		SelectableFunction& operator=(OperationT a_operation) {
			*this = nullptr;
			*this += std::move(a_operation);
			return *this;
		}

		void operator+=(OperationT a_operation) { m_operations.push_back(std::move(a_operation)); }

		template<std::semiregular... FArgTypes>
		ReturnType operator()(FArgTypes&&... a_params) {
			if(m_currentOperation >= 0 && m_currentOperation < m_operations.size()) {
				return m_operations.at(m_currentOperation)(std::forward<FArgTypes>(a_params)...);
			}
			return ReturnType{};
		}

		explicit operator bool() const { return static_cast<bool>(m_operations.at(m_currentOperation)); }

		void SetOperation(std::size_t a_op) { m_currentOperation = a_op; }

		OperationT& operator[](std::size_t a_index) { return m_operations.at(a_index); }

	  private:
		std::vector<OperationT> m_operations;
		std::size_t m_currentOperation{0};
	};
}    // namespace CR::Engine::Core
