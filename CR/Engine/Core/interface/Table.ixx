module;

#include <core/Log.h>

export module CR.Engine.Core.Table;

import CR.Engine.Core.TypeTraits;

import<array>;
import<bitset>;
import<cinttypes>;
import<concepts>;
import<string>;
import<tuple>;
import<type_traits>;
import<unordered_map>;
import<utility>;

export namespace CR::Engine::Core {
	template<typename... t_columnsSubset>
	class TableBinding {
		using t_refType = std::tuple<t_columnsSubset*...>;
		using t_valType = std::tuple<t_columnsSubset...>;

	  public:
		TableBinding() = default;
		TableBinding(t_columnsSubset*... a_data) : m_dataRef(a_data...) {}
		~TableBinding() = default;

		TableBinding(const TableBinding& a_other) {
			CreateCopy(a_other.m_dataRef, std::index_sequence_for<t_columnsSubset...>{});
		}

		TableBinding(TableBinding&& a_other) = delete;

		TableBinding& operator=(const TableBinding& a_other) {
			CreateCopy(a_other.m_dataRef, std::index_sequence_for<t_columnsSubset...>{});
			return *this;
		}
		TableBinding& operator=(TableBinding&& a_other) = delete;

		template<std::size_t Index>
		std::tuple_element_t<Index, TableBinding>& get() {
			return *std::get<Index>(m_dataRef);
		}

		template<std::size_t Index>
		const std::tuple_element_t<Index, TableBinding>& get() const {
			return *std::get<Index>(m_dataRef);
		}

		void Set(t_columnsSubset*... a_data) { m_dataRef = t_refType(a_data...); }

		void SetNull() { m_dataRef = t_refType(static_cast<t_columnsSubset*>(nullptr)...); }

	  private:
		template<std::size_t... indices>
		void CreateCopy(const t_refType& a_other, std::index_sequence<indices...>) {
			t_valType* data = ::new(m_data) t_valType{*std::get<indices>(a_other)...};
			Set(&std::get<indices>(*data)...);
		}

		t_refType m_dataRef;
		// This is to preserver normal structured binding semantics.
		// i.e. auto [foo] will be a "copy" and auto& [foo] will be a "reference"
		// This may have perf downsides though. try to not make copies or construct these in Table's
		// implemenation when possible.
		// std::aligned_storage_t wont compile, not sure why.
		// std::aligned_storage_t<sizeof(t_valType), alignof(t_valType)> m_data;
		alignas(t_valType) std::byte m_data[sizeof(t_valType)];
	};

	// t_column... is a struct(or primitive) holding the data for one column of a row. t_column... should not
	// contain the primary key. You can have more than one t_column type. This allows creating a SOAOS layout.
	// The primary key is in one array, and each column in an array. You can iterate the rows of the table for
	// just one column. You can grab multiple iterators if you need to walk multiple columns at once. The
	// fastest way to get to a row for a column is by index, not by primary key, you can save these indices
	// for future use. Indices are stable.
	//
	// c_maxSize - maximum rows. this many rows is allocated up front currently. Can use a lot of memory
	// There can be no more than 64K-1 rows. If you need more than that, probably should be writing custom
	// code. Using arrays currently, and std::bitset. For larger tables need a bitset replacement(which would
	// help here too). And should use std::vector instead of arrays for larger tables.
	template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
	class Table {
		static_assert(is_unique_v<t_columns...>, "column types must be unique currently");
		static_assert(c_maxSize < 0xffff, "no more than 64K-1 rows allowed");

		// If column is larger than this, should refactor. Bulk data should be stored elsewhere on the
		// heap(i.e. unique_ptr, or a vector). If still larger than this, then you probably aren't in
		// boyce-codd normal form. Or you just need to split up the table. You can of course have multiple
		// columns.
		static constexpr uint32_t c_maxRowSize = 64;
		static_assert((... && (sizeof(t_columns) <= c_maxRowSize)));

	  public:
		// A system wide constant, need to find a better place for it. Its duplicated in many public
		// interfaces
		inline static constexpr uint16_t c_unused{0xffff};

		Table(std::string_view a_tableName) :
		    m_tableName(a_tableName), m_defaultView(GetView<t_columns...>()){};
		~Table() = default;

		Table(const Table&) = delete;
		Table(Table&&)      = delete;

		Table& operator=(const Table&) = delete;
		Table& operator=(Table&&) = delete;

