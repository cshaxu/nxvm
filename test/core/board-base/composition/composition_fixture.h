#ifndef TEST_CORE_COMPOSITION_FIXTURE_H
#define TEST_CORE_COMPOSITION_FIXTURE_H
#include "core/x86/machine_interface.h"
#include "core/x86/firmware_interface.h"
#include "core/chips/cpu/cpu_interface.h"

typedef struct test_core_preview_publication {
    lib_u64 committed;
    lib_u64 cancelled;
    lib_size trace_count;
} test_core_preview_publication;

test_core_preview_publication test_core_capture_preview_publication(const core_machine *machine);
lib_bool test_core_preview_lexeme(core_machine *machine,
    core_machine_cpu_instruction_lexeme *lexeme);
lib_bool test_core_fpu_command_handoff(const core_machine *machine,
    lib_u8 opcode, lib_u8 modrm, lib_u64 *remaining_ticks);

/* Bare registration owner for CPU/PIC fixtures, not a second CPU executor. */
core_machine *test_core_port_owner_create(void);
void test_core_port_owner_destroy(core_machine *machine);

/* Caller owns the paused execution boundary during deliberate fault injection. */
void test_core_set_lifecycle(core_machine *machine, core_machine_lifecycle state);
void test_core_set_firmware_operation_active(core_machine *machine, lib_bool active);
void test_core_replace_firmware_provider(core_machine *machine,
    const core_machine_firmware_provider *provider);

lib_bool test_core_port_has_read(const core_machine *machine, lib_u16 port);
lib_bool test_core_port_has_write(const core_machine *machine, lib_u16 port);
void test_core_write_port_after_run(core_machine *machine, lib_u16 port, lib_u32 value);
lib_bool test_core_transaction_timing_is_disabled(const core_machine *machine);
lib_bool test_core_a20_is_enabled(const core_machine *machine);
lib_u64 test_core_elapsed_ticks(const core_machine *machine);
lib_bool test_core_cpu_is_halted(const core_machine *machine);
void test_core_request_cpu_shutdown(core_machine *machine);
lib_bool test_core_has_parity_storage(const core_machine *machine);
void test_core_flip_parity(core_machine *machine, lib_u32 physical);
lib_status test_core_read_physical(core_machine *machine, lib_u32 address,
    lib_uptr destination, lib_uptr bytes);
lib_status test_core_write_physical(core_machine *machine, lib_u32 address,
    lib_uptr source, lib_uptr bytes);
void test_core_invoke_parity_fault(core_machine *machine, lib_u32 physical);
lib_bool test_core_dma_wait_remaining_matches(const core_machine *machine,
    lib_u32 expected);
lib_bool test_core_refresh_request(core_machine *machine, lib_u8 *address);
lib_bool test_core_dma_hold_begin(core_machine *machine);
lib_bool test_core_dma_hold_excludes_cpu(core_machine *machine);
lib_bool test_core_dma_timing_matches(const core_machine *machine,
    lib_u32 wait_quanta, lib_bool gate_enabled, lib_bool ready);
core_machine_transaction_contract test_core_capture_transaction_contract(const core_machine *machine);
core_machine_retirement_time_contract test_core_retirement_contract(const core_machine *machine);
lib_bool test_core_has_rom_mapping_start(const core_machine *machine, lib_u32 physical_start);
lib_u32 test_core_read_port_after_run(core_machine *machine, lib_u16 port);
lib_bool test_core_instances_are_distinct(const core_machine *first,
    const core_machine *second);
#endif
