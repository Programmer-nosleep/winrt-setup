cmake_minimum_required(VERSION 3.24)

project(winrt_setup LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake)

set(CPPWINRT_CANDIDATE_PATHS)

foreach(candidate_root IN ITEMS "$ENV{WindowsSdkDir}/bin" "C:/Program Files (x86)/Windows Kits/10/bin")
    if(EXISTS "${candidate_root}")
        file(GLOB sdk_versions LIST_DIRECTORIES TRUE "${candidate_root}/*")
        list(SORT sdk_versions COMPARE NATURAL ORDER DESCENDING)
        foreach(sdk_version_dir IN LISTS sdk_versions)
            foreach(arch IN ITEMS x64 x86 arm64)
                list(APPEND CPPWINRT_CANDIDATE_PATHS "${sdk_version_dir}/${arch}")
            endforeach()
        endforeach()
    endif()
endforeach()

find_program(CPPWINRT_EXECUTABLE NAMES cppwinrt.exe PATHS ${CPPWINRT_CANDIDATE_PATHS} NO_DEFAULT_PATH)

if(NOT CPPWINRT_EXECUTABLE)
    message(
        FATAL_ERROR
        "cppwinrt.exe tidak ditemukan. Install Windows SDK / C++/WinRT tools terlebih dahulu."
    )
endif()

set(WINRT_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/cppwinrt")
set(WINRT_STAMP_FILE "${WINRT_GENERATED_DIR}/cppwinrt.stamp")

add_custom_command(
    OUTPUT "${WINRT_STAMP_FILE}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${WINRT_GENERATED_DIR}"
    COMMAND
        "${CPPWINRT_EXECUTABLE}"
        -input sdk
        -output "${WINRT_GENERATED_DIR}"
        -include Windows.Foundation
        -include Windows.Globalization
        -include Windows.Security.ExchangeActiveSyncProvisioning
        -include Windows.System.Profile
        -pch .
        -base
    COMMAND ${CMAKE_COMMAND} -E touch "${WINRT_STAMP_FILE}"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/platform/winrt_info.cpp"
        "${CPPWINRT_EXECUTABLE}"
    COMMENT "Generating C++/WinRT headers"
    VERBATIM
)

add_custom_target(winrt_projection DEPENDS "${WINRT_STAMP_FILE}")

CPMAddPackage(
    NAME fmt
    GITHUB_REPOSITORY fmtlib/fmt
    GIT_TAG 12.1.0
    OPTIONS
        "FMT_DOC OFF"
        "FMT_TEST OFF"
        "FMT_INSTALL ON"
)

CPMAddPackage(
    NAME SDL3
    GITHUB_REPOSITORY libsdl-org/SDL
    GIT_TAG release-3.4.2
    OPTIONS
        "SDL_SHARED ON"
        "SDL_STATIC OFF"
)

CPMAddPackage(
    NAME imgui
    GITHUB_REPOSITORY ocornut/imgui
    GIT_TAG v1.92.6
    DOWNLOAD_ONLY YES
)

CPMAddPackage(
    NAME lunasvg
    GITHUB_REPOSITORY sammycage/lunasvg
    GIT_TAG v3.3.0
    OPTIONS
        "LUNASVG_BUILD_EXAMPLES OFF"
        "PLUTOVG_BUILD_EXAMPLES OFF"
)

if(NOT TARGET imgui)
    add_library(imgui STATIC
        "${imgui_SOURCE_DIR}/imgui.cpp"
        "${imgui_SOURCE_DIR}/imgui_draw.cpp"
        "${imgui_SOURCE_DIR}/imgui_tables.cpp"
        "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
    )

    target_include_directories(
        imgui
        PUBLIC
            "${imgui_SOURCE_DIR}"
            "${imgui_SOURCE_DIR}/backends"
    )

    target_link_libraries(imgui PUBLIC SDL3::SDL3 opengl32)
    add_library(imgui::imgui ALIAS imgui)
endif()

add_executable(winrt_hello
    src/app/main.cpp
    src/app/app_types.cpp
    src/app/app_types.h
    src/app/application.cpp
    src/app/application.h
    src/event/input.h
    src/platform/winrt_info.cpp
    src/platform/winrt_info.h
    src/style/ui_style.cpp
    src/style/ui_style.h
    src/ui/workspace_model.cpp
    src/ui/workspace_model.h
    src/ui/container_blur.cpp
    src/ui/container_blur.h
    src/ui/svg_icon_cache.cpp
    src/ui/svg_icon_cache.h
    src/ui/workspace_widgets.cpp
    src/ui/workspace_widgets.h
    src/render/gl_api.cpp
    src/render/gl_api.h
    src/utils/math3d.h
    src/render/shader_program.cpp
    src/render/shader_program.h
    src/render/infinite_grid_renderer.cpp
    src/render/infinite_grid_renderer.h
    src/render/skydome_renderer.cpp
    src/render/skydome_renderer.h
    src/render/world_renderer.cpp
    src/render/world_renderer.h
    src/world/orbit_camera.cpp
    src/world/orbit_camera.h
    src/world/world_scene.cpp
    src/world/world_scene.h
)

add_dependencies(winrt_hello winrt_projection)

target_include_directories(
    winrt_hello
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${WINRT_GENERATED_DIR}"
)

target_compile_definitions(
    winrt_hello
    PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        SDL_MAIN_HANDLED
)

target_link_libraries(
    winrt_hello
    PRIVATE
        fmt::fmt
        imgui::imgui
        lunasvg::lunasvg
        SDL3::SDL3
        opengl32
        advapi32
        windowsapp
)

if(MSVC)
    target_compile_options(
        winrt_hello
        PRIVATE
            /W4
            /permissive-
            /EHsc
            /utf-8
            /Zc:__cplusplus
    )
endif()

set_target_properties(
    winrt_hello
    PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:winrt_hello>"
)

if(WIN32)
    add_custom_command(
        TARGET winrt_hello
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:winrt_hello>
            $<TARGET_FILE_DIR:winrt_hello>
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/res"
            "$<TARGET_FILE_DIR:winrt_hello>/res"
        COMMAND_EXPAND_LISTS
    )
endif()
