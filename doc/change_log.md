# Change Log

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

