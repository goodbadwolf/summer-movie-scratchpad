cmake_minimum_required(VERSION 3.15)

macro(resource_dirs dirs)
    foreach(dir ${dirs})
        # Replace / at the end of the path (copy dir content VS copy dir)
        string(REGEX REPLACE "/+$" "" dirclean "${dir}")
        message(STATUS "Copying resource ${dirclean}")
        file(COPY ${dirclean} DESTINATION ${CMAKE_BINARY_DIR})
    endforeach()
endmacro()

project(scratchpad)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(GLEW   REQUIRED)
find_package(glm    REQUIRED)

add_executable(scratchpad scratchpad.cxx)
target_link_libraries(scratchpad ${OPENGL_gl_LIBRARY} GLEW glfw)

# Change the location to wherever your aneursym obj files are present.
resource_dirs( "${CMAKE_SOURCE_DIR}/../aneurysm")
set(copy_source_dir "${CMAKE_SOURCE_DIR}/")
set(copy_dest_dir "${CMAKE_BINARY_DIR}/")
set(copy_file_name "shaders")
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${copy_source_dir}/${copy_file_name}" "${copy_dest_dir}/${copy_file_name}"
    COMMENT "Copying ${copy_file_name} to build directory"
)