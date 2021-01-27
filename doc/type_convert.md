# Type Convert

- 0: error
- 1: convertible
- 2: copy

**function compatibility** 

|           | `T`  | `T&` | `const T&` | `T&&` | `const T&&` |
| :-------: | :--: | :--: | :------: | :--: | :-------: |
| `T`       | 0    | 0 | 0     | 0 | 0    |
| `T&`      | 0 | 0    | 1     | 1 | 1      |
| `const T&` | 0 | 1 | 0        | 1 | 1      |
| `T&&`     | 0 | 1 | 1     | 0    | 1      |
| `const T&&` | 0 | 1 | 1     | 1 | 0         |

**parameter-argument reference priority compatible** 

|           | `T`  | `T&` | `const T&` | `T&&` | `const T&&` |
| :---------: | :----: | :----: | :--------: | :----: | :---------: |
| `T`       | -    | 0 | 0     | 1 | 0      |
| `T&`      | 0 | -    | 0     | 0 | 0      |
| `const T&` | 1 | 1 | -        | 1 | 1      |
| `T&&`     | 1 | 0 | 0     | -    | 0      |
| `const T&&` | 1 | 0 | 0     | 1 | -         |

**parameter-argument reference compatible** 

|           | `T`  | `T&` | `const T&` | `T&&` | `const T&&` |
| :---------: | :----: | :----: | :--------: | :----: | :---------: |
| `T`       | -    | 2 | 2     | 1 | 2      |
| `T&`      | 0 | -    | 0     | 0 | 0      |
| `const T&` | 1 | 1 | -        | 1 | 1      |
| `T&&`     | 1 | 0 | 0     | -    | 0      |
| `const T&&` | 1 | 0 | 0     | 1 | -         |

