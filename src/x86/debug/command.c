/* DEBUG is the debug console for users to break, trace, lookup,
 * and print virtual machine devices. */

#include "x86/debug/debug_interface.h"
#include "x86/debug/command_runtime.h"
#include "x86/xasm32/xasm32_interface.h"
#include "lib/storage/file_interface.h"
#include "lib/storage/medium_interface.h"

#define DEBUG_MAXNARG 256
#define DEBUG_MAXNASMARG 4

typedef enum command_continuation {
    COMMAND_CONTINUATION_NONE,
    COMMAND_CONTINUATION_ASSEMBLE,
    COMMAND_CONTINUATION_ENTER,
    COMMAND_CONTINUATION_REGISTER,
    COMMAND_CONTINUATION_VERBAL,
    COMMAND_CONTINUATION_XASSEMBLE,
    COMMAND_CONTINUATION_XENTER,
    COMMAND_CONTINUATION_XREGISTER
} command_continuation;

typedef enum command_run_kind {
    COMMAND_RUN_NONE,
    COMMAND_RUN_TRACE_REAL,
    COMMAND_RUN_TRACE_LINEAR,
    COMMAND_RUN_BREAK_REAL,
    COMMAND_RUN_BREAK_LINEAR
} command_run_kind;

struct x86_debug {
    common_machine *machine;
    lib_status access_status;
    lib_bool defaults_ready;
    type_unsigned_32 assemble_linear;
    type_unsigned_32 dump_linear;
    type_unsigned_32 unassemble_linear;
    x86_debug_result *result;
    C_CHAR *output;
    lib_size output_length, output_capacity;
    lib_status output_status;
    C_CHAR input_prompt[X86_DEBUG_PROMPT_CAPACITY];
    STD_SIZE_T error_position;
    STD_SIZE_T argument_count;
    C_CHAR *arguments[DEBUG_MAXNARG];
    C_INT exit_requested;
    C_CHAR command_buffer[0x100];
    C_CHAR command_copy[0x100];
    C_CHAR file_name[0x100];
    type_unsigned_16 dump_segment;
    type_unsigned_16 dump_offset;
    type_unsigned_16 assemble_segment;
    type_unsigned_16 assemble_offset;
    type_unsigned_16 unassemble_segment;
    type_unsigned_16 unassemble_offset;
    type_unsigned_16 parsed_segment;
    type_unsigned_16 parsed_offset;
    C_CHAR pending_line[X86_DEBUG_LINE_CAPACITY];
    C_INT pending_line_available;
    command_continuation continuation;
    type_unsigned_32 breakpoint_linear;
    STD_SIZE_T breakpoint_remaining;
    STD_SIZE_T trace_remaining;
    command_run_kind run_kind;
    x86_debug_observation observation;
};

typedef x86_debug command_context;
typedef x86_debug_register command_register;
typedef x86_debug_watch_kind command_machine_watch_kind;

static C_INT command_copy_text_checked(C_CHAR *destination,
    STD_SIZE_T destination_capacity, const C_CHAR *source);

#define COMMAND_REGISTER_EAX X86_DEBUG_EAX
#define COMMAND_REGISTER_ECX X86_DEBUG_ECX
#define COMMAND_REGISTER_EDX X86_DEBUG_EDX
#define COMMAND_REGISTER_EBX X86_DEBUG_EBX
#define COMMAND_REGISTER_ESP X86_DEBUG_ESP
#define COMMAND_REGISTER_EBP X86_DEBUG_EBP
#define COMMAND_REGISTER_ESI X86_DEBUG_ESI
#define COMMAND_REGISTER_EDI X86_DEBUG_EDI
#define COMMAND_REGISTER_EIP X86_DEBUG_EIP
#define COMMAND_REGISTER_EFLAGS X86_DEBUG_EFLAGS
#define COMMAND_REGISTER_ES X86_DEBUG_ES
#define COMMAND_REGISTER_CS X86_DEBUG_CS
#define COMMAND_REGISTER_SS X86_DEBUG_SS
#define COMMAND_REGISTER_DS X86_DEBUG_DS
#define COMMAND_REGISTER_FS X86_DEBUG_FS
#define COMMAND_REGISTER_GS X86_DEBUG_GS
#define COMMAND_REGISTER_CR0 X86_DEBUG_CR0
#define COMMAND_REGISTER_CR1 X86_DEBUG_CR1
#define COMMAND_REGISTER_CR2 X86_DEBUG_CR2
#define COMMAND_REGISTER_CR3 X86_DEBUG_CR3
#define COMMAND_REGISTER_CR4 X86_DEBUG_CR4
#define COMMAND_REGISTER_WATCH_READ X86_DEBUG_WATCH_READ
#define COMMAND_REGISTER_WATCH_WRITE X86_DEBUG_WATCH_WRITE
#define COMMAND_REGISTER_WATCH_EXECUTE X86_DEBUG_WATCH_EXECUTE

static C_INT command_printf(command_context *command, const char *format, ...)
{
    lib_c_va_list args, measure;
    lib_i32 count;
    lib_size needed, capacity;
    char *expanded;
    if (command == STD_NULL || command->result == STD_NULL) return -1;
    /* Reentering a read continuation must not print its already displayed
     * prefix again. Reading the supplied line opens output for its result. */
    if (command->pending_line_available) return 0;
    if (command->output_status != LIB_STATUS_OK) return -1;
    lib_c_va_start(args, format);
    lib_c_va_copy(measure, args);
    count = lib_c_vsnprintf(NULL, 0u, format, measure);
    lib_c_va_end(measure);
    if (count < 0 || (lib_size)count >= LIB_SIZE_MAX - command->output_length) {
        command->output_status = LIB_STATUS_IO_ERROR;
    } else {
        needed = command->output_length + (lib_size)count + 1u;
        if (needed > command->output_capacity) {
            capacity = command->output_capacity > LIB_SIZE_MAX / 2u ? needed :
                command->output_capacity * 2u;
            if (capacity < needed) capacity = needed;
            if (capacity < 1024u) capacity = 1024u;
            expanded = lib_reallocate(command->output, capacity);
            if (expanded == NULL) command->output_status = LIB_STATUS_NO_MEMORY;
            else { command->output = expanded; command->output_capacity = capacity; }
        }
        if (command->output_status == LIB_STATUS_OK) {
            if (lib_c_vsnprintf(command->output + command->output_length,
                    command->output_capacity - command->output_length, format, args) != count)
                command->output_status = LIB_STATUS_IO_ERROR;
            else command->output_length += (lib_size)count;
        }
    }
    lib_c_va_end(args);
    return command->output_status == LIB_STATUS_OK ? count : -1;
}

static void command_begin_output(command_context *command, x86_debug_result *result)
{
    *result = (x86_debug_result) { .text = "", .keep_active = LIB_TRUE };
    command->result = result;
    command->output_length = 0u;
    command->output_status = LIB_STATUS_OK;
    if (command->output != NULL) command->output[0] = '\0';
}

static lib_status command_end_output(command_context *command)
{
    if (command->output_status != LIB_STATUS_OK) {
        command->continuation = COMMAND_CONTINUATION_NONE;
        command->pending_line_available = 0;
        command->run_kind = COMMAND_RUN_NONE;
        command->result->lifecycle_request = X86_DEBUG_LIFECYCLE_NONE;
        command->result->prompt[0] = '-';
        command->result->prompt[1] = '\0';
    }
    command->result->text = command->output_length != 0u ? command->output : "";
    command->result = NULL;
    return command->output_status;
}

static C_INT command_read_line(command_context *debugContext,
    C_CHAR *buffer, STD_SIZE_T buffer_size)
{
    if (debugContext == STD_NULL || buffer == STD_NULL || buffer_size == 0u) return 0;
    if (!debugContext->pending_line_available) {
        lib_size start = debugContext->output_length;
        while (start != 0u && debugContext->output[start - 1u] != '\n') --start;
        if (debugContext->output_status == LIB_STATUS_OK &&
            command_copy_text_checked(debugContext->input_prompt,
                sizeof(debugContext->input_prompt), debugContext->output + start)) {
            debugContext->output[start] = '\0';
            debugContext->output_length = start;
            debugContext->result->prompt_ready = LIB_TRUE;
        }
        return 0;
    }
    if (!command_copy_text_checked(buffer, buffer_size,
            debugContext->pending_line)) return 0;
    debugContext->pending_line_available = 0;
    return 1;
}

static C_INT command_execute(command_context *debugContext,
    const x86_debug_request *request,
    x86_debug_response *result)
{
    common_machine_debug_lease lease;
    lib_size response_size;

    _Static_assert(sizeof(*request) <= COMMON_MACHINE_DEBUG_REQUEST_CAPACITY,
        "x86 request must fit Machine transport");
    _Static_assert(sizeof(*result) <= COMMON_MACHINE_DEBUG_RESPONSE_CAPACITY,
        "x86 response must fit Machine transport");

    if (debugContext == STD_NULL || request == STD_NULL || result == STD_NULL) return 1;
    STD_MEMSET(result, 0, sizeof(*result));
    if (debugContext->access_status != LIB_STATUS_OK) return 1;
    debugContext->access_status = common_machine_debug_acquire(debugContext->machine, &lease);
    if (debugContext->access_status == LIB_STATUS_OK)
        debugContext->access_status = common_machine_debug_execute_with_lease(
            debugContext->machine, &lease, request, sizeof(*request),
            result, sizeof(*result), &response_size);
    if (debugContext->access_status == LIB_STATUS_OK && response_size != sizeof(*result)) {
        STD_MEMSET(result, 0, sizeof(*result));
        debugContext->access_status = LIB_STATUS_IO_ERROR;
    }
    if (debugContext->access_status != LIB_STATUS_OK) debugContext->error_position = 1u;
    return debugContext->access_status != LIB_STATUS_OK;
}

static C_INT command_read_register(command_context *debugContext,
    command_register register_id, type_unsigned_32 *value)
{
    x86_debug_response result;
    if (value == STD_NULL || command_execute(debugContext,
            &(x86_debug_request){
                .operation = X86_DEBUG_READ_REGISTER,
                .register_id = (lib_u32)register_id },
            &result)) return 1;
    *value = result.value;
    return 0;
}

static C_INT command_write_register(command_context *debugContext,
    command_register register_id, type_unsigned_32 value)
{
    x86_debug_response result;
    return command_execute(debugContext, &(x86_debug_request){
        .operation = X86_DEBUG_WRITE_REGISTER,
        .register_id = (lib_u32)register_id, .address = value }, &result);
}

static C_INT command_access_memory(command_context *debugContext,
    x86_debug_operation operation, type_unsigned_32 address,
    type_unsigned_16 segment, type_unsigned_16 offset, C_VOID *data,
    type_unsigned_8 bytes)
{
    x86_debug_request request = {0};
    x86_debug_response result;
    if (data == STD_NULL || bytes > sizeof(request.data)) {
        debugContext->access_status = LIB_STATUS_INVALID_ARGUMENT;
        debugContext->error_position = 1u;
        return 1;
    }
    request.operation = operation;
    request.address = address;
    request.segment = segment;
    request.offset = offset;
    request.bytes = bytes;
    if (operation == X86_DEBUG_WRITE_LINEAR ||
        operation == X86_DEBUG_WRITE_REAL)
        STD_MEMCPY(request.data, data, bytes);
    if (command_execute(debugContext, &request, &result)) {
        if (operation == X86_DEBUG_READ_LINEAR ||
            operation == X86_DEBUG_READ_REAL) STD_MEMSET(data, 0, bytes);
        return 1;
    }
    if (operation == X86_DEBUG_READ_LINEAR ||
        operation == X86_DEBUG_READ_REAL)
        STD_MEMCPY(data, result.data, bytes);
    return 0;
}

static C_INT command_read_port(command_context *debugContext,
    type_unsigned_16 port, type_unsigned_32 *value)
{
    x86_debug_response result;
    if (command_execute(debugContext, &(x86_debug_request){
            .operation = X86_DEBUG_READ_PORT, .port = port, .bytes = 1u }, &result))
        return 1;
    *value = result.value;
    return 0;
}

static type_unsigned_32 command_read_port_value(command_context *debugContext,
    type_unsigned_16 port)
{
    type_unsigned_32 value = 0u;
    (C_VOID)command_read_port(debugContext, port, &value);
    return value;
}

static C_INT command_write_port(command_context *debugContext,
    type_unsigned_16 port, type_unsigned_32 value)
{
    x86_debug_request request = {0};
    x86_debug_response result;
    request.operation = X86_DEBUG_WRITE_PORT;
    request.bytes = 1u;
    request.port = port;
    request.address = value;
    return command_execute(debugContext, &request, &result);
}

static type_unsigned_32 command_code_property(command_context *debugContext,
    x86_debug_operation operation)
{
    x86_debug_response result;
    return command_execute(debugContext,
        &(x86_debug_request){ .operation = operation }, &result) ? 0u : result.value;
}

static C_INT command_set_watch(command_context *debugContext,
    command_machine_watch_kind kind, type_unsigned_32 address)
{
    x86_debug_response result;
    return command_execute(debugContext, &(x86_debug_request){
        .operation = X86_DEBUG_SET_WATCH, .address = address,
        .watch_kind = kind }, &result);
}

static C_INT command_clear_watch(command_context *debugContext,
    command_machine_watch_kind kind)
{
    x86_debug_response result;
    return command_execute(debugContext, &(x86_debug_request){
        .operation = X86_DEBUG_CLEAR_WATCH, .watch_kind = kind }, &result);
}

