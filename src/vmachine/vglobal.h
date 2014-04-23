/* This file is a part of NXVM project. */

/* Type Definitions */

#ifndef NXVM_VGLOBAL_H
#define NXVM_VGLOBAL_H

/*#include "stdio.h"*/
#include "stdint.h"

#define VGLOBAL_SIZE_INTEGER 32

/*#define VGLOBAL_SIZE_STRING 0x100*/

typedef uint8_t   t_nubit4;
typedef uint8_t   t_nubit6;
typedef uint8_t   t_nubit8;
typedef int8_t    t_nsbit8;
typedef uint16_t  t_nubit16;
typedef int16_t   t_nsbit16;
typedef uint32_t  t_nubit32;
typedef int32_t   t_nsbit32;
#if VGLOBAL_SIZE_INTEGER == 64
typedef uint64_t  t_nubit64;
typedef int64_t   t_nsbit64;
#endif

typedef float     t_float32;
typedef double    t_float64;
/*typedef FILE    t_file;
typedef size_t    t_size;
typedef char      t_string[VGLOBAL_SIZE_STRING];*/

#if VGLOBAL_SIZE_INTEGER == 64
typedef t_nubit64 t_nubitcc;
typedef t_nsbit64 t_nsbitcc;
#else
typedef t_nubit32 t_nubitcc;
typedef t_nsbit32 t_nsbitcc;
#endif

typedef t_nubitcc t_bool;
typedef t_nubitcc t_vaddrcc;
typedef t_nubitcc t_faddrcc;

#define GetBit(a,b) (!!((a) &   (b)))
#define SetBit(a,b) (   (a) |=  (b))
#define ClrBit(a,b) (   (a) &= ~(b))
#define FUNEXEC(faddr) (*(void (*)(void))(faddr))()
#define SHL4(n) (((t_vaddrcc)(n))<<4)

#endif