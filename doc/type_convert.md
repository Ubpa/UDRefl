# Type Convert

- 0: error
- 1: convertible
- 2: copy

|           | T    | T&   | const T& | T&&  | const T&& |
| --------- | ---- | ---- | -------- | ---- | --------- |
| T         | -    | 0 | 0     | 1 | 0      |
| T&        | 0 | -    | 0     | 0 | 0      |
| const T&  | 1 | 1 | -        | 1 | 1      |
| T&&       | 1 | 0 | 0     | -    | 0      |
| const T&& | 1 | 0 | 0     | 1 | -         |

|           | T      | T&             | const T& | T&&             | const T&&       |
| --------- | ------ | -------------- | -------- | --------------- | --------------- |
| T         | -      | shrink     | shrink | expand          | shrink      |
| T&        | 0   | -              | 0     | 0            | 0            |
| const T&  | shrink | shrink+ expand | -        | shrink + expand | shrink + expand |
| T&&       | shrink | 0           | 0     | -               | 0            |
| const T&& | shrink | 0           | 0     | shrink          | -               |

