#pragma once

#include "../../Util.h"

namespace Ubpa::UDRefl::details {
	template<typename T, typename = void>
	struct IsArray : std::false_type {};
	template<typename T>
	struct IsArray < T, std::enable_if_t <
		// - iterator

		is_valid_v<container_begin, T>
		&& is_valid_v<container_begin_const, T>
		&& is_valid_v<container_cbegin, T>

		&& is_valid_v<container_end, T>
		&& is_valid_v<container_end_const, T>
		&& is_valid_v<container_cend, T>

		&& is_valid_v<container_rbegin, T>
		&& is_valid_v<container_rbegin_const, T>
		&& is_valid_v<container_crbegin, T>

		&& is_valid_v<container_rend, T>
		&& is_valid_v<container_rend_const, T>
		&& is_valid_v<container_crend, T>

		// - element access

		&& is_valid_v<container_at, T>
		&& is_valid_v<container_at_const, T>
		&& is_valid_v<operator_subscript, T>
		&& is_valid_v<operator_subscript_const, T>
		&& is_valid_v<container_data, T>
		&& is_valid_v<container_data_const, T>
		&& is_valid_v<container_front, T>
		&& is_valid_v<container_front_const, T>
		&& is_valid_v<container_back, T>
		&& is_valid_v<container_back_const, T>

		// - capacity

		&& is_valid_v<container_empty, T>
		&& is_valid_v<container_size, T>
		//&& is_valid_v<container_max_size, T>

		// - modifier

		&& is_valid_v<container_swap, T>

		>> : std::true_type{};

	template<typename T, typename = void>
	struct IsVector : std::false_type {};
	template<typename T>
	struct IsVector<T, std::enable_if_t<
		is_valid_v<container_assign, T>
		
		// - iterator

		&& is_valid_v<container_begin, T>
		&& is_valid_v<container_begin_const, T>
		&& is_valid_v<container_cbegin, T>

		&& is_valid_v<container_end, T>
		&& is_valid_v<container_end_const, T>
		&& is_valid_v<container_cend, T>

		&& is_valid_v<container_rbegin, T>
		&& is_valid_v<container_rbegin_const, T>
		&& is_valid_v<container_crbegin, T>

		&& is_valid_v<container_rend, T>
		&& is_valid_v<container_rend_const, T>
		&& is_valid_v<container_crend, T>

		// - element access

		&& is_valid_v<container_at, T>
		&& is_valid_v<container_at_const, T>
		&& is_valid_v<operator_subscript, T>
		&& is_valid_v<operator_subscript_const, T>
		&& is_valid_v<container_data, T>
		&& is_valid_v<container_data_const, T>
		&& is_valid_v<container_front, T>
		&& is_valid_v<container_front_const, T>
		&& is_valid_v<container_back, T>
		&& is_valid_v<container_back_const, T>

		// - capacity

		&& is_valid_v<container_empty, T>
		&& is_valid_v<container_size, T>
		//&& is_valid_v<container_max_size, T>
		&& is_valid_v<container_resize_0, T>
		&& is_valid_v<container_resize_1, T>
		&& is_valid_v<container_capacity, T>
		&& is_valid_v<container_reserve, T>
		&& is_valid_v<container_shrink_to_fit, T>

		// - modifier

		&& is_valid_v<container_clear, T>
		&& is_valid_v<container_insert_0, T>
		&& is_valid_v<container_insert_1, T>
		&& is_valid_v<container_insert_2, T>
		&& is_valid_v<container_erase_1, T> // C++11
		&& is_valid_v<container_erase_3, T> // C++11
		&& is_valid_v<container_push_back_0, T>
		&& is_valid_v<container_push_back_1, T>
		&& is_valid_v<container_pop_back, T>
		&& is_valid_v<container_swap, T>

		// - allocator

		&& is_valid_v<container_get_allocator, T>

		>> : std::true_type {};

	template<typename T, typename = void>
	struct IsDeque : std::false_type {};
	template<typename T>
	struct IsDeque < T, std::enable_if_t <
		is_valid_v<container_assign, T>

		// - iterator

		&& is_valid_v<container_begin, T>
		&& is_valid_v<container_begin_const, T>
		&& is_valid_v<container_cbegin, T>

		&& is_valid_v<container_end, T>
		&& is_valid_v<container_end_const, T>
		&& is_valid_v<container_cend, T>

		&& is_valid_v<container_rbegin, T>
		&& is_valid_v<container_rbegin_const, T>
		&& is_valid_v<container_crbegin, T>

		&& is_valid_v<container_rend, T>
		&& is_valid_v<container_rend_const, T>
		&& is_valid_v<container_crend, T>

