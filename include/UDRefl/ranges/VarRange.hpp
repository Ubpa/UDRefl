#pragma once

#include "common.hpp"

#include "ObjectTree.hpp"

namespace Ubpa::UDRefl {
	class UDRefl_core_API VarRange {
	public:
		// DFS
		// TypeInfo* and BaseInfo* maybe nullptr
		class UDRefl_core_API iterator {
		public:
			using value_type = std::tuple<Name, ObjectView>;
			using reference = const value_type&;
			using pointer = const value_type*;
			using iterator_category = std::forward_iterator_tag;

			struct Derived {
				Type type;
				TypeInfo* typeinfo; // not nullptr
				std::unordered_map<Type, BaseInfo>::iterator curbase;
			};

			iterator(ObjectTree::iterator typeiter,
				CVRefMode cvref_mode = CVRefMode::None,
				FieldFlag flag = FieldFlag::All);

			iterator& operator++();
			iterator operator++(int);

			reference operator*() const noexcept { return value; }
			pointer operator->() const noexcept { return &value; }

			UDRefl_core_API friend bool operator==(const iterator& lhs, const iterator& rhs);
			UDRefl_core_API friend bool operator!=(const iterator& lhs, const iterator& rhs);

			bool Valid() const noexcept { return mode != -1; }

			std::span<const Ranges::Derived> GetDeriveds() const noexcept { return typeiter.GetDeriveds(); }

			ObjectView GetObjectView() const { return std::get<ObjectView>(*typeiter); }
			TypeInfo* GetTypeInfo() const { return std::get<TypeInfo*>(*typeiter); }
			FieldInfo& GetFieldInfo() const { return curfield->second; }
		private:
			void update();

			CVRefMode cvref_mode; // fix
			FieldFlag flag; // fix

			ObjectTree::iterator typeiter;
			std::unordered_map<Name, FieldInfo>::iterator curfield;
			int mode;

			value_type value;
		};

		constexpr VarRange(ObjectView obj, FieldFlag flag) noexcept :
			objtree{ ObjectTree{obj} },
			flag{ obj.GetPtr()?flag:enum_within(flag, FieldFlag::Unowned) },
			cvref_mode{obj.GetType().GetCVRefMode()}
		{ assert(!CVRefMode_IsVolatile(cvref_mode)); }

		constexpr explicit VarRange(ObjectView obj) noexcept : VarRange{ obj, FieldFlag::All } {}
		constexpr explicit VarRange(Type type) noexcept : VarRange{ ObjectView{type}, FieldFlag::Unowned } {}
		constexpr VarRange(Type type, FieldFlag flag) noexcept : VarRange{ ObjectView{type}, flag } {}

		iterator begin() const { return { objtree.begin(), cvref_mode, flag }; }
		iterator end() const noexcept { return { objtree.end(), cvref_mode, flag }; }

	private:
		ObjectTree objtree;
		FieldFlag flag;
		CVRefMode cvref_mode;
	};

	template<typename T, FieldFlag flag = FieldFlag::Unowned>
	static constexpr VarRange VarRange_of = VarRange{ ObjectView_of<T>, flag };
}
