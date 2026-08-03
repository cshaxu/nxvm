/* Copyright 2012-2014 Neko. */

#ifndef NXVM_CORE_DMA_H
#define NXVM_CORE_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/machine/port.h"

#define NXVM_DEVICE_DMA "Intel 8237A"

typedef ntvdm64_type_unsigned_8 t_page;
typedef struct t_latch t_latch;
typedef struct t_ram t_ram;
typedef void (*core_machine_dma_device_provider)(void *owner, t_latch *latch);

#define VDMA_CHANNEL_COUNT 4

typedef struct {

    ntvdm64_type_unsigned_16 baseAddr[VDMA_CHANNEL_COUNT];  /* base address */
    ntvdm64_type_unsigned_16 baseCount[VDMA_CHANNEL_COUNT]; /* base word count */
    ntvdm64_type_unsigned_16 currAddr[VDMA_CHANNEL_COUNT];  /* current address */
    ntvdm64_type_unsigned_16 currCount[VDMA_CHANNEL_COUNT]; /* current word count */
    ntvdm64_type_unsigned_8  mode[VDMA_CHANNEL_COUNT];      /* mode register */
    t_page    page[VDMA_CHANNEL_COUNT];      /* page register */

    ntvdm64_type_unsigned_8 command; /* command register */
    ntvdm64_type_unsigned_8 status;  /* status register */
    ntvdm64_type_unsigned_8 mask;    /* mask register */
    ntvdm64_type_unsigned_8 request; /* request register */
    ntvdm64_type_unsigned_8 temp;    /* temporary register */
    ntvdm64_type_unsigned_8 drx;     /* dreq id of highest priority */
    ntvdm64_type_bool   flagMSB; /* flip-flop for msb/lsb */
    ntvdm64_type_bool   flagEOP; /* end of process */

    /* id of request in service in D5-D4, flag of in service in D0 */
    ntvdm64_type_unsigned_8 isr;
} t_dma_data;

typedef struct {
    /* Non-owning links to the same composition-owned DMA pair. */
    t_latch *latch;
    struct t_dma *peer;
    /* get data from device to latch */
    ntvdm64_type_flat_address fpReadDevice[VDMA_CHANNEL_COUNT];
    /* write data to device from latch */
    ntvdm64_type_flat_address fpWriteDevice[VDMA_CHANNEL_COUNT];
    /* send eop signal to device */
    ntvdm64_type_flat_address fpCloseDevice[VDMA_CHANNEL_COUNT];
    core_machine_dma_device_provider read_provider[VDMA_CHANNEL_COUNT];
    core_machine_dma_device_provider write_provider[VDMA_CHANNEL_COUNT];
    core_machine_dma_device_provider close_provider[VDMA_CHANNEL_COUNT];
    void *device_owner[VDMA_CHANNEL_COUNT];
} t_dma_connect;

typedef struct t_dma {
    t_dma_data data;
    t_dma_connect connect;
} t_dma;

typedef union {
    ntvdm64_type_unsigned_8  byte;
    ntvdm64_type_unsigned_16 word;
} t_latch_data;

struct t_latch {
    t_latch_data data;
};

/*
 * CMD:    DACK | DREQ | WS   | R    | TM   | CTRL | C0AD | M2M
 * MODE:   M1   | M0   | AIDS | AI   | TT1  | TT0  | CS1  | CS0
 * REQ:    x    | x    | x    | x    | DRQ3 | DRQ2 | DRQ1 | DRQ0
 * MASK:   x    | x    | x    | x    | DRQ3 | DRQ2 | DRQ1 | DRQ0
 * REQSC:  x    | x    | x    | x    | x    | SR   | CS1  | CS0
 * MASKSC: x    | x    | x    | x    | x    | SM   | CS1  | CS0
 * MASKAC: x    | x    | x    | x    | DRQ3 | DRQ2 | DRQ1 | DRQ0
 * STATUS: DRQ3 | DRQ2 | DRQ1 | DRQ0 | TC3  | TC2  | TC1  | TC0
 * ~ISR:   x    | x    | ISR1 | ISR0 | x    | x    | x    | IS
 */

/* command register bits */
#define VDMA_COMMAND_M2M    0x01 /* memory to memory */
#define VDMA_COMMAND_C0AD   0x02 /* channel 0 address hold */
#define VDMA_COMMAND_CTRL   0x04 /* controller disable(1) or enable(0) */
#define VDMA_COMMAND_TM     0x08 /* compressed(1) or normal(0) timing */
#define VDMA_COMMAND_R      0x10 /* rotating(1) or fixed(0) priority */
#define VDMA_COMMAND_WS     0x20 /* extended(1) or late(0) write selection */
#define VDMA_COMMAND_DREQSA 0x40 /* dreq sense active low(1) or high(0) */
#define VDMA_COMMAND_DACKSA 0x80 /* dack sense active high(1) or low(0) */

