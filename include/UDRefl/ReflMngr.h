#pragma once

#include "TypeInfo.h"
#include "Object.h"
#include "IDRegistry.h"

namespace Ubpa::UDRefl {
	constexpr TypeID GlobalID = TypeID{ TypeIDRegistry::Meta::global };

	class ReflMngr {
	public:
		static ReflMngr& Instance() noexcept {
			static ReflMngr instance;
			return instance;
		}

		//
		// Data
		/////////

		StrIDRegistry nregistry;
		TypeIDRegistry tregistry;

		std::unordered_map<TypeID, TypeInfo> typeinfos;

		// clear order
		// - field attrs
		// - type attrs
		// - type dynamic shared field
		// - typeinfos
		void Clear() noexcept;

		//
		// Lookup
		///////////

		bool IsRegistered(TypeID typeID) const noexcept { return typeinfos.find(typeID) != typeinfos.end(); }

		//
		// Factory
		////////////

		// field_data can be:
		// - static field: pointer to **non-void** type
		// - member object pointer: pointer to **non-void** type
		template<auto field_data>
		FieldPtr GenerateFieldPtr();

		// data can be:
		// 1. member object pointer
		// 2. pointer to **non-void** and **non-function** type
		// 3. functor : Value*(Object*)
		// > - result must be an pointer of **non-void** type
		// 4. enum value
		template<typename T>
		FieldPtr GenerateFieldPtr(T&& data);

		template<typename T, typename... Args>
		FieldPtr GenerateDynamicFieldPtr(Args&&... args);

		std::pair<StrID, FieldInfo> GenerateField(
			std::string_view name,
			FieldPtr fieldptr,
			AttrSet attrs = {})
		{ return { nregistry.Register(name), { std::move(fieldptr), std::move(attrs) } }; }

		template<auto field_data>
		std::pair<StrID, FieldInfo> GenerateField(
			std::string_view name,
			AttrSet attrs = {})
		{ return GenerateField(name, GenerateFieldPtr<field_data>(), std::move(attrs)); }

		template<typename T,
			std::enable_if_t<!std::is_same_v<std::decay_t<T>, FieldPtr>, int> = 0>
		std::pair<StrID, FieldInfo> GenerateField(
			std::string_view name,
			T&& data,
			AttrSet attrs = {})
		{ return GenerateField(name, GenerateFieldPtr(std::forward<T>(data)), std::move(attrs)); }

		template<typename T, typename... Args>
		std::pair<StrID, FieldInfo> GenerateDynamicFieldWithAttrs(
			std::string_view name,
			AttrSet attrs,
			Args&&... args)
		{ return GenerateField(name, GenerateDynamicFieldPtr<T>(std::forward<Args>(args)...), std::move(attrs)); }

		template<typename T, typename... Args>
		std::pair<StrID, FieldInfo> GenerateDynamicField(
			std::string_view name,
			Args&&... args)
		{ return GenerateDynamicFieldWithAttrs<T>(name, {}, std::forward<Args>(args)...); }

		template<typename Return>
		ResultDesc GenerateResultDesc();

		template<typename... Params>
		ParamList GenerateParamList() noexcept(sizeof...(Params) == 0);

		// funcptr can be
		// 1. member method : member function pointer
		// 2. static method : function pointer
		template<auto funcptr>
		MethodPtr GenerateMethodPtr();

		template<typename T, typename... Args>
		MethodPtr GenerateConstructorPtr();

		template<typename T>
		MethodPtr GenerateDestructorPtr();

		template<typename Func>
		MethodPtr GenerateMemberMethodPtr(Func&& func);

		template<typename Func>
		MethodPtr GenerateStaticMethodPtr(Func&& func);

		std::pair<StrID, MethodInfo> GenerateMethod(
			std::string_view name,
			MethodPtr methodptr,
			AttrSet attrs = {})
		{ return { nregistry.Register(name), { std::move(methodptr), std::move(attrs) } }; }

		template<auto funcptr>
		std::pair<StrID, MethodInfo> GenerateMethod(
			std::string_view name,
			AttrSet attrs = {})
		{ return GenerateMethod(name, GenerateMethodPtr<funcptr>(), std::move(attrs)); }