		// returns the new index, fatal error if table is full
		template<typename t_primaryKeyF, typename... t_columnsF>
		uint16_t insert(t_primaryKeyF&& a_key, t_columnsF&&... a_row) requires
		    !std::same_as<t_primaryKey, std::string>;
		template<typename... t_columnsF>
		uint16_t insert(const std::string_view a_key,
		                t_columnsF&&... a_row) requires std::same_as<t_primaryKey, std::string>;
		// row will be default constructed if not standard layout, otherwise uninitialized
		template<typename t_primaryKeyF>
		uint16_t insert(t_primaryKeyF&& a_key) requires !std::same_as<t_primaryKey, std::string>;
		uint16_t insert(const std::string_view a_key) requires std::same_as<t_primaryKey, std::string>;

		// problematic when t_primaryKey is a string type(char*, string, string_view, ect) should just take a
		// string_view for those. Wait until C++20 to fix, can be done cleaner there.
		[[nodiscard]] uint16_t GetIndex(const t_primaryKey& a_key) const;

		template<typename t_value>
		[[nodiscard]] const t_value& GetValue(uint16_t a_index) const;
		template<typename t_value>
		[[nodiscard]] t_value& GetValue(uint16_t a_index);

		[[nodiscard]] const std::tuple<const t_columns&...> operator[](uint16_t a_index) const;
		[[nodiscard]] std::tuple<t_columns&...> operator[](uint16_t a_index);

		void erase(uint16_t a_index);

		template<typename... t_columnsSubset>
		class Iterator {
		  public:
			using iterator_category = std::bidirectional_iterator_tag;
			using difference_type   = int32_t;
			using value_type        = TableBinding<std::decay_t<t_columnsSubset>...>;
			using pointer           = value_type*;
			using reference         = value_type&;

			Iterator() = delete;
			Iterator(Table& a_table, uint16_t a_index);
			~Iterator()                   = default;
			Iterator(const Iterator&)     = default;
			Iterator(Iterator&&) noexcept = default;
			Iterator& operator=(const Iterator&) = default;
			Iterator& operator=(Iterator&&) noexcept = default;

			reference operator*();
			pointer operator->();

			Iterator& operator++();
			Iterator& operator++(int);

			Iterator& operator--();
			Iterator& operator--(int);

			friend bool operator==(const Iterator& a_first, const Iterator& a_second) {
				if(&a_first.m_table != &a_second.m_table) { return false; }
				return a_first.m_index == a_second.m_index;
			}
			friend bool operator!=(const Iterator& a_first, const Iterator& a_second) {
				return !(a_first == a_second);
			}

		  private:
			void SetBinding();

			Table& m_table;
			uint16_t m_index{0};
			value_type m_binding{};
		};

		template<typename... t_columnsSubset>
		class ConstIterator {
		  public:
			using iterator_category = std::bidirectional_iterator_tag;
			using difference_type   = int32_t;
			using value_type        = TableBinding<const std::decay_t<t_columnsSubset>...>;
			using pointer           = const value_type*;
			using reference         = const value_type&;

			ConstIterator() = delete;
			ConstIterator(const Table& a_table, uint16_t a_index);
			~ConstIterator()                        = default;
			ConstIterator(const ConstIterator&)     = default;
			ConstIterator(ConstIterator&&) noexcept = default;
			ConstIterator& operator=(const ConstIterator&) = default;
			ConstIterator& operator=(ConstIterator&&) noexcept = default;

			reference operator*();
			pointer operator->();

			ConstIterator& operator++();
			ConstIterator& operator++(int);

			ConstIterator& operator--();
			ConstIterator& operator--(int);

			friend bool operator==(const ConstIterator& a_first, const ConstIterator& a_second) {
				if(&a_first.m_table != &a_second.m_table) { return false; }
				return a_first.m_index == a_second.m_index;
			}
			friend bool operator!=(const ConstIterator& a_first, const ConstIterator& a_second) {
				return !(a_first == a_second);
			}

		  private:
			void SetBinding();

			const Table& m_table;
			uint16_t m_index{0};
			value_type m_binding{};
		};

		template<typename... t_viewSubset>
		class View {
		  public:
			View() = delete;
			View(Table& a_table) : m_table(a_table) {}
			~View()               = default;
			View(const View&)     = default;
			View(View&&) noexcept = default;
			View& operator=(const View&) = default;
			View& operator=(View&&) noexcept = default;

