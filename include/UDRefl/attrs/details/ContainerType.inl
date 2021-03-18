#pragma once

#include "../../Util.hpp"

#include <set>

template<typename Key, typename Compare, typename Allocator>
struct Ubpa::UDRefl::SpecializeIsSet<std::set<Key, Compare, Allocator>> : std::true_type {};

namespace Ubpa::UDRefl {
	template<typename T>
	concept IsRawArray = true
		&& container_begin<T&>
		&& container_begin<const T&>
		&& container_cbegin<T&>

		&& container_end<T&>
		&& container_end<const T&>
		&& container_cend<T&>

		&& container_rbegin<T&>
		&& container_rbegin<const T&>
		&& container_crbegin<T&>

		&& container_rend<T&>
		&& container_rend<const T&>
		&& container_crend<T&>

		&& container_subscript_size<T&>
		&& container_subscript_size<const T&>

		&& container_data<T&>
		&& container_data<const T&>

		&& container_empty<T>
		&& container_size<T>

		&& container_swap<T&>
		;

	template<typename T>
	concept IsTuple = true
		&& tuple_size<T>
		;

	template<typename T>
	concept IsArray = IsRawArray<T>
		&& container_at_size<T>
		&& container_at_size<const T>

		&& container_front<T>
		&& container_front<const T>

		&& container_back<T>
		&& container_back<const T>
		;

	template<typename T>
	concept IsVector = IsArray<T>
		&& container_assign<T>

		&& container_resize_cnt<T>
		&& container_resize_cnt_value<T>
		&& container_capacity<T>
		&& container_reserve<T>
		&& container_shrink_to_fit<T>

		&& container_clear<T>
		&& container_insert_citer_clvalue<T>
		&& container_insert_citer_rvalue<T>
		&& container_insert_citer_cnt<T>
		&& container_erase_citer<T>
		&& container_erase_range_citer<T>
		&& container_push_back_clvalue<T>
		&& container_push_back_rvalue<T>
		&& container_pop_back<T>
		;

	template<typename T>
	concept IsDeque = true
		&& container_assign<T>

		&& container_begin<T>
		&& container_begin<const T>
		&& container_cbegin<T>

		&& container_end<T>
		&& container_end<const T>
		&& container_cend<T>

		&& container_rbegin<T>
		&& container_rbegin<const T>
		&& container_crbegin<T>

		&& container_rend<T>
		&& container_rend<const T>
		&& container_crend<T>

		&& container_at_size<T>
		&& container_at_size<const T>

		&& container_subscript_size<T>
		&& container_subscript_size<const T>

		&& container_front<T>
		&& container_front<const T>

		&& container_back<T>
		&& container_back<const T>

		&& container_empty<T>
		&& container_size<T>

		&& container_resize_cnt<T>
		&& container_resize_cnt_value<T>
		&& container_shrink_to_fit<T>
		&& container_clear<T>
		&& container_insert_citer_clvalue<T>
		&& container_insert_citer_rvalue<T>
		&& container_insert_citer_cnt<T>
		&& container_erase_citer<T>
		&& container_erase_range_citer<T>
		&& container_push_front_clvalue<T>
		&& container_push_front_rvalue<T>
		&& container_pop_front<T>
		&& container_push_back_clvalue<T>
		&& container_push_back_rvalue<T>
		&& container_pop_back<T>

		&& container_swap<T>
		;

	template<typename T>
	concept IsForwardList = true
		&& container_assign<T>

		&& container_begin<T>
		&& container_begin<const T>
		&& container_cbegin<T>

		&& container_end<T>
		&& container_end<const T>
		&& container_cend<T>

		&& container_front<T>
		&& container_front<const T>
		&& container_empty<T>
		&& container_resize_cnt<T>
		&& container_resize_cnt_value<T>

