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
#include "vcpu.h"
#include "vram.h"
#include "vpic.h"
/*#include "vpit.h"
#include "vcmos.h"
#include "vdma.h"
#include "vfdc.h"
#include "vfdd.h"
#include "vkbc.h"
#include "vkeyb.h"*/

#ifdef VMACHINE_DEBUG
#include "qdbios.h"
#include "qdfdd.h"
#define vfdd qdfdd
#endif

typedef struct {
	t_bool    flagmode;         /* mode flag: console (0) or application (1) */
	t_bool    flaginit;      /* vmachine is initialized (1) or finalized (0) */
	t_bool    flagrun;         /* vmachine is running (1) or not running (0) */
	t_bool    flagtrace;
	t_bool    flagbreak;                    /* breakpoint set (1) or not (0) */
	t_bool    flagrecord;
	t_nubit16 breakcs, breakip;
} t_machine;

extern t_machine vmachine;

void vmachineSetRecordFile(t_string fname);
void vmachineInsertFloppy(t_string fname);
void vmachineRemoveFloppy(t_string fname);
void vmachineStart();
void vmachineStop();

void vmachineRefresh();
void vmachineInit();
void vmachineFinal();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
