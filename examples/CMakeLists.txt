cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
add_compile_options(-std=c++1z)
project(GDT_EXAMPLES VERSION 0.1.0 LANGUAGES CXX)

add_executable(
    empty_app
    empty_app.cc
    ../src/backends/sdl/imgui_impl_sdl.cpp
    )
target_link_libraries(empty_app gdt)
target_include_directories(empty_app PUBLIC
    ${COMMON_INCLUDE_DIRS}
    )

add_executable(
    basic_rendering
    basic_rendering.cc
    ../src/backends/sdl/imgui_impl_sdl.cpp
    )
target_link_libraries(basic_rendering gdt)
target_include_directories(basic_rendering PUBLIC
    ${COMMON_INCLUDE_DIRS}
    )

add_executable(
    physics_instancing
    physics_instancing.cc
    ../src/backends/glfw/imgui_impl_glfw.cpp
    )
target_link_libraries(physics_instancing gdt)
target_include_directories(physics_instancing PUBLIC
    ${COMMON_INCLUDE_DIRS}
    )

#-------------------------------------------------------------------------------
# SKELETAL ANIMATION
add_executable(
    skeletal_animation
    skeletal_animation.cc
    ../src/backends/sdl/imgui_impl_sdl.cpp
    )
target_link_libraries(skeletal_animation gdt)
target_include_directories(skeletal_animation PUBLIC
    ${COMMON_INCLUDE_DIRS}
    )
#-------------------------------------------------------------------------------
# TEXT RENDERING
add_executable(
    text_rendering
    text_rendering.cc
    ../src/backends/sdl/imgui_impl_sdl.cpp
    )
target_link_libraries(text_rendering gdt)
target_include_directories(text_rendering PUBLIC
    ${COMMON_INCLUDE_DIRS}
    )


add_custom_command(
    OUTPUT 
      ${CMAKE_BINARY_DIR}/_docs/example_empty_app.rst
      ${CMAKE_BINARY_DIR}/_docs/example_basic_rendering.rst
      ${CMAKE_BINARY_DIR}/_docs/example_skeletal_animation.rst
      ${CMAKE_BINARY_DIR}/_docs/example_physics_instancing.rst
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/_docs/
    COMMAND awk -f ${CMAKE_CURRENT_SOURCE_DIR}/literst.awk
    ${CMAKE_CURRENT_SOURCE_DIR}/empty_app.cc 
      > ${CMAKE_BINARY_DIR}/_docs/example_empty_app.rst
      COMMAND awk -f ${CMAKE_CURRENT_SOURCE_DIR}/literst.awk
      ${CMAKE_CURRENT_SOURCE_DIR}/basic_rendering.cc 
      > ${CMAKE_BINARY_DIR}/_docs/example_basic_rendering.rst
      COMMAND awk -f ${CMAKE_CURRENT_SOURCE_DIR}/literst.awk
      ${CMAKE_CURRENT_SOURCE_DIR}/skeletal_animation.cc 
      > ${CMAKE_BINARY_DIR}/_docs/example_skeletal_animation.rst
      COMMAND awk -f ${CMAKE_CURRENT_SOURCE_DIR}/literst.awk
      ${CMAKE_CURRENT_SOURCE_DIR}/physics_instancing.cc 
      > ${CMAKE_BINARY_DIR}/_docs/example_physics_instancing.rst
    )
add_custom_target(examples_literal_docs ALL DEPENDS 
      ${CMAKE_BINARY_DIR}/_docs/example_empty_app.rst
      ${CMAKE_BINARY_DIR}/_docs/example_basic_rendering.rst
      ${CMAKE_BINARY_DIR}/_docs/example_skeletal_animation.rst
      ${CMAKE_BINARY_DIR}/_docs/example_physics_instancing.rst
    )
