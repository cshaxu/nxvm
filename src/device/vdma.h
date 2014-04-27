/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VDMA_H
#define NXVM_VDMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

#define NXVM_DEVICE_DMA "Intel 8237A"

typedef t_nubit8 t_page;

typedef struct {
	t_nubit16 baseaddr; /* base address */
	t_nubit16 basewc;   /* base word count */
	t_nubit16 curraddr; /* current address */
	t_nubit16 currwc;   /* current word count */
	t_nubit6  mode;     /* mode register */
	t_page    page;     /* page register */
	t_faddrcc devread;  /* get data from device to latch */
	t_faddrcc devwrite; /* write data to device from latch */
	t_faddrcc devfinal; /* send eop signal to device */
} t_dma_channel;

typedef struct {
	t_dma_channel channel[4];
	t_nubit8      command;
	t_nubit8      status;
	t_nubit4      mask;
	t_nubit4      request;
	t_nubit8      temp;
	t_bool        flagmsb; /* flip-flop for msb/lsb */
	t_nubit8      drx;     /* dreq id of highest priority */
	t_bool        flageop; /* end of process */
	t_nubit8      isr;     /* id of request in service in D5-D4,
	                        * flag of in service in D0 */
} t_dma;

typedef union {
	t_nubit8  byte;
	t_nubit16 word;
} t_latch;

extern t_latch vlatch;
extern t_dma vdma1, vdma2;

/* command register bits */
/* memory-to-memory enable */
#define VDMA_GetM2M(vdma)    (!!((vdma)->command & 0x01))
/* channel 0 address hold */    
#define VDMA_GetC0AD(vdma)   (!!((vdma)->command & 0x02))
/* dma controller disable */
#define VDMA_GetCTRL(vdma)   (!!((vdma)->command & 0x04))
/* normal(0) or compressed(1) timing */
#define VDMA_GetTM(vdma)     (!!((vdma)->command & 0x08))
/* normal(0) or rotating(1) priority */
#define VDMA_GetR(vdma)      (!!((vdma)->command & 0x10))
/* late(0) or extended(1) write selection */
#define VDMA_GetWS(vdma)     (!!((vdma)->command & 0x20))
/* dreq sense active high(0) or low(1) */
#define VDMA_GetDREQSA(vdma) (!!((vdma)->command & 0x40))
/* dack sense active low(0) or high(1) */
#define VDMA_GetDACKSA(vdma) (!!((vdma)->command & 0x80))

/* mode register bits */
/* verify(0) or write(1) or read(2) or illegal(3) */
#define VDMA_GetCS(vdma,id)   (((vdma)->channel[(id)].mode & 0x03))
/* autoinitialization enable */
#define VDMA_GetAI(vdma,id)   (!!((vdma)->channel[(id)].mode & 0x04))
/* address increment(0) or decrement(1) select */
#define VDMA_GetAIDS(vdma,id) (!!((vdma)->channel[(id)].mode & 0x08))
/* demand(0) or single(1) or block(2) or cascade(3) mode select */
#define VDMA_GetM(vdma,id)    (((vdma)->channel[(id)].mode & 0x30) >> 4)
             
/* request register bits */
#define VDMA_GetREQ(vdma,id) (!!((vdma)->request & (0x01 << (id))))
/* mask register bits */
#define VDMA_GetMASK(vdma,id) (!!((vdma)->mask    & (0x01 << (id))))
/* status register bits */
#define VDMA_GetTC(vdma,id)  (!!((vdma)->status  & (0x01 << (id))))
#define VDMA_GetDRQ(vdma,id) (!!((vdma)->status  & (0x10 << (id))))
/* in service bits */
#define VDMA_GetIS(vdma)  ((vdma)->isr & 0x01)
#define VDMA_GetISR(vdma) ((vdma)->isr >> 4)

void vdmaSetDRQ(t_nubit8 dreqid);

void vdmaRegister();

#define VDMA_POST "           \
; init vdma                 \n\
mov al, 00                  \n\
out 08, al ;                \n\
out d0, al ;                \n\
mov al, c0                  \n\
out d6, al ;                \n"

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
