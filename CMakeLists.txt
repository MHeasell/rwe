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
# rapidcheck relies on result_of, which is deprecated in favour of invoke_result.
# MSVC hides it from their stdlib unless you supply this option.
if(MSVC)
    add_definitions(-D_HAS_DEPRECATED_RESULT_OF)
endif()
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

    set(Boost_INCLUDE_DIRS "libs/_msvc/boost_1_74_0")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(Boost_LIBRARIES "${CMAKE_SOURCE_DIR}/libs/_msvc/boost_1_74_0/lib64-msvc-14.2/libboost_filesystem-vc142-mt-gd-x64-1_74.lib;${CMAKE_SOURCE_DIR}/libs/_msvc/boost_1_74_0/lib64-msvc-14.2/libboost_system-vc142-mt-gd-x64-1_74.lib;${CMAKE_SOURCE_DIR}/libs/_msvc/boost_1_74_0/lib64-msvc-14.2/libboost_program_options-vc142-mt-gd-x64-1_74.lib")
    else()
        set(Boost_LIBRARIES "${CMAKE_SOURCE_DIR}/libs/_msvc/boost_1_74_0/lib64-msvc-14.2/libboost_filesystem-vc142-mt-x64-1_74.lib;${CMAKE_SOURCE_DIR}/libs/_msvc/boost_1_74_0/lib64-msvc-14.2/libboost_system-vc142-mt-x64-1_74.lib;${CMAKE_SOURCE_DIR}/libs/_msvc/boost_1_74_0/lib64-msvc-14.2/libboost_program_options-vc142-mt-x64-1_74.lib")
    endif()
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
    find_windows_dll(MODPLUG_DLL "libmodplug-1.dll")
    find_windows_dll(OPUSFILE_DLL "libopusfile-0.dll")
    find_windows_dll(OPUS_DLL "libopus-0.dll")
    find_windows_dll(READLINE_DLL "libreadline8.dll")
    find_windows_dll(SNDFILE_DLL "libsndfile-1.dll")
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
    find_windows_dll(SSP_DLL "libssp-0.dll")
    find_windows_dll(DEFLATE_DLL "libdeflate.dll")
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

