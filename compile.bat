@echo off
IF /i "%1"=="icd" goto ICD
IF /i "%1"=="demo" goto DEMO

	echo "UNKNOWN MODE"
	goto EXIT
	
:ICD
echo "%1 %2 %3 %4"
"C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv" virtio-gpu-win-icd\virtio-gpu-win-icd.vcxproj %2 "%3|%4"
goto EXIT

:DEMO
"C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE\devenv" opengl_tests\opengl_context\opengl_context.vcxproj %2 "%3|%4"
goto EXIT
	
:EXIT

