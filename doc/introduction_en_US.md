# Introduction

> This document is machine translated.

The core type of the UDRefl library is `ReflMngr`, which manages all the data and provides a large number of interfaces.

`ObjectView` and `SharedObject` are types that users can easily use.

The use of dynamic library is mainly divided into two steps: registration, use.

## 0. Terminology

- Field: A member object of a type

- Basic Field `FieldFlag::Basic` : a non-static member object of type.

- Virtual Field `FieldFlag::Virtual`: a non-static member object of a type that contains a virtual base class or a reference member object of a type.

- Static Field `FieldFlag::Static`: a static member object of type.

- Dynamic Shared Field `FieldFlag::DynamicShared`: a dynamic member object of type that can be shared.

- Dynamic Buffer Field `FieldFlag::DynamicBuffer`: a dynamic member object of type that  is non-shared, can only hold a few simple types.

- Owned Field `FieldFlag::Owned`: Basic Field, Virtual Field.

- Unowned Field `FieldFlag::Unowned`: Static Field, Dynamic Shared Field, Dynamic Buffer Field.

- Variable: a member object of an object.

- Field Pointer `FieldPtr`: a class used to get a pointer to a variable.

- Method: a member method of an object.

- Method Pointer `MethodPtr`: a class used to call a member function.

- Variable Method `MethodFlag::Variable`: a non-constant and non-static member method.

- Constant Method `MethodFlag::Const`: a constant member method.

- Static Method `MethodFlag::Static`: a static member method.

- Type `Type`: type name `std::string_view` + hash code, can perform type calculation

- Name `Name`: string view `std::string_view` + hash code

