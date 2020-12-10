```

 __    __   _______  .______       _______  _______  __      
|  |  |  | |       \ |   _  \     |   ____||   ____||  |     
|  |  |  | |  .--.  ||  |_)  |    |  |__   |  |__   |  |     
|  |  |  | |  |  |  ||      /     |   __|  |   __|  |  |     
|  `--'  | |  '--'  ||  |\  \----.|  |____ |  |     |  `----.
 \______/  |_______/ | _| `._____||_______||__|     |_______|
                                                             

```

[![repo-size](https://img.shields.io/github/languages/code-size/Ubpa/UDRefl?style=flat)](https://github.com/Ubpa/UDRefl/archive/master.zip) [![tag](https://img.shields.io/github/v/tag/Ubpa/UDRefl)](https://github.com/Ubpa/UDRefl/tags) [![license](https://img.shields.io/github/license/Ubpa/UDRefl)](LICENSE) 

⭐ Star us on GitHub — it helps!

# UDRefl

> **U**bpa **D**ynamic **R**eflection

Extremely **fast** C++17 dynamic reflection library

## How to Use

> the example code is [here](src/test/00_readme/main.cpp) 

Suppose you need to reflect `struct Vec` 

```c++
struct Vec {
  float x;
  float y;
  float norm() const noexcept {
    return std::sqrt(x * x + y * y);
  }
};
```

### Manual registration

```c++
//
// string -> ID (size_t)
//////////////////////////

TypeID ID_Vec   = ReflMngr::Instance().tregistry.GetID<Vec>();

NameID ID_x = ReflMngr::Instance().nregistry.GetID("x");
NameID ID_y = ReflMngr::Instance().nregistry.GetID("y");

NameID ID_norm = ReflMngr::Instance().nregistry.GetID("norm");
NameID ID_ctor = ReflMngr::Instance().nregistry.GetID(NameRegistry::Meta::ctor); // meta function
NameID ID_dtor = ReflMngr::Instance().nregistry.GetID(NameRegistry::Meta::dtor); // meta function

//
// register
/////////////

ReflMngr::Instance().typeinfos[ID_Vec] = { // TypeInfo
  sizeof(Vec),  // size
  alignof(Vec), // alignment
  { // field infos
    { ID_x, { ReflMngr::Instance().GenerateFieldPtr<&Vec::x>() } }, // x
    { ID_x, { ReflMngr::Instance().GenerateFieldPtr<&Vec::y>() } }  // y
  },
  { // method infos
    {ID_ctor, {MethodPtr::GenerateDefaultConstructor<Vec>()}},                  // default ctor
    {ID_dtor, {MethodPtr::GenerateDestructor<Vec>()}},                          // dtor
    {ID_norm, {ID_norm, {ReflMngr::Instance().GenerateMethodPtr<&Vec::norm>()}} // norm
  }
};
```

### Iterate over members

```c++
ReflMngr::Instance().ForEachField(
  ID_Vec, // TypeID
  [](Type type, Field field) {
    std::cout << ReflMngr::Instance().nregistry.Nameof(field.ID) << std::endl;
  }
);

ReflMngr::Instance().ForEachMethod(
  ID_Vec, // TypeID
  [](Type type, Method method) {
    std::cout << ReflMngr::Instance().nregistry.Nameof(method.ID) << std::endl;
  }
);
```

### Constructing types

```c++
// [ or ]
// ObjectPtr v = ReflMngr::Instance().New(ID_Vec);
// // do something
// ReflMngr::Instance().Delete(v);

SharedObject v = ReflMngr::Instance().MakeShared(ID_Vec);
// do something
```

### Set/get variables

```c++
ReflMngr::Instance().RWVar(v, ID_x).As<float>() = 3.f;
ReflMngr::Instance().RWVar(v, ID_y).As<float>() = 4.f;
std::cout << "x: " << ReflMngr::Instance().RVar(v, ID_x).As<float>() << std::endl;
```

### Invoke Methods

```c++
float norm = ReflMngr::Instance().Invoke<float>(v.as_object_ptr(), ID_norm);
std::cout << "norm: " << norm << std::endl;
```

### Iterate over variables

```c++
ReflMngr::Instance().ForEachRVar(
  v, // ObjectPtr
  [](Type type, Field field, ConstObjectPtr var) {
    std::cout
      << ReflMngr::Instance().nregistry.Nameof(field.ID)
      << ": " << var.As<float>()
      << std::endl;
  }
);
```

### other example

- [const & static](src/test/02_const_static/main.cpp) 
- [method](src/test/03_method/main.cpp) 
- [enum](src/test/04_enum/main.cpp) 
- [overload](src/test/05_overload/main.cpp) 
- [inheritance](src/test/06_inheritance/main.cpp) 
- [virtual](src/test/07_virtual/main.cpp) 
- [attr](src/test/08_attr/main.cpp) 
- [lifecycle (malloc, free, ctor, dtor)](src/test/09_lifecycle/main.cpp) 

## Feature

- reflect  global fields, methods or enums
- classes with **single**-, **multiple**- and **virtual**-inheritance
- constructors (arbitrary argument count) and destructors
- methods (**virtual**, **abstract**, **overloaded**, arbitrary argument count)
- ability to invoke properties and methods of classes from any arbitrary class level
- no header pollution: the reflection information is created in the cpp file to minimize compile time when modifying the data
- working with custom types without the need of having the declaration of the type available at compile time (useful for plugins)
- possibility to add additional **metadata / attribute** to all reflection objects
- **no** macro usage
- **no** rtti required: contains a faster and across shared libraries working replacement
- **no** exceptions (this feature come with cost and is also regularly disabled on consoles)
- **no** external compiler or tool needed, only standard ISO C++17

## Licensing

You can copy and paste the license summary from below.

```
MIT License

Copyright (c) 2020 Ubpa

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

