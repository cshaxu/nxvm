#ifndef NXVM_QDFDD_H
#define NXVM_QDFDD_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "vglobal.h"
#include "vfdd.h"

t_vaddrcc qdfddGetAddress(t_nubit8 cyl,
                          t_nubit8 head,
                          t_nubit8 sector);
void qdfddResetDrive();
void qdfddGetDiskStatus();
void qdfddReadSector();
void qdfddWriteSector();
void qdfddGetParameter();
void qdfddGetDriveType();

void qdfddInit();
void qdfddFinal();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif