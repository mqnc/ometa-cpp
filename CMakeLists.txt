cmake_minimum_required(VERSION 3.5)
project(ometacpp)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER clang++-16)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ferror-limit=1 -ftemplate-backtrace-limit=0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O0 -ferror-limit=1 -ftemplate-backtrace-limit=0")

# build the transpiler from pre-transpiled code
add_executable(ometa-cpp src/ometa.cpp)
add_executable(ometa-cpp-debug src/ometa.cpp)
target_compile_definitions(ometa-cpp-debug PUBLIC DEBUG_PRINTS)

# create a directory for transpiled code
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/transpiled)

# transpile the transpiler
add_custom_command(
	OUTPUT ${PROJECT_BINARY_DIR}/transpiled/ometa.cpp
	DEPENDS ometa-cpp ${PROJECT_SOURCE_DIR}/examples/ometa.ometa
    COMMAND ${PROJECT_BINARY_DIR}/ometa-cpp
		${PROJECT_SOURCE_DIR}/examples/ometa.ometa
		${PROJECT_BINARY_DIR}/transpiled/ometa.cpp
)

# verify that transpiling the transpiler ometa code results in the same C++ code
# as what the transpiler was built from
add_custom_target(verify-self-transpilation
	COMMAND diff -s ${PROJECT_SOURCE_DIR}/src/ometa.cpp ${PROJECT_BINARY_DIR}/transpiled/ometa.cpp
	DEPENDS ${PROJECT_BINARY_DIR}/transpiled/ometa.cpp
)

# macro for source.ometa -> source.ometa.cpp -> target
macro(add_ometa_executable TARGET SOURCE)
	get_filename_component(SOURCE_FILENAME ${SOURCE} NAME)
	add_custom_command(
		OUTPUT ${PROJECT_BINARY_DIR}/transpiled/${SOURCE_FILENAME}.cpp
		DEPENDS ometa-cpp ${PROJECT_SOURCE_DIR}/${SOURCE}
		COMMAND ${PROJECT_BINARY_DIR}/ometa-cpp
			${PROJECT_SOURCE_DIR}/${SOURCE}
			${PROJECT_BINARY_DIR}/transpiled/${SOURCE_FILENAME}.cpp
	)
	add_executable(${TARGET} ${PROJECT_BINARY_DIR}/transpiled/${SOURCE_FILENAME}.cpp)
	target_include_directories(${TARGET} PRIVATE ${PROJECT_SOURCE_DIR})
endmacro()

add_ometa_executable(test examples/test.ometa)
add_ometa_executable(calculator examples/calculator.ometa)

