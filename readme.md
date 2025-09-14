# Aria

Aria is a simple-syntax interpreted language and this is the implement of Aria interpreter.

## Build

#### Requirements

C++20 or above, CMake >= 3.27, readline library (optional)

#### Build Options

| Option             | Description                     | Default Value |
| ------------------ | ------------------------------- | ------------- |
| `USE_READLINE`     | Enable readline library support | OFF           |
| `CMAKE_BUILD_TYPE` | Build type (Release/Debug)      | Release       |

#### Build Commands for Linux

```shell
mkdir build && cd build && cmake .. && make
```