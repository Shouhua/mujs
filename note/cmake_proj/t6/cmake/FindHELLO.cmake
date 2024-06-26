FIND_PATH(HELLO_INCLUDE_DIR hello.h /tmp/t3/include/hello)
FIND_LIBRARY(HELLO_LIBRARY NAMES hello PATHS /tmp/t3/lib)
# FIND_LIBRARY(HELLO_LIBRARY hello /tmp/t3/lib)
IF(NOT HELLO_INCLUDE_DIR)
	MESSAGE(FATAL_ERROR "hello include找不到")
ENDIF(NOT HELLO_INCLUDE_DIR)
IF(NOT HELLO_LIBRARY)
	MESSAGE(FATAL_ERROR "hello lib找不到")
ENDIF(NOT HELLO_LIBRARY)
IF(HELLO_INCLUDE_DIR AND HELLO_LIBRARY)
	SET(HELLO_FOUND TRUE)
ENDIF(HELLO_INCLUDE_DIR AND HELLO_LIBRARY)
IF(HELLO_FOUND)
	IF(NOT HELLO_FIND_QUIETLY)
		MESSAGE(STATUS "Found Hello: ${HELLO_LIBRARY}")
	ENDIF(NOT HELLO_FIND_QUIETLY)
ELSE(HELLO_FOUND)
	IF(HELLO_FIND_REQUIRED)
	   MESSAGE(FATAL_ERROR "Could not find hello library")
	ENDIF(HELLO_FIND_REQUIRED)	
ENDIF()