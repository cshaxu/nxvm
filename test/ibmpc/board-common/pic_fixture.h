#ifndef TEST_PIC_FIXTURE_H
#define TEST_PIC_FIXTURE_H
#include "ibmpc/board-common/pic_bus_interface.h"

#define VPIC_IRR_IRQ(id) (1u << (id))
#define VPIC_ISR_IRQ(id) (1u << (id))
#define VPIC_OCW1_IMR(id) (1u << (id))
#define VPIC_ICW1_SNGL 0x02u
#define VPIC_ICW1_IC4 0x01u
#define VPIC_ICW3_S(id) (1u << (id))
#define VPIC_POLL_I 0x80u

static inline lib_bool test_pic_bit_is_set(lib_u32 value, lib_u32 mask)
{
    return (value & mask) != 0u;
}

static inline lib_u32 test_pic_port_read(core_machine *machine, lib_u16 port)
{
    lib_u32 value;
    if (core_machine_bus_read(machine, port, &value) != LIB_STATUS_OK)
        exit(EXIT_FAILURE);
    return value;
}

static inline void test_pic_port_write(core_machine *machine, lib_u16 port,
    lib_u32 value)
{
    if (core_machine_bus_write(machine, port, value) != LIB_STATUS_OK)
        exit(EXIT_FAILURE);
}

static inline void test_pic_bind_source(core_machine_pic_irq_source **out_source,
    core_machine_pic_bus *master, core_machine_pic_bus *slave, lib_u8 irq)
{
    if (core_machine_pic_irq_source_bind(out_source, master, slave, irq) != LIB_STATUS_OK)
        exit(EXIT_FAILURE);
}

/* CPU fixtures program a single-controller vector instead of altering ICW2
 * behind the chip's back. This is setup, before any request is raised. */
static inline void test_pic_program_vector(core_machine_pic_bus *bus, lib_u8 vector)
{
    if (core_machine_pic_write_register(bus, 0u, 0x13u) != LIB_STATUS_OK ||
        core_machine_pic_write_register(bus, 1u, vector) != LIB_STATUS_OK ||
        core_machine_pic_write_register(bus, 1u, 1u) != LIB_STATUS_OK)
        exit(EXIT_FAILURE);
}

/* Copied register observation; guest poll reads still use the Core bus. */
static inline lib_u8 test_pic_read(core_machine_pic_bus *bus, lib_u8 selector)
{
    x86_pic_register_state state = {0};
    if (core_machine_pic_capture_registers(bus, &state) != LIB_STATUS_OK)
        exit(EXIT_FAILURE);
    return selector == 1u ? state.imr : (selector == 0x0bu ? state.isr : state.irr);
}

/* Verify the route by its resolved line, not an attachment's private irq field. */
static inline lib_bool test_pic_source_route(core_machine_pic_bus *master,
    core_machine_pic_bus *slave, core_machine_pic_irq_source *source, lib_u8 irq)
{
    x86_pic_register_state state = {0};
    core_machine_pic_irq_source_deassert(source);
    core_machine_pic_reset(master, slave);
    test_pic_program_vector(master, 8u);
    test_pic_program_vector(slave, 0x70u);
    core_machine_pic_irq_source_assert(source);
    if (core_machine_pic_capture_registers(irq < 8u ? master : slave,
            &state) != LIB_STATUS_OK) exit(EXIT_FAILURE);
    core_machine_pic_irq_source_deassert(source);
    core_machine_pic_reset(master, slave);
    return source != LIB_NULL && state.irr == (lib_u8)(1u << (irq & 7u));
}
#endif