static C_INT command_get_watch(command_context *debugContext,
    command_machine_watch_kind kind, type_unsigned_32 *out_address)
{
    x86_debug_response result;

    if (out_address == STD_NULL || command_execute(debugContext,
            &(x86_debug_request){
                .operation = X86_DEBUG_GET_WATCH,
                .watch_kind = kind }, &result)) return -1;
    *out_address = result.value;
    return result.enabled ? 1 : 0;
}

static C_INT command_is_paused(command_context *debugContext)
{
    common_machine_debug_lease lease;
    return debugContext != STD_NULL &&
        common_machine_debug_acquire(debugContext->machine, &lease) == LIB_STATUS_OK;
}

static C_VOID command_resume(command_context *debugContext)
{
    if (debugContext != STD_NULL && debugContext->result != STD_NULL)
        debugContext->result->lifecycle_request = X86_DEBUG_LIFECYCLE_RESUME;
}

static C_INT command_set_break(command_context *debugContext,
    type_unsigned_32 linear)
{
    x86_debug_request request = {0};
    x86_debug_response result;

    if (debugContext == STD_NULL) return 1;
    request.operation = X86_DEBUG_SET_EXECUTION_PLAN;
    request.execution_kind = X86_DEBUG_EXECUTION_BREAK_LINEAR;
    request.address = linear;
    if (command_execute(debugContext, &request, &result)) {
        command_printf(debugContext,
            "debug: fail to install breakpoint at L%08X\n", linear);
        return 1;
    }
    debugContext->breakpoint_linear = linear;
    return 0;
}

static C_VOID command_clear_break(command_context *debugContext)
{
    x86_debug_response result;

    if (debugContext == STD_NULL) return;
    (C_VOID)command_execute(debugContext, &(x86_debug_request) {
        .operation = X86_DEBUG_CLEAR_EXECUTION_PLAN }, &result);
    debugContext->breakpoint_linear = 0u;
    debugContext->breakpoint_remaining = 0u;
}

static C_INT command_set_trace(command_context *debugContext, STD_SIZE_T count)
{
    x86_debug_request request = {0};
    x86_debug_response result;

    if (debugContext == STD_NULL) return 1;
    request.operation = X86_DEBUG_SET_EXECUTION_PLAN;
    request.execution_kind = X86_DEBUG_EXECUTION_TRACE;
    request.instruction_count = count;
    if (command_execute(debugContext, &request, &result)) {
        command_printf(debugContext, "debug: fail to install trace plan.\n");
        return 1;
    }
    return 0;
}

static C_INT command_get_execution_result(command_context *debugContext,
    type_unsigned_32 *out_executed)
{
    x86_debug_response result;

    if (out_executed == STD_NULL || command_execute(debugContext,
            &(x86_debug_request) {
                .operation = X86_DEBUG_GET_EXECUTION_RESULT },
            &result) || !result.enabled) return 1;
    *out_executed = result.value;
    debugContext->observation = result.observation;
    return 0;
}

static C_INT command_begin_trace(command_context *debugContext,
    command_run_kind kind, STD_SIZE_T count)
{
    if (debugContext == STD_NULL || command_set_trace(debugContext,
            count < 0x100u ? 1u : count)) return 1;
    debugContext->run_kind = kind;
    debugContext->trace_remaining = count < 0x100u ? count : 1u;
    return 0;
}

static C_VOID command_begin_break(command_context *debugContext,
    STD_SIZE_T count)
{
    if (debugContext == STD_NULL) return;
    debugContext->run_kind = COMMAND_RUN_BREAK_LINEAR;
    debugContext->breakpoint_remaining = count;
}

static C_INT command_copy_text_checked(C_CHAR *destination,
    STD_SIZE_T destination_capacity, const C_CHAR *source)
{
    STD_SIZE_T source_bytes;

    if (destination == STD_NULL || source == STD_NULL || destination_capacity == 0u) {
        return 0;
    }
    source_bytes = STD_STRLEN(source);
    if (source_bytes >= destination_capacity) return 0;
    STD_MEMCPY(destination, source, source_bytes + 1u);
    return 1;
}

static C_INT command_append_text_checked(C_CHAR *destination,
    STD_SIZE_T destination_capacity, const C_CHAR *source)
{
    STD_SIZE_T destination_bytes;
    STD_SIZE_T source_bytes;

    if (destination == STD_NULL || source == STD_NULL || destination_capacity == 0u) {
        return 0;
    }
    destination_bytes = STD_STRLEN(destination);
    source_bytes = STD_STRLEN(source);
    if (destination_bytes >= destination_capacity ||
        source_bytes >= destination_capacity - destination_bytes) return 0;
    STD_MEMCPY(destination + destination_bytes, source, source_bytes + 1u);
    return 1;
}

static C_VOID rprintregs(command_context *debugContext);
static C_VOID command_print_segments(command_context *debugContext);
static C_VOID command_print_controls(command_context *debugContext);
static C_VOID command_print_watches(command_context *debugContext);

#define nErrPos (debugContext->error_position)
#define narg (debugContext->argument_count)
#define arg (debugContext->arguments)
#define flagExit (debugContext->exit_requested)
#define strCmdBuff (debugContext->command_buffer)
#define strCmdCopy (debugContext->command_copy)
#define strFileName (debugContext->file_name)
#define STD_PRINTF(...) command_printf(debugContext, __VA_ARGS__)
#define command_machine_read_line(buffer, size) command_read_line(debugContext, buffer, size)
#define command_machine_is_running() (!command_is_paused(debugContext))
#define command_machine_resume() command_resume(debugContext)
#define command_machine_is_paused() command_is_paused(debugContext)
#define command_machine_read_register(reg, value) command_read_register(debugContext, reg, value)
#define command_machine_write_register(reg, value) command_write_register(debugContext, reg, value)
#define command_machine_get_code_default_size() command_code_property(debugContext, X86_DEBUG_GET_CODE_DEFAULT_SIZE)
#define command_machine_get_code_base() command_code_property(debugContext, X86_DEBUG_GET_CODE_BASE)
#define command_machine_read_linear(address, out, size) command_access_memory(debugContext, X86_DEBUG_READ_LINEAR, address, 0u, 0u, out, size)
#define command_machine_write_linear(address, in, size) command_access_memory(debugContext, X86_DEBUG_WRITE_LINEAR, address, 0u, 0u, in, size)
#define command_machine_read_real(segment, offset, out, size) command_access_memory(debugContext, X86_DEBUG_READ_REAL, 0u, segment, offset, out, size)
#define command_machine_write_real(segment, offset, in, size) command_access_memory(debugContext, X86_DEBUG_WRITE_REAL, 0u, segment, offset, in, size)
#define command_machine_read_port(port) command_read_port_value(debugContext, port)
#define command_machine_write_port(port, value) command_write_port(debugContext, port, value)
#define command_machine_set_break_real(segment, offset) command_set_break(debugContext, ((type_unsigned_32)(segment) << 4u) + (offset))
#define command_machine_set_break_linear(address) command_set_break(debugContext, address)
#define command_machine_clear_break(linear) command_clear_break(debugContext)
#define command_machine_set_watch(kind, address) command_set_watch(debugContext, kind, address)
#define command_machine_clear_watch(kind) command_clear_watch(debugContext, kind)
#define command_machine_print_segment_registers() command_print_segments(debugContext)
#define command_machine_print_control_registers() command_print_controls(debugContext)
#define command_machine_print_watchpoints() command_print_watches(debugContext)

static type_unsigned_32 debug_register(command_context *debugContext, command_register reg)
{
    type_unsigned_32 value = 0;
    command_machine_read_register(reg, &value);
    return value;
}
static C_INT debug_set_register(command_context *debugContext, command_register reg, type_unsigned_32 value)
{
    return command_machine_write_register(reg, value);
}
static C_INT debug_set_word(command_context *debugContext, command_register reg, type_unsigned_16 value)
{
    /* Original _ax/_ip assignments preserve the upper half of their alias. */
    return debug_set_register(debugContext, reg,
        (debug_register(debugContext, reg) & 0xffff0000u) | value);
}
static C_INT debug_flag(command_context *debugContext, type_unsigned_32 mask)
{
    return (debug_register(debugContext, COMMAND_REGISTER_EFLAGS) & mask) != 0;
}
static C_VOID debug_set_flag(command_context *debugContext, type_unsigned_32 mask, C_INT set)
{
    type_unsigned_32 flags = debug_register(debugContext, COMMAND_REGISTER_EFLAGS);
    debug_set_register(debugContext, COMMAND_REGISTER_EFLAGS, set ? flags | mask : flags & ~mask);
}

static C_INT command_capture_cpu(command_context *debugContext,
    x86_debug_cpu_snapshot *out_snapshot)
{
    x86_debug_response result;

    if (out_snapshot == STD_NULL || command_execute(debugContext,
            &(x86_debug_request){
                .operation = X86_DEBUG_GET_CPU_SNAPSHOT },
            &result)) return 1;
    *out_snapshot = result.cpu;
    return 0;
}

static C_VOID command_print_segment(command_context *debugContext,
    const x86_debug_segment_snapshot *segment, const C_CHAR *label)
{
    command_printf(debugContext, "%s=%04X, Base=%08X, Limit=%08X, DPL=%01X, %s, ",
        label, segment->selector, segment->base, segment->limit, segment->dpl,
        segment->accessed ? "A" : "a");
    if (segment->executable) {
        command_printf(debugContext, "Code, %s, %s, %s\n",
            segment->conform ? "C" : "c",
            segment->readable ? "Rw" : "rw",
            segment->defsize ? "32" : "16");
    } else {
        command_printf(debugContext, "Data, %s, %s, %s\n",
            segment->expdown ? "E" : "e",
            segment->writable ? "RW" : "rw",
            segment->big ? "BIG" : "big");
    }
}

static C_VOID command_print_system_segment(command_context *debugContext,
    const x86_debug_segment_snapshot *segment, const C_CHAR *label)
{
    command_printf(debugContext, "%s=%04X, Base=%08X, Limit=%08X, DPL=%01X, Type=%04X\n",
        label, segment->selector, segment->base, segment->limit, segment->dpl,
        segment->type);
}

static C_VOID command_print_segments(command_context *debugContext)
{
    x86_debug_cpu_snapshot snapshot;

    if (command_capture_cpu(debugContext, &snapshot)) return;
    command_print_segment(debugContext, &snapshot.es, "ES");
    command_print_segment(debugContext, &snapshot.cs, "CS");
    command_print_segment(debugContext, &snapshot.ss, "SS");
    command_print_segment(debugContext, &snapshot.ds, "DS");
    command_print_segment(debugContext, &snapshot.fs, "FS");
    command_print_segment(debugContext, &snapshot.gs, "GS");
    command_print_system_segment(debugContext, &snapshot.tr, "TR  ");
    command_printf(debugContext, "LDTR=%04X, Base=%08X, Limit=%08X\n",
        snapshot.ldtr.selector, snapshot.ldtr.base, snapshot.ldtr.limit);
    command_printf(debugContext, "GDTR Base=%08X, Limit=%04X\n",
        snapshot.gdtr.base, snapshot.gdtr.limit);
    command_printf(debugContext, "IDTR Base=%08X, Limit=%04X\n",
        snapshot.idtr.base, snapshot.idtr.limit);
}

static C_VOID command_print_controls(command_context *debugContext)
{
    x86_debug_cpu_snapshot snapshot;

    if (command_capture_cpu(debugContext, &snapshot)) return;
    command_printf(debugContext, "CR0=%08X: %s %s %s %s %s %s\n", snapshot.cr0,
        snapshot.cr0 & 0x80000000u ? "PG" : "pg",
        snapshot.cr0 & 0x00000010u ? "ET" : "et",
        snapshot.cr0 & 0x00000008u ? "TS" : "ts",
        snapshot.cr0 & 0x00000004u ? "EM" : "em",
        snapshot.cr0 & 0x00000002u ? "MP" : "mp",
        snapshot.cr0 & 0x00000001u ? "PE" : "pe");
    command_printf(debugContext, "CR2=PFLR=%08X\n", snapshot.cr2);
    command_printf(debugContext, "CR3=PDBR=%08X\n", snapshot.cr3);
}

static C_VOID command_print_watches(command_context *debugContext)
{
    type_unsigned_32 address;

    if (command_get_watch(debugContext, COMMAND_REGISTER_WATCH_READ,
            &address) > 0)
        command_printf(debugContext, "Watch-read point: Lin=%08x\n", address);
    if (command_get_watch(debugContext, COMMAND_REGISTER_WATCH_WRITE,
            &address) > 0)
        command_printf(debugContext, "Watch-write point: Lin=%08x\n", address);
    if (command_get_watch(debugContext, COMMAND_REGISTER_WATCH_EXECUTE,
            &address) > 0)
        command_printf(debugContext, "Watch-exec point: Lin=%08x\n", address);
}

static C_VOID command_print_memory_accesses(command_context *debugContext)
{
    type_unsigned_8 index;

    for (index = 0u; index < debugContext->observation.count; ++index) {
        const x86_debug_memory_access *access =
            &debugContext->observation.accesses[index];
        command_printf(debugContext, "%s: Lin=%08x, Data=%08x%08x, Bytes=%x\n",
            access->write ? "Write" : "Read", access->linear,
            (type_unsigned_32)(access->data >> 32u),
            (type_unsigned_32)access->data, access->bytes);
    }
    if (debugContext->observation.truncated)
        command_printf(debugContext, "Additional accesses omitted (record capacity).\n");
}