		template<typename Func>
		std::pair<StrID, MethodInfo> GenerateMemberMethod(
			std::string_view name,
			Func&& func,
			AttrSet attrs = {})
		{ return GenerateMethod(name, GenerateMemberMethod(std::forward<Func>(func)), std::move(attrs)); }

		template<typename Func>
		std::pair<StrID, MethodInfo> GenerateStaticMethod(
			std::string_view name,
			Func&& func,
			AttrSet attrs = {})
		{ return GenerateMethod(name, GenerateStaticMethod(std::forward<Func>(func)), std::move(attrs)); }

		template<typename T, typename... Args>
		std::pair<StrID, MethodInfo> GenerateConstructor(AttrSet attrs = {})
		{ return { StrID{StrIDRegistry::Meta::ctor}, { GenerateConstructorPtr<T, Args...>(), std::move(attrs) } }; }

		template<typename T>
		std::pair<StrID, MethodInfo> GenerateDestructor(AttrSet attrs = {})
		{ return { StrID{StrIDRegistry::Meta::dtor}, { GenerateDestructorPtr<T>(), std::move(attrs) } }; }

		template<typename Derived, typename Base>
		static BaseInfo GenerateBaseInfo();

		//
		// Modifier
		/////////////

		TypeID RegisterType(std::string_view name, size_t size, size_t alignment);
		StrID AddField(TypeID typeID, std::string_view name, FieldInfo fieldinfo);
		StrID AddMethod(TypeID typeID, std::string_view name, MethodInfo methodinfo);
		bool AddBase(TypeID derivedID, TypeID baseID, BaseInfo baseinfo);

		// -- template --

		template<typename T>
		void RegisterType();

		// RegisterType<T>
		// AddConstructor<T, Args...>
		// AddDestructor<T>
		template<typename T, typename... Args>
		void RegisterTypeAuto(AttrSet attrs_ctor = {}, AttrSet attrs_dtor = {});

		// get TypeID from field_data
		template<auto field_data>
		StrID AddField(std::string_view name, AttrSet attrs = {});

		template<typename T,
			std::enable_if_t<!std::is_same_v<std::decay_t<T>, FieldInfo>, int> = 0>
		StrID AddField(TypeID typeID, std::string_view name, T&& data, AttrSet attrs = {})
		{ return AddField(typeID, name, GenerateFieldPtr(std::forward<T>(data)), std::move(attrs)); }

		template<typename T, typename... Args>
		StrID AddDynamicFieldWithAttr(TypeID typeID, std::string_view name, AttrSet attrs, Args&&... args)
		{ return AddField(typeID, name, { GenerateDynamicFieldPtr<T>(std::forward<Args>(args)...), std::move(attrs) }); }

		template<typename T, typename... Args>
		StrID AddDynamicField(TypeID typeID, std::string_view name, Args&&... args)
		{ return AddDynamicFieldWithAttr<T>(typeID, name, {}, std::forward<Args>(args)...); }

		// funcptr is member function pointer
		// get TypeID from funcptr
		template<auto member_func_ptr>
		StrID AddMethod(std::string_view name, AttrSet attrs = {});

		// funcptr is function pointer
		template<auto func_ptr>
		StrID AddMethod(TypeID typeID, std::string_view name, AttrSet attrs = {});

		template<typename T, typename... Args>
		bool AddConstructor(AttrSet attrs = {});
		template<typename T>
		bool AddDestructor(AttrSet attrs = {});

		template<typename Func>
		StrID AddMemberMethod(std::string_view name, Func&& func, AttrSet attrs = {});

		template<typename Func>
		StrID AddStaticMethod(TypeID typeID, std::string_view name, Func&& func, AttrSet attrs = {})
		{ return AddMethod(typeID, name, { GenerateStaticMethodPtr(std::forward<Func>(func)), std::move(attrs) }); }

		template<typename Derived, typename... Bases>
		bool AddBases();

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