		&& container_clear<T>
		&& container_insert_after_clvalue<T>
		&& container_insert_after_rvalue<T>
		&& container_insert_after_cnt<T>
		&& container_erase_after<T>
		&& container_erase_after_range<T>
		&& container_push_front_clvalue<T>
		&& container_push_front_rvalue<T>
		&& container_pop_front<T>
		&& container_swap<T>
		&& container_merge_l<T>
		&& container_merge_r<T>

		&& container_splice_after_l<T>
		&& container_splice_after_r<T>
		&& container_splice_after_it_l<T>
		&& container_splice_after_it_r<T>
		&& container_splice_after_range_l<T>
		&& container_splice_after_range_r<T>
		&& container_remove<T>
		&& container_reverse<T>
		&& container_unique<T>
		&& container_sort<T>
		;

	template<typename T>
	concept IsList = true
		&& container_assign<T>

		&& container_begin<T>
		&& container_begin<const T>
		&& container_cbegin<T>

		&& container_end<T>
		&& container_end<const T>
		&& container_cend<T>

		&& container_rbegin<T>
		&& container_rbegin<const T>
		&& container_crbegin<T>

		&& container_rend<T>
		&& container_rend<const T>
		&& container_crend<T>

		&& container_front<T>
		&& container_front<const T>
		&& container_back<T>
		&& container_back<const T>
		&& container_empty<T>
		&& container_size<T>
		&& container_resize_cnt<T>
		&& container_resize_cnt_value<T>

		&& container_clear<T>
		&& container_insert_citer_clvalue<T>
		&& container_insert_citer_rvalue<T>
		&& container_insert_citer_cnt<T>
		&& container_erase_citer<T>
		&& container_erase_range_citer<T>
		&& container_push_front_clvalue<T>
		&& container_push_front_rvalue<T>
		&& container_pop_front<T>
		&& container_push_back_clvalue<T>
		&& container_push_back_rvalue<T>
		&& container_pop_back<T>
		&& container_swap<T>
		&& container_merge_l<T>
		&& container_merge_r<T>

		&& container_splice_l<T>
		&& container_splice_r<T>
		&& container_splice_it_l<T>
		&& container_splice_it_r<T>
		&& container_splice_range_l<T>
		&& container_splice_range_r<T>
		&& container_remove<T>
		&& container_reverse<T>
		&& container_unique<T>
		&& container_sort<T>
		;

	template<typename T>
	concept IsMultiSet = true
		&& container_begin<T>
		&& container_begin<const T>
		&& container_cbegin<T>

		&& container_end<T>
		&& container_end<const T>
		&& container_cend<T>

		&& container_rbegin<T>
		&& container_rbegin<const T>
		&& container_crbegin<T>

		&& container_rend<T>
		&& container_rend<const T>
		&& container_crend<T>

		&& container_empty<T>
		&& container_size<T>

		&& container_clear<T>
		&& container_insert_clvalue<T>
		&& container_insert_rvalue<T>
		&& container_insert_rnode<T>
		&& container_insert_citer_clvalue<T>
		&& container_insert_citer_rvalue<T>
		&& container_insert_citer_rnode<T>

		&& container_erase_citer<T>
		&& container_erase_key<T>
		&& container_erase_range_citer<T>

		&& container_swap<T>
		&& container_merge_l<T>
		&& container_merge_r<T>
		&& container_extract_citer<T>
		&& container_extract_key<T>

		&& container_count<T>
		&& container_find<T>
		&& container_find<const T>
		&& container_contains<T>
		&& container_lower_bound<T>
		&& container_lower_bound<const T>
		&& container_upper_bound<T>
		&& container_upper_bound<const T>
		&& container_equal_range<T>
		&& container_equal_range<const T>
		;

	template<typename T>
	concept IsSet = IsMultiSet<T>
		&& SpecializeIsSet<T>::value
		;

	template<typename T>
	concept IsMultiMap = IsMultiSet<T>
		&& container_mapped_type<T>
		;

	template<typename T>
	concept IsMap = IsMultiMap<T>
		&& container_at_key<T>
		&& container_at_key<const T>
		//&& container_subscript_key_cl<T>
		//&& container_subscript_key_r<T>
		;
	
