#include "type.h"
#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_executor_fixture.h"
typedef struct { core_machine_trace_event event; C_INT count; } fixture;
static C_VOID capture(C_VOID *context, const core_machine_trace_event *event)
{ fixture *state = (fixture *)context; if (event->type == CORE_MACHINE_TRACE_EXTERNAL_TIME) { state->event = *event; ++state->count; } }
C_INT main(C_VOID)
{ core_machine *machine = STD_NULL; core_machine_trace_provider trace; fixture state = { { 0 }, 0 }; C_INT failed = 0;
  trace.callback = capture; trace.context = &state;
  failed |= test_core_machine_create_executor(0u, &machine) != TYPE_STATUS_OK || core_machine_set_trace_provider(machine, &trace) != TYPE_STATUS_OK || core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK || core_machine_reset(machine) != TYPE_STATUS_OK || core_machine_advance_time(machine, 37u) != TYPE_STATUS_OK || state.count != 1 || state.event.value != 37u || state.event.elapsed_ticks != 37u;
  core_machine_destroy(machine); if (failed) return 1; STD_PRINTF("M5:T375:S18:EXTERNAL-TIME-TRACE:OK\n"); return 0; }
