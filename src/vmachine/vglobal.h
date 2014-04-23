/* This file is a part of NXVM project. */

// Type Definitions

#ifndef NXVM_VGLOBAL_H
#define NXVM_VGLOBAL_H

#include "stdint.h"
#include "coption.h"

#define VGLOBAL_SIZE_STRING 0x100

typedef uint8_t		t_nubit4;
typedef uint8_t		t_nubit6;
typedef uint8_t		t_nubit8;		//bit8: number, unsigned, 8-bit
typedef int8_t		t_nsbit8;
typedef uint16_t	t_nubit16;
typedef int16_t		t_nsbit16;
typedef uint32_t	t_nubit32;
typedef int32_t		t_nsbit32;
typedef uint64_t	t_nubit64;
typedef int64_t		t_nsbit64;

typedef float		t_float;
typedef double		t_double;
typedef char		t_string[VGLOBAL_SIZE_STRING];

#if NXVM_INTLEN == 32
typedef t_nubit32	t_bool;
typedef t_nubit32	t_nubitcc;
typedef t_nsbit32	t_nsbitcc;
typedef t_nubit32	t_vaddrcc;		// addr: variable, c compiler
typedef t_nubit32	t_faddrcc;		// addr: function, c compiler
#elif NXVM_INTLEN == 64
typedef t_nubit64	t_bool;
typedef t_nubit64	t_nubitcc;
typedef t_nsbit64	t_nsbitcc;
typedef t_nubit64	t_vaddrcc;	// addr: variable, c compiler
typedef t_nubit64	t_faddrcc;	// addr: function, c compilers
#endif

#define FUNEXEC(faddr) (*(void (*)(void))(faddr))()
#define SHL4(n) (((t_vaddrcc)n)<<4)

#endif