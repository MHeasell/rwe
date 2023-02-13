rem Build the launcher
REM  cd launcher || goto :error
REM  echo node version || goto :error
REM  CALL node --version || goto :error
REM  echo npm version || goto :error
REM  CALL npm --version || goto :error
REM  CALL npm ci || goto :error
REM  CALL npm run tsc || goto :error
REM  CALL npm test || goto :error
REM  CALL npm run lint || goto :error
REM  CALL npm run package || goto :error
REM  cd .. || goto :error

rem Build RWE

cd "%APPVEYOR_BUILD_FOLDER%" || goto :error
IF "%RWE_COMPILER%"=="MSYS" (
    rem Required for bash to set itself up

    rem Core update (in case any core packages are outdated)
    C:\msys64\usr\bin\bash -lc "pacman --noconfirm -Syuu" || goto :error
    rem Normal update
    C:\msys64\usr\bin\bash -lc "pacman --noconfirm -Syuu" || goto :error

    SET CHERE_INVOKING=yes
    SET MSYSTEM=MINGW64
    C:\msys64\usr\bin\bash -lc "cd $APPVEYOR_BUILD_FOLDER && ./appveyor.bash" || goto :error

    goto :EOF
) ELSE IF "%RWE_COMPILER%"=="MSVC" (
    rem Fetch libs bundle
    IF NOT EXIST "%APPVEYOR_BUILD_FOLDER%\libs\_msvc" (
        C:\Python36-x64\python.exe fetch-msvc-libs.py || goto :error
    )

    mkdir build || goto :error
    cd build || goto :error
    cmake --version || goto :error
    cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=%Configuration% .. || goto :error
    msbuild "Robot War Engine.sln" /m /p:Configuration=%Configuration% || goto :error
    %Configuration%\rwe_test.exe || goto :error

    goto :EOF
) ELSE (
    echo "Unrecognised RWE_COMPILER: %RWE_COMPILER%"
    exit /b 1
)

:error
exit /b %errorlevel%
