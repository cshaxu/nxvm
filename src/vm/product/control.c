#include "type.h"

#include "lib/host/sync_interface.h"
#include "vm/product/control.h"

#define VM_PRODUCT_CONTROL_FACT_CAPACITY 64u

struct vm_product_control {
    host_sync_event *ready;
    STD_ATOMIC_FLAG lock;
    vm_product_control_fact facts[VM_PRODUCT_CONTROL_FACT_CAPACITY];
    STD_SIZE_T first;
    STD_SIZE_T count;
    C_INT accepting;
    C_INT delivery_failed;
    vm_session_lifecycle lifecycle;
};

static C_VOID vm_product_control_lock(vm_product_control *control)
{
    while (STD_ATOMIC_FLAG_TEST_AND_SET_EXPLICIT(&control->lock,
        STD_MEMORY_ORDER_ACQUIRE)) { }
}

static C_VOID vm_product_control_unlock(vm_product_control *control)
{
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&control->lock, STD_MEMORY_ORDER_RELEASE);
}

type_status vm_product_control_create(vm_product_control **out_control)
{
    vm_product_control *control;

    if (out_control == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_control = STD_NULL;
    control = STD_CALLOC(1u, sizeof(*control));
    if (control == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    control->lock = (STD_ATOMIC_FLAG)ATOMIC_FLAG_INIT;
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&control->lock, STD_MEMORY_ORDER_RELEASE);
    if (host_sync_event_create(&control->ready) != LIB_STATUS_OK) {
        STD_FREE(control);
        return TYPE_STATUS_NO_MEMORY;
    }
    control->accepting = TYPE_TRUE;
    control->lifecycle = VM_SESSION_STOPPED;
    *out_control = control;
    return TYPE_STATUS_OK;
}

C_VOID vm_product_control_destroy(vm_product_control *control)
{
    if (control == STD_NULL) return;
    host_sync_event_destroy(control->ready);
    STD_FREE(control);
}

type_status vm_product_control_publish(vm_product_control *control,
    const vm_product_control_fact *fact)
{
    STD_SIZE_T index;

    if (control == STD_NULL || fact == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    vm_product_control_lock(control);
    if (!control->accepting) {
        vm_product_control_unlock(control);
        return TYPE_STATUS_INVALID_STATE;
    }
    if (control->count == VM_PRODUCT_CONTROL_FACT_CAPACITY) {
        control->delivery_failed = TYPE_TRUE;
        host_sync_event_signal(control->ready);
        vm_product_control_unlock(control);
        return TYPE_STATUS_NO_MEMORY;
    }
    index = (control->first + control->count) % VM_PRODUCT_CONTROL_FACT_CAPACITY;
    control->facts[index] = *fact;
    ++control->count;
    host_sync_event_signal(control->ready);
    vm_product_control_unlock(control);
    return TYPE_STATUS_OK;
}

type_status vm_product_control_take(vm_product_control *control,
    vm_product_control_fact *out_fact, C_UINT timeout_milliseconds)
{
    if (control == STD_NULL || out_fact == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (host_sync_event_wait(control->ready, timeout_milliseconds) !=
        HOST_SYNC_WAIT_SIGNALED) return TYPE_STATUS_INVALID_STATE;
    vm_product_control_lock(control);
    if (control->delivery_failed) {
        vm_product_control_unlock(control);
        return TYPE_STATUS_NO_MEMORY;
    }
    if (control->count == 0u) {
        vm_product_control_unlock(control);
        return TYPE_STATUS_INVALID_STATE;
    }
    *out_fact = control->facts[control->first];
    control->first = (control->first + 1u) % VM_PRODUCT_CONTROL_FACT_CAPACITY;
    --control->count;
    if (control->count == 0u) host_sync_event_reset(control->ready);
    vm_product_control_unlock(control);
    return TYPE_STATUS_OK;
}

C_VOID vm_product_control_close(vm_product_control *control)
{
    if (control == STD_NULL) return;
    vm_product_control_lock(control);
    control->accepting = TYPE_FALSE;
    host_sync_event_signal(control->ready);
    vm_product_control_unlock(control);
}

const C_CHAR *vm_product_control_note_lifecycle(vm_product_control *control,
    vm_session_lifecycle lifecycle)
{
    const C_CHAR *name;

    if (control == STD_NULL) return "stopped";
    if (lifecycle == VM_SESSION_RESET) return "reset";
    if (lifecycle == VM_SESSION_RUNNING) {
        name = control->lifecycle == VM_SESSION_PAUSED ? "resumed" : "started";
    } else if (lifecycle == VM_SESSION_PAUSED) {
        name = "paused";
    } else {
        name = "stopped";
    }
    control->lifecycle = lifecycle;
    return name;
}
