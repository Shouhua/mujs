cmake_minimum_required(VERSION 3.26)
project(CALCTEST)
aux_source_directory(./ SRC)
include_directories(${HEAD_PATH})
# include_directories(${HEAD_PATH})
link_libraries(${CALC_LIB}) # 添加静态库
set(EXECUTABLE_OUTPUT_PATH ${EXEC_PATH})
add_executable(${APP_NAME_1} ${SRC})