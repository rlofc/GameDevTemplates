cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
add_compile_options(-std=c++1z)
project(GDT VERSION 0.1.0 LANGUAGES CXX)

find_package(PkgConfig REQUIRED)

# The following options will be used to set up a custom blend
# of backend libraries. For now, while GDT is still evolving
# everything is turned on by default.
option(PLATFORM_IS_SDL "PLATFORM_IS_SDL" ON)
option(PLATFORM_IS_GLFW "PLATFORM_IS_GLFW" ON)

option(GRAPHICS_IS_OPENGL "GRAPHICS_IS_OPENGL" ON)
#option(GRAPHICS_IS_VULKAN "GRAPHICS_IS_VULKAN" OFF)

option(AUDIO_IS_OPENAL "AUDIO_IS_OPENAL" ON)
#option(AUDIO_IS_FMOD "AUDIO_IS_FMOD" OFF)

option(PHYSICS_IS_BULLET "PHYSICS_IS_BULLET" ON)
#option(PHYSICS_IS_NEWTON "PHYSICS_IS_NEWTON" OFF)

option(BUILD_EXAMPLES_TOO "BUILD_EXAMPLES_TOO" ON)

if (PLATFORM_IS_SDL)
  add_definitions(-DPLATFORM_IS_SDL)
  find_package(SDL2 REQUIRED)
  set(SOURCE_FILES ${SOURCE_FILES} src/backends/sdl/sdl.cc)
  set(BACKEND_LIBS ${BACKEND_LIBS} SDL2)
endif()

if (PLATFORM_IS_GLFW)
  add_definitions(-DPLATFORM_IS_GLFW)
  pkg_search_module(GLFW REQUIRED glfw3)
  set(SOURCE_FILES ${SOURCE_FILES} src/backends/glfw/glfw.cc)
  set(BACKEND_INCLUDE_DIRS ${BACKEND_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS})
  set(BACKEND_LIBS ${BACKEND_LIBS} ${GLFW_LIBRARIES})
endif()

if (GRAPHICS_IS_OPENGL)
  find_package(OpenGL REQUIRED)
  set(BACKEND_LIBS ${BACKEND_LIBS} GL)
endif()

if (AUDIO_IS_OPENAL)
  find_package(OpenAL REQUIRED)
  set(SOURCE_FILES ${SOURCE_FILES} src/backends/openal/openal.cc)
  set(BACKEND_LIBS ${BACKEND_LIBS} openal vorbisfile)
endif()

if (PHYSICS_IS_BULLET)
  find_package( Bullet REQUIRED )
  set(BACKEND_INCLUDE_DIRS ${BACKEND_INCLUDE_DIRS} 
      ${BULLET_INCLUDE_DIR}
      src/backends/bullet
  )
  set(BACKEND_LIBS ${BACKEND_LIBS} ${BULLET_LIBRARIES})
endif()

# There are GDT's source files
set(SOURCE_FILES 
    ${SOURCE_FILES}
	src/core/math.cc
	src/utils/logger.cc
	src/core/easing.cc
	src/core/camera.cc
	src/core/animation.cc
	src/core/loader.cc
	src/core/timeline.cc
    src/imgui/imgui.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_gdt.cc
	src/utils/lodepng.cc)

add_library(
  gdt STATIC
    ${SOURCE_FILES}
)


add_definitions( -DDEBUG_LOGS -DINFO_LOGS -DWARNING_LOGS -DERROR_LOGS)

target_include_directories(gdt
    PUBLIC
    ${BACKEND_INCLUDE_DIRS}
    src
    src/backends/blueprints
    src/core
    src/utils
    src/imgui
)

set (COMMON_LIBS
      ${BACKEND_LIBS}
)

set (COMMON_INCLUDE_DIRS
    ${BACKEND_INCLUDE_DIRS}
    src
    src/backends/blueprints
    src/core
    src/utils
    src/imgui
)

set (GDT_RESOURCES_DIR
  res/
)

target_link_libraries(gdt INTERFACE ${COMMON_LIBS})


# add a target to generate API documentation with Doxygen
find_package(Doxygen)
#option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" OFF)
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})
if(BUILD_DOCUMENTATION)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif()
    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${doxyfile_in} ${doxyfile} @ONLY)
    add_custom_target(doxygen_docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)
    add_custom_command(TARGET doxygen_docs POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/docs
            ${CMAKE_BINARY_DIR}/_docs)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Sphinx REQUIRED)
option(BUILD_SPHINX "Create and install the HTML based API documentation (requires Doxygen)" ${SPHINX_FOUND})
if(BUILD_SPHINX)
    # configured documentation tools and intermediate build results
    set(BINARY_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/_build")
    # Sphinx cache with pickled ReST documents
    set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")
    # HTML output directory
    set(SPHINX_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/html")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/docs/conf.py.in"
        "${BINARY_BUILD_DIR}/conf.py"
        @ONLY)
    add_custom_target(gdt_sphinx_docs ALL
        ${SPHINX_EXECUTABLE}
            -Q -b html
            -c "${BINARY_BUILD_DIR}"
            -d "${SPHINX_CACHE_DIR}"
            "${CMAKE_BINARY_DIR}/_docs/"
            "${SPHINX_HTML_DIR}"
        COMMENT "Building HTML documentation with Sphinx")
    add_dependencies(gdt_sphinx_docs doxygen_docs)
    add_dependencies(gdt_sphinx_docs examples_literal_docs)
endif()


if (BUILD_EXAMPLES_TOO)
  add_subdirectory(examples)
endif()

