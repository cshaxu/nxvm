Neko's Virtual DOS Machine
==========================
A virtual DOS machine that runs legacy 16-bit DOS applications on various platforms  
Author: Xu Ha (cshaxu@gmail.com)  


Introduction
------------
As far as we know, Microsoft Windows no longer supports 16-bit MS-DOS application and they removed NTVDM, since virtual 8086 mode is not available under amd64 extended mode. This project aimed at building a virtual dos machine for 64-bit Operating Systems.  


The idea of this project is to emulate a 16-bit CPU and provide BIOS/DOS interrupt services to MS-DOS applications. In NKVDM, 16-bit applications runs as before in 32-bit Windows. We want to create a compatible environment for 16-bit applications, for example, applications can access disk files on host machine using legacy DOS interrupts instead of using network sharing.  


Now we already have a PC emulator which could run in Win32 console and it supports all 8086 instructions - MS-DOS 6.22 runs well inside this PC emulator. The only work left for this project is to construct the built-in dos kernel for the emulator, so that it can load dos executables and execute them under any available platforms. Please visit the homepage of NXVM project at https://github.com/cshaxu/nxvm.  


Currently I am learning the MS-DOS internals and here's the roadmap.  


### Replace MS-DOS disk related interrupt services  
Collect all INT services dealing with Disk  
MS-DOS Read/Write To Host Machine Disk/Files  
Keep io.sys, msdos.sys and command.com in disk image  


### Hack command interpreter (Exec, PSP)  
Collect all Exec related interrupt services  
Learn about DOS Kernel  
Load and execute command.com from host machine  
Keep io.sys and msdos.sys in disk image  


### Hack Dos Kernel  
Implement Dos Kernel in C and load it into NKVDM  
Keep io.sys in disk image  


### Hack Dos IO  
Move io.sys out of disk image  
Implement Dos IO in C for NKVDM  
Remove disk image and NKVDM runs without MS-DOS support  


Compiling
---------
Copy source files from NXVM project to NKVDM source folder:  
nxvm/src/ -> nkvdm/src/nxvmsrc/

For Windows,  
Add the files to the NKVDM project settings and compile.  


For Linux/UNIX,  
Simply use automake scripts:  
./configure  
make  


Quick Start
-----------


Component
---------


Demo
----


References
----------
