#ifndef TEST_X86_NXVM_EMULATOR_KBC_IRQ_FIXTURE_H
#define TEST_X86_NXVM_EMULATOR_KBC_IRQ_FIXTURE_H
#include "x86-nxvm/board-base/pic_bus_interface.h"

typedef struct test_kbc_irq_wiring {
    x86_nxvm_board_pic_irq_source *keyboard;
    x86_nxvm_board_pic_irq_source *auxiliary;
} test_kbc_irq_wiring;

static void test_kbc_irq_output(void *context, lib_bool auxiliary, lib_bool asserted)
{
    test_kbc_irq_wiring *wiring = context;
    x86_nxvm_board_pic_irq_source *source = auxiliary ? wiring->auxiliary : wiring->keyboard;
    if (asserted) x86_nxvm_board_pic_irq_source_assert(source);
    else x86_nxvm_board_pic_irq_source_deassert(source);
}

static lib_status test_kbc_bind_irq(test_kbc_irq_wiring *wiring,
    x86_nxvm_board_pic_bus *master, x86_nxvm_board_pic_bus *slave)
{
    lib_status status = x86_nxvm_board_pic_irq_source_bind(&wiring->keyboard, master, slave, 1u);
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_board_pic_irq_source_bind(&wiring->auxiliary, master, slave, 12u);
    return status;
}
#endif
