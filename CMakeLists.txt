cmake_minimum_required(VERSION 3.8)
project(rwe)
enable_testing()

function(target_copy_dll target dll)
    if(WIN32)
        find_program(TARGET_DLL ${dll})

        if(TARGET_DLL STREQUAL "TARGET_DLL-NOTFOUND")
            message(FATAL_ERROR "${dll} not found")
        endif()

        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${TARGET_DLL} ${PROJECT_BINARY_DIR})
    endif()
endfunction()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

set(CMAKE_CXX_STANDARD 17)

find_package(Boost 1.54.0 REQUIRED)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_net REQUIRED)

find_package(ZLIB REQUIRED)

set(SOURCE_FILES
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
    src/rwe/rwe_string.cpp src/rwe/rwe_string.h)

add_library(librwe STATIC ${SOURCE_FILES})
set_target_properties(librwe PROPERTIES PREFIX "")
target_compile_options(librwe PUBLIC "-Wall" "-Wextra")
target_include_directories(librwe PUBLIC "src")
target_include_directories(librwe PUBLIC ${Boost_INCLUDE_DIRS})

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

set(TEST_FILES
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
