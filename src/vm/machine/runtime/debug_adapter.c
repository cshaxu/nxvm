#include "vm/machine/runtime/debug_adapter.h"

#include "core/machine/debug_interface.h"
#include "vm/machine/debug.h"
#include "vm/machine/runtime/machine_private.h"

static type_status vm_machine_common_debug_map_watch(
    common_machine_debug_watch_kind source,
    core_machine_debug_watch_kind *out_target)
{
    if (out_target == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    switch (source) {
    case COMMON_MACHINE_DEBUG_WATCH_READ: *out_target = CORE_MACHINE_DEBUG_WATCH_READ; return TYPE_STATUS_OK;
    case COMMON_MACHINE_DEBUG_WATCH_WRITE: *out_target = CORE_MACHINE_DEBUG_WATCH_WRITE; return TYPE_STATUS_OK;
    case COMMON_MACHINE_DEBUG_WATCH_EXECUTE: *out_target = CORE_MACHINE_DEBUG_WATCH_EXECUTE; return TYPE_STATUS_OK;
    default: return TYPE_STATUS_INVALID_ARGUMENT;
    }
}

lib_status vm_machine_common_debug_execute(void *context,
    const common_machine_debug_request *request,
    common_machine_debug_result *out_result)
{
    vm_machine *machine = context;
    core_machine_debug_register register_id;
    core_machine_debug_watch_kind watch_kind;
    type_unsigned_32 value = 0u;

    if (machine == STD_NULL || request == LIB_NULL || out_result == LIB_NULL || request->bytes > COMMON_MACHINE_DEBUG_BYTES) return LIB_STATUS_INVALID_ARGUMENT;
    STD_MEMSET(out_result, 0, sizeof(*out_result));
    if (request->operation == COMMON_MACHINE_DEBUG_READ_REGISTER || request->operation == COMMON_MACHINE_DEBUG_WRITE_REGISTER) {
        if (request->register_id >= CORE_MACHINE_DEBUG_REGISTER_COUNT) return LIB_STATUS_INVALID_ARGUMENT;
        register_id = (core_machine_debug_register)request->register_id;
        if (request->operation == COMMON_MACHINE_DEBUG_READ_REGISTER) {
            if (core_machine_debug_read_register(machine->core_machine, register_id, &out_result->value) != TYPE_STATUS_OK) return LIB_STATUS_INVALID_STATE;
        } else if (core_machine_debug_write_register(machine->core_machine, register_id, request->address) != TYPE_STATUS_OK) return LIB_STATUS_INVALID_STATE;
        return LIB_STATUS_OK;
    }
    if (request->operation == COMMON_MACHINE_DEBUG_READ_LINEAR || request->operation == COMMON_MACHINE_DEBUG_WRITE_LINEAR) {
        if (request->bytes == 0u) return LIB_STATUS_INVALID_ARGUMENT;
        if ((request->operation == COMMON_MACHINE_DEBUG_READ_LINEAR ? core_machine_debug_read_linear(machine->core_machine, request->address, out_result->data, request->bytes) : core_machine_debug_write_linear(machine->core_machine, request->address, request->data, request->bytes)) != TYPE_STATUS_OK) return LIB_STATUS_INVALID_STATE;
        out_result->bytes = request->bytes; return LIB_STATUS_OK;
    }
    if (request->operation == COMMON_MACHINE_DEBUG_READ_REAL || request->operation == COMMON_MACHINE_DEBUG_WRITE_REAL) {
        if (request->bytes == 0u) return LIB_STATUS_INVALID_ARGUMENT;
        if ((request->operation == COMMON_MACHINE_DEBUG_READ_REAL ? core_machine_debug_read_real(machine->core_machine, request->segment, request->offset, out_result->data, request->bytes) : core_machine_debug_write_real(machine->core_machine, request->segment, request->offset, request->data, request->bytes)) != TYPE_STATUS_OK) return LIB_STATUS_INVALID_STATE;
        out_result->bytes = request->bytes; return LIB_STATUS_OK;
    }
    if (request->operation == COMMON_MACHINE_DEBUG_READ_PORT) {
        if (core_machine_debug_read_port(machine->core_machine, request->port, &value) != TYPE_STATUS_OK) return LIB_STATUS_INVALID_STATE;
        out_result->value = value; return LIB_STATUS_OK;
    }
    if (request->operation == COMMON_MACHINE_DEBUG_WRITE_PORT) return core_machine_debug_write_port(machine->core_machine, request->port, request->address) == TYPE_STATUS_OK ? LIB_STATUS_OK : LIB_STATUS_INVALID_STATE;
    if (request->operation == COMMON_MACHINE_DEBUG_GET_CODE_DEFAULT_SIZE) {
        C_INT code_size;
        if (core_machine_debug_get_code_default_size(machine->core_machine, &code_size) != TYPE_STATUS_OK) return LIB_STATUS_INVALID_STATE;
        out_result->value = (type_unsigned_32)code_size; return LIB_STATUS_OK;
    }
    if (request->operation == COMMON_MACHINE_DEBUG_GET_CODE_BASE) return core_machine_debug_get_code_base(machine->core_machine, &out_result->value) == TYPE_STATUS_OK ? LIB_STATUS_OK : LIB_STATUS_INVALID_STATE;
    if (request->operation == COMMON_MACHINE_DEBUG_SET_WATCH || request->operation == COMMON_MACHINE_DEBUG_CLEAR_WATCH) {
        if (vm_machine_common_debug_map_watch(request->watch_kind, &watch_kind) != TYPE_STATUS_OK) return LIB_STATUS_INVALID_ARGUMENT;
        return (request->operation == COMMON_MACHINE_DEBUG_SET_WATCH ? core_machine_debug_set_watchpoint(machine->core_machine, watch_kind, request->address) : core_machine_debug_clear_watchpoint(machine->core_machine, watch_kind)) == TYPE_STATUS_OK ? LIB_STATUS_OK : LIB_STATUS_INVALID_STATE;
    }
    if (request->operation == COMMON_MACHINE_DEBUG_GET_WATCH) {
        type_bool enabled;

        if (vm_machine_common_debug_map_watch(request->watch_kind, &watch_kind) != TYPE_STATUS_OK) return LIB_STATUS_INVALID_ARGUMENT;
        if (core_machine_debug_get_watchpoint(machine->core_machine, watch_kind,
                &enabled, &out_result->value) != TYPE_STATUS_OK) return LIB_STATUS_INVALID_STATE;
        out_result->enabled = enabled ? LIB_TRUE : LIB_FALSE;
        return LIB_STATUS_OK;
    }
    if (request->operation == COMMON_MACHINE_DEBUG_SET_EXECUTION_PLAN) {
        return vm_machine_debug_set_execution_plan(&machine->debug, request) ==
            TYPE_STATUS_OK ? LIB_STATUS_OK : LIB_STATUS_INVALID_ARGUMENT;
    }
    if (request->operation == COMMON_MACHINE_DEBUG_CLEAR_EXECUTION_PLAN) {
        vm_machine_debug_clear_execution_plan(&machine->debug);
        return LIB_STATUS_OK;
    }
    if (request->operation == COMMON_MACHINE_DEBUG_GET_EXECUTION_RESULT) {
        vm_machine_pause_reason reason;
        type_unsigned_64 executed;

        if (!vm_machine_debug_take_completion(&machine->debug, &reason,
                &executed)) return LIB_STATUS_OK;
        out_result->enabled = LIB_TRUE;
        out_result->value = executed > UINT32_MAX ? UINT32_MAX :
            (type_unsigned_32)executed;
        return reason == VM_MACHINE_PAUSE_TRACE ||
            reason == VM_MACHINE_PAUSE_BREAKPOINT ? LIB_STATUS_OK :
            LIB_STATUS_INVALID_STATE;
    }
    return LIB_STATUS_INVALID_ARGUMENT;
}
