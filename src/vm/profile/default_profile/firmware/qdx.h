/* Copyright 2012-2014 Neko. */

#ifndef NXVM_QDX_H
#define NXVM_QDX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "core/machine/vglobal.h"

typedef struct {
    t_faddrcc table[0x100];
} t_qdx;

t_qdx *vm_profile_default_qdx_current(void);
void vm_profile_default_qdx_bind_live(t_qdx *qdx);
void vm_profile_default_qdx_unbind_live(void);

/* Transitional direct alias to the one composition-owned QDX table. */
#define qdxTable (vm_profile_default_qdx_current()->table)

void qdxExecInt(t_nubit8 intId);

void qdxInit();
void qdxReset();
void qdxRefresh();
void qdxFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
