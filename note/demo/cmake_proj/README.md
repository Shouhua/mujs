## cmake常用语法
```
cmake_minimum_required(VERSION 3.26)
project(hello)
set(SRC_LIST main.c)
add_subdirectory(src)
message(STATUS "this is hello project message")

add_executable(hello ${SRC_LIST})
```
## 参考文档
- https://gavinliu6.github.io/CMake-Practice-zh-CN/#/
- https://www.hahack.com/codes/cmake/
