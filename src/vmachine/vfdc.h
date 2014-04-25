/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VFDC_H
#define NXVM_VFDC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

#define NXVM_DEVICE_FDC "Intel 8272A"

typedef struct {
	t_nubit8 dor; /* digital output register */
	t_nubit8 msr; /* main status register */
	t_nubit8 dr;  /* data register */
	t_nubit8 dir; /* digital input register */
	t_nubit8 ccr; /* configuration control register */

	t_nubit4 hut; /* head unload time */
	t_nubit4 hlt; /* head load time */
	t_nubit8 srt; /* step rate time */
	t_bool flagndma; /* 0 = dma mode; 1 = non-dma mode */
	t_bool flagintr; /* 0 = no intr; 1 = has intr */

	t_nubit8 rwid; /* io port command/result rw id */
	t_nubit8 cmd[9];
	t_nubit8 ret[7];
	t_nubit8 st0,st1,st2,st3; /* state registers */
} t_fdc;

extern t_fdc vfdc;

/* Command Bytes */
//#define VFDC_Get_A0(cbyte)  /* Address Line 0 */
//#define VFDC_Get_C(cbyte)   /* Cylinder Number (0-79) */
//#define VFDC_Get_D(cbyte)   /* Data */
//#define VFDC_Get_DB(cbyte)  /* Data Bus (D7-D0) */
//#define VFDC_Get_DTL(cbyte) /* Data Length */
//#define VFDC_Get_EOT(cbyte) /* End Of Track */
//#define VFDC_Get_GPL(cbyte) /* Gap Length */
//#define VFDC_Get_H(cbyte)   /* Head Address (0-1) */
//#define VFDC_Get_MFM(cbyte) /* fm or mfm mode */
//#define VFDC_Get_MT(cbyte)  /* multi-track */
//#define VFDC_Get_N(cbyte)   /* number */
//#define VFDC_Get_NCN(cbyte) /* new cylinder number */
//#define VFDC_Get_R(cbyte)   /* record */
//#define VFDC_Get_RW(cbyte)  /* read/write signal */
//#define VFDC_Get_SC(cbyte)  /* sector */
//#define VFDC_Get_SK(cbyte)  /* skip */
//#define VFDC_Get_STP(cbyte) /* step */
#define VFDC_GetENRQ(cbyte) ((cbyte) & 0x08)     /* enable dma and intr */
#define VFDC_GetDS(cbyte)   ((cbyte) & 0x03)     /* drive select (ds0,ds1) */
#define VFDC_GetHUT(cbyte)  ((cbyte) & 0x0f)     /* head unload time */
#define VFDC_GetSRT(cbyte)  ((cbyte) >> 4)       /* step rate time */
#define VFDC_GetHLT(cbyte)  ((cbyte) >> 1)       /* head load time */
#define VFDC_GetNDMA(cbyte) ((cbyte) & 0x01)     /* non-dma mode */
#define VFDC_GetHDS(cbyte)  (!!((cbyte) & 0x04)) /* head select (0 or 1) */
#define VFDC_GetBPS(cbyte)  (0x0080 << (cbyte))  /* bytes per sector */
/* sector size code */
t_nubit8 VFDC_GetBPSC(t_nubit16 cbyte);

#define vfdcPIORead   vfdcTransRead
#define vfdcDMARead   vfdcTransRead
#define vfdcPIOWrite  vfdcTransWrite
#define vfdcDMAWrite  vfdcTransWrite
#define vfdcPIOFinal  vfdcTransFinal
#define vfdcDMAFinal  vfdcTransFinal

void vfdcTransRead();
void vfdcTransWrite();
void vfdcTransInit();
void vfdcTransFinal();

void vfdcRegister();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
