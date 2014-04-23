/* This file is a part of NXVM project. */

/* Direct Memory Access Controller: Intel 8237A (Master+Slave) */

#ifndef NXVM_VDMA_H
#define NXVM_VDMA_H

#include "vglobal.h"

#define VDMA_DEBUG

typedef t_nubit8 t_page;

typedef struct {
	t_nubit16 baseaddr;                                      /* base address */
	t_nubit16 basewc;                                     /* base word count */
	t_nubit16 curraddr;                                   /* current address */
	t_nubit16 currwc;                                  /* current word count */
	t_nubit6  mode;                                         /* mode register */
	t_page    page;                                         /* page register */
	t_faddrcc devread;                  /* get data from device to dma_latch */
	t_faddrcc devwrite;               /* write data to device from dma_latch */
} t_dma_channel;

typedef struct {
	t_dma_channel channel[4];
	t_nubit8      command;
	t_nubit8      status;
	t_nubit4      mask;
	t_nubit4      request;
	t_nubit8      temp;
	t_bool        flagmsb;                          /* flip-flop for msb/lsb */
	t_nubit8      drx;                        /* dreq id of highest priority */
	t_bool        eop;                                     /* end of process */
	t_nubit8      isr;
              /* id of request in service in D5-D4, flag of in service in D0 */
} t_dma;

typedef union {
	t_nubit8  byte;
	t_nubit16 word;
} t_dma_latch;

extern t_dma_latch vdmalatch;
extern t_dma vdma1,vdma2;

void IO_Read_0000();
void IO_Read_0001();
void IO_Read_0002();
void IO_Read_0003();
void IO_Read_0004();
void IO_Read_0005();
void IO_Read_0006();
void IO_Read_0007();
void IO_Read_0008();
void IO_Read_000D();

void IO_Write_0000();
void IO_Write_0001();
void IO_Write_0002();
void IO_Write_0003();
void IO_Write_0004();
void IO_Write_0005();
void IO_Write_0006();
void IO_Write_0007();
void IO_Write_0008();
void IO_Write_0009();
void IO_Write_000A();
void IO_Write_000B();
void IO_Write_000C();
void IO_Write_000D();
void IO_Write_000E();
void IO_Write_000F();

void IO_Read_0081();
void IO_Read_0082();
void IO_Read_0083();
void IO_Read_0087();
void IO_Read_0089();
void IO_Read_008A();
void IO_Read_008B();
void IO_Read_008F();

void IO_Write_0081();
void IO_Write_0082();
void IO_Write_0083();
void IO_Write_0087();
void IO_Write_0089();
void IO_Write_008A();
void IO_Write_008B();
void IO_Write_008F();

void IO_Read_00C0();
void IO_Read_00C2();
void IO_Read_00C4();
void IO_Read_00C6();
void IO_Read_00C8();
void IO_Read_00CA();
void IO_Read_00CC();
void IO_Read_00CE();
void IO_Read_00D0();
void IO_Read_00DA();

void IO_Write_00C0();
void IO_Write_00C2();
void IO_Write_00C4();
void IO_Write_00C6();
void IO_Write_00C8();
void IO_Write_00CA();
void IO_Write_00CC();
void IO_Write_00CE();
void IO_Write_00D0();
void IO_Write_00D2();
void IO_Write_00D4();
void IO_Write_00D6();
void IO_Write_00D8();
void IO_Write_00DA();
void IO_Write_00DC();
void IO_Write_00DE();

#ifdef VDMA_DEBUG
void IO_Read_FF00();                                /* print all info of dma */
void IO_Write_FF00();                         /* vdmaReset - Clear Registers */
void IO_Write_FF01();                           /* vdmaSetDRQ - Hardware DREQ*/
void IO_Write_FF02();                    /* vdmaRefresh - Detect and Execute */
#endif

void vdmaSetDRQ(t_nubit8 dreqid);

void vdmaReset();
void vdmaRefresh();
void vdmaInit();
void vdmaFinal();

#endif