		// variable object
		ObjectPtr      RWVar(TypeID      typeID, StrID fieldID) noexcept;
		// object
		ConstObjectPtr RVar (TypeID      typeID, StrID fieldID) const noexcept;
		// variable
		ObjectPtr      RWVar(ObjectPtr      obj, StrID fieldID) noexcept;
		// all
		ConstObjectPtr RVar (ConstObjectPtr obj, StrID fieldID) const noexcept;
		// variable, for diamond inheritance
		ObjectPtr      RWVar(ObjectPtr      obj, TypeID baseID, StrID fieldID) noexcept;
		// all, for diamond inheritance
		ConstObjectPtr RVar (ConstObjectPtr obj, TypeID baseID, StrID fieldID) const noexcept;

		//
		// Invoke
		///////////

		bool IsStaticInvocable(TypeID typeID, StrID methodID, Span<const TypeID> argTypeIDs = {}) const noexcept;
		bool IsConstInvocable (TypeID typeID, StrID methodID, Span<const TypeID> argTypeIDs = {}) const noexcept;
		bool IsInvocable      (TypeID typeID, StrID methodID, Span<const TypeID> argTypeIDs = {}) const noexcept;

		InvokeResult Invoke(
			TypeID typeID,
			StrID methodID,
			void* result_buffer = nullptr,
			Span<const TypeID> argTypeIDs = {},
			void* args_buffer = nullptr) const;

		InvokeResult Invoke(
			ConstObjectPtr obj,
			StrID methodID,
			void* result_buffer = nullptr,
			Span<const TypeID> argTypeIDs = {},
			void* args_buffer = nullptr) const;

		InvokeResult Invoke(
			ObjectPtr obj,
			StrID methodID,
			void* result_buffer = nullptr,
			Span<const TypeID> argTypeIDs = {},
			void* args_buffer = nullptr) const;

		// -- template --

		template<typename... Args>
		bool IsStaticInvocable(TypeID typeID, StrID methodID) const noexcept;
		template<typename... Args>
		bool IsConstInvocable (TypeID typeID, StrID methodID) const noexcept;
		template<typename... Args>
		bool IsInvocable      (TypeID typeID, StrID methodID) const noexcept;

		template<typename T>
		T InvokeRet(TypeID      typeID, StrID methodID, Span<const TypeID> argTypeIDs = {}, void* args_buffer = nullptr) const;
		template<typename T>
		T InvokeRet(ConstObjectPtr obj, StrID methodID, Span<const TypeID> argTypeIDs = {}, void* args_buffer = nullptr) const;
		template<typename T>
		T InvokeRet(ObjectPtr      obj, StrID methodID, Span<const TypeID> argTypeIDs = {}, void* args_buffer = nullptr) const;

		template<typename... Args>
		InvokeResult InvokeArgs(TypeID      typeID, StrID methodID, void* result_buffer, Args... args) const;
		template<typename... Args>
		InvokeResult InvokeArgs(ConstObjectPtr obj, StrID methodID, void* result_buffer, Args... args) const;
		template<typename... Args>
		InvokeResult InvokeArgs(ObjectPtr      obj, StrID methodID, void* result_buffer, Args... args) const;

		template<typename T, typename... Args>
		T Invoke(TypeID      typeID, StrID methodID, Args... args) const;
		template<typename T, typename... Args>
		T Invoke(ConstObjectPtr obj, StrID methodID, Args... args) const;
		template<typename T, typename... Args>
		T Invoke(ObjectPtr      obj, StrID methodID, Args... args) const;

		//
		// Meta
		/////////

		bool IsConstructible(TypeID typeID, Span<const TypeID> argTypeIDs = {}) const noexcept;
		bool IsDestructible (TypeID typeID) const noexcept;

		bool Construct(ObjectPtr      obj, Span<const TypeID> argTypeIDs = {}, void* args_buffer = nullptr) const;
		bool Destruct (ConstObjectPtr obj) const;

		void* Malloc(size_t size) const;
		bool  Free  (void* ptr) const;

		void* AlignedMalloc(size_t size, size_t alignment) const;
		bool  AlignedFree  (void* ptr) const;

		ObjectPtr New   (TypeID typeID, Span<const TypeID> argTypeIDs = {}, void* args_buffer = nullptr) const;
		bool      Delete(ConstObjectPtr obj) const;

