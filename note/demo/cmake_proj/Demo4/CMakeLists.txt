cmake_minimum_required(VERSION 3.26)
project(Demo4)

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