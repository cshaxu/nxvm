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
	t_bool    flaginit;
	t_bool    flagrun;
	t_bool    flagtrace;
	t_bool    flagrecord;
	t_bool    flagdisplay;
} t_machine;

extern t_machine vmachine;

void vmachineSetRecordFile(t_string fname);
void vmachineInsertFloppy(t_string fname);
void vmachineRemoveFloppy(t_string fname);
void vmachineStart();

void vmachineRefresh();
void vmachineInit();
void vmachineFinal();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
