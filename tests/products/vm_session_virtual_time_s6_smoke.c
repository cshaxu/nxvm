#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session.h"
#include "vm/composition/session/virtual_time.h"

typedef struct virtual_time_fixture {
    C_INT next_calls;
    C_INT reset_calls;
    type_unsigned_64 next_ticks;
} virtual_time_fixture;

static type_status fixture_next(C_VOID *context,
    type_unsigned_64 *out_source_ticks)
{
    virtual_time_fixture *fixture = (virtual_time_fixture *)context;

    if (fixture == STD_NULL || out_source_ticks == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    ++fixture->next_calls;
    *out_source_ticks = fixture->next_ticks;
    return TYPE_STATUS_OK;
}

static C_VOID fixture_reset(C_VOID *context)
{
    virtual_time_fixture *fixture = (virtual_time_fixture *)context;

    if (fixture != STD_NULL) ++fixture->reset_calls;
}

static C_INT verify_default_source_does_not_advance(
    const core_machine_run_result *waiting)
{
    vm_session *session = STD_NULL;
    type_unsigned_64 before = 0u;
    type_unsigned_64 after = 0u;
    C_INT advanced = 1;
    C_INT failed = 0;

    if (waiting == STD_NULL || vm_session_create(STD_NULL, &session) !=
        TYPE_STATUS_OK || core_machine_get_elapsed_ticks(session->core_machine,
            &before) != TYPE_STATUS_OK) {
        vm_session_destroy(session);
        return 1;
    }
    STD_ATOMIC_STORE(&session->control.flagRun, TYPE_TRUE);
    failed |= vm_session_virtual_time_on_waiting(session, waiting, &advanced) !=
        TYPE_STATUS_OK || advanced;
    failed |= core_machine_get_elapsed_ticks(session->core_machine, &after) !=
        TYPE_STATUS_OK || after != before;
    STD_ATOMIC_STORE(&session->control.flagRun, TYPE_FALSE);
    vm_session_destroy(session);
    return failed;
}

int main(void)
{
    virtual_time_fixture fixture = { 0, 0, 37u };
    const vm_session_virtual_time_source source = {
        fixture_next, fixture_reset, &fixture
    };
    const vm_session_config config = {
        .virtual_time_source = &source
    };
    const core_machine_run_result waiting = {
        .reason = CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT
    };
    const core_machine_run_result non_waiting = {
        .reason = CORE_MACHINE_STOP_BUDGET
    };
    vm_session *session = STD_NULL;
    type_unsigned_64 before = 0u;
    type_unsigned_64 after = 0u;
    C_INT resets_before = 0;
    C_INT advanced = 0;
    C_INT failed = 0;

    if (vm_session_create(&config, &session) != TYPE_STATUS_OK ||
        core_machine_get_elapsed_ticks(session->core_machine, &before) !=
            TYPE_STATUS_OK) {
        vm_session_destroy(session);
        return 1;
    }
    failed |= vm_session_virtual_time_on_waiting(session, &waiting, &advanced) !=
        TYPE_STATUS_INVALID_STATE;
    failed |= fixture.next_calls != 0;
    STD_ATOMIC_STORE(&session->control.flagRun, TYPE_TRUE);
    failed |= vm_session_virtual_time_on_waiting(session, &non_waiting, &advanced) !=
        TYPE_STATUS_INVALID_STATE;
    failed |= fixture.next_calls != 0;
    failed |= vm_session_virtual_time_on_waiting(session, &waiting, &advanced) !=
        TYPE_STATUS_OK || !advanced || fixture.next_calls != 1;
    failed |= core_machine_get_elapsed_ticks(session->core_machine, &after) !=
        TYPE_STATUS_OK || after != before + fixture.next_ticks;
    fixture.next_ticks = 0u;
    advanced = 1;
    failed |= vm_session_virtual_time_on_waiting(session, &waiting, &advanced) !=
        TYPE_STATUS_OK || advanced || fixture.next_calls != 2;
    resets_before = fixture.reset_calls;
    STD_ATOMIC_STORE(&session->control.flagRun, TYPE_FALSE);
    vm_session_reset(session);
    failed |= fixture.reset_calls != resets_before + 1;
    vm_session_destroy(session);
    failed |= verify_default_source_does_not_advance(&waiting);
    if (failed) return 1;
    puts("M5:T375:S6:VIRTUAL-TIME-SOURCE:OK");
    return 0;
}
