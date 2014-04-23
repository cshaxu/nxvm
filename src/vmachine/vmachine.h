/* This file is a part of NXVM project. */

// Global Variables and Virtual Machine Assembly

#ifndef NXVM_VMACHINE_H
#define NXVM_VMACHINE_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#define VMACHINE_DEBUG

#include "vglobal.h"
#include "vport.h"
#include "vram.h"
#if VGLOBAL_ECPU_MODE != TEST_VCPU
#include "ecpu/ecpu.h"
#endif
#include "vcpu.h"
#include "vpic.h"
#include "vcmos.h"
#include "vdma.h"
#include "vfdc.h"
#include "vfdd.h"
#include "vhdd.h"
#include "vpit.h"
#include "vkbc.h"
//#include "vkeyb.h"
#include "vvadp.h"
//#include "vdisp.h"


#ifdef VMACHINE_DEBUG
#include "bios/qdbios.h"
#endif

typedef struct {
	t_bool    flagrun;         /* vmachine is running (1) or not running (0) */
	t_bool    flagmode;         /* mode flag: console (0) or application (1) */
	t_bool    flagboot;             /* boot from floppy (0) or hard disk (1) */
	t_bool    flagbreak;                    /* breakpoint set (1) or not (0) */
	t_nubit16 breakcs, breakip;
	t_bool    flagrecord;
	t_nubitcc tracecnt;
	t_bool    flagbreakx;
	t_nubit32 breaklinear;
} t_machine;

extern t_machine vmachine;

void vmachineStart();
void vmachineReset();
void vmachineStop();
void vmachineResume();

void vmachineRefresh();
void vmachineInit();
void vmachineFinal();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
