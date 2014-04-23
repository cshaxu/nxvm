/* This file is a part of NekoVMac project. */

#ifndef NVM_VGLOBAL_H
#define NVM_VGLOBAL_H

//#define ia32
#define amd64

typedef unsigned char t_nubit8;		//bit8: number, 8-bit
typedef signed char t_nsbit8;
typedef unsigned short t_nubit16;
typedef signed short t_nsbit16;
typedef unsigned long t_nubit32;
typedef signed long t_nsbit32;

#ifdef ia32
typedef unsigned int t_nubitcc;
typedef signed int t_nsbitcc;
typedef unsigned int t_vaddrcc;		// addr: variable, c compiler
#endif

#ifdef amd64
typedef unsigned __int64 t_nubitcc;
typedef signed __int64 t_nsbitcc;
typedef unsigned __int64 t_vaddrcc;		// addr: variable, c compiler
#endif

typedef unsigned char t_bool;
typedef void (*t_faddrcc)(void);	// addr: function, c compiler

#define SHL4(n) (((t_vaddrcc)n)<<4)

#endif