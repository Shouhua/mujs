### 功能
主要练习自定义cmake模块，在文件中如何使用
### cmake重点
- 设置module path
```shell
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
```
- 寻找package
```shell
find_package(HELLO REQUIRED)
if(HELLO_FOUND)
	add_executable(main main.c)
	include_directories(${HELLO_INCLUDE_DIR})
	target_link_libraries(main ${HELLO_LIBRARY})
endif(HELLO_FOUND)

```
- 模块相关的变量
比如find_package(CURL REQUIRED)  
CURL_FOUND, 判断模块是否被找到，没有找到，按照工程需要关闭某些特性，给出提醒或者中止编译  
CURL_INCLUDE_DIR or CURL_INCLUDES  
CURL_LIBRARY or CURL_LIBRARIES  
如果<name>_FOUND为真，则将<name>_INCLUDE_DIR加入INCLUDE_DIRECTORIES，将<name>_LIBRARY加入TARGET_LINK_LIBRARIES中