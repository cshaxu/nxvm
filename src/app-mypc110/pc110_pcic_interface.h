/* Copyright 2012-2026 Neko. */
#ifndef MYPC110_PCIC_INTERFACE_H
#define MYPC110_PCIC_INTERFACE_H

#include "x86-core/machine/machine_interface.h"
#include "x86-nxvm/board-base/pic_bus_interface.h"
#include "x86-nxvm/chips/rb5c396/rb5c396_interface.h"
#include "lib/storage/medium_interface.h"
#include "app-mypc110/pc110_firmware_interface.h"

typedef struct mypc110_pcic {
    x86_rb5c396 *controller;
    x86_nxvm_board_pic_bus *master;
    x86_nxvm_board_pic_bus *slave;
    x86_nxvm_board_pic_irq_source *irq[X86_RB5C396_SLOT_COUNT];
    x86_nxvm_board_pic_irq_source *io_irq[X86_RB5C396_SLOT_COUNT];
    lib_u8 selected_irq[X86_RB5C396_SLOT_COUNT];
    lib_u8 selected_io_irq[X86_RB5C396_SLOT_COUNT];
    mypc110_firmware *firmware;
    lib_bool resume_button;
    lib_bool resume_signal;
    x86_core_machine *machine;
    lib_storage_medium *common[X86_RB5C396_SLOT_COUNT];
    lib_storage_medium *attribute[X86_RB5C396_SLOT_COUNT];
    lib_bool readonly[X86_RB5C396_SLOT_COUNT];
    lib_u32 range_first;
    lib_size range_bytes;
} mypc110_pcic;

lib_status mypc110_pcic_initialize(mypc110_pcic *pcic, x86_rb5c396 *controller,
    x86_nxvm_board_pic_bus *master, x86_nxvm_board_pic_bus *slave);
lib_status mypc110_pcic_set_slot_inputs(mypc110_pcic *pcic, lib_u8 slot,
    const x86_rb5c396_slot_inputs *inputs);
lib_status mypc110_pcic_set_resume_button(mypc110_pcic *pcic, lib_bool asserted);
void mypc110_pcic_reset(mypc110_pcic *pcic);
lib_status mypc110_pcic_install_routes(x86_core_machine *machine,
    mypc110_pcic *pcic);
/* The caller owns both medium leases and detaches them before destruction. */
lib_status mypc110_pcic_set_memory_card(mypc110_pcic *pcic, lib_u8 slot,
    lib_storage_medium *common, lib_storage_medium *attribute, lib_bool readonly);

#endif
