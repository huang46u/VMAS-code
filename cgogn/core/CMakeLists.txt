project(cgogn_core
	LANGUAGES CXX
)

# Hide symbols by default
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

find_package(Eigen3 3.2)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(${PROJECT_NAME} STATIC "")
add_library(cgogn::core ALIAS ${PROJECT_NAME})

set(src_list
		"${CMAKE_CURRENT_LIST_DIR}/types/attribute_handler.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/cell_marker.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/cells_set.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/mesh_traits.h"
		
		"${CMAKE_CURRENT_LIST_DIR}/types/mesh_views/cell_cache.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/mesh_views/cell_filter.h"

		"${CMAKE_CURRENT_LIST_DIR}/types/maps/map_base.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/map_base.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/dart_marker.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/dart.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/cell.h"

		"${CMAKE_CURRENT_LIST_DIR}/types/maps/cmap/cmap_base.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/cmap/cmap0.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/cmap/cmap0.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/cmap/cmap1.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/cmap/cmap1.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/cmap/cmap2.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/cmap/cmap2.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/cmap/cmap3.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/cmap/cmap3.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/cmap/cph3.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/cmap/cph3.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/cmap/graph.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/cmap/graph.cpp"
		
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/gmap/gmap_base.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/gmap/gmap0.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/gmap/gmap0.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/gmap/gmap1.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/gmap/gmap1.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/gmap/gmap2.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/gmap/gmap2.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/gmap/gmap3.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/maps/gmap/gmap3.cpp"

		"${CMAKE_CURRENT_LIST_DIR}/types/incidence_graph/incidence_graph_base.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/incidence_graph/incidence_graph.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/incidence_graph/incidence_graph.cpp"

		"${CMAKE_CURRENT_LIST_DIR}/types/triangle_soup/triangle_soup.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/triangle_soup/triangle_soup.cpp"

		"${CMAKE_CURRENT_LIST_DIR}/types/container/attribute_container.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/container/attribute_container.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/types/container/chunk_array.h"
		"${CMAKE_CURRENT_LIST_DIR}/types/container/vector.h"

		"${CMAKE_CURRENT_LIST_DIR}/functions/attributes.h"
		"${CMAKE_CURRENT_LIST_DIR}/functions/convert.h"
		"${CMAKE_CURRENT_LIST_DIR}/functions/mesh_info.h"
		"${CMAKE_CURRENT_LIST_DIR}/functions/traversals/vertex.h"
		"${CMAKE_CURRENT_LIST_DIR}/functions/traversals/halfedge.h"
		"${CMAKE_CURRENT_LIST_DIR}/functions/traversals/edge.h"
		"${CMAKE_CURRENT_LIST_DIR}/functions/traversals/face.h"
		"${CMAKE_CURRENT_LIST_DIR}/functions/traversals/volume.h"
		
		"${CMAKE_CURRENT_LIST_DIR}/utils/assert.h"
		"${CMAKE_CURRENT_LIST_DIR}/utils/assert.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/utils/buffers.h"
		"${CMAKE_CURRENT_LIST_DIR}/utils/definitions.h"
		"${CMAKE_CURRENT_LIST_DIR}/utils/numerics.h"
		"${CMAKE_CURRENT_LIST_DIR}/utils/string.h"
		"${CMAKE_CURRENT_LIST_DIR}/utils/string.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/utils/thread_pool.h"
		"${CMAKE_CURRENT_LIST_DIR}/utils/thread_pool.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/utils/thread.h"
		"${CMAKE_CURRENT_LIST_DIR}/utils/thread.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/utils/tuples.h"
		"${CMAKE_CURRENT_LIST_DIR}/utils/type_traits.h"

		"${CMAKE_CURRENT_LIST_DIR}/ui_modules/mesh_provider.h"
		"${CMAKE_CURRENT_LIST_DIR}/ui_modules/mesh_data.h"
)
target_sources(${PROJECT_NAME} PRIVATE ${src_list})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${src_list})

# use of target_compile_options to have transitive flags
if(CGOGN_USE_SIMD)
	target_compile_options(${PROJECT_NAME} PUBLIC ${CGOGN_SSE_FLAGS})
	target_compile_definitions(${PROJECT_NAME} PUBLIC "CGOGN_USE_SIMD")
