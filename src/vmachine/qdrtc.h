#ifndef NXVM_QDRTC_H
#define NXVM_QDRTC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "time.h"
#include "vglobal.h"

typedef struct {
	t_nubit8 iobyte;
	time_t start;
} t_rtc;

extern t_rtc qdrtc;

void IO_Read_0071();
void IO_Write_0070();

void qdrtcGetTimeTickCount();
void qdrtcSetTimeTickCount();
void qdrtcGetCmosTime();
void qdrtcSetCmosTime();
void qdrtcGetCmosDate();
void qdrtcSetCmosDate();
void qdrtcSetAlarmClock();

void qdrtcUpdateTime();

void qdrtcInit();
void qdrtcFinal();

#ifdef __cplusplus
}/*cends*/
#endif

#endif