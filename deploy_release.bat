@echo off
copy %1 %USERPROFILE%\Desktop\VM\
copy %1 %~dp0\test_suite\
copy %1 %~dp0\opengl_tests\opengl_context\x64\Release\