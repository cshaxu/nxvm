#ifndef NXVM_QDBIOS_H
#define NXVM_QDBIOS_H

#include "vglobal.h"

void INT_16();

t_bool qdbiosExecInt(t_nubit8 intid);
void qdbiosInit();
void qdbiosFinal();

#endif