file(GLOB IMGUI_SOURCE_FILES
    ${PROJECT_SOURCE_DIR}/libs/imgui/*.cpp
    )

set(IMGUI_IMPL_FILES
    ${PROJECT_SOURCE_DIR}/libs/imgui/examples/imgui_impl_sdl.cpp
    ${PROJECT_SOURCE_DIR}/libs/imgui/examples/imgui_impl_opengl3.cpp
    )

set(SOURCE_FILES
    ${IMGUI_IMPL_FILES}
    ${IMGUI_SOURCE_FILES}
    src/rwe/AbstractViewport.cpp
    src/rwe/AbstractViewport.h
    src/rwe/AudioService.cpp
    src/rwe/AudioService.h
    src/rwe/BoxTreeSplit.cpp
    src/rwe/BoxTreeSplit.h
    src/rwe/ColorPalette.cpp
    src/rwe/ColorPalette.h
    src/rwe/CroppedViewport.cpp
    src/rwe/CroppedViewport.h
    src/rwe/CursorService.cpp
    src/rwe/CursorService.h
    src/rwe/FeatureMediaInfo.cpp
    src/rwe/FeatureMediaInfo.h
    src/rwe/FlashEffect.cpp
    src/rwe/FlashEffect.h
    src/rwe/GameCameraState.cpp
    src/rwe/GameCameraState.h
    src/rwe/GameHash.cpp
    src/rwe/GameHash.h
    src/rwe/GameHash_util.cpp
    src/rwe/GameHash_util.h
    src/rwe/GameNetworkService.cpp
    src/rwe/GameNetworkService.h
    src/rwe/GameScene.cpp
    src/rwe/GameScene.h
    src/rwe/GameScene_util.cpp
    src/rwe/GameScene_util.h
    src/rwe/GlobalConfig.cpp
    src/rwe/GlobalConfig.h
    src/rwe/ImGuiContext.cpp
    src/rwe/ImGuiContext.h
    src/rwe/InGameSoundsInfo.cpp
    src/rwe/InGameSoundsInfo.h
    src/rwe/Index.h
    src/rwe/LoadingNetworkService.cpp
    src/rwe/LoadingNetworkService.h
    src/rwe/LoadingScene.cpp
    src/rwe/LoadingScene.h
    src/rwe/MainMenuModel.cpp
    src/rwe/MainMenuModel.h
    src/rwe/MainMenuScene.cpp
    src/rwe/MainMenuScene.h
    src/rwe/MapTerrainGraphics.cpp
    src/rwe/MapTerrainGraphics.h
    src/rwe/Mesh.cpp
    src/rwe/Mesh.h
    src/rwe/MeshDatabase.cpp
    src/rwe/MeshDatabase.h
    src/rwe/MeshService.cpp
    src/rwe/MeshService.h
    src/rwe/MinHeap.h
    src/rwe/MovementClassCollisionService.cpp
    src/rwe/MovementClassCollisionService.h
    src/rwe/OpaqueField.h
    src/rwe/OpaqueId.h
    src/rwe/OpaqueId_io.h
    src/rwe/OpaqueUnit.h
    src/rwe/PathMapping.cpp
    src/rwe/PathMapping.h
    src/rwe/PlayerColorIndex.cpp
    src/rwe/PlayerColorIndex.h
    src/rwe/PlayerCommand.h
    src/rwe/PlayerCommandService.cpp
    src/rwe/PlayerCommandService.h
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
    src/rwe/ShaderService.cpp
    src/rwe/ShaderService.h
    src/rwe/SharedHandle.h
    src/rwe/SideData.cpp
    src/rwe/SideData.h
    src/rwe/SimpleVectorMap.h
    src/rwe/SoundClass.cpp
    src/rwe/SoundClass.h
    src/rwe/TextureService.cpp
    src/rwe/TextureService.h
    src/rwe/UiRenderService.cpp
    src/rwe/UiRenderService.h
    src/rwe/UniqueHandle.h
    src/rwe/UnitDatabase.cpp
    src/rwe/UnitDatabase.h
    src/rwe/UnitFactory.cpp
    src/rwe/UnitFactory.h
    src/rwe/UnitModelDefinition.cpp
    src/rwe/UnitModelDefinition.h
    src/rwe/UnitPieceMeshInfo.cpp
    src/rwe/UnitPieceMeshInfo.h
    src/rwe/UnitSoundType.h
    src/rwe/VectorMap.cpp
    src/rwe/VectorMap.h
    src/rwe/Viewport.cpp
    src/rwe/Viewport.h
    src/rwe/WeaponMediaInfo.cpp
    src/rwe/WeaponMediaInfo.h
    src/rwe/atlas_util.cpp
    src/rwe/atlas_util.h
    src/rwe/camera_util.cpp
    src/rwe/camera_util.h
    src/rwe/cob/CobAngle.h
    src/rwe/cob/CobAngularSpeed.h
    src/rwe/cob/CobAxis.h
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
    src/rwe/cob/CobSfxType.h
    src/rwe/cob/CobSleepDuration.h
    src/rwe/cob/CobSpeed.h
    src/rwe/cob/CobThread.cpp
    src/rwe/cob/CobThread.h
    src/rwe/cob/CobTime.h
    src/rwe/cob/CobValueId.h
    src/rwe/cob/cob_util.cpp
    src/rwe/cob/cob_util.h
    src/rwe/collection_util.h
    src/rwe/dump_util.cpp
    src/rwe/dump_util.h
    src/rwe/events.cpp
    src/rwe/events.h
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
    src/rwe/grid/DiscreteRect.cpp
    src/rwe/grid/DiscreteRect.h
    src/rwe/grid/EightWayDirection.cpp
    src/rwe/grid/EightWayDirection.h
    src/rwe/grid/Grid.h
    src/rwe/grid/Point.cpp
    src/rwe/grid/Point.h
    src/rwe/io/_3do/_3do.cpp
    src/rwe/io/_3do/_3do.h
    src/rwe/io/cob/Cob.cpp
    src/rwe/io/cob/Cob.h
    src/rwe/io/fbi/FbiAnglePerTick.h
    src/rwe/io/fbi/FbiDistancePerTick.h
    src/rwe/io/fbi/FbiDistancePerTickSquared.h
    src/rwe/io/fbi/UnitFbi.h
    src/rwe/io/fbi/io.cpp
    src/rwe/io/fbi/io.h
    src/rwe/io/featuretdf/FeatureTdf.h
    src/rwe/io/featuretdf/io.cpp
    src/rwe/io/featuretdf/io.h
    src/rwe/io/fnt/Fnt.cpp
    src/rwe/io/fnt/Fnt.h
    src/rwe/io/gaf/GafArchive.cpp
    src/rwe/io/gaf/GafArchive.h
    src/rwe/io/gaf/GafReaderAdapter.h
    src/rwe/io/gaf/gaf_headers.h
    src/rwe/io/gaf/gaf_util.cpp
    src/rwe/io/gaf/gaf_util.h
    src/rwe/io/gui/gui.cpp
    src/rwe/io/gui/gui.h
    src/rwe/io/hpi/HpiArchive.cpp
    src/rwe/io/hpi/HpiArchive.h
    src/rwe/io/hpi/hpi_headers.h
    src/rwe/io/hpi/hpi_util.cpp
    src/rwe/io/hpi/hpi_util.h
    src/rwe/io/ota/ota.cpp
    src/rwe/io/ota/ota.h
    src/rwe/io/pcx/pcx.cpp
    src/rwe/io/pcx/pcx.h
    src/rwe/io/tdf/ListTdfAdapter.cpp
    src/rwe/io/tdf/ListTdfAdapter.h
    src/rwe/io/tdf/NetSchemaTdfAdapter.cpp
    src/rwe/io/tdf/NetSchemaTdfAdapter.h
    src/rwe/io/tdf/SimpleTdfAdapter.cpp
    src/rwe/io/tdf/SimpleTdfAdapter.h
    src/rwe/io/tdf/TdfBlock.cpp
    src/rwe/io/tdf/TdfBlock.h
    src/rwe/io/tdf/TdfParser.cpp
    src/rwe/io/tdf/TdfParser.h
    src/rwe/io/tdf/tdf.cpp
    src/rwe/io/tdf/tdf.h
    src/rwe/io/tnt/TntArchive.cpp
    src/rwe/io/tnt/TntArchive.h
    src/rwe/io/weapontdf/WeaponTdf.cpp
    src/rwe/io/weapontdf/WeaponTdf.h
    src/rwe/io_utils.cpp
    src/rwe/io_utils.h
    src/rwe/ip_util.cpp
    src/rwe/ip_util.h
    src/rwe/match.h
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
    src/rwe/network_util.cpp
    src/rwe/network_util.h
    src/rwe/observable/BehaviorSubject.h
    src/rwe/observable/Observable.h
    src/rwe/observable/Subject.h
    src/rwe/observable/Subscription.h
    src/rwe/optional_io.h
    src/rwe/optional_util.h
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
    src/rwe/proto/serialization.cpp
    src/rwe/proto/serialization.h
    src/rwe/range_util.h
    src/rwe/render/FrameBufferHandle.h
    src/rwe/render/GlIdentifier.h
    src/rwe/render/GlMesh.cpp
    src/rwe/render/GlMesh.h
    src/rwe/render/GraphicsContext.cpp
    src/rwe/render/GraphicsContext.h
    src/rwe/render/OpenGlVersion.h
    src/rwe/render/RenderBufferHandle.h
    src/rwe/render/ShaderHandle.h
    src/rwe/render/ShaderMesh.cpp
    src/rwe/render/ShaderMesh.h
    src/rwe/render/ShaderProgramHandle.h
    src/rwe/render/Sprite.cpp
    src/rwe/render/Sprite.h
    src/rwe/render/SpriteSeries.cpp
    src/rwe/render/SpriteSeries.h
    src/rwe/render/TextureArrayHandle.h
    src/rwe/render/TextureArrayRegion.h
    src/rwe/render/TextureHandle.h
    src/rwe/render/TextureRegion.cpp
    src/rwe/render/TextureRegion.h
    src/rwe/render/UniformLocation.h
    src/rwe/render/VaoHandle.h
    src/rwe/render/VboHandle.h
    src/rwe/resource_io.cpp
    src/rwe/resource_io.h
    src/rwe/rwe_string.cpp
    src/rwe/rwe_string.h
    src/rwe/rwe_time.cpp
    src/rwe/rwe_time.h
    src/rwe/sim/Energy.h
    src/rwe/sim/FeatureDefinition.cpp
    src/rwe/sim/FeatureDefinition.h
    src/rwe/sim/FeatureDefinitionId.h
    src/rwe/sim/FeatureId.h
    src/rwe/sim/GameSimulation.cpp
    src/rwe/sim/GameSimulation.h
    src/rwe/sim/GameTime.cpp
    src/rwe/sim/GameTime.h
    src/rwe/sim/MapFeature.cpp
    src/rwe/sim/MapFeature.h
    src/rwe/sim/MapTerrain.cpp
    src/rwe/sim/MapTerrain.h
    src/rwe/sim/Metal.h
    src/rwe/sim/MovementClass.cpp
    src/rwe/sim/MovementClass.h
    src/rwe/sim/MovementClassId.h
    src/rwe/sim/OccupiedGrid.cpp
    src/rwe/sim/OccupiedGrid.h
    src/rwe/sim/Particle.cpp
    src/rwe/sim/Particle.h
    src/rwe/sim/PlayerId.h
    src/rwe/sim/Projectile.cpp
    src/rwe/sim/Projectile.h
    src/rwe/sim/ProjectileId.h
    src/rwe/sim/ProjectilePhysicsType.h
    src/rwe/sim/SimAngle.cpp
    src/rwe/sim/SimAngle.h
    src/rwe/sim/SimScalar.cpp
    src/rwe/sim/SimScalar.h
    src/rwe/sim/SimTicksPerSecond.h
    src/rwe/sim/SimVector.h
    src/rwe/sim/Unit.cpp
    src/rwe/sim/Unit.h
    src/rwe/sim/UnitBehaviorService.cpp
    src/rwe/sim/UnitBehaviorService.h
    src/rwe/sim/UnitFireOrders.h
    src/rwe/sim/UnitId.h
    src/rwe/sim/UnitMesh.cpp
    src/rwe/sim/UnitMesh.h
    src/rwe/sim/UnitOrder.h
    src/rwe/sim/UnitPieceDefinition.cpp
    src/rwe/sim/UnitPieceDefinition.h
    src/rwe/sim/UnitWeapon.h
    src/rwe/sim/WeaponDefinition.cpp
    src/rwe/sim/WeaponDefinition.h
    src/rwe/sim/movement.cpp
    src/rwe/sim/movement.h
    src/rwe/sim/unit_util.cpp
    src/rwe/sim/unit_util.h
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
    target_compile_options(librwe PUBLIC "/std:c++17")
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
    target_copy_file(librwe ${DEFLATE_DLL}) # MSYS2 libtiff depends on deflate
    target_copy_file(librwe ${MODPLUG_DLL}) # MSYS2 SDL2_mixer depends on modplug
    target_copy_file(librwe ${OPUSFILE_DLL}) # MSYS2 SDL2_mixer depends on opusfile
    target_copy_file(librwe ${SSP_DLL}) # MSYS2 SDL2_mixer libflac depends on ssp
    target_copy_file(librwe ${OPUS_DLL})
    target_copy_file(librwe ${READLINE_DLL})
    target_copy_file(librwe ${SNDFILE_DLL})
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

target_include_directories(librwe PUBLIC ${PROJECT_SOURCE_DIR}/libs/imgui)
target_include_directories(librwe PUBLIC ${PROJECT_SOURCE_DIR}/libs/imgui/examples)

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
    src/rwe/BoxTreeSplit.test.cpp
    src/rwe/GameHash_util.test.cpp
    src/rwe/GameScene_util.test.cpp
    src/rwe/MinHeap.test.cpp
    src/rwe/Result.test.cpp
    src/rwe/SideData.test.cpp
    src/rwe/VectorMap.test.cpp
    src/rwe/Viewport.test.cpp
    src/rwe/cob/cob_util.test.cpp
    src/rwe/dump_util.test.cpp
    src/rwe/geometry/BoundingBox3f.test.cpp
    src/rwe/geometry/Circle2f.test.cpp
    src/rwe/geometry/CollisionMesh.test.cpp
    src/rwe/geometry/Plane3f.test.cpp
    src/rwe/geometry/Ray3f.test.cpp
    src/rwe/geometry/Rectangle2f.test.cpp
    src/rwe/geometry/Triangle3f.test.cpp
    src/rwe/grid/DiscreteRect.test.cpp
    src/rwe/grid/EightWayDirection.test.cpp
    src/rwe/grid/Grid.test.cpp
    src/rwe/grid/Point.test.cpp
    src/rwe/io/featuretdf/io.test.cpp
    src/rwe/io/gui/gui.test.cpp
    src/rwe/io/ota/ota.test.cpp
    src/rwe/io/tdf/ListTdfAdapter.test.cpp
    src/rwe/io/tdf/NetSchemaTdfAdapter.test.cpp
    src/rwe/io/tdf/SimpleTdfAdapter.test.cpp
    src/rwe/io/tdf/TdfBlock.test.cpp
    src/rwe/ip_util.test.cpp
    src/rwe/math/Matrix4f.test.cpp
    src/rwe/math/Vector2f.test.cpp
    src/rwe/math/Vector3f.test.cpp
    src/rwe/math/rwe_math.test.cpp
    src/rwe/network_util.test.cpp
    src/rwe/pathfinding/pathfinding_utils.test.cpp
    src/rwe/rc_gen_optional.h
    src/rwe/rwe_string.test.cpp
    src/rwe/sim/SimAngle.test.cpp
    src/rwe/sim/SimVector.test.cpp
    src/rwe/sim/unit_util.test.cpp
    src/rwe/util.test.cpp
    )

add_executable(rwe_test src/test.cpp ${TEST_FILES})
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
    "launcher\\\\rwe-launcher;${PROJECT_NAME} Launcher ${RWE_GIT_DESCRIPTION}"
    )

set(CPACK_PACKAGE_VENDOR "Michael Heasell")

set(CPACK_OUTPUT_FILE_PREFIX "dist")

set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")

set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

include(CPack)
