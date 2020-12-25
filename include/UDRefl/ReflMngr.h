#pragma once

#include "attrs/ContainerType.h"

#include "TypeInfo.h"

namespace Ubpa::UDRefl {
	constexpr TypeID GlobalID = TypeIDRegistry::MetaID::global;

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
		// - enumerator
		template<auto field_data>
		FieldPtr GenerateFieldPtr();

		// data can be:
		// 1. member object pointer
		// 2. pointer to **non-void** and **non-function** type
		// 3. functor : Value*(Object*)
		// > - result must be an pointer of **non-void** type
		// 4. enumerator
		template<typename T>
		FieldPtr GenerateFieldPtr(T&& data);

		// if T is bufferable, T will be stored as buffer,
		// else we will use std::make_shared to store it
		// require alignof(T) <= alignof(std::max_align_t)
		template<typename T, typename... Args>
		FieldPtr GenerateDynamicFieldPtr(Args&&... args);

		// if T is bufferable, T will be stored as buffer,
		// else we will use std::alloc_shared to store it
		template<typename T, typename Alloc, typename... Args>
		FieldPtr GenerateDynamicFieldPtrByAlloc(const Alloc& alloc, Args&&... args);

		std::pair<StrID, FieldInfo> GenerateField(
			std::string_view name,
			FieldPtr fieldptr,
			AttrSet attrs = {})
		{ return { nregistry.Register(name), { std::move(fieldptr), std::move(attrs) } }; }

		// field_data can be:
		// - static field: pointer to **non-void** type
		// - member object pointer: pointer to **non-void** type
		// - enumerator
		template<auto field_data>
		std::pair<StrID, FieldInfo> GenerateField(
			std::string_view name,
			AttrSet attrs = {})
		{ return GenerateField(name, GenerateFieldPtr<field_data>(), std::move(attrs)); }

		// data can be:
		// 1. member object pointer
		// 2. pointer to **non-void** and **non-function** type
		// 3. functor : Value*(Object*)
		// > - result must be an pointer of **non-void** type
		// 4. enumerator
		template<typename T,
			std::enable_if_t<!std::is_same_v<std::decay_t<T>, FieldPtr>, int> = 0>
		std::pair<StrID, FieldInfo> GenerateField(
			std::string_view name,
			T&& data,
			AttrSet attrs = {})
		{ return GenerateField(name, GenerateFieldPtr(std::forward<T>(data)), std::move(attrs)); }

		// if T is bufferable, T will be stored as buffer,
		// else we will use std::make_shared to store it
		// require alignof(T) <= alignof(std::max_align_t)
		template<typename T, typename... Args>
		std::pair<StrID, FieldInfo> GenerateDynamicFieldWithAttrs(
			std::string_view name,
			AttrSet attrs,
			Args&&... args)
		{ return GenerateField(name, GenerateDynamicFieldPtr<T>(std::forward<Args>(args)...), std::move(attrs)); }

		// if T is bufferable, T will be stored as buffer,
		// else we will use std::make_shared to store it
		// require alignof(T) <= alignof(std::max_align_t)
		template<typename T, typename... Args>
		std::pair<StrID, FieldInfo> GenerateDynamicField(
			std::string_view name,
			Args&&... args)
		{ return GenerateDynamicFieldWithAttrs<T>(name, {}, std::forward<Args>(args)...); }

		// if T is bufferable, T will be stored as buffer,
		// else we will use std::make_shared to store it
		template<typename T, typename Alloc, typename... Args>
		std::pair<StrID, FieldInfo> GenerateDynamicFieldByAllocWithAttrs(
			std::string_view name,
			const Alloc& alloc,
			AttrSet attrs,
			Args&&... args)
		{ return GenerateField(name, GenerateDynamicFieldPtrByAlloc<T>(alloc, std::forward<Args>(args)...), std::move(attrs)); }

		// if T is bufferable, T will be stored as buffer,
		// else we will use std::make_shared to store it
		template<typename T, typename Alloc, typename... Args>
		std::pair<StrID, FieldInfo> GenerateDynamicFieldByAlloc(
			std::string_view name,
			const Alloc& alloc,
			Args&&... args)
		{ return GenerateDynamicFieldByAllocWithAttrs<T>(name, alloc, {}, std::forward<Args>(args)...); }

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

