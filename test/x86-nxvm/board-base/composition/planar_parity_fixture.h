#ifndef TEST_PLANAR_PARITY_FIXTURE_H
#define TEST_PLANAR_PARITY_FIXTURE_H
#include "x86-nxvm/board-base/machine_board_interface.h"

/* Board wiring uses public fault and exclusive-lease operations. Storage
 * corruption and allocator identities stay with the Machine owner tests. */
lib_i32 test_planar_parity_memory_fault(x86_core_machine *machine,
    x86_nxvm_board_state *board);
lib_i32 test_planar_parity_memory_binding(x86_core_machine *machine,
    const void *expected);
lib_i32 test_planar_parity_unbound_reconfigure(void);
#endif
