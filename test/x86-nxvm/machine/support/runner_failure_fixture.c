#include "runner_failure_fixture.h"
#include "x86-nxvm/machine/machine_private.h"

static lib_atomic_uptr rejected_run;
static lib_atomic_uptr rejected_reset;

static lib_bool runner_take_rejection(lib_atomic_uptr *target,
    const x86_core_machine *machine)
{
    lib_uptr expected = (lib_uptr)machine;

    return expected != 0u && lib_atomic_uptr_compare_exchange_strong_explicit(
        target, &expected, 0u, LIB_MEMORY_ORDER_ACQ_REL, LIB_MEMORY_ORDER_ACQUIRE);
}

static lib_status runner_test_run(x86_core_machine *machine,
    x86_core_machine_run_budget budget, x86_core_machine_run_result *result)
{
    if (runner_take_rejection(&rejected_run, machine)) {
        *result = (x86_core_machine_run_result){.reason = X86_CORE_MACHINE_STOP_NONE};
        return LIB_STATUS_INVALID_STATE;
    }
    return x86_core_machine_run(machine, budget, result);
}

static lib_status runner_test_reset(x86_core_machine *machine)
{
    return runner_take_rejection(&rejected_reset, machine) ?
        LIB_STATUS_UNSUPPORTED : x86_core_machine_reset(machine);
}

/* Same-owner fault injection exercises the real adapter control/runner path;
 * it neither changes Core's lifecycle nor replaces its frozen firmware. */
#define x86_core_machine_reset runner_test_reset
#include "x86-nxvm/machine/control.c"
#undef x86_core_machine_reset
#define x86_core_machine_run runner_test_run
#include "x86-nxvm/machine/runner.c"
#undef x86_core_machine_run

static lib_status runner_set_rejection(lib_atomic_uptr *target,
    const x86_nxvm_machine *machine)
{
    lib_uptr expected = lib_atomic_uptr_load_explicit(target, LIB_MEMORY_ORDER_ACQUIRE);
    const lib_uptr desired = machine == LIB_NULL ? 0u :
        (lib_uptr)machine->x86_core_machine;

    return lib_atomic_uptr_compare_exchange_strong_explicit(target, &expected,
        desired, LIB_MEMORY_ORDER_ACQ_REL, LIB_MEMORY_ORDER_ACQUIRE) ?
        LIB_STATUS_OK : LIB_STATUS_INVALID_STATE;
}

lib_status vm_test_runner_reject_run(const x86_nxvm_machine *machine)
{
    return runner_set_rejection(&rejected_run, machine);
}

lib_status vm_test_runner_reject_reset(const x86_nxvm_machine *machine)
{
    return runner_set_rejection(&rejected_reset, machine);
}

lib_bool vm_test_runner_failure_pending(void)
{
    return lib_atomic_uptr_load_explicit(&rejected_run, LIB_MEMORY_ORDER_ACQUIRE) != 0u ||
        lib_atomic_uptr_load_explicit(&rejected_reset, LIB_MEMORY_ORDER_ACQUIRE) != 0u;
}
