# Change Log

[TOC]

## 0.8

- 0.8.0
  - remove `ConstObjectPtr` and `SharedConstObject` 
  - `TypeID` -> `Type`, `StrID` -> `Name` 

## 0.7

- 0.7.3
  - support `std::pair` (first, second)
  - support `std::tuple` 
  - `IDRegistry` name require ending with `'\0'` 
- 0.7.2
  - add algorithm `Contains*` 
  - `GetTemporaryResource` for user arguments buffer
  - fix bugs for `NewArgsGuard` 
  - `ArgsBuffer` -> `ArgPtrBuffer` 
- 0.7.1
  - cast APIs with `ConstObjectPtr` support reference
  - improve `ObjectPtrBase::operator bool` (as a meta function)
  - **refactor compatible, invokable and invoke** 
  - add `IsNonArgCopyConstructible` 
  - add more type compuations
    - `AddLValueReference` 
    - `AddRValueReference` 
  - fix bugs
    - `TypeID_of<Args>...` -> `TypeID_of<decltype(args)>...` 
    - `AInvoke`: `ObjectPtr `-> `&{T}` 
- 0.7.0
  - use C++20
  - use `std::span` 
  - rename `RegisterTypeAuto` to `RegisterType` 
  - auto register type's inner type (e.g. `value_type`, `key_type`, `mapped_type`, `iterator`, ...) 
  - remove `RegisterType` in `ReflMngr::GenerateParamList` and `ReflMngr::GenerateResultDesc` 
  - delete target `pch` 

## 0.6

- 0.6.1
  - `RWVar` support non-constant reference object.
  - improve `MethodPtr` 
  - improve `TypeIDRegistry::Register/RegisterUnmanaged`  
  - simplify `ObjectPtr` and `SharedObj`'s `operator x=` (`x` is `+,-,*,/,...`)
  - `ReflMngr` add algorithms for owned
- 0.6.0
  - copy argument for `T <- T&/const T&/const T&&` when invoking.
  - use perfect forwarding for invoke.
  - refactor `ParamList ` 
    - `ParamList::IsCompatible` -> `ReflMngr::IsCompatible` 
    - `ParamList` -> `std::vector<TypeID>` 
  - `ReflMngr::RVar` support reference object.
  - `args_buffer`'s type : `void*` -> `ArgsBuffer` (= `void* const*`).
  - improve test
  - `TypeIDRegistry` add functions
    - `RegisterAddRValueReference` 
    - `RegisterAddLValueReference` 
    - `RegisterAddConstRValueReference` 
  - `ReflMngr` add functions
    - `MNew` 
    - `MDelete` 