		// - element access

		&& is_valid_v<container_at, T>
		&& is_valid_v<container_at_const, T>
		&& is_valid_v<operator_subscript, T>
		&& is_valid_v<operator_subscript_const, T>
		&& is_valid_v<container_front, T>
		&& is_valid_v<container_front_const, T>
		&& is_valid_v<container_back, T>
		&& is_valid_v<container_back_const, T>

		// - capacity

		&& is_valid_v<container_empty, T>
		&& is_valid_v<container_size, T>
		//&& is_valid_v<container_max_size, T>
		&& is_valid_v<container_resize_0, T>
		&& is_valid_v<container_resize_1, T>
		&& is_valid_v<container_shrink_to_fit, T>

		// - modifier

		&& is_valid_v<container_clear, T>
		&& is_valid_v<container_insert_0, T>
		&& is_valid_v<container_insert_1, T>
		&& is_valid_v<container_insert_2, T>
		&& is_valid_v<container_erase_1, T> // C++11
		&& is_valid_v<container_erase_3, T> // C++11
		&& is_valid_v<container_push_front_0, T>
		&& is_valid_v<container_push_front_1, T>
		&& is_valid_v<container_pop_front, T>
		&& is_valid_v<container_push_back_0, T>
		&& is_valid_v<container_push_back_1, T>
		&& is_valid_v<container_pop_back, T>
		&& is_valid_v<container_swap, T>

		// - allocator

		&& is_valid_v<container_get_allocator, T>

		>> : std::true_type{};

	// TODO : list

	template<typename T, typename = void>
	struct IsSet : std::false_type {};
	template<typename T>
	struct IsSet < T, std::enable_if_t <
		// - iterator

		is_valid_v<container_begin, T>
		&& is_valid_v<container_begin_const, T>
		&& is_valid_v<container_cbegin, T>

		&& is_valid_v<container_end, T>
		&& is_valid_v<container_end_const, T>
		&& is_valid_v<container_cend, T>

		&& is_valid_v<container_rbegin, T>
		&& is_valid_v<container_rbegin_const, T>
		&& is_valid_v<container_crbegin, T>

		&& is_valid_v<container_rend, T>
		&& is_valid_v<container_rend_const, T>
		&& is_valid_v<container_crend, T>

		// - capacity

		&& is_valid_v<container_empty, T>
		&& is_valid_v<container_size, T>
		//&& is_valid_v<container_max_size, T>

		// - modifier

		&& is_valid_v<container_clear, T>
		&& is_valid_v<container_insert_0, T>
		&& is_valid_v<container_insert_1, T>
		&& is_valid_v<container_insert_2, T>
		&& is_valid_v<container_insert_3, T>
		&& is_valid_v<container_insert_4, T>
		&& is_valid_v<container_erase_0, T>
		&& is_valid_v<container_erase_1, T>
		&& is_valid_v<container_erase_3, T>
		&& is_valid_v<container_erase_4, T>
		&& is_valid_v<container_swap, T>
		&& is_valid_v<container_merge_0, T>
		&& is_valid_v<container_merge_1, T>
		&& is_valid_v<container_extract_0, T>
		&& is_valid_v<container_extract_1, T>

		// - lookup

		&& is_valid_v<container_count, T>
		&& is_valid_v<container_find, T>
		&& is_valid_v<container_find_const, T>
		&& is_valid_v<container_lower_bound, T>
		&& is_valid_v<container_lower_bound_const, T>
		&& is_valid_v<container_upper_bound, T>
		&& is_valid_v<container_upper_bound_const, T>
		&& is_valid_v<container_equal_range, T>
		&& is_valid_v<container_equal_range_const, T>

		// - observers

		&& is_valid_v<container_key_comp, T>
		&& is_valid_v<container_value_comp, T>

		// - allocator

		&& is_valid_v<container_get_allocator, T>

		>> : std::true_type{};

	template<typename T, typename = void>
	struct IsMap : std::false_type {};
	template<typename T>
	struct IsMap < T, std::enable_if_t <
		IsSet<T>::value

		// - element access

		&& is_valid_v<container_at_key, T>
		&& is_valid_v<container_at_key_const, T>
		&& is_valid_v<container_subscript_key_0, T>
		&& is_valid_v<container_subscript_key_1, T>

		// - modifier

		&& is_valid_v<container_insert_or_assign_0, T>
		&& is_valid_v<container_insert_or_assign_1, T>
		&& is_valid_v<container_insert_or_assign_2, T>
		&& is_valid_v<container_insert_or_assign_3, T>
		&& is_valid_v<container_insert_or_assign_4, T>
		&& is_valid_v<container_insert_or_assign_5, T>
		&& is_valid_v<container_insert_or_assign_6, T>
		&& is_valid_v<container_insert_or_assign_7, T>