else()
	target_compile_definitions(${PROJECT_NAME} PUBLIC "EIGEN_DONT_VECTORIZE")
endif()

target_compile_options(${PROJECT_NAME} PUBLIC
	# g++
#	$<$<CXX_COMPILER_ID:GNU>:$<BUILD_INTERFACE:-Wall>>
#	$<$<CXX_COMPILER_ID:GNU>:-Wextra>
	$<$<CXX_COMPILER_ID:GNU>:-Wnon-virtual-dtor>
	$<$<CXX_COMPILER_ID:GNU>:-Wno-unknown-pragmas>
	# Clang
	$<$<CXX_COMPILER_ID:Clang>:$<BUILD_INTERFACE:-Wall>>
	$<$<CXX_COMPILER_ID:Clang>:-Wextra>
	$<$<CXX_COMPILER_ID:Clang>:-Wnon-virtual-dtor>
	$<$<CXX_COMPILER_ID:Clang>:-Wno-missing-braces>
	$<$<CXX_COMPILER_ID:Clang>:-Wno-unknown-pragmas>
	# MSVC
	#Specifies the level of warning to be generated by the compiler. Valid warning levels for n range from 0 to 4.
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
	# C4127 - conditional expression is constant
	$<$<CXX_COMPILER_ID:MSVC>:/wd4127>
	# C4714 - marked as __forceinline but not inlined
	$<$<CXX_COMPILER_ID:MSVC>:/wd4714>
	# C4505 - unreferenced local function has been removed (impossible to deactive selectively)
	$<$<CXX_COMPILER_ID:MSVC>:/wd4505>
	# C4910 - __declspec(dllexport)' and 'extern' are incompatible on an explicit instantiation
	$<$<CXX_COMPILER_ID:MSVC>:/wd4910>
	# C4251 - 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
	$<$<CXX_COMPILER_ID:MSVC>:/wd4251>
	$<$<CXX_COMPILER_ID:MSVC>:/wd4702> # uneraechable code 
	$<$<CXX_COMPILER_ID:MSVC>:/wd4244> # conversion
	$<$<CXX_COMPILER_ID:MSVC>:/wd4267> # siz_t ->int
	$<$<CXX_COMPILER_ID:MSVC>:/wd4456> # declaration  shadow ..
	$<$<CXX_COMPILER_ID:MSVC>:/wd4459> # type declaration  shadow ..
	$<$<CXX_COMPILER_ID:MSVC>:/wd4100> # parameter not referenced
	$<$<CXX_COMPILER_ID:MSVC>:/wd4189> # var initialized but not referenced
	$<$<CXX_COMPILER_ID:MSVC>:/EHsc>
	$<$<CXX_COMPILER_ID:MSVC>:/bigobj>
	)

if(CGOGN_INSANE_WARN_LEVEL)
	target_compile_options(${PROJECT_NAME} PUBLIC
		$<$<CXX_COMPILER_ID:GNU>:-pedantic>
		$<$<CXX_COMPILER_ID:GNU>:-Wno-long-long>
		$<$<CXX_COMPILER_ID:GNU>:-Wconversion>
		$<$<CXX_COMPILER_ID:GNU>:-Winline>
		$<$<CXX_COMPILER_ID:GNU>:-Wsign-conversion>
		$<$<CXX_COMPILER_ID:GNU>:-Wdouble-promotion>
		# Clang
		$<$<CXX_COMPILER_ID:Clang>:-Weverything>
		$<$<CXX_COMPILER_ID:Clang>:-Wno-unused-macros>
		$<$<CXX_COMPILER_ID:Clang>:-Wno-disabled-macro-expansion>
		$<$<CXX_COMPILER_ID:Clang>:-Wno-covered-switch-default>
		$<$<CXX_COMPILER_ID:Clang>:-Wno-padded>
		$<$<CXX_COMPILER_ID:Clang>:-Wno-float-equal>
		# Ignore warnings about global variables ctors and dtors
		$<$<CXX_COMPILER_ID:Clang>:-Wno-global-constructors>
		# Ignore warnings about global destructor
		$<$<CXX_COMPILER_ID:Clang>:-Wno-exit-time-destructors>
		# Turn this on to detect documentation errors (very useful)
		$<$<CXX_COMPILER_ID:Clang>:-Wno-documentation>
		# Ignore unknown documentation command (There are nrecognized but valid doxygen commands !)
		$<$<CXX_COMPILER_ID:Clang>:-Wno-documentation-unknown-command>
		# Ignore warnings about C++98 compatibility
		$<$<CXX_COMPILER_ID:Clang>:-Wno-c++98-compat>
		# Ignore warnings about c++98 compat pedantic mode
		$<$<CXX_COMPILER_ID:Clang>:-Wno-c++98-compat-pedantic>
		# Ignore warnings about C++11 extensions (cgogn is promoting c++11 )
		$<$<CXX_COMPILER_ID:Clang>:-Wno-c++11-extensions>)