#define _eax debug_register(debugContext, COMMAND_REGISTER_EAX)
#define _ecx debug_register(debugContext, COMMAND_REGISTER_ECX)
#define _edx debug_register(debugContext, COMMAND_REGISTER_EDX)
#define _ebx debug_register(debugContext, COMMAND_REGISTER_EBX)
#define _esp debug_register(debugContext, COMMAND_REGISTER_ESP)
#define _ebp debug_register(debugContext, COMMAND_REGISTER_EBP)
#define _esi debug_register(debugContext, COMMAND_REGISTER_ESI)
#define _edi debug_register(debugContext, COMMAND_REGISTER_EDI)
#define _eflags debug_register(debugContext, COMMAND_REGISTER_EFLAGS)
#define _eip debug_register(debugContext, COMMAND_REGISTER_EIP)
#define _cr(i) debug_register(debugContext, (command_register)(COMMAND_REGISTER_CR0 + (i)))
#define _ax ((type_unsigned_16)_eax)
#define _cx ((type_unsigned_16)_ecx)
#define _dx ((type_unsigned_16)_edx)
#define _bx ((type_unsigned_16)_ebx)
#define _sp ((type_unsigned_16)_esp)
#define _bp ((type_unsigned_16)_ebp)
#define _si ((type_unsigned_16)_esi)
#define _di ((type_unsigned_16)_edi)
#define _ip ((type_unsigned_16)_eip)
#define _es ((type_unsigned_16)debug_register(debugContext, COMMAND_REGISTER_ES))
#define _cs ((type_unsigned_16)debug_register(debugContext, COMMAND_REGISTER_CS))
#define _ss ((type_unsigned_16)debug_register(debugContext, COMMAND_REGISTER_SS))
#define _ds ((type_unsigned_16)debug_register(debugContext, COMMAND_REGISTER_DS))
#define _fs ((type_unsigned_16)debug_register(debugContext, COMMAND_REGISTER_FS))
#define _gs ((type_unsigned_16)debug_register(debugContext, COMMAND_REGISTER_GS))

static C_VOID seterr(command_context *debugContext, STD_SIZE_T pos)
{
    nErrPos = (STD_SIZE_T)(arg[pos] - strCmdCopy + STD_STRLEN(arg[pos]) + 1);
}
static type_unsigned_8 scannubit8(command_context *debugContext, C_CHAR *s)
{
    type_unsigned_8 ans = 0;
    STD_SIZE_T i = 0;
    if (STD_STRLEN(s) == 3u && s[0] == '\'' && s[2] == '\'')
    {
        return s[1];
    }
    type_string_lower(s);
    while (s[i] != '\0' && s[i] != '\n')
    {
        if (i > 1)
        {
            seterr(debugContext, narg - 1);
            break;
        }
        ans <<= 4;
        if (s[i] > 0x2f && s[i] < 0x3a)
        {
            ans += s[i] - 0x30;
        }
        else if (s[i] > 0x60 && s[i] < 0x67)
        {
            ans += s[i] - 0x57;
        }
        else
        {
            seterr(debugContext, narg - 1);
        }
        ++i;
    }
    return ans;
}
static type_unsigned_16 scannubit16(command_context *debugContext, C_CHAR *s)
{
    type_unsigned_16 ans = 0;
    STD_SIZE_T i = 0;
    type_string_lower(s);
    while (s[i] != '\0' && s[i] != '\n')
    {
        if (i > 3)
        {
            seterr(debugContext, narg - 1);
            break;
        }
        ans <<= 4;
        if (s[i] > 0x2f && s[i] < 0x3a)
        {
            ans += s[i] - 0x30;
        }
        else if (s[i] > 0x60 && s[i] < 0x67)
        {
            ans += s[i] - 0x57;
        }
        else
        {
            seterr(debugContext, narg - 1);
        }
        ++i;
    }
    return ans;
}
static type_unsigned_32 scannubit32(command_context *debugContext, C_CHAR *s)
{
    type_unsigned_32 ans = 0;
    STD_SIZE_T i = 0;
    type_string_lower(s);
    while (s[i] != '\0' && s[i] != '\n')
    {
        if (i > 7)
        {
            seterr(debugContext, narg - 1);
            break;
        }
        ans <<= 4;
        if (s[i] > 0x2f && s[i] < 0x3a)
        {
            ans += s[i] - 0x30;
        }
        else if (s[i] > 0x60 && s[i] < 0x67)
        {
            ans += s[i] - 0x57;
        }
        else
        {
            seterr(debugContext, narg - 1);
        }
        ++i;
    }
    return ans;
}

#define dumpSegRec (debugContext->dump_segment)
#define dumpPtrRec (debugContext->dump_offset)
#define asmSegRec (debugContext->assemble_segment)
#define asmPtrRec (debugContext->assemble_offset)
#define uasmSegRec (debugContext->unassemble_segment)
#define uasmPtrRec (debugContext->unassemble_offset)
#define seg (debugContext->parsed_segment)
#define ptr (debugContext->parsed_offset)

static C_VOID addrparse(command_context *debugContext, type_unsigned_16 defseg, const C_CHAR *addr)
{
    C_CHAR *colon;
    C_CHAR copy[0x100];
    if (!command_copy_text_checked(copy, sizeof(copy), addr)) return;
    type_string_lower(copy);
    colon = lib_c_strchr(copy, ':');
    seg = defseg;
    if (colon != STD_NULL)
    {
        *colon++ = '\0';
        if (copy[0] == '\0' || colon[0] == '\0' || lib_c_strchr(colon, ':'))
        {
            seterr(debugContext, narg - 1u);
            return;
        }
        if (!STD_STRCMP(copy, "es")) seg = _es;
        else if (!STD_STRCMP(copy, "cs")) seg = _cs;
        else if (!STD_STRCMP(copy, "ss")) seg = _ss;
        else if (!STD_STRCMP(copy, "ds")) seg = _ds;
        else seg = scannubit16(debugContext, copy);
    }
    ptr = scannubit16(debugContext, colon != STD_NULL ? colon : copy);
}

/* Consume a real range, leaving the next argument for a destination/list.
 * A 32-bit count represents the complete 64 KiB segment without wrapping. */
static type_unsigned_32 scanrange(command_context *debugContext,
    type_unsigned_16 default_segment, type_unsigned_32 default_count,
    STD_SIZE_T *next)
{
    type_unsigned_32 count, end;
    STD_SIZE_T i = 2u;
    if (narg < 2u) { seterr(debugContext, 0u); return 0u; }
    addrparse(debugContext, default_segment, arg[1]);
    count = default_count;
    if (count > 0x10000u - ptr) count = 0x10000u - ptr;
    if (i < narg)
    {
        if (arg[i][0] == 'l')
        {
            C_CHAR *length = arg[i++] + 1;
            if (!*length && i < narg) length = arg[i++];
            if (!*length) { seterr(debugContext, i - 1u); return 0u; }
            count = scannubit16(debugContext, length);
            if (count == 0u) count = 0x10000u;
        }
        else if ((arg[i][0] >= '0' && arg[i][0] <= '9') ||
                 (arg[i][0] >= 'a' && arg[i][0] <= 'f'))
        {
            end = scannubit16(debugContext, arg[i++]);
            if (end < ptr) { seterr(debugContext, i - 1u); return 0u; }
            count = end - ptr + 1u;
        }
    }
    *next = i;
    if (count > 0x10000u - ptr) seterr(debugContext, i - 1u);
    return nErrPos ? 0u : count;
}

/* Validate the entire list before any memory operation; quoted bytes retain
 * case and delimiters. Repeated matching quotes encode one literal quote. */
static STD_SIZE_T scanlist(command_context *debugContext, STD_SIZE_T first,
    type_unsigned_8 *bytes)
{
    STD_SIZE_T count = 0u, i;
    for (i = first; i < narg; ++i)
    {
        C_CHAR *p = arg[i];
        if (*p == '\'' || *p == '"')
        {
            C_CHAR quote = *p++;
            while (*p)
            {
                if (*p == quote)
                {
                    if (p[1] != quote) break;
                    ++p;
                }
                bytes[count++] = (type_unsigned_8)*p++;
            }
            if (*p != quote || p[1] != '\0')
            {
                seterr(debugContext, i);
                return 0u;
            }
        }
        else bytes[count++] = scannubit8(debugContext, p);
        if (nErrPos) return 0u;
    }
    if (count == 0u) seterr(debugContext, narg - 1u);
    return count;
}

/* DEBUG CMD BEGIN */
/* assemble */
static C_VOID aconsole(command_context *debugContext)
{
    STD_SIZE_T i, len, errAsmPos;
    C_CHAR cmdAsmBuff[0x100];
    type_unsigned_8 acode[15];
    C_INT flagExitAsm = 0;
    while (!flagExitAsm)
    {
        STD_PRINTF("%04X:%04X ", asmSegRec, asmPtrRec);
        if (!command_machine_read_line(cmdAsmBuff, sizeof(cmdAsmBuff))) return;
        type_string_lower(cmdAsmBuff);
        if (!STD_STRLEN(cmdAsmBuff))
        {
            flagExitAsm = 1;
            continue;
        }
        if (cmdAsmBuff[0] == ';')
        {
            continue;
        }
        errAsmPos = 0;
        if (x86_xasm32_assemble(cmdAsmBuff, STD_STRLEN(cmdAsmBuff),
                acode, sizeof(acode), &len,
                command_machine_get_code_default_size()) != TYPE_STATUS_OK) {
            len = 0u;
        }
        if (!len)
        {
            errAsmPos = STD_STRLEN(cmdAsmBuff) + 9;
        }
        else
        {
            if (command_machine_write_linear((asmSegRec << 4) + asmPtrRec, (C_VOID *)acode, (type_unsigned_8)len))
            {
                STD_PRINTF("debug: fail to write to L%08X\n", (asmSegRec << 4) + asmPtrRec);
                return;
            }
            asmPtrRec += (type_unsigned_16)len;
        }
        if (errAsmPos)
        {
            for (i = 0; i < errAsmPos; ++i)
            {
                STD_PRINTF(" ");
            }
            STD_PRINTF("^ Error\n");
        }
    }
}
static C_VOID a(command_context *debugContext)
{
    if (narg == 1)
    {
        aconsole(debugContext);
    }
    else if (narg == 2)
    {
        addrparse(debugContext, _cs, arg[1]);
        if (nErrPos)
        {
            return;
        }
        asmSegRec = seg;
        asmPtrRec = ptr;
        aconsole(debugContext);
    }
    else
    {
        seterr(debugContext, 2);
    }
}
/* compare */
static C_VOID c(command_context *debugContext)
{
    STD_SIZE_T next;
    type_unsigned_32 i, count = scanrange(debugContext, _ds, 128u, &next);
    type_unsigned_16 seg1 = seg, ptr1 = ptr;
    type_unsigned_8 val1, val2;
    if (nErrPos) return;
    if (next + 1u != narg) { seterr(debugContext, narg - 1u); return; }
    addrparse(debugContext, _ds, arg[next]);
    if (nErrPos) return;
    for (i = 0u; i < count; ++i)
    {
        if (command_machine_read_real(seg1, (type_unsigned_16)(ptr1 + i), &val1, 1) ||
            command_machine_read_real(seg, (type_unsigned_16)(ptr + i), &val2, 1)) return;
        if (val1 != val2)
        {
            STD_PRINTF("%04X:%04X  ", seg1, (type_unsigned_16)(ptr1 + i));
            STD_PRINTF("%02X  %02X", val1, val2);
            STD_PRINTF("  %04X:%04X\n", seg, (type_unsigned_16)(ptr + i));
        }
    }
}

/* dump */
static C_VOID dprint(command_context *debugContext, type_unsigned_16 segment, type_unsigned_16 start, type_unsigned_16 end)
{
    C_CHAR t, c[0x11];
    type_unsigned_16 iaddr;
    if (start > end)
        end = 0xffff;
    c[0x10] = '\0';
    if (end < start)
    {
        end = 0xffff;
    }
    for (iaddr = start - (start % 0x10); iaddr <= end + (0x10 - end % 0x10) - 1; ++iaddr)
    {
        if (iaddr % 0x10 == 0)
        {
            STD_PRINTF("%04X:%04X  ", segment, iaddr);
        }
        if (iaddr < start || iaddr > end)
        {
            STD_PRINTF("  ");
            c[iaddr % 0x10] = ' ';
        }
        else
        {
            command_machine_read_real(segment, iaddr, (C_VOID *)(&c[iaddr % 0x10]), 1);
            STD_PRINTF("%02X", c[iaddr % 0x10] & 0xff);
            t = c[iaddr % 0x10];
            if ((t >= 1 && t <= 7) || t == ' ' ||
                (t >= 11 && t <= 12) ||
                (t >= 14 && t <= 31) ||
                (t >= 33))
            {
            }
            else
            {
                c[iaddr % 0x10] = '.';
            }
        }
        STD_PRINTF(" ");
        if (iaddr % 0x10 == 7 && iaddr >= start && iaddr < end)
        {
            STD_PRINTF("\b-");
        }
        if ((iaddr + 1) % 0x10 == 0)
        {
            STD_PRINTF("  %s\n", c);
        }
        if (iaddr == 0xffff)
        {
            break;
        }
    }
    dumpSegRec = segment;
    dumpPtrRec = end + 1;
}
static C_VOID d(command_context *debugContext)
{
    STD_SIZE_T next;
    type_unsigned_32 count;
    if (narg == 1u)
        dprint(debugContext, dumpSegRec, dumpPtrRec,
            (type_unsigned_16)(dumpPtrRec + 0x7fu));
    else
    {
        count = scanrange(debugContext, _ds, 128u, &next);
        if (nErrPos) return;
        if (next != narg) { seterr(debugContext, next); return; }
        dprint(debugContext, seg, ptr, (type_unsigned_16)(ptr + count - 1u));
    }
}

