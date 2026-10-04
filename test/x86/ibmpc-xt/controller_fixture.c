#include "controller_fixture.h"
#include "x86/ibmpc-xt/xt_ppi_keyboard.h"

lib_status test_xt_ppi_initialize(core_machine_xt_ppi_keyboard *keyboard,
    const core_machine_xt_ppi_keyboard_config *config, core_machine *machine)
{
    return core_machine_xt_ppi_keyboard_initialize(keyboard, config, machine);
}

void test_xt_ppi_finalize(core_machine_xt_ppi_keyboard *keyboard)
{
    core_machine_xt_ppi_keyboard_finalize(keyboard);
}

lib_bool test_xt_ppi_unpublished(const core_machine_xt_ppi_keyboard *keyboard)
{
    return keyboard->ppi == LIB_NULL;
}

lib_bool test_xt_ppi_expect_byte_ready(const core_machine_xt_ppi_keyboard *keyboard,
    lib_bool expected)
{
    return keyboard->byte_ready != expected;
}

lib_bool test_xt_ppi_expect_irq1(const core_machine_xt_ppi_keyboard *keyboard,
    lib_bool expected)
{
    return keyboard->irq1_asserted != expected;
}

lib_bool test_xt_ppi_expect_nmi(const core_machine_xt_ppi_keyboard *keyboard,
    lib_bool expected)
{
    return keyboard->nmi_signaled != expected;
}

lib_bool xt_construction_rollback(void)
{
    const core_machine_xt_ppi_keyboard_config config = {
        0x60u, 0x61u, 0x62u, 0x63u, 1u, 0u, 0u};
    lib_bool failed = LIB_FALSE;
    for (lib_size fail_at = 1u; fail_at <= 8u; ++fail_at) {
        core_machine_xt_ppi_keyboard keyboard = {0};
        failed |= test_xt_ppi_core_construction(&keyboard, &config, fail_at);
    }
    return failed;
}
