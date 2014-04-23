#ifndef NXVM_QDBIOS_H
#define NXVM_QDBIOS_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "vglobal.h"

void qdbiosExecInt(t_nubit8 intid);
void qdbiosRefresh();
void qdbiosInit();
void qdbiosFinal();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif