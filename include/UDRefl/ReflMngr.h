#pragma once

#include "NameRegistry.h"
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

		NameRegistry registry;
		std::unordered_map<size_t, TypeInfo> typeinfos;
		std::unordered_map<size_t, EnumInfo> enuminfos;

		//
		// Cast
		/////////

		ObjectPtr StaticCast_DerivedToBase (ObjectPtr obj, size_t typeID) const noexcept;
		ObjectPtr StaticCast_BaseToDerived (ObjectPtr obj, size_t typeID) const noexcept;
		ObjectPtr DynamicCast_BaseToDerived(ObjectPtr obj, size_t typeID) const noexcept;
		ObjectPtr StaticCast               (ObjectPtr obj, size_t typeID) const noexcept;
		ObjectPtr DynamicCast              (ObjectPtr obj, size_t typeID) const noexcept;
		
		ConstObjectPtr StaticCast_DerivedToBase (ConstObjectPtr obj, size_t typeID) const noexcept
		{ return StaticCast_DerivedToBase (reinterpret_cast<ObjectPtr&>(obj), typeID); }

		ConstObjectPtr StaticCast_BaseToDerived (ConstObjectPtr obj, size_t typeID) const noexcept
		{ return StaticCast_BaseToDerived (reinterpret_cast<ObjectPtr&>(obj), typeID); }

		ConstObjectPtr DynamicCast_BaseToDerived(ConstObjectPtr obj, size_t typeID) const noexcept
		{ return DynamicCast_BaseToDerived(reinterpret_cast<ObjectPtr&>(obj), typeID); }

		ConstObjectPtr StaticCast               (ConstObjectPtr obj, size_t typeID) const noexcept
		{ return StaticCast               (reinterpret_cast<ObjectPtr&>(obj), typeID); }

		ConstObjectPtr DynamicCast              (ConstObjectPtr obj, size_t typeID) const noexcept
		{ return DynamicCast              (reinterpret_cast<ObjectPtr&>(obj), typeID); }

		//
		// Field
		//////////

		// read/write field, non-const
		ObjectPtr RWField(ObjectPtr obj, size_t fieldID) const noexcept;
		// read field, non-const + const
		ConstObjectPtr RField(ConstObjectPtr obj, size_t fieldID) const noexcept;

		//
		// Invoke
		///////////

		bool IsStaticInvocable(size_t typeID, size_t methodID, Span<size_t> argTypeIDs) const noexcept;
		bool IsConstInvocable(size_t typeID, size_t methodID, Span<size_t> argTypeIDs) const noexcept;
		bool IsInvocable(size_t typeID, size_t methodID, Span<size_t> argTypeIDs) const noexcept;

		InvokeResult Invoke(size_t typeID, size_t methodID, Span<size_t> argTypeIDs, void* args_buffer, void* result_buffer) const;
		InvokeResult Invoke(ConstObjectPtr obj, size_t methodID, Span<size_t> argTypeIDs, void* args_buffer, void* result_buffer) const;
		InvokeResult Invoke(ObjectPtr obj, size_t methodID, Span<size_t> argTypeIDs, void* args_buffer, void* result_buffer) const;

		//
		// Algorithm
		//////////////

		// self typeinfo and all bases' typeinfo
		// [args]
		// 0: type ID
		void ForEachTypeID(
			size_t typeID,
			const std::function<void(size_t)>& func) const;

		// self typeinfo and all bases' typeinfo
		// [args]
		// 0: type ID
		// 1: TypeInfo
		void ForEachTypeInfo(
			size_t typeID,
			const std::function<void(size_t, const TypeInfo&)>& func) const;

		// self fieldinfos and all bases' fieldinfos
		// [args]
		// 0: type ID
		// 1: TypeInfo
		// 2: field ID
		// 3: FieldInfo
		void ForEachFieldInfo(
			size_t typeID,
			const std::function<void(size_t, const TypeInfo&, size_t, const FieldInfo&)>& func) const;

		// self [r/w] fields and all bases' [r/w] fields
		// [args]
		// 0: type ID
		// 1: TypeInfo
		// 2: field ID
		// 3: FieldInfo
		// 4: field
		void ForEachRWField(
			ObjectPtr obj,
			const std::function<void(size_t, const TypeInfo&, size_t, const FieldInfo&, ObjectPtr)>& func) const;

		// self [r] fields and all bases' [r] fields
		// [args]
		// 0: type ID
		// 1: TypeInfo
		// 2: field ID
		// 3: FieldInfo
		// 4: field
		void ForEachRField(
			ConstObjectPtr obj,
			const std::function<void(size_t, const TypeInfo&, size_t, const FieldInfo&, ConstObjectPtr)>& func) const;

	private:
		ReflMngr() = default;
	};
}
