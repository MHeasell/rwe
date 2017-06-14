cmake_minimum_required(VERSION 3.8)
project(rwe)
enable_testing()

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES
    src/rwe/geometry/BoundingBox3f.cpp
    src/rwe/geometry/BoundingBox3f.h
    src/rwe/geometry/Plane3f.cpp
    src/rwe/geometry/Plane3f.h
    src/rwe/geometry/Ray3f.cpp
    src/rwe/geometry/Ray3f.h
    src/rwe/geometry/Triangle3f.cpp
    src/rwe/geometry/Triangle3f.h
    src/rwe/math/Vector2f.cpp
    src/rwe/math/Vector2f.h
    src/rwe/math/Vector3f.cpp
    src/rwe/math/Vector3f.h
    src/rwe/math/rwe_math.cpp
    src/rwe/math/rwe_math.h
)

add_library(librwe STATIC ${SOURCE_FILES})
set_target_properties(librwe PROPERTIES PREFIX "")
target_include_directories(librwe PUBLIC "src")
add_executable(rwe src/main.cpp)
target_link_libraries(rwe librwe)

set(TEST_FILES
    test/rwe/geometry/BoundingBox3f_test.cpp
    test/rwe/geometry/Plane3f_test.cpp
    test/rwe/geometry/Ray3f_test.cpp
    test/rwe/geometry/Triangle3f_test.cpp
    test/rwe/math/Vector3f_test.cpp
)

add_executable(rwe_test test/main.cpp ${TEST_FILES})
target_include_directories(rwe_test PRIVATE "libs/catch")
target_link_libraries(rwe_test librwe)
add_test(NAME rwe_test COMMAND rwe_test)
