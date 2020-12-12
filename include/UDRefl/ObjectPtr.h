#pragma once

#include "Util.h"

#include <UTemplate/TypeID.h>
#include <UContainer/Span.h>

#include <cassert>
#include <functional>

namespace Ubpa::UDRefl {
	struct InvokeResult {
		bool success{ false };
		TypeID resultID;
		Destructor destructor;

		template<typename T>
		T Move(void* result_buffer);
	};

	struct TypeInfo;
	struct FieldInfo;

	struct TypeRef {
		TypeID ID;
		TypeInfo& info;
	};

	struct FieldRef {
		StrID ID;
		FieldInfo& info;
	};

	class ConstObjectPtr;

	class ObjectPtrBase {
	public:
		constexpr ObjectPtrBase() noexcept : ptr{ nullptr } {}
		constexpr ObjectPtrBase(std::nullptr_t) noexcept : ObjectPtrBase{} {}
		constexpr ObjectPtrBase(TypeID ID, std::nullptr_t) noexcept : ID{ ID }, ptr{ nullptr } {}
		constexpr ObjectPtrBase(TypeID ID, void* ptr) noexcept : ID{ ID }, ptr{ ptr }{}
		explicit constexpr ObjectPtrBase(TypeID ID) noexcept : ObjectPtrBase{ ID, nullptr } {}

		constexpr TypeID GetID() const noexcept { return ID; }

		template<typename T>
		constexpr bool Is() const noexcept { return ID == TypeID::of<T>; }

		constexpr void Reset() noexcept { ptr = nullptr; }
		constexpr void Clear() noexcept { *this = ObjectPtrBase{}; }

		constexpr operator bool() const noexcept { return ptr != nullptr; }

		//
		// ReflMngr
		/////////////

		// all
		ConstObjectPtr RVar(StrID fieldID) const noexcept;

		// all, for diamond inheritance
		ConstObjectPtr RVar(TypeID baseID, StrID fieldID) const noexcept;

		// self [r] vars and all bases' [r] vars
		void ForEachRVar(const std::function<bool(TypeRef, FieldRef, ConstObjectPtr)>& func) const;

	protected:
		template<typename T>
		constexpr T* AsPtr() const noexcept {
			assert(Is<T>());
			return reinterpret_cast<T*>(ptr);
		}

		template<typename T>
		constexpr T& As() const noexcept { assert(ptr); return *AsPtr<T>(); }

	protected:
		void* ptr;
		TypeID ID;
	};

	class ConstObjectPtr : public ObjectPtrBase {
	public:
		using ObjectPtrBase::ObjectPtrBase;

		constexpr ConstObjectPtr(TypeID ID, const void* ptr) noexcept : ObjectPtrBase{ ID, const_cast<void*>(ptr) } {}

		constexpr const void* GetPtr() const noexcept { return ptr; }

		template<typename T>
		constexpr const T* AsPtr() const noexcept { return ObjectPtrBase::AsPtr<T>(); }
		template<typename T>
		constexpr const T& As() const noexcept { return ObjectPtrBase::As<T>(); }

		constexpr operator const void* () const noexcept { return ptr; }

		constexpr       ConstObjectPtr* operator->()       noexcept { return this; }
		constexpr const ConstObjectPtr* operator->() const noexcept { return this; }

		//
		// ReflMngr
		/////////////
		
		ConstObjectPtr StaticCast_DerivedToBase (TypeID baseID   ) const noexcept;
		ConstObjectPtr StaticCast_BaseToDerived (TypeID derivedID) const noexcept;
		ConstObjectPtr DynamicCast_BaseToDerived(TypeID derivedID) const noexcept;
		ConstObjectPtr StaticCast               (TypeID typeID   ) const noexcept;
		ConstObjectPtr DynamicCast              (TypeID typeID   ) const noexcept;

		bool IsInvocable(StrID methodID, Span<TypeID> argTypeIDs = {}) const noexcept;

		InvokeResult Invoke(
			StrID methodID,
			Span<TypeID> argTypeIDs = {},
			void* args_buffer = nullptr,
			void* result_buffer = nullptr) const;

		template<typename... Args>
		bool IsInvocable(StrID methodID) const noexcept;

		template<typename T>
		T InvokeRet(StrID methodID, Span<TypeID> argTypeIDs = {}, void* args_buffer = nullptr) const;

		template<typename... Args>
		InvokeResult InvokeArgs(StrID methodID, void* result_buffer, Args... args) const;

		template<typename T, typename... Args>
		T Invoke(StrID methodID, Args... args) const;
	};

	class ObjectPtr : public ObjectPtrBase {
	public:
		using ObjectPtrBase::ObjectPtrBase;

		constexpr void* GetPtr() const noexcept { return ptr; }

		using ObjectPtrBase::AsPtr;
		using ObjectPtrBase::As;

		constexpr operator void*() const noexcept { return ptr; }
		constexpr operator ConstObjectPtr() const noexcept { return {ID, ptr}; }

		constexpr       ObjectPtr* operator->()       noexcept { return this; }
		constexpr const ObjectPtr* operator->() const noexcept { return this; }

		//
		// ReflMngr
		/////////////

		// variable
		ObjectPtr      RWVar(StrID fieldID) const noexcept;
		// variable, for diamond inheritance
		ObjectPtr      RWVar(TypeID baseID, StrID fieldID) const noexcept;

		ObjectPtr StaticCast_DerivedToBase (TypeID baseID)    const noexcept;
		ObjectPtr StaticCast_BaseToDerived (TypeID derivedID) const noexcept;
		ObjectPtr DynamicCast_BaseToDerived(TypeID derivedID) const noexcept;
		ObjectPtr StaticCast               (TypeID typeID)    const noexcept;
		ObjectPtr DynamicCast              (TypeID typeID)    const noexcept;

		bool IsInvocable(StrID methodID, Span<TypeID> argTypeIDs = {}) const noexcept;

		InvokeResult Invoke(
			StrID methodID,
			Span<TypeID> argTypeIDs = {},
			void* args_buffer = nullptr,
			void* result_buffer = nullptr) const;

		template<typename... Args>
		bool IsInvocable(StrID methodID) const noexcept;

		template<typename T>
		T InvokeRet(StrID methodID, Span<TypeID> argTypeIDs = {}, void* args_buffer = nullptr) const;

		template<typename... Args>
		InvokeResult InvokeArgs(StrID methodID, void* result_buffer, Args... args) const;

		template<typename T, typename... Args>
		T Invoke(StrID methodID, Args... args) const;

		// self [r/w] vars and all bases' [r/w] vars
		void ForEachRWVar(const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func) const;
	};
}

#include "details/ObjectPtr.inl"
