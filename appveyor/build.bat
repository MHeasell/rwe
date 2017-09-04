rem Required for bash to set itself up
SET MSYSTEM=MINGW64

rem Build the project
C:\msys64\usr\bin\bash -lc "cd $APPVEYOR_BUILD_FOLDER && mkdir build && cd build && cmake -G 'Unix Makefiles' .. && make -j 2 && ./rwe_test" || goto :error

rem Create the build artifacts
C:\msys64\usr\bin\bash -lc "cd $APPVEYOR_BUILD_FOLDER/build && make -j 2 package" || goto :error

goto :EOF

:error
exit /b %errorlevel%
