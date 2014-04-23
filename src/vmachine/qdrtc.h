#ifndef NXVM_QDRTC_H
#define NXVM_QDRTC_H

#include "vglobal.h"

void qdrtcGetTimeTickCount();
void qdrtcSetTimeTickCount();
void qdrtcGetCmosTime();
void qdrtcSetCmosTime();
void qdrtcGetCmosDate();
void qdrtcSetCmosDate();
void qdrtcSetAlarmClock();

void qdrtcInit();
void qdrtcFinal();

#endif