/* Copyright 2012-2026 Neko. */
#ifndef MYPC110_PCIC_INTERFACE_H
#define MYPC110_PCIC_INTERFACE_H

#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"
#include "x86-nxvm/chips/rb5c396/rb5c396_interface.h"

typedef struct mypc110_pcic {
    x86_rb5c396 *controller;
    x86_nxvm_board_pic_bus *master;
    x86_nxvm_board_pic_bus *slave;
    x86_nxvm_board_pic_irq_source *irq[X86_RB5C396_SLOT_COUNT];
    lib_u8 selected_irq[X86_RB5C396_SLOT_COUNT];
} mypc110_pcic;

lib_status mypc110_pcic_initialize(mypc110_pcic *pcic, x86_rb5c396 *controller,
    x86_nxvm_board_pic_bus *master, x86_nxvm_board_pic_bus *slave);
lib_status mypc110_pcic_set_slot_inputs(mypc110_pcic *pcic, lib_u8 slot,
    const x86_rb5c396_slot_inputs *inputs);
void mypc110_pcic_reset(mypc110_pcic *pcic);
lib_status mypc110_pcic_install_routes(x86_core_machine *machine,
    mypc110_pcic *pcic);

#endif
