cmake_minimum_required(VERSION 3.26)
project(Demo4)

set(Demo_VERSION_MAJOR 1)
set(Demo_VERSION_MINOR 0)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -O3 -Wall")

configure_file(
	${PROJECT_SOURCE_DIR}/config.h.in
	${PROJECT_BINARY_DIR}/config.h
)

option(USE_MYMATH "Use provided math implementation" ON)

if(USE_MYMATH)
	MESSAGE(STATUS "使用自定义math库")
	include_directories("${PROJECT_SOURCE_DIR}/math" ${PROJECT_BINARY_DIR})
	add_subdirectory(math)
	set(EXTRA_LIBS ${EXTRA_LIBS} MathFunctions)
else()
	include_directories(${PROJECT_BINARY_DIR})
	set(EXTRA_LIBS m)
endif()

aux_source_directory(. DIR_SRCS)

add_executable(Demo ${DIR_SRCS})
target_link_libraries(Demo ${EXTRA_LIBS})