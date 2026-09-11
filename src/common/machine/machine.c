#include "common/machine/machine_interface.h"

#include "lib/host/sync_interface.h"

#include <stdatomic.h>
#include <stdlib.h>

#define COMMON_MACHINE_QUEUE_CAPACITY 32u

struct common_machine {
    host_sync_event *ready;
    atomic_flag lock;
    common_machine_request requests[COMMON_MACHINE_QUEUE_CAPACITY];
    lib_size first;
    lib_size count;
    lib_bool accepting;
    lib_u32 run_id;
    common_machine_driver driver;
    lib_u64 debug_generation;
};

static void common_machine_lock(common_machine *machine)
{
    while (atomic_flag_test_and_set_explicit(&machine->lock, memory_order_acquire)) { }
}

static void common_machine_unlock(common_machine *machine)
{
    atomic_flag_clear_explicit(&machine->lock, memory_order_release);
}

lib_status common_machine_create(common_machine **out_machine)
{
    common_machine *machine;

    if (out_machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_machine = LIB_NULL;
    machine = calloc(1u, sizeof(*machine));
    if (machine == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    machine->lock = (atomic_flag)ATOMIC_FLAG_INIT;
    atomic_flag_clear_explicit(&machine->lock, memory_order_release);
    if (host_sync_event_create(&machine->ready) != LIB_STATUS_OK) {
        free(machine);
        return LIB_STATUS_NO_MEMORY;
    }
    machine->accepting = LIB_TRUE;
    machine->debug_generation = 1u;
    *out_machine = machine;
    return LIB_STATUS_OK;
}

void common_machine_destroy(common_machine *machine)
{
    if (machine == LIB_NULL) return;
    host_sync_event_destroy(machine->ready);
    free(machine);
}

void common_machine_close(common_machine *machine)
{
    if (machine == LIB_NULL) return;
    common_machine_lock(machine);
    machine->accepting = LIB_FALSE;
    ++machine->debug_generation;
    host_sync_event_signal(machine->ready);
    common_machine_unlock(machine);
}

lib_status common_machine_bind_driver(common_machine *machine,
    const common_machine_driver *driver)
{
    if (machine == LIB_NULL || driver == LIB_NULL ||
        driver->consume_request == LIB_NULL || driver->is_paused == LIB_NULL ||
        driver->execute_debug == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    common_machine_lock(machine);
    if (!machine->accepting || machine->driver.consume_request != LIB_NULL) {
        common_machine_unlock(machine);
        return LIB_STATUS_INVALID_STATE;
    }
    machine->driver = *driver;
    common_machine_unlock(machine);
    return LIB_STATUS_OK;
}

lib_status common_machine_bind_run(common_machine *machine, lib_u32 run_id)
{
    if (machine == LIB_NULL || run_id == 0u) return LIB_STATUS_INVALID_ARGUMENT;
    common_machine_lock(machine);
    if (!machine->accepting) {
        common_machine_unlock(machine);
        return LIB_STATUS_INVALID_STATE;
    }
    machine->run_id = run_id;
    common_machine_unlock(machine);
    return LIB_STATUS_OK;
}

lib_status common_machine_submit(common_machine *machine,
    const common_machine_request *request)
{
    lib_size index;

    if (machine == LIB_NULL || request == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    common_machine_lock(machine);
    if (!machine->accepting || machine->driver.consume_request == LIB_NULL) {
        common_machine_unlock(machine);
        return LIB_STATUS_INVALID_STATE;
    }
    if (machine->count == COMMON_MACHINE_QUEUE_CAPACITY) {
        common_machine_unlock(machine);
        return LIB_STATUS_LIMIT_EXCEEDED;
    }
    index = (machine->first + machine->count) % COMMON_MACHINE_QUEUE_CAPACITY;
    machine->requests[index] = *request;
    if (machine->requests[index].run_id == 0u) machine->requests[index].run_id =
        machine->run_id;
    ++machine->count;
    host_sync_event_signal(machine->ready);
    common_machine_unlock(machine);
    return LIB_STATUS_OK;
}

lib_status common_machine_observe_safe_point(common_machine *machine)
{
    common_machine_request request;
    common_machine_driver driver;

    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    common_machine_lock(machine);
    if (machine->count == 0u) {
        common_machine_unlock(machine);
        return LIB_STATUS_NOT_CURRENT;
    }
    request = machine->requests[machine->first];
    machine->first = (machine->first + 1u) % COMMON_MACHINE_QUEUE_CAPACITY;
    --machine->count;
    if (machine->count == 0u) host_sync_event_reset(machine->ready);
    driver = machine->driver;
    if (request.run_id != 0u && request.run_id != machine->run_id) {
        common_machine_unlock(machine);
        return LIB_STATUS_NOT_CURRENT;
    }
    if (request.kind == COMMON_MACHINE_REQUEST_RESUME ||
        request.kind == COMMON_MACHINE_REQUEST_RESET ||
        request.kind == COMMON_MACHINE_REQUEST_STOP) ++machine->debug_generation;
    common_machine_unlock(machine);
    driver.consume_request(driver.context, &request);
    return LIB_STATUS_OK;
}

lib_status common_machine_wait(common_machine *machine,
    lib_u32 timeout_milliseconds)
{
    host_sync_wait_result result;

    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    common_machine_lock(machine);
    if (machine->count != 0u) {
        common_machine_unlock(machine);
        return LIB_STATUS_OK;
    }
    if (!machine->accepting) {
        common_machine_unlock(machine);
        return LIB_STATUS_INVALID_STATE;
    }
    host_sync_event_reset(machine->ready);
    common_machine_unlock(machine);
    result = host_sync_event_wait(machine->ready, timeout_milliseconds);
    return result == HOST_SYNC_WAIT_SIGNALED ? LIB_STATUS_OK :
        result == HOST_SYNC_WAIT_TIMED_OUT ? LIB_STATUS_NOT_CURRENT : LIB_STATUS_INVALID_STATE;
}

lib_status common_machine_debug_acquire(common_machine *machine,
    common_machine_debug_lease *out_lease)
{
    common_machine_driver driver;

    if (machine == LIB_NULL || out_lease == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    common_machine_lock(machine);
    driver = machine->driver;
    if (!machine->accepting || driver.is_paused == LIB_NULL ||
        !driver.is_paused(driver.context)) {
        common_machine_unlock(machine);
        return LIB_STATUS_INVALID_STATE;
    }
    out_lease->generation = machine->debug_generation;
    common_machine_unlock(machine);
    return LIB_STATUS_OK;
}

lib_status common_machine_debug_execute_with_lease(common_machine *machine,
    const common_machine_debug_lease *lease,
    const common_machine_debug_request *request,
    common_machine_debug_result *out_result)
{
    common_machine_driver driver;

    if (machine == LIB_NULL || lease == LIB_NULL || request == LIB_NULL ||
        out_result == LIB_NULL || request->bytes > COMMON_MACHINE_DEBUG_BYTES)
        return LIB_STATUS_INVALID_ARGUMENT;
    common_machine_lock(machine);
    driver = machine->driver;
    if (!machine->accepting || lease->generation == 0u ||
        lease->generation != machine->debug_generation || driver.is_paused == LIB_NULL ||
        driver.execute_debug == LIB_NULL || !driver.is_paused(driver.context)) {
        common_machine_unlock(machine);
        return LIB_STATUS_INVALID_STATE;
    }
    common_machine_unlock(machine);
    return driver.execute_debug(driver.context, request, out_result);
}

void common_machine_debug_invalidate(common_machine *machine)
{
    if (machine == LIB_NULL) return;
    common_machine_lock(machine);
    ++machine->debug_generation;
    common_machine_unlock(machine);
}
