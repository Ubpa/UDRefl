#pragma once

#include "FieldInfo.h"
#include "MethodInfo.h"
#include "BaseInfo.h"

namespace Ubpa::UDRefl {
	struct TypeInfo {
		//
		// Data
		/////////

		size_t size;
		size_t alignment;
		std::unordered_map<StrID, FieldInfo> fieldinfos;
		std::unordered_multimap<StrID, MethodInfo> methodinfos;
		std::unordered_map<TypeID, BaseInfo> baseinfos;
		AttrSet attrs;

		//
		// Field
		//////////

		// without bases, variable object
		ObjectPtr      RWVar(StrID fieldID) noexcept;
		// without bases, object
		ConstObjectPtr RVar (StrID fieldID) const noexcept;
		// without bases, variable
		ObjectPtr      RWVar(void*       obj, StrID fieldID) noexcept;
		// without bases, all
		ConstObjectPtr RVar (const void* obj, StrID fieldID) const noexcept;

		//
		// Invoke
		///////////

		// without bases, static
		bool IsStaticInvocable(StrID methodID, Span<TypeID> argTypeIDs) const noexcept;
		// without bases, const + static
		bool IsConstInvocable (StrID methodID, Span<TypeID> argTypeIDs) const noexcept;
		// without bases, non-const + const + static
		bool IsInvocable      (StrID methodID, Span<TypeID> argTypeIDs) const noexcept;

		// without bases, static
		InvokeResult Invoke(StrID methodID, Span<TypeID> argTypeIDs, void* args_buffer, void* result_buffer) const;
		// without bases, const + static
		InvokeResult Invoke(const void* obj, StrID methodID, Span<TypeID> argTypeIDs, void* args_buffer, void* result_buffer) const;
		// without bases, non-const + const + static, non-const && static > const
		InvokeResult Invoke(void* obj, StrID methodID, Span<TypeID> argTypeIDs, void* args_buffer, void* result_buffer) const;
	};

	struct Type {
		TypeID ID;
		const TypeInfo& info;
	};
}