/* enter */
static C_VOID e(command_context *debugContext)
{
    STD_SIZE_T i;
    type_unsigned_8 val;
    C_CHAR s[0x100];
    if (narg == 1)
    {
        seterr(debugContext, 0);
    }
    else if (narg == 2)
    {
        addrparse(debugContext, _ds, arg[1]);
        if (nErrPos)
        {
            return;
        }
        STD_PRINTF("%04X:%04X  ", seg, ptr);
        command_machine_read_real(seg, ptr, (C_VOID *)(&val), 1);
        STD_PRINTF("%02X", val);
        STD_PRINTF(".");
        if (!command_machine_read_line(s, sizeof(s))) return;
        type_string_lower(s);              /* MARK */
        val = scannubit8(debugContext, s); /* MARK */
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            command_machine_write_real(seg, ptr, (C_VOID *)(&val), 1);
        }
    }
    else
    {
        type_unsigned_8 bytes[X86_DEBUG_LINE_CAPACITY];
        STD_SIZE_T count;
        addrparse(debugContext, _ds, arg[1]);
        count = scanlist(debugContext, 2u, bytes);
        if (nErrPos) return;
        for (i = 0u; i < count; ++i)
            if (command_machine_write_real(seg, (type_unsigned_16)(ptr + i),
                    &bytes[i], 1)) return;
    }
}
/* fill */
static C_VOID f(command_context *debugContext)
{
    STD_SIZE_T next, length;
    type_unsigned_32 i, count = scanrange(debugContext, _ds, 128u, &next);
    type_unsigned_8 bytes[X86_DEBUG_LINE_CAPACITY];
    if (nErrPos) return;
    length = scanlist(debugContext, next, bytes);
    if (nErrPos) return;
    for (i = 0u; i < count; ++i)
        if (command_machine_write_real(seg, (type_unsigned_16)(ptr + i),
                &bytes[i % length], 1)) return;
}

/* go */
static C_VOID rprintregs(command_context *debugContext);
static C_VOID g(command_context *debugContext)
{
    type_unsigned_16 start_segment, start_offset;
    if (command_machine_is_running())
    {
        STD_PRINTF("Machine is already running.\n");
        return;
    }
    switch (narg)
    {
    case 1:
        command_machine_clear_break(0);
        break;
    case 2:
        addrparse(debugContext, _cs, arg[1]);
        if (nErrPos) return;
        if (command_machine_set_break_real(seg, ptr)) return;
        break;
    case 3:
        addrparse(debugContext, _cs, arg[1]);
        start_segment = seg; start_offset = ptr;
        addrparse(debugContext, start_segment, arg[2]);
        if (nErrPos) return;
        if (debug_set_register(debugContext, COMMAND_REGISTER_CS, start_segment))
        {
            STD_PRINTF("debug: fail to load cs from %04X\n", start_segment);
            return;
        }
        if (debug_set_word(debugContext, COMMAND_REGISTER_EIP, start_offset)) return;
        if (command_machine_set_break_real(seg, ptr)) return;
        break;
    default:
        seterr(debugContext, narg - 1);
        break;
    }
    if (nErrPos)
    {
        return;
    }
    debugContext->run_kind = COMMAND_RUN_BREAK_REAL;
    command_machine_resume();
}
/* hex */
static C_VOID h(command_context *debugContext)
{
    type_unsigned_16 val1, val2;
    if (narg != 3)
    {
        seterr(debugContext, narg - 1);
    }
    else
    {
        val1 = scannubit16(debugContext, arg[1]);
        val2 = scannubit16(debugContext, arg[2]);
        if (!nErrPos)
        {
            STD_PRINTF("%04X", (type_unsigned_16)(val1 + val2));
            STD_PRINTF("  ");
            STD_PRINTF("%04X", (type_unsigned_16)(val1 - val2));
            STD_PRINTF("\n");
        }
    }
}
/* input */
static C_VOID i(command_context *debugContext)
{
    type_unsigned_16 in;
    if (narg != 2)
    {
        seterr(debugContext, narg - 1);
    }
    else
    {
        in = scannubit16(debugContext, arg[1]);
        if (nErrPos)
        {
            return;
        }
        STD_PRINTF("%08X\n", command_machine_read_port(in));
    }
}
/* load */
static C_VOID l(command_context *debugContext)
{
    type_unsigned_16 i = 0;
    type_unsigned_32 len = 0;
    lib_storage_medium *medium = LIB_NULL;
    lib_size bytes;
    lib_size total;
    lib_status status;
    type_unsigned_8 data[X86_DEBUG_BYTES];

    if (!STD_STRLEN(strFileName)) {
        STD_PRINTF("File not found\n");
        return;
    }
    switch (narg) {
    case 1: seg = _cs; ptr = 0x100; break;
    case 2: addrparse(debugContext, _cs, arg[1]); break;
    default: seterr(debugContext, narg - 1); break;
    }
    if (nErrPos) return;
    status = lib_storage_medium_open(strFileName, LIB_STORAGE_MEDIUM_READONLY,
        &medium);
    total = status == LIB_STATUS_OK ? lib_storage_medium_byte_count(medium) : 0u;
    while (!nErrPos && debugContext->access_status == LIB_STATUS_OK &&
        status == LIB_STATUS_OK && len < total) {
        lib_size index;

        bytes = total - len < sizeof(data) ? total - len : sizeof(data);
        status = lib_storage_medium_read_at(medium, len, data, bytes);
        if (status != LIB_STATUS_OK) break;
        for (index = 0u; index < bytes; ++index) {
            command_machine_write_real(seg + i, ptr + len++, &data[index], 1);
            i = (type_unsigned_16)(len / 0x10000u);
        }
    }
    lib_storage_medium_destroy(&medium);
    if (status != LIB_STATUS_OK) STD_PRINTF("File not found\n");
    debug_set_word(debugContext, COMMAND_REGISTER_ECX, (type_unsigned_16)(len & 0xffffu));
    debug_set_word(debugContext, COMMAND_REGISTER_EBX, (type_unsigned_16)(len >> 16u));
}
/* move */
static C_VOID m(command_context *debugContext)
{
    STD_SIZE_T next;
    type_unsigned_32 i, offset, count = scanrange(debugContext, _ds, 128u, &next);
    type_unsigned_16 seg1 = seg, ptr1 = ptr;
    type_unsigned_8 val;
    C_INT backward;
    if (nErrPos) return;
    if (next + 1u != narg) { seterr(debugContext, narg - 1u); return; }
    addrparse(debugContext, _ds, arg[next]);
    if (nErrPos) return;
    if (((seg1 << 4) + ptr1) == ((seg << 4) + ptr)) return;
    backward = ((seg1 << 4) + ptr1) < ((seg << 4) + ptr);
    for (i = 0u; i < count; ++i)
    {
        offset = backward ? count - 1u - i : i;
        if (command_machine_read_real(seg1, (type_unsigned_16)(ptr1 + offset), &val, 1) ||
            command_machine_write_real(seg, (type_unsigned_16)(ptr + offset), &val, 1)) return;
    }
}

/* name */
static C_VOID n(command_context *debugContext)
{
    if (narg != 2)
        seterr(debugContext, narg - 1);
    else
        if (!command_copy_text_checked(strFileName, sizeof(strFileName),
                arg[1])) seterr(debugContext, 1u);
}
/* output */
static C_VOID o(command_context *debugContext)
{
    type_unsigned_16 out;
    type_unsigned_32 value;
    if (narg != 3)
        seterr(debugContext, narg - 1);
    else
    {
        out = scannubit16(debugContext, arg[1]);
        if (nErrPos)
            return;
        value = scannubit32(debugContext, arg[2]);
        if (nErrPos)
            return;
        command_machine_write_port(out, value);
    }
}
/* quit */
static C_VOID q(command_context *debugContext)
{
    flagExit = 1;
}
/* register */
static type_unsigned_8 uprintins(command_context *debugContext, type_unsigned_16 segment, type_unsigned_16 off)
{
    STD_SIZE_T i;
    type_unsigned_8 first = off > 0xfff1u ? (type_unsigned_8)(0x10000u - off) : 15u;
    STD_SIZE_T sbin_remaining;
    C_INT binary_failed = TYPE_FALSE;
    C_INT format_result;
    type_unsigned_8 len;
    type_unsigned_8 ucode[15];
    C_CHAR str[0x100], stmt[0x100], sbin[0x100];
    C_CHAR *sbin_cursor;
    if (command_machine_read_real(segment, off, ucode, first) ||
        (first < 15u && command_machine_read_real(segment, 0u, ucode + first, 15u - first)))
    {
        len = 0;
        (C_VOID)STD_SNPRINTF(str, sizeof(str), "%04X:%04X <ERROR>", segment, off);
    }
    else
    {
        lib_size instruction_bytes = 0u;
        if (x86_xasm32_disassemble(ucode, sizeof(ucode), stmt,
                sizeof(stmt), &i, &instruction_bytes,
                0) != TYPE_STATUS_OK) {
            len = 0u;
            (void)lib_c_snprintf(stmt, sizeof(stmt), "<ERROR>");
        } else {
            len = (type_unsigned_8)instruction_bytes;
        }
        sbin[0] = 0;
        sbin_cursor = sbin;
        sbin_remaining = sizeof(sbin);
        for (i = 0; i < len; ++i)
        {
            format_result = STD_SNPRINTF_APPEND(&sbin_cursor, &sbin_remaining,
                "%02X", (type_unsigned_8)ucode[i]);
            if (format_result < 0) {
                binary_failed = TYPE_TRUE;
                len = 0;
                (C_VOID)STD_SNPRINTF(str, sizeof(str), "%04X:%04X <ERROR>",
                    segment, off);
                break;
            }
        }
        if (!binary_failed) {
            (C_VOID)STD_SNPRINTF(str, sizeof(str), "%04X:%04X %s", segment, off,
                sbin);
        }
        for (i = STD_STRLEN(str); i < 24; ++i)
        {
            if (!command_append_text_checked(str, sizeof(str), " ")) {
                len = 0u;
                (C_VOID)STD_SNPRINTF(str, sizeof(str), "%04X:%04X <ERROR>",
                    segment, off);
                break;
            }
        }
        if (!command_append_text_checked(str, sizeof(str), stmt)) {
            len = 0u;
            (C_VOID)STD_SNPRINTF(str, sizeof(str), "%04X:%04X <ERROR>",
                segment, off);
        }
    }
    STD_PRINTF("%s\n", str);
    return len;
}
static C_VOID rprintflags(command_context *debugContext)
{
    STD_PRINTF("%s ", debug_flag(debugContext, 0x0800u) ? "OV" : "NV");
    STD_PRINTF("%s ", debug_flag(debugContext, 0x0400u) ? "DN" : "UP");
    STD_PRINTF("%s ", debug_flag(debugContext, 0x0200u) ? "EI" : "DI");
    STD_PRINTF("%s ", debug_flag(debugContext, 0x0080u) ? "NG" : "PL");
    STD_PRINTF("%s ", debug_flag(debugContext, 0x0040u) ? "ZR" : "NZ");
    STD_PRINTF("%s ", debug_flag(debugContext, 0x0010u) ? "AC" : "NA");
    STD_PRINTF("%s ", debug_flag(debugContext, 0x0004u) ? "PE" : "PO");
    STD_PRINTF("%s ", debug_flag(debugContext, 0x0001u) ? "CY" : "NC");
}
static C_VOID rprintregs(command_context *debugContext)
{
    STD_PRINTF("AX=%04X", _ax);
    STD_PRINTF("  BX=%04X", _bx);
    STD_PRINTF("  CX=%04X", _cx);
    STD_PRINTF("  DX=%04X", _dx);
    STD_PRINTF("  SP=%04X", _sp);
    STD_PRINTF("  BP=%04X", _bp);
    STD_PRINTF("  SI=%04X", _si);
    STD_PRINTF("  DI=%04X", _di);
    STD_PRINTF("\nDS=%04X", _ds);
    STD_PRINTF("  ES=%04X", _es);
    STD_PRINTF("  SS=%04X", _ss);
    STD_PRINTF("  CS=%04X", _cs);
    STD_PRINTF("  IP=%04X", _ip);
    STD_PRINTF("   ");
    rprintflags(debugContext);
    STD_PRINTF("\n");
    uprintins(debugContext, _cs, _ip);
    uasmSegRec = _cs;
    uasmPtrRec = _ip;
}
static C_VOID rscanregs(command_context *debugContext)
{
    type_unsigned_16 value;
    C_CHAR s[0x100];
    if (!STD_STRCMP(arg[1], "ax"))
    {
        STD_PRINTF("AX ");
        STD_PRINTF("%04X", _ax);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_word(debugContext, COMMAND_REGISTER_EAX, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "bx"))
    {
        STD_PRINTF("BX ");
        STD_PRINTF("%04X", _bx);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_word(debugContext, COMMAND_REGISTER_EBX, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "cx"))
    {
        STD_PRINTF("CX ");
        STD_PRINTF("%04X", _cx);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_word(debugContext, COMMAND_REGISTER_ECX, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "dx"))
    {
        STD_PRINTF("DX ");
        STD_PRINTF("%04X", _dx);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_word(debugContext, COMMAND_REGISTER_EDX, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "bp"))
    {
        STD_PRINTF("BP ");
        STD_PRINTF("%04X", _bp);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_word(debugContext, COMMAND_REGISTER_EBP, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "sp"))
    {
        STD_PRINTF("SP ");
        STD_PRINTF("%04X", _sp);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_word(debugContext, COMMAND_REGISTER_ESP, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "si"))
    {
        STD_PRINTF("SI ");
        STD_PRINTF("%04X", _si);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_word(debugContext, COMMAND_REGISTER_ESI, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "di"))
    {
        STD_PRINTF("DI ");
        STD_PRINTF("%04X", _di);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_word(debugContext, COMMAND_REGISTER_EDI, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "ss"))
    {
        STD_PRINTF("SS ");
        STD_PRINTF("%04X", _ss);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, COMMAND_REGISTER_SS, (type_unsigned_16)value))
            {
                STD_PRINTF("debug: fail to load ss from %04X\n", (type_unsigned_16)value);
            }
        }
    }
    else if (!STD_STRCMP(arg[1], "cs"))
    {
        STD_PRINTF("CS ");
        STD_PRINTF("%04X", _cs);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, COMMAND_REGISTER_CS, (type_unsigned_16)value))
            {
                STD_PRINTF("debug: fail to load cs from %04X\n", (type_unsigned_16)value);
            }
        }
    }
    else if (!STD_STRCMP(arg[1], "ds"))
    {
        STD_PRINTF("DS ");
        STD_PRINTF("%04X", _ds);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, COMMAND_REGISTER_DS, (type_unsigned_16)value))
            {
                STD_PRINTF("debug: fail to load ds from %04X\n", (type_unsigned_16)value);
            }
        }
    }
    else if (!STD_STRCMP(arg[1], "es"))
    {
        STD_PRINTF("ES ");
        STD_PRINTF("%04X", _es);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, COMMAND_REGISTER_ES, (type_unsigned_16)value))
            {
                STD_PRINTF("debug: fail to load es from %04X\n", (type_unsigned_16)value);
            }
        }
    }
    else if (!STD_STRCMP(arg[1], "ip"))
    {
        STD_PRINTF("IP ");
        STD_PRINTF("%04X", _ip);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_word(debugContext, COMMAND_REGISTER_EIP, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "f"))
    {
        rprintflags(debugContext);
        STD_PRINTF(" -");
        if (!command_machine_read_line(s, sizeof(s))) return;
        type_string_lower(s);
        if (!STD_STRCMP(s, "ov"))
        {
            debug_set_flag(debugContext, 0x0800u, 1);
        }
        else if (!STD_STRCMP(s, "nv"))
        {
            debug_set_flag(debugContext, 0x0800u, 0);
        }
        else if (!STD_STRCMP(s, "dn"))
        {
            debug_set_flag(debugContext, 0x0400u, 1);
        }
        else if (!STD_STRCMP(s, "up"))
        {
            debug_set_flag(debugContext, 0x0400u, 0);
        }
        else if (!STD_STRCMP(s, "ei"))
        {
            debug_set_flag(debugContext, 0x0200u, 1);
        }
        else if (!STD_STRCMP(s, "di"))
        {
            debug_set_flag(debugContext, 0x0200u, 0);
        }
        else if (!STD_STRCMP(s, "ng"))
        {
            debug_set_flag(debugContext, 0x0080u, 1);
        }
        else if (!STD_STRCMP(s, "pl"))
        {
            debug_set_flag(debugContext, 0x0080u, 0);
        }
        else if (!STD_STRCMP(s, "zr"))
        {
            debug_set_flag(debugContext, 0x0040u, 1);
        }
        else if (!STD_STRCMP(s, "nz"))
        {
            debug_set_flag(debugContext, 0x0040u, 0);
        }
        else if (!STD_STRCMP(s, "ac"))
        {
            debug_set_flag(debugContext, 0x0010u, 1);
        }
        else if (!STD_STRCMP(s, "na"))
        {
            debug_set_flag(debugContext, 0x0010u, 0);
        }
        else if (!STD_STRCMP(s, "pe"))
        {
            debug_set_flag(debugContext, 0x0004u, 1);
        }
        else if (!STD_STRCMP(s, "po"))
        {
            debug_set_flag(debugContext, 0x0004u, 0);
        }
        else if (!STD_STRCMP(s, "cy"))
        {
            debug_set_flag(debugContext, 0x0001u, 1);
        }
        else if (!STD_STRCMP(s, "nc"))
        {
            debug_set_flag(debugContext, 0x0001u, 0);
        }
        else
        {
            STD_PRINTF("bf Error\n");
        }
    }
    else
    {
        STD_PRINTF("br Error\n");
    }
}
static C_VOID r(command_context *debugContext)
{
    if (narg == 1)
    {
        rprintregs(debugContext);
    }
    else if (narg == 2)
    {
        rscanregs(debugContext);
    }
    else
        seterr(debugContext, 2);
}
/* search */
static C_VOID s(command_context *debugContext)
{
    STD_SIZE_T next, length, j;
    type_unsigned_32 i, count = scanrange(debugContext, _ds, 128u, &next);
    type_unsigned_8 bytes[X86_DEBUG_LINE_CAPACITY], val;
    if (nErrPos) return;
    length = scanlist(debugContext, next, bytes);
    if (nErrPos || count < length) return;
    for (i = 0u; i <= count - length; ++i)
    {
        for (j = 0u; j < length; ++j)
        {
            if (command_machine_read_real(seg, (type_unsigned_16)(ptr + i + j), &val, 1)) return;
            if (val != bytes[j]) break;
        }
        if (j == length) STD_PRINTF("%04X:%04X  \n", seg, (type_unsigned_16)(ptr + i));
    }
}