			Iterator<t_viewSubset...> begin() { return Iterator<t_viewSubset...>(m_table, 0); }
			Iterator<t_viewSubset...> end() { return Iterator<t_viewSubset...>(m_table, c_maxSize); }
			ConstIterator<t_viewSubset...> begin() const { return ConstIterator<t_viewSubset...>(&this, 0); }
			ConstIterator<t_viewSubset...> end() const {
				return ConstIterator<t_viewSubset...>(&this, c_maxSize);
			}
			ConstIterator<t_viewSubset...> cbegin() const { return ConstIterator<t_viewSubset...>(&this, 0); }
			ConstIterator<t_viewSubset...> cend() const {
				return ConstIterator<t_viewSubset...>(&this, c_maxSize);
			}

		  private:
			Table& m_table;
		};

		template<typename... t_viewSubset>
		class ConstView {
		  public:
			ConstView() = delete;
			ConstView(const Table& a_table) : m_table(a_table) {}
			~ConstView()                    = default;
			ConstView(const ConstView&)     = default;
			ConstView(ConstView&&) noexcept = default;
			ConstView& operator=(const ConstView&) = default;
			ConstView& operator=(ConstView&&) noexcept = default;

			ConstIterator<t_viewSubset...> begin() { return ConstIterator<t_viewSubset...>(m_table, 0); }
			ConstIterator<t_viewSubset...> end() {
				return ConstIterator<t_viewSubset...>(m_table, c_maxSize);
			}
			ConstIterator<t_viewSubset...> begin() const { return ConstIterator<t_viewSubset...>(&this, 0); }
			ConstIterator<t_viewSubset...> end() const {
				return ConstIterator<t_viewSubset...>(&this, c_maxSize);
			}
			ConstIterator<t_viewSubset...> cbegin() const { return ConstIterator<t_viewSubset...>(&this, 0); }
			ConstIterator<t_viewSubset...> cend() const {
				return ConstIterator<t_viewSubset...>(&this, c_maxSize);
			}

		  private:
			const Table& m_table;
		};

		template<typename... t_viewSubset>
		[[nodiscard]] View<t_viewSubset...> GetView() {
			return View<t_viewSubset...>(*this);
		}

		template<typename... t_viewSubset>
		[[nodiscard]] const ConstView<t_viewSubset...> GetView() const {
			return ConstView<t_viewSubset...>(*this);
		}

		Iterator<t_columns...> begin() { return m_defaultView.begin(); }
		Iterator<t_columns...> end() { return m_defaultView.end(); }

		ConstIterator<t_columns...> begin() const { return m_defaultView.begin(); }
		ConstIterator<t_columns...> end() const { return m_defaultView.end(); }
		ConstIterator<t_columns...> cbegin() const { return m_defaultView.cbegin(); }
		ConstIterator<t_columns...> cend() const { return m_defaultView.cend(); }

	  private:
		// returns c_maxSize if couldn't find one
		[[nodiscard]] uint16_t FindUnused();

		template<typename t_column>
		void ClearColumnDefault(uint16_t a_index);
		template<size_t index, typename t_column>
		void ClearColumn(uint16_t a_index, t_column&& a_column);
		void ClearRowDefault(uint16_t a_index);
		template<size_t... tupleIndices>
		void ClearRow(uint16_t a_index, std::index_sequence<tupleIndices...>, t_columns&&... row);

		template<std::size_t... I>
		[[nodiscard]] std::tuple<t_columns&...> GetValues(uint16_t a_index, std::index_sequence<I...>);

