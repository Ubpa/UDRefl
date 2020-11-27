#pragma once

#include "NameRegistry.h"
#include "TypeInfo.h"
#include "EnumInfo.h"

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

		InvokeResult Invoke(size_t typeID, size_t methodID, Span<size_t> argTypeIDs, void* buffer) const;
		InvokeResult Invoke(ConstObjectPtr obj, size_t methodID, Span<size_t> argTypeIDs, void* buffer) const;
		InvokeResult Invoke(ObjectPtr obj, size_t methodID, Span<size_t> argTypeIDs, void* buffer) const;

	private:
		ReflMngr() = default;
	};
}