endif()

if(${CGOGN_USE_CXX11_ABI})
	target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<CXX_COMPILER_ID:GNU>:_GLIBCXX_USE_CXX11_ABI=1>)
#else()
#	target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<CXX_COMPILER_ID:GNU>:_GLIBCXX_USE_CXX11_ABI=0>)
endif()

# see https://gcc.gnu.org/onlinedocs/libstdc++/manual/using_macros.html
if(${CGOGN_USE_GLIBCXX_DEBUG})
	target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<CXX_COMPILER_ID:GNU>:_GLIBCXX_DEBUG>)
	if(${CGOGN_USE_GLIBCXX_DEBUG_PEDANTIC})
		target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<CXX_COMPILER_ID:GNU>:_GLIBCXX_DEBUG_PEDANTIC>)
	endif(${CGOGN_USE_GLIBCXX_DEBUG_PEDANTIC})
endif(${CGOGN_USE_GLIBCXX_DEBUG})

if(${CGOGN_USE_PARALLEL_GLIBCXX})
	if(${CGOGN_USE_GLIBCXX_DEBUG})
		message(WARNING "Can't use at the same time the libc++ debug and parallel modes.")
	else (${CGOGN_USE_GLIBCXX_DEBUG})
		target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<CXX_COMPILER_ID:GNU>:_GLIBCXX_PARALLEL>)
	endif(${CGOGN_USE_GLIBCXX_DEBUG})
endif(${CGOGN_USE_PARALLEL_GLIBCXX})


target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<CONFIG:Release>:$<$<CXX_COMPILER_ID:GNU>:_FORTIFY_SOURCE=2>>)

## CGOGN_WIN_VER : has value 61 for windows 7, 62 for windows 8, 63 for windows 8.1, 100 for windows 10
set(WIN_VERSION "")
string(REPLACE "." "" WIN_VERSION ${CMAKE_SYSTEM_VERSION})
target_compile_definitions(${PROJECT_NAME} PUBLIC
	$<$<CXX_COMPILER_ID:MSVC>:NOMINMAX>
	$<$<CXX_COMPILER_ID:MSVC>:CGOGN_CPP_STD=17>
	$<$<CXX_COMPILER_ID:MSVC>:_USE_MATH_DEFINES>
	$<$<CXX_COMPILER_ID:MSVC>:CGOGN_WIN_VER=${WIN_VERSION}>)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

if(CGOGN_USE_SIMD)
	CGOGN_CHECK_FOR_SSE()
endif(CGOGN_USE_SIMD)

if(${CGOGN_USE_OPENMP})
	if(OpenMP_FOUND OR OPENMP_FOUND OR OpenMP_CXX_FOUND)
		if(TARGET OpenMP::OpenMP_CXX)
			target_link_libraries(${PROJECT_NAME} INTERFACE OpenMP::OpenMP_CXX)
		else()
			# Clang may need -fopenmp=libiomp5 instead, can't be detected here without CMake 3.9
			target_link_libraries(${PROJECT_NAME} INTERFACE
				$<$<CXX_COMPILER_ID:GNU>:${OpenMP_CXX_FLAGS}>
				$<$<CXX_COMPILER_ID:Clang>:${OpenMP_CXX_FLAGS}>
				$<$<CXX_COMPILER_ID:Intel>:${OpenMP_CXX_FLAGS}>
				)
			target_compile_options(${PROJECT_NAME} INTERFACE ${OpenMP_CXX_FLAGS})
		endif()
		# Pass the required flags to code
		target_compile_definitions(${PROJECT_NAME} INTERFACE ${OpenMP_CXX_FLAGS})
	endif()
