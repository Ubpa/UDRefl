#pragma once

#include "TypeTree.h"

namespace Ubpa::UDRefl {
	// DFS
	class MethodRange {
	public:
		class iterator {
		public:
			using value_type = std::pair<const Name, MethodInfo>;
			using reference = value_type&;
			using pointer = value_type*;
			using iterator_category = std::forward_iterator_tag;

			iterator(TypeTree::iterator typeiter, MethodFlag flag = MethodFlag::All);

			iterator& operator++();
			iterator operator++(int);

			reference operator*() const noexcept { return *curmethod; }
			pointer operator->() const noexcept { return curmethod.operator->(); }

			friend bool operator==(const iterator& lhs, const iterator& rhs);
			friend bool operator!=(const iterator& lhs, const iterator& rhs);

			bool Valid() const noexcept { return typeiter.Valid(); }
			std::span<const Ranges::Derived> GetDeriveds() const noexcept { return typeiter.GetDeriveds(); }
		private:
			void update();
			TypeTree::iterator typeiter;
			MethodFlag flag;
			int mode;
			std::unordered_map<Name, MethodInfo>::iterator curmethod;
		};

		constexpr MethodRange(Type root, MethodFlag flag) noexcept :
			root_tree{ TypeTree{root} }, flag{ flag } {}
		constexpr explicit MethodRange(Type root) noexcept : MethodRange{ root, MethodFlag::All } {}
		iterator begin() const { return { root_tree.begin(), flag }; }
		iterator end() const noexcept { return { root_tree.end(), flag }; }

		Type GetType() const noexcept { return root_tree.GetType(); }

	private:
		TypeTree root_tree;
		MethodFlag flag;
	};

	template<typename T, MethodFlag flag = MethodFlag::All>
	static constexpr MethodRange MethodRange_of = MethodRange{ Type_of<T>, flag };
}
