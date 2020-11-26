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

		NameRegistry registry;
		std::unordered_map<size_t, TypeInfo> typeinfos;
		std::unordered_map<size_t, EnumInfo> enuminfos;

		bool IsCastable(size_t fromID, size_t toID) const noexcept;
		// if failed, return nullptr
		ObjectPtr Cast(ObjectPtr obj, size_t typeID) const noexcept;
		// if failed, return nullptr
		ConstObjectPtr Cast(ConstObjectPtr obj, size_t typeID) const noexcept { return Cast(reinterpret_cast<ObjectPtr&>(obj), typeID); }

		// read/write field, non-const
		ObjectPtr RWField(ObjectPtr obj, size_t fieldID) const noexcept;
		// read field, non-const + const
		ConstObjectPtr RField(ConstObjectPtr obj, size_t fieldID) const noexcept;

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
