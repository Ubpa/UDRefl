# 入门

UDRefl 库的核心类型为 `ReflMngr`，它管理了所有数据，并提供了大量的接口。

而 `ObjectView` 和 `SharedObject` 则是用户可以轻松使用的类型。

运用动态库主要分为两个步骤：注册、使用。

## 0. 术语

- 域：类型的成员对象

- 基本域 `FieldFlag::Basic` ：类型的非静态成员对象

- 虚拟域 `FieldFlag::Virtual`：含虚基类的类型的非静态成员对象或类型的引用成员对象

- 静态域 `FieldFlag::Static`：类型的静态成员对象

- 动态共享域 `FieldFlag::DynamicShared`：类型的动态成员对象，可共享

- 动态缓冲域 `FieldFlag::DynamicBuffer`：类型的动态成员对象，非共享，只能存一些简单类型

- 自有域 `FieldFlag::Owned`：基础域，虚拟域

- 无主域 `FieldFlag::Unowned`：静态域，动态共享域，动态缓冲域

- 变量：对象的成员对象

- 域指针 `FieldPtr`：用于获取变量指针的类

- 方法：类型的成员函数

- 方法指针 `MethodPtr`：用于调用成员函数的类

- 变量方法 `MethodFlag::Variable`：非 constant 非 static 的成员函数

- 常量方法 `MethodFlag::Const`：constant 的成员函数

- 静态方法 `MethodFlag::Static`：static 的成员函数

- 类型 `Type`：类型名 `std::string_view` + 哈希值，可进行类型计算

- 名字 `Name`：字符视图 `std::string_view` + 哈希值

