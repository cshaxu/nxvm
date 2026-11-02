/* Copyright 2012-2026 Neko. */
#include "app-mypc110/pc110_pcic_interface.h"

static lib_status mypc110_pcic_refresh(mypc110_pcic *pcic)
{
    lib_u8 selected;
    lib_status status;
    for (lib_u8 slot = 0u; slot < X86_RB5C396_SLOT_COUNT; ++slot) {
        status = x86_rb5c396_status_irq(pcic->controller, slot, &selected);
        if (status != LIB_STATUS_OK) return status;
        if (selected == pcic->selected_irq[slot]) continue;
        x86_nxvm_board_pic_irq_source_deassert(pcic->irq[slot]);
        if (selected != 0u) {
            status = x86_nxvm_board_pic_irq_source_bind(&pcic->irq[slot],
                pcic->master, pcic->slave, selected);
            if (status != LIB_STATUS_OK) return status;
            x86_nxvm_board_pic_irq_source_assert(pcic->irq[slot]);
        }
        pcic->selected_irq[slot] = selected;
    }
    return LIB_STATUS_OK;
}

lib_status mypc110_pcic_initialize(mypc110_pcic *pcic, x86_rb5c396 *controller,
    x86_nxvm_board_pic_bus *master, x86_nxvm_board_pic_bus *slave)
{
    lib_status status;
    if (pcic == LIB_NULL || controller == LIB_NULL || master == LIB_NULL || slave == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *pcic = (mypc110_pcic){.controller = controller, .master = master, .slave = slave};
    for (lib_u8 slot = 0u; slot < X86_RB5C396_SLOT_COUNT; ++slot) {
        status = x86_nxvm_board_pic_irq_source_bind(&pcic->irq[slot], master, slave, 3u);
        if (status != LIB_STATUS_OK) return status;
    }
    return mypc110_pcic_refresh(pcic);
}

lib_status mypc110_pcic_set_slot_inputs(mypc110_pcic *pcic, lib_u8 slot,
    const x86_rb5c396_slot_inputs *inputs)
{
    lib_status status;
    if (pcic == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_rb5c396_set_slot_inputs(pcic->controller, slot, inputs);
    return status == LIB_STATUS_OK ? mypc110_pcic_refresh(pcic) : status;
}

void mypc110_pcic_reset(mypc110_pcic *pcic)
{
    if (pcic == LIB_NULL) return;
    for (lib_u8 slot = 0u; slot < X86_RB5C396_SLOT_COUNT; ++slot) {
        x86_nxvm_board_pic_irq_source_deassert(pcic->irq[slot]);
        pcic->selected_irq[slot] = 0u;
    }
    x86_rb5c396_reset(pcic->controller);
}

static lib_status mypc110_pcic_port_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    lib_u8 value;
    lib_status status;

    (void)tick;
    if (out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    mypc110_pcic *pcic = owner;
    status = x86_rb5c396_read(pcic->controller, port, &value);
    if (status == LIB_STATUS_OK) status = mypc110_pcic_refresh(pcic);
    if (status == LIB_STATUS_OK) *out_value = value;
    return status;
}

static lib_status mypc110_pcic_port_write(void *owner, lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    if (access_bytes != 1u || value > 0xffu) return LIB_STATUS_INVALID_ARGUMENT;
    mypc110_pcic *pcic = owner;
    const lib_status status = x86_rb5c396_write(pcic->controller, port, (lib_u8)value);
    return status == LIB_STATUS_OK ? mypc110_pcic_refresh(pcic) : status;
}

lib_status mypc110_pcic_install_routes(x86_core_machine *machine,
    mypc110_pcic *pcic)
{
    const x86_core_machine_port_route routes[] = {
        {X86_RB5C396_INDEX_PORT, mypc110_pcic_port_read,
            mypc110_pcic_port_write, pcic, LIB_FALSE, X86_RB5C396_DATA_PORT},
        {X86_RB5C396_DATA_PORT, mypc110_pcic_port_read,
            mypc110_pcic_port_write, pcic, LIB_FALSE, 0u}
    };

    if (machine == LIB_NULL || pcic == LIB_NULL || pcic->controller == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    return x86_core_machine_install_port_routes(machine, routes,
        sizeof(routes) / sizeof(routes[0]));
}
