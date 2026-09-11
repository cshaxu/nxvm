#include "common/machine/machine_interface.h"

typedef struct common_machine_observer {
    common_machine_request requests[3u];
    lib_size count;
    lib_bool paused;
} common_machine_observer;

static void common_machine_observe(void *context,
    const common_machine_request *request)
{
    common_machine_observer *observer = context;

    if (observer != LIB_NULL && request != LIB_NULL && observer->count < 3u)
        observer->requests[observer->count++] = *request;
}

static lib_bool common_machine_observer_paused(void *context)
{
    const common_machine_observer *observer = context;
    return observer != LIB_NULL && observer->paused;
}

static lib_status common_machine_observer_debug(void *context,
    const common_machine_debug_request *request,
    common_machine_debug_result *out_result)
{
    (void)context;
    if (request == LIB_NULL || out_result == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_result = (common_machine_debug_result) { .value = request->address };
    return LIB_STATUS_OK;
}

int main(void)
{
    common_machine *machine = LIB_NULL;
    common_machine_observer observer = { .paused = LIB_TRUE };
    common_machine_driver driver = {
        common_machine_observe, common_machine_observer_paused,
        common_machine_observer_debug, &observer
    };
    common_machine_request request = { .kind = COMMON_MACHINE_REQUEST_INPUT,
        .run_id = 7u };
    common_machine_debug_lease lease;
    common_machine_debug_result result;

    if (common_machine_create(&machine) != LIB_STATUS_OK ||
        common_machine_bind_driver(machine, &driver) != LIB_STATUS_OK ||
        common_machine_bind_run(machine, 7u) != LIB_STATUS_OK ||
        common_machine_submit(machine, &request) != LIB_STATUS_OK ||
        common_machine_observe_safe_point(machine) != LIB_STATUS_OK ||
        observer.count != 1u || observer.requests[0u].run_id != 7u ||
        common_machine_debug_acquire(machine, &lease) != LIB_STATUS_OK ||
        common_machine_debug_execute_with_lease(machine, &lease,
            &(common_machine_debug_request) { .operation =
                COMMON_MACHINE_DEBUG_READ_LINEAR, .address = 0x1234u },
            &result) != LIB_STATUS_OK || result.value != 0x1234u) {
        common_machine_destroy(machine);
        return 1;
    }
    if (common_machine_submit(machine, &(common_machine_request) { .kind =
            COMMON_MACHINE_REQUEST_INPUT, .run_id = 6u }) != LIB_STATUS_OK ||
        common_machine_observe_safe_point(machine) != LIB_STATUS_NOT_CURRENT ||
        observer.count != 1u || common_machine_submit(machine, &(common_machine_request) { .kind =
            COMMON_MACHINE_REQUEST_RESUME }) != LIB_STATUS_OK ||
        common_machine_observe_safe_point(machine) != LIB_STATUS_OK ||
        common_machine_debug_execute_with_lease(machine, &lease,
            &(common_machine_debug_request) {0}, &result) != LIB_STATUS_INVALID_STATE) {
        common_machine_destroy(machine);
        return 1;
    }
    common_machine_close(machine);
    if (common_machine_submit(machine, &request) != LIB_STATUS_INVALID_STATE) {
        common_machine_destroy(machine);
        return 1;
    }
    common_machine_destroy(machine);
    return 0;
}
