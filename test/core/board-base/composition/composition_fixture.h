#ifndef TEST_CORE_COMPOSITION_FIXTURE_H
#define TEST_CORE_COMPOSITION_FIXTURE_H
#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/firmware_interface.h"
#include "x86-core/cpu/cpu_interface.h"

/* Public opaque registration lifetime for CPU/PIC wiring assays. */
x86_core_machine *test_core_port_owner_create(void);
void test_core_port_owner_destroy(x86_core_machine *machine);

/* Operational stopped-bus probes: reads consume data and writes submit zero.
 * Callers must reset before relying on the devices' initial state again. */
lib_bool test_core_port_accepts_read(x86_core_machine *machine, lib_u16 port);
lib_bool test_core_port_accepts_write(x86_core_machine *machine, lib_u16 port);
void test_core_write_port_after_run(x86_core_machine *machine, lib_u16 port, lib_u32 value);
lib_bool test_core_a20_is_enabled(const x86_core_machine *machine);
lib_u64 test_core_elapsed_ticks(const x86_core_machine *machine);
lib_bool test_core_cpu_is_halted(const x86_core_machine *machine);
lib_u32 test_core_read_port_after_run(x86_core_machine *machine, lib_u16 port);
#endif
