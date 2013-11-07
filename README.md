Neko's x86 Virtual Machine
==========================
A light-weight pc emulator

Author: Xu Ha (cshaxu@gatech.edu)

Introduction
------------
 This project includes an integrated implementation of a light-weight x86 PC emulator.

 It emulated a PC machine, including an Intel 80386 CPU and all other necessary devices, such as RAM, DMA, PIC, Floppy, Hard Drive, Keyboard, Display and so on.

 Therefore it supports the operating systems based on x86, such as MS-DOS.

Components
----------

### Emulator Itself
0. Main Entry and Version Label (src/main.c)
1. Virtual Machine Console (src/console.ch)
2. Platform-related Components (src/vmachine/system/*.ch)
3. Basic I/O System (src/vmacheine/bios/*.ch)
4. Hardware Emulation Modules (src/vmachine/v*.ch)

 The emulator is basically divided into 4 parts: VM console, Hardware Emulation, BIOS and Platform-related Part.

 VM console provides a command-line interface which accepts user input and provides machine info.
 Hardware Emulation is the "hardware" part, which provides the hardware logic of all the devices emulated.
 BIOS is the "software" part, which provides POST and interrupt services routines.
 Platform-related Part basically provides keyboard and display, and it is designed to work with different operating systems.

 The emulator is NOT just an emulator. It does more than simply emulating a PC. Users may debug an operating system in NXVM.

### Debugger
0. Debugger (src/vmachine/debug/debug.ch)

 The debugger is used to test and debug the guest operating system running inside NXVM.

 User may print/modify CPU registers, print/modify/search/compare/fill/watch RAM area, operate device through I/O ports, trace/dump CPU instructions and set breakpoints.

 NXVM has both 16-bit debugger and 32-bit debugger. The command usage of 16-bit debugger is almost the same as the MS-DOS debugger, and all the memory addresses are represented in physical address. The 32-bit debugger uses linear address format. The help command '?' will introduce all available commands supported.

### Recorder
0. Recorder (src/vmachine/debug/record.ch)

 The recorder provides various modes to dump CPU register values and read/write operations at each CPU instruction.

### Assembler/Disassembler
0. i386 Assembler (src/vmachine/debug/aasm32.ch)
1. i386 Disassembler (src/vmachine/debug/dasm32.ch)
 
 The emulator includes an integral assembler and disassembler. They translates Assembly codes from/to machine codes. All Intel-format x86 instructions are supported.
 The assembler and disassembler are used as a part of debugger, which provides readable instructions to user.

### Environment

 Windows: The emulator provides two modes of display: Win32 Console and Win32 App Window.
 In the previous mode, the keyboard and display are emulated using Win32 Console APIs, while in the later mode, emulator accepts input and provides output using a Window controlled by message loop.

 The Win32 App Window mode is preferred for two reasons:
0. The bitmap font used in MS-DOS is applied in Win32 App Window Mode.
1. The NXVM console window still exists and won't be overrided by NXVM display.

 Linux: The emulator runs in a terminal window of at least 80x25 size. It works like Win32 Console mode. It has some known issues which are not yet fixed.
 
### Compiling

 The compiling options are defined in src/vmachine/vglobal.h, to specify 32/64 bit compilation. It also has a macro to specify the platform WIN32/LINUX.
 In most cases, user doesn't need to change anything there.

 Windows: IDE is MS Visual Studio 2008 or higher.
 
 Create an empty Win32 Console project and add all .c and .h files in the folders src/ and src/vmachine. Then remove the following files:
0. src/vmachine/vcpuapi.h
1. src/vmachine/vcpuapi.cc
2. src/vmachine/vcpuins_i8086.c
3. src/vmachine/linux.c
4. src/vmachine/linux.h

 Linux: makefile is provided in src/, and could be compiled by 'make' command.
 Before compiling, you need to install ncurses and pthread libraries.

Snapshots
---------
![index.html](Snapshots/index.png)
![search.html](Snapshots/search.png)
![video.html](Snapshots/video.png)
![image.html](Snapshots/image.png)
