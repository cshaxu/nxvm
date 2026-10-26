/* Copyright 2012-2026 Neko. */
#ifndef X86_RB5C396_H
#define X86_RB5C396_H

#include "x86-nxvm/chips/rb5c396/rb5c396_interface.h"

struct x86_rb5c396 {
    lib_u8 index;
    lib_u8 registers[X86_RB5C396_SLOT_COUNT][X86_RB5C396_REGISTER_COUNT];
    x86_rb5c396_slot_inputs inputs[X86_RB5C396_SLOT_COUNT];
};

#endif
