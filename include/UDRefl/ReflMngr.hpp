#pragma once

#include "Info.hpp"

namespace Ubpa::UDRefl {
	constexpr Type GlobalType = TypeIDRegistry::Meta::global;
	constexpr ObjectView Global = { GlobalType, nullptr };

	class UDRefl_core_API ReflMngr {
	public:
		static ReflMngr& Instance() noexcept;

		//
		// Data
		/////////
		//
		// enum is a special type (all member is static)
		//

		mutable NameIDRegistry nregistry;
		mutable TypeIDRegistry tregistry;

		std::unordered_map<Type, TypeInfo> typeinfos;

		TypeInfo* GetTypeInfo(Type type) const;
		SharedObject GetTypeAttr(Type type, Type attr_type) const;
		SharedObject GetFieldAttr(Type type, Name field_name, Type attr_type) const;
		SharedObject GetMethodAttr(Type type, Name method_name, Type attr_type) const;

		void SetTemporaryResource(std::shared_ptr<std::pmr::memory_resource> rsrc);
		void SetObjectResource(std::shared_ptr<std::pmr::memory_resource> rsrc);

		std::pmr::memory_resource* GetTemporaryResource() const { return temporary_resource.get(); }
		std::pmr::memory_resource* GetObjectResource() const { return object_resource.get(); }

		// clear order
		// - field attrs
		// - type attrs
		// - type dynamic shared field
		// - typeinfos
		// - temporary_resource
		// - object_resource
		void Clear() noexcept;

		//
		// Traits
		///////////

		bool ContainsVirtualBase(Type type) const;

		//
		// Factory
		////////////
		//
		// - we will register the value type when generating FieldPtr, so those APIs aren't static
		//

		// field_data can be:
		// - static field: pointer to **non-void** type
		// - member object pointer
		// - enumerator
		template<auto field_data, bool NeedRegisterFieldType = true>
		FieldPtr GenerateFieldPtr();

		template<auto field_data>
		FieldPtr SimpleGenerateFieldPtr() { return GenerateFieldPtr<field_data, false>(); }

		// data can be:
		// 1. member object pointer
		// 2. pointer to **non-void** and **non-function** type
		// 3. functor : Value*(Object*)  / Value&(Object*)
		// 4. enumerator
		template<typename T, bool NeedRegisterFieldType = true>
		FieldPtr GenerateFieldPtr(T&& data);

		template<typename T>
		FieldPtr SimpleGenerateFieldPtr(T&& data) { return GenerateFieldPtr<T, false>(std::forward<T>(data)); }

		// if T is bufferable, T will be stored as buffer,
		// else we will use MakeShared to store it
		template<typename T, typename... Args>
		FieldPtr GenerateDynamicFieldPtr(Args&&... args);

		template<typename T, typename... Args>
		FieldPtr SimpleGenerateDynamicFieldPtr(Args&&... args);

		template<typename... Params>
		static ParamList GenerateParamList() noexcept(sizeof...(Params) == 0);

		// funcptr can be
		// 1. member method : member function pointer
		// 2. static method : function pointer
		template<auto funcptr>
		static MethodPtr GenerateMethodPtr();

		// void(T&, Args...)
		template<typename T, typename... Args>
		static MethodPtr GenerateConstructorPtr();

		// void(const T&)
		template<typename T>
		static MethodPtr GenerateDestructorPtr();

		// Func: Ret(const? Object&, Args...)
		template<typename Func>
		static MethodPtr GenerateMemberMethodPtr(Func&& func);

		// Func: Ret(Args...)
		template<typename Func>
		static MethodPtr GenerateStaticMethodPtr(Func&& func);

		template<typename Derived, typename Base>
		static BaseInfo GenerateBaseInfo();

		//
		// Modifier
		/////////////

		// if is_trivial, register a trivial copy ctor
		Type RegisterType(Type type, size_t size, size_t alignment, bool is_polymorphic = false, bool is_trivial = false);
		Name AddField(Type type, Name field_name, FieldInfo fieldinfo);
		Name AddMethod(Type type, Name method_name, MethodInfo methodinfo);
		Type AddBase(Type derived, Type base, BaseInfo baseinfo);
		bool AddTypeAttr(Type type, Attr attr);
		bool AddFieldAttr(Type type, Name field_name, Attr attr);
		bool AddMethodAttr(Type type, Name method_name, Attr attr);
		
		Name AddTrivialDefaultConstructor(Type type);
		Name AddTrivialCopyConstructor   (Type type);
		Name AddZeroDefaultConstructor   (Type type);
		Name AddDefaultConstructor       (Type type);
		Name AddDestructor               (Type type);

		// - data-driven

		// require
		// - all bases aren't polymorphic and don't contain any virtual base
		// - field_types.size() == field_names.size()
		// auto compute
		// - size & alignment of type
		// - baseinfos
		// - fields' forward offset value
		Type RegisterType(
			Type type,
			std::span<const Type> bases,
			std::span<const Type> field_types,
			std::span<const Name> field_names,
			bool is_trivial = false
		);

