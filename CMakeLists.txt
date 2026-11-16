cmake_minimum_required(VERSION 3.20)

project(chekker_comp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)

# OpenGL
find_package(OpenGL REQUIRED)

# Main application
add_executable(chekker_comp
        src/base.cpp

        # ImGui core
        include/imgui/imgui.cpp
        include/imgui/imgui_draw.cpp
        include/imgui/imgui_tables.cpp
        include/imgui/imgui_widgets.cpp
        include/imgui/imgui_demo.cpp

        # Backends
        include/imgui/backends/imgui_impl_glfw.cpp
        include/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(chekker_comp PRIVATE
        include/imgui
        external/glfw/include
)

target_link_libraries(chekker_comp PRIVATE
        glfw
        OpenGL::GL
)

if(MINGW)
    target_link_options(chekker_comp PRIVATE -static-libgcc -static-libstdc++)
    get_filename_component(CODEHUB_MINGW_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
    set(CODEHUB_WINPTHREAD_DLL "${CODEHUB_MINGW_BIN}/libwinpthread-1.dll")
    get_filename_component(CODEHUB_MINGW_ROOT "${CODEHUB_MINGW_BIN}/.." ABSOLUTE)
    if(EXISTS "${CODEHUB_MINGW_ROOT}/licenses/winpthreads/COPYING")
        set(CODEHUB_WINPTHREAD_LICENSE_DIR "${CODEHUB_MINGW_ROOT}/licenses/winpthreads")
    else()
        set(CODEHUB_WINPTHREAD_LICENSE_DIR "${CODEHUB_MINGW_ROOT}/share/licenses/winpthreads")
    endif()
    foreach(CODEHUB_WINPTHREAD_LICENSE COPYING COPYING.MinGW-w64-runtime.txt COPYING.MinGW-w64.txt)
        if(NOT EXISTS "${CODEHUB_WINPTHREAD_LICENSE_DIR}/${CODEHUB_WINPTHREAD_LICENSE}")
            message(FATAL_ERROR "Missing winpthreads license file: ${CODEHUB_WINPTHREAD_LICENSE_DIR}/${CODEHUB_WINPTHREAD_LICENSE}")
        endif()
    endforeach()
endif()

set(CODEHUB_FONT "${CMAKE_CURRENT_SOURCE_DIR}/assets/static/NotoSans-Regular.ttf")
set(CODEHUB_FONT_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/assets/OFL.txt")
set(CODEHUB_ASSET_OUTPUT_DIR "$<TARGET_FILE_DIR:chekker_comp>/assets")

add_custom_command(TARGET chekker_comp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CODEHUB_ASSET_OUTPUT_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CODEHUB_FONT}" "${CODEHUB_ASSET_OUTPUT_DIR}/NotoSans-Regular.ttf"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${CODEHUB_FONT_LICENSE}" "${CODEHUB_ASSET_OUTPUT_DIR}/OFL.txt"
        VERBATIM
)

if(MINGW)
    if(NOT EXISTS "${CODEHUB_WINPTHREAD_DLL}")
        message(FATAL_ERROR "Missing MinGW runtime DLL: ${CODEHUB_WINPTHREAD_DLL}")
    endif()
    add_custom_command(TARGET chekker_comp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CODEHUB_WINPTHREAD_DLL}" "$<TARGET_FILE_DIR:chekker_comp>/libwinpthread-1.dll"
            COMMAND ${CMAKE_COMMAND} -E make_directory
                    "$<TARGET_FILE_DIR:chekker_comp>/licenses/winpthreads"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CODEHUB_WINPTHREAD_LICENSE_DIR}/COPYING"
                    "$<TARGET_FILE_DIR:chekker_comp>/licenses/winpthreads/COPYING"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CODEHUB_WINPTHREAD_LICENSE_DIR}/COPYING.MinGW-w64-runtime.txt"
                    "$<TARGET_FILE_DIR:chekker_comp>/licenses/winpthreads/COPYING.MinGW-w64-runtime.txt"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CODEHUB_WINPTHREAD_LICENSE_DIR}/COPYING.MinGW-w64.txt"
                    "$<TARGET_FILE_DIR:chekker_comp>/licenses/winpthreads/COPYING.MinGW-w64.txt"
            VERBATIM
    )
endif()

install(TARGETS chekker_comp RUNTIME DESTINATION .)
install(FILES "${CODEHUB_FONT}" "${CODEHUB_FONT_LICENSE}" DESTINATION assets)
if(MINGW)
    install(FILES "${CODEHUB_WINPTHREAD_DLL}" DESTINATION .)
    install(FILES
            "${CODEHUB_WINPTHREAD_LICENSE_DIR}/COPYING"
            "${CODEHUB_WINPTHREAD_LICENSE_DIR}/COPYING.MinGW-w64-runtime.txt"
            "${CODEHUB_WINPTHREAD_LICENSE_DIR}/COPYING.MinGW-w64.txt"
            DESTINATION licenses/winpthreads
    )
endif()