/* trace */
static C_VOID t(command_context *debugContext)
{
    type_unsigned_16 count;
    if (command_machine_is_running())
    {
        STD_PRINTF("Machine is already running.\n");
        return;
    }
    switch (narg)
    {
    case 1:
        count = 1;
        break;
    case 2:
        count = scannubit16(debugContext, arg[1]);
        break;
    case 3:
        addrparse(debugContext, _cs, arg[1]);
        count = scannubit16(debugContext, arg[2]);
        if (nErrPos) return;
        if (debug_set_register(debugContext, COMMAND_REGISTER_CS, seg))
        {
            STD_PRINTF("debug: fail to load cs from %04X\n", seg);
            return;
        }
        debug_set_word(debugContext, COMMAND_REGISTER_EIP, ptr);
        break;
    default:
        seterr(debugContext, narg - 1);
        return;
    }
    if (nErrPos)
    {
        return;
    }
    /* The original loop synchronously waited after each instruction.  The
     * common/session boundary is asynchronous: one trace budget and one
     * resume request preserve the requested stop point without polling. */
    if (count == 0u) return;
    if (command_begin_trace(debugContext, COMMAND_RUN_TRACE_REAL, count)) return;
    command_machine_resume();
}
/* unassemble */
static C_VOID uprint(command_context *debugContext, type_unsigned_16 segment, type_unsigned_16 start, type_unsigned_16 end)
{
    type_unsigned_8 len;
    type_unsigned_32 boundary;
    if (start > end)
    {
        end = 0xffff;
    }
    while (start <= end)
    {
        len = uprintins(debugContext, segment, start);
        if (len == 0u) break;
        boundary = (type_unsigned_32)start + (type_unsigned_32)len;
        start = (type_unsigned_16)boundary;
        if (boundary > 0xffff)
        {
            break;
        }
    }
    uasmSegRec = segment;
    uasmPtrRec = start;
    return;
}
static C_VOID u(command_context *debugContext)
{
    STD_SIZE_T next;
    type_unsigned_32 count;
    if (narg == 1u)
        uprint(debugContext, uasmSegRec, uasmPtrRec,
            (type_unsigned_16)(uasmPtrRec + 0x1fu));
    else
    {
        count = scanrange(debugContext, _cs, 32u, &next);
        if (nErrPos) return;
        if (next != narg) { seterr(debugContext, next); return; }
        uprint(debugContext, seg, ptr, (type_unsigned_16)(ptr + count - 1u));
    }
}

/* verbal */
static C_VOID v(command_context *debugContext)
{
    STD_SIZE_T i;
    C_CHAR str[0x100];
    STD_PRINTF(":");
    if (!command_machine_read_line(str, sizeof(str))) return;
    if (STD_STRLEN(str) != 0u && str[STD_STRLEN(str) - 1u] == '\n')
        str[STD_STRLEN(str) - 1u] = '\0';
    for (i = 0; i < STD_STRLEN(str); ++i)
    {
        STD_PRINTF("%02X", str[i]);
        if (!((i + 1) % 0x10))
        {
            STD_PRINTF("\n");
        }
        else if (!((i + 1) % 0x08) && (str[i + 1] != '\0'))
        {
            STD_PRINTF("-");
        }
        else
        {
            STD_PRINTF(" ");
        }
    }
    if (i % 0x10)
    {
        STD_PRINTF("\n");
    }
}
/* write */
static C_VOID w(command_context *debugContext)
{
    STD_SIZE_T i = 0;
    type_unsigned_32 len = (_bx << 16) + _cx;
    type_unsigned_8 data[X86_DEBUG_BYTES];
    lib_storage_file_writer *writer = LIB_NULL;
    lib_status status;
    if (!STD_STRLEN(strFileName))
    {
        STD_PRINTF("(W)rite error, no destination defined\n");
        return;
    }
    switch (narg)
    {
    case 1: seg = _cs; ptr = 0x100; break;
    case 2: addrparse(debugContext, _cs, arg[1]); break;
    default: seterr(debugContext, narg - 1); break;
    }
    if (nErrPos) return;
    status = lib_storage_file_writer_open(strFileName,
        LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer);
    if (status != LIB_STATUS_OK)
    {
        STD_PRINTF("File not found\n");
        return;
    }
    STD_PRINTF("Writing ");
    STD_PRINTF("%04X", _bx);
    STD_PRINTF("%04X", _cx);
    STD_PRINTF(" bytes\n");
    while (i < len) {
        lib_size count = len - i < sizeof(data) ? len - i : sizeof(data);
        lib_size index;
        for (index = 0u; index < count; ++index)
            command_machine_read_real(seg, (type_unsigned_16)(ptr + i + index), &data[index], 1);
        if (debugContext->access_status != LIB_STATUS_OK) break;
        if (lib_storage_file_writer_write(writer, data, count) !=
                LIB_STATUS_OK) {
            STD_PRINTF("File write failed\n");
            break;
        }
        i += count;
    }
    if (lib_storage_file_writer_close(writer) != LIB_STATUS_OK)
        STD_PRINTF("File write failed\n");
}
/* DEBUG CMD END */

