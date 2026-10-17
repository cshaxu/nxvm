#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "x86-core/machine/machine.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/debug_interface.h"
#include "executor_fixture.h"
typedef struct { x86_core_machine_trace_event event, retirement; lib_i32 count, retired; } fixture;
static void capture(void *context, const x86_core_machine_trace_event *event)
{ fixture *state = (fixture *)context; if (event->type == X86_CORE_MACHINE_TRACE_EXTERNAL_TIME) { state->event = *event; ++state->count; }
  if (event->type == X86_CORE_MACHINE_TRACE_CPU_RETIRE) { state->retirement = *event; ++state->retired; } }

static lib_bool bounded_retirement_trace(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 16u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80486SX
    };
    const lib_u8 code[] = {0x90u, 0x90u};
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EFLAGS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CR0),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x100u,
            [X86_CORE_CPU_DEBUG_EFLAGS] = 2u, [X86_CORE_CPU_DEBUG_CR0] = 0x60000010u}
    };
    fixture state = {0};
    const x86_core_machine_trace_provider trace = {capture, &state};
    x86_core_machine *machine = LIB_NULL;
    lib_bool failed = x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;

    if (!failed) failed = x86_core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK;
    if (!failed) {
        failed = x86_core_machine_debug_patch_registers(machine, &entry) != LIB_STATUS_OK ||
            x86_core_machine_memory_write(machine, 0x100u, code, sizeof(code)) != LIB_STATUS_OK;
    }
    for (lib_u8 tick = 1u; !failed && tick <= 5u; ++tick) {
        x86_core_machine_run_result result;

        failed = x86_core_machine_run(machine, (x86_core_machine_run_budget){1u, 1u},
                &result) != LIB_STATUS_OK || result.ticks != 1u || result.elapsed_ticks != tick ||
            result.executed != (tick == 5u ? 1u : 0u) ||
            state.retired != (tick == 5u ? 1 : 0) || state.count != (tick < 5u ? tick : 4);
    }
    if (!failed) failed = state.retirement.value != 5u ||
        state.retirement.elapsed_ticks != 4u || state.event.elapsed_ticks != 3u ||
        state.event.value != 1u || machine->cpu_retirement_wait_pending;
    x86_core_machine_destroy(machine);
    return !failed;
}

lib_i32 main(void)
{ x86_core_machine *machine = LIB_NULL; x86_core_machine_trace_provider trace; fixture state = {0}; lib_i32 failed = 0;
  trace.callback = capture; trace.context = &state;
  if (test_x86_core_machine_create_executor(16u * 1024u * 1024u, &machine) != LIB_STATUS_OK) return 1;
  machine->retirement_time_contract = X86_CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC;
  failed |= x86_core_machine_set_trace_provider(machine, &trace) != LIB_STATUS_OK || x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK || x86_core_machine_reset(machine) != LIB_STATUS_OK || x86_core_machine_advance_time(machine, 37u) != LIB_STATUS_OK || state.count != 1 || state.event.value != 37u || state.event.elapsed_ticks != 0u;
  x86_core_machine_destroy(machine); if (failed || !bounded_retirement_trace()) return 1; lib_c_printf("EXTERNAL-TIME-TRACE:OK\n"); return 0; }