endif()

# Profiler compilation flags
if(CGOGN_WITH_GPROF)
	message(STATUS "Building for code profiling")
	target_compile_definitions(${PROJECT_NAME} PUBLIC  -pg -DPROFILER)
endif(CGOGN_WITH_GPROF)

# Code coverage compilation flags
if(CGOGN_WITH_GCOV)
	message(STATUS "Building for coverage analysis")
	target_compile_definitions(${PROJECT_NAME} PUBLIC   --coverage)
endif(CGOGN_WITH_GCOV)

# Compilation flags for Google's AddressSanitizer
# These flags can only be specified for dynamic builds
if(CGOGN_WITH_ASAN)
	message(STATUS "Building with AddressSanitizer (debug only)")
	target_compile_definitions(${PROJECT_NAME} PUBLIC  $<$<CONFIG:Debug>:-fsanitize=address -fno-omit-frame-pointer -O1>)
endif(CGOGN_WITH_ASAN)
#TODO Use native GCC stack smash Protection and buffer overflow detection in debug when no asan ??

# Compilation flags for Google's ThreadSanitizer
# Does not work for the moment: cannot figure out how to link with library libtsan
if(CGOGN_WITH_TSAN)
	message(STATUS "Building with ThreadSanitizer (debug only)")
	target_compile_definitions(${PROJECT_NAME} PUBLIC  $<$<CONFIG:Debug>:-fsanitize=thread>)
endif(CGOGN_WITH_TSAN)

if(CGOGN_ENABLE_LTO)
	target_compile_options(${PROJECT_NAME} PUBLIC
		$<$<CONFIG:Release>:$<$<CXX_COMPILER_ID:GNU>:-flto>>
		$<$<CONFIG:Release>:$<$<CXX_COMPILER_ID:Clang>:-flto>>)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
	DEBUG_POSTFIX "_d"
	EXPORT_NAME core
)

target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads Eigen3::Eigen)

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
	$<BUILD_INTERFACE:${CGOGN_SOURCE_DIR}>
	$<BUILD_INTERFACE:${CGOGN_THIRDPARTY_TERMCOLOR_INCLUDE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/cgogn/thirdparty>
)

# Write out cgogn_core_export.h to the current binary directory
generate_export_header(${PROJECT_NAME})

if(${CGOGN_EXTERNAL_TEMPLATES})
	target_compile_definitions(${PROJECT_NAME} PUBLIC "CGOGN_USE_EXTERNAL_TEMPLATES")
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC "CGOGN_LITTLE_ENDIAN=1234")
target_compile_definitions(${PROJECT_NAME} PUBLIC "CGOGN_BIG_ENDIAN=4321")

if(CGOGN_TEST_BIG_ENDIAN)
	target_compile_definitions(${PROJECT_NAME} PUBLIC "CGOGN_ENDIANNESS=CGOGN_BIG_ENDIAN")
else()
	target_compile_definitions(${PROJECT_NAME} PUBLIC "CGOGN_ENDIANNESS=CGOGN_LITTLE_ENDIAN")
endif()

configure_file(${PROJECT_SOURCE_DIR}/cgogn_core.pc.in ${CMAKE_CURRENT_BINARY_DIR}/cgogn_core.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cgogn_core.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
	COMPONENT cgogn_core_libraries
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cgogn/core
	COMPONENT cgogn_core_headers
	FILES_MATCHING PATTERN "*.h"
	REGEX "(examples|tests)" EXCLUDE
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cgogn_core_export.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cgogn/core
	COMPONENT cgogn_core_headers
)

cgogn_create_package("${CMAKE_CURRENT_SOURCE_DIR}")

# VisualStudio nice source organisation
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER cgogn)
