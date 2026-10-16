#ifndef TEST_X86_NXVM_BOARD_AT_PARITY_REGISTRATION_FIXTURE_H
#define TEST_X86_NXVM_BOARD_AT_PARITY_REGISTRATION_FIXTURE_H
#include "lib/types/types_interface.h"

void test_at_parity_reject_ports(lib_bool reject);
lib_bool test_at_parity_rejection_reached(void);
#endif
