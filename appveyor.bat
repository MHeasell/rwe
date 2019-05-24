rem Build the launcher
cd launcher || goto :error
echo node version || goto :error
CALL node --version || goto :error
echo npm version || goto :error
CALL npm --version || goto :error
CALL npm ci || goto :error
CALL npm run package || goto :error
cd .. || goto :error

rem Build RWE

cd "%APPVEYOR_BUILD_FOLDER%" || goto :error
IF "%RWE_COMPILER%"=="MSYS" (
    rem Required for bash to set itself up
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
