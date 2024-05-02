
#          Copyright Michael Florian Hava.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.26)
project(PTL)

add_library(ptl INTERFACE)
	target_compile_features(ptl INTERFACE cxx_std_17)
	target_include_directories(ptl INTERFACE "inc")
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		target_compile_options(ptl INTERFACE -Wall -Wextra -Wpedantic -Wconversion)
	elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		target_compile_options(ptl INTERFACE
			# warning flags
			/W4 # enable all sensible warnings
			/wd4003 # ignore not enough parameters for function macro
			/wd4099 # ignore mixing struct and class keyword for same type
			/wd4456 # ignore local shadowing
			/wd4457 # ignore function parameter shadowing
			/wd4458 # ignore class member shadowing
			/wd4459 # ignore global shadowing
			/wd4848 # ignore no_unique_address is a vendor extension in C++17
			/we4700 # treat use of uninitialized variables as compile error
			# conformance flags
			/permissive- # enable conformance mode (TODO: [C++20] this is the default!)
			/Zc:__cplusplus # update __cplusplus according to standard (stays at C++98 otherwise)
			/Zc:preprocessor # enable standard conformant preprocessor
			# miscellaneous flags
			/MP # parallel compilation
			/JMC # improve debugging support by only stepping into own code
			/bigobj # use "big object files" to circumvent class of errors that are related to heavy template or machine generated code
		)
	endif()


option(PTL_BUILD_TESTS "Build tests" OFF)
if(PTL_BUILD_TESTS)
	find_package(Catch2 CONFIG REQUIRED)
	enable_testing()

	add_executable(test-ptl)
		file(GLOB_RECURSE PTL CONFIGURE_DEPENDS "inc/*")
			source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/inc FILES ${PTL})
			target_sources(test-ptl PRIVATE ${PTL}) #including PTL sources here to have them included in IDEs
		file(GLOB_RECURSE PTL CONFIGURE_DEPENDS "test/*")
			source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/test FILES ${PTL})
			target_sources(test-ptl PRIVATE ${PTL})
		target_link_libraries(test-ptl PRIVATE ptl Catch2::Catch2 Catch2::Catch2WithMain)

	file(GLOB CLASSES CONFIGURE_DEPENDS "inc/ptl/*")
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
