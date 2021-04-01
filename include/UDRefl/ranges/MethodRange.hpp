#pragma once

#include "ObjectTree.hpp"

namespace Ubpa::UDRefl {
	// DFS
	class UDRefl_core_API MethodRange {
	public:
		class UDRefl_core_API iterator {
		public:
			using value_type = std::pair<const Name, MethodInfo>;
			using reference = value_type&;
			using pointer = value_type*;
			using iterator_category = std::forward_iterator_tag;

			iterator(ObjectTree::iterator typeiter, MethodFlag flag = MethodFlag::All);

			iterator& operator++();
			iterator operator++(int);

			reference operator*() const noexcept { return *curmethod; }
			pointer operator->() const noexcept { return curmethod.operator->(); }

			UDRefl_core_API friend bool operator==(const iterator& lhs, const iterator& rhs);
			UDRefl_core_API friend bool operator!=(const iterator& lhs, const iterator& rhs);

			bool Valid() const noexcept { return typeiter.Valid(); }
			std::span<const Ranges::Derived> GetDeriveds() const noexcept { return typeiter.GetDeriveds(); }
			ObjectView GetObjectView() const { return std::get<ObjectView>(*typeiter); }
			TypeInfo* GetTypeInfo() const { return std::get<TypeInfo*>(*typeiter); }
		private:
			void update();
			ObjectTree::iterator typeiter;
			MethodFlag flag;
			int mode;
			std::unordered_map<Name, MethodInfo>::iterator curmethod;
		};

		constexpr MethodRange(ObjectView obj, MethodFlag flag) noexcept :
			objtree{ ObjectTree{obj} }, flag{ flag } {}
		constexpr explicit MethodRange(ObjectView obj) noexcept : MethodRange{ obj, MethodFlag::All } {}
		constexpr explicit MethodRange(Type type) noexcept : MethodRange{ ObjectView{type}, MethodFlag::All } {}
		constexpr MethodRange(Type type, MethodFlag flag) noexcept : MethodRange{ ObjectView{type}, flag } {}
		
		iterator begin() const { return { objtree.begin(), flag }; }
		iterator end() const noexcept { return { objtree.end(), flag }; }

	private:
		ObjectTree objtree;
		MethodFlag flag;
	};

	template<typename T, MethodFlag flag = MethodFlag::All>
	static constexpr MethodRange MethodRange_of = MethodRange{ ObjectView_of<T>, flag };
}