		std::string m_tableName;
		std::bitset<c_maxSize> m_used;
		t_primaryKey m_primaryKeys[c_maxSize];
		std::unordered_map<t_primaryKey, uint16_t> m_lookUp;
		std::tuple<std::array<t_columns, c_maxSize>...> m_rows;
		View<t_columns...> m_defaultView;
	};
}    // namespace CR::Engine::Core

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
inline uint16_t CR::Engine::Core::Table<c_maxSize, t_primaryKey, t_columns...>::FindUnused() {
	// TODO: std::bitset sucks, unless its 64 bits or less, can't do a bitscan manually even.
	for(uint16_t i = 0; i < c_maxSize; ++i) {
		if(!m_used[i]) { return i; }
	}
	return c_maxSize;
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename t_column>
void CR::Engine::Core::Table<c_maxSize, t_primaryKey, t_columns...>::ClearColumnDefault(
    [[maybe_unused]] uint16_t a_index) {
	if constexpr(!std::is_standard_layout_v<t_column>) { std::get<t_column>(m_rows)[a_index] = t_column{}; }
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<size_t index, typename t_column>
void CR::Engine::Core::Table<c_maxSize, t_primaryKey, t_columns...>::ClearColumn(
    [[maybe_unused]] uint16_t a_index, [[maybe_unused]] t_column&& a_column) {
	if constexpr(!std::is_standard_layout_v<t_column>) {
		std::get<index>(m_rows)[a_index] = std::move(a_column);
	}
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
void CR::Engine::Core::Table<c_maxSize, t_primaryKey, t_columns...>::ClearRowDefault(uint16_t a_index) {
	(ClearColumnDefault<t_columns>(a_index), ...);
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<size_t... tupleIndices>
void CR::Engine::Core::Table<c_maxSize, t_primaryKey, t_columns...>::ClearRow(
    uint16_t a_index, std::index_sequence<tupleIndices...>, t_columns&&... a_row) {
	(ClearColumn<tupleIndices>(a_index, std::move(a_row)), ...);
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename t_primaryKeyF, typename... t_columnsF>
inline uint16_t CR::Engine::Core::Table<c_maxSize, t_primaryKey, t_columns...>::insert(
    t_primaryKeyF&& a_key, t_columnsF&&... a_row) requires !std::same_as<t_primaryKey, std::string> {
	uint16_t unusedIndex = insert(std::forward<t_primaryKeyF>(a_key));

	ClearRow(unusedIndex, std::index_sequence_for<t_columnsF...>{}, std::forward<t_columnsF>(a_row)...);

	return unusedIndex;
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename... t_columnsF>
inline uint16_t CR::Engine::Core::Table<c_maxSize, t_primaryKey, t_columns...>::insert(
    const std::string_view a_key, t_columnsF&&... a_row) requires std::same_as<t_primaryKey, std::string> {
	uint16_t unusedIndex = insert(a_key);

	ClearRow(unusedIndex, std::index_sequence_for<t_columnsF...>{}, std::forward<t_columnsF>(a_row)...);

	return unusedIndex;
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename t_primaryKeyF>
inline uint16_t
    CR::Engine::Core::Table<c_maxSize, t_primaryKey, t_columns...>::insert(t_primaryKeyF&& a_key) requires
    !std::same_as<t_primaryKey, std::string> {
	CR_REQUIRES_AUDIT(m_lookUp.find(a_key) == std::end(m_lookUp),
	                  "Tried to insert, but row already exists with this key {}", m_tableName);

	uint16_t unusedIndex = FindUnused();
	CR_REQUIRES_AUDIT(unusedIndex != c_maxSize, "Ran out of available rows in table {}", m_tableName);

	m_lookUp.emplace(a_key, unusedIndex);
	m_primaryKeys[unusedIndex] = std::forward<t_primaryKeyF>(a_key);

	m_used[unusedIndex] = true;

	return unusedIndex;
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
inline uint16_t CR::Engine::Core::Table<c_maxSize, t_primaryKey, t_columns...>::insert(
    const std::string_view a_key) requires std::same_as<t_primaryKey, std::string> {
	CR_REQUIRES_AUDIT(m_lookUp.find(std::string(a_key)) == std::end(m_lookUp),
	                  "Tried to insert, but row already exists with this key {}", m_tableName);

	uint16_t unusedIndex = FindUnused();
	CR_REQUIRES_AUDIT(unusedIndex != c_maxSize, "Ran out of available rows in table {}", m_tableName);

	m_lookUp.emplace(a_key, unusedIndex);
	m_primaryKeys[unusedIndex] = a_key;

	m_used[unusedIndex] = true;

	return unusedIndex;
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
inline uint16_t CR::Engine::Core::Table<c_maxSize, t_primaryKey, t_columns...>::GetIndex(
    const t_primaryKey& a_key) const {
	auto iter = m_lookUp.find(a_key);
	if(iter == m_lookUp.end()) {
		return c_unused;
	} else {
		return iter->second;
	}
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename t_value>
inline const t_value&
    CR::Engine::Core::Table<c_maxSize, t_primaryKey, t_columns...>::GetValue(uint16_t a_index) const {
	CR_REQUIRES_AUDIT(a_index != c_unused && m_used[a_index], "asked for an unused row in table {}",
	                  m_tableName);

	return std::get<std::array<t_value, c_maxSize>>(m_rows)[a_index];
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename t_value>
inline t_value& CR::Engine::Core::Table<c_maxSize, t_primaryKey, t_columns...>::GetValue(uint16_t a_index) {
	CR_REQUIRES_AUDIT(a_index != c_unused && m_used[a_index], "asked for an unused row in table {}",
	                  m_tableName);

	return std::get<std::array<t_value, c_maxSize>>(m_rows)[a_index];
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<std::size_t... I>
[[nodiscard]] std::tuple<t_columns&...>
    CR::Engine::Core::Table<c_maxSize, t_primaryKey, t_columns...>::GetValues(uint16_t a_index,
                                                                              std::index_sequence<I...>) {
	return std::tuple<t_columns&...>(std::get<I>(m_rows)[a_index]...);
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
[[nodiscard]] const std::tuple<const t_columns&...>
    CR::Engine::Core::Table<c_maxSize, t_primaryKey, t_columns...>::operator[](uint16_t a_index) const {
	CR_REQUIRES_AUDIT(a_index != c_unused && m_used[a_index], "asked for an unused row in table {}",
	                  m_tableName);

	return GetValues(a_index, std::index_sequence_for<t_columns...>{});
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
[[nodiscard]] std::tuple<t_columns&...>
    CR::Engine::Core::Table<c_maxSize, t_primaryKey, t_columns...>::operator[](uint16_t a_index) {
	return GetValues(a_index, std::index_sequence_for<t_columns...>{});
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
inline void CR::Engine::Core::Table<c_maxSize, t_primaryKey, t_columns...>::erase(uint16_t a_index) {
	CR_REQUIRES_AUDIT(a_index != c_unused && m_used[a_index], "tried to delete an unused row in table {}",
	                  m_tableName);

	m_used[a_index] = false;

	// To save on memory, if not a standard layout, then clear out the data. Hopefully if not standard layout
	// then a proper move assignment was written.
	if constexpr(!std::is_standard_layout_v<t_primaryKey>) { m_primaryKeys[a_index] = t_primaryKey{}; }
	ClearRowDefault(a_index);
}

//////////////////////////////
// Iterator
//////////////////////////////

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename... t_columnsSubset>
CR::Engine::Core::Table<c_maxSize, t_primaryKey, t_columns...>::Iterator<t_columnsSubset...>::Iterator(
    Table& a_table, uint16_t a_index) :
    m_table(a_table),
    m_index(a_index) {
	while(m_index < c_maxSize && !m_table.m_used[m_index]) { ++m_index; }
	SetBinding();
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename... t_columnsSubset>
auto CR::Engine::Core::Table<c_maxSize, t_primaryKey,
                             t_columns...>::Iterator<t_columnsSubset...>::operator++() -> Iterator& {
	if(m_index == c_maxSize) { return *this; }
	++m_index;
	while(m_index < c_maxSize && !m_table.m_used[m_index]) { ++m_index; }
	SetBinding();
	return *this;
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename... t_columnsSubset>
auto CR::Engine::Core::Table<c_maxSize, t_primaryKey, t_columns...>::Iterator<t_columnsSubset...>::operator++(
    int) -> Iterator& {
	Iterator tmp = *this;
	++(*this);
	return tmp;
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename... t_columnsSubset>
auto CR::Engine::Core::Table<c_maxSize, t_primaryKey,
                             t_columns...>::Iterator<t_columnsSubset...>::operator--() -> Iterator& {
	if(m_index == 0) { return *this; }
	uint16_t oldIndex = m_index;
	--m_index;
	while(m_index > 0 && !m_table.m_used[m_index]) { --m_index; }
	if(!m_table.m_used[m_index]) { m_index = oldIndex; }
	SetBinding();
	return *this;
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename... t_columnsSubset>
auto CR::Engine::Core::Table<c_maxSize, t_primaryKey, t_columns...>::Iterator<t_columnsSubset...>::operator--(
    int) -> Iterator& {
	Iterator tmp = *this;
	--(*this);
	return tmp;
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename... t_columnsSubset>
auto CR::Engine::Core::Table<c_maxSize, t_primaryKey, t_columns...>::Iterator<t_columnsSubset...>::operator*()
    -> reference {
	return m_binding;
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename... t_columnsSubset>
auto CR::Engine::Core::Table<c_maxSize, t_primaryKey,
                             t_columns...>::Iterator<t_columnsSubset...>::operator->() -> pointer {
	return &m_binding;
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename... t_columnsSubset>
void CR::Engine::Core::Table<c_maxSize, t_primaryKey,
                             t_columns...>::Iterator<t_columnsSubset...>::SetBinding() {
	if(m_index >= c_maxSize || !m_table.m_used[m_index]) {
		m_binding.SetNull();
	} else {
		m_binding.Set((&std::get<std::array<t_columnsSubset, c_maxSize>>(m_table.m_rows)[m_index])...);
	}
}

//////////////////////////////
// ConstIterator
//////////////////////////////

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename... t_columnsSubset>
CR::Engine::Core::Table<c_maxSize, t_primaryKey,
                        t_columns...>::ConstIterator<t_columnsSubset...>::ConstIterator(const Table& a_table,
                                                                                        uint16_t a_index) :
    m_table(a_table),
    m_index(a_index) {
	while(m_index < c_maxSize && !m_table.m_used[m_index]) { ++m_index; }
	SetBinding();
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename... t_columnsSubset>
auto CR::Engine::Core::Table<c_maxSize, t_primaryKey,
                             t_columns...>::ConstIterator<t_columnsSubset...>::operator++()
    -> ConstIterator& {
	if(m_index == c_maxSize) { return *this; }
	++m_index;
	while(m_index < c_maxSize && !m_table.m_used[m_index]) { ++m_index; }
	SetBinding();
	return *this;
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename... t_columnsSubset>
auto CR::Engine::Core::Table<c_maxSize, t_primaryKey,
                             t_columns...>::ConstIterator<t_columnsSubset...>::operator++(int)
    -> ConstIterator& {
	Iterator tmp = *this;
	++(*this);
	return tmp;
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename... t_columnsSubset>
auto CR::Engine::Core::Table<c_maxSize, t_primaryKey,
                             t_columns...>::ConstIterator<t_columnsSubset...>::operator--()
    -> ConstIterator& {
	if(m_index == 0) { return *this; }
	uint16_t oldIndex = m_index;
	--m_index;
	while(m_index > 0 && !m_table.m_used[m_index]) { --m_index; }
	if(!m_table.m_used[m_index]) { m_index = oldIndex; }
	SetBinding();
	return *this;
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename... t_columnsSubset>
auto CR::Engine::Core::Table<c_maxSize, t_primaryKey,
                             t_columns...>::ConstIterator<t_columnsSubset...>::operator--(int)
    -> ConstIterator& {
	Iterator tmp = *this;
	--(*this);
	return tmp;
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename... t_columnsSubset>
auto CR::Engine::Core::Table<c_maxSize, t_primaryKey,
                             t_columns...>::ConstIterator<t_columnsSubset...>::operator*() -> reference {
	return m_binding;
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename... t_columnsSubset>
auto CR::Engine::Core::Table<c_maxSize, t_primaryKey,
                             t_columns...>::ConstIterator<t_columnsSubset...>::operator->() -> pointer {
	return &m_binding;
}

template<uint16_t c_maxSize, std::regular t_primaryKey, typename... t_columns>
template<typename... t_columnsSubset>
void CR::Engine::Core::Table<c_maxSize, t_primaryKey,
                             t_columns...>::ConstIterator<t_columnsSubset...>::SetBinding() {
	if(m_index >= c_maxSize || !m_table.m_used[m_index]) {
		m_binding.SetNull();
	} else {
		m_binding.Set((&std::get<std::array<t_columnsSubset, c_maxSize>>(m_table.m_rows)[m_index])...);
	}
}

namespace std {
	template<typename... t_columnsSubset>
	struct tuple_size<::CR::Engine::Core::TableBinding<t_columnsSubset...>>
	    : integral_constant<size_t, sizeof...(t_columnsSubset)> {};

	template<size_t Index, typename... t_columnsSubset>
	struct tuple_element<Index, ::CR::Engine::Core::TableBinding<t_columnsSubset...>>
	    : tuple_element<Index, tuple<t_columnsSubset...>> {};
}    // namespace std