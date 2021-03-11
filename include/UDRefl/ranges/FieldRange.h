#pragma once

#include "TypeTree.h"

namespace Ubpa::UDRefl {
	// DFS
	class FieldRange {
	public:
		class iterator {
		public:
			using value_type = std::pair<const Name, FieldInfo>;
			using reference = value_type&;
			using pointer = value_type*;
			using iterator_category = std::forward_iterator_tag;

			iterator(TypeTree::iterator typeiter, FieldFlag flag = FieldFlag::All);

			iterator& operator++();
			iterator operator++(int);

			reference operator*() const noexcept { return *curfield; }
			pointer operator->() const noexcept { return curfield.operator->(); }

			friend bool operator==(const iterator& lhs, const iterator& rhs);
			friend bool operator!=(const iterator& lhs, const iterator& rhs);

			bool Valid() const noexcept { return typeiter.Valid(); }
			std::span<const Ranges::Derived> GetDeriveds() const noexcept { return typeiter.GetDeriveds(); }
		private:
			void update();
			TypeTree::iterator typeiter;
			FieldFlag flag;
			int mode;
			std::unordered_map<Name, FieldInfo>::iterator curfield;
		};

		constexpr FieldRange(Type root, FieldFlag flag) noexcept :
			root_tree{ TypeTree{root} }, flag{ flag } {}
		constexpr explicit FieldRange(Type root) noexcept : FieldRange{ root, FieldFlag::All } {}
		iterator begin() const { return { root_tree.begin(), flag }; }
		iterator end() const noexcept { return { root_tree.end(), flag }; }

		Type GetType() const noexcept { return root_tree.GetType(); }

	private:
		TypeTree root_tree;
		FieldFlag flag;
	};

	template<typename T, FieldFlag flag = FieldFlag::All>
	static constexpr FieldRange FieldRange_of = FieldRange{ Type_of<T>, flag };
}
