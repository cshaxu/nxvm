#ifndef TEST_X86_NXVM_BOARD_BASE_KBC_CONTROLLER_FIXTURE_H
#define TEST_X86_NXVM_BOARD_BASE_KBC_CONTROLLER_FIXTURE_H
#include "x86-core/machine/machine_interface.h"

void test_kbc_initialize_pic(x86_core_machine *machine);
lib_i32 test_kbc_board_cpu_reset_irq1(void);
#endif
