#pragma once

#include "../Util.hpp"

#include <set>

namespace Ubpa::UDRefl {
	// TODO: list, forward_list, stack, queue

	// RawArray -> Array -> Vector
	// Stack -> PriorityQueue
	// Tuple -> Array
	// Tuple -> Pair
	// MultiSet -> Set
	// MultiSet -> MultiMap -> Map
	// UnorderedMultiSet -> UnorderedSet
	// UnorderedMultiSet -> UnorderedMultiMap -> UnorderedMap
	enum class ContainerType {
		None,
		Array,
		Deque,
		ForwardList,
		List,
		Map,
		MultiMap,
		MultiSet,
		Optional,
		Pair,
		PriorityQueue,
		Queue,
		RawArray,
		Set,
		Span,
		Stack,
		Tuple,
		UnorderedMap,
		UnorderedMultiSet,
		UnorderedMultiMap,
		UnorderedSet,
		Variant,
		Vector
	};
		
	// SpecializeIsSet<std::set<...>> is std::true_type
	template<typename T> struct SpecializeIsSet : std::false_type{};
	// traits
	// ref : https://zh.cppreference.com/w/cpp/container
	/*
	template<typename T>
	concept Is* = ...;
	*/
}

#include "details/ContainerType.inl"
