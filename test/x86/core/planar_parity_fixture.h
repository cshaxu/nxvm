#ifndef TEST_PLANAR_PARITY_FIXTURE_H
#define TEST_PLANAR_PARITY_FIXTURE_H
#include "x86/ibmpc-common/machine_board_interface.h"

/* Core-owned parity storage/fault assertions; expected is an identity token
 * borrowed for this comparison, not a dereferenceable AT owner. */
lib_i32 test_planar_parity_memory_fault(core_machine *machine);
lib_i32 test_planar_parity_memory_binding(core_machine *machine,
    const void *expected);
lib_i32 test_planar_parity_unbound_reconfigure(void);
#endif
