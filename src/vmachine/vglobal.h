/* This file is a part of NXVM project. */

/* Type Definitions */

#ifndef NXVM_VGLOBAL_H
#define NXVM_VGLOBAL_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "stdint.h"

#define VGLOBAL_VAR_WIN32 0
#define VGLOBAL_VAR_LINUX 1
#define VGLOBAL_VAR_CON   0
#define VGLOBAL_VAR_APP   1

/* COMPLIATION DEFINITIONS ***************************************************/
#define VGLOBAL_SIZE_INTEGER 32
#define VGLOBAL_PLATFORM     VGLOBAL_VAR_WIN32
#define VGLOBAL_APPTYPE      VGLOBAL_VAR_APP
/*****************************************************************************/

typedef char     *t_string;
typedef uint8_t   t_nubit1;
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
#if VGLOBAL_SIZE_INTEGER == 64
typedef t_nubit64 t_nubitcc;
typedef t_nsbit64 t_nsbitcc;
#else
typedef t_nubit32 t_nubitcc;
typedef t_nsbit32 t_nsbitcc;
#endif
typedef t_nubit1  t_bool;
typedef t_nubitcc t_vaddrcc;
typedef t_nubitcc t_faddrcc;

#define p_nubit1  (t_nubit1 *)
#define p_nubit4  (t_nubit4 *)
#define p_nubit6  (t_nubit6 *)
#define p_nubit8  (t_nubit8 *)
#define p_nsbit8  (t_nsbit8 *)
#define p_nubit16 (t_nubit16 *)
#define p_nsbit16 (t_nsbit16 *)
#define p_nubit32 (t_nubit32 *)
#define p_nsbit32 (t_nsbit32 *)
#if VGLOBAL_SIZE_INTEGER == 64
#define p_nubit64 (t_nubit64 *)
#define p_nsbit64 (t_nsbit64 *)
#endif
#define p_float32 (t_float32 *)
#define p_float64 (t_float64 *)
#define p_nubitcc (t_nubitcc *)
#define p_nsbitcc (t_nsbitcc *)
#define p_bool    (t_bool *)

#define d_nubit1(n)  (*(t_nubit1 *)(n))
#define d_nubit4(n)  (*(t_nubit4 *)(n))
#define d_nubit6(n)  (*(t_nubit6 *)(n))
#define d_nubit8(n)  (*(t_nubit8 *)(n))
#define d_nsbit8(n)  (*(t_nsbit8 *)(n))
#define d_nubit16(n) (*(t_nubit16 *)(n))
#define d_nsbit16(n) (*(t_nsbit16 *)(n))
#define d_nubit32(n) (*(t_nubit32 *)(n))
#define d_nsbit32(n) (*(t_nsbit32 *)(n))
#if VGLOBAL_SIZE_INTEGER == 64
#define d_nubit64(n) (*(t_nubit64 *)(n))
#define d_nsbit64(n) (*(t_nsbit64 *)(n))
#endif
#define d_float32(n) (*(t_float32 *)(n))
#define d_float64(n) (*(t_float64 *)(n))
#define d_nubitcc(n) (*(t_nubitcc *)(n))
#define d_nsbitcc(n) (*(t_nsbitcc *)(n))
#define d_bool(n)    (*(t_bool *)(n))

#define GetBit(s, f) (!!((s) & (f)))
#define SetBit(d, s) ((d) |= (s))
#define ClrBit(d, s) ((d) &= ~(s))
#define MakeBit(d, s, f)  ((f) ? SetBit((d), (s)) : ClrBit((d), (s)))
#define GetMSB(n, b) (GetBit((n), (1 << ((b) - 1))))
#define GetLSB(n, b) (GetBit((n), 1))

#define ExecFun(faddr) (*(void (*)(void))(faddr))()

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif