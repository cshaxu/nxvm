#include "boot_fixture.h"
#include "ibmpc/board-xt/xt_ppi_keyboard.h"

lib_status test_xt_boot_ppi_output(const core_machine_xt_ppi_keyboard *keyboard,
    lib_u8 selector, x86_ppi8255_pins *pins)
{
    return x86_ppi8255_output(keyboard->ppi, selector, pins);
}

lib_bool test_xt_boot_byte_ready(const core_machine_xt_ppi_keyboard *keyboard)
{
    return keyboard->byte_ready;
}
