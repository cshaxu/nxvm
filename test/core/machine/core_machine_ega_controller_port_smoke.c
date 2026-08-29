#include "type.h"

#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"

static C_INT core_machine_ega_controller_write(t_ram *memory, type_unsigned_32 physical,
    type_unsigned_8 value)
{
    return core_machine_memory_write_physical(memory, physical,
        (type_virtual_address)&value, sizeof(value)) == TYPE_STATUS_OK;
}

static C_INT core_machine_ega_controller_read(t_ram *memory, type_unsigned_32 physical,
    type_unsigned_8 *value)
{
    return core_machine_memory_read_physical(memory, physical,
        (type_virtual_address)value, sizeof(*value)) == TYPE_STATUS_OK;
}

C_INT main(C_VOID)
{
    const core_machine_vadp_ega_sequencer_config sequencer = {
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, TYPE_FALSE
    };
    const core_machine_vadp_ega_controller_config controllers = {
        { 0xf0u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0xf5u, 0x00u, 0xffu },
        { 0xffu, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
            0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
            0x01u, 0x00u, 0x0fu, 0x00u, 0x00u }
    };
    t_port port;
    t_ram memory;
    t_vadp vadp;
    type_unsigned_8 value = 0u;
    type_unsigned_64 dirty_generation;
    C_INT failed = 0;

    STD_MEMSET(&memory, 0, sizeof(memory));
    core_machine_port_initialize(&port);
    core_machine_memory_initialize(&memory);
    core_machine_vadp_initialize(&vadp, &port);
    core_machine_vadp_configure_ega_ports(&vadp, &port);
    failed |= core_machine_vadp_configure_ega_sequencer(&vadp, &memory,
        &sequencer) != TYPE_STATUS_OK;
    failed |= core_machine_vadp_configure_ega_controllers(&vadp,
        &controllers) != TYPE_STATUS_OK;

    failed |= core_machine_port_read(&port, 0x03ceu) != 0u;
    failed |= core_machine_port_read(&port, 0x03cfu) != 0u;
    core_machine_port_write(&port, 0x03ceu, 6u);
    failed |= core_machine_port_read(&port, 0x03cfu) != 0x05u;
    core_machine_port_write(&port, 0x03ceu, 0u);
    failed |= core_machine_port_read(&port, 0x03cfu) != 0u;
    core_machine_port_write(&port, 0x03ceu, 6u);
    core_machine_port_write(&port, 0x03cfu, 0xffu);
    failed |= core_machine_port_read(&port, 0x03cfu) != 0x0fu ||
        vadp.data.graphics[6] != 0x0fu;
    core_machine_port_write(&port, 0x03ceu, 31u);
    core_machine_port_write(&port, 0x03cfu, 0xa5u);
    failed |= core_machine_port_read(&port, 0x03cfu) != 0u;
    core_machine_port_write(&port, 0x03ceu, 6u);
    failed |= core_machine_port_read(&port, 0x03cfu) != 0x0fu ||
        vadp.data.graphics[6] != 0x0fu;

    core_machine_port_write(&port, 0x03ceu, 6u);
    core_machine_port_write(&port, 0x03cfu, 0x00u);
    failed |= !core_machine_vadp_ega_aperture_contains(&vadp, 0x000a0000u,
        0x00020000u) || core_machine_vadp_ega_aperture_contains(&vadp,
        0x000c0000u, 1u);
    core_machine_port_write(&port, 0x03cfu, 0x05u);
    failed |= !core_machine_vadp_ega_aperture_contains(&vadp, 0x000a0000u,
        0x00010000u) || core_machine_vadp_ega_aperture_contains(&vadp,
        0x000b0000u, 1u);
    core_machine_port_write(&port, 0x03cfu, 0x09u);
    failed |= !core_machine_vadp_ega_aperture_contains(&vadp, 0x000b0000u,
        0x00008000u) || core_machine_vadp_ega_aperture_contains(&vadp,
        0x000a0000u, 1u);
    dirty_generation = vadp.data.dirty_generation;
    failed |= !core_machine_ega_controller_write(&memory, 0x000b0000u, 0x5au);
    failed |= vadp.data.dirty_generation != dirty_generation + 1u;
    dirty_generation = vadp.data.dirty_generation;
    failed |= !core_machine_ega_controller_write(&memory, 0x000a0000u, 0xa5u);
    failed |= vadp.data.dirty_generation != dirty_generation;
    core_machine_port_write(&port, 0x03cfu, 0x0du);
    failed |= !core_machine_vadp_ega_aperture_contains(&vadp, 0x000b8000u,
        0x00008000u) || core_machine_vadp_ega_aperture_contains(&vadp,
        0x000b0000u, 1u);

    core_machine_port_write(&port, 0x03c0u, 0x31u);
    core_machine_port_write(&port, 0x03c0u, 0xffu);
    failed |= core_machine_port_read(&port, 0x03c1u) != 0x3fu ||
        vadp.data.attribute[17] != 0x3fu ||
        !vadp.data.attribute_display_enabled;
    (C_VOID)core_machine_port_read(&port, 0x03dau);
    core_machine_port_write(&port, 0x03c0u, 0x00u);
    failed |= core_machine_port_read(&port, 0x03c1u) != 0x3fu ||
        vadp.data.attribute[0] != 0x3fu;
    (C_VOID)core_machine_port_read(&port, 0x03dau);
    core_machine_port_write(&port, 0x03c0u, 0x12u);
    core_machine_port_write(&port, 0x03c0u, 0xf5u);
    failed |= core_machine_port_read(&port, 0x03c1u) != 0x05u ||
        vadp.data.attribute[18] != 0x05u;
    (C_VOID)core_machine_port_read(&port, 0x03dau);
    core_machine_port_write(&port, 0x03c0u, 0x1fu);
    core_machine_port_write(&port, 0x03c0u, 0xffu);
    failed |= core_machine_port_read(&port, 0x03c1u) != 0u;
    (C_VOID)core_machine_port_read(&port, 0x03dau);
    core_machine_port_write(&port, 0x03c0u, 0x12u);
    failed |= core_machine_port_read(&port, 0x03c1u) != 0x05u ||
        vadp.data.attribute[18] != 0x05u;

    (C_VOID)core_machine_port_read(&port, 0x03dau);
    core_machine_port_write(&port, 0x03c0u, 0x10u);
    core_machine_port_write(&port, 0x03c0u, 0xffu);
    failed |= vadp.data.attribute[16] != 0x0fu;
    (C_VOID)core_machine_port_read(&port, 0x03dau);
    core_machine_port_write(&port, 0x03c0u, 0x13u);
    core_machine_port_write(&port, 0x03c0u, 0xffu);
    failed |= vadp.data.attribute[19] != 0x0fu;
    (C_VOID)core_machine_port_read(&port, 0x03dau);
    core_machine_port_write(&port, 0x03c0u, 0x14u);
    core_machine_port_write(&port, 0x03c0u, 0xffu);
    failed |= vadp.data.attribute[20] != 0u;

    core_machine_port_write(&port, 0x03c4u, 2u);
    core_machine_port_write(&port, 0x03c5u, 0x05u);
    core_machine_port_write(&port, 0x03ceu, 5u);
    core_machine_port_write(&port, 0x03cfu, 0x03u);
    failed |= !core_machine_ega_controller_write(&memory, 0x000b8000u, 0xa6u);
    failed |= !core_machine_ega_controller_read(&memory, 0x000b8000u, &value) ||
        value != 0xa6u;

    core_machine_vadp_reset(&vadp);
    failed |= core_machine_port_read(&port, 0x03c4u) != 0u ||
        core_machine_port_read(&port, 0x03c5u) != 0x03u;
    failed |= core_machine_port_read(&port, 0x03ceu) != 0u ||
        core_machine_port_read(&port, 0x03cfu) != 0u;

    if (failed) {
        STD_FPRINTF(STD_STDERR,
            "M5:T236:S1:EGA-CONTROLLER:FAIL graphics=%02x,%02x attr=%02x phase=%d\n",
            vadp.data.graphics[0], vadp.data.graphics[6], vadp.data.attribute[0],
            vadp.data.attribute_data_phase);
        core_machine_vadp_finalize(&vadp);
        core_machine_memory_finalize(&memory);
        core_machine_port_finalize(&port);
        return 1;
    }
    core_machine_vadp_finalize(&vadp);
    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&port);
    STD_PRINTF("M5:T236:S1:EGA-CONTROLLER:PORT:OK\n");
    STD_PRINTF("M5:T480:S3:COMMON-OWNER:OK\n");
    return 0;
}
