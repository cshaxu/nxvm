/* This file is a part of NVMx86 project. */

#ifndef NVM_VGLOBAL_H
#define NVM_VGLOBAL_H

#include "coption.h"

typedef unsigned char t_nubit8;		//bit8: number, 8-bit
typedef signed char t_nsbit8;
typedef unsigned short t_nubit16;
typedef signed short t_nsbit16;
typedef unsigned long t_nubit32;
typedef signed long t_nsbit32;

#if NVM_INTLEN == 32
typedef unsigned int t_bool;
typedef unsigned int t_nubitcc;
typedef signed int t_nsbitcc;
typedef unsigned int t_vaddrcc;		// addr: variable, c compiler
typedef unsigned int t_faddrcc;		// addr: function, c compiler
#elif NVM_INTLEN == 64
typedef unsigned long long t_bool;
typedef unsigned long long t_nubitcc;
typedef signed long long t_nsbitcc;
typedef unsigned long long t_vaddrcc;	// addr: variable, c compiler
typedef unsigned long long t_faddrcc;	// addr: function, c compiler
#else
#endif

#define FUNEXEC(faddr) (*(void (*)(void))(faddr))()
#define SHL4(n) (((t_vaddrcc)n)<<4)

#endif