		// Func: Ret(const? volatile? Object&, Args...)
		template<typename Func>
		MethodPtr GenerateMemberMethodPtr(Func&& func);

		// Func: Ret(Args...)
		template<typename Func>
		MethodPtr GenerateStaticMethodPtr(Func&& func);

		std::pair<StrID, MethodInfo> GenerateMethod(
			std::string_view name,
			MethodPtr methodptr,
			AttrSet attrs = {})
		{ return { nregistry.Register(name), { std::move(methodptr), std::move(attrs) } }; }

		// funcptr can be
		// 1. member method : member function pointer
		// 2. static method : function pointer
		template<auto funcptr>
		std::pair<StrID, MethodInfo> GenerateMethod(
			std::string_view name,
			AttrSet attrs = {})
		{ return GenerateMethod(name, GenerateMethodPtr<funcptr>(), std::move(attrs)); }

		// Func: Ret(const? volatile? Object&, Args...)
		template<typename Func>
		std::pair<StrID, MethodInfo> GenerateMemberMethod(
			std::string_view name,
			Func&& func,
			AttrSet attrs = {})
		{ return GenerateMethod(name, GenerateMemberMethod(std::forward<Func>(func)), std::move(attrs)); }

		// Func: Ret(Args...)
		template<typename Func>
		std::pair<StrID, MethodInfo> GenerateStaticMethod(
			std::string_view name,
			Func&& func,
			AttrSet attrs = {})
		{ return GenerateMethod(name, GenerateStaticMethod(std::forward<Func>(func)), std::move(attrs)); }

		template<typename T, typename... Args>
		std::pair<StrID, MethodInfo> GenerateConstructor(AttrSet attrs = {})
		{ return { StrIDRegistry::MetaID::ctor, { GenerateConstructorPtr<T, Args...>(), std::move(attrs) } }; }

		template<typename T>
		std::pair<StrID, MethodInfo> GenerateDestructor(AttrSet attrs = {})
		{ return { StrIDRegistry::MetaID::dtor, { GenerateDestructorPtr<T>(), std::move(attrs) } }; }

		template<typename Derived, typename Base>
		static BaseInfo GenerateBaseInfo();

		//
		// Modifier
		/////////////

		TypeID RegisterType(std::string_view name, size_t size, size_t alignment);
		StrID AddField(TypeID typeID, std::string_view name, FieldInfo fieldinfo);
		StrID AddMethod(TypeID typeID, std::string_view name, MethodInfo methodinfo);
		bool AddBase(TypeID derivedID, TypeID baseID, BaseInfo baseinfo);
		bool AddAttr(TypeID typeID, const Attr& attr);

		// -- template --

		template<typename T>
		void RegisterType();

		// RegisterType<T>
		// AddConstructor<T>
		// AddDestructor<T>
		template<typename T>
		void RegisterTypeAuto(AttrSet attrs_ctor = {}, AttrSet attrs_dtor = {});

		// get TypeID from field_data
		// field_data can be
		// 1. member object pointer
		// 2. enumerator
		template<auto field_data>
		StrID AddField(std::string_view name, AttrSet attrs = {});

		// data can be:
		// 1. member object pointer
		// 2. enumerator
		// 3. pointer to **non-void** and **non-function** type
		// 4. functor : Value*(Object*)
		// > - result must be an pointer of **non-void** type
		template<typename T,
			std::enable_if_t<!std::is_same_v<std::decay_t<T>, FieldInfo>, int> = 0>
		StrID AddField(TypeID typeID, std::string_view name, T&& data, AttrSet attrs = {})
		{ return AddField(typeID, name, { GenerateFieldPtr(std::forward<T>(data)), std::move(attrs) }); }

		// data can be:
		// 1. member object pointer
		// 2. functor : Value*(Object*)
		// > - result must be an pointer of **non-void** type
		// 3. enumerator
		template<typename T,
			std::enable_if_t<!std::is_same_v<std::decay_t<T>, FieldInfo>, int> = 0>
		StrID AddField(std::string_view name, T&& data, AttrSet attrs = {});