- 对象视图 `ObjectView`：对象指针 `void*` + 类型 `Type` 

  > `Type` 详细请查看 [UTemplate/Type.h](https://github.com/Ubpa/UTemplate/blob/master/include/UTemplate/Type.h) 
  >
  > 若想了解其中用到的技术，可阅读 [C++ 生成式的编译期类型名](https://zhuanlan.zhihu.com/p/336243278) 

- 共享对象 `SharedObject`：对象视图 `ObjectView` 的子类，多了 `std::shared_ptr<void>` 来存储对象

- 属性 `Attr`：域，方法或类的附加信息，它是一个共享对象

## 1. 注册

我们需要注册的是类信息 `TypeInfo`，其包含

- 类型大小
- 类型对齐
- 是否多态
- 域信息 map
- 方法信息 multi-map
- 基类信息 map
- 属性集

下边会详细介绍如何注册各种信息

### 1.1 注册类型

类型注册可使用如下接口

```c++
Type ReflMngr::RegisterType(Type type, size_t size, size_t alignment)
```

另外提供了模板函数

```c++
template<typename T>
void ReflMngr::RegisterType();
```

其中自动获取了类型的名字，大小和对齐，并且通过 `template<T> void details::TypeAutoRegister<T>::run()` 自动注册了一些相关的类型与函数（构造，析构，赋值，标准容器相关接口等等），非常方便。

### 1.2 注册域

域关键就是域指针。通过它和相应对象，可以得到其域的指针。总共分为 5 类：

- :star:前向偏移量：`size_t` 类型，通过偏移对象指针得到变量指针，对应基础域
- 偏移函数：`Offsetor = std::function<void*(void*)>`，用于含虚基类的类型，对应虚拟域
- 静态指针：`void*`，用于静态成员，对应静态域
- 动态共享指针：`std::shared_ptr<void>`，用于共享的动态成员，对应动态共享域
- 动态缓冲区：`FieldPtr::Buffer = std::aligned_storage_t<64>`，存于缓冲区的动态成员，对应动态缓冲域

对于每一种类型，`FieldPtr` 都有相应的构造函数，如下

```c++
FieldPtr::FieldPtr(); // 默认构造函数
FieldPtr::FieldPtr(Type type, size_t forward_offset_value); // 前向偏移量
FieldPtr::FieldPtr(Type type, Offsetor offsetor); // 偏移函数
FieldPtr::FieldPtr(Type type, void* ptr); // 静态指针
FieldPtr::FieldPtr(ObjectView static_obj); // 静态指针
FieldPtr::FieldPtr(SharedObject obj) noexcept; // 动态共享指针
FieldPtr::FieldPtr(Type type, const Buffer& buffer); // 动态缓冲区
```

生成域指针后，可以构造域信息 `FieldInfo`（`FieldPtr` + `AttrSet`）

另外，我们还提供一些方便的模板函数用来生成 `FieldPtr`，它可以自动识别基本域，虚拟域，静态域和枚举，并且使用 `ReflMngr::RegisterType` 注册域的类型

```c++
template<auto field_data>
FieldPtr ReflMngr::GenerateFieldPtr();

template<typename T>
FieldPtr ReflMngr::GenerateFieldPtr(T&& data);
```

> **示例 1** 
>
> 现有如下类型
>
> ```c++
> struct Data {
>     float x;
>     static float y;
>     float& z;
> }
> enum class Color { Red };
> ```
>
> 生成域指针的方法如下
>
> ```c++
> auto fieldptr_x = Mngr->GenerateFieldPtr<&Data::x>();
> auto fieldptr_y = Mngr->GenerateFieldPtr<&Data::y>();
> auto fieldptr_z = Mngr->GenerateFieldPtr([](Data* data){ return &data.z; });
> auto fieldptr_Red = Mngr->GenerateFieldPtr<Color::Red>();
> ```

对于动态域，可使用

```c++
template<typename T, typename... Args>
FieldPtr ReflMngr::GenerateDynamicFieldPtr(Args&&... args);
```

如果 `T` 满足 `FieldPtr::IsBufferable<T>()`，则生成动态缓冲域指针，否则生成动态共享域指针

另外，还可以直接注册域，使用如下接口

```c++
template<auto field_data>
bool ReflMngr::AddField(Name name, AttrSet attrs = {});
template<typename T>
bool ReflMngr::AddField(Type type, Name name, T&& data, AttrSet attrs = {})
template<typename T, typename... Args>
bool AddDynamicField(Type type, Name name, Args&&... args);
```

> **示例 2** 
>
> 对于示例 1 里的类型，可以按如下方式注册域
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

### 1.3 注册方法

注册方法相关接口的设计与注册域类似，分两类：生成方法指针，注册方法信息。

生成方法指针使用如下接口

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

对于构造函数，可使用如下接口

```c++
// void(T&, Args...)
template<typename T, typename... Args>
static MethodPtr ReflMngr::GenerateConstructorPtr();
```

> **示例 3** 
>
> ```c++
> struct Funcs {
>     void foo();
>     void bar();
>     void bar() const;
>     static void cat();
> }
> ```
>
> 可以按如下方式生成 `Funcs::foo` 的方法指针
>
> ```c++
> ReflMngr::GenerateMethodPtr<&Funcs::foo>();
> ```
>
> 对于重载函数 `Funcs::bar`，则需要使用 `MemFuncOf<...>` 来解决重载问题
>
> ```c++
> ReflMngr::GenerateMethodPtr<MemFuncOf<Funcs, void()>>::get(&Funcs::bar)>();
> ReflMngr::GenerateMethodPtr<MemFuncOf<Funcs, void()const>>::get(&Funcs::bar)>();
> ```

得到方法指针后，可以用如下接口注册到类型信息中

```c++
Name ReflMngr::AddMethod(Type type, Name method_name, MethodInfo methodinfo);
```

另外，还可以直接注册域，使用如下接口

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

> **示例 4** 
>
> 对于示例 3 的类型，可以按如下方式注册方法
>
> ```c++
> Mngr->RegisterType<Funcs>();
> Mngr->AddMethod<&Funcs::foo>("foo");
> Mngr->AddMethod<MemFuncOf<Funcs, void()>>::get(&Funcs::bar)>("bar");
> Mngr->AddMethod<MemFuncOf<Funcs, void()const>>::get(&Funcs::bar)>("bar");
> Mngr->AddMethod<&Funcs::cat>(Type_of<Funcs>, "cat");
> ```
>

### 1.4 注册基类

基类信息 `BaseInfo` 由 3 个 cast 函数和两个 bool 组成

- `Offsetor static_derived_to_base` 
- `Offsetor static_base_to_derived` 
- `Offsetor dynamic_base_to_derived` 
- `bool is_polymorphic` 
- `bool is_virtual` 

在构造 `BaseInfo` 时，要求

- 一定要有 `Offsetor static_derived_to_base` 
- 若为虚基类，则没有 `Offsetor static_base_to_derived`；否则须有它
- 若为基类为多态的，则须有 `Offsetor dynamic_base_to_derived`；否则须无它

我们支持基本继承，多继承，虚继承，可以使用如下接口生成基类信息

```c++
template<typename Derived, typename Base>
static BaseInfo ReflMngr::GenerateBaseInfo();
```

内部会自动识别该基类

接着可以将其注册

```c++
Type ReflMngr::AddBase(Type derived, Type base, BaseInfo baseinfo);
```

另外我们也提供了直接注册的接口

```c++
template<typename Derived, typename... Bases>
bool ReflMngr::AddBases();
```

> 示例 5
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

### 1.5 注册属性

属性 `Attr` 就是共享对象 `SharedObject`，创建属性可用

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

域信息 `FieldInfo`，方法信息 `MethodInfo` 和类信息 `TypeInfo` 都可以附加一些属性，按 `Type` 进行索引。

一般注册上述信息的接口里都能附加上属性集。

另外还可用如下接口添加属性

```c++
bool AddTypeAttr(Type type, Attr attr);
bool AddFieldAttr(Type type, Name field_name, Attr attr);
bool AddMethodAttr(Type type, Name method_name, Attr attr);
```

## 2. 使用

使用方式主要分 2 类

- `ReflMngr`：提供大量的函数接口
- `ObjectView`：包含 `ReflMngr` 的各种接口，方便使用

### 2.1 使用 ReflMngr

使用 ReflMngr 主要分 3 个方面

- Var：获取对象的变量
- Invoke：调用对象的函数
- Algorithm：{遍历/聚集/查找} {对象/类型} 的 {基类/方法/域/变量}

#### 2.1.1 Var

获取变量有两个接口

```c++
ObjectView ReflMngr::Var(ObjectView obj, Name field_name, FieldFlag flag = FieldFlag::All) const;

// for diamond inheritance
ObjectView ReflMngr::Var(ObjectView obj, Type base, Name field_name, FieldFlag flag = FieldFlag::All) const;
```

注意，结果的类型 `Type` 会根据 `obj` 的类型 `Type` 而变，比如 `obj` 是右值引用，则结果的类型也变成相应的右值引用类型；`obj` 是 `const`，则结果的类型也会是 `const` 

#### 2.1.2 Invoke

这一部分稍微有点复杂。

首先调用函数前可以先检测函数是否能调用成功

```c++
Type ReflMngr::IsInvocable(
  Type type,
  Name method_name,
  std::span<const Type> argTypes = {},
  MethodFlag flag = MethodFlag::All) const;
```

如果返回值 `Type` 是有效的，则可调用成功，并且其即为该函数的返回值类型，你可以利用该 `Type` 去查询类型的大小和对齐，用于创建调用函数所需的返回值缓冲区。

调用函数方式如下

```c++
Type ReflMngr::BInvoke(
  ObjectView obj,
  Name method_name,
  void* result_buffer = nullptr,
  ArgsView args = {},
  MethodFlag flag = MethodFlag::All,
  std::pmr::memory_resource* temp_args_rsrc = Mngr->GetTemporaryResource()) const;
```

其中 `ArgsView args` 由参数指针缓冲区和类型数组组成。

`BInvoke` 会搜寻该类及其基类的所有同名函数，并且自动进行参数转换（参数类型自动改变，甚至构造临时对象，与 C++ 的机制类似）。构造临时参数对象时，会进行动态内存分配，该接口的 `temp_args_rsrc` 就用于此目的。默认是使用了一个 `std::pmr::synchronized_pool_resource`，它是有锁的。在多线程的情形下，可以每个线程使用一个无锁的 memory_resource 来提高效率。

这个接口是最基础的，因此取名为 **B**Invoke，`B` 是 basic 的意思。

如果调用失败，返回值 `Type` 是无效的。

为了简化返回值缓冲区的构造，提供了如下接口

```c++
SharedObject ReflMngr::MInvoke(
  ObjectView obj,
  Name method_name,
  std::pmr::memory_resource* rst_rsrc,
  ArgsView args = {},
  MethodFlag flag = MethodFlag::All,
  std::pmr::memory_resource* temp_args_rsrc = Mngr->GetTemporaryResource()) const;
```

只需提供一个 `std::pmr::memory_resource* rst_rsrc`，内部会根据返回值类型自动分配返回值缓冲区，并交由返回值 `SharedObject` 释放。

在以下特殊情形里，不需要额外分配返回值缓冲区

- 返回值类型是 `void`/引用类型/`ObjectView`，则返回值 `SharedObject` 会退化成 `ObjectView`（`SharedObject::IsObjecView() == true`）
- 返回值类型是 `SharedObject`，则直接返回它

`ReflMngr` 内置了两个资源默认用于返回值缓冲区构造和构造临时参数对象，因此还提供了一个最简化的接口

```c++
SharedObject Invoke(
  ObjectView obj,
  Name method_name,
  ArgsView args = {},
  MethodFlag flag = MethodFlag::All,
  std::pmr::memory_resource* temp_args_rsrc = Mngr->GetTemporaryResource()) const;
```

该接口默认使用 `ReflMngr` 的内置资源

如果已知返回值类型，还可以用下边的模板函数

```c++
template<typename T>
T Invoke(
  ObjectView obj,
  Name method_name,
  ArgsView args = {},
  MethodFlag flag = MethodFlag::All,
  std::pmr::memory_resource* temp_args_rsrc = Mngr.GetTemporaryResource()) const;
```

为了简化参数的构造，提供了临时参数视图 `template<size_t N> class TempArgsView` 

> 示例
>
> ```c++
> ReflMngr::Invoke<T>(obj, method_name, TempArgsView{ std::forward<Args>(args)... });
> ```

此外还额外提供了关于类型构造和析构的一些函数，可查看 `ReflMngr` 接口的 Make 部分，这里只简单介绍如下接口

```c++
SharedObject ReflMngr::MakeShared(Type type, ArgsView args = {}) const;
```

其内部调用了类型的相应构造函数。

#### 2.1.3 Algorithm

首先是遍历算法

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

各接口都接受一个 `std::function`，遍历中的每一个项都会调用该函数，其返回值 `bool` 用于控制遍历，其为 `true` 时继续遍历，其为 `false` 时停止遍历。另外还可以用 `flag` 来指明遍历范围。

基于遍历算法，提供了一些简易的接口，包括 `Find`，`Get` 和 `Contains`，具体可查看源码 `ReflMngr` 的 Algorithm 部分。

### 2.2 使用 ObjectView

`ObjectView` 和 `SharedObject` 的关系就像 `std::string_view` 和 `std::string` 的关系，前者不管理资源，因此你需要保证 `SharedObject` 的生命期长于 `ObjectView`。

`ObjectView` 包含 4 类接口

- 自身接口：`GetType`，`GetPtr()`，`AsPtr<T>()`，`As<T>()` 等

- ReflMngr 接口：同于 ReflMngr，放在此处可以简化一些接口的使用

  > **示例** 
  >
  > ```c++
  > Mngr->Invoke(v, "norm");
  > // v.s.
  > v.Invoke("norm");
  > ```

- Meta 接口：重载的大量 operator 以及容器相关接口，用于简化一些接口的使用

  > **示例** 
  >
  > 运算符
  >
  > ```c++
  > Mngr->Invoke(v, NameIDRegistry::Meta::operator_add, v);
  > // v.s.
  > v + v; // call overloaded ObjectView::operator+;
  > ```
  >
  > 容器
  >
  > ```c++
  > SharedObject begin_iter = v.Invoke(NameIDRegistry::Meta::container_begin);
  > SharedObject end_iter = v.Invoke(NameIDRegistry::Meta::container_end);
  > for(auto iter = begin_iter; iter != end_iter; ++iter) { /*...*/ }
  > // v.s.
  > for(auto elem : v) { /*...*/ }
  > ```

- 类型运算：同于 `ReflMngr::tregistry` 的接口，放在此处可以简化一些接口的使用

  > **示例** 
  >
  > ```c++
  > ObjectView const_v{ Mngr->tregistry.RegisterAddConst(v.GetType()), v.GetPtr() };
  > // v.s.
  > ObjectView const_v = v.AddConst();
  > ```

## 3. 总结

该库核心思路大致介绍完毕了，接下来可阅读首页的各种示例。另外还可以阅读 [include/UDRefl](../include/UDRefl) 下的各头文件了解更多细节。

> TODO: 手册

