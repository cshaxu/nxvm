#include "time_fixture.h"
#include "core/x86/machine.h"

lib_status test_core_machine_advance_time(core_machine *machine, lib_u64 ticks)
{
    return core_machine_advance_time(machine, ticks);
}

static void test_core_time_counter(void *context, lib_u64 due_tick)
{
    lib_u32 *count = (lib_u32 *)context;

    (void)due_tick;
    if (count != LIB_NULL) ++*count;
}

lib_status test_core_schedule_counter(core_machine *machine, lib_u64 due_tick,
    lib_u32 *count)
{
    core_machine_timeline_token token;

    return core_machine_timeline_schedule(&machine->timeline, due_tick,
        test_core_time_counter, count, &token);
}