		template<typename T, typename... Args>
		StrID AddDynamicFieldWithAttr(TypeID typeID, std::string_view name, AttrSet attrs, Args&&... args)
		{ return AddField(typeID, name, { GenerateDynamicFieldPtr<T>(std::forward<Args>(args)...), std::move(attrs) }); }

		template<typename T, typename... Args>
		StrID AddDynamicField(TypeID typeID, std::string_view name, Args&&... args)
		{ return AddDynamicFieldWithAttr<T>(typeID, name, {}, std::forward<Args>(args)...); }

		template<typename T, typename Alloc, typename... Args>
		StrID AddDynamicFieldByAllocWithAttr(TypeID typeID, std::string_view name, const Alloc& alloc, AttrSet attrs, Args&&... args)
		{ return AddField(typeID, name, { GenerateDynamicFieldPtrByAlloc<T>(alloc, std::forward<Args>(args)...), std::move(attrs) }); }

		template<typename T, typename Alloc, typename... Args>
		StrID AddDynamicFieldByAlloc(TypeID typeID, std::string_view name, const Alloc& alloc, Args&&... args)
		{ return AddDynamicFieldByAllocWithAttr<T>(typeID, name, alloc, {}, std::forward<Args>(args)...); }

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

		// Func: Ret(const? volatile? Object&, Args...)
		template<typename Func>
		StrID AddMemberMethod(std::string_view name, Func&& func, AttrSet attrs = {});

		// Func: Ret(Args...)
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

		InvocableResult IsStaticInvocable(TypeID typeID, StrID methodID, Span<const TypeID> argTypeIDs = {}) const noexcept;
		InvocableResult IsConstInvocable (TypeID typeID, StrID methodID, Span<const TypeID> argTypeIDs = {}) const noexcept;
		InvocableResult IsInvocable      (TypeID typeID, StrID methodID, Span<const TypeID> argTypeIDs = {}) const noexcept;

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
		InvocableResult IsStaticInvocable(TypeID typeID, StrID methodID) const noexcept;
		template<typename... Args>
		InvocableResult IsConstInvocable (TypeID typeID, StrID methodID) const noexcept;
		template<typename... Args>
		InvocableResult IsInvocable      (TypeID typeID, StrID methodID) const noexcept;

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
		ObjectPtr NewAuto(Args... args);

		// if T is not register, call RegisterTypeAuto
		// else add ctor
		template<typename T, typename... Args>
		SharedObject MakeSharedAuto(Args... args);

		//
		// Algorithm
		//////////////

