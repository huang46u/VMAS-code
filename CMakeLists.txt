cmake_minimum_required(VERSION 3.7.2 FATAL_ERROR)

set(CGOGN_VERSION_MAJOR 3)
set(CGOGN_VERSION_MINOR 0)
set(CGOGN_VERSION_PATCH 0)

project(CGoGN
	VERSION ${CGOGN_VERSION_MAJOR}.${CGOGN_VERSION_MINOR}.${CGOGN_VERSION_PATCH}
	LANGUAGES CXX C
)

foreach(p
	CMP0048 # version
	CMP0054 # CMake 3.1
	CMP0072 # opengl
	CMP0092 # remove /W3 by default in MSVC
)
	if(POLICY ${p})
		cmake_policy(SET ${p} NEW)
	endif()
endforeach()

#### Default build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

#### CGoGN PATH
set(CGOGN_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

#### CGoGN DATA PATH
set(CGOGN_DATA_PATH "${CGOGN_SOURCE_DIR}/data/")
add_definitions("-DCGOGN_DATA_PATH=${CGOGN_DATA_PATH}")

#### Here are located the FindPackages that we need
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}")

#### Compile Options
include(GNUInstallDirs)

include(cmake/utilities.cmake)
include(cmake/CheckSIMDFeatures.cmake)

if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
	  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
		  ${CMAKE_CURRENT_BINARY_DIR}/stage/${CMAKE_INSTALL_BINDIR})
  endif()
  if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
	  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
		  ${CMAKE_CURRENT_BINARY_DIR}/stage/${CMAKE_INSTALL_LIBDIR})
  endif()
  if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
	  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
		  ${CMAKE_CURRENT_BINARY_DIR}/stage/${CMAKE_INSTALL_LIBDIR})
endif()

#### ThirdParty options
set(CGOGN_THIRDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
option(CGOGN_BUILD_EXAMPLES "Build some example apps." ON)
option(CGOGN_USE_SIMD "Enable SIMD instructions (sse,avx...)" ON)
option(CGOGN_ENABLE_LTO "Enable link-time optimizations (only with gcc)" ON)
if (NOT MSVC)
	option(CGOGN_USE_CXX11_ABI "use the CXX11 ABI." ON)
endif(NOT MSVC)
option(CGOGN_GL43_DEBUG_MODE "GL43 DEBUGGING MODE")

## Position independent code (-fPIC)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

#### RPATH
if(UNIX)
	# RPATH is a field in ELF binaries that is used as a hint by the system
	# loader to find needed shared libraries.
	#
	# In the build directory, cmake creates binaries with absolute paths in
	# RPATH.  And by default, it strips RPATH from installed binaries.  Here we
	# use CMAKE_INSTALL_RPATH to set a relative RPATH.  By doing so, we avoid
	# the need to play with LD_LIBRARY_PATH to get applications to run.
	set(CMAKE_INSTALL_RPATH "../lib:../../lib")
	if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
		set(CMAKE_MACOSX_RPATH ON)
	endif()
endif(UNIX)

## Deduce build type of not forced by setting the CMAKE_BUILD_TYPE var
deduce_build_type()

#### Endianness detection
include (TestBigEndian)
test_big_endian(CGOGN_TEST_BIG_ENDIAN)

#### Link time optimisation
if (CGOGN_ENABLE_LTO AND ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU") AND (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug"))
	find_program(GCC_AR gcc-ar)
	if (GCC_AR)
		set(CMAKE_AR ${GCC_AR})
	endif()
	find_program(GCC_RANLIB gcc-ranlib)
	if (GCC_RANLIB)
		set(CMAKE_RANLIB ${GCC_RANLIB})
	endif()
endif()

include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(APPLE)
	find_library(CORE_FOUNDATION CoreFoundation)
	find_library(CARBON Carbon)
endif()

if(MSVC)
  string(REGEX REPLACE "/W3" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  string(REGEX REPLACE "-W3" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
  add_compile_definitions(_SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING)
endif()

set(CGOGN_SYSTEM_MODULE_PATH ${CGOGN_SOURCE_DIR}/cgogn)

add_subdirectory(${CGOGN_THIRDPARTY_DIR})

cgogn_list_subdirectory(CGOGN_CONFIGURED_MODULES ${CGOGN_SYSTEM_MODULE_PATH})

foreach(subdir ${CGOGN_CONFIGURED_MODULES})
	option(CGOGN_MODULE_${subdir} "Enable CGOGN component ${subdir}" ON)
	if(CGOGN_MODULE_${subdir})
		list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cgogn/${subdir}")
	endif()
endforeach()

foreach(subdir ${CGOGN_CONFIGURED_MODULES})
	if(CGOGN_MODULE_${subdir})
		add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cgogn/${subdir})
	endif()
endforeach()

foreach(subdir ${CGOGN_CONFIGURED_MODULES})
	if(CGOGN_MODULE_${subdir})
		if(CGOGN_BUILD_EXAMPLES)
			if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/cgogn/${subdir}/apps/)
				add_subdirectory(cgogn/${subdir}/apps)
			endif()
		endif()
	endif()
endforeach()

unset(CGOGN_SYSTEM_MODULE_PATH)
