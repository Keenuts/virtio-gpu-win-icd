@echo off
copy %1 %USERPROFILE%\Desktop\VM\
copy %1 %~dp0\test_suite\

setlocal enableextensions
mkdir %~dp0\opengl_tests\opengl_context\x64\Debug\
endlocal

copy %1 %~dp0\opengl_tests\opengl_context\x64\Debug\