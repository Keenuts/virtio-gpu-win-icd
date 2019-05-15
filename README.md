# [IN PROGRESS] Windows ICD for Qemu/KVM

Temporary repo for an OpenGL ICD.

This project is **NOT** supposed to compile. It was compiling a long time ago, but I do not know how broken it is now.
This project **ONLY** works in **1** specific setup, on QEMU/KVM, with the proper custom forked version of the VirtioGPU kernel driver.

The compile.bat file is just a wrapper to *devenv*. Usage is something like

./compile.bat  ICD/DEMO  Debug/Release Win32/other-target-archi and maybe a 4th parameter like x86/x64, not sure anymore

Looking at that link might help: https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=vs-2015
