cmake_minimum_required(VERSION 3.5)
project(ometacpp)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_COMPILER clang++-16)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ferror-limit=1 -ftemplate-backtrace-limit=0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O0 -ferror-limit=1 -ftemplate-backtrace-limit=0")

# build the transpiler from pre-transpiled code
add_executable(ometa-cpp ometa.cpp)
target_include_directories(ometa-cpp PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_executable(ometa-cpp-debug ometa.cpp)
target_include_directories(ometa-cpp-debug PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_compile_definitions(ometa-cpp-debug PUBLIC DEBUG_PRINTS)

# create a directory for transpiled code
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/transpiled)

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
	target_include_directories(${TARGET} PRIVATE ${PROJECT_SOURCE_DIR}/include)
endmacro()

# examples
add_ometa_executable(ometa-self examples/ometa.ometa)
add_ometa_executable(test examples/test.ometa)
add_ometa_executable(calculator examples/calculator.ometa)

# verify that transpiling the transpiler ometa code results in the same C++ code
# as what the transpiler was built from
add_custom_target(verify-self-transpilation ALL
	COMMAND diff -s
		${PROJECT_SOURCE_DIR}/ometa.cpp
		${PROJECT_BINARY_DIR}/transpiled/ometa.ometa.cpp
	DEPENDS ${PROJECT_BINARY_DIR}/transpiled/ometa.ometa.cpp
)

# update workflow:

# ometa-cpp:
#    new_parser_from_old_source.ometa
# -> new_parser_from_old_source.ometa.cpp
# -> new-parser-from-old-source
add_custom_command(
	OUTPUT ${PROJECT_BINARY_DIR}/transpiled/new_parser_from_old_source.ometa.cpp
	DEPENDS ometa-cpp
	COMMAND ${PROJECT_BINARY_DIR}/ometa-cpp
		${PROJECT_SOURCE_DIR}/update/new_parser_from_old_source.ometa
		${PROJECT_BINARY_DIR}/transpiled/new_parser_from_old_source.ometa.cpp
)
add_executable(new-parser-from-old-source EXCLUDE_FROM_ALL
	${PROJECT_BINARY_DIR}/transpiled/new_parser_from_old_source.ometa.cpp)
target_include_directories(new-parser-from-old-source PRIVATE ${PROJECT_SOURCE_DIR}/include)

# new-parser-from-old-source:
#    new_parser_from_new_source.ometa
# -> new_parser_from_new_source.ometa.cpp
# -> new-parser-from-new-source
add_custom_command(
	OUTPUT ${PROJECT_BINARY_DIR}/transpiled/new_parser_from_new_source.ometa.cpp
	DEPENDS new-parser-from-old-source
	COMMAND ${PROJECT_BINARY_DIR}/new-parser-from-old-source
		${PROJECT_SOURCE_DIR}/update/new_parser_from_new_source.ometa
		${PROJECT_BINARY_DIR}/transpiled/new_parser_from_new_source.ometa.cpp
)
add_executable(new-parser-from-new-source EXCLUDE_FROM_ALL
	${PROJECT_BINARY_DIR}/transpiled/new_parser_from_new_source.ometa.cpp)
target_include_directories(new-parser-from-new-source PRIVATE ${PROJECT_SOURCE_DIR}/update/include)

# new-parser-from-new-source:
#    new_parser_from_new_source.ometa
# -> new_parser_self_transpiled.ometa.cpp
add_custom_command(
	OUTPUT ${PROJECT_BINARY_DIR}/transpiled/new_parser_self_transpiled.ometa.cpp
	DEPENDS new-parser-from-new-source
	COMMAND ${PROJECT_BINARY_DIR}/new-parser-from-new-source
		${PROJECT_SOURCE_DIR}/update/new_parser_from_new_source.ometa
		${PROJECT_BINARY_DIR}/transpiled/new_parser_self_transpiled.ometa.cpp
)

# verify that the new parser correctly transpiles itself
add_custom_target(verify-update
	COMMAND diff -s
		${PROJECT_BINARY_DIR}/transpiled/new_parser_from_new_source.ometa.cpp
		${PROJECT_BINARY_DIR}/transpiled/new_parser_self_transpiled.ometa.cpp
	DEPENDS
		new-parser-from-new-source
		${PROJECT_BINARY_DIR}/transpiled/new_parser_self_transpiled.ometa.cpp
)
set_target_properties(verify-update PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_executable(playground playground.cpp)