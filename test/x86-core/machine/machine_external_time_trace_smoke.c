#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "x86-core/machine/machine.h"
#include "x86-core/machine/machine_interface.h"
#include "executor_fixture.h"
typedef struct { x86_core_machine_trace_event event; lib_i32 count; } fixture;
static void capture(void *context, const x86_core_machine_trace_event *event)
{ fixture *state = (fixture *)context; if (event->type == X86_CORE_MACHINE_TRACE_EXTERNAL_TIME) { state->event = *event; ++state->count; } }
lib_i32 main(void)
{ x86_core_machine *machine = LIB_NULL; x86_core_machine_trace_provider trace; fixture state = { { 0 }, 0 }; lib_i32 failed = 0;
  trace.callback = capture; trace.context = &state;
  if (test_core_machine_create_executor(16u * 1024u * 1024u, &machine) != LIB_STATUS_OK) return 1;
  machine->retirement_time_contract = X86_CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC;
  failed |= x86_core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK || x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK || x86_core_machine_reset(machine) != LIB_STATUS_OK || x86_core_machine_advance_time(machine, 37u) != LIB_STATUS_OK || state.count != 1 || state.event.value != 37u || state.event.elapsed_ticks != 0u;
  x86_core_machine_destroy(machine); if (failed) return 1; lib_c_printf("EXTERNAL-TIME-TRACE:OK\n"); return 0; }
