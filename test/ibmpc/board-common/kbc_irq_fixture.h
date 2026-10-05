#ifndef TEST_IBMPC_COMMON_KBC_IRQ_FIXTURE_H
#define TEST_IBMPC_COMMON_KBC_IRQ_FIXTURE_H
#include "ibmpc/board-common/pic_bus_interface.h"

typedef struct test_kbc_irq_wiring {
    core_machine_pic_irq_source *keyboard;
    core_machine_pic_irq_source *auxiliary;
} test_kbc_irq_wiring;

static void test_kbc_irq_output(void *context, lib_bool auxiliary, lib_bool asserted)
{
    test_kbc_irq_wiring *wiring = context;
    core_machine_pic_irq_source *source = auxiliary ? wiring->auxiliary : wiring->keyboard;
    if (asserted) core_machine_pic_irq_source_assert(source);
    else core_machine_pic_irq_source_deassert(source);
}

static lib_status test_kbc_bind_irq(test_kbc_irq_wiring *wiring,
    core_machine_pic_bus *master, core_machine_pic_bus *slave)
{
    lib_status status = core_machine_pic_irq_source_bind(&wiring->keyboard, master, slave, 1u);
    if (status == LIB_STATUS_OK)
        status = core_machine_pic_irq_source_bind(&wiring->auxiliary, master, slave, 12u);
    return status;
}
#endif
