/* Moved from NXVM's command/priority corpus; private checks stay chip-local. */
#include "x86/chips/pic8259/pic.h"

static x86_pic *initialize(lib_u8 mode)
{
    x86_pic *pic = LIB_NULL;
    if (x86_pic_create(LIB_TRUE, &pic) != LIB_STATUS_OK) return LIB_NULL;
    x86_pic_write_register(pic, 0u, 0x11u);
    x86_pic_write_register(pic, 1u, 0x08u);
    x86_pic_write_register(pic, 1u, 0x04u);
    x86_pic_write_register(pic, 1u, mode);
    return pic;
}
static void raise(x86_pic *pic, lib_u8 line)
{
    x86_pic_set_inputs(pic, 0u, (lib_u8)(1u << line), 0u);
}
static lib_bool pending(x86_pic *pic)
{
    x86_pic_request request;
    return x86_pic_select(pic, &request) == LIB_STATUS_OK;
}
static lib_u8 peek(x86_pic *pic)
{
    x86_pic_request request;
    return x86_pic_select(pic, &request) == LIB_STATUS_OK ? request.vector : 0u;
}
static lib_u8 read_command(x86_pic *pic)
{
    lib_u8 value = 0u;
    x86_pic_read_register(pic, 0u, &value);
    return value;
}

static lib_i32 pic_command_priority_test_initialization_and_registers(void)
{
    x86_pic *pic = LIB_NULL;
    lib_i32 failed = 0;

    pic = initialize(0x01u);
    raise(pic, 5u);
    x86_pic_write_register(pic, 0u, 0x0au);
    failed |= read_command(pic) != VPIC_IRR_IRQ(5u);
    failed |= !pending(pic);
    failed |= x86_pic_acknowledge(pic).vector != 0x0du;
    x86_pic_write_register(pic, 0u, 0x0bu);
    failed |= read_command(pic) != VPIC_ISR_IRQ(5u);
    x86_pic_write_register(pic, 1u, VPIC_OCW1_IMR(1u));
    x86_pic_write_register(pic, 0u, 0x11u);
    failed |= pic->data.irr != 0u || pic->data.imr != 0u ||
        pic->data.isr != 0u || pic->data.irx != 0u ||
        pic->data.icw2 != 0u || pic->data.icw3 != 0u ||
        pic->data.icw4 != 0u || pic->data.imr != 0u ||
        pic->data.ocw2 != 0u ||
        pic->data.ocw3 != VPIC_OCW3_RR ||
        pic->data.status != ICW2;
    failed |= x86_pic_acknowledge(pic).vector != 0u ||
        pic->data.isr != 0u;
    x86_pic_destroy(pic);

    pic = initialize(0x01u);
    failed |= pending(pic) ||
        peek(pic) != 0u ||
        x86_pic_acknowledge(pic).vector != 0x0fu ||
        pic->data.irr != 0u || pic->data.isr != 0u;
    x86_pic_destroy(pic);
    return failed;
}

static lib_i32 pic_command_priority_test_eoi_and_rotation(void)
{
    x86_pic *pic = LIB_NULL;
    lib_i32 failed = 0;

    pic = initialize(0x01u);
    raise(pic, 3u);
    raise(pic, 5u);
    failed |= x86_pic_acknowledge(pic).vector != 0x0bu;
    failed |= pending(pic);
    x86_pic_write_register(pic, 0u, 0x63u);
    failed |= pic->data.isr != 0u;
    failed |= x86_pic_acknowledge(pic).vector != 0x0du;
    x86_pic_write_register(pic, 0u, 0x20u);

    x86_pic_destroy(pic);
    pic = initialize(0x01u);
    raise(pic, 0u);
    raise(pic, 3u);
    failed |= x86_pic_acknowledge(pic).vector != 0x08u;
    x86_pic_write_register(pic, 0u, 0xa0u);
    failed |= pic->data.irx != 1u || pic->data.isr != 0u;
    raise(pic, 0u);
    failed |= x86_pic_acknowledge(pic).vector != 0x0bu;
    x86_pic_write_register(pic, 0u, 0x20u);
    failed |= x86_pic_acknowledge(pic).vector != 0x08u;
    x86_pic_write_register(pic, 0u, 0x20u);
    x86_pic_destroy(pic);

    pic = initialize(0x01u);
    raise(pic, 5u);
    failed |= x86_pic_acknowledge(pic).vector != 0x0du;
    raise(pic, 3u);
    failed |= x86_pic_acknowledge(pic).vector != 0x0bu;
    x86_pic_write_register(pic, 0u, 0xe5u);
    failed |= !PIC_BIT_IS_SET(pic->data.isr, VPIC_ISR_IRQ(3u)) ||
        PIC_BIT_IS_SET(pic->data.isr, VPIC_ISR_IRQ(5u)) ||
        pic->data.irx != 6u;
    x86_pic_write_register(pic, 0u, 0x63u);
    x86_pic_destroy(pic);
    return failed;
}

static lib_i32 pic_command_priority_test_aeoi(void)
{
    x86_pic *pic = LIB_NULL;
    lib_i32 failed = 0;

    pic = initialize(0x03u);
    raise(pic, 1u);
    failed |= x86_pic_acknowledge(pic).vector != 0x09u ||
        pic->data.isr != 0u;
    raise(pic, 4u);
    failed |= x86_pic_acknowledge(pic).vector != 0x0cu ||
        pic->data.isr != 0u;
    x86_pic_destroy(pic);
    return failed;
}

static lib_i32 pic_ocw3_bits(void)
{
    x86_pic *pic = initialize(0x01u);
    lib_i32 failed = 0;
    if (pic == LIB_NULL) return 1;
    raise(pic, 4u);
    x86_pic_write_register(pic, 0u, 0x0cu);
    failed |= read_command(pic) != 0x84u ||
        PIC_BIT_IS_SET(pic->data.ocw3, VPIC_OCW3_P);
    x86_pic_write_register(pic, 0u, 0x68u);
    failed |= !PIC_BIT_IS_SET(pic->data.ocw3, VPIC_OCW3_SMM);
    x86_pic_write_register(pic, 0u, 0x0bu);
    failed |= !PIC_BIT_IS_SET(pic->data.ocw3, VPIC_OCW3_SMM);
    x86_pic_write_register(pic, 0u, 0x48u);
    failed |= PIC_BIT_IS_SET(pic->data.ocw3, VPIC_OCW3_SMM);
    x86_pic_destroy(pic);
    return failed;
}

int main(void)
{
    return pic_command_priority_test_initialization_and_registers() |
        pic_command_priority_test_eoi_and_rotation() |
        pic_command_priority_test_aeoi() | pic_ocw3_bits();
}
