# Type Convert

## reference

- 0: error
- 1: convertible
- 2: constructible

**function reference compatibility** 

|           | `T`  | `T&` | `const T&` | `T&&` |
| :-------: | :--: | :--: | :------: | :--: |
| `T`       | 0    | 0 | 0     | 0 |
| `T&`      | 0 | 0    | 1     | 1 |
| `const T&` | 0 | 1 | 0        | 1 |
| `T&&`     | 0 | 1 | 1     | 0    |
| `const T&&` | 0 | 1 | 1     | 1 |

**parameter-argument reference non copied argument compatible** 

|           | `T`  | `T&` | `const T&` | `T&&` | `const T&&` | `const T` |
| :---------: | :----: | :----: | :--------: | :----: | :---------: | :---------: |
| `T`       | -    | 0 | 0     | 1 | 0      | 0 |
| `T&`      | 0 | -    | 0     | 0 | 0      | 0 |
| `const T&` | 1 | 1 | -        | 1 | 1      | 1 |
| `T&&`     | 1 | 0 | 0     | -    | 0      | 0 |
| `const T&&` | 1 | 0 | 0     | 1 | -         | 1 |

**parameter-argument reference priority compatible** 

|           | `T`  | `T&` | `const T&` | `T&&` | `const T&&` | `const T` |
| :---------: | :----: | :----: | :--------: | :----: | :---------: | :---------: |
| `T`       | -    | 0 | 0     | 1 | 0      | 0 |
| `T&`      | 0 | -    | 0     | 0 | 0      | 0 |
| `const T&` | 0 | 0 | -        | 0 | 0      | 1 |
| `T&&`     | 1 | 0 | 0     | -    | 0      | 0 |
| `const T&&` | 0 | 0 | 0     | 0 | -         | 0 |

**parameter-argument reference compatible** 

|           | `T`  | `T&` | `const T&` | `T&&` | `const T&&` | `const T` |
| :---------: | :----: | :----: | :--------: | :----: | :---------: | :---------: |
| `T`       | -    | 2 | 2     | 1 | 2      | 2 |
| `T&`      | 0 | -    | 0     | 0 | 0      | 0 |
| `const T&` | 1 | 1 | -        | 1 | 1      | 1 |
| `T&&`     | 1 | 0 | 0     | -    | 0      | 2 |
| `const T&&` | 1 | 0 | 0     | 1 | -         | 1 |

## pointer & array

- 0: error
- 1: decay

|             | `T*` | `const T*` | `T[N]` | `const T[N]` |
| :---------: | :--: | :--------: | :----: | :----------: |
|    `T*`     |  -   |     0      |   1    |      0       |
| `const T*`  |  1   |     -      |   1    |      1       |
|    `T[]`    |  1   |     0      |  -/1   |      0       |
| `const T[]` |  1   |     1      |   1    |     -/1      |