/* EXTENDED DEBUG CMD BEGIN */
#define xalin debugContext->assemble_linear
#define xdlin debugContext->dump_linear
#define xulin debugContext->unassemble_linear
static C_INT xcheckrange(command_context *debugContext,
    type_unsigned_32 linear, type_unsigned_32 count)
{
    if (count != 0u && count - 1u > LIB_UINT32_MAX - linear) {
        seterr(debugContext, narg - 1);
        return 0;
    }
    return 1;
}
/* print */
static type_unsigned_8 xuprintins(command_context *debugContext, type_unsigned_32 linear)
{
    STD_SIZE_T i;
    STD_SIZE_T sbin_remaining;
    C_INT binary_failed = TYPE_FALSE;
    C_INT format_result;
    type_unsigned_8 len;
    /* The decoder needs 15 host bytes; accept only bytes read before address end. */
    type_unsigned_8 ucode[15] = {0};
    type_unsigned_8 available = linear > LIB_UINT32_MAX - 14u ?
        (type_unsigned_8)(LIB_UINT32_MAX - linear + 1u) : 15u;
    C_CHAR str[0x100], stmt[0x100], sbin[0x100];
    C_CHAR *sbin_cursor;
    if (command_machine_read_linear(linear, (C_VOID *)ucode, available))
    {
        len = 0;
        (C_VOID)STD_SNPRINTF(str, sizeof(str), "L%08X <ERROR>", linear);
    }
    else
    {
        lib_size instruction_bytes = 0u;
        if (x86_xasm32_disassemble(ucode, sizeof(ucode), stmt,
                sizeof(stmt), &i, &instruction_bytes,
                command_machine_get_code_default_size()) != TYPE_STATUS_OK ||
            instruction_bytes > available) {
            len = 0u;
            (void)lib_c_snprintf(stmt, sizeof(stmt), "<ERROR>");
        } else {
            len = (type_unsigned_8)instruction_bytes;
        }
        sbin[0] = 0;
        sbin_cursor = sbin;
        sbin_remaining = sizeof(sbin);
        for (i = 0; i < len; ++i)
        {
            format_result = STD_SNPRINTF_APPEND(&sbin_cursor, &sbin_remaining,
                "%02X", (type_unsigned_8)ucode[i]);
            if (format_result < 0) {
                binary_failed = TYPE_TRUE;
                len = 0;
                (C_VOID)STD_SNPRINTF(str, sizeof(str), "L%08X <ERROR>", linear);
                break;
            }
        }
        if (!binary_failed) {
            (C_VOID)STD_SNPRINTF(str, sizeof(str), "L%08X %s ", linear, sbin);
        }
        for (i = STD_STRLEN(str); i < 24; ++i)
        {
            if (!command_append_text_checked(str, sizeof(str), " ")) {
                len = 0u;
                (C_VOID)STD_SNPRINTF(str, sizeof(str), "L%08X <ERROR>", linear);
                break;
            }
        }
        if (!command_append_text_checked(str, sizeof(str), stmt)) {
            len = 0u;
            (C_VOID)STD_SNPRINTF(str, sizeof(str), "L%08X <ERROR>", linear);
        }
    }
    STD_PRINTF("%s\n", str);
    return len;
}
static C_VOID xrprintreg(command_context *debugContext)
{
    type_unsigned_32 flags = _eflags;
    STD_PRINTF("EAX=%08X", _eax);
    STD_PRINTF(" EBX=%08X", _ebx);
    STD_PRINTF(" ECX=%08X", _ecx);
    STD_PRINTF(" EDX=%08X", _edx);
    STD_PRINTF("\nESP=%08X", _esp);
    STD_PRINTF(" EBP=%08X", _ebp);
    STD_PRINTF(" ESI=%08X", _esi);
    STD_PRINTF(" EDI=%08X", _edi);
    STD_PRINTF("\nEIP=%08X", _eip);
    STD_PRINTF(" EFL=%08X", flags);
    STD_PRINTF(": ");
    STD_PRINTF("%s ", flags & 0x20000u ? "VM" : "vm");
    STD_PRINTF("%s ", flags & 0x10000u ? "RF" : "rf");
    STD_PRINTF("%s ", flags & 0x04000u ? "NT" : "nt");
    STD_PRINTF("IOPL=%01X ", (flags >> 12u) & 3u);
    STD_PRINTF("%s ", flags & 0x00800u ? "OF" : "of");
    STD_PRINTF("%s ", flags & 0x00400u ? "DF" : "df");
    STD_PRINTF("%s ", flags & 0x00200u ? "IF" : "if");
    STD_PRINTF("%s ", flags & 0x00100u ? "TF" : "tf");
    STD_PRINTF("%s ", flags & 0x00080u ? "SF" : "sf");
    STD_PRINTF("%s ", flags & 0x00040u ? "ZF" : "zf");
    STD_PRINTF("%s ", flags & 0x00010u ? "AF" : "af");
    STD_PRINTF("%s ", flags & 0x00004u ? "PF" : "pf");
    STD_PRINTF("%s ", flags & 0x00001u ? "CF" : "cf");
    STD_PRINTF("\n");
    xulin = command_machine_get_code_base() + _eip;
    xuprintins(debugContext, xulin);
}
/* assemble */
static C_VOID xaconsole(command_context *debugContext)
{
    STD_SIZE_T i, len, errAsmPos;
    C_CHAR astmt[0x100];
    type_unsigned_8 acode[15];
    C_INT flagExitAsm = 0;
    while (!flagExitAsm)
    {
        STD_PRINTF("L%08X ", xalin);
        if (!command_machine_read_line(astmt, sizeof(astmt))) return;
        if (STD_STRLEN(astmt) != 0u && astmt[STD_STRLEN(astmt) - 1u] == '\n')
            astmt[STD_STRLEN(astmt) - 1u] = '\0';
        if (!STD_STRLEN(astmt))
        {
            flagExitAsm = 1;
            continue;
        }
        errAsmPos = 0;
        if (x86_xasm32_assemble(astmt, STD_STRLEN(astmt), acode,
                sizeof(acode), &len,
                command_machine_get_code_default_size()) != TYPE_STATUS_OK) {
            len = 0u;
        }
        if (!len)
        {
            errAsmPos = STD_STRLEN(astmt) + 9;
        }
        else
        {
            if (!xcheckrange(debugContext, xalin, (type_unsigned_32)len)) return;
            if (command_machine_write_linear(xalin, (C_VOID *)acode, (type_unsigned_8)len))
            {
                STD_PRINTF("debug: fail to write to L%08X\n", xalin);
                return;
            }
            if (len > LIB_UINT32_MAX - xalin) return;
            xalin += (type_unsigned_32)len;
        }
        if (errAsmPos)
        {
            for (i = 0; i < errAsmPos; ++i)
            {
                STD_PRINTF(" ");
            }
            STD_PRINTF("^ Error\n");
        }
    }
}
static C_VOID xa(command_context *debugContext)
{
    if (narg == 1)
    {
        xaconsole(debugContext);
    }
    else if (narg == 2)
    {
        xalin = scannubit32(debugContext, arg[1]);
        if (nErrPos)
        {
            return;
        }
        xaconsole(debugContext);
    }
    else
    {
        seterr(debugContext, 2);
    }
}
/* compare */
static C_VOID xc(command_context *debugContext)
{
    STD_SIZE_T i, count;
    type_unsigned_32 lin1, lin2;
    type_unsigned_8 val1, val2;
    if (narg != 4)
    {
        seterr(debugContext, narg - 1);
    }
    else
    {
        lin1 = scannubit32(debugContext, arg[1]);
        if (nErrPos)
        {
            return;
        }
        lin2 = scannubit32(debugContext, arg[2]);
        if (nErrPos)
        {
            return;
        }
        count = scannubit32(debugContext, arg[3]);
        if (nErrPos)
        {
            return;
        }
        if (!count)
        {
            return;
        }
        if (!xcheckrange(debugContext, lin1, (type_unsigned_32)count) ||
            !xcheckrange(debugContext, lin2, (type_unsigned_32)count)) return;
        for (i = 0; i < count; ++i)
        {
            if (command_machine_read_linear((type_unsigned_32)(lin1 + i), (C_VOID *)(&val1), 1))
            {
                STD_PRINTF("debug: fail to read from L%08X.\n", (type_unsigned_32)(lin1 + i));
                return;
            }
            if (command_machine_read_linear((type_unsigned_32)(lin2 + i), (C_VOID *)(&val2), 1))
            {
                STD_PRINTF("debug: fail to read from L%08X.\n", (type_unsigned_32)(lin2 + i));
                return;
            }
            if (val1 != val2)
                STD_PRINTF("L%08X  %02X  %02X  L%08X\n",
                           (type_unsigned_32)(lin1 + i), val1, val2, (type_unsigned_32)(lin2 + i));
        }
    }
}
/* dump */
static C_VOID xdprint(command_context *debugContext, type_unsigned_32 linear, type_unsigned_32 count)
{
    C_CHAR t, c[0x11];
    type_unsigned_32 ilinear;
    type_unsigned_32 start = linear;
    type_unsigned_32 end;
    c[0x10] = '\0';
    if (!count)
    {
        return;
    }
    if (!xcheckrange(debugContext, linear, count)) return;
    end = linear + (count - 1u);
    for (ilinear = start - (start % 0x10); ilinear <= (end | 0x0fu); ++ilinear)
    {
        if (ilinear % 0x10 == 0)
            STD_PRINTF("L%08X  ", ilinear);
        if (ilinear < start || ilinear > end)
        {
            STD_PRINTF("  ");
            c[ilinear % 0x10] = ' ';
        }
        else
        {
            if (command_machine_read_linear(ilinear, (C_VOID *)(&c[ilinear % 0x10]), 1))
            {
                STD_PRINTF("debug: fail to read from L%08X\n", ilinear);
                return;
            }
            else
            {
                STD_PRINTF("%02X", c[ilinear % 0x10] & 0xff);
                t = c[ilinear % 0x10];
                if ((t >= 1 && t <= 7) || t == ' ' ||
                    (t >= 11 && t <= 12) ||
                    (t >= 14 && t <= 31) ||
                    (t >= 33))
                    ;
                else
                    c[ilinear % 0x10] = '.';
            }
        }
        STD_PRINTF(" ");
        if (ilinear % 0x10 == 7 && ilinear >= start && ilinear < end)
            STD_PRINTF("\b-");
        if (ilinear % 0x10 == 0x0f)
        {
            STD_PRINTF("  %s\n", c);
        }
        if (ilinear == 0xffffffff)
        {
            break;
        }
    }
    xdlin = ilinear;
}
static C_VOID xd(command_context *debugContext)
{
    type_unsigned_32 count;
    if (narg == 1)
    {
        xdprint(debugContext, xdlin, 0x80);
    }
    else if (narg == 2)
    {
        xdlin = scannubit32(debugContext, arg[1]);
        if (nErrPos)
        {
            return;
        }
        xdprint(debugContext, xdlin, 0x80);
    }
    else if (narg == 3)
    {
        xdlin = scannubit32(debugContext, arg[1]);
        count = scannubit32(debugContext, arg[2]);
        if (nErrPos)
        {
            return;
        }
        xdprint(debugContext, xdlin, count);
    }
    else
    {
        seterr(debugContext, 3);
    }
}
/* enter */
static C_VOID xe(command_context *debugContext)
{
    STD_SIZE_T i;
    type_unsigned_8 val;
    type_unsigned_32 linear;
    C_CHAR s[0x100];
    if (narg == 1)
    {
        seterr(debugContext, 0);
    }
    else if (narg == 2)
    {
        linear = scannubit32(debugContext, arg[1]);
        if (nErrPos)
        {
            return;
        }
        if (command_machine_read_linear(linear, (C_VOID *)(&val), 1))
        {
            STD_PRINTF("debug: fail to read from L%08X.\n", linear);
            return;
        }
        STD_PRINTF("L%08X  %02X.", linear, val);
        if (!command_machine_read_line(s, sizeof(s))) return;
        type_string_lower(s);
        val = scannubit8(debugContext, s);
        if (nErrPos)
        {
            return;
        }
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (command_machine_write_linear(linear, (C_VOID *)(&val), 1))
            {
                STD_PRINTF("debug: fail to write to L%08X.\n", linear);
            }
        }
    }
    else
    {
        type_unsigned_8 bytes[X86_DEBUG_LINE_CAPACITY];
        STD_SIZE_T count;
        linear = scannubit32(debugContext, arg[1]);
        count = scanlist(debugContext, 2u, bytes);
        if (nErrPos || !xcheckrange(debugContext, linear, (type_unsigned_32)count)) return;
        for (i = 0u; i < count; ++i)
            if (command_machine_write_linear(linear + (type_unsigned_32)i,
                    &bytes[i], 1)) return;
    }
}
/* fill */
static C_VOID xf(command_context *debugContext)
{
    STD_SIZE_T i, count, length;
    type_unsigned_32 linear;
    type_unsigned_8 bytes[X86_DEBUG_LINE_CAPACITY];
    if (narg < 4u) { seterr(debugContext, narg - 1u); return; }
    linear = scannubit32(debugContext, arg[1]);
    count = scannubit32(debugContext, arg[2]);
    length = scanlist(debugContext, 3u, bytes);
    if (nErrPos || !xcheckrange(debugContext, linear, (type_unsigned_32)count)) return;
    for (i = 0u; i < count; ++i)
        if (command_machine_write_linear(linear + (type_unsigned_32)i,
                &bytes[i % length], 1)) return;
}

