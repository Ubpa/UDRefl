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

## Feature



## Example

```c++
#include <UDRefl/UDRefl.h>
#include <iostream>

using namespace Ubpa::UDRefl;

struct Point {
  float x;
  float y;
};

int main() {
  TypeID ID_Point = ReflMngr::Instance().tregistry.Register("Point");
  TypeID ID_float = ReflMngr::Instance().tregistry.Register("float");
  
  NameID ID_x = ReflMngr::Instance().nregistry.Register("x");
  NameID ID_y = ReflMngr::Instance().nregistry.Register("y");
  
  { // register Point
    TypeInfo typeinfo{{
        { ID_x, { { ID_float, offsetof(Point, x) } }},
        { ID_y, { { ID_float, offsetof(Point, y) } }}
    }};
    ReflMngr::Instance().typeinfos.emplace(ID_Point, std::move(typeinfo));
  }
  
  Point p;
  ObjectPtr ptr{ ID_Point, &p };
  ReflMngr::Instance().RWField(ptr, ID_x).As<float>() = 1.f;
  ReflMngr::Instance().RWField(ptr, ID_y).As<float>() = 2.f;
  
  ReflMngr::Instance().ForEachRField(
    ptr,
    [](TypeID typeID, const TypeInfo& typeinfo, NameID fieldID, const FieldInfo& fieldinfo, ConstObjectPtr field) {
      std::cout
        << ReflMngr::Instance().registry.Nameof(fieldID)
        << ": " << field.As<float>()
        << std::endl;
    }
  );
}
```

result is

```
x: 1
y: 2
```

## Documentation



## Integration



## TODO

- [x] virtual
- [x] return
- [x] foreach
- [x] attr
- [ ] ID
- [ ] life cycle (ctor, dtor, copy, etc.)
- [ ] global


## Compiler compatibility



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

