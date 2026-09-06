#include "type.h"

#include "lib/observability/outcome.h"

C_INT main(C_VOID)
{
    lib_observability_outcome *outcome = STD_NULL;
    lib_observability_outcome_snapshot snapshot;

    if (lib_observability_outcome_create(&outcome) != LIB_STATUS_OK ||
        lib_observability_outcome_capture(outcome, &snapshot) != LIB_STATUS_OK ||
        snapshot.valid || snapshot.status != LIB_STATUS_OK || snapshot.sequence != 0u ||
        lib_observability_outcome_publish(outcome, LIB_STATUS_IO_ERROR) !=
            LIB_STATUS_IO_ERROR ||
        lib_observability_outcome_capture(outcome, &snapshot) != LIB_STATUS_OK ||
        !snapshot.valid || snapshot.status != LIB_STATUS_IO_ERROR || snapshot.sequence != 1u) {
        lib_observability_outcome_destroy(outcome);
        return 1;
    }
    lib_observability_outcome_clear(outcome);
    if (lib_observability_outcome_capture(outcome, &snapshot) != LIB_STATUS_OK ||
        snapshot.valid || snapshot.status != LIB_STATUS_OK || snapshot.sequence != 1u) {
        lib_observability_outcome_destroy(outcome);
        return 1;
    }
    lib_observability_outcome_destroy(outcome);
    puts("M5:T522:S10:OBSERVABILITY-OUTCOME:OK");
    return 0;
}
