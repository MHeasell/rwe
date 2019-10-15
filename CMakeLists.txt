cmake_minimum_required(VERSION 3.8)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Type of build" FORCE)
    message(STATUS "No build type specified, using default: ${CMAKE_BUILD_TYPE}")
else()
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

find_package(Git REQUIRED)

execute_process(
    COMMAND "${GIT_EXECUTABLE}" "describe" "--dirty=-d"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE RWE_GIT_DESCRIPTION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT RWE_GIT_DESCRIPTION)
    message(FATAL_ERROR "Failed to get the Git description")
endif()

if(NOT RWE_GIT_DESCRIPTION MATCHES "^v([0-9]+\\.[0-9]+\\.[0-9]+)")
    message(FATAL_ERROR "Failed to parse version from Git")
endif()

project("Robot War Engine" VERSION ${CMAKE_MATCH_1})
enable_testing()

set(RC_ENABLE_CATCH ON CACHE BOOL "Enables Catch support in RapidCheck" FORCE)
set(RC_ENABLE_BOOST ON CACHE BOOL "Enables Boost support in RapidCheck" FORCE)
add_subdirectory("libs/rapidcheck")

set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory("libs/json")

function(find_windows_dll var dll)
    if(WIN32)
        find_program(${var} ${dll})

        if(${${var}} STREQUAL "${var}-NOTFOUND")
            message(FATAL_ERROR "${dll} not found")
        endif()

        message("Found ${dll}: ${${var}}")
    else()
        set(${var} "${var}-NOTFOUND" CACHE PATH "path to ${dll}")
    endif()
endfunction()

function(target_copy_file target file)
    if(WIN32)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${file} ${PROJECT_BINARY_DIR})

        get_filename_component("${file}_NAME" ${file} NAME)
        install(FILES ${PROJECT_BINARY_DIR}/${${file}_NAME} DESTINATION ".")
    endif()
endfunction()

function(target_copy_dll target dll)
    if(WIN32)
        find_windows_dll("${dll}_DLL" ${dll})
        target_copy_file(${target} ${${dll}_DLL})
    endif()
endfunction()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

if(NOT MSVC)
    set(CMAKE_CXX_STANDARD 17)
endif()

find_package(OpenGL REQUIRED)

