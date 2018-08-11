rem Build the launcher
cd launcher || goto :error
CALL npm install || goto :error
CALL npm run package || goto :error
cd .. || goto :error

rem Build RWE

IF "%RWE_COMPILER%"=="MSYS" (
    rem Required for bash to set itself up
    SET MSYSTEM=MINGW64

    rem Refresh packages list
    C:\msys64\usr\bin\bash -lc "pacman -Syq" || goto :error

    rem Install build tools
    C:\msys64\usr\bin\bash -lc "pacman -Sq --needed --noconfirm make unzip mingw-w64-x86_64-cmake mingw-w64-x86_64-toolchain" || goto :error

    rem Install project dependencies
    C:\msys64\usr\bin\bash -lc "pacman -Sq --needed --noconfirm mingw-w64-x86_64-boost mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_image mingw-w64-x86_64-SDL2_mixer mingw-w64-x86_64-SDL2_net mingw-w64-x86_64-glew mingw-w64-x86_64-smpeg2 mingw-w64-x86_64-zlib mingw-w64-x86_64-libpng" || goto :error

    rem Build protobuf
    C:\msys64\usr\bin\bash -lc "cd $APPVEYOR_BUILD_FOLDER && cd libs && ./build-protobuf.sh" || goto :error

    rem Build the project
    C:\msys64\usr\bin\bash -lc "cd $APPVEYOR_BUILD_FOLDER && mkdir build && cd build && cmake -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=$Configuration .. && make -j 2 && ./rwe_test" || goto :error

    rem Create the build artifacts
    C:\msys64\usr\bin\bash -lc "cd $APPVEYOR_BUILD_FOLDER/build && make -j 2 package" || goto :error

    rem Push the build artifacts
    C:\msys64\usr\bin\bash -lc "cd $APPVEYOR_BUILD_FOLDER/build/dist && (for i in *; do appveyor PushArtifact "$i"; done)" || goto :error

    goto :EOF
) ELSE IF "%RWE_COMPILER%"=="MSVC" (
    cd "%APPVEYOR_BUILD_FOLDER%" || goto :error

    rem Fetch libs bundle
    IF NOT EXIST "%APPVEYOR_BUILD_FOLDER%\libs\_msvc" (
        C:\Python36-x64\python.exe fetch-msvc-libs.py || goto :error
    )

    mkdir build || goto :error
    cd build || goto :error
    cmake -G "Visual Studio 15 2017 Win64" -DCMAKE_BUILD_TYPE=%Configuration% .. || goto :error
    msbuild "Robot War Engine.sln" /m /p:Configuration=%Configuration% || goto :error
    %Configuration%\rwe_test.exe || goto :error

    goto :EOF
) ELSE (
    echo "Unrecognised RWE_COMPILER: %RWE_COMPILER%"
    exit /b 1
)

:error
exit /b %errorlevel%
