/* This file is a part of NXVM project. */

#ifndef NXVM_RECORD_H
#define NXVM_RECORD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../vcpuins.h"

#define RECORD_SIZE         0xffff             /* maximum record number */
#define RECORD_SELECT_FIRST 0x0000/* keep first (1) records or last (0) */

typedef struct {
	t_cpurec rec[RECORD_SIZE];
	t_nubitcc start, size;
	t_bool flagnow; /* record now! */
	char fn[0x100];
	FILE *fp;
} t_record;

extern t_record vrecord;

void recordNow(const t_strptr fname);
void recordDump(const t_strptr fname);
void recordInit();
void recordExec(t_cpurec *rcpurec);
void recordFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
