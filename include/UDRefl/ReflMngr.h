#pragma once

#include "Registry.h"
#include "TypeInfo.h"
#include "EnumInfo.h"

#include <functional>

namespace Ubpa::UDRefl {
	class ReflMngr {
	public:
		static ReflMngr& Instance() noexcept {
			static ReflMngr instance;
			return instance;
		}

		//
		// Data
		/////////

		NameRegistry nregistry;
		TypeRegistry tregistry;
		std::unordered_map<TypeID, TypeInfo> typeinfos;
		std::unordered_map<TypeID, EnumInfo> enuminfos;

		//
		// Cast
		/////////

		ObjectPtr StaticCast_DerivedToBase (ObjectPtr obj, TypeID typeID) const noexcept;
		ObjectPtr StaticCast_BaseToDerived (ObjectPtr obj, TypeID typeID) const noexcept;
		ObjectPtr DynamicCast_BaseToDerived(ObjectPtr obj, TypeID typeID) const noexcept;
		ObjectPtr StaticCast               (ObjectPtr obj, TypeID typeID) const noexcept;
		ObjectPtr DynamicCast              (ObjectPtr obj, TypeID typeID) const noexcept;
		
		ConstObjectPtr StaticCast_DerivedToBase (ConstObjectPtr obj, TypeID typeID) const noexcept
		{ return StaticCast_DerivedToBase (reinterpret_cast<ObjectPtr&>(obj), typeID); }

		ConstObjectPtr StaticCast_BaseToDerived (ConstObjectPtr obj, TypeID typeID) const noexcept
		{ return StaticCast_BaseToDerived (reinterpret_cast<ObjectPtr&>(obj), typeID); }

		ConstObjectPtr DynamicCast_BaseToDerived(ConstObjectPtr obj, TypeID typeID) const noexcept
		{ return DynamicCast_BaseToDerived(reinterpret_cast<ObjectPtr&>(obj), typeID); }

		ConstObjectPtr StaticCast               (ConstObjectPtr obj, TypeID typeID) const noexcept
		{ return StaticCast               (reinterpret_cast<ObjectPtr&>(obj), typeID); }

		ConstObjectPtr DynamicCast              (ConstObjectPtr obj, TypeID typeID) const noexcept
		{ return DynamicCast              (reinterpret_cast<ObjectPtr&>(obj), typeID); }

		//
		// Field
		//////////

		// read/write field, non-const
		ObjectPtr RWField(ObjectPtr obj, NameID fieldID) const noexcept;
		// read field, non-const + const
		ConstObjectPtr RField(ConstObjectPtr obj, NameID fieldID) const noexcept;

		//
		// Invoke
		///////////

		bool IsStaticInvocable(TypeID typeID, NameID methodID, Span<TypeID> argTypeIDs) const noexcept;
		bool IsConstInvocable(TypeID typeID, NameID methodID, Span<TypeID> argTypeIDs) const noexcept;
		bool IsInvocable(TypeID typeID, NameID methodID, Span<TypeID> argTypeIDs) const noexcept;

		InvokeResult Invoke(TypeID typeID, NameID methodID, Span<TypeID> argTypeIDs, void* args_buffer, void* result_buffer) const;
		InvokeResult Invoke(ConstObjectPtr obj, NameID methodID, Span<TypeID> argTypeIDs, void* args_buffer, void* result_buffer) const;
		InvokeResult Invoke(ObjectPtr obj, NameID methodID, Span<TypeID> argTypeIDs, void* args_buffer, void* result_buffer) const;

		//
		// Algorithm
		//////////////

		// self typeinfo and all bases' typeinfo
		// [args]
		// 0: type ID
		void ForEachTypeID(
			TypeID typeID,
			const std::function<void(TypeID)>& func) const;

		// self typeinfo and all bases' typeinfo
		// [args]
		// 0: type ID
		// 1: TypeInfo
		void ForEachTypeInfo(
			TypeID typeID,
			const std::function<void(TypeID, const TypeInfo&)>& func) const;

		// self fieldinfos and all bases' fieldinfos
		// [args]
		// 0: type ID
		// 1: TypeInfo
		// 2: field ID
		// 3: FieldInfo
		void ForEachFieldInfo(
			TypeID typeID,
			const std::function<void(TypeID, const TypeInfo&, NameID, const FieldInfo&)>& func) const;

		// self [r/w] fields and all bases' [r/w] fields
		// [args]
		// 0: type ID
		// 1: TypeInfo
		// 2: field ID
		// 3: FieldInfo
		// 4: field
		void ForEachRWField(
			ObjectPtr obj,
			const std::function<void(TypeID, const TypeInfo&, NameID, const FieldInfo&, ObjectPtr)>& func) const;

		// self [r] fields and all bases' [r] fields
		// [args]
		// 0: type ID
		// 1: TypeInfo
		// 2: field ID
		// 3: FieldInfo
		// 4: field
		void ForEachRField(
			ConstObjectPtr obj,
			const std::function<void(TypeID, const TypeInfo&, NameID, const FieldInfo&, ConstObjectPtr)>& func) const;

	private:
		ReflMngr() = default;
	};
}
