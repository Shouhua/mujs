set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
add_executable(hello main.c)
install(TARGETS hello RUNTIME DESTINATION bin)