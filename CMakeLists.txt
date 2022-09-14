cmake_minimum_required(VERSION 3.21.0)

project(
    glsl2msl
    LANGUAGES CXX
)

set(SPIRV-Headers_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Deps/SPIRV-Headers/" CACHE STRING "Header location")
add_subdirectory("Deps/SPIRV-Tools" SPIRV-Tools)

set(SPIRV_CROSS_CLI OFF CACHE BOOL "")
set(SPIRV_CROSS_ENABLE_HLSL OFF CACHE BOOL "")
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "")
set(SPIRV_CROSS_SKIP_INSTALL ON CACHE BOOL "")
add_subdirectory("Deps/SPIRV-Cross" SPIRV-Cross)

set(ENNABLE_EXCEPTIONS_ON_MSVC OFF CACHE BOOL "")
set(ENABLE_CTEST OFF CACHE BOOL "")
set(DISABLE_EXCEPTIONS ON CACHE BOOL "")
set(BUILD_TESTING OFF CACHE BOOL "")

add_subdirectory("Deps/glslang" glslang)

set(SHADERC_ENABLE_TESTS OFF)
set(SHADERC_SKIP_TESTS ON CACHE BOOL "Skip building tests" FORCE)
set(SHADERC_SKIP_INSTALL ON CACHE BOOL "Skip installation" FORCE)
add_subdirectory("Deps/shaderc" shaderc)

include(cmake/headers.cmake)
include(cmake/sources.cmake)

source_group(
    TREE
        ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
        ${HEADERS}
        ${SOURCES}
)

add_executable(
    ${PROJECT_NAME}
    ${HEADERS}
    ${SOURCES}
)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC
        spirv-cross-core
        spirv-cross-glsl
        spirv-cross-cpp
        spirv-cross-reflect
        spirv-cross-msl
        spirv-cross-util
        shaderc
)

target_compile_features(
    ${PROJECT_NAME}
    PRIVATE
        cxx_std_20
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
        "App/include"
        "Deps/SPIRV-Cross"
        "Deps/SPIRV-Cross/include"
        "Deps/shaderc/libshaderc/include"

)

if (MSVC)
    add_compile_definitions(_UNICODE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")

    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE
            /MP
            /await
            /W4
            /WX
            /permissive-
            /Zc:alignedNew
            /Zc:auto
            /Zc:__cplusplus
            /Zc:externC
            /Zc:externConstexpr
            /Zc:forScope
            /Zc:hiddenFriend
            /Zc:implicitNoexcept
            /Zc:lambda
            /Zc:noexceptTypes
            /Zc:preprocessor
            /Zc:referenceBinding
            /Zc:rvalueCast
            /Zc:sizedDealloc
            /Zc:strictStrings
            /Zc:ternary
            /Zc:threadSafeInit
            /Zc:trigraphs
            /Zc:wchar_t
    )

    target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:/fp:strict>;/ZI;/Od;/Gy")
    target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:/Zi;/fp:fast;/GL;/Gy-;/Gm-;/Gw;/sdl-;/GS-;/guard:cf-;/O2;/Ob2;/Oi;/Ot;/Oy;/fp:except->")

    target_link_options(
        ${PROJECT_NAME}
        PRIVATE
            /WX
            "$<$<CONFIG:DEBUG>:/INCREMENTAL;/OPT:NOREF;/OPT:NOICF>"
            "$<$<CONFIG:RELEASE>:/LTCG;/INCREMENTAL:NO;/OPT:REF;/OPT:ICF;/DEBUG:FULL>"
    )
endif ()