/* go */
static C_VOID xg(command_context *debugContext)
{
    STD_SIZE_T count = 0;
    type_unsigned_32 linear;
    if (command_machine_is_running())
    {
        STD_PRINTF("Machine is already running.\n");
        return;
    }
    switch (narg)
    {
    case 1:
        g(debugContext);
        return;
    case 2:
        linear = scannubit32(debugContext, arg[1]);
        count = 1;
        break;
    case 3:
        linear = scannubit32(debugContext, arg[1]);
        count = scannubit32(debugContext, arg[2]);
        break;
    default:
        seterr(debugContext, narg - 1);
        break;
    }
    if (nErrPos)
    {
        return;
    }
    if (count == 0u) return;
    if (command_machine_set_break_linear(linear)) return;
    command_begin_break(debugContext, count);
    command_machine_resume();
}
/* move */
static C_VOID xm(command_context *debugContext)
{
    type_unsigned_8 val;
    STD_SIZE_T i;
    type_unsigned_32 lin1, lin2, count, offset;
    if (narg != 4)
    {
        seterr(debugContext, narg - 1);
    }
    else
    {
        lin1 = scannubit32(debugContext, arg[1]);
        if (nErrPos)
        {
            return;
        }
        lin2 = scannubit32(debugContext, arg[2]);
        if (nErrPos)
        {
            return;
        }
        count = scannubit32(debugContext, arg[3]);
        if (nErrPos)
        {
            return;
        }
        if (!xcheckrange(debugContext, lin1, count) ||
            !xcheckrange(debugContext, lin2, count) || lin1 == lin2) return;
        for (i = 0; i < count; ++i)
        {
            offset = lin2 > lin1 ? count - 1u - (type_unsigned_32)i : (type_unsigned_32)i;
            if (command_machine_read_linear(lin1 + offset, (C_VOID *)(&val), 1))
            {
                STD_PRINTF("debug: fail to read from L%08X.\n", lin1 + offset);
                return;
            }
            if (command_machine_write_linear(lin2 + offset, (C_VOID *)(&val), 1))
            {
                STD_PRINTF("debug: fail to write to L%08X.\n", lin2 + offset);
                return;
            }
        }
    }
}
/* search */
static C_VOID xs(command_context *debugContext)
{
    STD_SIZE_T i, count, bcount;
    type_unsigned_32 linear;
    type_unsigned_8 mem[256], line[256];
    if (narg < 4)
    {
        seterr(debugContext, narg - 1);
    }
    else
    {
        linear = scannubit32(debugContext, arg[1]);
        if (nErrPos)
        {
            return;
        }
        count = scannubit32(debugContext, arg[2]);
        if (nErrPos)
        {
            return;
        }
        bcount = scanlist(debugContext, 3u, line);
        if (nErrPos || !xcheckrange(debugContext, linear, (type_unsigned_32)count)) return;
        if (count < bcount) return;
        for (i = 0; i <= count - bcount; ++i)
        {
            if (command_machine_read_linear((type_unsigned_32)(linear + i), (C_VOID *)mem, (type_unsigned_8)bcount))
            {
                STD_PRINTF("debug: fail to read from L%08X.\n", linear + i);
                return;
            }
            if (!STD_MEMCMP((C_VOID *)mem, (C_VOID *)line, bcount))
            {
                STD_PRINTF("L%08X\n", linear + i);
            }
        }
    }
}
/* trace */
static C_VOID xt(command_context *debugContext)
{
    type_unsigned_32 count;
    if (command_machine_is_running())
    {
        STD_PRINTF("Machine is already running.\n");
        return;
    }
    switch (narg)
    {
    case 1:
        count = 1;
        break;
    case 2:
        count = scannubit32(debugContext, arg[1]);
        break;
    default:
        seterr(debugContext, narg - 1);
        return;
    }
    if (nErrPos)
    {
        return;
    }
    if (count == 0u) return;
    if (command_begin_trace(debugContext, COMMAND_RUN_TRACE_LINEAR, count)) return;
    command_machine_resume();
}
/* register */
static C_VOID xrscanreg(command_context *debugContext)
{
    type_unsigned_32 value;
    C_CHAR s[0x100];
    if (!STD_STRCMP(arg[1], "eax"))
    {
        STD_PRINTF("EAX ");
        STD_PRINTF("%08X", _eax);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_EAX, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "ecx"))
    {
        STD_PRINTF("ECX ");
        STD_PRINTF("%08X", _ecx);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_ECX, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "edx"))
    {
        STD_PRINTF("EDX ");
        STD_PRINTF("%08X", _edx);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_EDX, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "ebx"))
    {
        STD_PRINTF("EBX ");
        STD_PRINTF("%08X", _ebx);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_EBX, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "esp"))
    {
        STD_PRINTF("ESP ");
        STD_PRINTF("%08X", _esp);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_ESP, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "ebp"))
    {
        STD_PRINTF("EBP ");
        STD_PRINTF("%08X", _ebp);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_EBP, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "esi"))
    {
        STD_PRINTF("ESI ");
        STD_PRINTF("%08X", _esi);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_ESI, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "edi"))
    {
        STD_PRINTF("EDI ");
        STD_PRINTF("%08X", _edi);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_EDI, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "eip"))
    {
        STD_PRINTF("EIP ");
        STD_PRINTF("%08X", _eip);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_EIP, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "eflags"))
    {
        STD_PRINTF("EFLAGS ");
        STD_PRINTF("%08X", _eflags);
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_EFLAGS, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "es"))
    {
        command_machine_print_segment_registers();
        STD_PRINTF(":");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, COMMAND_REGISTER_ES, (type_unsigned_16)value))
            {
                STD_PRINTF("debug: fail to load es from %04X\n", (type_unsigned_16)value);
            }
        }
    }
    else if (!STD_STRCMP(arg[1], "cs"))
    {
        command_machine_print_segment_registers();
        STD_PRINTF(":");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, COMMAND_REGISTER_CS, (type_unsigned_16)value))
            {
                STD_PRINTF("debug: fail to load cs from %04X\n", (type_unsigned_16)value);
            }
        }
    }
    else if (!STD_STRCMP(arg[1], "ss"))
    {
        command_machine_print_segment_registers();
        STD_PRINTF(":");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, COMMAND_REGISTER_SS, (type_unsigned_16)value))
            {
                STD_PRINTF("debug: fail to load ss from %04X\n", (type_unsigned_16)value);
            }
        }
    }
    else if (!STD_STRCMP(arg[1], "ds"))
    {
        command_machine_print_segment_registers();
        STD_PRINTF(":");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, COMMAND_REGISTER_DS, (type_unsigned_16)value))
            {
                STD_PRINTF("debug: fail to load ds from %04X\n", (type_unsigned_16)value);
            }
        }
    }
    else if (!STD_STRCMP(arg[1], "fs"))
    {
        command_machine_print_segment_registers();
        STD_PRINTF(":");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, COMMAND_REGISTER_FS, (type_unsigned_16)value))
            {
                STD_PRINTF("debug: fail to load fs from %04X\n", (type_unsigned_16)value);
            }
        }
    }
    else if (!STD_STRCMP(arg[1], "gs"))
    {
        command_machine_print_segment_registers();
        STD_PRINTF(":");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, COMMAND_REGISTER_GS, (type_unsigned_16)value))
            {
                STD_PRINTF("debug: fail to load gs from %04X\n", (type_unsigned_16)value);
            }
        }
    }
    else if (!STD_STRCMP(arg[1], "cr0"))
    {
        STD_PRINTF("CR0 ");
        STD_PRINTF("%08X", _cr(0));
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_CR0, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "cr1"))
    {
        STD_PRINTF("CR1 ");
        STD_PRINTF("%08X", _cr(1));
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_CR1, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "cr2"))
    {
        STD_PRINTF("CR2 ");
        STD_PRINTF("%08X", _cr(2));
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_CR2, value);
        }
    }
    else if (!STD_STRCMP(arg[1], "cr3"))
    {
        STD_PRINTF("CR3 ");
        STD_PRINTF("%08X", _cr(3));
        STD_PRINTF("\n:");
        if (!command_machine_read_line(s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_CR3, value);
        }
    }
    else
    {
        STD_PRINTF("br Error\n");
    }
}
static C_VOID xr(command_context *debugContext)
{
    if (narg == 1)
    {
        xrprintreg(debugContext);
    }
    else if (narg == 2)
    {
        xrscanreg(debugContext);
    }
    else
    {
        seterr(debugContext, 2);
    }
}
/* unassemble */
static C_VOID xuprint(command_context *debugContext, type_unsigned_32 linear, type_unsigned_32 count)
{
    type_unsigned_32 len = 0;
    STD_SIZE_T i;
    for (i = 0; i < count; ++i)
    {
        len = xuprintins(debugContext, linear);
        if (!len || len > LIB_UINT32_MAX - linear)
        {
            break;
        }
        linear += len;
    }
    xulin = linear;
}
static C_VOID xu(command_context *debugContext)
{
    type_unsigned_32 count;
    if (narg == 1)
    {
        xuprint(debugContext, xulin, 10);
    }
    else if (narg == 2)
    {
        xulin = scannubit32(debugContext, arg[1]);
        if (nErrPos)
        {
            return;
        }
        xuprint(debugContext, xulin, 0x10);
    }
    else if (narg == 3)
    {
        xulin = scannubit32(debugContext, arg[1]);
        count = scannubit32(debugContext, arg[2]);
        if (nErrPos)
        {
            return;
        }
        xuprint(debugContext, xulin, count);
    }
    else
    {
        seterr(debugContext, 3);
    }
}
/* watch */
static C_VOID xw(command_context *debugContext)
{
    type_unsigned_32 linear;
    switch (narg)
    {
    case 1:
        command_machine_print_watchpoints();
        break;
    case 2:
        switch (arg[1][0])
        {
        case 'r':
            if (command_machine_clear_watch(COMMAND_REGISTER_WATCH_READ)) return;
            STD_PRINTF("Watch-read point removed.\n");
            break;
        case 'w':
            if (command_machine_clear_watch(COMMAND_REGISTER_WATCH_WRITE)) return;
            STD_PRINTF("Watch-write point removed.\n");
            break;
        case 'e':
            if (command_machine_clear_watch(COMMAND_REGISTER_WATCH_EXECUTE)) return;
            STD_PRINTF("Watch-exec point removed.\n");
            break;
        case 'u':
            if (command_machine_clear_watch(COMMAND_REGISTER_WATCH_READ) ||
                command_machine_clear_watch(COMMAND_REGISTER_WATCH_WRITE) ||
                command_machine_clear_watch(COMMAND_REGISTER_WATCH_EXECUTE)) return;
            STD_PRINTF("All watch points removed.\n");
            break;
        default:
            seterr(debugContext, 1);
            break;
        }
        break;
    case 3:
        switch (arg[1][0])
        {
        case 'r':
            linear = scannubit32(debugContext, arg[2]);
            if (nErrPos) return;
            command_machine_set_watch(COMMAND_REGISTER_WATCH_READ, linear);
            break;
        case 'w':
            linear = scannubit32(debugContext, arg[2]);
            if (nErrPos) return;
            command_machine_set_watch(COMMAND_REGISTER_WATCH_WRITE, linear);
            break;
        case 'e':
            linear = scannubit32(debugContext, arg[2]);
            if (nErrPos) return;
            command_machine_set_watch(COMMAND_REGISTER_WATCH_EXECUTE, linear);
            break;
        default:
            seterr(debugContext, 2);
            break;
        }
        break;
    default:
        seterr(debugContext, narg - 1);
        break;
    }
}
static C_VOID xhelp(command_context *debugContext)
{
    STD_PRINTF("assemble        XA [address]\n");
    STD_PRINTF("compare         XC addr1 addr2 count_byte\n");
    STD_PRINTF("dump            XD [address [count_byte]]\n");
    STD_PRINTF("enter           XE address [byte_list]\n");
    STD_PRINTF("fill            XF address count_byte byte_list\n");
    STD_PRINTF("go              XG [address [count_instr]]\n");
    STD_PRINTF("move            XM addr1 addr2 count_byte\n");
    STD_PRINTF("register        XR [register]\n");
    STD_PRINTF("  regular         XREG\n");
    STD_PRINTF("  segment         XSREG\n");
    STD_PRINTF("  control         XCREG\n");
    STD_PRINTF("search          XS address count_byte byte_list\n");
    STD_PRINTF("trace           XT [count_instr]\n");
    STD_PRINTF("unassemble      XU [address [count_instr]]\n");
    STD_PRINTF("watch           XW r/w/e address\n");
}
static C_VOID x(command_context *debugContext)
{
    STD_SIZE_T i;
    arg[narg] = arg[0];
    for (i = 1; i < narg; ++i)
    {
        arg[i - 1] = arg[i];
    }
    arg[narg - 1] = arg[narg];
    arg[narg] = STD_NULL;
    narg--;
    if (!STD_STRCMP(arg[0], "\?"))
    {
        xhelp(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "a"))
    {
        xa(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "c"))
    {
        xc(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "d"))
    {
        xd(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "e"))
    {
        xe(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "f"))
    {
        xf(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "g"))
    {
        xg(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "m"))
    {
        xm(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "r"))
    {
        xr(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "s"))
    {
        xs(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "t"))
    {
        xt(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "u"))
    {
        xu(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "w"))
    {
        xw(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "reg"))
    {
        xrprintreg(debugContext);
    }
    else if (!STD_STRCMP(arg[0], "sreg"))
    {
        command_machine_print_segment_registers();
    }
    else if (!STD_STRCMP(arg[0], "creg"))
    {
        command_machine_print_control_registers();
    }
    else
    {
        arg[0] = arg[narg];
        seterr(debugContext, 0);
    }
}
/* EXTENDED DEBUG CMD END */

/* main routines */
static C_VOID help(command_context *debugContext)
{
    STD_PRINTF("assemble        A [address]\n");
    STD_PRINTF("compare         C range address\n");
    STD_PRINTF("dump            D [range]\n");
    STD_PRINTF("enter           E address [list]\n");
    STD_PRINTF("fill            F range list\n");
    STD_PRINTF("go              G [[address] breakpoint]\n");
    /* STD_PRINTF("go              G [=address] [addresses]\n"); */
    STD_PRINTF("hex             H value1 value2\n");
    STD_PRINTF("input           I port\n");
    STD_PRINTF("load            L [address]\n");
    /* STD_PRINTF("load            L [address] [drive] [firstsector] [number]\n"); */
    STD_PRINTF("move            M range address\n");
    STD_PRINTF("name            N pathname\n");
    /* STD_PRINTF("name            N [pathname] [arglist]\n"); */
    STD_PRINTF("output          O port byte\n");
    /* !STD_PRINTF("proceed           P [nx=address] [number]\n"); */
    STD_PRINTF("quit            Q \n");
    STD_PRINTF("register        R [register]\n");
    STD_PRINTF("search          S range list\n");
    STD_PRINTF("trace           T [[address] value]\n");
    /* STD_PRINTF("trace           T [=address] [value]\n"); */
    STD_PRINTF("unassemble      U [range]\n");
    STD_PRINTF("range           address [end | L length]\n");
    STD_PRINTF("list            hex bytes and quoted strings\n");
    STD_PRINTF("verbal          V \n");
    STD_PRINTF("write           W [address]\n");
    STD_PRINTF("debug32         X?\n");
    /* STD_PRINTF("write           W [address] [drive] [firstsector] [number]\n"); */
    /* STD_PRINTF("allocate expanded memory        XA [#pages]\n"); */
    /* STD_PRINTF("deallocate expanded memory      XD [handle]\n"); */
    /* STD_PRINTF("map expanded memory pages       XM [Lpage] [Ppage] [handle]\n"); */
    /* STD_PRINTF("display expanded memory status  XS\n"); */
}

