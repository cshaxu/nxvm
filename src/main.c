/* This file is a part of NXVM project. */

#define PRODUCT "Neko's x86 Virtual Machine [0.2.0xa0]\n\
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
 * Module: Console
 *     Files:   main.c, global.h
 *     Files:   console.c, console.h
 *     Files:   dosint.c, dosint.h
 *
 * Module: Debugger
 * Component 1: debugger
 *     Files:   debug.c, debug.h
 *     Phase 1:
 *     Phase 2: 01/01/2012 - 01/24/2012
 *     Phase 3:
 *     Phase 4:
 *     Phase 5:
 *     Phase 6:
 * Component 2: asembler
 *     Files:   asm86.c, asm86.h
 *     Phase 1:
 *     Phase 2: 01/25/2012 - 01/31/2012
 *     Phase 3:
 *     Phase 4:
 *     Phase 5:
 *     Phase 6:
 * Component 3: disassembler 
 *     Files:   asm86.c, asm86.h
 *     Phase 1:
 *     Phase 2: 02/01/2012 - 02/05/2012
 *     Phase 3:
 *     Phase 4:
 *     Phase 5:
 *     Phase 6:
 *
 * Module: VMachine - Global
 * Component 1: types and definitions
 *     Files:   global.h, coption.h
 *     Phase 1:
 *     Phase 2: done
 *     Phase 3:
 *     Phase 4:
 *     Phase 5:
 *     Phase 6:
 * Component 2: chassis
 *     Files:   vmachine.c, vmachine.h
 *     Phase 1:
 *     Phase 2: done
 *     Phase 3:
 *     Phase 4:
 *     Phase 5:
 *     Phase 6:
 *
 * Module: VMachine - Kernel
 * Component 1: cpu (8086)
 *     Files:   vcpu.c, vcpu.h
 *     Files:   vcpuins.c, vcpuins.h
 *     Phase 1:
 *     Phase 2: 02/06/2012 - 02/12/2012
 *     Phase 3:
 *     Phase 4:
 *     Phase 5:
 *     Phase 6:
 *     Debug:   04/14/2013 - 04/20/2013
 *     Note:    tested by msdos 6.22
 * Component 2: ram
 *     Files:   vram.c, vram.h
 *     Phase 1:
 *     Phase 2: 02/06/2012 - 02/12/2012
 *     Phase 3:
 *     Phase 4:
 *     Phase 5:
 *     Phase 6:
 *     Note:    simply tested
 * Component 3: pic (8259)
 *     Files:   vpic.c, vpic.h
 *     Phase 1: 02/06/2012 - 02/12/2012
 *     Phase 2: 02/06/2012 - 02/12/2012
 *     Phase 3: 03/17/2013 - 03/21/2013
 *     Phase 4: 03/20/2013 - 03/21/2013
 *     Phase 5: 03/21/2013 - 03/21/2013
 *     Phase 6:
 *     Note:    fully tested, except for
 *              OCW2, ICW4(AEOI) not tested
 * Component 4: pit (8254)
 *     Files:   vpit.c, vpit.h
 *     Phase 1: 03/22/2013 - 03/23/2013
 *     Phase 2: 03/03/2012 - 04/01/2012
 *     Phase 3: 03/23/2013 - 03/25/2013
 *     Phase 4: 03/25/2013 - 03/25/2013
 *     Phase 5:
 *     Phase 6:
 *     Note:    not fully implemented (read-back command)
 *              simply tested
 * Component 5: dma (8237)
 *     Files:   vdma.c, vdma.h
 *     Phase 1: 03/25/2013 - 03/28/2013
 *     Phase 2: 03/03/2012 - 04/01/2012
 *     Phase 3: 03/25/2013 - 03/28/2013
 *     Phase 4: 03/25/2013 - 03/28/2013
 *     Phase 5:
 *     Phase 6:
 *     Note:    simply tested
 * Componenet 6: fdc (8272)
 *     Files:   vfdc.c, vfdc.h
 *     Phase 1: 04/03/2013 - 04/06/2013
 *     Phase 2: 03/07/2012 - 03/18/2012
 *     Phase 3: 04/03/2013 - 04/05/2013
 *     Phase 4: 04/06/2013 - 04/06/2013
 *     Phase 5:
 *     Phase 6:
 *     Note:    simply tested
 * Componenet 7: fdd
 *     Files:   vfdd.c, vfdd.h
 *     Phase 1: 04/03/2013 - 04/05/2013
 *     Phase 2: 03/08/2012 - 03/18/2012
 *     Phase 3: 04/03/2013 - 04/05/2013
 *     Phase 4: 04/06/2013 - 04/06/2013
 *     Phase 5:
 *     Phase 6:
 *     Note:    simply tested
 *
 * Module: VMachine - HCI
 * Component 1: kbc
 *     Files:   vkbc.c, vkbc.h
 *     Phase 1: 04/07/2013-04/11/2013
 *     Phase 2: 04/07/2013-04/11/2013
 *     Phase 3:
 *     Phase 4:
 *     Phase 5:
 *     Phase 6:
 *     Note:    not fully implemented
 *              not tested
 * Component 2: keyb
 *     Files:   vkeyb.c, vkeyb.h
 *     Phase 1: 04/09/2013-04/11/2013
 *     Phase 2: 04/09/2013-04/11/2013
 *     Phase 3:
 *     Phase 4:
 *     Phase 5:
 *     Phase 6:
 *     Note:    not fully implemented
 *              not tested
 *              apiKeyEvent() MUST send scan code 1 to vkeyb
 * Component 3: vvadp
 *     Files:  vvadp.c, vvadp.h
 * Component 4: display (?)
 *     Files:  vdisp.c, vdisp.h
 *
 * Module: VMachine - BIOS
 * Component 1: cmos
 *     Files:   vcmos.c, vcmos.h
 *     Phase 1:
 *     Phase 2: 03/02/2012 - 04/01/2012
 *     Phase 3:
 *     Phase 4:
 *     Phase 5:
 *     Phase 6:
 *     Note:    init regs; continue when creating BIOS
 * Component 2: bios
 *     Files:   vbios.asm
 *
 * Module: VMachine - Platform Related APIs
 * Component 1: none
 *     Files:   api.c, vapi.h
 * Component 2: win32con
 *     Files:   win32con.c, win32con.h
 * Component 3: win32app
 *     Files:   win32app.c, win32app.h
 *
 * Module: VMachine - Additional
 * Component 1: serial
 *     Files:   vseri.c, vseri.h
 * Component 2: parallel
 *     Files:   vpara.c, vpara.h
 * Component 3: hdc, hdd
 *     Files:   vhdc.c, vhdc.h, vhdd.c, vhdd.h
 * Component 4: printer
 *     Files:   vprinter.c, vprinter.h
 * Component 5: cdrom
 *     Files:   vcdrom.c, vcdrom.h
 *
 * Format Unify:
 * all boolean type should be t_bool, declared TRUE or FALSE
 * all size type should be t_size
 * all id type should be t_id
 * check all includes, macro names, flag names
 * check all null pointer: use NULL instead of 0
 * use assert
 * replace all constants with MACRO
 * all t_bool variable names start with "flag"
 * new flag system: FLAG_X_X 0x0010, bit macros
 * replace all (void *) with t_vaddrcc
 *
 * NEXT: CPUINS Trace List
 *
 */

#include "stdio.h"
#include "stdlib.h"

#include "console/console.h"

int main(int argc, char **argv)
{
	fprintf(stdout,"%s\n",PRODUCT);
	console();
	return 0;
}
