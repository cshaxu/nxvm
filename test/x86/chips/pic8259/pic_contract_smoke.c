#include "x86/chips/pic8259/pic8259_interface.h"

static void program(x86_pic *pic, lib_u8 vector, lib_u8 cascade, lib_u8 mode)
{
    x86_pic_write_register(pic, 0u, 0x11u);
    x86_pic_write_register(pic, 1u, vector);
    x86_pic_write_register(pic, 1u, cascade);
    x86_pic_write_register(pic, 1u, mode);
}

static lib_u8 read(x86_pic *pic, lib_u8 command)
{
    lib_u8 value = 0xffu;
    x86_pic_write_register(pic, 0u, command);
    x86_pic_read_register(pic, 0u, &value);
    return value;
}

static lib_bool single_chip(void)
{
    x86_pic *pic = LIB_NULL;
    x86_pic_request request;
    lib_u32 timing[8] = { 0u, 3u };
    lib_u64 ticks = 99u;
    lib_bool failed = LIB_FALSE;
    if (x86_pic_create(LIB_TRUE, &pic) != LIB_STATUS_OK) return LIB_TRUE;
    program(pic, 0x20u, 0u, 1u);
    x86_pic_set_inputs(pic, 0u, 0x22u, 0u);
    x86_pic_register_state registers = {0u, 0xa5u, 0u};
    failed |= x86_pic_capture_registers(LIB_NULL, &registers) != LIB_STATUS_INVALID_ARGUMENT || registers.imr != 0xa5u;
    failed |= x86_pic_capture_registers(pic, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= x86_pic_capture_registers(pic, &registers) != LIB_STATUS_OK ||
        registers.irr != 0x22u || registers.imr != 0u || registers.isr != 0u;
    registers.irr = 0u; /* Mutating the copied snapshot cannot mask chip requests. */
    failed |= x86_pic_select(pic, &request) != LIB_STATUS_OK;
    failed |= request.vector != 0x21u || request.cascade;
    request = x86_pic_acknowledge(pic);
    failed |= request.vector != 0x21u || read(pic, 0x0bu) != 2u;
    failed |= x86_pic_select(pic, &request) != LIB_STATUS_INVALID_STATE;
    x86_pic_write_register(pic, 0u, 0x20u);
    failed |= read(pic, 0x0cu) != 0x85u;
    failed |= read(pic, 0x0bu) != 0x20u;
    x86_pic_write_register(pic, 0u, 0x20u);
    failed |= x86_pic_acknowledge(pic).vector != 0x27u;

    x86_pic_set_irq_timing(pic, timing);
    x86_pic_write_register(pic, 1u, 2u);
    x86_pic_set_inputs(pic, 0u, 2u, 0u);
    x86_pic_write_register(pic, 1u, 0u);
    failed |= x86_pic_select(pic, &request) != LIB_STATUS_INVALID_STATE;
    failed |= x86_pic_ticks_until_event(pic, &ticks) != LIB_STATUS_OK || ticks != 3u;
    x86_pic_advance(pic, 2u);
    failed |= x86_pic_select(pic, &request) != LIB_STATUS_INVALID_STATE;
    x86_pic_advance(pic, 1u);
    failed |= x86_pic_select(pic, &request) != LIB_STATUS_OK;
    failed |= request.vector != 0x21u;
    x86_pic_reset(pic);
    failed |= x86_pic_acknowledge(pic).vector != 0u;
    failed |= x86_pic_ticks_until_event(pic, &ticks) != LIB_STATUS_INVALID_STATE;
    x86_pic_destroy(pic);
    return failed;
}

static lib_bool cascade(void)
{
    x86_pic *master = LIB_NULL;
    x86_pic *slave = LIB_NULL;
    x86_pic_request request;
    lib_u8 address = 0xffu;
    lib_bool failed = LIB_FALSE;
    if (x86_pic_create(LIB_TRUE, &master) != LIB_STATUS_OK) return LIB_TRUE;
    if (x86_pic_create(LIB_FALSE, &slave) != LIB_STATUS_OK) {
        x86_pic_destroy(master);
        return LIB_TRUE;
    }
    program(master, 8u, 4u, 0x11u);
    program(slave, 0x70u, 2u, 1u);
    failed |= !x86_pic_cascade_address(slave, &address) || address != 2u;
    /* Slave ICW3 is an identity, not the master's bitmask of attached chips. */
    x86_pic_set_inputs(slave, 0u, 2u, 0u);
    failed |= x86_pic_select(slave, &request) != LIB_STATUS_OK;
    failed |= request.vector != 0x71u || request.cascade;
    x86_pic_set_inputs(master, 0u, 0u, 4u);
    request = x86_pic_acknowledge(master);
    failed |= !request.cascade || request.line != 2u;
    failed |= x86_pic_acknowledge(slave).vector != 0x71u;
    x86_pic_set_inputs(slave, 0u, 1u, 0u);
    x86_pic_set_inputs(master, 0u, 0u, 4u);
    /* SFNM permits a higher-priority slave request through its in-service line. */
    failed |= x86_pic_select(master, &request) != LIB_STATUS_OK || !request.cascade;
    /* Poll remains local and does not apply that pair-level SFNM override. */
    failed |= read(master, 0x0cu) != 0u;
    x86_pic_set_inputs(master, 0u, 0u, 0u);
    failed |= x86_pic_select(master, &request) != LIB_STATUS_INVALID_STATE;
    x86_pic_destroy(slave);
    x86_pic_destroy(master);
    return failed;
}

int main(void)
{
    return single_chip() | cascade();
}
