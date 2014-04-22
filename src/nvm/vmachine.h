/* This file is a part of NekoVMac project. */

#define NVM_DEBUG

#ifndef NVM_VMACHINE_H
#define NVM_VMACHINE_H

typedef unsigned char t_nubit8;		//bit8: number, 8-bit
typedef signed char t_nsbit8;
typedef unsigned short t_nubit16;
typedef signed short t_nsbit16;
typedef unsigned long t_nubit32;
typedef signed long t_nsbit32;
typedef unsigned long t_vaddr32;	// addr: variable, 32-bit
typedef unsigned int t_vaddrcc;		// addr: variable, c compiler
typedef unsigned int t_bool;
typedef void (*t_faddrcc)(void);	// addr: function, c compiler

extern t_nubit32 memorySize;
extern t_vaddrcc memoryBase;
extern t_faddrcc insInTable[0x10000];
extern t_faddrcc insOutTable[0x10000];
extern t_faddrcc ins86Table[0x100];
extern t_bool testFlag;

void NVMInit();
void NVMRun();
void NVMTerm();

#endif