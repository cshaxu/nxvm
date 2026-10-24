/* Copyright 2012-2026 Neko. */
#ifndef X86_F65535_H
#define X86_F65535_H

#include "x86-nxvm/chips/f65535/f65535_interface.h"

struct x86_f65535 {
    lib_u8 extension_index;
    lib_u8 extensions[X86_F65535_EXTENSION_REGISTER_COUNT];
    lib_u8 vram[X86_F65535_VRAM_BYTES];
};

#endif
