#include "vm/composition/session/start_outcome.h"

int main(void)
{
    vm_session_start_outcome *outcome = STD_NULL;
    vm_session_start_outcome_snapshot snapshot;

    if (vm_session_start_outcome_create(STD_NULL) != TYPE_STATUS_INVALID_ARGUMENT ||
        vm_session_start_outcome_capture(STD_NULL, &snapshot) !=
            TYPE_STATUS_INVALID_ARGUMENT || vm_session_start_outcome_create(&outcome) !=
            TYPE_STATUS_OK || vm_session_start_outcome_publish(STD_NULL,
            TYPE_STATUS_FAULT) != TYPE_STATUS_INVALID_ARGUMENT ||
        vm_session_start_outcome_capture(outcome, STD_NULL) !=
            TYPE_STATUS_INVALID_ARGUMENT ||
        vm_session_start_outcome_capture(outcome, &snapshot) != TYPE_STATUS_OK ||
        snapshot.valid || snapshot.status != TYPE_STATUS_OK || snapshot.sequence != 0u ||
        vm_session_start_outcome_publish(outcome, TYPE_STATUS_FAULT) !=
            TYPE_STATUS_FAULT ||
        vm_session_start_outcome_capture(outcome, &snapshot) != TYPE_STATUS_OK ||
        !snapshot.valid || snapshot.status != TYPE_STATUS_FAULT || snapshot.sequence != 1u) {
        vm_session_start_outcome_destroy(outcome);
        return 1;
    }
    vm_session_start_outcome_clear(outcome);
    if (vm_session_start_outcome_capture(outcome, &snapshot) != TYPE_STATUS_OK ||
        snapshot.valid || snapshot.status != TYPE_STATUS_OK || snapshot.sequence != 1u) {
        vm_session_start_outcome_destroy(outcome);
        return 1;
    }
    vm_session_start_outcome_destroy(outcome);
    puts("M5:T523:S4:OBSERVABILITY-OUTCOME:OK");
    return 0;
}
