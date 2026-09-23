#include "lib/types/types_interface.h"
#include "app-nxvm/machine/debug_adapter.h"

#include "app-nxvm/devices/debug_interface.h"
#include "app-nxvm/machine/debug.h"
#include "app-nxvm/machine/machine_private.h"

/* Core and Lib intentionally use distinct status domains.  This is the sole
 * debug-adapter crossing: retain every shared classification and map Core's
 * residual FAULT to Lib's generic operational failure. */
static lib_status vm_machine_debug_status_from_type(type_status status)
{
    switch (status) {
    case TYPE_STATUS_OK: return LIB_STATUS_OK;
    case TYPE_STATUS_INVALID_ARGUMENT: return LIB_STATUS_INVALID_ARGUMENT;
    case TYPE_STATUS_INVALID_STATE: return LIB_STATUS_INVALID_STATE;
    case TYPE_STATUS_UNSUPPORTED: return LIB_STATUS_UNSUPPORTED;
    case TYPE_STATUS_NO_MEMORY: return LIB_STATUS_NO_MEMORY;
    case TYPE_STATUS_FAULT: return LIB_STATUS_IO_ERROR;
    }
    return LIB_STATUS_IO_ERROR;
}

static type_status vm_machine_debug_map_watch(
    x86_debug_watch_kind source,
    core_machine_debug_watch_kind *out_target)
{
    if (out_target == LIB_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    switch (source) {
    case X86_DEBUG_WATCH_READ: *out_target = CORE_MACHINE_DEBUG_WATCH_READ; return TYPE_STATUS_OK;
    case X86_DEBUG_WATCH_WRITE: *out_target = CORE_MACHINE_DEBUG_WATCH_WRITE; return TYPE_STATUS_OK;
    case X86_DEBUG_WATCH_EXECUTE: *out_target = CORE_MACHINE_DEBUG_WATCH_EXECUTE; return TYPE_STATUS_OK;
    default: return TYPE_STATUS_INVALID_ARGUMENT;
    }
}

static C_VOID vm_machine_debug_copy_observation(x86_debug_observation *out_observation,
    const core_machine_debug_instruction_observation *source)
{
    lib_u8 index;

    if (out_observation == LIB_NULL || source == LIB_NULL) return;
    lib_memory_set(out_observation, 0, sizeof(*out_observation));
    out_observation->count = source->memory_access_count;
    for (index = 0u; index < source->memory_access_count; ++index) {
        out_observation->accesses[index] = (x86_debug_memory_access) {
            .write = source->memory_accesses[index].write ? LIB_TRUE : LIB_FALSE,
            .linear = source->memory_accesses[index].linear,
            .bytes = source->memory_accesses[index].bytes,
            .data = source->memory_accesses[index].data
        };
    }
    if (!source->watch_hit) return;
    out_observation->watch_hit = LIB_TRUE;
    out_observation->watch_kind = (x86_debug_watch_kind)source->watch_kind;
    out_observation->watch_address = source->watch_address;
}

static C_VOID vm_machine_debug_copy_segment(
    x86_debug_segment_snapshot *out_segment,
    const core_machine_debug_segment_snapshot *source)
{
    if (out_segment == LIB_NULL || source == LIB_NULL) return;
    *out_segment = (x86_debug_segment_snapshot) {
        .selector = source->selector, .base = source->base,
        .limit = source->limit, .dpl = source->dpl,
        .type = source->type, .accessed = source->accessed,
        .executable = source->executable, .conform = source->conform,
        .readable = source->readable, .defsize = source->defsize,
        .big = source->big, .expdown = source->expdown,
        .writable = source->writable
    };
}

static lib_status vm_machine_debug_capture_cpu(vm_machine *machine,
    x86_debug_cpu_snapshot *out_snapshot)
{
    core_machine_debug_cpu_snapshot state;
    type_status status;

    if (machine == LIB_NULL || out_snapshot == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = core_machine_debug_capture_cpu_snapshot(machine->core_machine, &state);
    if (status != TYPE_STATUS_OK) return vm_machine_debug_status_from_type(status);
    lib_memory_set(out_snapshot, 0, sizeof(*out_snapshot));
    vm_machine_debug_copy_segment(&out_snapshot->es, &state.es);
    vm_machine_debug_copy_segment(&out_snapshot->cs, &state.cs);
    vm_machine_debug_copy_segment(&out_snapshot->ss, &state.ss);
    vm_machine_debug_copy_segment(&out_snapshot->ds, &state.ds);
    vm_machine_debug_copy_segment(&out_snapshot->fs, &state.fs);
    vm_machine_debug_copy_segment(&out_snapshot->gs, &state.gs);
    vm_machine_debug_copy_segment(&out_snapshot->tr, &state.tr);
    vm_machine_debug_copy_segment(&out_snapshot->ldtr, &state.ldtr);
    vm_machine_debug_copy_segment(&out_snapshot->gdtr, &state.gdtr);
    vm_machine_debug_copy_segment(&out_snapshot->idtr, &state.idtr);
    out_snapshot->cr0 = state.cr0;
    out_snapshot->cr2 = state.cr2;
    out_snapshot->cr3 = state.cr3;
    return LIB_STATUS_OK;
}

static lib_status vm_machine_debug_execute_request(vm_machine *machine,
    const x86_debug_request *request, x86_debug_response *out_result)
{
    core_machine_debug_register register_id;
    core_machine_debug_watch_kind watch_kind;
    lib_u32 value = 0u;
    type_status status;

    if (machine == LIB_NULL || request == LIB_NULL || out_result == LIB_NULL ||
        request->bytes > X86_DEBUG_BYTES) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_set(out_result, 0, sizeof(*out_result));
    if (request->operation == X86_DEBUG_READ_REGISTER || request->operation == X86_DEBUG_WRITE_REGISTER) {
        if (request->register_id >= CORE_MACHINE_DEBUG_REGISTER_COUNT) return LIB_STATUS_INVALID_ARGUMENT;
        register_id = (core_machine_debug_register)request->register_id;
        if (request->operation == X86_DEBUG_READ_REGISTER) {
            status = core_machine_debug_read_register(machine->core_machine,
                register_id, &out_result->value);
        } else {
            status = core_machine_debug_write_register(machine->core_machine,
                register_id, request->address);
        }
        if (status != TYPE_STATUS_OK) return vm_machine_debug_status_from_type(status);
        return LIB_STATUS_OK;
    }
    if (request->operation == X86_DEBUG_READ_LINEAR || request->operation == X86_DEBUG_WRITE_LINEAR) {
        if (request->bytes == 0u) return LIB_STATUS_INVALID_ARGUMENT;
        status = request->operation == X86_DEBUG_READ_LINEAR ?
            core_machine_debug_read_linear(machine->core_machine, request->address,
                out_result->data, request->bytes) :
            core_machine_debug_write_linear(machine->core_machine, request->address,
                request->data, request->bytes);
        if (status != TYPE_STATUS_OK) return vm_machine_debug_status_from_type(status);
        out_result->bytes = request->bytes; return LIB_STATUS_OK;
    }
    if (request->operation == X86_DEBUG_READ_REAL || request->operation == X86_DEBUG_WRITE_REAL) {
        if (request->bytes == 0u) return LIB_STATUS_INVALID_ARGUMENT;
        status = request->operation == X86_DEBUG_READ_REAL ?
            core_machine_debug_read_real(machine->core_machine, request->segment,
                request->offset, out_result->data, request->bytes) :
            core_machine_debug_write_real(machine->core_machine, request->segment,
                request->offset, request->data, request->bytes);
        if (status != TYPE_STATUS_OK) return vm_machine_debug_status_from_type(status);
        out_result->bytes = request->bytes; return LIB_STATUS_OK;
    }
    if (request->operation == X86_DEBUG_READ_PORT) {
        status = core_machine_debug_read_port(machine->core_machine, request->port, &value);
        if (status != TYPE_STATUS_OK) return vm_machine_debug_status_from_type(status);
        out_result->value = value; return LIB_STATUS_OK;
    }
    if (request->operation == X86_DEBUG_WRITE_PORT) {
        status = core_machine_debug_write_port(machine->core_machine, request->port,
            request->address);
        return vm_machine_debug_status_from_type(status);
    }
    if (request->operation == X86_DEBUG_GET_CODE_DEFAULT_SIZE) {
        C_INT code_size;
        status = core_machine_debug_get_code_default_size(machine->core_machine,
            &code_size);
        if (status != TYPE_STATUS_OK) return vm_machine_debug_status_from_type(status);
        out_result->value = (lib_u32)code_size; return LIB_STATUS_OK;
    }
    if (request->operation == X86_DEBUG_GET_CODE_BASE) {
        status = core_machine_debug_get_code_base(machine->core_machine,
            &out_result->value);
        return vm_machine_debug_status_from_type(status);
    }
    if (request->operation == X86_DEBUG_GET_CPU_SNAPSHOT)
        return vm_machine_debug_capture_cpu(machine, &out_result->cpu);
    if (request->operation == X86_DEBUG_SET_WATCH || request->operation == X86_DEBUG_CLEAR_WATCH) {
        if (vm_machine_debug_map_watch(request->watch_kind, &watch_kind) != TYPE_STATUS_OK) return LIB_STATUS_INVALID_ARGUMENT;
        status = request->operation == X86_DEBUG_SET_WATCH ?
            core_machine_debug_set_watchpoint(machine->core_machine, watch_kind,
                request->address) :
            core_machine_debug_clear_watchpoint(machine->core_machine, watch_kind);
        return vm_machine_debug_status_from_type(status);
    }
    if (request->operation == X86_DEBUG_GET_WATCH) {
        type_bool enabled;

        if (vm_machine_debug_map_watch(request->watch_kind, &watch_kind) != TYPE_STATUS_OK) return LIB_STATUS_INVALID_ARGUMENT;
        status = core_machine_debug_get_watchpoint(machine->core_machine, watch_kind,
            &enabled, &out_result->value);
        if (status != TYPE_STATUS_OK) return vm_machine_debug_status_from_type(status);
        out_result->enabled = enabled ? LIB_TRUE : LIB_FALSE;
        return LIB_STATUS_OK;
    }
    if (request->operation == X86_DEBUG_SET_EXECUTION_PLAN) {
        return vm_machine_debug_set_execution_plan(&machine->debug, request) ==
            TYPE_STATUS_OK ? LIB_STATUS_OK : LIB_STATUS_INVALID_ARGUMENT;
    }
    if (request->operation == X86_DEBUG_CLEAR_EXECUTION_PLAN) {
        vm_machine_debug_clear_execution_plan(&machine->debug);
        return LIB_STATUS_OK;
    }
    if (request->operation == X86_DEBUG_GET_EXECUTION_RESULT) {
        vm_machine_debug_stop_reason reason;
        lib_u64 executed;

        if (!vm_machine_debug_take_completion(&machine->debug, &reason,
                &executed)) return LIB_STATUS_OK;
        out_result->enabled = LIB_TRUE;
        out_result->value = executed > UINT32_MAX ? UINT32_MAX :
            (lib_u32)executed;
        if (machine->debug.observation_valid)
            vm_machine_debug_copy_observation(&out_result->observation,
                &machine->debug.observation);
        return reason == VM_MACHINE_DEBUG_STOP_TRACE ||
            reason == VM_MACHINE_DEBUG_STOP_BREAKPOINT ||
            reason == VM_MACHINE_DEBUG_STOP_WATCHPOINT ? LIB_STATUS_OK :
            LIB_STATUS_INVALID_STATE;
    }
    return LIB_STATUS_INVALID_ARGUMENT;
}

lib_status vm_machine_debug_execute(C_VOID *context,
    const C_VOID *request, lib_size request_size,
    C_VOID *response, lib_size response_capacity, lib_size *response_size)
{
    lib_status status;

    if (response_size == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *response_size = 0u;
    if (request == LIB_NULL || request_size != sizeof(x86_debug_request) ||
        response == LIB_NULL || response_capacity < sizeof(x86_debug_response))
        return LIB_STATUS_INVALID_ARGUMENT;
    status = vm_machine_debug_execute_request(context, request, response);
    if (status == LIB_STATUS_OK) *response_size = sizeof(x86_debug_response);
    return status;
}