/* mode register bits */
#define VDMA_MODE_CS   0x03 /* channel select */
#define VDMA_MODE_TT   0x0c /* transfer type */
#define VDMA_MODE_AI   0x10 /* anto-initialization */
#define VDMA_MODE_AIDS 0x20 /* address decrement(1) or increment(0) select */
#define VDMA_MODE_M    0xc0 /* mode select */

/* request register bits */
#define VDMA_REQUEST_DRQ(id) (1 << (id))

/* mask register bits */
#define VDMA_MASK_VALID 0x0f
#define VDMA_MASK_DRQ(id) (1 << (id))

/* request single command bits */
#define VDMA_REQSC_CS    0x03 /* channel select */
#define VDMA_REQSC_SR    0x04 /* set(1) or reset(0) request bit */
#define VDMA_REQSC_VALID 0x07

/* mask single command bits */
#define VDMA_MASKSC_CS 0x03 /* channel select */
#define VDMA_MASKSC_SM 0x04 /* set(1) or clear(0) mask bit */

/* mask all command bits  */
#define VDMA_MASKAC_VALID 0x0f

/* status register bits */
#define VDMA_STATUS_TC(id)  (1 << (id))
#define VDMA_STATUS_DRQ(id) (1 << ((id) + 4))
#define VDMA_STATUS_TCS  0x0f
#define VDMA_STATUS_DRQS 0xf0

/* in service register bits */
#define VDMA_ISR_IS  0x01 /* has request in service */
#define VDMA_ISR_ISR 0x30 /* id of request (channel) in service */

/* select mode register channel */
#define VDMA_GetMODE_CS(cmode) ((cmode) & VDMA_MODE_CS)
/* verify(0) or write(1) or read(2) or illegal(3) */
#define VDMA_GetMODE_TT(cmode) (((cmode) & VDMA_MODE_TT) >> 2)
/* demand(0) or single(1) or block(2) or cascade(3) mode select */
#define VDMA_GetMODE_M(cmode)  (((cmode) & VDMA_MODE_M) >> 6)

/* tells if drq id is in request register */
#define VDMA_GetREQUEST_DRQ(creq, id) (NTVDM64_TYPE_GET_BIT((creq), VDMA_REQUEST_DRQ(id)))

/* select request register channel */
#define VDMA_GetREQSC_CS(creqsc) ((creqsc) & VDMA_REQSC_CS)

/* select mask register channel */
#define VDMA_GetMASKSC_CS(cmasksc) ((cmasksc) & VDMA_MASKSC_CS)

/* get terminal counter */
#define VDMA_GetSTATUS_TC(cstatus, id)  (NTVDM64_TYPE_GET_BIT((cstatus), VDMA_STATUS_TC(id)))
/* get drq in status register */
#define VDMA_GetSTATUS_DRQ(cstatus, id) (NTVDM64_TYPE_GET_BIT((cstatus), VDMA_STATUS_DRQ(id)))
/* get all drqs in status register */
#define VDMA_GetSTATUS_DRQS(cstatus)    (((cstatus) & VDMA_STATUS_DRQS) >> 4)

/* nxvm defined vdma in-service-register bits */
#define VDMA_GetISR_ISR(cisr) (((cisr) & VDMA_ISR_ISR) >> 4)
#define VDMA_SetISR(cisr, id) ((cisr) = (VDMA_ISR_IS | ((id) << 4)))

void core_machine_dma_initialize(t_latch *latch, t_dma *primary,
    t_dma *secondary, t_port *port);
void core_machine_dma_reset(t_latch *latch, t_dma *primary,
    t_dma *secondary);
void core_machine_dma_refresh(t_latch *latch, t_dma *primary,
    t_dma *secondary, t_ram *ram);
void core_machine_dma_set_drq(t_dma *primary, t_dma *secondary,
    ntvdm64_type_unsigned_8 drq_id);
void core_machine_dma_add_device(t_dma *primary, t_dma *secondary,
    ntvdm64_type_unsigned_8 drq_id, ntvdm64_type_flat_address read_device, ntvdm64_type_flat_address write_device,
    ntvdm64_type_flat_address close_device);
void core_machine_dma_bind_device(t_dma *primary, t_dma *secondary,
    ntvdm64_type_unsigned_8 drq_id, core_machine_dma_device_provider read_provider,
    core_machine_dma_device_provider write_provider,
    core_machine_dma_device_provider close_provider, void *owner);
void core_machine_dma_finalize(t_latch *latch, t_dma *primary,
    t_dma *secondary);

#define VDMA_POST "\
; init vdma      \n\
mov al, 00       \n\
out 08, al ;     \n\
out d0, al ;     \n\
mov al, c0       \n\
out d6, al ;     \n"

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
