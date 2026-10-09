/* Copyright 2012-2014 Neko. */

#ifndef X86_PIT_H
#define X86_PIT_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lib/types/types_interface.h"

#include "core/chips/pit825x/pit825x_interface.h"

typedef enum {
    VPIT_STATUS_RW_READY,
    VPIT_STATUS_RW_LSB,
    VPIT_STATUS_RW_MSB
} t_pit_data_status_rw;

typedef struct {
    /* control words[0-2] for counter 0-2, and cw[3] is read-back command */
    lib_u8 cw[4];

    lib_u16 init[3];  /* initial counts */
    lib_u16 count[3]; /* counter[0-2] */
    lib_u16 latch[3]; /* latch counts */
    lib_u8 status_latch[3]; /* read-back status bytes */

    lib_bool flagReady[3]; /* flag of ready */
    lib_bool flagLatch[3]; /* flag of latch status */
    lib_bool flagStatusLatch[3]; /* flag of pending status read-back */
    lib_bool flagOutput[3]; /* retained counter-model OUT state */
    lib_bool flagActive[3]; /* a loaded waveform is currently counting */
    lib_bool flagPulseLow[3]; /* one elapsed-tick low strobe is pending */
    lib_bool flagLoadPending[3]; /* completed CR write awaits CE load */
    lib_bool flagTrigger[3]; /* rising GATE trigger for modes 1/5 */
    lib_bool flagRestart[3]; /* rising GATE reload for modes 2/3 */

    lib_u32 reload[3]; /* effective binary/BCD reload; zero is never stored */
    lib_u32 remaining[3]; /* effective count exposed through count[] */
    lib_u32 phase[3]; /* remaining high/low phase for mode 3 */

    t_pit_data_status_rw flagRead[3];  /* flag of low byte read */
    t_pit_data_status_rw flagWrite[3]; /* flag of low byte write */
} t_pit_data;

typedef struct {
    lib_bool flagGate[3];  /* current GATE input level */
    x86_pit_output_provider output[3];
    void *output_owner[3];
} t_pit_connect;

struct x86_pit {
    x86_pit_personality personality;
    t_pit_data data;
    t_pit_connect connect;
};

/*
 * Ctrl Word: SC1 | SC0 | RW1   | RW0    | M2   | M1   | M0   | BCD
 * Latch Cmd: SC1 | SC0 | 0     | 0      | x    | x    | x    | x
 * Read-back: I   | I   | COUNT | STATUS | CNT2 | CNT1 | CNT0 | 0
 * Stus Byte: OUT | NC  | RW1   | RW0    | M2   | M1   | M0   | BCD
 */

/* control word bits */
#define VPIT_CW_BCD 0x01 /* bcd(1) or binary(0) counter */
#define VPIT_CW_M   0x0e /* counter mode bits */
#define VPIT_CW_RW  0x30 /* read/write/latch format bits */
#define VPIT_CW_SC  0xc0 /* counter select bits or read-back command */
#define VPIT_GetCW_SC(cw)  (((cw) & VPIT_CW_SC) >> 6)
#define VPIT_GetCW_RW(cw)  (((cw) & VPIT_CW_RW) >> 4)
#define VPIT_GetCW_M(cw)   (((cw) & VPIT_CW_M)  >> 1)

/* latch command bits */
#define VPIT_LC_SC 0xc0 /* counter select bits */

/* read-back bits */
#define VPIT_RB_CNT(id) (1 << ((id) + 1))
#define VPIT_RB_CNTS    0x0e /* select counters indivisually */
#define VPIT_RB_STATUS  0x10 /* latch status of selected counters*/
#define VPIT_RB_COUNT   0x20 /* latch count of selected counters */

/* status byte bits */
#define VPIT_SB_BCD 0x01 /* bcd(1) or binary(0) counter */
#define VPIT_SB_M   0x0e /* counter mode bits */
#define VPIT_SB_RW  0x30 /* read/write/latch format bits */
#define VPIT_SB_NC  0x40 /* null count (1) or count available (0) */
#define VPIT_SB_OUT 0x80 /* state of out pin high(1) or low(0) */

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