static C_VOID parse(command_context *debugContext)
{
    C_CHAR *p;
    if (!command_copy_text_checked(strCmdCopy, sizeof(strCmdCopy), strCmdBuff)) return;
    narg = 0u;
    nErrPos = 0u;
    p = strCmdCopy;
    while (*p)
    {
        C_CHAR *start, quote = 0;
        while (*p && lib_c_strchr(" ,\t\n\r\f", *p)) ++p;
        if (!*p) break;
        start = p;
        while (*p)
        {
            if (quote)
            {
                if (*p == quote)
                {
                    if (p[1] == quote) { p += 2; continue; }
                    quote = 0;
                }
            }
            else
            {
                if (lib_c_strchr(" ,\t\n\r\f", *p)) break;
                if (*p == '\'' || *p == '"') quote = *p;
                else if (*p >= 'A' && *p <= 'Z') *p += 'a' - 'A';
            }
            ++p;
        }
        if (*p) *p++ = '\0';
        arg[narg++] = start;
        if (narg == 1u && STD_STRLEN(start) > 1u) arg[narg++] = start + 1;
        if (quote) { seterr(debugContext, narg - 1u); break; }
    }
    arg[narg] = STD_NULL;
}

static C_VOID exec(command_context *debugContext)
{
    if (nErrPos) return;
    if (!arg[0])
    {
        return;
    }
    switch (arg[0][0])
    {
    case '\?':
        help(debugContext);
        break;
    case 'a':
        a(debugContext);
        break;
    case 'c':
        c(debugContext);
        break;
    case 'd':
        d(debugContext);
        break;
    case 'e':
        e(debugContext);
        break;
    case 'f':
        f(debugContext);
        break;
    case 'g':
        g(debugContext);
        break;
    case 'h':
        h(debugContext);
        break;
    case 'i':
        i(debugContext);
        break;
    case 'l':
        l(debugContext);
        break;
    case 'm':
        m(debugContext);
        break;
    case 'n':
        n(debugContext);
        break;
    case 'o':
        o(debugContext);
        break;
    case 'q':
        q(debugContext);
        break;
    case 'r':
        r(debugContext);
        break;
    case 's':
        s(debugContext);
        break;
    case 't':
        t(debugContext);
        break;
    case 'u':
        u(debugContext);
        break;
    case 'v':
        v(debugContext);
        break;
    case 'w':
        w(debugContext);
        break;
    case 'x':
        x(debugContext);
        break;
    default:
        seterr(debugContext, 0);
        break;
    }
}

static C_VOID command_initialize(x86_debug *command,
    common_machine *machine)
{
    lib_release(command->output);
    STD_MEMSET(command, 0, sizeof(*command));
    command->machine = machine;
}

lib_status x86_debug_create(x86_debug **out_command)
{
    x86_debug *command;

    if (out_command == STD_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_command = STD_NULL;
    command = (x86_debug *)STD_CALLOC(1u, sizeof(*command));
    if (command == STD_NULL) return LIB_STATUS_NO_MEMORY;
    *out_command = command;
    return LIB_STATUS_OK;
}

void x86_debug_destroy(x86_debug *command)
{
    if (command == STD_NULL) return;
    x86_debug_close(command);
    lib_release(command->output);
    STD_FREE(command);
}

lib_status x86_debug_open(x86_debug *command,
    common_machine *machine)
{
    if (command == STD_NULL || machine == STD_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    x86_debug_close(command);
    command_initialize(command, machine);
    return LIB_STATUS_OK;
}

void x86_debug_close(x86_debug *command)
{
    if (command == STD_NULL) return;
    common_machine_debug_cancel(command->machine);
    command->machine = STD_NULL;
    command->continuation = COMMAND_CONTINUATION_NONE;
}

static void command_prompt(x86_debug *command)
{
    const char *prompt = command->continuation == COMMAND_CONTINUATION_NONE ?
        "-" : command->input_prompt;
    if (command->result != STD_NULL) {
        (void)command_copy_text_checked(command->result->prompt,
            sizeof(command->result->prompt), prompt);
        command->result->prompt_ready = LIB_TRUE;
    }
}

static void command_prepare_continuation(x86_debug *command)
{
    if (command->argument_count == 0u || command->arguments[0] == STD_NULL)
        return;
    if (command->arguments[0][0] == 'a')
        command->continuation = COMMAND_CONTINUATION_ASSEMBLE;
    else if (command->arguments[0][0] == 'e' &&
        command->argument_count == 2u)
        command->continuation = COMMAND_CONTINUATION_ENTER;
    else if (command->arguments[0][0] == 'r' &&
        command->argument_count == 2u)
        command->continuation = COMMAND_CONTINUATION_REGISTER;
    else if (command->arguments[0][0] == 'v')
        command->continuation = COMMAND_CONTINUATION_VERBAL;
    else if (command->arguments[0][0] == 'x' &&
        command->argument_count >= 2u) {
        if (!STD_STRCMP(command->arguments[1], "a"))
            command->continuation = COMMAND_CONTINUATION_XASSEMBLE;
        else if (!STD_STRCMP(command->arguments[1], "e") &&
            command->argument_count == 3u)
            command->continuation = COMMAND_CONTINUATION_XENTER;
        else if (!STD_STRCMP(command->arguments[1], "r") &&
            command->argument_count == 3u)
            command->continuation = COMMAND_CONTINUATION_XREGISTER;
    }
}

/* CLI lifetime is independent of machine access. Help, arithmetic, filename
 * selection and exit do not acquire a machine lease or initialize addresses. */
static lib_bool command_needs_machine(x86_debug *command)
{
    const char *name;
    if (command->continuation != COMMAND_CONTINUATION_NONE)
        return command->continuation != COMMAND_CONTINUATION_VERBAL;
    if (command->argument_count == 0u) return LIB_FALSE;
    name = command->arguments[0];
    if (lib_c_strchr("acdefgilmorstuwx", name[0]) == NULL) return LIB_FALSE;
    return lib_c_strcmp(name, "?") != 0 && lib_c_strcmp(name, "h") != 0 &&
        lib_c_strcmp(name, "n") != 0 && lib_c_strcmp(name, "q") != 0 &&
        !(name[0] == 'x' && command->argument_count == 2u &&
            lib_c_strcmp(command->arguments[1], "?") == 0);
}

static lib_bool command_prepare_machine(x86_debug *debugContext)
{
    common_machine_debug_lease lease;
    debugContext->access_status = common_machine_debug_acquire(debugContext->machine, &lease);
    if (debugContext->access_status != LIB_STATUS_OK) return LIB_FALSE;
    if (!debugContext->defaults_ready) {
        asmSegRec = uasmSegRec = _cs;
        asmPtrRec = uasmPtrRec = _ip;
        dumpSegRec = _ds;
        dumpPtrRec = (type_unsigned_16)(_ip) / 0x10 * 0x10;
        xulin = command_machine_get_code_base() + _eip;
        debugContext->defaults_ready = debugContext->access_status == LIB_STATUS_OK;
    }
    return debugContext->access_status == LIB_STATUS_OK;
}

static void command_report_access(x86_debug *command)
{
    if (command->access_status == LIB_STATUS_OK) return;
    command->continuation = COMMAND_CONTINUATION_NONE;
    command->pending_line_available = 0;
    command->run_kind = COMMAND_RUN_NONE;
    command->result->lifecycle_request = X86_DEBUG_LIFECYCLE_NONE;
    command->output_length = 0u;
    command_printf(command, "%s\r\n\r\n",
        command->access_status == LIB_STATUS_INVALID_STATE ?
            "Machine must be paused for this debug operation." :
        command->access_status == LIB_STATUS_UNSUPPORTED ?
            "Debug operation is unsupported by this machine." :
            "Debug machine access failed.");
}

lib_status x86_debug_submit_line(x86_debug *command,
    const char *line, x86_debug_result *out_result)
{
    command_context *debugContext = command;
    STD_SIZE_T i;

    if (command == STD_NULL || line == STD_NULL || out_result == STD_NULL ||
        command->machine == STD_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    command_begin_output(command, out_result);
    command->access_status = LIB_STATUS_OK;
    if (lib_text_length(line) >= sizeof(command->command_buffer)) {
        command_printf(command, "Debug command is too long.\r\n\r\n");
        goto finished;
    }
    if (command->continuation == COMMAND_CONTINUATION_NONE) {
        if (!command_copy_text_checked(strCmdBuff, sizeof(strCmdBuff), line))
            return LIB_STATUS_INVALID_ARGUMENT;
        parse(debugContext);
        if (!nErrPos && command_needs_machine(command) && !command_prepare_machine(command))
            goto finished;
        if (!nErrPos) command_prepare_continuation(command);
        exec(debugContext);
    } else {
        if (command_needs_machine(command) && !command_prepare_machine(command)) goto finished;
        if (!command_copy_text_checked(command->pending_line,
                sizeof(command->pending_line), line)) return LIB_STATUS_INVALID_ARGUMENT;
        command->pending_line_available = 1;
        switch (command->continuation) {
        case COMMAND_CONTINUATION_ASSEMBLE:
            aconsole(debugContext);
            if (line[0] == '\0') command->continuation = COMMAND_CONTINUATION_NONE;
            break;
        case COMMAND_CONTINUATION_ENTER: e(debugContext); command->continuation = COMMAND_CONTINUATION_NONE; break;
        case COMMAND_CONTINUATION_REGISTER: rscanregs(debugContext); command->continuation = COMMAND_CONTINUATION_NONE; break;
        case COMMAND_CONTINUATION_VERBAL: v(debugContext); command->continuation = COMMAND_CONTINUATION_NONE; break;
        case COMMAND_CONTINUATION_XASSEMBLE:
            xaconsole(debugContext);
            if (line[0] == '\0') command->continuation = COMMAND_CONTINUATION_NONE;
            break;
        case COMMAND_CONTINUATION_XENTER: xe(debugContext); command->continuation = COMMAND_CONTINUATION_NONE; break;
        case COMMAND_CONTINUATION_XREGISTER: xrscanreg(debugContext); command->continuation = COMMAND_CONTINUATION_NONE; break;
        default: break;
        }
    }
    if (!out_result->prompt_ready) command->continuation = COMMAND_CONTINUATION_NONE;
    if (nErrPos) {
        command->continuation = COMMAND_CONTINUATION_NONE;
        for (i = 0u; i < nErrPos; ++i) STD_PRINTF(" ");
        STD_PRINTF("^ Error\n");
    }
finished:
    command_report_access(command);
    command_prompt(command);
    if (flagExit) out_result->keep_active = LIB_FALSE;
    return command_end_output(command);
}

lib_status x86_debug_observe_machine(x86_debug *command,
    x86_debug_machine_state state, lib_status status,
    x86_debug_result *out_result)
{
    type_unsigned_32 executed;

    if (command == STD_NULL || out_result == STD_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    command_begin_output(command, out_result);
    if (state == X86_DEBUG_MACHINE_STOPPED || state == X86_DEBUG_MACHINE_RESET ||
        state == X86_DEBUG_MACHINE_FAULT) {
        command->run_kind = COMMAND_RUN_NONE;
        return command_end_output(command);
    }
    if (state != X86_DEBUG_MACHINE_PAUSED || status != LIB_STATUS_OK)
        return command_end_output(command);
    command->access_status = LIB_STATUS_OK;
    if (command_get_execution_result(command, &executed)) {
        if (command->access_status == LIB_STATUS_OK && command->run_kind == COMMAND_RUN_BREAK_REAL)
            rprintregs(command);
        command->run_kind = COMMAND_RUN_NONE;
        command_report_access(command);
        command_prompt(command);
        return command_end_output(command);
    }
    if (command->observation.watch_hit) {
        const char *names[] = { "read", "write", "execute" };
        command_printf(command, "Watch-%s hit: Lin=%08x\n",
            names[command->observation.watch_kind], command->observation.watch_address);
        command_print_memory_accesses(command);
        command->run_kind = COMMAND_RUN_NONE;
        xrprintreg(command);
    }
    switch (command->run_kind) {
    case COMMAND_RUN_TRACE_REAL:
        rprintregs(command);
        if (command->trace_remaining != 0u) --command->trace_remaining;
        break;
    case COMMAND_RUN_TRACE_LINEAR:
        command_print_memory_accesses(command);
        xrprintreg(command);
        if (command->trace_remaining != 0u) --command->trace_remaining;
        break;
    case COMMAND_RUN_BREAK_REAL:
        rprintregs(command);
        break;
    case COMMAND_RUN_BREAK_LINEAR:
        command_printf(command, "%d instructions executed before the break point.\n",
            executed);
        xrprintreg(command);
        if (command->breakpoint_remaining != 0u)
            --command->breakpoint_remaining;
        break;
    default:
        break;
    }
    if ((command->run_kind == COMMAND_RUN_TRACE_REAL ||
            command->run_kind == COMMAND_RUN_TRACE_LINEAR) &&
        command->trace_remaining != 0u) {
        command_printf(command, "\n");
        if (command_set_trace(command, 1u)) {
            command->run_kind = COMMAND_RUN_NONE;
            command_report_access(command);
            command_prompt(command);
            return command_end_output(command);
        }
        out_result->lifecycle_request = X86_DEBUG_LIFECYCLE_RESUME;
    } else if (command->run_kind == COMMAND_RUN_BREAK_LINEAR &&
        command->breakpoint_remaining != 0u) {
        if (command_set_break(command, command->breakpoint_linear)) {
            command->run_kind = COMMAND_RUN_NONE;
            command_report_access(command);
            command_prompt(command);
            return command_end_output(command);
        }
        out_result->lifecycle_request = X86_DEBUG_LIFECYCLE_RESUME;
    } else {
        command->run_kind = COMMAND_RUN_NONE;
        command_clear_break(command);
    }
    command_report_access(command);
    command_prompt(command);
    return command_end_output(command);
}
