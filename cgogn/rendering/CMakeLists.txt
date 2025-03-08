project(cgogn_rendering
	LANGUAGES CXX C
)

find_package(cgogn_core REQUIRED)
find_package(ImGUI REQUIRED)

add_library(${PROJECT_NAME} STATIC "")
add_library(cgogn::rendering ALIAS ${PROJECT_NAME})

set(src_list
		"${CMAKE_CURRENT_LIST_DIR}/drawer.h"
		"${CMAKE_CURRENT_LIST_DIR}/drawer.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/mesh_render.h"
		"${CMAKE_CURRENT_LIST_DIR}/mesh_render_base.h"
		"${CMAKE_CURRENT_LIST_DIR}/mesh_render.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/types.h"
		"${CMAKE_CURRENT_LIST_DIR}/gl_image.h"
		"${CMAKE_CURRENT_LIST_DIR}/gl_image.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/vbo.h"
		"${CMAKE_CURRENT_LIST_DIR}/ebo.h"
		"${CMAKE_CURRENT_LIST_DIR}/vao.h"
		"${CMAKE_CURRENT_LIST_DIR}/texture.h"
		"${CMAKE_CURRENT_LIST_DIR}/fbo.h"
		"${CMAKE_CURRENT_LIST_DIR}/fbo.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/vbo_update.h"
		"${CMAKE_CURRENT_LIST_DIR}/shader_program.h"
		"${CMAKE_CURRENT_LIST_DIR}/shader_program.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/shape_drawer.h"
		"${CMAKE_CURRENT_LIST_DIR}/shape_drawer.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/skelshape.h"
		"${CMAKE_CURRENT_LIST_DIR}/skelshape.cpp"

		"${CMAKE_CURRENT_LIST_DIR}/ui_modules/point_cloud_render.h"
		"${CMAKE_CURRENT_LIST_DIR}/ui_modules/surface_render.h"

		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_flat.h"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_flat.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_flat_color_per_vertex.h"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_flat_color_per_vertex.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_flat_scalar_per_vertex.h"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_flat_scalar_per_vertex.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_flat_scalar_per_face.h"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_flat_scalar_per_face.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_flat_color_per_face.h"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_flat_color_per_face.cpp"

		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_phong.h"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_phong.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_phong_color_per_vertex.h"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_phong_color_per_vertex.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_phong_scalar_per_vertex.h"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_phong_scalar_per_vertex.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_phong_color_per_face.h"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_phong_color_per_face.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_phong_scalar_per_face.h"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_phong_scalar_per_face.cpp"
		
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_bold_line.h"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_bold_line.cpp"

		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_point_sprite.h"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_point_sprite.cpp"

		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_fullscreen_texture.h"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_fullscreen_texture.cpp"

		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_frame2d.h"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_frame2d.cpp"

		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_function_color_maps.h"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/shader_function_color_maps.cpp"
		
		"${CMAKE_CURRENT_LIST_DIR}/shaders/outliner.h"
		"${CMAKE_CURRENT_LIST_DIR}/shaders/outliner.cpp"
)

target_sources(${PROJECT_NAME} PRIVATE ${src_list})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${src_list})

set_target_properties(${PROJECT_NAME} PROPERTIES
	DEBUG_POSTFIX "_d"
	EXPORT_NAME rendering
)

target_include_directories(${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
	$<BUILD_INTERFACE:${CGOGN_SOURCE_DIR}/thirdparty>
	$<BUILD_INTERFACE:${CGOGN_SOURCE_DIR}/thirdparty/imgui>
	$<BUILD_INTERFACE:${CGOGN_SOURCE_DIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/cgogn/thirdparty>
)

target_link_libraries(${PROJECT_NAME} cgogn::core ImGUI)

# Write out cgogn_rendering_export.h to the current binary directory
generate_export_header(${PROJECT_NAME})

set(PKG_CONFIG_REQUIRES "cgogn_core ImGUI")
configure_file(${PROJECT_SOURCE_DIR}/cgogn_rendering.pc.in ${CMAKE_CURRENT_BINARY_DIR}/cgogn_rendering.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cgogn_rendering.pc
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
	COMPONENT cgogn_rendering_libraries
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cgogn/rendering
	COMPONENT cgogn_rendering_headers
	FILES_MATCHING PATTERN "*.h"
	REGEX "(examples|tests)" EXCLUDE
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cgogn_rendering_export.h
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cgogn/rendering
	COMPONENT cgogn_rendering_headers
)

cgogn_create_package("${CMAKE_CURRENT_SOURCE_DIR}")

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER cgogn)
