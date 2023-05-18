cmake_minimum_required(VERSION 3.26)

find_package(Git)
if(Git_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
	execute_process(
		COMMAND
			"${GIT_EXECUTABLE}" describe --tags --abbrev=0
		WORKING_DIRECTORY
			${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE PROJECT_VERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
else()
	message(FATAL_ERROR "Could not determine Git tag")
endif()

string(REPLACE "." ";" VERSION_LIST ${PROJECT_VERSION})
list(GET VERSION_LIST 0 JS_VERSION_MAJOR)
list(GET VERSION_LIST 1 JS_VERSION_MINOR)
list(GET VERSION_LIST 2 JS_VERSION_PATCH)

configure_file(
	${CMAKE_SOURCE_DIR}/config.h.in
	${CMAKE_SOURCE_DIR}/config.h
)

project(mujs VERSION ${PROJECT_VERSION} LANGUAGES C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# 设置默认编译类型为Debug，使用cmake -DCMAKE_BUILD_TYPE=Release
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "options: Debug Release Sanitize" FORCE)
endif()


list(APPEND INCLUDE mujs.h jsi.h regexp.h utf.h astnames.h opnames.h list.h)
list(APPEND SRC jsarray.c jsboolean.c jsbuiltin.c jscompile.c jsdate.c jsdtoa.c jserror.c
	jsfunction.c jsgc.c jsintern.c jslex.c jsmath.c jsnumber.c jsobject.c json.c jsparse.c
	jsproperty.c jsregexp.c jsrepr.c jsrun.c jsstate.c jsstring.c jsvalue.c jsloop.c jstimer.c
	jsxhr.c jstask.c regexp.c utf.c
)

set(AST_HEADER_FILE "astnames.h")
set(OP_HEADER_FILE "opnames.h")

file(READ "jsi.h" JSI_CONTENT)

string(REGEX MATCHALL "(AST|EXP|STM)_[A-Za-z0-9_]+" AST_RAW_NAMES ${JSI_CONTENT})
file(WRITE ${AST_HEADER_FILE} "")
foreach(name ${AST_RAW_NAMES})
	string(REGEX REPLACE "[ \t\r\n]" "" name ${name})	
	string(REPLACE "AST_" "" name ${name})	
	string(TOLOWER ${name} name)
	file(APPEND ${AST_HEADER_FILE} "\"${name}\",\n")
endforeach()
list(APPEND INCLUDE ${AST_HEADER_FILE})

string(REGEX MATCHALL "[^A-Za-z_0-9]OP_[A-Za-z0-9_]+" OP_RAW_NAMES ${JSI_CONTENT})
file(WRITE ${OP_HEADER_FILE} "")
foreach(name ${OP_RAW_NAMES})
	string(REGEX REPLACE "[ \t\r\n]" "" name ${name})	
	string(REGEX REPLACE "^OP_" "" name ${name})	
	string(TOLOWER ${name} name)
	file(APPEND ${OP_HEADER_FILE} "\"${name}\",\n")
endforeach()
list(APPEND INCLUDE ${OP_HEADER_FILE})

# find_package(Python3 REQUIRED)
# find_package(CURL REQUIRED)
# if(CURL_FOUND AND Python3_FOUND)
# 	set(UNICODE_DATA_FILE "UnicodeData.txt")
# 	set(UTF_DATA_HEADER "utfdata.h")
# 	execute_process(
# 		COMMAND
# 			curl -s -o ${UNICODE_DATA_FILE} "https://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt"
# 		WORKING_DIRECTORY 
# 			${CMAKE_SOURCE_DIR}
# 	)
# 	execute_process(
# 		COMMAND
# 			${Python3_EXECUTABLE} genucd.py ${UNICODE_DATA_FILE}
# 		WORKING_DIRECTORY 
# 			${CMAKE_SOURCE_DIR}
# 		OUTPUT_FILE "${UTF_DATA_HEADER}"
# 	)
# 	list(APPEND INCLUDE "${UTF_DATA_HEADER}")
# else()
# 	message(FATAL_ERROR "please install curl and python3")
# endif()

set(CMAKE_C_FLAGS "-pedantic -Wall -Wextra -Wno-unused-parameter")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	find_package(Python3 REQUIRED)
	find_package(CURL REQUIRED)
	if(CURL_FOUND AND Python3_FOUND)
		set(UNICODE_DATA_FILE "UnicodeData.txt")
		set(UTF_DATA_HEADER "utfdata.h")
		add_custom_command(
			OUTPUT ${UNICODE_DATA_FILE}
			COMMAND curl -s -o ${UNICODE_DATA_FILE} "https://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt"
			# WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			VERBATIM
		)
		add_custom_command(
			OUTPUT "${UTF_DATA_HEADER}"
			COMMAND ${Python3_EXECUTABLE} "${CMAKE_SOURCE_DIR}/genucd.py" "${CMAKE_BINARY_DIR}/${UNICODE_DATA_FILE}" > ${UTF_DATA_HEADER}
			DEPENDS ${UNICODE_DATA_FILE}
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			VERBATIM
		)
		add_custom_target(generate_utf_header DEPENDS ${UTF_DATA_HEADER})
	else()
		message(FATAL_ERROR "please install curl and python3")
	endif()

	set(DEBUG_EXE "mujs-debug")
	add_library("${DEBUG_EXE}_shared" SHARED ${SRC} ${INCLUDE})
	set_target_properties("${DEBUG_EXE}_shared" PROPERTIES OUTPUT_NAME "${DEBUG_EXE}")
	set_target_properties("${DEBUG_EXE}_shared" PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${JS_VERSION_MAJOR})
	# target_include_directories("${DEBUG_EXE}_shared" PRIVATE ${CMAKE_BINARY_DIR})
	target_compile_options("${DEBUG_EXE}_shared" PRIVATE -shared PRIVATE -fPIC)

	add_library("${DEBUG_EXE}_static" STATIC ${SRC} ${INCLUDE})
	set_target_properties("${DEBUG_EXE}_static" PROPERTIES OUTPUT_NAME "${DEBUG_EXE}")
	# target_include_directories("${DEBUG_EXE}_static" PRIVATE ${CMAKE_BINARY_DIR})

	list(APPEND SRC main.c)
	add_executable("${DEBUG_EXE}" ${SRC} ${INCLUDE})
	add_dependencies("${DEBUG_EXE}" generate_utf_header)
	add_dependencies("${DEBUG_EXE}_shared" generate_utf_header)
	add_dependencies("${DEBUG_EXE}_static" generate_utf_header)
	target_compile_definitions("${DEBUG_EXE}" PRIVATE READLINE)
	target_compile_options("${DEBUG_EXE}" PRIVATE -g PRIVATE -fPIC)
	target_link_libraries("${DEBUG_EXE}" PRIVATE event curl readline m)
	# target_include_directories("${DEBUG_EXE}" PRIVATE ${CMAKE_BINARY_DIR})

	set(CMAKE_INSTALL_PREFIX "/tmp/mujs")
	install(TARGETS "${DEBUG_EXE}_shared" "${DEBUG_EXE}_static" "${DEBUG_EXE}"
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION lib
		RUNTIME DESTINATION bin
	)
	install(FILES mujs.h DESTINATION "include")
endif()
unset(CMAKE_BUILD_TYPE CACHE)