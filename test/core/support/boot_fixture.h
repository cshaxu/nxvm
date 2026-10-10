#ifndef TEST_CORE_BOOT_FIXTURE_H
#define TEST_CORE_BOOT_FIXTURE_H
#include "core/x86/machine_interface.h"
#include "core/x86/memory_interface.h"

typedef struct test_core_boot_observation {
    lib_size memory_bytes;
    lib_bool a20;
    core_machine_cpu_profile cpu_profile;
    lib_u64 elapsed_ticks;
    lib_size rom_mapping_count;
    lib_u8 transaction_owner;
    lib_u8 hold_owner;
    lib_bool hold_acknowledged;
} test_core_boot_observation;

typedef struct test_core_boot_rom_mapping {
    lib_u32 physical_start;
    lib_size bytes;
} test_core_boot_rom_mapping;

/* Synchronous diagnostics on the executor thread, including in a write
 * observer. No execution, ownership transfer or paused-debug admission. */
test_core_boot_observation test_core_boot_capture(const core_machine *machine);
test_core_boot_rom_mapping test_core_boot_rom_at(const core_machine *machine,
    lib_size index);
void test_core_boot_capture_write_cpu(const core_machine *machine,
    core_machine_cpu_state *cpu);
void test_core_boot_capture_diagnostic(const core_machine *machine,
    core_machine_cpu_diagnostic *diagnostic);
lib_status test_core_boot_read_reset(core_machine *machine, lib_u32 address,
    lib_uptr destination, lib_uptr bytes);
lib_status test_core_boot_query(const core_machine *machine, lib_u32 address,
    lib_uptr bytes, core_machine_memory_access access,
    core_machine_memory_route *route);
lib_bool test_core_boot_bind_write_observer(core_machine *machine,
    core_machine_memory_write_observer observer, void *context);
#endif