if(MSVC)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(Protobuf_LIBRARY "${CMAKE_SOURCE_DIR}/libs/_msvc/protobuf-3.5.1/lib/libprotobufd.lib")
    else()
        set(Protobuf_LIBRARY "${CMAKE_SOURCE_DIR}/libs/_msvc/protobuf-3.5.1/lib/libprotobuf.lib")
    endif()
    set(Protobuf_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libs/_msvc/protobuf-3.5.1/include")
    set(Protobuf_PROTOC_EXECUTABLE "${CMAKE_SOURCE_DIR}/libs/_msvc/protobuf-3.5.1/bin/protoc.exe")

    set(GLEW_DLL "${CMAKE_SOURCE_DIR}/libs/_msvc/glew-2.1.0/bin/Release/x64/glew32.dll")
    set(GLEW_LIBRARIES "${CMAKE_SOURCE_DIR}/libs/_msvc/glew-2.1.0/lib/Release/x64/glew32.lib")
    set(GLEW_INCLUDE_DIRS "libs/_msvc/glew-2.1.0/include")

    set(Boost_INCLUDE_DIRS "libs/_msvc/boost_1_66_0")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(Boost_LIBRARIES "${CMAKE_SOURCE_DIR}/libs/_msvc/boost_1_66_0/lib64-msvc-14.1/libboost_filesystem-vc141-mt-gd-x64-1_66.lib;${CMAKE_SOURCE_DIR}/libs/_msvc/boost_1_66_0/lib64-msvc-14.1/libboost_system-vc141-mt-gd-x64-1_66.lib;${CMAKE_SOURCE_DIR}/libs/_msvc/boost_1_66_0/lib64-msvc-14.1/libboost_program_options-vc141-mt-gd-x64-1_66.lib")
    else()
        set(Boost_LIBRARIES "${CMAKE_SOURCE_DIR}/libs/_msvc/boost_1_66_0/lib64-msvc-14.1/libboost_filesystem-vc141-mt-x64-1_66.lib;${CMAKE_SOURCE_DIR}/libs/_msvc/boost_1_66_0/lib64-msvc-14.1/libboost_system-vc141-mt-x64-1_66.lib;${CMAKE_SOURCE_DIR}/libs/_msvc/boost_1_66_0/lib64-msvc-14.1/libboost_program_options-vc141-mt-x64-1_66.lib")
    endif()
    add_definitions(-DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)
    add_definitions(-DBOOST_DATE_TIME_NO_LIB)
    add_definitions(-DBOOST_REGEX_NO_LIB)

    set(SDL2_DLL "${CMAKE_SOURCE_DIR}/libs/_msvc/SDL2-2.0.7/lib/x64/SDL2.dll")
    set(SDL2_LIBRARY "${CMAKE_SOURCE_DIR}/libs/_msvc/SDL2-2.0.7/lib/x64/SDL2.lib;${CMAKE_SOURCE_DIR}/libs/_msvc/SDL2-2.0.7/lib/x64/SDL2main.lib")
    set(SDL2_INCLUDE_DIR "libs/_msvc/SDL2-2.0.7/include")

    set(SDL2_IMAGE_DLL "${CMAKE_SOURCE_DIR}/libs/_msvc/SDL2_image-2.0.2/lib/x64/SDL2_image.dll")
    set(SDL2_IMAGE_LIBRARIES "${CMAKE_SOURCE_DIR}/libs/_msvc/SDL2_image-2.0.2/lib/x64/SDL2_image.lib")
    set(SDL2_IMAGE_INCLUDE_DIRS "libs/_msvc/SDL2_image-2.0.2/include")

    set(SDL2_MIXER_DLL "${CMAKE_SOURCE_DIR}/libs/_msvc/SDL2_mixer-2.0.2/lib/x64/SDL2_mixer.dll")
    set(SDL2_MIXER_LIBRARIES "${CMAKE_SOURCE_DIR}/libs/_msvc/SDL2_mixer-2.0.2/lib/x64/SDL2_mixer.lib")
    set(SDL2_MIXER_INCLUDE_DIRS "libs/_msvc/SDL2_mixer-2.0.2/include")

    set(ZLIB_DLL "${CMAKE_SOURCE_DIR}/libs/_msvc/zlib-1.2.11/lib/zlib1.dll")
    set(ZLIB_LIBRARIES "${CMAKE_SOURCE_DIR}/libs/_msvc/zlib-1.2.11/lib/zdll.lib")
    set(ZLIB_INCLUDE_DIRS "libs/_msvc/zlib-1.2.11/include")

    set(PNG_DLL "${CMAKE_SOURCE_DIR}/libs/_msvc/lpng1634/lib/libpng16-16.dll")
    set(PNG_LIBRARIES "${CMAKE_SOURCE_DIR}/libs/_msvc/lpng1634/lib/libpng16-16.lib")
    set(PNG_INCLUDE_DIRS "libs/_msvc/lpng1634/include")

    set(FLAC_DLL "${CMAKE_SOURCE_DIR}/libs/_msvc/extras/libFLAC-8.dll")
    set(JPEG_DLL "${CMAKE_SOURCE_DIR}/libs/_msvc/extras/libjpeg-9.dll")
    set(MPG_DLL "${CMAKE_SOURCE_DIR}/libs/_msvc/extras/libmpg123-0.dll")
    set(OGG_DLL "${CMAKE_SOURCE_DIR}/libs/_msvc/extras/libogg-0.dll")
    set(TIFF_DLL "${CMAKE_SOURCE_DIR}/libs/_msvc/extras/libtiff-5.dll")
    set(LZMA_DLL "LZMA_DLL-NOTFOUND") # bundled libtiff doesn't depend on liblzma
    set(VORBIS_DLL "${CMAKE_SOURCE_DIR}/libs/_msvc/extras/libvorbis-0.dll")
    set(VORBISFILE_DLL "${CMAKE_SOURCE_DIR}/libs/_msvc/extras/libvorbisfile-3.dll")
    set(WEBP_DLL "${CMAKE_SOURCE_DIR}/libs/_msvc/extras/libwebp-7.dll")
else()
    set(Protobuf_LIBRARY "${CMAKE_SOURCE_DIR}/libs/_protobuf-install/lib/libprotobuf.a")
    set(Protobuf_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/libs/_protobuf-install/include")
    if(WIN32)
        set(Protobuf_PROTOC_EXECUTABLE "${CMAKE_SOURCE_DIR}/libs/_protobuf-install/bin/protoc.exe")
    else()
        set(Protobuf_PROTOC_EXECUTABLE "${CMAKE_SOURCE_DIR}/libs/_protobuf-install/bin/protoc")
    endif()

    find_package(GLEW REQUIRED)
    find_windows_dll(GLEW_DLL "glew32.dll")

    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_NO_BOOST_CMAKE ON)
    find_package(Boost 1.54.0 REQUIRED COMPONENTS filesystem program_options)

    find_package(SDL2 REQUIRED)
    find_windows_dll(SDL2_DLL "SDL2.dll")
    find_package(SDL2_image REQUIRED)
    find_windows_dll(SDL2_IMAGE_DLL "SDL2_image.dll")
    find_package(SDL2_mixer REQUIRED)
    find_windows_dll(SDL2_MIXER_DLL "SDL2_mixer.dll")

    find_package(ZLIB REQUIRED)
    find_windows_dll(ZLIB_DLL "zlib1.dll")
    find_package(PNG REQUIRED)
    find_windows_dll(PNG_DLL "libpng16-16.dll")

    find_windows_dll(FLAC_DLL "libFLAC-8.dll")
    find_windows_dll(JPEG_DLL "libjpeg-8.dll")
    find_windows_dll(MPG_DLL "libmpg123-0.dll")
    find_windows_dll(OGG_DLL "libogg-0.dll")
    find_windows_dll(TIFF_DLL "libtiff-5.dll")
    find_windows_dll(LZMA_DLL "liblzma-5.dll")
    find_windows_dll(VORBIS_DLL "libvorbis-0.dll")
    find_windows_dll(VORBISFILE_DLL "libvorbisfile-3.dll")
    find_windows_dll(WEBP_DLL "libwebp-7.dll")
    find_windows_dll(ZSTD_DLL "libzstd.dll")
    find_windows_dll(FLUIDSYNTH_DLL "libfluidsynth-2.dll")
    find_windows_dll(MODPLUG_DLL "libmodplug-1.dll")
    find_windows_dll(OPUSFILE_DLL "libopusfile-0.dll")
    find_windows_dll(OPUS_DLL "libopus-0.dll")
    find_windows_dll(PORTAUDIO_DLL "libportaudio-2.dll")
    find_windows_dll(READLINE_DLL "libreadline8.dll")
    find_windows_dll(SNDFILE_DLL "libsndfile-1.dll")
    find_windows_dll(SPEEX_DLL "libspeex-1.dll")
    find_windows_dll(VORBISENC_DLL "libvorbisenc-2.dll")
    find_windows_dll(TERMCAP_DLL "libtermcap-0.dll")
    find_windows_dll(GLIB2_DLL "libglib-2.0-0.dll")
    find_windows_dll(GMODULE2_DLL "libgmodule-2.0-0.dll")
    find_windows_dll(STDC++_DLL "libstdc++-6.dll")
    find_windows_dll(GCC_S_SEH_DLL "libgcc_s_seh-1.dll")
    find_windows_dll(WINPTHREAD_DLL "libwinpthread-1.dll")
    find_windows_dll(INTL_DLL "libintl-8.dll")
    find_windows_dll(PCRE_DLL "libpcre-1.dll")
    find_windows_dll(ICONV_DLL "libiconv-2.dll")
endif()

# We should have std::chrono everywhere,
# but boost sometimes fails to detect this (e.g. with clang 5 on Ubuntu)
# and falls back to using boost::chrono.
# This enables the use of std::chrono explicitly
# in boost.asio.
add_definitions(-DBOOST_ASIO_HAS_STD_CHRONO)

set(Protobuf_USE_STATIC_LIBS ON)
find_package(Protobuf REQUIRED)

protobuf_generate_cpp(PROTO_SOURCE_FILES PROTO_HEADER_FILES proto/network.proto)
message("proto src files: ${PROTO_SOURCE_FILES}")
message("proto header files: ${PROTO_HEADER_FILES}")

set(SOURCE_FILES
    src/rwe/AudioService.cpp
    src/rwe/AudioService.h
    src/rwe/BoxTreeSplit.cpp
    src/rwe/BoxTreeSplit.h
    src/rwe/Cob.cpp
    src/rwe/Cob.h
    src/rwe/ColorPalette.cpp
    src/rwe/ColorPalette.h
    src/rwe/CursorService.cpp
    src/rwe/CursorService.h
    src/rwe/DiscreteRect.cpp
    src/rwe/DiscreteRect.h
    src/rwe/EightWayDirection.cpp
    src/rwe/EightWayDirection.h
    src/rwe/Energy.h
    src/rwe/Explosion.cpp
    src/rwe/Explosion.h
    src/rwe/FeatureDefinition.cpp
    src/rwe/FeatureDefinition.h
    src/rwe/FeatureId.h
    src/rwe/Fnt.cpp
    src/rwe/Fnt.h
    src/rwe/Gaf.cpp
    src/rwe/Gaf.h
    src/rwe/GameHash.cpp
    src/rwe/GameHash.h
    src/rwe/GameHash_util.cpp
    src/rwe/GameHash_util.h
    src/rwe/GameNetworkService.cpp
    src/rwe/GameNetworkService.h
    src/rwe/GameScene.cpp
    src/rwe/GameScene.h
    src/rwe/GameSimulation.cpp
    src/rwe/GameSimulation.h
    src/rwe/GameTime.cpp
    src/rwe/GameTime.h
    src/rwe/GlIdentifier.h
    src/rwe/GlMesh.cpp
    src/rwe/GlMesh.h
    src/rwe/GraphicsContext.cpp
    src/rwe/GraphicsContext.h
    src/rwe/Grid.h
    src/rwe/Hpi.cpp
    src/rwe/Hpi.h
    src/rwe/InGameSoundsInfo.cpp
    src/rwe/InGameSoundsInfo.h
    src/rwe/LaserProjectile.cpp
    src/rwe/LaserProjectile.h
    src/rwe/LoadingNetworkService.cpp
    src/rwe/LoadingNetworkService.h
    src/rwe/LoadingScene.cpp
    src/rwe/LoadingScene.h
    src/rwe/MainMenuModel.cpp
    src/rwe/MainMenuModel.h
    src/rwe/MainMenuScene.cpp
    src/rwe/MainMenuScene.h
    src/rwe/MapFeature.cpp
    src/rwe/MapFeature.h
    src/rwe/MapFeatureService.cpp
    src/rwe/MapFeatureService.h
    src/rwe/MapTerrain.cpp
    src/rwe/MapTerrain.h
    src/rwe/Mesh.cpp
    src/rwe/Mesh.h
    src/rwe/MeshService.cpp
    src/rwe/MeshService.h
    src/rwe/Metal.h
    src/rwe/MinHeap.h
    src/rwe/MovementClass.cpp
    src/rwe/MovementClass.h
    src/rwe/MovementClassCollisionService.cpp
    src/rwe/MovementClassCollisionService.h
    src/rwe/MovementClassId.h
    src/rwe/OccupiedGrid.cpp
    src/rwe/OccupiedGrid.h
    src/rwe/OpaqueField.h
    src/rwe/OpaqueId.h
    src/rwe/OpaqueId_io.h
    src/rwe/OpaqueUnit.h
    src/rwe/OpenGlVersion.h
    src/rwe/PlayerColorIndex.cpp
    src/rwe/PlayerColorIndex.h
    src/rwe/PlayerCommand.h
    src/rwe/PlayerCommandService.cpp
    src/rwe/PlayerCommandService.h
    src/rwe/PlayerId.h
    src/rwe/Point.cpp
    src/rwe/Point.h
    src/rwe/ProjectileId.h
    src/rwe/ProjectileRenderType.h
    src/rwe/RadiansAngle.cpp
    src/rwe/RadiansAngle.h
    src/rwe/RenderService.cpp
    src/rwe/RenderService.h
    src/rwe/Result.h
    src/rwe/SceneContext.h
    src/rwe/SceneManager.cpp
    src/rwe/SceneManager.h
    src/rwe/SceneTime.cpp
    src/rwe/SceneTime.h
    src/rwe/SdlContextManager.cpp
    src/rwe/SdlContextManager.h
    src/rwe/SelectionMesh.h
    src/rwe/ShaderHandle.h
    src/rwe/ShaderMesh.cpp
    src/rwe/ShaderMesh.h
    src/rwe/ShaderProgramHandle.h
    src/rwe/ShaderService.cpp
    src/rwe/ShaderService.h
    src/rwe/SharedHandle.h
    src/rwe/SideData.cpp
    src/rwe/SideData.h
    src/rwe/SimAngle.cpp
    src/rwe/SimAngle.h
    src/rwe/SimScalar.cpp
    src/rwe/SimScalar.h
    src/rwe/SimVector.h
    src/rwe/SoundClass.cpp
    src/rwe/SoundClass.h
    src/rwe/Sprite.cpp
    src/rwe/Sprite.h
    src/rwe/SpriteSeries.cpp
    src/rwe/SpriteSeries.h
    src/rwe/TextureHandle.h
    src/rwe/TextureRegion.cpp
    src/rwe/TextureRegion.h
    src/rwe/TextureService.cpp
    src/rwe/TextureService.h
    src/rwe/UiRenderService.cpp
    src/rwe/UiRenderService.h
    src/rwe/UniformLocation.h
    src/rwe/UniqueHandle.h
    src/rwe/Unit.cpp
    src/rwe/Unit.h
    src/rwe/UnitBehaviorService.cpp
    src/rwe/UnitBehaviorService.h
    src/rwe/UnitDatabase.cpp
    src/rwe/UnitDatabase.h
    src/rwe/UnitFactory.cpp
    src/rwe/UnitFactory.h
    src/rwe/UnitFireOrders.h
    src/rwe/UnitId.h
    src/rwe/UnitMesh.cpp
    src/rwe/UnitMesh.h
    src/rwe/UnitOrder.h
    src/rwe/UnitWeapon.h
    src/rwe/VaoHandle.h
    src/rwe/VboHandle.h
    src/rwe/VectorMap.cpp
    src/rwe/VectorMap.h
    src/rwe/ViewportService.cpp
    src/rwe/ViewportService.h
    src/rwe/Weapon.h
    src/rwe/WeaponTdf.cpp
    src/rwe/WeaponTdf.h
    src/rwe/_3do.cpp
    src/rwe/_3do.h
    src/rwe/camera/AbstractCamera.cpp
    src/rwe/camera/AbstractCamera.h
    src/rwe/camera/CabinetCamera.cpp
    src/rwe/camera/CabinetCamera.h
    src/rwe/camera/UiCamera.cpp
    src/rwe/camera/UiCamera.h
    src/rwe/cob/CobAngle.h
    src/rwe/cob/CobAngularSpeed.h
    src/rwe/cob/CobConstants.h
    src/rwe/cob/CobEnvironment.cpp
    src/rwe/cob/CobEnvironment.h
    src/rwe/cob/CobExecutionContext.cpp
    src/rwe/cob/CobExecutionContext.h
    src/rwe/cob/CobExecutionService.cpp
    src/rwe/cob/CobExecutionService.h
    src/rwe/cob/CobFunction.cpp
    src/rwe/cob/CobFunction.h
    src/rwe/cob/CobOpCode.h
    src/rwe/cob/CobPosition.h
    src/rwe/cob/CobSleepDuration.h
    src/rwe/cob/CobSpeed.h
    src/rwe/cob/CobThread.cpp
    src/rwe/cob/CobThread.h
    src/rwe/cob/CobValueId.h
    src/rwe/cob/cob_util.cpp
    src/rwe/cob/cob_util.h
    src/rwe/collection_util.h
    src/rwe/dump_util.cpp
    src/rwe/dump_util.h
    src/rwe/events.cpp
    src/rwe/events.h
    src/rwe/fbi/FbiAnglePerTick.h
    src/rwe/fbi/FbiDistancePerTick.h
    src/rwe/fbi/FbiDistancePerTickSquared.h
    src/rwe/fbi/UnitFbi.cpp
    src/rwe/fbi/UnitFbi.h
    src/rwe/fixed_point.cpp
    src/rwe/fixed_point.h
    src/rwe/float_math.cpp
    src/rwe/float_math.h
    src/rwe/geometry/BoundingBox3f.h
    src/rwe/geometry/BoundingBox3x.h
    src/rwe/geometry/Circle2f.h
    src/rwe/geometry/Circle2x.h
    src/rwe/geometry/CollisionMesh.cpp
    src/rwe/geometry/CollisionMesh.h
    src/rwe/geometry/Line3f.h
    src/rwe/geometry/Line3x.h
    src/rwe/geometry/Plane3f.h
    src/rwe/geometry/Plane3x.h
    src/rwe/geometry/Ray3f.h
    src/rwe/geometry/Ray3x.h
    src/rwe/geometry/Rectangle2f.h
    src/rwe/geometry/Rectangle2x.h
    src/rwe/geometry/Triangle3f.h
    src/rwe/geometry/Triangle3x.h
    src/rwe/gui.cpp
    src/rwe/gui.h
    src/rwe/io_utils.cpp
    src/rwe/io_utils.h
    src/rwe/ip_util.cpp
    src/rwe/ip_util.h
    src/rwe/math/Matrix4f.h
    src/rwe/math/Matrix4x.h
    src/rwe/math/Vector2f.h
    src/rwe/math/Vector2x.h
    src/rwe/math/Vector3f.cpp
    src/rwe/math/Vector3f.h
    src/rwe/math/Vector3x.h
    src/rwe/math/rwe_math.cpp
    src/rwe/math/rwe_math.h
    src/rwe/matrix_util.cpp
    src/rwe/matrix_util.h
    src/rwe/movement.cpp
    src/rwe/movement.h
    src/rwe/network_util.cpp
    src/rwe/network_util.h
    src/rwe/observable/BehaviorSubject.h
    src/rwe/observable/Observable.h
    src/rwe/observable/Subject.h
    src/rwe/observable/Subscription.h
    src/rwe/optional_io.h
    src/rwe/optional_util.h
    src/rwe/ota.cpp
    src/rwe/ota.h
    src/rwe/overloaded.h
    src/rwe/pathfinding/AStarPathFinder.h
    src/rwe/pathfinding/AbstractUnitPathFinder.cpp
    src/rwe/pathfinding/AbstractUnitPathFinder.h
    src/rwe/pathfinding/OctileDistance.cpp
    src/rwe/pathfinding/OctileDistance.h
    src/rwe/pathfinding/OctileDistance_io.cpp
    src/rwe/pathfinding/OctileDistance_io.h
    src/rwe/pathfinding/PathCost.cpp
    src/rwe/pathfinding/PathCost.h
    src/rwe/pathfinding/PathFindingService.cpp
    src/rwe/pathfinding/PathFindingService.h
    src/rwe/pathfinding/UnitPath.h
    src/rwe/pathfinding/UnitPathFinder.cpp
    src/rwe/pathfinding/UnitPathFinder.h
    src/rwe/pathfinding/UnitPerimeterPathFinder.cpp
    src/rwe/pathfinding/UnitPerimeterPathFinder.h
    src/rwe/pathfinding/pathfinding_utils.cpp
    src/rwe/pathfinding/pathfinding_utils.h
    src/rwe/pcx.cpp
    src/rwe/pcx.h
    src/rwe/proto/serialization.cpp
    src/rwe/proto/serialization.h
    src/rwe/range_util.h
    src/rwe/resource_io.cpp
    src/rwe/resource_io.h
    src/rwe/rwe_string.cpp
    src/rwe/rwe_string.h
    src/rwe/rwe_time.cpp
    src/rwe/rwe_time.h
    src/rwe/tdf.cpp
    src/rwe/tdf.h
    src/rwe/tdf/ListTdfAdapter.cpp
    src/rwe/tdf/ListTdfAdapter.h
    src/rwe/tdf/SimpleTdfAdapter.cpp
    src/rwe/tdf/SimpleTdfAdapter.h
    src/rwe/tdf/TdfBlock.cpp
    src/rwe/tdf/TdfBlock.h
    src/rwe/tdf/TdfParser.cpp
    src/rwe/tdf/TdfParser.h
    src/rwe/tnt/TntArchive.cpp
    src/rwe/tnt/TntArchive.h
    src/rwe/ui/UiComponent.cpp
    src/rwe/ui/UiComponent.h
    src/rwe/ui/UiFactory.cpp
    src/rwe/ui/UiFactory.h
    src/rwe/ui/UiLabel.cpp
    src/rwe/ui/UiLabel.h
    src/rwe/ui/UiLightBar.cpp
    src/rwe/ui/UiLightBar.h
    src/rwe/ui/UiListBox.cpp
    src/rwe/ui/UiListBox.h
    src/rwe/ui/UiPanel.cpp
    src/rwe/ui/UiPanel.h
    src/rwe/ui/UiScrollBar.cpp
    src/rwe/ui/UiScrollBar.h
    src/rwe/ui/UiStagedButton.cpp
    src/rwe/ui/UiStagedButton.h
    src/rwe/ui/UiSurface.cpp
    src/rwe/ui/UiSurface.h
    src/rwe/unit_util.cpp
    src/rwe/unit_util.h
    src/rwe/util.cpp
    src/rwe/util.h
    src/rwe/vertex_height.cpp
    src/rwe/vertex_height.h
    src/rwe/vfs/AbstractVirtualFileSystem.h
    src/rwe/vfs/AbstractVirtualFilesSystem.cpp
    src/rwe/vfs/CompositeVirtualFileSystem.cpp
    src/rwe/vfs/CompositeVirtualFileSystem.h
    src/rwe/vfs/DirectoryFileSystem.cpp
    src/rwe/vfs/DirectoryFileSystem.h
    src/rwe/vfs/HpiFileSystem.cpp
    src/rwe/vfs/HpiFileSystem.h
    )

if(WIN32)
    add_definitions(-DRWE_PLATFORM_WINDOWS)
else()
    add_definitions(-DRWE_PLATFORM_LINUX)
endif()

add_library(librwe STATIC ${SOURCE_FILES} ${PROTO_SOURCE_FILES} ${PROTO_HEADER_FILES})
set_target_properties(librwe PROPERTIES PREFIX "")
if(MSVC)
    target_compile_options(librwe PUBLIC "/std:c++latest")
    add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
else()
    target_compile_options(librwe PUBLIC "-Wall" "-Wextra")
endif()
target_include_directories(librwe PUBLIC "src")
configure_file("src/rwe/config.h.in" "config/rwe/config.h" @ONLY)
target_include_directories(librwe PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/config")

target_include_directories(librwe PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(librwe ${Boost_LIBRARIES})
if(WIN32)
    # winsock needed for boost asio on windows
    target_link_libraries(librwe ws2_32)
endif()

target_include_directories(librwe PUBLIC "libs/utfcpp/source")
target_include_directories(librwe PUBLIC "libs/spdlog/include")

target_link_libraries(librwe ${OPENGL_LIBRARIES})

target_copy_file(librwe ${GLEW_DLL})
if(MSVC)
    target_link_libraries(librwe ${GLEW_LIBRARIES})
    target_include_directories(librwe PUBLIC ${GLEW_INCLUDE_DIRS})
else()
    target_link_libraries(librwe GLEW::GLEW)
endif()

target_copy_file(librwe ${SDL2_DLL})
target_link_libraries(librwe ${SDL2_LIBRARY})
target_include_directories(librwe PUBLIC ${SDL2_INCLUDE_DIR})

target_copy_file(librwe ${SDL2_IMAGE_DLL})
target_link_libraries(librwe ${SDL2_IMAGE_LIBRARIES})
target_include_directories(librwe PUBLIC ${SDL2_IMAGE_INCLUDE_DIRS})

target_copy_file(librwe ${SDL2_MIXER_DLL})
target_link_libraries(librwe ${SDL2_MIXER_LIBRARIES})
target_include_directories(librwe PUBLIC ${SDL2_MIXER_INCLUDE_DIRS})

target_copy_file(librwe ${ZLIB_DLL})
target_link_libraries(librwe ${ZLIB_LIBRARIES})
target_include_directories(librwe PUBLIC ${ZLIB_INCLUDE_DIRS})

target_copy_file(librwe ${PNG_DLL})

target_copy_file(librwe ${FLAC_DLL})
target_copy_file(librwe ${JPEG_DLL})
target_copy_file(librwe ${MPG_DLL})
target_copy_file(librwe ${OGG_DLL})
target_copy_file(librwe ${TIFF_DLL})
if(WIN32 AND NOT MSVC)
    target_copy_file(librwe ${LZMA_DLL}) # MSYS2 libtiff depends on liblzma
    target_copy_file(librwe ${ZSTD_DLL}) # MSYS2 libtiff depends on zstd
    target_copy_file(librwe ${FLUIDSYNTH_DLL}) # MSYS2 SDL2_mixer depends on fluidsynth
    target_copy_file(librwe ${MODPLUG_DLL}) # MSYS2 SDL2_mixer depends on modplug
    target_copy_file(librwe ${OPUSFILE_DLL}) # MSYS2 SDL2_mixer depends on opusfile
    target_copy_file(librwe ${OPUS_DLL})
    target_copy_file(librwe ${PORTAUDIO_DLL})
    target_copy_file(librwe ${READLINE_DLL})
    target_copy_file(librwe ${SNDFILE_DLL})
    target_copy_file(librwe ${SPEEX_DLL})
    target_copy_file(librwe ${VORBISENC_DLL})
    target_copy_file(librwe ${TERMCAP_DLL})
    target_copy_file(librwe ${GLIB2_DLL})
    target_copy_file(librwe ${GMODULE2_DLL})
    target_copy_file(librwe ${STDC++_DLL})
    target_copy_file(librwe ${GCC_S_SEH_DLL})
    target_copy_file(librwe ${WINPTHREAD_DLL})
    target_copy_file(librwe ${INTL_DLL})
    target_copy_file(librwe ${PCRE_DLL})
    target_copy_file(librwe ${ICONV_DLL})
endif()
target_copy_file(librwe ${VORBIS_DLL})
target_copy_file(librwe ${VORBISFILE_DLL})
target_copy_file(librwe ${WEBP_DLL})

target_link_libraries(librwe ${Protobuf_LIBRARIES})
target_include_directories(librwe PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(librwe PUBLIC ${CMAKE_CURRENT_BINARY_DIR}) # for generated headers

target_link_libraries(librwe nlohmann_json::nlohmann_json)

add_custom_target(rwe_shaders)
add_custom_command(TARGET rwe_shaders PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/shaders" "${PROJECT_BINARY_DIR}/shaders")

add_executable(rwe src/main.cpp)
target_link_libraries(rwe librwe)
add_dependencies(rwe rwe_shaders)

add_executable(hpi_test src/hpi_test.cpp)
target_link_libraries(hpi_test librwe)

add_executable(vfs_test src/vfs_test.cpp)
target_link_libraries(vfs_test librwe)

add_executable(gaf_test src/gaf_test.cpp)
target_link_libraries(gaf_test librwe)
target_link_libraries(gaf_test ${PNG_LIBRARIES})
target_include_directories(gaf_test PUBLIC ${PNG_INCLUDE_DIRS})
target_include_directories(gaf_test PUBLIC libs/png++-0.2.9)
if(WIN32)
    target_compile_definitions(gaf_test PRIVATE __STDC_LIB_EXT1__=1)
endif()

add_executable(pcx_test src/pcx_test.cpp)
target_link_libraries(pcx_test librwe)
target_link_libraries(pcx_test ${PNG_LIBRARIES})
target_include_directories(pcx_test PUBLIC ${PNG_INCLUDE_DIRS})
target_include_directories(pcx_test PUBLIC libs/png++-0.2.9)
if(WIN32)
    target_compile_definitions(pcx_test PRIVATE __STDC_LIB_EXT1__=1)
endif()

add_executable(tnt_test src/tnt_test.cpp)
target_link_libraries(tnt_test librwe)
target_link_libraries(tnt_test ${PNG_LIBRARIES})
target_include_directories(tnt_test PUBLIC ${PNG_INCLUDE_DIRS})
target_include_directories(tnt_test PUBLIC libs/png++-0.2.9)
if(WIN32)
    target_compile_definitions(tnt_test PRIVATE __STDC_LIB_EXT1__=1)
endif()

add_executable(3do_test src/3do_test.cpp)
target_link_libraries(3do_test librwe)

add_executable(cob_test src/cob_test.cpp)
target_link_libraries(cob_test librwe)

add_executable(fnt_test src/fnt_test.cpp)
target_link_libraries(fnt_test librwe)
target_link_libraries(fnt_test ${PNG_LIBRARIES})
target_include_directories(fnt_test PUBLIC ${PNG_INCLUDE_DIRS})
target_include_directories(fnt_test PUBLIC libs/png++-0.2.9)
if(WIN32)
    target_compile_definitions(fnt_test PRIVATE __STDC_LIB_EXT1__=1)
endif()

add_executable(texture_test src/texture_test.cpp)
target_link_libraries(texture_test librwe)
target_link_libraries(texture_test ${PNG_LIBRARIES})
target_include_directories(texture_test PUBLIC ${PNG_INCLUDE_DIRS})
target_include_directories(texture_test PUBLIC libs/png++-0.2.9)
if(WIN32)
    target_compile_definitions(texture_test PRIVATE __STDC_LIB_EXT1__=1)
endif()

add_executable(rwe_bridge src/bridge.cpp)
target_link_libraries(rwe_bridge PRIVATE librwe)
target_link_libraries(rwe_bridge PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(rwe_bridge PRIVATE ${PNG_LIBRARIES})
target_include_directories(rwe_bridge PUBLIC ${PNG_INCLUDE_DIRS})
target_include_directories(rwe_bridge PUBLIC libs/png++-0.2.9)
if(WIN32)
    target_compile_definitions(rwe_bridge PRIVATE __STDC_LIB_EXT1__=1)
endif()

set(TEST_FILES
    test/rwe/BoxTreeSplit_test.cpp
    test/rwe/DiscreteRect_test.cpp
    test/rwe/EightWayDirection_test.cpp
    test/rwe/FeatureDefinition_test.cpp
    test/rwe/GameHash_util_test.cpp
    test/rwe/Grid_test.cpp
    test/rwe/ListTdfAdapter_test.cpp
    test/rwe/MinHeap_test.cpp
    test/rwe/Point_test.cpp
    test/rwe/Result_test.cpp
    test/rwe/SideData_test.cpp
    test/rwe/SimAngle_test.cpp
    test/rwe/SimVector_test.cpp
    test/rwe/SimpleTdfAdapter_test.cpp
    test/rwe/TdfBlock_test.cpp
    test/rwe/VectorMap_test.cpp
    test/rwe/ViewportService_test.cpp
    test/rwe/camera/CabinetCamera_test.cpp
    test/rwe/cob/cob_util_test.cpp
    test/rwe/dump_util_test.cpp
    test/rwe/geometry/BoundingBox3f_test.cpp
    test/rwe/geometry/Circle2f_test.cpp
    test/rwe/geometry/CollisionMesh_test.cpp
    test/rwe/geometry/Plane3f_test.cpp
    test/rwe/geometry/Ray3f_test.cpp
    test/rwe/geometry/Rectangle2f_test.cpp
    test/rwe/geometry/Triangle3f_test.cpp
    test/rwe/gui_test.cpp
    test/rwe/ip_util_test.cpp
    test/rwe/math/Matrix4f_test.cpp
    test/rwe/math/Vector2f_test.cpp
    test/rwe/math/Vector3f_test.cpp
    test/rwe/math/rwe_math_test.cpp
    test/rwe/network_util_test.cpp
    test/rwe/ota_test.cpp
    test/rwe/pathfinding/pathfinding_utils_test.cpp
    test/rwe/rc_gen_optional.h
    test/rwe/rwe_string_test.cpp
    test/rwe/unit_util_test.cpp
    test/rwe/util_test.cpp
    )

add_executable(rwe_test test/main.cpp ${TEST_FILES})
target_link_libraries(rwe_test Catch2::Catch2)
target_link_libraries(rwe_test rapidcheck_catch)
target_link_libraries(rwe_test rapidcheck_boost)
target_link_libraries(rwe_test librwe)
add_test(NAME rwe_test COMMAND rwe_test)

install(TARGETS rwe rwe_bridge
    RUNTIME DESTINATION .
    )
install(FILES LICENSE README.md DESTINATION .)

install(DIRECTORY shaders DESTINATION .)

install(DIRECTORY launcher/rwe-launcher-win32-x64/ DESTINATION launcher)

set(CPACK_GENERATOR "ZIP;NSIS")

set(CPACK_PACKAGE_VERSION ${RWE_GIT_DESCRIPTION})

set(CPACK_PACKAGE_FILE_NAME "RobotWarEngine-${RWE_GIT_DESCRIPTION}-${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_TYPE}")

set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${PROJECT_SOURCE_DIR}/README.md")

set(CPACK_PACKAGE_EXECUTABLES
    "rwe;${PROJECT_NAME} ${RWE_GIT_DESCRIPTION}"
    "launcher\\\\rwe-launcher;${PROJECT_NAME} Launcher ${RWE_GIT_DESCRIPTION}"
    )

set(CPACK_PACKAGE_VENDOR "Michael Heasell")

set(CPACK_OUTPUT_FILE_PREFIX "dist")

set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")

set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

include(CPack)
