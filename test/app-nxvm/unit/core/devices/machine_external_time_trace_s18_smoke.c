#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_executor_fixture.h"
typedef struct { core_machine_trace_event event; lib_i32 count; } fixture;
static void capture(void *context, const core_machine_trace_event *event)
{ fixture *state = (fixture *)context; if (event->type == CORE_MACHINE_TRACE_EXTERNAL_TIME) { state->event = *event; ++state->count; } }
lib_i32 main(void)
{ core_machine *machine = LIB_NULL; core_machine_trace_provider trace; fixture state = { { 0 }, 0 }; lib_i32 failed = 0;
  trace.callback = capture; trace.context = &state;
  if (test_core_machine_create_executor(0u, &machine) != LIB_STATUS_OK) return 1;
  machine->retirement_time_contract = CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC;
  failed |= core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK || core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK || core_machine_reset(machine) != LIB_STATUS_OK || core_machine_advance_time(machine, 37u) != LIB_STATUS_OK || state.count != 1 || state.event.value != 37u || state.event.elapsed_ticks != 0u;
  core_machine_destroy(machine); if (failed) return 1; printf("M5:T375:S18:EXTERNAL-TIME-TRACE:OK\n"); return 0; }
