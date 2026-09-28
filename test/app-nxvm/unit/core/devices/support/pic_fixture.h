#ifndef TEST_PIC_FIXTURE_H
#define TEST_PIC_FIXTURE_H
#include "app-nxvm/devices/pic_bus.h"

/* CPU fixtures program a single-controller vector instead of altering ICW2
 * behind the chip's back. This is setup, before any request is raised. */
static inline void test_pic_program_vector(core_machine_pic_bus *bus, lib_u8 vector)
{
    x86_pic_write_register(bus->device, 0u, 0x13u);
    x86_pic_write_register(bus->device, 1u, vector);
    x86_pic_write_register(bus->device, 1u, 1u);
}

/* Architectural fixture reads; 01h selects IMR, 0Ah IRR, 0Bh ISR.
 * Command-register inspection returns to IRR selection. This mutates OCW3,
 * so it is not suitable for live failure diagnostics or OCW3 protocol tests. */
static inline lib_u8 test_pic_read(const core_machine_pic_bus *bus, lib_u8 selector)
{
    lib_u8 value = 0u;
    if (selector == 1u) {
        x86_pic_read_register(bus->device, 1u, &value);
    } else {
        x86_pic_write_register(bus->device, 0u, selector);
        x86_pic_read_register(bus->device, 0u, &value);
        x86_pic_write_register(bus->device, 0u, 0x0au);
    }
    return value;
}
#endif
