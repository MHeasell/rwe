cmake_minimum_required(VERSION 3.8)
project(rwe)
enable_testing()

function(target_copy_dll target dll)
    if(WIN32)
        find_program("${dll}_DLL" ${dll})

        if("${dll}_DLL" STREQUAL "${dll}_DLL-NOTFOUND")
            message(FATAL_ERROR "${dll} not found")
        endif()

        message("Found ${dll}: ${${dll}_DLL}")

        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${${dll}_DLL} ${PROJECT_BINARY_DIR})
    endif()
endfunction()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

set(CMAKE_CXX_STANDARD 17)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.54.0 REQUIRED COMPONENTS filesystem)

find_package(PNG REQUIRED)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_net REQUIRED)

find_package(ZLIB REQUIRED)

set(SOURCE_FILES
    src/rwe/ColorPalette.cpp
    src/rwe/ColorPalette.h
    src/rwe/Gaf.cpp src/rwe/Gaf.h
    src/rwe/GraphicsContext.cpp
    src/rwe/GraphicsContext.h
    src/rwe/Hpi.cpp
    src/rwe/Hpi.h
    src/rwe/OtherTriangleScene.cpp
    src/rwe/OtherTriangleScene.h
    src/rwe/SceneManager.cpp
    src/rwe/SceneManager.h
    src/rwe/SdlContextManager.cpp
    src/rwe/SdlContextManager.h
    src/rwe/SharedTextureHandle.cpp
    src/rwe/SharedTextureHandle.h
    src/rwe/TextureHandle.cpp
    src/rwe/TextureHandle.h
    src/rwe/TriangleScene.cpp
    src/rwe/TriangleScene.h
    src/rwe/geometry/BoundingBox3f.cpp
    src/rwe/geometry/BoundingBox3f.h
    src/rwe/geometry/Plane3f.cpp
    src/rwe/geometry/Plane3f.h
    src/rwe/geometry/Ray3f.cpp
    src/rwe/geometry/Ray3f.h
    src/rwe/geometry/Triangle3f.cpp
    src/rwe/geometry/Triangle3f.h
    src/rwe/math/Matrix4f.cpp
    src/rwe/math/Matrix4f.h
    src/rwe/math/Vector2f.cpp
    src/rwe/math/Vector2f.h
    src/rwe/math/Vector3f.cpp
    src/rwe/math/Vector3f.h
    src/rwe/math/rwe_math.cpp
    src/rwe/math/rwe_math.h
    src/rwe/pcx.cpp
    src/rwe/pcx.h
    src/rwe/rwe_string.cpp
    src/rwe/rwe_string.h
    src/rwe/tdf/SimpleTdfAdapter.cpp
    src/rwe/tdf/SimpleTdfAdapter.h
    src/rwe/tdf/TdfParser.cpp
    src/rwe/tdf/TdfParser.h
    src/rwe/vfs/AbstractVirtualFileSystem.h
    src/rwe/vfs/CompositeVirtualFileSystem.cpp
    src/rwe/vfs/CompositeVirtualFileSystem.h
    src/rwe/vfs/DirectoryFileSystem.cpp
    src/rwe/vfs/DirectoryFileSystem.h
    src/rwe/vfs/HpiFileSystem.cpp
    src/rwe/vfs/HpiFileSystem.h
    )

add_library(librwe STATIC ${SOURCE_FILES})
set_target_properties(librwe PROPERTIES PREFIX "")
target_compile_options(librwe PUBLIC "-Wall" "-Wextra")
target_include_directories(librwe PUBLIC "src")

target_include_directories(librwe PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(librwe ${Boost_LIBRARIES})

target_include_directories(librwe PUBLIC "libs/utfcpp/source")

target_link_libraries(librwe ${OPENGL_LIBRARIES})

target_include_directories(librwe PUBLIC ${GLEW_INCLUDE_DIRS})
target_link_libraries(librwe ${GLEW_LIBRARIES})

target_link_libraries(librwe ${SDL2_LIBRARY})
target_link_libraries(librwe ${SDL2_IMAGE_LIBRARIES})
target_link_libraries(librwe ${SDL2_MIXER_LIBRARIES})
target_link_libraries(librwe ${SDL2_NET_LIBRARIES})

target_include_directories(librwe PUBLIC ${SDL2_INCLUDE_DIR})
target_include_directories(librwe PUBLIC ${SDL2_IMAGE_INCLUDE_DIRS})
target_include_directories(librwe PUBLIC ${SDL2_MIXER_INCLUDE_DIRS})
target_include_directories(librwe PUBLIC ${SDL2_NET_INCLUDE_DIRS})

target_link_libraries(librwe ${ZLIB_LIBRARIES})
target_include_directories(librwe PUBLIC ZLIB_INCLUDE_DIRS)
target_copy_dll(librwe "zlib1.dll")

add_executable(rwe src/main.cpp)
target_link_libraries(rwe librwe)

add_executable(hpi_test src/hpi_test.cpp)
target_link_libraries(hpi_test librwe)
target_link_libraries(hpi_test -static)

add_executable(vfs_test src/vfs_test.cpp)
target_link_libraries(vfs_test librwe)
target_link_libraries(vfs_test -static)

add_executable(gaf_test src/gaf_test.cpp)
target_copy_dll(gaf_test "libpng16-16.dll")
target_link_libraries(gaf_test ${PNG_LIBRARIES})
target_link_libraries(gaf_test librwe)
target_link_libraries(gaf_test -static)
target_include_directories(gaf_test PUBLIC ${PNG_INCLUDE_DIRS})
target_include_directories(gaf_test PUBLIC libs/png++-0.2.9)
target_compile_definitions(gaf_test PRIVATE __STDC_LIB_EXT1__=1)

add_executable(pcx_test src/pcx_test.cpp)
target_copy_dll(pcx_test "libpng16-16.dll")
target_link_libraries(pcx_test ${PNG_LIBRARIES})
target_link_libraries(pcx_test librwe)
target_link_libraries(pcx_test -static)
target_include_directories(pcx_test PUBLIC ${PNG_INCLUDE_DIRS})
target_include_directories(pcx_test PUBLIC libs/png++-0.2.9)
target_compile_definitions(pcx_test PRIVATE __STDC_LIB_EXT1__=1)

set(TEST_FILES
    test/rwe/SimpleTdfAdapter_test.cpp
    test/rwe/geometry/BoundingBox3f_test.cpp
    test/rwe/geometry/Plane3f_test.cpp
    test/rwe/geometry/Ray3f_test.cpp
    test/rwe/geometry/Triangle3f_test.cpp
    test/rwe/math/Matrix4f_test.cpp
    test/rwe/math/Vector3f_test.cpp
    test/rwe/rwe_string_test.cpp)

add_executable(rwe_test test/main.cpp ${TEST_FILES})
target_include_directories(rwe_test PRIVATE "libs/catch")
target_link_libraries(rwe_test librwe)
add_test(NAME rwe_test COMMAND rwe_test)
