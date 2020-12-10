#pragma once

#include "TypeInfo.h"
#include "EnumInfo.h"
#include "SharedObject.h"

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
		// Factory
		////////////

		template<auto field_ptr>
		FieldPtr GenerateFieldPtr();

		// data can be:
		// 1. member object pointer
		// 2. pointer to **non-void** and **non-function** type
		// 3. functor : Value*(Object*)
		// > - <Object> can be any **non-const** type because it's useless
		// > - result must be an pointer of **non-void** type
		template<typename T>
		FieldPtr GenerateFieldPtr(T&& data);

		template<typename T, typename... Args>
		FieldPtr GenerateDynamicFieldPtr(Args&&... args);

		template<typename Return>
		ResultDesc GenerateResultDesc();

		template<typename... Params>
		ParamList GenerateParamList() noexcept(sizeof...(Params) == 0);

		template<auto funcptr>
		MethodPtr GenerateMethodPtr();

		template<typename Func>
		MethodPtr GenerateMemberMethodPtr(Func&& func);

		template<typename Func>
		MethodPtr GenerateStaticMethodPtr(Func&& func);

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

		// {static|dynamic} variable
		ObjectPtr      RWVar(TypeID      typeID, NameID fieldID) noexcept;
		// {static|dynamic} {variable|const}
		ConstObjectPtr RVar (TypeID      typeID, NameID fieldID) const noexcept;
		// variable
		ObjectPtr      RWVar(ObjectPtr      obj, NameID fieldID) noexcept;
		// all
		ConstObjectPtr RVar (ConstObjectPtr obj, NameID fieldID) const noexcept;

		//
		// Invoke
		///////////

		bool IsStaticInvocable(TypeID typeID, NameID methodID, Span<TypeID> argTypeIDs = {}) const noexcept;
		bool IsConstInvocable (TypeID typeID, NameID methodID, Span<TypeID> argTypeIDs = {}) const noexcept;
		bool IsInvocable      (TypeID typeID, NameID methodID, Span<TypeID> argTypeIDs = {}) const noexcept;

		InvokeResult Invoke(
			TypeID typeID,
			NameID methodID,
			Span<TypeID> argTypeIDs = {},
			void* args_buffer = nullptr,
			void* result_buffer = nullptr) const;

		InvokeResult Invoke(
			ConstObjectPtr obj,
			NameID methodID,
			Span<TypeID> argTypeIDs = {},
			void* args_buffer = nullptr,
			void* result_buffer = nullptr) const;

		InvokeResult Invoke(
			ObjectPtr obj,
			NameID methodID,
			Span<TypeID> argTypeIDs = {},
			void* args_buffer = nullptr,
			void* result_buffer = nullptr) const;

		// -- template --

		template<typename... Args>
		bool IsStaticInvocable(TypeID typeID, NameID methodID) const noexcept;
		template<typename... Args>
		bool IsConstInvocable (TypeID typeID, NameID methodID) const noexcept;
		template<typename... Args>
		bool IsInvocable      (TypeID typeID, NameID methodID) const noexcept;

		template<typename T>
		T InvokeRet(TypeID      typeID, NameID methodID, Span<TypeID> argTypeIDs = {}, void* args_buffer = nullptr) const;
		template<typename T>
		T InvokeRet(ConstObjectPtr obj, NameID methodID, Span<TypeID> argTypeIDs = {}, void* args_buffer = nullptr) const;
		template<typename T>
		T InvokeRet(ObjectPtr      obj, NameID methodID, Span<TypeID> argTypeIDs = {}, void* args_buffer = nullptr) const;

		template<typename... Args>
		InvokeResult InvokeArgs(TypeID      typeID, NameID methodID, void* result_buffer, Args... args) const;
		template<typename... Args>
		InvokeResult InvokeArgs(ConstObjectPtr obj, NameID methodID, void* result_buffer, Args... args) const;
		template<typename... Args>
		InvokeResult InvokeArgs(ObjectPtr      obj, NameID methodID, void* result_buffer, Args... args) const;

		template<typename T, typename... Args>
		T Invoke(TypeID      typeID, NameID methodID, Args... args) const;
		template<typename T, typename... Args>
		T Invoke(ConstObjectPtr obj, NameID methodID, Args... args) const;
		template<typename T, typename... Args>
		T Invoke(ObjectPtr      obj, NameID methodID, Args... args) const;

		template<typename Obj, typename... Args>
		bool IsStaticInvocable(NameID methodID) const noexcept;
		template<typename Obj, typename... Args>
		bool IsConstInvocable (NameID methodID) const noexcept;
		template<typename Obj, typename... Args>
		bool IsInvocable      (NameID methodID) const noexcept;
		template<typename Obj, typename T>
		T InvokeRet(NameID methodID, Span<TypeID> argTypeIDs = {}, void* args_buffer = nullptr) const;
		template<typename Obj, typename... Args>
		InvokeResult InvokeArgs(NameID methodID, void* result_buffer, Args... args) const;
		template<typename Obj, typename T, typename... Args>
		T Invoke(NameID methodID, Args... args) const;

		//
		// Meta
		/////////

		// global {static|dynamic} variable
		ObjectPtr      RWVar(NameID fieldID) noexcept
		{ return RWVar(TypeRegistry::DirectGetID(TypeRegistry::Meta::global), fieldID); }
		// global {static|dynamic} {variable|const}
		ConstObjectPtr RVar(NameID fieldID) const noexcept
		{ return RVar (TypeRegistry::DirectGetID(TypeRegistry::Meta::global), fieldID); }

		bool IsInvocable(NameID methodID, Span<TypeID> argTypeIDs = {}) const noexcept
		{ return IsInvocable(TypeRegistry::DirectGetID(TypeRegistry::Meta::global), methodID, argTypeIDs); }

		InvokeResult Invoke(NameID methodID, Span<TypeID> argTypeIDs = {}, void* args_buffer = nullptr, void* result_buffer = nullptr) const
		{ return Invoke     (TypeRegistry::DirectGetID(TypeRegistry::Meta::global), methodID, argTypeIDs, args_buffer, result_buffer); }

		bool IsConstructible(TypeID typeID, Span<TypeID> argTypeIDs = {}) const noexcept;
		bool IsDestructible (TypeID typeID) const noexcept;

		bool Construct(ObjectPtr      obj, Span<TypeID> argTypeIDs = {}, void* args_buffer = nullptr) const;
		bool Destruct (ConstObjectPtr obj) const;

		void* Malloc(uint64_t size) const;
		bool  Free  (void* ptr) const;

		void* AlignedMalloc(uint64_t size, uint64_t alignment) const;
		bool  AlignedFree  (void* ptr) const;

		ObjectPtr New   (TypeID typeID, Span<TypeID> argTypeIDs = {}, void* args_buffer = nullptr) const;
		bool      Delete(ConstObjectPtr obj) const;

		SharedObject MakeShared(TypeID typeID, Span<TypeID> argTypeIDs = {}, void* args_buffer = nullptr) const;

		// -- template --

		template<typename... Args>
		bool IsInvocable(NameID methodID) const noexcept;

		template<typename T>
		T InvokeRet(NameID methodID, Span<TypeID> argTypeIDs = {}, void* args_buffer = nullptr) const;

		template<typename... Args>
		InvokeResult InvokeArgs(NameID methodID, void* result_buffer, Args... args) const;

		template<typename T, typename... Args>
		T Invoke(NameID methodID, Args... args) const;

		template<typename... Args>
		bool IsConstructible(TypeID typeID) const noexcept;

		template<typename... Args>
		bool Construct(ObjectPtr obj, Args... args) const;
		
		template<typename... Args>
		ObjectPtr New(TypeID typeID, Args... args) const;

		template<typename T, typename... Args>
		ObjectPtr New(Args... args) const;

		template<typename... Args>
		SharedObject MakeShared(TypeID typeID, Args... args) const;

		template<typename T, typename... Args>
		SharedObject MakeShared(Args... args) const;

		//
		// Algorithm
		//////////////

		// self typeID and all bases' typeID
		void ForEachTypeID(
			TypeID typeID,
			const std::function<void(TypeID)>& func) const;

		// self type and all bases' type
		void ForEachType(
			TypeID typeID,
			const std::function<void(Type)>& func) const;

		// self fields and all bases' fields
		void ForEachField(
			TypeID typeID,
			const std::function<void(Type, Field)>& func) const;

		// self methods and all bases' methods
		void ForEachMethod(
			TypeID typeID,
			const std::function<void(Type, Method)>& func) const;

		// self [r/w] vars and all bases' [r/w] vars
		void ForEachRWVar(
			ObjectPtr obj,
			const std::function<void(Type, Field, ObjectPtr)>& func) const;

		// self [r] vars and all bases' [r] vars
		void ForEachRVar(
			ConstObjectPtr obj,
			const std::function<void(Type, Field, ConstObjectPtr)>& func) const;

	private:
		ReflMngr();
	};
}

#include "details/ReflMngr.inl"