- Object View `ObjectView`: object pointer `void*` + type `Type` 

  > See the [UTemplate/Type.h](https://github.com/Ubpa/UTemplate/blob/master/include/UTemplate/Type.h) for details on `Type`.

- Shared Object `SharedObject`: a derived class of object view `ObjectView`, contains a `std::shared_ptr<void>` to store the object.

- Attribute `Attr`: additional information about fields, methods or type, which is a shared object

## 1. Registration

What we need to register is the class information `TypeInfo`, it contains

- size of type
- alignment of type
- is polymorphic
- field info map
- method info multi-map
- base info map
- attribute set

Details on how to register various information will be provided below.

### 1.1 Type Registration

Type registration can be done using the following interface

```c++
Type ReflMngr::RegisterType(Type type, size_t size, size_t alignment)
```

Template functions are also provided

```c++
template<typename T>
void ReflMngr::RegisterType();
```

which automatically gets the type name, size and alignment, and automatically registers some related types and functions (constructor, destructor, assignment, standard container-related interfaces, etc.) via `template<T> void details::TypeAutoRegister<T>::run()`, which is very convenient.

### 1.2 Field Registration

The key of field is the field pointer. Through it and the corresponding object, you can get the pointer to its domain. In total, there are 5 categories:

- :star: Forward offset value: `size_t` type, which offsets the object pointer to a variable corresponding to the underlying field
- Offset function: `Offsetor = std::function<void*(void*)>`, for types with virtual base classes, corresponding to virtual fields
- Static pointer: `void*` for static members, corresponding to static fields
- Dynamic shared pointer: `std::shared_ptr<void>`, for the shared dynamic member, corresponding to the dynamic shared field
- Dynamic Buffer: `FieldPtr::Buffer = std::aligned_storage_t<64>`, stored in the dynamic member of the Buffer, corresponding to the dynamic Buffer field

For each case, `FieldPtr` has a corresponding constructor, as follows

```c++
FieldPtr::FieldPtr(); // default constructor
FieldPtr::FieldPtr(Type type, size_t forward_offset_value); // forward offset
FieldPtr::FieldPtr(Type type, Offsetor offsetor); // offsetor
FieldPtr::FieldPtr(Type type, void* ptr); // static pointer
FieldPtr::FieldPtr(ObjectView static_obj); // static pointer
FieldPtr::FieldPtr(SharedObject obj) noexcept; // dynamic shared pointer
FieldPtr::FieldPtr(Type type, const Buffer& buffer); // dynamic buffer
```

After the field pointer is generated, field information `FieldInfo`（`FieldPtr` + `AttrSet`） can be constructed.

In addition, we also provide some convenient template functions to generate `FieldPtr`, which automatically recognizes base fields, virtual fields, static fields, and enumerations, and registers the type of the field with `ReflMngr::registerType`.

```c++
template<auto field_data>
FieldPtr ReflMngr::GenerateFieldPtr();

template<typename T>
FieldPtr ReflMngr::GenerateFieldPtr(T&& data);
```

> **Example 1** 
>
> The following types are available
>
> ```c++
> struct Data {
>  float x;
>  static float y;
>  float& z;
> }
> enum class Color { Red };
> ```
>
> The way to generate a field pointer is as follows
>
> ```c++
> auto fieldptr_x = Mngr->GenerateFieldPtr<&Data::x>();
> auto fieldptr_y = Mngr->GenerateFieldPtr<&Data::y>();
> auto fieldptr_z = Mngr->GenerateFieldPtr([](Data* data){ return &data.z; });
> auto fieldptr_Red = Mngr->GenerateFieldPtr<Color::Red>();
> ```

For dynamic field, can be used

```c++
template<typename T, typename... Args>
FieldPtr ReflMngr::GenerateDynamicFieldPtr(Args&&... args);
```

If `T` meets `FieldPtr::isBufferable<T>()`, the dynamic buffer field pointer is generated, otherwise the dynamic shared field pointer is generated.

Alternatively, you can register the field directly, using the following interface

```c++
template<auto field_data>
bool ReflMngr::AddField(Name name, AttrSet attrs = {});
template<typename T>
bool ReflMngr::AddField(Type type, Name name, T&& data, AttrSet attrs = {})
template<typename T, typename... Args>
bool AddDynamicField(Type type, Name name, Args&&... args);
```

> **Example 2** 
>
> For the types in Example 1, you can register the field as follows
>
> ```c++
> Mngr->RegisterType<Data>();
> Mngr->AddField<&Data::x>("x");
> Mngr->AddField<&Data::y>(Type_of<Data>, "y");
> Mngr->AddField(Type_of<Data>, "z", [](Data* data)->float&{ return data->z; });
> 
> Mngr->RegisterType<Color>();
> Mngr->AddField<Color::Red>();
> ```

### 1.3 Method Registration

The design of the interface related to the registration method is similar to that of the registration field, which is divided into two categories: generating method Pointers and registering method information.

The following interface is used to generate the method pointer

```c++
template<auto funcptr>
static MethodPtr ReflMngr::GenerateMethodPtr();

// Func: Ret(const? Object&, Args...)
template<typename Func>
static MethodPtr ReflMngr::GenerateMemberMethodPtr(Func&& func);

// Func: Ret(Args...)
template<typename Func>
static MethodPtr ReflMngr::GenerateStaticMethodPtr(Func&& func);
```

For constructors, you can use the following interface

```c++
// void(T&, Args...)
template<typename T, typename... Args>
static MethodPtr ReflMngr::GenerateConstructorPtr();
```

> **Example 3** 
>
> ```c++
> struct Funcs {
>  void foo();
>  void bar();
>  void bar() const;
>  static void cat();
> }
> ```
>
> A method pointer to `Funcs::foo` can be generated as follows
>
> ```c++
> ReflMngr::GenerateMethodPtr<&Funcs::foo>();
> ```
>
> For the overloaded function `Funcs::bar`, you need to use `MemFuncOf<...> `to solve the overload problem
>
> ```c++
> ReflMngr::GenerateMethodPtr<MemFuncOf<Funcs, void()>>::get(&Funcs::bar)>();
> ReflMngr::GenerateMethodPtr<MemFuncOf<Funcs, void()const>>::get(&Funcs::bar)>();
> ```

Once you have a method pointer, you can register it with the type information using the following interface

```c++
Name ReflMngr::AddMethod(Type type, Name method_name, MethodInfo methodinfo);
```

Alternatively, you can register the field directly, using the following interface

```c++
template<auto member_func_ptr>
bool ReflMngr::AddMethod(Name name, AttrSet attrs = {});
template<auto func_ptr>
bool ReflMngr::AddMethod(Type type, Name name, AttrSet attrs = {});
template<typename T, typename... Args>
bool ReflMngr::AddConstructor(AttrSet attrs = {});
// Func: Ret(const? Object&, Args...)
template<typename Func>
bool ReflMngr::AddMemberMethod(Name name, Func&& func, AttrSet attrs = {});
// Func: Ret(Args...)
template<typename Func>
bool ReflMngr::AddStaticMethod(Type type, Name name, Func&& func, AttrSet attrs = {});
template<typename T, typename... Args>
bool ReflMngr::AddConstructor(AttrSet attrs = {});
template<typename T>
bool ReflMngr::AddDestructor(AttrSet attrs = {});
```

> **Example 4** 
>
> For the types of Example 3, you can register methods as follows
>
> ```c++
> Mngr->RegisterType<Funcs>();
> Mngr->AddMethod<&Funcs::foo>("foo");
> Mngr->AddMethod<MemFuncOf<Funcs, void()>>::get(&Funcs::bar)>("bar");
> Mngr->AddMethod<MemFuncOf<Funcs, void()const>>::get(&Funcs::bar)>("bar");
> Mngr->AddMethod<&Funcs::cat>(Type_of<Funcs>, "cat");
> ```
>

### 1.4 Base Registration

The base class information `BaseInfo` consists of three cast functions and two bool

- `Offsetor static_derived_to_base` 
- `Offsetor static_base_to_derived` 
- `Offsetor dynamic_base_to_derived` 
- `bool is_polymorphic` 
- `bool is_virtual` 

Requires when constructing `BaseInfo`

- You must provide `Offsetor static_derived_to_base` 
- If it is a virtual base class, you must not provide `Offsetor static_base_to_derived`; Otherwise you must provide it.
- If it is polymorphic, you must provide  `Offsetor dynamic_base_to_derived`; Otherwise you must provide it.

We support basic inheritance, multiple inheritance, virtual inheritance, you can use the following interface to generate base class information.

```c++
template<typename Derived, typename Base>
static BaseInfo ReflMngr::GenerateBaseInfo();
```

The base class is automatically recognized internally.

You can then register it.

```c++
Type ReflMngr::AddBase(Type derived, Type base, BaseInfo baseinfo);
```

In addition, we also provide direct registration interface

```c++
template<typename Derived, typename... Bases>
bool ReflMngr::AddBases();
```

> Example 5
>
> ```c++
> struct A{};
> struct B : virtual A{};
> struct C : virtual A{};
> struct D : B, C {};
> 
> Mngr->RegisterType<A>();
> Mngr->RegisterType<B>();
> Mngr->RegisterType<C>();
> Mngr->RegisterType<D>();
> Mngr->AddBases<B, A>();
> Mngr->AddBases<C, A>();
> Mngr->AddBases<D, B, C>();
> ```

### 1.5 Attribute Registration

Attribute `Attr` is a `SharedObject` and can be created

```c++
SharedObject ReflMngr::MakeShared(
  Type type,
  std::span<const Type> argTypes = {},
  ArgPtrBuffer argptr_buffer = nullptr) const;

template<typename... Args>
SharedObject ReflMngr::MakeShared(Type type, Args&&... args) const;

template<typename T, typename... Args>
SharedObject ReflMngr::MakeSharedAuto(Args... args);
```

Field information `FieldInfo`, method information `MethodInfo`, and class information `TypeInfo` can all be indexed by `Type` with attributes attached.

Usually the interface that registers the above information can attach a property set.

You can also add properties with the following interface

```c++
bool AddTypeAttr(Type type, Attr attr);
bool AddFieldAttr(Type type, Name field_name, Attr attr);
bool AddMethodAttr(Type type, Name method_name, Attr attr);
```

## 2. Use

It can be used in two categories

- `ReflMngr` : Provides a large number of functional interfaces
- `ObjectView` : Contains various interfaces of `ReflMngr` for easy use

### 2.1 Use `ReflMngr` 

There are three main aspects to using `ReflMngr`

- var: Gets the variable of the object
- invoke: Call the function of the object
- algorithm: {traverse/aggregate/find} {base classes/methods/fields/variables}

#### 2.1.1 Var

There are two interfaces for getting variables

```c++
ObjectView ReflMngr::Var(ObjectView obj, Name field_name, FieldFlag flag = FieldFlag::All) const;

// for diamond inheritance
ObjectView ReflMngr::Var(ObjectView obj, Type base, Name field_name, FieldFlag flag = FieldFlag::All) const;
```

Note that the Type `Type` of the result will change according to the Type `Type` of `obj`. For example, if `obj` is an rvalue reference, the Type of the result will change to the corresponding rvalue reference Type. If `obj` is `const`, the result type will also be `const`

#### 2.1.2 Invoke

This is a little bit more complicated.

You can check if the function was called successfully before calling it first

```c++
Type ReflMngr::IsInvocable(
  Type type,
  Name method_name,
  std::span<const Type> argTypes = {},
  MethodFlag flag = MethodFlag::All,
  std::pmr::memory_resource* temp_args_rsrc = Mngr->GetTemporaryResource()) const;
```

If the return value `Type` is valid, then the call succeeds, and it is the return value Type of the function. You can use this `Type` to query the size and alignment of the Type to create the return value buffer needed to call the function.

The function is called as follows

```c++
Type ReflMngr::BInvoke(
  ObjectView obj,
  Name method_name,
  void* result_buffer = nullptr,
  ArgsView args = {},
  MethodFlag flag = MethodFlag::All,
  std::pmr::memory_resource* temp_args_rsrc = Mngr->GetTemporaryResource()) const;
```

Where `argptrBuffer = void* const*` consisting of a parameter pointer buffer and a type array.

`BInvoke` searches for all functions of the same name for this class and its base class, and automatically converts arguments (parameter types automatically change and even constructs temporary objects, similar to C++).When temporary parameter objects are constructed, dynamic memory is allocated, and the `temp_args_rsrc` of this interface is used for this purpose. The default is to use a `std::pmr::synchronized_pool_resource`, which is locked. In the case of multiple threads, an unlocked memory_resource can be used per thread to increase efficiency.

This interface is the most basic, hence the name **B**Invoke, where `B` means BASIC.

If the call fails, the return value `Type` is invalid.

To simplify the construction of return value buffers, the following interfaces are provided

```c++
SharedObject ReflMngr::MInvoke(
  ObjectView obj,
  Name method_name,
  std::pmr::memory_resource* rst_rsrc,
  ArgsView args = {},
  MethodFlag flag = MethodFlag::All,
  std::pmr::memory_resource* temp_args_rsrc = Mngr->GetTemporaryResource()) const;
```

Just provide a `std::pmr::memory_resource* rst_rsrc` and the return buffer will be automatically allocated according to the return value type and released by the return value `sharedObject`.

In the following special cases, no additional return value buffers need to be allocated

- is the return value type ` void ` / reference type / ` ObjectView `, the return value ` SharedObject ` will degenerate into a ` ObjectView ` (` SharedObject: : IsObjecView () = = true `)
- The return value type is` sharedObject `, which returns it directly

`ReflMngr` has two built-in resources for return buffer construction and temporary parameter object construction by default, so it also provides a minimalist interface

```c++
SharedObject Invoke(
  ObjectView obj,
  Name method_name,
  ArgsView args = {},
  MethodFlag flag = MethodFlag::All,
  std::pmr::memory_resource* temp_args_rsrc = Mngr->GetTemporaryResource()) const;
```

This interface uses `ReflMngr`'s built-in resources by default

To simplify parameter construction, a temporary arguments view `template<size_t N> class TempArgsView` is provided.

> Example
>
> ```c++
> ReflMngr::Invoke<T>(obj, method_name, TempArgsView{ std::forward<Args>(args)... });
> ```

It also provides additional functions on type construction and destructing. See the Make section of the `ReflMngr` interface, which is just a brief introduction to the following

```c++
SharedObject ReflMngr::MakeShared(Type type, ArgsView args = {}) const;
```

Internally, the corresponding constructor of the type is called.

#### 2.1.3 Algorithm

The first is the ergodic method

```c++
void ReflMngr::ForEachTypeInfo(Type type,
  const std::function<bool(InfoTypePair)>& func) const;

void ReflMngr::ForEachField(Type type,
  const std::function<bool(InfoTypePair, InfoFieldPair)>& func,
  FieldFlag flag = FieldFlag::All) const;

void ReflMngr::ForEachMethod(Type type,
  const std::function<bool(InfoTypePair, InfoMethodPair)>& func,
  MethodFlag flag = MethodFlag::All) const;

void ReflMngr::ForEachVar(ObjectView obj,
  const std::function<bool(InfoTypePair, InfoFieldPair, ObjectView)>& func,
  FieldFlag flag = FieldFlag::All) const;
```

Each interface accepts a `std::::function`, which is called for each item in the traversal and returns a value of `bool` to control the traversal, which continues when `true` and stops when `false`.You can also use `flag` to indicate the traversal range.

Some simple interfaces are provided based on traversal algorithms, including `Find`, `Get`, and `Contains`. For details, see the Algorithm section of the source code `ReflMngr`.

### 2.2 Use `ObjectView` 

The relationship between `objectView` and `sharedObject` is like that between `std::::string_view` and `std::::string`. The former does not manage resources, so you need to ensure that `sharedObject` has a longer lifetime than `objectView`.

`ObjectView` contains four classes of interfaces

- their own interface: ` GetType `, ` GetPtr() `, ` AsPtr<T>() `, ` As<T>()`, etc

- `ReflMngr` Interface: Similar to `ReflMngr`, some interfaces can be simplified here

  > **Example** 
  >
  > ```c++
  > Mngr->Invoke(v, "norm");
  > // v.s.
  > v.Invoke("norm");
  > ```

- Meta interfaces: Overloaded a large number of Operators and container-related interfaces to simplify the use of some interfaces

  > **Example** 
  >
  > operator
  >
  > ```c++
  > Mngr->Invoke(v, NameIDRegistry::Meta::operator_add, v);
  > // v.s.
  > v + v; // call overloaded ObjectView::operator+;
  > ```
  >
  > container
  >
  > ```c++
  > SharedObject begin_iter = v.Invoke(NameIDRegistry::Meta::container_begin);
  > SharedObject end_iter = v.Invoke(NameIDRegistry::Meta::container_end);
  > for(auto iter = begin_iter; iter != end_iter; ++iter) { /*...*/ }
  > // v.s.
  > for(auto elem : v) { /*...*/ }
  > ```

- Type operation: Similar to the `ReflMngr::tRegistry` interface, this can simplify the use of some interfaces

  > **Example** 
  >
  > ```c++
  > ObjectView const_v{ Mngr->tregistry.RegisterAddConst(v.GetType()), v.GetPtr() };
  > // v.s.
  > ObjectView const_v = v.AddConst();
  > ```

## 3. Summary

This is the end of the general introduction of the core ideas of the library, then you can read the home page of various examples.Also read [include/UDRefl](../include/UDRefl) for more details.

> TODO: manual