		// -- template --

		// call
		// - RegisterType(type_name<T>(), sizeof(T), alignof(T), std::is_polymorphic<T>, std::is_trivial_v<T>)
		// - details::TypeAutoRegister<T>::run
		// you can custom type register by specialize details::TypeAutoRegister<T>
		template<typename T>
		void RegisterType();

		// get TypeID from field_data
		// field_data can be
		// 1. member object pointer
		// 2. enumerator
		template<auto field_data, bool NeedRegisterFieldType = true>
		bool AddField(Name name, AttrSet attrs = {});

		template<auto field_data>
		bool SimpleAddField(Name name, AttrSet attrs = {})
		{ return AddField<field_data, false>(name, std::move(attrs)); }

		// data can be:
		// 1. member object pointer
		// 2. enumerator
		// 3. pointer to **non-void** and **non-function** type
		// 4. functor : Value*(Object*) / Value&(Object*)
		template<typename T, bool NeedRegisterFieldType = true> requires std::negation_v<std::is_same<std::decay_t<T>, FieldInfo>>
		bool AddField(Type type, Name name, T&& data, AttrSet attrs = {})
		{ return AddField(type, name, { GenerateFieldPtr<T, NeedRegisterFieldType>(std::forward<T>(data)), std::move(attrs) }); }

		template<typename T> requires std::negation_v<std::is_same<std::decay_t<T>, FieldInfo>>
		bool SimpleAddField(Type type, Name name, T&& data, AttrSet attrs = {})
		{ return AddField<T, false>(type, name, std::forward<T>(data), std::move(attrs)); }

		// data can be:
		// 1. member object pointer
		// 2. functor : Value*(Object*)
		// > - result must be an pointer of **non-void** type
		// 3. enumerator
		template<typename T, bool NeedRegisterFieldType = true> requires std::negation_v<std::is_same<std::decay_t<T>, FieldInfo>>
		bool AddField(Name name, T&& data, AttrSet attrs = {});

		template<typename T> requires std::negation_v<std::is_same<std::decay_t<T>, FieldInfo>>
		bool SimpleAddField(Name name, T&& data, AttrSet attrs = {})
		{ return AddField<T, false>(name, std::forward<T>(data), std::move(attrs)); }

		template<typename T, typename... Args>
		bool AddDynamicFieldWithAttr(Type type, Name name, AttrSet attrs, Args&&... args)
		{ return AddField(type, name, { GenerateDynamicFieldPtr<T>(std::forward<Args>(args)...), std::move(attrs) }); }

		template<typename T, typename... Args>
		bool SimpleAddDynamicFieldWithAttr(Type type, Name name, AttrSet attrs, Args&&... args)
		{ return AddField(type, name, { GenerateDynamicFieldPtr<T, false>(std::forward<Args>(args)...), std::move(attrs) }); }

		template<typename T, typename... Args>
		bool AddDynamicField(Type type, Name name, Args&&... args)
		{ return AddDynamicFieldWithAttr<T>(type, name, {}, std::forward<Args>(args)...); }

		template<typename T, typename... Args>
		bool SimpleAddDynamicField(Type type, Name name, Args&&... args)
		{ return SimpleAddDynamicFieldWithAttr<T>(type, name, {}, std::forward<Args>(args)...); }

		// funcptr is member function pointer
		// get TypeID from funcptr
		template<auto member_func_ptr>
		bool AddMethod(Name name, AttrSet attrs = {});

		// funcptr is function pointer
		template<auto func_ptr>
		bool AddMethod(Type type, Name name, AttrSet attrs = {});

		template<typename T, typename... Args>
		bool AddConstructor(AttrSet attrs = {});
		template<typename T>
		bool AddDestructor(AttrSet attrs = {});

		// Func: Ret(const? Object&, Args...)
		template<typename Func>
		bool AddMemberMethod(Name name, Func&& func, AttrSet attrs = {});

		// Func: Ret(Args...)
		template<typename Func>
		bool AddStaticMethod(Type type, Name name, Func&& func, AttrSet attrs = {})
		{ return AddMethod(type, name, { GenerateStaticMethodPtr(std::forward<Func>(func)), std::move(attrs) }); }

		template<typename Derived, typename... Bases>
		bool AddBases();

		//
		// Cast
		/////////

		ObjectView StaticCast_DerivedToBase (ObjectView obj, Type type) const;
		ObjectView StaticCast_BaseToDerived (ObjectView obj, Type type) const;
		ObjectView DynamicCast_BaseToDerived(ObjectView obj, Type type) const;
		ObjectView StaticCast               (ObjectView obj, Type type) const;
		ObjectView DynamicCast              (ObjectView obj, Type type) const;

		//
		// Var
		////////
		//
		// - result type of Var maintains the CVRefMode of the input
		//

		ObjectView Var(ObjectView obj,            Name field_name, FieldFlag flag = FieldFlag::All) const;
		// for diamond inheritance
		ObjectView Var(ObjectView obj, Type base, Name field_name, FieldFlag flag = FieldFlag::All) const;

