/* Copyright 2012-2014 Neko. */

/* NXVMAPI implements all functions that are called from vcpu.c
 * and vcpuins.c, and these functions are connected to bochsapi. */

#include "bochsapi.h"

#include "../src/utils.h"
#include "../src/device/device.h"
#include "../src/device/vport.h"
#include "../src/device/vram.h"
#include "../src/device/vpic.h"

/* utils */
void utilsTracePrint(t_utils_trace *rtrace) {}
int PRINTF(const char *_Format, ...) {
    int nWrittenBytes = 0;
    va_list arg_ptr;
    va_start(arg_ptr, _Format);
    nWrittenBytes = vfprintf(stdout, _Format, arg_ptr);
    va_end(arg_ptr);
    fflush(stdout);
    return nWrittenBytes;
}
void* MEMSET(void *_Dst, int _Val, size_t _Size) {
    return memset(_Dst, _Val, _Size);
}
void* MEMCPY(void *_Dst, const void *_Src, size_t _Size) {
    return memcpy(_Dst, _Src, _Size);
}
void utilsSleep(unsigned int milisec) {}

/* device */
void deviceStop() {
    bochsApiDeviceStop();
}

/* vport */
t_port vport;
void vportExecRead(t_nubit16 portId) {}
void vportExecWrite(t_nubit16 portId) {}

/* vram */
void vramReadPhysical(t_nubit32 physical, t_vaddrcc rdest, t_nubitcc byte) {
    bochsApiRamReadPhysical(physical, rdest, (t_nubit8) byte);
}
void vramWritePhysical(t_nubit32 physical, t_vaddrcc rsrc, t_nubitcc byte) {
    bochsApiRamWritePhysical(physical, rsrc, (t_nubit8) byte);
}

/* vpic */
t_bool vpicScanINTR() {
    return False;
}
t_nubit8 vpicGetINTR() {
    return 0xff;
}