	template<typename T>
	concept IsUnorderedMultiSet = true
		&& container_begin<T>
		&& container_begin<const T>
		&& container_cbegin<T>

		&& container_end<T>
		&& container_end<const T>
		&& container_cend<T>

		&& container_empty<T>
		&& container_size<T>
		&& container_bucket_count<T>
		&& container_reserve<T>

		&& container_clear<T>
		&& container_insert_clvalue<T>
		&& container_insert_rvalue<T>
		&& container_insert_rnode<T>
		&& container_insert_citer_clvalue<T>
		&& container_insert_citer_rvalue<T>
		&& container_insert_citer_rnode<T>

		&& container_erase_citer<T>
		&& container_erase_key<T>
		&& container_erase_range_citer<T>

		&& container_swap<T>
		&& container_merge_l<T>
		&& container_merge_r<T>
		&& container_extract_citer<T>
		&& container_extract_key<T>

		&& container_count<T>
		&& container_find<T>
		&& container_find<const T>
		&& container_contains<T>
		&& container_equal_range<T>
		&& container_equal_range<const T>
		;

	template<typename T>
	concept IsUnorderedSet = IsUnorderedMultiSet<T>;

	template<typename T>
	concept IsUnorderedMultiMap = IsUnorderedMultiSet<T>
		&& container_mapped_type<T>
		;

	template<typename T>
	concept IsUnorderedMap = IsUnorderedMultiMap<T>
		&& container_at_key<T>
		&& container_at_key<const T>
		//&& container_subscript_key_cl<T>
		//&& container_subscript_key_r<T>
		;

	template<typename T>
	concept IsPair = IsTuple<T>
		&& pair_first<T>
		&& pair_second<T>
		;

	template<typename T>
	concept IsStack = true
		&& container_top<T>
		&& container_top<const T>
		&& container_empty<T>
		&& container_size<T>

		&& container_push_clvalue<T>
		&& container_push_rvalue<T>
		&& container_pop<T>
		&& container_swap<T>
		;

	template<typename T>
	concept IsPriorityQueue = IsStack<T>
		&& requires{ typename T::value_compare; }
		;

	template<typename T>
	concept IsQueue = true
		&& container_front<T>
		&& container_front<const T>
		&& container_back<T>
		&& container_back<const T>
		&& container_empty<T>
		&& container_size<T>

		&& container_pop<T>
		&& container_push_clvalue<T>
		&& container_push_rvalue<T>
		&& container_swap<T>
		;

	template<typename T>
	concept IsSpan = true
		&& container_begin<T>
		&& container_begin<const T>

		&& container_end<T>
		&& container_end<const T>

		&& container_rbegin<T>
		&& container_rbegin<const T>

		&& container_rend<T>
		&& container_rend<const T>

		&& container_front<T>
		&& container_front<const T>

		&& container_back<T>
		&& container_back<const T>

		&& container_subscript_size<T>
		&& container_subscript_size<const T>

		&& container_data<T>
		&& container_data<const T>

		&& container_size<T>
		&& container_size_bytes<T>
		&& container_empty<T>
		;

	template<typename T>
	concept IsVariant = true
		&& variant_size<T>
		&& variant_index<T>
		&& variant_valueless_by_exception<T>
		;

	template<typename T>
	concept IsOptional = true
		&& container_value_type<T>
		&& optional_has_value<T>
		&& optional_value<T>
		&& optional_value<const T>
		&& optional_reset<T>
		;

	template<typename T>
	concept IsContainerType = false
		|| IsRawArray<T>
		|| IsDeque<T>
		|| IsForwardList<T>
		|| IsList<T>
		|| IsMultiSet<T>
		|| IsUnorderedMultiSet<T>
		|| IsStack<T>
		|| IsQueue<T>
		|| IsTuple<T>
		|| IsSpan<T>
		|| IsVariant<T>
		|| IsOptional<T>
		;
}
