#pragma once

#include "common.h"

#include <UDRefl/Object.h>

namespace Ubpa::UDRefl {
	class ObjectTree {
	public:
		// DFS
		// TypeInfo* and BaseInfo* maybe nullptr
		class iterator {
		public:
			using value_type = std::tuple<TypeInfo*, ObjectView>;
			using reference = const value_type&;
			using pointer = const value_type*;
			using iterator_category = std::forward_iterator_tag;

			// true: begin
			// false: end
			iterator(ObjectView obj, bool begin_or_end);

			iterator& operator++();
			iterator operator++(int);

			reference operator*() const noexcept { return value; }
			pointer operator->() const noexcept { return &value; }

			friend bool operator==(const iterator& lhs, const iterator& rhs);
			friend bool operator!=(const iterator& lhs, const iterator& rhs);

			bool Valid() const noexcept { return mode != -1; }
			std::span<const Ranges::Derived> GetDeriveds() const noexcept { return { deriveds.begin(), deriveds.end() }; }

		private:
			friend ObjectTree;

			void update();

			small_vector<Type, 4> visitedVBs;
			small_vector<Ranges::Derived, 8> deriveds;
			bool curbase_valid;
			int mode;

			value_type value;
		};

		constexpr ObjectTree(ObjectView obj) noexcept :
			obj{ obj.RemoveConstReference() } {}

		iterator begin() const { return { obj, true }; }
		iterator end() const noexcept { return { obj, false }; }

	private:
		ObjectView obj;
	};

	template<typename T>
	static constexpr ObjectTree ObjectTree_of = ObjectTree{ ObjectView_of<T> };
}
