
#          Copyright Michael Florian Hava.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.13)
project(PTL)

option(PTL_BUILD_TESTS "Build tests" OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/static)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(ptl_flags INTERFACE)
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	target_compile_options(ptl_flags INTERFACE -Wall -Wextra -Wpedantic -Wconversion)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	target_compile_definitions(ptl_flags INTERFACE _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS _CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS _SILENCE_FPOS_SEEKPOS_DEPRECATION_WARNING)
	target_compile_options(ptl_flags INTERFACE
		/W4 /JMC /MP /permissive- /bigobj
		/wd4003 # ignore not enough parameters for function macro
		/wd4456 # ignore local shadowing
		/wd4457 # ignore function parameter shadowing
		/wd4458 # ignore class member shadowing
		/wd4459 # ignore global shadowing
	)
	target_link_options(ptl_flags INTERFACE
		/ignore:4099 # ignore mixing struct and class keyword for same type
	)
endif()

add_library(ptl INTERFACE)
target_include_directories(ptl INTERFACE "inc")
target_link_libraries(ptl INTERFACE ptl_flags)

if(PTL_BUILD_TESTS)
	find_package(Catch2 CONFIG REQUIRED)

	file(GLOB_RECURSE PTL "inc/*")
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/inc FILES ${PTL})
	file(GLOB_RECURSE PTL_TEST "test/*")
	source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/test FILES ${PTL_TEST})
	add_executable(test-ptl ${PTL} ${PTL_TEST}) #including PTL sources here to have them included in IDEs
	target_link_libraries(test-ptl PRIVATE ptl Catch2::Catch2)
	set_target_properties(test-ptl PROPERTIES FOLDER "Portable Template Library")
endif()

find_package(Doxygen 1.8.2) #Doxygen 1.8.2 is the first version to support C++11
if(Doxygen_FOUND)
	set(DOXYGEN_PROJECT_NAME "Portable Template Library")
	set(DOXYGEN_PROJECT_BRIEF "A collection of binary stable types targeted at cross-compiler interop")
	set(DOXYGEN_OUTPUT_DIRECTORY "docs")
	set(DOXYGEN_BUILTIN_STL_SUPPORT YES)
	set(DOXYGEN_SHOW_USED_FILES NO)
	set(DOXYGEN_SHOW_FILES NO)
	set(DOXYGEN_WARN_IF_UNDOCUMENTED NO)
	set(DOXYGEN_FILE_PATTERNS "*.hpp")
	set(DOXYGEN_EXCLUDE_SYMBOLS "ptl::internal;ptl::internal::*")
	set(DOXYGEN_GENERATE_TREEVIEW YES)
	set(DOXYGEN_MACRO_EXPANSION YES)
	set(DOXYGEN_COLLABORATION_GRAPH NO)
	doxygen_add_docs(PTL-Documentation "inc;docs")
	set_target_properties(PTL-Documentation PROPERTIES FOLDER "Portable Template Library")
endif()
