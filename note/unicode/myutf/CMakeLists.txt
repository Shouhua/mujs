cmake_minimum_required(VERSION 3.22)
project(UTF)

add_library(myutf lib/utf.c)
target_include_directories(myutf PRIVATE include)
add_executable(utf src/main.c)
target_link_libraries(utf PRIVATE myutf)
target_include_directories(utf PRIVATE include)
add_custom_command(TARGET utf POST_BUILD
    COMMAND bash -c "awk -v FS=';' '{ print $1 }' \"${CMAKE_SOURCE_DIR}/src/UnicodeData.txt\" > \"${CMAKE_BINARY_DIR}/codepoint.txt\""
    VERBATIM
)