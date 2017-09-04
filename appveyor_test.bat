rem Required for bash to set itself up
SET MSYSTEM=MINGW64

rem Test the project
C:\msys64\usr\bin\bash -lc "cd $APPVEYOR_BUILD_FOLDER && cd build && ./rwe_test -r junit -o test_output.xml" || goto :error

C:\msys64\usr\bin\bash -lc "curl https://ci.appveyor.com/api/testresults/junit/$APPVEYOR_JOB_ID -F file=$APPVEYOR_BUILD_FOLDER/build/test_output.xml" || goto :error

goto :EOF

:error
exit /b %errorlevel%
