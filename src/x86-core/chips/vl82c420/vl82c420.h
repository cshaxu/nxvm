#ifndef X86_VL82C420_H
#define X86_VL82C420_H

#include "x86-core/chips/vl82c420/vl82c420_interface.h"

struct x86_vl82c420 {
    lib_u8 scamp[128];
    lib_u8 block2[256];
    lib_u8 eced[256];
    lib_u8 selected[3];
    lib_u8 unlock_progress;
    lib_bool block2_unlocked;
    lib_bool smi_armed;
    lib_bool smi_asserted;
};

#endif
