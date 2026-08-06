#include "type.h"

#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"

static C_INT core_machine_ega_planar_write(t_ram *memory, uint32_t physical,
    uint8_t value)
{
    return core_machine_memory_write_physical(memory, physical,
        (type_virtual_address)&value, sizeof(value)) == TYPE_STATUS_OK;
}

static C_INT core_machine_ega_planar_read(t_ram *memory, uint32_t physical,
    uint8_t *value)
{
    return core_machine_memory_read_physical(memory, physical,
        (type_virtual_address)value, sizeof(*value)) == TYPE_STATUS_OK;
}

C_INT main(C_VOID)
{
    const core_machine_vadp_ega_sequencer_config sequencer = {
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, CORE_MACHINE_VADP_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u
    };
    const core_machine_vadp_ega_controller_config controllers = {
        { 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x05u, 0x00u, 0xffu },
        { 0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
            0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
            0x01u, 0x00u, 0x0fu, 0x00u, 0x00u }
    };
    t_port port;
    t_ram memory;
    t_vadp vadp;
    uint8_t value = 0u;
    C_INT failed = 0;

    STD_MEMSET(&memory, 0, sizeof(memory));
    core_machine_port_initialize(&port);
    core_machine_memory_initialize(&memory);
    core_machine_vadp_initialize(&vadp, &port);
    failed |= core_machine_vadp_configure_ega_sequencer(&vadp, &memory,
        &sequencer) != TYPE_STATUS_OK;
    failed |= core_machine_vadp_configure_ega_controllers(&vadp,
        &controllers) != TYPE_STATUS_OK;

    failed |= !core_machine_vadp_ega_aperture_contains(&vadp, 0x000a0000u,
        0x00010000u);
    failed |= core_machine_vadp_ega_aperture_contains(&vadp, 0x000b0000u, 1u);
    failed |= core_machine_vadp_ega_aperture_contains(&vadp, 0x000a0000u,
        0x00010001u);

    core_machine_port_write(&port, 0x03c4u, 2u);
    core_machine_port_write(&port, 0x03c5u, 0x0fu);
    failed |= core_machine_port_read(&port, 0x03c5u) != 0x0fu;
    core_machine_port_write(&port, 0x03ceu, 5u);
    core_machine_port_write(&port, 0x03cfu, 0x00u);
    failed |= core_machine_port_read(&port, 0x03cfu) != 0x00u;
    core_machine_port_write(&port, 0x03ceu, 6u);
    failed |= core_machine_port_read(&port, 0x03cfu) != 0x05u;
    (C_VOID)core_machine_port_read(&port, 0x03dau);
    core_machine_port_write(&port, 0x03c0u, 0x10u);
    core_machine_port_write(&port, 0x03c0u, 0x01u);
    failed |= core_machine_port_read(&port, 0x03c1u) != 0x01u;

    failed |= !core_machine_ega_planar_write(&memory, 0x000a0000u, 0xa5u);
    failed |= !core_machine_ega_planar_read(&memory, 0x000a0000u, &value) ||
        value != 0xa5u;
    core_machine_port_write(&port, 0x03ceu, 6u);
    core_machine_port_write(&port, 0x03cfu, 0x09u);
    failed |= !core_machine_vadp_ega_aperture_contains(&vadp, 0x000b0000u,
        0x00008000u);
    failed |= !core_machine_ega_planar_read(&memory, 0x000a0000u, &value) ||
        value != 0xa5u;

    core_machine_vadp_finalize(&vadp);
    core_machine_memory_finalize(&memory);
    core_machine_port_finalize(&port);
    if (failed) {
        STD_FPRINTF(STD_STDERR, "M5:T238:S1:EGA-PLANAR:PORT:FAIL\n");
        return 1;
    }
    STD_PRINTF("M5:T238:S1:EGA-PLANAR:PORT:OK\n");
    return 0;
}
