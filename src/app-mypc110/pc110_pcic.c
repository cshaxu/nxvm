/* Copyright 2012-2026 Neko. */
#include "app-mypc110/pc110_pcic_interface.h"

static lib_status mypc110_pcic_memory_refresh(mypc110_pcic *pcic)
{
    lib_u32 first = 0u, last = 0u;
    lib_bool found = LIB_FALSE;
    lib_status status;

    if (pcic->machine == LIB_NULL) return LIB_STATUS_OK;
    for (lib_u8 slot = 0u; slot < X86_RB5C396_SLOT_COUNT; ++slot) {
        x86_rb5c396_slot_inputs inputs;
        status = x86_rb5c396_get_slot_inputs(pcic->controller, slot, &inputs);
        if (status != LIB_STATUS_OK) return status;
        if (pcic->common[slot] == LIB_NULL || !inputs.card_present) continue;
        for (lib_u8 window = 0u; window < X86_RB5C396_MEMORY_WINDOW_COUNT; ++window) {
            lib_u32 start, bytes;
            status = x86_rb5c396_memory_window_range(pcic->controller, slot, window,
                &start, &bytes);
            if (status == LIB_STATUS_UNSUPPORTED) continue;
            if (status != LIB_STATUS_OK) return status;
            if (!found || start < first) first = start;
            if (!found || start + bytes > last) last = start + bytes;
            found = LIB_TRUE;
        }
    }
    if (pcic->range_bytes != 0u)
        x86_core_machine_signal_memory_decode_change(pcic->machine,
            pcic->range_first, pcic->range_bytes);
    status = x86_core_machine_set_memory_device_range(pcic->machine, pcic,
        found ? first : 0u, found ? last - first : 0u);
    if (status != LIB_STATUS_OK) return status;
    pcic->range_first = found ? first : 0u;
    pcic->range_bytes = found ? last - first : 0u;
    return LIB_STATUS_OK;
}

static lib_status mypc110_pcic_memory_find(mypc110_pcic *pcic, lib_u32 physical,
    lib_uptr bytes, x86_rb5c396_memory_mapping *mapping, lib_u8 *selected)
{
    for (lib_u8 slot = 0u; slot < X86_RB5C396_SLOT_COUNT; ++slot) {
        x86_rb5c396_slot_inputs inputs;
        lib_status input_status = x86_rb5c396_get_slot_inputs(pcic->controller, slot, &inputs);
        if (input_status != LIB_STATUS_OK) return input_status;
        if (pcic->common[slot] == LIB_NULL || !inputs.card_present) continue;
        for (lib_u8 window = 0u; window < X86_RB5C396_MEMORY_WINDOW_COUNT; ++window) {
            lib_status status = x86_rb5c396_decode_memory(pcic->controller,
                slot, window, physical, mapping);
            if (status == LIB_STATUS_UNSUPPORTED) continue;
            if (status != LIB_STATUS_OK) return status;
            if (bytes > mapping->contiguous_bytes) return LIB_STATUS_UNSUPPORTED;
            *selected = slot;
            return LIB_STATUS_OK;
        }
    }
    return LIB_STATUS_UNSUPPORTED;
}

static lib_status mypc110_pcic_memory_query(void *owner, lib_u32 physical,
    lib_uptr bytes, x86_core_machine_memory_access access)
{
    x86_rb5c396_memory_mapping mapping;
    lib_u8 slot;
    (void)access;
    return mypc110_pcic_memory_find(owner, physical, bytes, &mapping, &slot);
}

static lib_status mypc110_pcic_memory_access(mypc110_pcic *pcic, lib_u32 physical,
    lib_uptr buffer, lib_uptr bytes, lib_bool write)
{
    x86_rb5c396_memory_mapping mapping;
    lib_storage_medium *medium;
    lib_u8 slot;
    lib_size available;
    lib_status status = mypc110_pcic_memory_find(pcic, physical, bytes, &mapping, &slot);
    if (status != LIB_STATUS_OK) return status;
    medium = mapping.attribute ? pcic->attribute[slot] : pcic->common[slot];
    available = medium == LIB_NULL ? 0u : lib_storage_medium_byte_count(medium);
    available = mapping.card_address < available ? available - mapping.card_address : 0u;
    if (available > bytes) available = bytes;
    if (!write) {
        lib_memory_set((void *)buffer, 0xff, bytes);
        return available == 0u ? LIB_STATUS_OK : lib_storage_medium_read_at(medium,
            mapping.card_address, (void *)buffer, available);
    }
    if (available == 0u || pcic->readonly[slot] || mapping.write_protected)
        return LIB_STATUS_OK;
    return lib_storage_medium_write_at(medium, mapping.card_address,
        (const void *)buffer, available);
}

