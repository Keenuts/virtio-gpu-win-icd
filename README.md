# Windows ICD for Qemu/KVM

Proof of concept repo for an OpenGL ICD.

This project is **NOT** supposed to compile. It was compiling a long time ago,
but I do not know how broken it is now. This project **ONLY** works in **1**
specific setup, on QEMU/KVM, with the proper custom forked version of the
VirtioGPU kernel driver.

The compile.bat file is just a wrapper to *devenv*. Usage is something like
```
./compile.bat  ICD/DEMO  Debug/Release Win32/other-target-archi
```
and maybe a 4th parameter like x86/x64, not sure anymore

Looking at that link might help:
https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=vs-2015

## Where to start if you want to work on this project?

Even if everything compiles, and you use the provided patched kernel driver,
things might not work anymore. The graphic driver on windows was written
without any support/documentation from Microsoft. When writing this code, I
had to fumble around to understand how windows behaved. And because of time
constraints, I decided to bypass everything using the escape function:
https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dkmddi/nc-d3dkmddi-dxgkddi_escape

- You need a linux host with QEMU and a windows guest.
- You need to use a virtio-gpu device (not QXL).
- You need to build/install a special windows guest kernel driver:
  https://github.com/Keenuts/virtio-gpu-win
- You need to build this ICD and the example OpenGL app.
- Using a normal userland debugger on Windows, make sure this custom
  Opengl32.dll is used when you start the demo app.
- Once you know the custom ICD is used, use windbg to check that the custom
  kernel driver is being used (The fact that Windows is unstable is a good
  indicator that this custom driver is in use).
- Use a debugger/print statements on the host (require building you own
  qemu/virglrenderer with debug flags or print statements), and check the host
  receives the virtio-gpu commands.

Once you observe the virtio-gpu implementation on the host receiving commands
from the OpenGL sample app, you are good to go! Next step is to implement a
proper ICD (reuse MESA), and integrate with D3DKMD properly.

Additional informations can be found here:
https://www.studiopixl.com/2017-08-27/3d-acceleration-using-virtio.html