		SharedObject MakeShared(TypeID typeID, Span<const TypeID> argTypeIDs = {}, void* args_buffer = nullptr) const;

		// -- template --

		template<typename... Args>
		bool IsConstructible(TypeID typeID) const noexcept;

		template<typename... Args>
		bool Construct(ObjectPtr obj, Args... args) const;
		
		template<typename... Args>
		ObjectPtr New(TypeID typeID, Args... args) const;

		template<typename... Args>
		SharedObject MakeShared(TypeID typeID, Args... args) const;

		// if T is not register, call RegisterTypeAuto
		// else add ctor
		template<typename T, typename... Args>
		ObjectPtr New(Args... args);

		// if T is not register, call RegisterTypeAuto
		// else add ctor
		template<typename T, typename... Args>
		SharedObject MakeShared(Args... args);

		//
		// Algorithm
		//////////////

		// self typeID and all bases' typeID
		void ForEachTypeID(
			TypeID typeID,
			const std::function<bool(TypeID)>& func) const;

		// self type and all bases' type
		void ForEachType(
			TypeID typeID,
			const std::function<bool(TypeRef)>& func) const;

		// self fields and all bases' fields
		void ForEachField(
			TypeID typeID,
			const std::function<bool(TypeRef, FieldRef)>& func) const;

		// self methods and all bases' methods
		void ForEachMethod(
			TypeID typeID,
			const std::function<bool(TypeRef, Method)>& func) const;

		// self [r/w] vars and all bases' [r/w] vars
		void ForEachRWVar(
			ObjectPtr obj,
			const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func) const;

		// self [r] vars and all bases' [r] vars
		void ForEachRVar(
			ConstObjectPtr obj,
			const std::function<bool(TypeRef, FieldRef, ConstObjectPtr)>& func) const;

		// self [r/w] object vars and all bases' [r/w] object vars
		void ForEachRWVar(
			TypeID typeID,
			const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func) const;

		// self [r] object vars and all bases' [r] object vars
		void ForEachRVar(
			TypeID typeID,
			const std::function<bool(TypeRef, FieldRef, ConstObjectPtr)>& func) const;

		//
		// Memory
		///////////

		void ReleaseMono();
		void* MAllocate(MemoryResourceType type, size_t size, size_t alignment = alignof(std::max_align_t));
		void  MDeallocate(MemoryResourceType type, void* ptr, size_t size, size_t alignment = alignof(std::max_align_t));

		SharedObject MInvoke(
			TypeID typeID,
			StrID methodID,
			Span<const TypeID> argTypeIDs = {},
			void* args_buffer = nullptr,
			MemoryResourceType memory_rsrc_type = MemoryResourceType::SYNC);

		SharedObject MInvoke(
			ConstObjectPtr obj,
			StrID methodID,
			Span<const TypeID> argTypeIDs = {},
			void* args_buffer = nullptr,
			MemoryResourceType memory_rsrc_type = MemoryResourceType::SYNC);

		SharedObject MInvoke(
			ObjectPtr obj,
			StrID methodID,
			Span<const TypeID> argTypeIDs = {},
			void* args_buffer = nullptr,
			MemoryResourceType memory_rsrc_type = MemoryResourceType::SYNC);

		template<typename... Args>
		SharedObject MInvoke(
			TypeID typeID,
			StrID methodID,
			MemoryResourceType memory_rsrc_type,
			Args... args);

		template<typename... Args>
		SharedObject MInvoke(
			ConstObjectPtr obj,
			StrID methodID,
			MemoryResourceType memory_rsrc_type,
			Args... args);

		template<typename... Args>
		SharedObject MInvoke(
			ObjectPtr obj,
			StrID methodID,
			MemoryResourceType memory_rsrc_type,
			Args... args);

	private:
		ReflMngr();
		~ReflMngr();

		std::pmr::monotonic_buffer_resource    mono_rsrc;
		std::pmr::unsynchronized_pool_resource unsync_rsrc;
		std::pmr::synchronized_pool_resource   sync_rsrc;
	};
}

#include "details/ReflMngr.inl"