		>> : std::true_type{};

	template<typename T, typename = void>
	struct IsUnorderedSet : std::false_type {};
	template<typename T>
	struct IsUnorderedSet < T, std::enable_if_t <
		// - iterator

		is_valid_v<container_begin, T>
		&& is_valid_v<container_begin_const, T>
		&& is_valid_v<container_cbegin, T>

		&& is_valid_v<container_end, T>
		&& is_valid_v<container_end_const, T>
		&& is_valid_v<container_cend, T>

		// - capacity

		&& is_valid_v<container_empty, T>
		&& is_valid_v<container_size, T>
		//&& is_valid_v<container_max_size, T>
		&& is_valid_v<container_bucket_count, T>
		&& is_valid_v<container_reserve, T>

		// - modifier

		&& is_valid_v<container_clear, T>
		&& is_valid_v<container_insert_0, T>
		&& is_valid_v<container_insert_1, T>
		&& is_valid_v<container_insert_2, T>
		&& is_valid_v<container_insert_3, T>
		&& is_valid_v<container_insert_4, T>
		&& is_valid_v<container_erase_0, T>
		&& is_valid_v<container_erase_1, T>
		&& is_valid_v<container_erase_3, T>
		&& is_valid_v<container_erase_4, T>
		&& is_valid_v<container_swap, T>
		&& is_valid_v<container_merge_0, T>
		&& is_valid_v<container_merge_1, T>
		&& is_valid_v<container_extract_0, T>
		&& is_valid_v<container_extract_1, T>

		// - lookup

		&& is_valid_v<container_count, T>
		&& is_valid_v<container_find, T>
		&& is_valid_v<container_find_const, T>
		&& is_valid_v<container_equal_range, T>
		&& is_valid_v<container_equal_range_const, T>

		// - observers

		&& is_valid_v<container_hash_function, T>
		&& is_valid_v<container_key_eq, T>

		// - allocator

		&& is_valid_v<container_get_allocator, T>

		>> : std::true_type{};

	template<typename T, typename = void>
	struct IsUnorderedMap : std::false_type {};
	template<typename T>
	struct IsUnorderedMap < T, std::enable_if_t <
		IsUnorderedSet<T>::value

		// - element access

		&& is_valid_v<container_at_key, T>
		&& is_valid_v<container_at_key_const, T>
		&& is_valid_v<container_subscript_key_0, T>
		&& is_valid_v<container_subscript_key_1, T>

		// - modifier

		&& is_valid_v<container_insert_or_assign_0, T>
		&& is_valid_v<container_insert_or_assign_1, T>
		&& is_valid_v<container_insert_or_assign_2, T>
		&& is_valid_v<container_insert_or_assign_3, T>
		&& is_valid_v<container_insert_or_assign_4, T>
		&& is_valid_v<container_insert_or_assign_5, T>
		&& is_valid_v<container_insert_or_assign_6, T>
		&& is_valid_v<container_insert_or_assign_7, T>

		>> : std::true_type{};
}

template<typename T>
struct Ubpa::UDRefl::IsArray : Ubpa::UDRefl::details::IsArray<T> {};
template<typename T>
struct Ubpa::UDRefl::IsVector : Ubpa::UDRefl::details::IsVector<T> {};
template<typename T>
struct Ubpa::UDRefl::IsDeque : Ubpa::UDRefl::details::IsDeque<T> {};
template<typename T>
struct Ubpa::UDRefl::IsSet : Ubpa::UDRefl::details::IsSet<T> {};
template<typename T>
struct Ubpa::UDRefl::IsMap : Ubpa::UDRefl::details::IsMap<T> {};
template<typename T>
struct Ubpa::UDRefl::IsUnorderedSet : Ubpa::UDRefl::details::IsUnorderedSet<T> {};
template<typename T>
struct Ubpa::UDRefl::IsUnorderedMap : Ubpa::UDRefl::details::IsUnorderedMap<T> {};

template<typename T>
struct Ubpa::UDRefl::IsContainer : std::bool_constant<
	Ubpa::UDRefl::IsArray_v<T>
	|| Ubpa::UDRefl::IsVector_v<T>
	|| Ubpa::UDRefl::IsDeque_v<T>
	|| Ubpa::UDRefl::IsSet_v<T>
	|| Ubpa::UDRefl::IsMap_v<T>
	|| Ubpa::UDRefl::IsUnorderedSet_v<T>
	|| Ubpa::UDRefl::IsUnorderedMap_v<T>
> {};
