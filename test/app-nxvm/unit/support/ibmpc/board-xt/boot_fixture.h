#ifndef TEST_XT_BOOT_FIXTURE_H
#define TEST_XT_BOOT_FIXTURE_H
#include "core/board-xt/xt_ppi_keyboard_interface.h"
#include "core/chips/ppi8255/ppi8255_interface.h"

lib_status test_xt_boot_ppi_output(const core_machine_xt_ppi_keyboard *keyboard,
    lib_u8 selector, x86_ppi8255_pins *pins);
lib_bool test_xt_boot_byte_ready(const core_machine_xt_ppi_keyboard *keyboard);
#endif
