## cmake
- 通常在根目录下添加build文件夹，用于编译防止生成文件跟源文件混淆
- 个人喜好，将方法名小写，里面关键字大写，比如cmake_minimum_required(VERSION 3.26)
## cmake语法积累
```shell
cmake_minimal_required(VERSION 3.26)
project(CLAC c)
# aux_src_list(./src SRC_LIST)
# set(SRC_LIST add.c mul.c)
file(GLOB SRC_LIST ${PROJECT_SRC_DIR}/*.c)
list(REMOVE_ITEM ${SRC_LIST} ${PROJECT_SRC_DIR}/main.c)
add_executable(calc ${SRC_LIST})
```
- 指定使用的c/c++标准  
1. set(CMAKE_C_STANDARD 90), 90代表c89/c90
2. set_property(TARGET tgt PROPERTY C_STANDARD 90)
3. cmake -DCMAKE_C_STANDARD=90
- 指定输入路径
set(EXECUTE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
- 搜索文件
1. aux_source_directory(<dir> <variable>)
2. file(GLOB/GLOB_RECURSE 变量名 要搜索的文件路径和文件类型)  
file(GLOB MAIN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE MAIN_HEAD ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
- include_directories(headpath) 包含头文件
- 设置动态库输出路径 
1. set(EXECUTABLE_OUTPUT_PATH path)，适用于动态库
2. set(LIBRARY_OUTPUT_PATH path)， LIBRARY_OUTPUT_PATH对静态库和动态库都适用
3. add_subdirectory(src_dir bin_dir EXCLUDE_FROM_ALL)，里面的bin_dir也可以设置动态库路径
- 链接静态库和动态库，link_libraries和target_link_libraries
link_libraries需要在add_executable前面使用，target_link_libraries需要生成target后使用，可以从参数看出来  
```target_link_libraries(target item1 item2...)```
- 指定自定义链接库路径
```link_directories(${PROJECT_SOURCE_DIR}/lib)```
- 日志
```
message([STATUS|WARNING|AUTHOR_WARNING|FATAL_ERROR|SEND_ERROR] "message to display")
```
其中STATUS会输出到stdout，其他输出到stderr; FATAL_ERROR会终止所有处理过程，SEND_ERROR会继续执行，但是会跳过生成步骤
- 变量操作，主要是使用set，list，string，file等函数
```
set(TEMP "hello, world")
file(GLOB SRC ${PROJECT_SOURCE_DIR}/src/*.c)
set(SRC ${SRC} ${TEMP})
list(APPEND SRC ${SRC} ${TEMP})
list(APPEND/LENGTH/JOIN...)
```
- gcc宏定义
```add_difinitions(-DDebug)```
- options可以在执行cmake时指定，比如cmake -DOPT1=ON
```opions(OPT1 '演示使用opt1' OFF)``` 如果不指定默认值，默认OFF
- FIND_LIBRARY(libhello hello /tmp/t3/lib), 可以判断libhello变量
- FIND_PATH(HELLO_INCLUDE_DIR hello.h /tmp/t3/include/hello), 可以判断HELLO_INCLUDE_DIR进行其他操作
- set(var value CACHE type FORCE)  
CACHE相当于将设置变量放到CMakeCache.txt文件中，相当于全局变量，全局都能访问到, 修改缓存变量时，必须加上FORCE
- string(func string out-var)  
使用string的func转化string到out-var变量中
- add_subdirectory(src_dir bin_dir EXCLUDE_FROM_ALL)  
主要想说明EXCLUDE_FROM_ALL，使用这个关键字默认情况下子目录的依赖不会编译，**[但是如果使用target_link_directories()依赖了相应的库，还是会编译](https://www.jianshu.com/p/07acea4e86a3)**
## 参考文档
- https://gavinliu6.github.io/CMake-Practice-zh-CN/#/
- https://www.hahack.com/codes/cmake/
