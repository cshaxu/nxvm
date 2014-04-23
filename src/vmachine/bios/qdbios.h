/* This file is a part of NXVM project. */

#ifndef NXVM_QDBIOS_H
#define NXVM_QDBIOS_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "../vglobal.h"

extern t_faddrcc qdbiosInt[0x100];

void qdbiosSetInt(t_nubit8 intid, t_nubit16 intcs, t_nubit16 intip);
void qdbiosAsmInt(t_string stmt);
void qdbiosMakeInt(t_nubit8 intid, t_string stmt);
void qdbiosExecInt(t_nubit8 intid);
void qdbiosReset(); /* POST */

#define asmint(n) if (1)

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
