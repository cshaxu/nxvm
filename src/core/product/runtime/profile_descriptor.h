#ifndef NXVM_RUNTIME_PROFILE_DESCRIPTOR_H
#define NXVM_RUNTIME_PROFILE_DESCRIPTOR_H

#include "type.h"


#ifdef __cplusplus
extern "C" {
#endif

enum {
    NXVM_RUNTIME_DEVICE_CPU = UINT64_C(1) << 0,
    NXVM_RUNTIME_DEVICE_RAM = UINT64_C(1) << 1,
    NXVM_RUNTIME_DEVICE_PORTS = UINT64_C(1) << 2,
    NXVM_RUNTIME_DEVICE_PIC = UINT64_C(1) << 3,
    NXVM_RUNTIME_DEVICE_PIT = UINT64_C(1) << 4,
    NXVM_RUNTIME_DEVICE_KEYBOARD = UINT64_C(1) << 5,
    NXVM_RUNTIME_DEVICE_TEXT_DISPLAY = UINT64_C(1) << 6,
    NXVM_RUNTIME_DEVICE_BIOS = UINT64_C(1) << 7,
    NXVM_RUNTIME_DEVICE_CMOS = UINT64_C(1) << 8,
    NXVM_RUNTIME_DEVICE_DMA = UINT64_C(1) << 9,
    NXVM_RUNTIME_DEVICE_FDC = UINT64_C(1) << 10,
    NXVM_RUNTIME_DEVICE_FDD = UINT64_C(1) << 11,
    NXVM_RUNTIME_DEVICE_HDC = UINT64_C(1) << 12,
    NXVM_RUNTIME_DEVICE_HDD = UINT64_C(1) << 13,
    NXVM_RUNTIME_DEVICE_VADP = UINT64_C(1) << 14
};

typedef struct core_product_runtime_profile_descriptor {
    const C_CHAR *name;
    uint64_t devices;
    C_INT permits_disk_boot;
    C_INT uses_legacy_adapter;
} core_product_runtime_profile_descriptor;

#ifdef __cplusplus
}
#endif

#endif
