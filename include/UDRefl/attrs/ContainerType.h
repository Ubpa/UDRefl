#pragma once

namespace Ubpa::UDRefl {
	// TODO: tuple, pair
	enum class ContainerType {
		Unknown,
		Array,
		Vector,
		Deque,
		ForwardList,
		List,
		Set,
		Map,
		UnorderedSet,
		UnorderedMap,
		Stack,
		Queue
	};

	// traits

	template<typename T> struct IsContainer;
	template<typename T> constexpr bool IsContainer_v = IsContainer<T>::value;

	template<typename T> struct IsArray;
	template<typename T> constexpr bool IsArray_v = IsArray<T>::value;
	template<typename T> struct IsVector;
	template<typename T> constexpr bool IsVector_v = IsVector<T>::value;
	template<typename T> struct IsDeque;
	template<typename T> constexpr bool IsDeque_v = IsDeque<T>::value;
	template<typename T> struct IsForwardList;
	template<typename T> constexpr bool IsForwardList_v = IsForwardList<T>::value;
	template<typename T> struct IsList; // TODO
	template<typename T> constexpr bool IsList_v = IsList<T>::value;
	template<typename T> struct IsSet;
	template<typename T> constexpr bool IsSet_v = IsSet<T>::value;
	template<typename T> struct IsMap;
	template<typename T> constexpr bool IsMap_v = IsMap<T>::value;
	template<typename T> struct IsUnorderedSet;
	template<typename T> constexpr bool IsUnorderedSet_v = IsUnorderedSet<T>::value;
	template<typename T> struct IsUnorderedMap;
	template<typename T> constexpr bool IsUnorderedMap_v = IsUnorderedMap<T>::value;
	template<typename T> struct IsStack; // TODO
	template<typename T> constexpr bool IsStack_v = IsStack<T>::value;
	template<typename T> struct IsQueue; // TODO
	template<typename T> constexpr bool IsQueue_v = IsQueue<T>::value;
}

#include "details/ContainerType.inl"
