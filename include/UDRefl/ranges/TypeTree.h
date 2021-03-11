#pragma once

#include "common.h"

namespace Ubpa::UDRefl {
	class TypeTree {
	public:
		// DFS
		// TypeInfo* and BaseInfo* maybe nullptr
		class iterator {
		public:
			using value_type = std::tuple<Type, TypeInfo*>;
			using reference = const value_type&;
			using pointer = const value_type*;
			using iterator_category = std::forward_iterator_tag;

			// true: begin
			// false: end
			iterator(Type root, bool begin_or_end);

			iterator& operator++();
			iterator operator++(int);

			reference operator*() const noexcept { return value; }
			pointer operator->() const noexcept { return &value; }

			friend bool operator==(const iterator& lhs, const iterator& rhs);
			friend bool operator!=(const iterator& lhs, const iterator& rhs);

			bool Valid() const noexcept { return mode != -1; }
			std::span<const Ranges::Derived> GetDeriveds() const noexcept { return { deriveds.begin(), deriveds.end() }; }

		private:
			friend TypeTree;

			void update();
			
			Type root; // fixed

			small_vector<Type, 8> visitedVBs;
			small_vector<Ranges::Derived, 16> deriveds;
			bool curbase_valid;
			int mode;

			value_type value;
		};

		constexpr explicit TypeTree(Type root) noexcept : root{ root.RemoveCVRef() } {}
		iterator begin() const { return { root, true }; }
		iterator end() const noexcept { return { root, false }; }

		Type GetType() const noexcept { return root; }
	private:
		Type root; // raw
	};

	template<typename T>
	static constexpr TypeTree TypeTree_of = TypeTree{ Type_of<T> };
}
