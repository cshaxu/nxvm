/* Copyright 2012-2014 Neko. */

#ifndef NXVM_DEBUG_H
#define NXVM_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

void debugMain();
void debugRecordStart(const t_strptr fileName);
void debugRecordStop();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
