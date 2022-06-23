
#          Copyright Michael Florian Hava.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.23)
project(PTL)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_library(ptl INTERFACE)
	target_include_directories(ptl INTERFACE "inc")
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		target_compile_options(ptl INTERFACE -Wall -Wextra -Wpedantic -Wconversion)
	elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		target_compile_definitions(ptl INTERFACE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS _CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS _SILENCE_FPOS_SEEKPOS_DEPRECATION_WARNING)
		target_compile_options(ptl INTERFACE
			/W4 /JMC /MP /permissive- /bigobj
			/wd4003 # ignore not enough parameters for function macro
			/wd4456 # ignore local shadowing
			/wd4457 # ignore function parameter shadowing
			/wd4458 # ignore class member shadowing
			/wd4459 # ignore global shadowing
		)
		target_link_options(ptl INTERFACE
			/ignore:4099 # ignore mixing struct and class keyword for same type
		)
	endif()


option(PTL_BUILD_TESTS "Build tests" OFF)
if(PTL_BUILD_TESTS)
	find_package(Catch2 CONFIG REQUIRED)
	enable_testing()

	add_executable(test-ptl)
		file(GLOB_RECURSE PTL "inc/*")
			source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/inc FILES ${PTL})
		file(GLOB_RECURSE PTL_TEST "test/*")
			source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/test FILES ${PTL_TEST})
		target_sources(test-ptl PRIVATE ${PTL} ${PTL_TEST}) #including PTL sources here to have them included in IDEs
		target_link_libraries(test-ptl PRIVATE ptl Catch2::Catch2 Catch2::Catch2WithMain)

	file(GLOB CLASSES "inc/ptl/*")
	foreach(CLASS ${CLASSES})
		add_test(NAME ${CLASS} COMMAND test-ptl "-n ${CLASS}")
	endforeach()
endif()


option(PTL_BUILD_DOCUMENTATION "Build documentation" OFF)
if(PTL_BUILD_DOCUMENTATION)
	find_package(Doxygen 1.9.1)

	set(DOXYGEN_PROJECT_NAME "Portable Template Library")
	set(DOXYGEN_PROJECT_BRIEF "A collection of binary stable types targeted at cross-compiler interop")
	set(DOXYGEN_OUTPUT_DIRECTORY "docs")
	set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
	set(DOXYGEN_SHOW_USED_FILES NO)
	set(DOXYGEN_SHOW_FILES NO)
	set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
	set(DOXYGEN_FILE_PATTERNS "*.hpp")
	set(DOXYGEN_EXCLUDE_SYMBOLS "ptl::internal*")
	set(DOXYGEN_GENERATE_TREEVIEW YES)
	set(DOXYGEN_MACRO_EXPANSION YES)
	set(DOXYGEN_COLLABORATION_GRAPH NO)
	doxygen_add_docs(Documentation "inc;docs")
endif()
