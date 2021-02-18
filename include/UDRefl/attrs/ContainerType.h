#pragma once

#include "../Util.h"

#include <set>

namespace Ubpa::UDRefl {
	// TODO: list, forward_list, stack, queue

	// RawArray -> Array -> Vector
	// Tuple -> Array
	// Tuple -> Pair
	// MultiSet -> Set
	// MultiSet -> MultiMap -> Map
	// UnorderedMultiSet -> UnorderedSet
	// UnorderedMultiSet -> UnorderedMultiMap -> UnorderedMap
	// Stack -> PriorityQueue
	enum class ContainerType {
		None,
		RawArray,
		Tuple,
		Array,
		Vector,
		Deque,
		ForwardList,
		List,
		Set,
		MultiSet,
		Map,
		MultiMap,
		UnorderedSet,
		UnorderedMultiSet,
		UnorderedMap,
		UnorderedMultiMap,
		Stack,
		PriorityQueue,
		Queue,
		Pair,
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
