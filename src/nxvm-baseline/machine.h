/* Copyright 2012-2014 Neko. */

#ifndef NXVM_MACHINE_H
#define NXVM_MACHINE_H

#ifdef __cplusplus
extern "C" {
#endif

void machineStart();
void machineReset();
void machineStop();
void machineResume();

void machineInit();
void machineFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
