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
 NXVM includes the following modules:

 Main Entry and Version Label (src/main.c)
 Virtual Machine Console (src/console.ch)
 Debugger (src/vmachine/debug/debug.ch)
 Recorder (src/vmachine/debug/record.ch)
 i386 Assembler (src/vmachine/debug/aasm32.ch)
 i386 Disassembler (src/vmachine/debug/dasm32.ch)
 Platform-related Components (src/vmachine/system/*.ch)
 Basic I/O System (src/vmacheine/bios/*.ch)
 Hardware Emulation Modules (src/vmachine/v*.ch)

### Emulator Itself
 The emulator is basically divided into 4 parts: VM console, Hardware Emulation, BIOS and Platform-related Part.

 VM console provides a command-line interface which accepts user input and provides machine info.
 Hardware Emulation is the "hardware" part, which provides the hardware logic of all the devices emulated.
 BIOS is the "software" part, which provides POST and interrupt services routines.
 Platform-related Part basically provides keyboard and display, and it is designed to work with different operating systems.

 The emulator is NOT just an emulator. It does more than simply emulating a PC. Users may debug an operating system in NXVM.

### Debugger
 The debugger is used to test and debug the guest operating system running inside NXVM.

 User may print/modify CPU registers, print/modify/search/compare/fill/watch RAM area, operate device through I/O ports, trace/dump CPU instructions and set breakpoints.

 NXVM has both 16-bit debugger and 32-bit debugger. The command usage of 16-bit debugger is almost the same as the MS-DOS debugger, and all the memory addresses are represented in physical address. The 32-bit debugger uses linear address format. The help command '?' will introduce all available commands supported.

### Recorder
 The recorder provides various modes to dump CPU register values and read/write operations at each CPU instruction.

### Assembler/Disassembler
 The emulator includes an integral assembler and disassembler. They translates Assembly codes from/to machine codes. All Intel-format x86 instructions are supported.
 The assembler and disassembler are used as a part of debugger, which provides readable instructions to user.
 
Snapshots
---------
![index.html](Snapshots/index.png)
![search.html](Snapshots/search.png)
![video.html](Snapshots/video.png)
![image.html](Snapshots/image.png)
