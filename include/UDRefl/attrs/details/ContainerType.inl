#pragma once

#include "../../Util.h"

namespace Ubpa::UDRefl::details {
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
}

template<typename T>
struct Ubpa::UDRefl::IsVector : Ubpa::UDRefl::details::IsVector<T> {};
