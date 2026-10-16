#ifndef TEST_X86_NXVM_EMULATOR_BOARD_BINDING_FIXTURE_H
#define TEST_X86_NXVM_EMULATOR_BOARD_BINDING_FIXTURE_H
#include "x86-core/machine/attachment_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"

/* Observe actual Board-produced public bind values, not Machine storage.
 * The copied context is borrowed for identity checks over that lifetime. */
lib_status test_board_binding_create(const x86_nxvm_board_config *config,
    x86_core_machine **out_machine, x86_core_machine_attachment *out_expected);

/* Reject the actual neutral-constructor call; validation must precede it. */
lib_status test_board_constructor_failure(const x86_nxvm_board_config *config,
    x86_core_machine **out_machine, x86_nxvm_board_state **out_board,
    lib_size *out_attempts);
/* Dependencies: DMA, PIC, PIT, video, KBC, XT PPI, auxiliary PIT, A20 ports.
 * Each rejects the real Board call and verifies destruction exactly once. */
lib_i32 test_board_device_failure(const x86_nxvm_board_config *config,
    lib_u32 dependency);
void test_board_reject_ports(lib_bool reject);
lib_bool test_board_ports_rejected(lib_size count);
lib_size test_board_submitted_port_count(void);
/* Synchronous observation of the real neutral-constructor input. Clear both
 * borrowed outputs before their local storage leaves scope. */
void test_board_observe_constructor(x86_core_machine_executor_config *config,
    lib_size *count);
#endif
