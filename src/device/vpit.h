/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VPIT_H
#define NXVM_VPIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

#define NXVM_DEVICE_PIT "Intel 8254"

typedef enum {
    VPIT_STATUS_RW_READY,
    VPIT_STATUS_RW_LSB,
    VPIT_STATUS_RW_MSB
} t_pit_data_status_rw;

typedef struct {
    /* control words[0-2] for counter 0-2, and cw[3] is read-back command */
    t_nubit8 cw[4];

    t_nubit16 init[3];  /* initial counts */
    t_nubit16 count[3]; /* counter[0-2] */
    t_nubit16 latch[3]; /* latch counts */

    t_bool flagReady[3]; /* flag of ready */
    t_bool flagLatch[3]; /* flag of latch status */

    t_pit_data_status_rw flagRead[3];  /* flag of low byte read */
    t_pit_data_status_rw flagWrite[3]; /* flag of low byte write */
} t_pit_data;

typedef struct {
    t_bool flagGate[3];  /* enable or disable counter */
    t_faddrcc fpOut[3]; /* action when out signal is valid */
} t_pit_connect;

typedef struct {
    t_pit_data data;
    t_pit_connect connect;
} t_pit;

extern t_pit vpit;

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

void vpitSetGate(t_nubit8 id, t_bool flagGate);

#define vpitAddMe(id) vpitAddDevice((id), (t_faddrcc) pitOut);
void vpitAddDevice(int id, t_faddrcc fpOut);

void vpitInit();
void vpitReset();
void vpitRefresh();
void vpitFinal();

#define VPIT_POST "                                 \
; init vpit                                       \n\
mov al, 36 ; 0011 0110 mode = 3, counter = 0, 16b \n\
out 43, al                                        \n\
mov al, 00                                        \n\
out 40, al ; initial count (0x10000)              \n\
out 40, al                                        \n\
mov al, 54 ; 0101 0100 mode = 2, counter = 1, LSB \n\
out 43, al                                        \n\
mov al, 12                                        \n\
out 41, al ; initial count (0x12)                 \n"

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
