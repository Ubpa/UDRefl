#pragma once

#include "common.h"

namespace Ubpa::UDRefl {
	class VarRange {
	public:
		// DFS
		// TypeInfo* and BaseInfo* maybe nullptr
		class iterator {
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

			iterator(Type root, void* ptr, bool begin_or_end,
				CVRefMode cvref_mode = CVRefMode::None,
				FieldFlag flag = FieldFlag::All);

			iterator& operator++();
			iterator operator++(int);

			reference operator*() const noexcept { return value; }
			pointer operator->() const noexcept { return &value; }

			friend bool operator==(const iterator& lhs, const iterator& rhs);
			friend bool operator!=(const iterator& lhs, const iterator& rhs);

			bool Valid() const noexcept { return mode != -1; }

			std::span<const Ranges::Derived> GetDeriveds() const noexcept
			{ return { deriveds.begin(), deriveds.end() }; }

			const Type& GetType() const { return type; }
			TypeInfo* GetTypeInfo() const { return typeinfo; }
			FieldInfo& GetFieldInfo() const { return curfield->second; }
		private:
			void update();

			Type root; // fix
			CVRefMode cvref_mode; // fix
			FieldFlag flag; // fix

			small_vector<Type, 8> visitedVBs;
			small_vector<Ranges::Derived, 16> deriveds;
			bool curbase_valid;
			Type type;
			TypeInfo* typeinfo;
			void* ptr;
			std::unordered_map<Name, FieldInfo>::iterator curfield;
			bool visited_curtype;
			int mode;

			value_type value;
		};

		constexpr VarRange(ObjectView obj, FieldFlag flag) noexcept :
			root{ obj.GetType().RemoveCVRef() },
			cvref_mode{ obj.GetType().GetCVRefMode() },
			ptr{ obj.GetPtr() },
			flag{ obj.GetPtr() ? flag : enum_within(flag, FieldFlag::Unowned) } {}

		constexpr explicit VarRange(ObjectView obj) noexcept : VarRange{obj, FieldFlag::All} {}

		iterator begin() const { return { root,ptr,true,cvref_mode,flag }; }
		iterator end() const noexcept { return { root,ptr,false,cvref_mode,flag }; }

		Type GetType() const noexcept { return root; }
		FieldFlag GetFieldFlag() const noexcept { return flag; }
	private:
		Type root;
		CVRefMode cvref_mode;
		void* ptr;
		FieldFlag flag;
	};

	template<typename T, FieldFlag flag = FieldFlag::All>
	static constexpr VarRange VarRange_of = VarRange{ ObjectView_of<T>, flag };
}
