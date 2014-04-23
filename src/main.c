/* This file is a part of NXVM project. */

#define PRODUCT "Neko's x86 Virtual Machine [0.3.0150]\n\
Copyright (c) 2012-2013 Neko. All rights reserved.\n"

/*
 * Developer: Xu Ha
 * Email:     cshaxu@gatech.edu
 * Start:     01/25/2012
 * End:       (null)
 *
 * Project Phases
 * 1. Read documents and list all necessary functionality
 * 2. Code the functions
 * 3. Add comment for each I/O function, check logic
 * 4. Create test functions and corresponding test case
 * 5. Perform unit tests
 * 6. Read official document and PC.PDF, then check logic
 *
 * Modules:
 * 1-Console
 *   main.c
 *   console.c
 *   console.h
 * 2-Debugger:
 *  a-Debugging Console
 *   vmachine/debug/debug.c
 *   vmachine/debug/debug.h
 *  b-Assembler for Intel 8086
 *   vmachine/debug/aasm.c
 *   vmachine/debug/aasm.h
 *  c-Disassembler for Intel 80386
 *   vmachine/debug/dasm32.c
 *   vmachine/debug/dasm32.h
 *  d-Recorder
 *   vmachine/debug/record.c
 *   vmachine/debug/record.h
 * 3-Virtual Machine Kernel
 *  a-Definitions and Interface
 *   vmachine/stdint.h
 *   vmachine/vglobal.h
 *   vmachine/vmachine.c
 *   vmachine/vmachine.h
 *  b-Input/Output Ports
 *   vmachine/vport.c
 *   vmachine/vport.h
 *  c-Random Accessing Memory
 *   vmachine/vram.c
 *   vmachine/vram.h
 *  d-Center Processing Unit (Intel 8086/80386)
 *   vmachine/vcpuins-i8086.c
 *   vmachine/vcpuins.c
 *   vmachine/vcpuins.h
 *   vmachine/vcpu.c
 *   vmachine/vcpu.h
 *  e-Programmable Interrupt Controller (8259)
 *   vmachine/vpic.c
 *   vmachine/vpic.h
 *   NOTE: fully tested, except for OCW2, ICW4(AEOI)
 *  f-Programmable Internal Timer (8254)
 *   vmachine/vpit.c
 *   vmachine/vpit.h
 *   NOTE: not fully implemented (read-back command); not fully tested
 *  g-Direct Memory Accessing Controller (8237)
 *   vmachine/vdma.c
 *   vmachine/vdma.h
 *  h-Hard Disk Driver
 *   vmachine/vhdc.c
 *   vmachine/vhdc.h
 *   vmachine/vhdd.c
 *   vmachine/vhdd.h
 *   NOTE: not implemented; only qd
 *  i-Floppy Drive Controller (8272)
 *   vmachine/vfdc.c
 *   vmachine/vfdc.h
 *   vmachine/vfdd.c
 *   vmachine/vfdd.h
 *  j-Keyboard Controller
 *   vmachine/vkbc.c
 *   vmachine/vkbc.h
 *   NOTE: not implemented
 *  k-Video Adapter
 *   vmachine/vvadp.c
 *   vmachine/vvadp.h
 *   NOTE: not implemented
 *  l-Human-Computer Interface for Linux
 *   vmachine/system/linux.c
 *   vmachine/system/linux.h
 *  m-Human-Computer Interface for Windows
 *   vmachine/system/win32.c
 *   vmachine/system/win32.h
 *   vmachine/system/win32app.c
 *   vmachine/system/win32app.h
 *   vmachine/system/win32con.c
 *   vmachine/system/win32con.h
 *   vmachine/system/w32adisp.c
 *   vmachine/system/w32adisp.h
 *   vmachine/system/w32cdisp.c
 *   vmachine/system/w32cdisp.h
 *  n-CMOS and BIOS
 *   vmachine/vcmos.c
 *   vmachine/vcmos.h
 *   vmachine/bios/*.*
 *  z-additional
 *    serial:   vseri.c,    vseri.h
 *    parallel: vpara.c,    vpara.h
 *    printer:  vprinter.c, vprinter.h
 *    cdrom:    vcdrom.c,   vcdrom.h
 *
 * NXVM in the year 2012
 * 01/01-02/05  built debugger
 * 02/06-02/12  built i8086 cpu
 * 02/06-02/12  built pic
 * 03/02-04/01  built cmos
 * 03/03-04/01  built pit, dma
 * 03/07-03/18  built fdc, fdd
 *
 * NXVM in the year 2013
 * 03/03-04/01  built pit, cont
 * 03/17-03/21  built pic, cont
 * 03/25-03/28  built dma, cont
 * 04/03-04/06  built fdd, cont
 * 04/07-04/11  built kbc, qdkeyb
 * 04/14-05/05  debugged i8086 cpu
 * 04/21-05/23  updated debugger
 * 05/24-07/31  built 80386 cpu
 * 08/06-08/09  built dasm32
 * 08/10-08/15  built aasm32
 *
 * Milestones
 * 0x45 Console, 8086 CPU, Debug, Aasm, Dasm
 * 0x82 Basic Devices (QD Devices)
 * 0x90 Win32 Application
 * 0xbc Win32 Console
 * 0xbf Linux Console
 * 0xca New BIOS System
 * 0xcc New Console Commands
 * 0xdf Rebuilt Aasm/Dasm/Debug System
 * 0xf2 Clean CPUINS; Hard Disk Installed
 * 0xf5 i386 Build Begin
 * 010d Bitmap Font Implemented
 * 0118 i386 Milestone 1 (32-bit ins)
 * 0128 i386 Milestone 2 (MMU)
 * 012e Bochs Tester Constructed
 * 013a Built Dasm 32
 * 0140 Built Trace System
 * 0143 Built Debug 32
 * 0144 Built Aasm 32
 * 014c Rebuilt vmachine.ch
 *
 * Format Unify:
 * new flag system: FLAG_X_X 0x0010, bit macros
 *
 */

#include <stdio.h>

#include "console.h"

int main(int argc, char **argv)
{
	fprintf(stdout, "%s\n", PRODUCT);
	fprintf(stdout, "Built on %s at %s.\n", __DATE__, __TIME__);
	console();
	return 0;
}