		//
		// Invoke
		///////////
		//
		// - 'B' means basic
		// - 'M' means memory
		// - auto search methods in bases
		// - support overload
		// - require IsCompatible()
		// - MInvoke will allocate buffer for result, and move to SharedObject
		// - if result is a reference, SharedObject is a ObjectView actually
		// - if result is ObjectView or SharedObject, then MInvoke's result is it.
		// - temp_args_rsrc is used for temporary allocation of arguments (release before return)
		//

		// parameter <- argument
		// - ObjectView
		// - same
		// - reference + static_cast derived -> base
		// > - 0 (invalid), 1 (convertible), 2 (constructible)
		// > - table
		//     |    -     | T | T & | const T & | T&& | const T&& |
		//     |      T   | - |  2  |     2     |  1  |     2     |
		//     |      T & | 0 |  -  |     0     |  0  |     0     |
		//     |const T & | 1 |  1  |     -     |  1  |     1     |
		//     |      T&& | 1 |  0  |     0     |  -  |     0     |
		//     |const T&& | 1 |  0  |     0     |  1  |     -     |
		// - pointer and array (non cvref)
		// > - 0 (invalid), 1 (convertible)
		// > - table
		//     |     -     | T * | const T * | T[] | const T[] |
		//     |       T * |  -  |     0     |  1  |     0     |
		//     | const T * |  1  |     -     |  1  |     1     |
		//     |       T[] |  1  |     0     | -/1 |     0     |
		//     | const T[] |  1  |     1     |  1  |    -/1    |
		bool IsCompatible(std::span<const Type> paramTypes, std::span<const Type> argTypes) const;

		Type IsInvocable(Type type, Name method_name, std::span<const Type> argTypes = {}, MethodFlag flag = MethodFlag::All) const;

		Type BInvoke(
			ObjectView obj,
			Name method_name,
			void* result_buffer = nullptr,
			ArgsView args = {},
			MethodFlag flag = MethodFlag::All,
			std::pmr::memory_resource* temp_args_rsrc = ReflMngr_GetTemporaryResource()) const;

		SharedObject MInvoke(
			ObjectView obj,
			Name method_name,
			std::pmr::memory_resource* rst_rsrc,
			ArgsView args = {},
			MethodFlag flag = MethodFlag::All,
			std::pmr::memory_resource* temp_args_rsrc = ReflMngr_GetTemporaryResource()) const;

		SharedObject Invoke(
			ObjectView obj,
			Name method_name,
			ArgsView args = {},
			MethodFlag flag = MethodFlag::All,
			std::pmr::memory_resource* temp_args_rsrc = ReflMngr_GetTemporaryResource()) const
		{ return MInvoke(obj, method_name, object_resource.get(), args, flag, temp_args_rsrc); }

		// -- template --

		template<typename... Args>
		Type IsInvocable(Type type, Name method_name, MethodFlag flag = MethodFlag::All) const;

		template<typename T>
		T Invoke(
			ObjectView obj,
			Name method_name,
			ArgsView args = {},
			MethodFlag flag = MethodFlag::All,
			std::pmr::memory_resource* temp_args_rsrc = ReflMngr_GetTemporaryResource()) const;

		//
		// Make
		/////////
		//
		// - if the type doesn't contains any ctor, then we use trivial ctor (do nothing)
		// - if the type doesn't contains any dtor, then we use trivial dtor (do nothing)
		//

		bool IsConstructible    (Type type, std::span<const Type> argTypes = {}) const;
		bool IsCopyConstructible(Type type) const;
		bool IsMoveConstructible(Type type) const;
		bool IsDestructible     (Type type) const;

		bool Construct(ObjectView obj, ArgsView args = {}) const;
		bool Destruct (ObjectView obj) const;

		ObjectView   MNew       (Type      type, std::pmr::memory_resource* rsrc, ArgsView args = {}) const;
		SharedObject MMakeShared(Type      type, std::pmr::memory_resource* rsrc, ArgsView args = {}) const;
		bool         MDelete    (ObjectView obj, std::pmr::memory_resource* rsrc                    ) const;

		ObjectView   New       (Type      type, ArgsView args = {}) const;
		SharedObject MakeShared(Type      type, ArgsView args = {}) const;
		bool         Delete    (ObjectView obj                    ) const;

		// -- template --

		template<typename... Args> bool IsConstructible(Type type) const;

	private:
		ReflMngr();
		~ReflMngr();

		// for
		// - argument copy
		// - user argument buffer
		std::shared_ptr<std::pmr::memory_resource> temporary_resource;

		// for
		// - New/MakeShared
		std::shared_ptr<std::pmr::memory_resource> object_resource;
	};

	inline static ReflMngr& Mngr = ReflMngr::Instance();
	inline static const ObjectView MngrView = { Type_of<ReflMngr>, &ReflMngr::Instance() };
}

#include "details/ReflMngr.inl"