static lib_status mypc110_pcic_memory_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    (void)observe_only;
    return mypc110_pcic_memory_access(owner, physical, destination, bytes, LIB_FALSE);
}

static lib_status mypc110_pcic_memory_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    return mypc110_pcic_memory_access(owner, physical, source, bytes, LIB_TRUE);
}

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
    if (status == LIB_STATUS_OK) status = mypc110_pcic_refresh(pcic);
    if (status == LIB_STATUS_OK) status = mypc110_pcic_memory_refresh(pcic);
    return status;
}

lib_status mypc110_pcic_set_memory_card(mypc110_pcic *pcic, lib_u8 slot,
    lib_storage_medium *common, lib_storage_medium *attribute, lib_bool readonly)
{
    const x86_rb5c396_slot_inputs inputs = {
        .card_present = common != LIB_NULL, .ready = common != LIB_NULL,
        .write_protected = readonly};
    lib_status status;

    if (pcic == LIB_NULL || slot >= X86_RB5C396_SLOT_COUNT ||
        (common == LIB_NULL && attribute != LIB_NULL)) return LIB_STATUS_INVALID_ARGUMENT;
    pcic->common[slot] = common;
    pcic->attribute[slot] = attribute;
    pcic->readonly[slot] = readonly;
    status = mypc110_pcic_set_slot_inputs(pcic, slot, &inputs);
    return status;
}

void mypc110_pcic_reset(mypc110_pcic *pcic)
{
    if (pcic == LIB_NULL) return;
    for (lib_u8 slot = 0u; slot < X86_RB5C396_SLOT_COUNT; ++slot) {
        x86_nxvm_board_pic_irq_source_deassert(pcic->irq[slot]);
        pcic->selected_irq[slot] = 0u;
    }
    x86_rb5c396_reset(pcic->controller);
    if (mypc110_pcic_memory_refresh(pcic) != LIB_STATUS_OK && pcic->machine != LIB_NULL)
        (void)x86_core_machine_report_fault(pcic->machine, 0x50434943u);
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
    lib_u8 index = 0u;
    lib_status status;
    if (port == X86_RB5C396_DATA_PORT) {
        status = x86_rb5c396_read(pcic->controller, X86_RB5C396_INDEX_PORT, &index);
        if (status != LIB_STATUS_OK) return status;
    }
    status = x86_rb5c396_write(pcic->controller, port, (lib_u8)value);
    if (status == LIB_STATUS_OK && port == X86_RB5C396_DATA_PORT &&
        (index & 0x80u) == 0u) {
        const lib_u8 reg = index & 0x3fu;
        if (reg == 0x06u || (reg >= 0x10u && reg < 0x38u &&
            ((reg - 0x10u) & 7u) < 6u))
            status = mypc110_pcic_memory_refresh(pcic);
    }
    return status == LIB_STATUS_OK ? mypc110_pcic_refresh(pcic) : status;
}

lib_status mypc110_pcic_install_routes(x86_core_machine *machine,
    mypc110_pcic *pcic)
{
    const x86_core_machine_memory_device_route memory = {
        0u, 1u, {mypc110_pcic_memory_read, mypc110_pcic_memory_write,
            mypc110_pcic_memory_query}, X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT_PARTIAL,
        LIB_NULL};
    const x86_core_machine_port_route routes[] = {
        {X86_RB5C396_INDEX_PORT, mypc110_pcic_port_read,
            mypc110_pcic_port_write, pcic, LIB_FALSE, X86_RB5C396_DATA_PORT},
        {X86_RB5C396_DATA_PORT, mypc110_pcic_port_read,
            mypc110_pcic_port_write, pcic, LIB_FALSE, 0u}
    };

    if (machine == LIB_NULL || pcic == LIB_NULL || pcic->controller == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    lib_status status = x86_core_machine_install_memory_device_routes(machine,
        &memory, 1u, LIB_NULL, LIB_NULL, pcic);
    if (status != LIB_STATUS_OK) return status;
    pcic->machine = machine;
    status = mypc110_pcic_memory_refresh(pcic);
    if (status == LIB_STATUS_OK)
        status = x86_core_machine_install_port_routes(machine, routes,
            sizeof(routes) / sizeof(routes[0]));
    return status;
}