		// ForEach (DFS)

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
			const std::function<bool(TypeRef, MethodRef)>& func) const;

		// self [r/w] object vars and all bases' [r/w] object vars
		void ForEachRWVar(
			TypeID typeID,
			const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func) const;

		// self [r] object vars and all bases' [r] object vars
		void ForEachRVar(
			TypeID typeID,
			const std::function<bool(TypeRef, FieldRef, ConstObjectPtr)>& func) const;

		// self [r/w] vars and all bases' [r/w] vars
		// if obj is &{const{T}}, then return directly
		void ForEachRWVar(
			ObjectPtr obj,
			const std::function<bool(TypeRef, FieldRef, ObjectPtr)>& func) const;

		// self [r] vars and all bases' [r] vars
		void ForEachRVar(
			ConstObjectPtr obj,
			const std::function<bool(TypeRef, FieldRef, ConstObjectPtr)>& func) const;

		// Gather (DFS)

		std::vector<TypeID>                                        GetTypeIDs        (TypeID      typeID);
		std::vector<TypeRef>                                       GetTypes          (TypeID      typeID);
		std::vector<TypeFieldRef>                                  GetTypeFields     (TypeID      typeID);
		std::vector<FieldRef>                                      GetFields         (TypeID      typeID);
		std::vector<TypeMethodRef>                                 GetTypeMethods    (TypeID      typeID);
		std::vector<MethodRef>                                     GetMethods        (TypeID      typeID);
		std::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>>      GetTypeFieldRWVars(TypeID      typeID);
		std::vector<ObjectPtr>                                     GetRWVars         (TypeID      typeID);
		std::vector<std::tuple<TypeRef, FieldRef, ConstObjectPtr>> GetTypeFieldRVars (TypeID      typeID);
		std::vector<ConstObjectPtr>                                GetRVars          (TypeID      typeID);
		std::vector<std::tuple<TypeRef, FieldRef, ObjectPtr>>      GetTypeFieldRWVars(ObjectPtr      obj);
		std::vector<ObjectPtr>                                     GetRWVars         (ObjectPtr      obj);
		std::vector<std::tuple<TypeRef, FieldRef, ConstObjectPtr>> GetTypeFieldRVars (ConstObjectPtr obj);
		std::vector<ConstObjectPtr>                                GetRVars          (ConstObjectPtr obj);

		// Find (DFS)

		std::optional<TypeID   > FindTypeID(TypeID      typeID, const std::function<bool(TypeID        )>& func) const;
		std::optional<TypeRef  > FindType  (TypeID      typeID, const std::function<bool(TypeRef       )>& func) const;
		std::optional<FieldRef > FindField (TypeID      typeID, const std::function<bool(FieldRef      )>& func) const;
		std::optional<MethodRef> FindMethod(TypeID      typeID, const std::function<bool(MethodRef     )>& func) const;
		ObjectPtr                FindRWVar (TypeID      typeID, const std::function<bool(ObjectPtr     )>& func) const;
		ConstObjectPtr           FindRVar  (TypeID      typeID, const std::function<bool(ConstObjectPtr)>& func) const;
		ObjectPtr                FindRWVar (ObjectPtr      obj, const std::function<bool(ObjectPtr     )>& func) const;
		ConstObjectPtr           FindRVar  (ConstObjectPtr obj, const std::function<bool(ConstObjectPtr)>& func) const;

		//
		// Memory
		///////////

		SharedObject MInvoke(
			TypeID typeID,
			StrID methodID,
			Span<const TypeID> argTypeIDs = {},
			void* args_buffer = nullptr,
			std::pmr::memory_resource* result_rsrc = std::pmr::get_default_resource());

		SharedObject MInvoke(
			ConstObjectPtr obj,
			StrID methodID,
			Span<const TypeID> argTypeIDs = {},
			void* args_buffer = nullptr,
			std::pmr::memory_resource* result_rsrc = std::pmr::get_default_resource());

		SharedObject MInvoke(
			ObjectPtr obj,
			StrID methodID,
			Span<const TypeID> argTypeIDs = {},
			void* args_buffer = nullptr,
			std::pmr::memory_resource* result_rsrc = std::pmr::get_default_resource());

		template<typename... Args>
		SharedObject MInvoke(
			TypeID typeID,
			StrID methodID,
			std::pmr::memory_resource* result_rsrc,
			Args... args);

		template<typename... Args>
		SharedObject MInvoke(
			ConstObjectPtr obj,
			StrID methodID,
			std::pmr::memory_resource* result_rsrc,
			Args... args);

		template<typename... Args>
		SharedObject MInvoke(
			ObjectPtr obj,
			StrID methodID,
			std::pmr::memory_resource* result_rsrc,
			Args... args);

		template<typename... Args>
		SharedObject DMInvoke(
			TypeID typeID,
			StrID methodID,
			Args... args);

		template<typename... Args>
		SharedObject DMInvoke(
			ConstObjectPtr obj,
			StrID methodID,
			Args... args);

		template<typename... Args>
		SharedObject DMInvoke(
			ObjectPtr obj,
			StrID methodID,
			Args... args);

		//
		// Type
		/////////

		DereferenceProperty GetDereferenceProperty(TypeID ID) const;
		TypeID Dereference(TypeID ID) const;
		ObjectPtr Dereference(ConstObjectPtr pointer_obj) const;
		ConstObjectPtr DereferenceAsConst(ConstObjectPtr pointer_obj) const;

	private:
		ReflMngr();
		~ReflMngr();
	};

	inline static std::add_const_t<ReflMngr*> Mngr = &ReflMngr::Instance();
}

#include "details/ReflMngr.inl"
