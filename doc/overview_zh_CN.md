# 总览

反射是一个十分基础的语言功能，C++ 缺乏这块的内容。UDRefl 是动态反射库，能将 C++ 的类型几乎完美地动态表示出来，从而用于支撑需要反射的应用。此外，用户也能注册动态类型，不需要 C++ 的定义。

动态反射库主要分成三个部分

- 类型各项信息的存储：成员变量，成员函数，基类，类名，大小等
- 类型各项信息的注册
- 类型各项信息的使用：获取成员变量，调用成员函数等

## 1. 存储

### 1.1 成员变量

成员变量也称为域 `Field`，对象一般占一个连续空间，而域可以通过相对该空间的一个偏移量而取到。因此我们可以存储一个偏移量来表示域信息。对于较为复杂的域（不在连续空间上），可以使用一个偏移函数 `void*(void*)` 来表示域信息（使用 `void` 是为了类型擦除）。

> 其他种类的域
>
> - 静态域：用 `void*`（静态域就是一个固定位置的变量，所以用一个指针即可）
> - 动态共享域：这里的“动态”相对于“静态”，静态域是程序自带的，而动态域就是动态生成的，方便用于动态地添加一些类似于静态域的变量。因为使用了 `std::shared_ptr<void>`，所以称之为“共享”。
> - 动态缓冲域：该域类似于动态共享域，区别在于使用了 `std::aligned_storage_t<64>` 来存储，因为其空间位于栈上，性能更好些。因为使用了类型擦除，所以要求该类型是平凡的（trivial）。平凡的类没有特殊的构造析构函数，可以直接拷贝。

### 1.2 成员函数

成员函数可以用一个函数指针表示，为了进行类型擦除，该库使用了 `void(void* obj, void* result_buffer, ArgsView args)` 作为统一的函数签名，并用 `std::function<void(void*,void*,ArgsView)` 存储。

### 1.3 基类

一个类可以有一个或多个基类，基类一般处于对象的连续空间中，所以想要通过一个对象指针得到基类指针，可以用一个偏移量来完成。但由于存在虚基类，所以为了更加泛化，我使用了偏移函数 `void*(void*)`。

### 1.4 类

类信息由成员变量表（变量名 -> 成员变量），成员函数表（函数名 -> 成员函数），基类表（类名 -> 基类），类名，大小等组成。

## 2. 注册

注册可以纯动态的方式进行（不需要C++的类型定义）。

也可以借助 C++ 的类型和强大的模板编程能力，使得注册过程变得十分简单（比如，提供一个成员变量指针和名字就可以完成注册域）。

## 3. 使用

基本上 C++ 能怎么用，该动态反射库就能怎么用。

接口都位于 `Ubpa::UDRefl::ReflMngr` 中，并且为了方便用户，我设计了对象视图 `ObjectView`，内含一些 `ReflMngr` 的接口。因为对象视图不管理对象空间，所以称为视图。因此，我还额外设计了共享对象 `SharedObject` 来管理对象空间。

在 [README.md](../README.md) 中有提到一些接口的基本操作。

这里再提一些细节

- 成员变量的获取会搜寻继承树
- 成员函数的调用会搜寻继承树，会根据参数类型决定重载函数，调用时可能会进行参数转换（比如函数需要 `int` 类型的参数，而用户提供的参数类型为 `float`，则动态库会转换该参数）

## 4. 扩展

### 4.1 自举

我们将动态反射库（绝大部分类型）注册到了动态反射库里，完成了自举。

打开 CMake 选项 `Ubpa_UDRefl_Build_ext_Bootstrap` 即可构建相关项目。
