/* Copyright 2012-2014 Neko. */

#ifndef NXVM_QDBIOS_H
#define NXVM_QDBIOS_H

#ifdef __cplusplus
extern "C" {
#endif

extern t_faddrcc qdbiosInt[0x100];

void qdbiosMakeInt(t_nubit8 intid, t_strptr stmt);
void qdbiosExecInt(t_nubit8 intid);

void qdbiosRegister();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
