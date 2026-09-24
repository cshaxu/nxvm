/* DEBUG is the debug console for users to break, trace, lookup,
 * and print virtual machine devices. */

#include "x86/debug/debug_interface.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"
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
    lib_u32 assemble_linear;
    lib_u32 dump_linear;
    lib_u32 unassemble_linear;
    x86_debug_result *result;
    char *output;
    lib_size output_length, output_capacity;
    lib_status output_status;
    char input_prompt[X86_DEBUG_PROMPT_CAPACITY];
    lib_size error_position;
    lib_size argument_count;
    char *arguments[DEBUG_MAXNARG];
    lib_i32 exit_requested;
    char command_buffer[0x100];
    char command_copy[0x100];
    char file_name[0x100];
    lib_u16 dump_segment;
    lib_u16 dump_offset;
    lib_u16 assemble_segment;
    lib_u16 assemble_offset;
    lib_u16 unassemble_segment;
    lib_u16 unassemble_offset;
    lib_u16 parsed_segment;
    lib_u16 parsed_offset;
    char pending_line[X86_DEBUG_LINE_CAPACITY];
    lib_i32 pending_line_available;
    command_continuation continuation;
    lib_u32 breakpoint_linear;
    lib_size breakpoint_remaining;
    lib_size trace_remaining;
    command_run_kind run_kind;
    x86_debug_observation observation;
};

typedef x86_debug command_context;
typedef x86_debug_register command_register;
typedef x86_debug_watch_kind x86_debug_watch_kind;

static void command_text_lower(char *text)
{
    while (text != LIB_NULL && *text != '\0') {
        if (*text >= 'A' && *text <= 'Z') *text = (char)(*text + ('a' - 'A'));
        ++text;
    }
}

static lib_i32 command_format_append(char **cursor, lib_size *remaining,
    const char *format, ...)
{
    lib_c_va_list arguments;
    lib_i32 written;
    if (cursor == LIB_NULL || *cursor == LIB_NULL || remaining == LIB_NULL ||
        *remaining == 0u) return -1;
    lib_c_va_start(arguments, format);
    written = lib_c_vsnprintf(*cursor, *remaining, format, arguments);
    lib_c_va_end(arguments);
    if (written < 0 || (lib_size)written >= *remaining) return -1;
    *cursor += written;
    *remaining -= (lib_size)written;
    return written;
}
static lib_i32 command_copy_text_checked(char *destination,
    lib_size destination_capacity, const char *source);

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

static lib_i32 command_printf(command_context *command, const char *format, ...)
{
    lib_c_va_list args, measure;
    lib_i32 count;
    lib_size needed, capacity;
    char *expanded;
    if (command == LIB_NULL || command->result == LIB_NULL) return -1;
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

static lib_i32 command_read_line(command_context *debugContext,
    char *buffer, lib_size buffer_size)
{
    if (debugContext == LIB_NULL || buffer == LIB_NULL || buffer_size == 0u) return 0;
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

static lib_i32 command_execute(command_context *debugContext,
    const x86_debug_request *request,
    x86_debug_response *result)
{
    common_machine_debug_lease lease;
    lib_size response_size;

    _Static_assert(sizeof(*request) <= COMMON_MACHINE_DEBUG_REQUEST_CAPACITY,
        "x86 request must fit Machine transport");
    _Static_assert(sizeof(*result) <= COMMON_MACHINE_DEBUG_RESPONSE_CAPACITY,
        "x86 response must fit Machine transport");

    if (debugContext == LIB_NULL || request == LIB_NULL || result == LIB_NULL) return 1;
    lib_memory_set(result, 0, sizeof(*result));
    if (debugContext->access_status != LIB_STATUS_OK) return 1;
    debugContext->access_status = common_machine_debug_acquire(debugContext->machine, &lease);
    if (debugContext->access_status == LIB_STATUS_OK)
        debugContext->access_status = common_machine_debug_execute_with_lease(
            debugContext->machine, &lease, request, sizeof(*request),
            result, sizeof(*result), &response_size);
    if (debugContext->access_status == LIB_STATUS_OK && response_size != sizeof(*result)) {
        lib_memory_set(result, 0, sizeof(*result));
        debugContext->access_status = LIB_STATUS_IO_ERROR;
    }
    if (debugContext->access_status != LIB_STATUS_OK) debugContext->error_position = 1u;
    return debugContext->access_status != LIB_STATUS_OK;
}

static lib_i32 command_read_register(command_context *debugContext,
    command_register register_id, lib_u32 *value)
{
    x86_debug_response result;
    if (value == LIB_NULL || command_execute(debugContext,
            &(x86_debug_request){
                .operation = X86_DEBUG_READ_REGISTER,
                .register_id = (lib_u32)register_id },
            &result)) return 1;
    *value = result.value;
    return 0;
}

static lib_i32 command_write_register(command_context *debugContext,
    command_register register_id, lib_u32 value)
{
    x86_debug_response result;
    return command_execute(debugContext, &(x86_debug_request){
        .operation = X86_DEBUG_WRITE_REGISTER,
        .register_id = (lib_u32)register_id, .address = value }, &result);
}

static lib_i32 command_access_memory(command_context *debugContext,
    x86_debug_operation operation, lib_u32 address,
    lib_u16 segment, lib_u16 offset, void *data,
    lib_u8 bytes)
{
    x86_debug_request request = {0};
    x86_debug_response result;
    if (data == LIB_NULL || bytes > sizeof(request.data)) {
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
        lib_memory_copy(request.data, data, bytes);
    if (command_execute(debugContext, &request, &result)) {
        if (operation == X86_DEBUG_READ_LINEAR ||
            operation == X86_DEBUG_READ_REAL) lib_memory_set(data, 0, bytes);
        return 1;
    }
    if (operation == X86_DEBUG_READ_LINEAR ||
        operation == X86_DEBUG_READ_REAL)
        lib_memory_copy(data, result.data, bytes);
    return 0;
}

static lib_i32 command_read_linear(command_context *debugContext, lib_u32 address,
    void *data, lib_u8 bytes)
{ return command_access_memory(debugContext, X86_DEBUG_READ_LINEAR, address, 0u, 0u, data, bytes); }

static lib_i32 command_write_linear(command_context *debugContext, lib_u32 address,
    void *data, lib_u8 bytes)
{ return command_access_memory(debugContext, X86_DEBUG_WRITE_LINEAR, address, 0u, 0u, data, bytes); }

static lib_i32 command_read_real(command_context *debugContext, lib_u16 segment,
    lib_u16 offset, void *data, lib_u8 bytes)
{ return command_access_memory(debugContext, X86_DEBUG_READ_REAL, 0u, segment, offset, data, bytes); }

static lib_i32 command_write_real(command_context *debugContext, lib_u16 segment,
    lib_u16 offset, void *data, lib_u8 bytes)
{ return command_access_memory(debugContext, X86_DEBUG_WRITE_REAL, 0u, segment, offset, data, bytes); }
static lib_i32 command_read_port(command_context *debugContext,
    lib_u16 port, lib_u32 *value)
{
    x86_debug_response result;
    if (command_execute(debugContext, &(x86_debug_request){
            .operation = X86_DEBUG_READ_PORT, .port = port, .bytes = 1u }, &result))
        return 1;
    *value = result.value;
    return 0;
}

static lib_u32 command_read_port_value(command_context *debugContext,
    lib_u16 port)
{
    lib_u32 value = 0u;
    (void)command_read_port(debugContext, port, &value);
    return value;
}

static lib_i32 command_write_port(command_context *debugContext,
    lib_u16 port, lib_u32 value)
{
    x86_debug_request request = {0};
    x86_debug_response result;
    request.operation = X86_DEBUG_WRITE_PORT;
    request.bytes = 1u;
    request.port = port;
    request.address = value;
    return command_execute(debugContext, &request, &result);
}

static lib_u32 command_code_property(command_context *debugContext,
    x86_debug_operation operation)
{
    x86_debug_response result;
    return command_execute(debugContext,
        &(x86_debug_request){ .operation = operation }, &result) ? 0u : result.value;
}

static lib_i32 command_set_watch(command_context *debugContext,
    x86_debug_watch_kind kind, lib_u32 address)
{
    x86_debug_response result;
    return command_execute(debugContext, &(x86_debug_request){
        .operation = X86_DEBUG_SET_WATCH, .address = address,
        .watch_kind = kind }, &result);
}

static lib_i32 command_clear_watch(command_context *debugContext,
    x86_debug_watch_kind kind)
{
    x86_debug_response result;
    return command_execute(debugContext, &(x86_debug_request){
        .operation = X86_DEBUG_CLEAR_WATCH, .watch_kind = kind }, &result);
}

static lib_i32 command_get_watch(command_context *debugContext,
    x86_debug_watch_kind kind, lib_u32 *out_address)
{
    x86_debug_response result;

    if (out_address == LIB_NULL || command_execute(debugContext,
            &(x86_debug_request){
                .operation = X86_DEBUG_GET_WATCH,
                .watch_kind = kind }, &result)) return -1;
    *out_address = result.value;
    return result.enabled ? 1 : 0;
}

static lib_i32 command_is_paused(command_context *debugContext)
{
    common_machine_debug_lease lease;
    return debugContext != LIB_NULL &&
        common_machine_debug_acquire(debugContext->machine, &lease) == LIB_STATUS_OK;
}

static void command_resume(command_context *debugContext)
{
    if (debugContext != LIB_NULL && debugContext->result != LIB_NULL)
        debugContext->result->lifecycle_request = X86_DEBUG_LIFECYCLE_RESUME;
}


static lib_i32 command_set_break(command_context *debugContext,
    lib_u32 linear)
{
    x86_debug_request request = {0};
    x86_debug_response result;

    if (debugContext == LIB_NULL) return 1;
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

static lib_i32 command_set_break_real(command_context *debugContext,
    lib_u16 segment, lib_u16 offset)
{ return command_set_break(debugContext, ((lib_u32)segment << 4u) + offset); }
static void command_clear_break(command_context *debugContext)
{
    x86_debug_response result;

    if (debugContext == LIB_NULL) return;
    (void)command_execute(debugContext, &(x86_debug_request) {
        .operation = X86_DEBUG_CLEAR_EXECUTION_PLAN }, &result);
    debugContext->breakpoint_linear = 0u;
    debugContext->breakpoint_remaining = 0u;
}

static lib_i32 command_set_trace(command_context *debugContext, lib_size count)
{
    x86_debug_request request = {0};
    x86_debug_response result;

    if (debugContext == LIB_NULL) return 1;
    request.operation = X86_DEBUG_SET_EXECUTION_PLAN;
    request.execution_kind = X86_DEBUG_EXECUTION_TRACE;
    request.instruction_count = count;
    if (command_execute(debugContext, &request, &result)) {
        command_printf(debugContext, "debug: fail to install trace plan.\n");
        return 1;
    }
    return 0;
}

static lib_i32 command_get_execution_result(command_context *debugContext,
    lib_u32 *out_executed)
{
    x86_debug_response result;

    if (out_executed == LIB_NULL || command_execute(debugContext,
            &(x86_debug_request) {
                .operation = X86_DEBUG_GET_EXECUTION_RESULT },
            &result) || !result.enabled) return 1;
    *out_executed = result.value;
    debugContext->observation = result.observation;
    return 0;
}

static lib_i32 command_begin_trace(command_context *debugContext,
    command_run_kind kind, lib_size count)
{
    if (debugContext == LIB_NULL || command_set_trace(debugContext,
            count < 0x100u ? 1u : count)) return 1;
    debugContext->run_kind = kind;
    debugContext->trace_remaining = count < 0x100u ? count : 1u;
    return 0;
}

static void command_begin_break(command_context *debugContext,
    lib_size count)
{
    if (debugContext == LIB_NULL) return;
    debugContext->run_kind = COMMAND_RUN_BREAK_LINEAR;
    debugContext->breakpoint_remaining = count;
}

static lib_i32 command_copy_text_checked(char *destination,
    lib_size destination_capacity, const char *source)
{
    lib_size source_bytes;

    if (destination == LIB_NULL || source == LIB_NULL || destination_capacity == 0u) {
        return 0;
    }
    source_bytes = lib_text_length(source);
    if (source_bytes >= destination_capacity) return 0;
    lib_memory_copy(destination, source, source_bytes + 1u);
    return 1;
}

static lib_i32 command_append_text_checked(char *destination,
    lib_size destination_capacity, const char *source)
{
    lib_size destination_bytes;
    lib_size source_bytes;

    if (destination == LIB_NULL || source == LIB_NULL || destination_capacity == 0u) {
        return 0;
    }
    destination_bytes = lib_text_length(destination);
    source_bytes = lib_text_length(source);
    if (destination_bytes >= destination_capacity ||
        source_bytes >= destination_capacity - destination_bytes) return 0;
    lib_memory_copy(destination + destination_bytes, source, source_bytes + 1u);
    return 1;
}

static void rprintregs(command_context *debugContext);
static void command_print_segments(command_context *debugContext);
static void command_print_controls(command_context *debugContext);
static void command_print_watches(command_context *debugContext);

#define nErrPos (debugContext->error_position)
#define narg (debugContext->argument_count)
#define arg (debugContext->arguments)
#define flagExit (debugContext->exit_requested)
#define strCmdBuff (debugContext->command_buffer)
#define strCmdCopy (debugContext->command_copy)
#define strFileName (debugContext->file_name)
static lib_u32 debug_register(command_context *debugContext, command_register reg)
{
    lib_u32 value = 0;
    command_read_register(debugContext, reg, &value);
    return value;
}
static lib_i32 debug_set_register(command_context *debugContext, command_register reg, lib_u32 value)
{
    return command_write_register(debugContext, reg, value);
}
static lib_i32 debug_set_word(command_context *debugContext, command_register reg, lib_u16 value)
{
    /* Original _ax/_ip assignments preserve the upper half of their alias. */
    return debug_set_register(debugContext, reg,
        (debug_register(debugContext, reg) & 0xffff0000u) | value);
}
static lib_i32 debug_flag(command_context *debugContext, lib_u32 mask)
{
    return (debug_register(debugContext, COMMAND_REGISTER_EFLAGS) & mask) != 0;
}
static void debug_set_flag(command_context *debugContext, lib_u32 mask, lib_i32 set)
{
    lib_u32 flags = debug_register(debugContext, COMMAND_REGISTER_EFLAGS);
    debug_set_register(debugContext, COMMAND_REGISTER_EFLAGS, set ? flags | mask : flags & ~mask);
}

static lib_i32 command_capture_cpu(command_context *debugContext,
    x86_debug_cpu_snapshot *out_snapshot)
{
    x86_debug_response result;

    if (out_snapshot == LIB_NULL || command_execute(debugContext,
            &(x86_debug_request){
                .operation = X86_DEBUG_GET_CPU_SNAPSHOT },
            &result)) return 1;
    *out_snapshot = result.cpu;
    return 0;
}

static void command_print_segment(command_context *debugContext,
    const x86_debug_segment_snapshot *segment, const char *label)
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

static void command_print_system_segment(command_context *debugContext,
    const x86_debug_segment_snapshot *segment, const char *label)
{
    command_printf(debugContext, "%s=%04X, Base=%08X, Limit=%08X, DPL=%01X, Type=%04X\n",
        label, segment->selector, segment->base, segment->limit, segment->dpl,
        segment->type);
}

static void command_print_segments(command_context *debugContext)
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

static void command_print_controls(command_context *debugContext)
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

static void command_print_watches(command_context *debugContext)
{
    lib_u32 address;

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

static void command_print_memory_accesses(command_context *debugContext)
{
    lib_u8 index;

    for (index = 0u; index < debugContext->observation.count; ++index) {
        const x86_debug_memory_access *access =
            &debugContext->observation.accesses[index];
        command_printf(debugContext, "%s: Lin=%08x, Data=%08x%08x, Bytes=%x\n",
            access->write ? "Write" : "Read", access->linear,
            (lib_u32)(access->data >> 32u),
            (lib_u32)access->data, access->bytes);
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
#define _ax ((lib_u16)_eax)
#define _cx ((lib_u16)_ecx)
#define _dx ((lib_u16)_edx)
#define _bx ((lib_u16)_ebx)
#define _sp ((lib_u16)_esp)
#define _bp ((lib_u16)_ebp)
#define _si ((lib_u16)_esi)
#define _di ((lib_u16)_edi)
#define _ip ((lib_u16)_eip)
#define _es ((lib_u16)debug_register(debugContext, COMMAND_REGISTER_ES))
#define _cs ((lib_u16)debug_register(debugContext, COMMAND_REGISTER_CS))
#define _ss ((lib_u16)debug_register(debugContext, COMMAND_REGISTER_SS))
#define _ds ((lib_u16)debug_register(debugContext, COMMAND_REGISTER_DS))
#define _fs ((lib_u16)debug_register(debugContext, COMMAND_REGISTER_FS))
#define _gs ((lib_u16)debug_register(debugContext, COMMAND_REGISTER_GS))

static void seterr(command_context *debugContext, lib_size pos)
{
    nErrPos = (lib_size)(arg[pos] - strCmdCopy + lib_text_length(arg[pos]) + 1);
}
static lib_u8 scannubit8(command_context *debugContext, char *s)
{
    lib_u8 ans = 0;
    lib_size i = 0;
    if (lib_text_length(s) == 3u && s[0] == '\'' && s[2] == '\'')
    {
        return s[1];
    }
    command_text_lower(s);
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
static lib_u16 scannubit16(command_context *debugContext, char *s)
{
    lib_u16 ans = 0;
    lib_size i = 0;
    command_text_lower(s);
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
static lib_u32 scannubit32(command_context *debugContext, char *s)
{
    lib_u32 ans = 0;
    lib_size i = 0;
    command_text_lower(s);
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

static void addrparse(command_context *debugContext, lib_u16 defseg, const char *addr)
{
    char *colon;
    char copy[0x100];
    if (!command_copy_text_checked(copy, sizeof(copy), addr)) return;
    command_text_lower(copy);
    colon = lib_text_find_character(copy, ':');
    seg = defseg;
    if (colon != LIB_NULL)
    {
        *colon++ = '\0';
        if (copy[0] == '\0' || colon[0] == '\0' || lib_text_find_character(colon, ':'))
        {
            seterr(debugContext, narg - 1u);
            return;
        }
        if (!lib_text_compare(copy, "es")) seg = _es;
        else if (!lib_text_compare(copy, "cs")) seg = _cs;
        else if (!lib_text_compare(copy, "ss")) seg = _ss;
        else if (!lib_text_compare(copy, "ds")) seg = _ds;
        else seg = scannubit16(debugContext, copy);
    }
    ptr = scannubit16(debugContext, colon != LIB_NULL ? colon : copy);
}

/* Consume a real range, leaving the next argument for a destination/list.
 * A 32-bit count represents the complete 64 KiB segment without wrapping. */
static lib_u32 scanrange(command_context *debugContext,
    lib_u16 default_segment, lib_u32 default_count,
    lib_size *next)
{
    lib_u32 count, end;
    lib_size i = 2u;
    if (narg < 2u) { seterr(debugContext, 0u); return 0u; }
    addrparse(debugContext, default_segment, arg[1]);
    count = default_count;
    if (count > 0x10000u - ptr) count = 0x10000u - ptr;
    if (i < narg)
    {
        if (arg[i][0] == 'l')
        {
            char *length = arg[i++] + 1;
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
static lib_size scanlist(command_context *debugContext, lib_size first,
    lib_u8 *bytes)
{
    lib_size count = 0u, i;
    for (i = first; i < narg; ++i)
    {
        char *p = arg[i];
        if (*p == '\'' || *p == '"')
        {
            char quote = *p++;
            while (*p)
            {
                if (*p == quote)
                {
                    if (p[1] != quote) break;
                    ++p;
                }
                bytes[count++] = (lib_u8)*p++;
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
static void aconsole(command_context *debugContext)
{
    lib_size i, len, errAsmPos;
    char cmdAsmBuff[0x100];
    lib_u8 acode[15];
    lib_i32 flagExitAsm = 0;
    while (!flagExitAsm)
    {
        command_printf(debugContext, "%04X:%04X ", asmSegRec, asmPtrRec);
        if (!command_read_line(debugContext, cmdAsmBuff, sizeof(cmdAsmBuff))) return;
        command_text_lower(cmdAsmBuff);
        if (!lib_text_length(cmdAsmBuff))
        {
            flagExitAsm = 1;
            continue;
        }
        if (cmdAsmBuff[0] == ';')
        {
            continue;
        }
        errAsmPos = 0;
        if (x86_xasm32_assemble(cmdAsmBuff, lib_text_length(cmdAsmBuff),
                acode, sizeof(acode), &len,
                command_code_property(debugContext, X86_DEBUG_GET_CODE_DEFAULT_SIZE)) != LIB_STATUS_OK) {
            len = 0u;
        }
        if (!len)
        {
            errAsmPos = lib_text_length(cmdAsmBuff) + 9;
        }
        else
        {
            if (command_write_linear(debugContext, (asmSegRec << 4) + asmPtrRec, (void *)acode, (lib_u8)len))
            {
                command_printf(debugContext, "debug: fail to write to L%08X\n", (asmSegRec << 4) + asmPtrRec);
                return;
            }
            asmPtrRec += (lib_u16)len;
        }
        if (errAsmPos)
        {
            for (i = 0; i < errAsmPos; ++i)
            {
                command_printf(debugContext, " ");
            }
            command_printf(debugContext, "^ Error\n");
        }
    }
}
static void a(command_context *debugContext)
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
static void c(command_context *debugContext)
{
    lib_size next;
    lib_u32 i, count = scanrange(debugContext, _ds, 128u, &next);
    lib_u16 seg1 = seg, ptr1 = ptr;
    lib_u8 val1, val2;
    if (nErrPos) return;
    if (next + 1u != narg) { seterr(debugContext, narg - 1u); return; }
    addrparse(debugContext, _ds, arg[next]);
    if (nErrPos) return;
    for (i = 0u; i < count; ++i)
    {
        if (command_read_real(debugContext, seg1, (lib_u16)(ptr1 + i), &val1, 1) ||
            command_read_real(debugContext, seg, (lib_u16)(ptr + i), &val2, 1)) return;
        if (val1 != val2)
        {
            command_printf(debugContext, "%04X:%04X  ", seg1, (lib_u16)(ptr1 + i));
            command_printf(debugContext, "%02X  %02X", val1, val2);
            command_printf(debugContext, "  %04X:%04X\n", seg, (lib_u16)(ptr + i));
        }
    }
}

/* dump */
static void dprint(command_context *debugContext, lib_u16 segment, lib_u16 start, lib_u16 end)
{
    char t, c[0x11];
    lib_u16 iaddr;
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
            command_printf(debugContext, "%04X:%04X  ", segment, iaddr);
        }
        if (iaddr < start || iaddr > end)
        {
            command_printf(debugContext, "  ");
            c[iaddr % 0x10] = ' ';
        }
        else
        {
            command_read_real(debugContext, segment, iaddr, (void *)(&c[iaddr % 0x10]), 1);
            command_printf(debugContext, "%02X", c[iaddr % 0x10] & 0xff);
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
        command_printf(debugContext, " ");
        if (iaddr % 0x10 == 7 && iaddr >= start && iaddr < end)
        {
            command_printf(debugContext, "\b-");
        }
        if ((iaddr + 1) % 0x10 == 0)
        {
            command_printf(debugContext, "  %s\n", c);
        }
        if (iaddr == 0xffff)
        {
            break;
        }
    }
    dumpSegRec = segment;
    dumpPtrRec = end + 1;
}
static void d(command_context *debugContext)
{
    lib_size next;
    lib_u32 count;
    if (narg == 1u)
        dprint(debugContext, dumpSegRec, dumpPtrRec,
            (lib_u16)(dumpPtrRec + 0x7fu));
    else
    {
        count = scanrange(debugContext, _ds, 128u, &next);
        if (nErrPos) return;
        if (next != narg) { seterr(debugContext, next); return; }
        dprint(debugContext, seg, ptr, (lib_u16)(ptr + count - 1u));
    }
}

/* enter */
static void e(command_context *debugContext)
{
    lib_size i;
    lib_u8 val;
    char s[0x100];
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
        command_printf(debugContext, "%04X:%04X  ", seg, ptr);
        command_read_real(debugContext, seg, ptr, (void *)(&val), 1);
        command_printf(debugContext, "%02X", val);
        command_printf(debugContext, ".");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        command_text_lower(s);              /* MARK */
        val = scannubit8(debugContext, s); /* MARK */
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            command_write_real(debugContext, seg, ptr, (void *)(&val), 1);
        }
    }
    else
    {
        lib_u8 bytes[X86_DEBUG_LINE_CAPACITY];
        lib_size count;
        addrparse(debugContext, _ds, arg[1]);
        count = scanlist(debugContext, 2u, bytes);
        if (nErrPos) return;
        for (i = 0u; i < count; ++i)
            if (command_write_real(debugContext, seg, (lib_u16)(ptr + i),
                    &bytes[i], 1)) return;
    }
}
/* fill */
static void f(command_context *debugContext)
{
    lib_size next, length;
    lib_u32 i, count = scanrange(debugContext, _ds, 128u, &next);
    lib_u8 bytes[X86_DEBUG_LINE_CAPACITY];
    if (nErrPos) return;
    length = scanlist(debugContext, next, bytes);
    if (nErrPos) return;
    for (i = 0u; i < count; ++i)
        if (command_write_real(debugContext, seg, (lib_u16)(ptr + i),
                &bytes[i % length], 1)) return;
}

/* go */
static void rprintregs(command_context *debugContext);
static void g(command_context *debugContext)
{
    lib_u16 start_segment, start_offset;
    if ((!command_is_paused(debugContext)))
    {
        command_printf(debugContext, "Machine is already running.\n");
        return;
    }
    switch (narg)
    {
    case 1:
        command_clear_break(debugContext);
        break;
    case 2:
        addrparse(debugContext, _cs, arg[1]);
        if (nErrPos) return;
        if (command_set_break_real(debugContext, seg, ptr)) return;
        break;
    case 3:
        addrparse(debugContext, _cs, arg[1]);
        start_segment = seg; start_offset = ptr;
        addrparse(debugContext, start_segment, arg[2]);
        if (nErrPos) return;
        if (debug_set_register(debugContext, COMMAND_REGISTER_CS, start_segment))
        {
            command_printf(debugContext, "debug: fail to load cs from %04X\n", start_segment);
            return;
        }
        if (debug_set_word(debugContext, COMMAND_REGISTER_EIP, start_offset)) return;
        if (command_set_break_real(debugContext, seg, ptr)) return;
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
    command_resume(debugContext);
}
/* hex */
static void h(command_context *debugContext)
{
    lib_u16 val1, val2;
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
            command_printf(debugContext, "%04X", (lib_u16)(val1 + val2));
            command_printf(debugContext, "  ");
            command_printf(debugContext, "%04X", (lib_u16)(val1 - val2));
            command_printf(debugContext, "\n");
        }
    }
}
/* input */
static void i(command_context *debugContext)
{
    lib_u16 in;
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
        command_printf(debugContext, "%08X\n", command_read_port_value(debugContext, in));
    }
}
/* load */
static void l(command_context *debugContext)
{
    lib_u16 i = 0;
    lib_u32 len = 0;
    lib_storage_medium *medium = LIB_NULL;
    lib_size bytes;
    lib_size total;
    lib_status status;
    lib_u8 data[X86_DEBUG_BYTES];

    if (!lib_text_length(strFileName)) {
        command_printf(debugContext, "File not found\n");
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
            command_write_real(debugContext, seg + i, ptr + len++, &data[index], 1);
            i = (lib_u16)(len / 0x10000u);
        }
    }
    lib_storage_medium_destroy(&medium);
    if (status != LIB_STATUS_OK) command_printf(debugContext, "File not found\n");
    debug_set_word(debugContext, COMMAND_REGISTER_ECX, (lib_u16)(len & 0xffffu));
    debug_set_word(debugContext, COMMAND_REGISTER_EBX, (lib_u16)(len >> 16u));
}
/* move */
static void m(command_context *debugContext)
{
    lib_size next;
    lib_u32 i, offset, count = scanrange(debugContext, _ds, 128u, &next);
    lib_u16 seg1 = seg, ptr1 = ptr;
    lib_u8 val;
    lib_i32 backward;
    if (nErrPos) return;
    if (next + 1u != narg) { seterr(debugContext, narg - 1u); return; }
    addrparse(debugContext, _ds, arg[next]);
    if (nErrPos) return;
    if (((seg1 << 4) + ptr1) == ((seg << 4) + ptr)) return;
    backward = ((seg1 << 4) + ptr1) < ((seg << 4) + ptr);
    for (i = 0u; i < count; ++i)
    {
        offset = backward ? count - 1u - i : i;
        if (command_read_real(debugContext, seg1, (lib_u16)(ptr1 + offset), &val, 1) ||
            command_write_real(debugContext, seg, (lib_u16)(ptr + offset), &val, 1)) return;
    }
}

/* name */
static void n(command_context *debugContext)
{
    if (narg != 2)
        seterr(debugContext, narg - 1);
    else
        if (!command_copy_text_checked(strFileName, sizeof(strFileName),
                arg[1])) seterr(debugContext, 1u);
}
/* output */
static void o(command_context *debugContext)
{
    lib_u16 out;
    lib_u32 value;
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
        command_write_port(debugContext, out, value);
    }
}
/* quit */
static void q(command_context *debugContext)
{
    flagExit = 1;
}
/* register */
static lib_u8 uprintins(command_context *debugContext, lib_u16 segment, lib_u16 off)
{
    lib_size i;
    lib_u8 first = off > 0xfff1u ? (lib_u8)(0x10000u - off) : 15u;
    lib_size sbin_remaining;
    lib_i32 binary_failed = LIB_FALSE;
    lib_i32 format_result;
    lib_u8 len;
    lib_u8 ucode[15];
    char str[0x100], stmt[0x100], sbin[0x100];
    char *sbin_cursor;
    if (command_read_real(debugContext, segment, off, ucode, first) ||
        (first < 15u && command_read_real(debugContext, segment, 0u, ucode + first, 15u - first)))
    {
        len = 0;
        (void)lib_c_snprintf(str, sizeof(str), "%04X:%04X <ERROR>", segment, off);
    }
    else
    {
        lib_size instruction_bytes = 0u;
        if (x86_xasm32_disassemble(ucode, sizeof(ucode), stmt,
                sizeof(stmt), &i, &instruction_bytes,
                0) != LIB_STATUS_OK) {
            len = 0u;
            (void)lib_c_snprintf(stmt, sizeof(stmt), "<ERROR>");
        } else {
            len = (lib_u8)instruction_bytes;
        }
        sbin[0] = 0;
        sbin_cursor = sbin;
        sbin_remaining = sizeof(sbin);
        for (i = 0; i < len; ++i)
        {
            format_result = command_format_append(&sbin_cursor, &sbin_remaining,
                "%02X", (lib_u8)ucode[i]);
            if (format_result < 0) {
                binary_failed = LIB_TRUE;
                len = 0;
                (void)lib_c_snprintf(str, sizeof(str), "%04X:%04X <ERROR>",
                    segment, off);
                break;
            }
        }
        if (!binary_failed) {
            (void)lib_c_snprintf(str, sizeof(str), "%04X:%04X %s", segment, off,
                sbin);
        }
        for (i = lib_text_length(str); i < 24; ++i)
        {
            if (!command_append_text_checked(str, sizeof(str), " ")) {
                len = 0u;
                (void)lib_c_snprintf(str, sizeof(str), "%04X:%04X <ERROR>",
                    segment, off);
                break;
            }
        }
        if (!command_append_text_checked(str, sizeof(str), stmt)) {
            len = 0u;
            (void)lib_c_snprintf(str, sizeof(str), "%04X:%04X <ERROR>",
                segment, off);
        }
    }
    command_printf(debugContext, "%s\n", str);
    return len;
}
static void rprintflags(command_context *debugContext)
{
    command_printf(debugContext, "%s ", debug_flag(debugContext, 0x0800u) ? "OV" : "NV");
    command_printf(debugContext, "%s ", debug_flag(debugContext, 0x0400u) ? "DN" : "UP");
    command_printf(debugContext, "%s ", debug_flag(debugContext, 0x0200u) ? "EI" : "DI");
    command_printf(debugContext, "%s ", debug_flag(debugContext, 0x0080u) ? "NG" : "PL");
    command_printf(debugContext, "%s ", debug_flag(debugContext, 0x0040u) ? "ZR" : "NZ");
    command_printf(debugContext, "%s ", debug_flag(debugContext, 0x0010u) ? "AC" : "NA");
    command_printf(debugContext, "%s ", debug_flag(debugContext, 0x0004u) ? "PE" : "PO");
    command_printf(debugContext, "%s ", debug_flag(debugContext, 0x0001u) ? "CY" : "NC");
}
static void rprintregs(command_context *debugContext)
{
    command_printf(debugContext, "AX=%04X", _ax);
    command_printf(debugContext, "  BX=%04X", _bx);
    command_printf(debugContext, "  CX=%04X", _cx);
    command_printf(debugContext, "  DX=%04X", _dx);
    command_printf(debugContext, "  SP=%04X", _sp);
    command_printf(debugContext, "  BP=%04X", _bp);
    command_printf(debugContext, "  SI=%04X", _si);
    command_printf(debugContext, "  DI=%04X", _di);
    command_printf(debugContext, "\nDS=%04X", _ds);
    command_printf(debugContext, "  ES=%04X", _es);
    command_printf(debugContext, "  SS=%04X", _ss);
    command_printf(debugContext, "  CS=%04X", _cs);
    command_printf(debugContext, "  IP=%04X", _ip);
    command_printf(debugContext, "   ");
    rprintflags(debugContext);
    command_printf(debugContext, "\n");
    uprintins(debugContext, _cs, _ip);
    uasmSegRec = _cs;
    uasmPtrRec = _ip;
}
static void rscanregs(command_context *debugContext)
{
    lib_u16 value;
    char s[0x100];
    if (!lib_text_compare(arg[1], "ax"))
    {
        command_printf(debugContext, "AX ");
        command_printf(debugContext, "%04X", _ax);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_word(debugContext, COMMAND_REGISTER_EAX, value);
        }
    }
    else if (!lib_text_compare(arg[1], "bx"))
    {
        command_printf(debugContext, "BX ");
        command_printf(debugContext, "%04X", _bx);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_word(debugContext, COMMAND_REGISTER_EBX, value);
        }
    }
    else if (!lib_text_compare(arg[1], "cx"))
    {
        command_printf(debugContext, "CX ");
        command_printf(debugContext, "%04X", _cx);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_word(debugContext, COMMAND_REGISTER_ECX, value);
        }
    }
    else if (!lib_text_compare(arg[1], "dx"))
    {
        command_printf(debugContext, "DX ");
        command_printf(debugContext, "%04X", _dx);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_word(debugContext, COMMAND_REGISTER_EDX, value);
        }
    }
    else if (!lib_text_compare(arg[1], "bp"))
    {
        command_printf(debugContext, "BP ");
        command_printf(debugContext, "%04X", _bp);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_word(debugContext, COMMAND_REGISTER_EBP, value);
        }
    }
    else if (!lib_text_compare(arg[1], "sp"))
    {
        command_printf(debugContext, "SP ");
        command_printf(debugContext, "%04X", _sp);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_word(debugContext, COMMAND_REGISTER_ESP, value);
        }
    }
    else if (!lib_text_compare(arg[1], "si"))
    {
        command_printf(debugContext, "SI ");
        command_printf(debugContext, "%04X", _si);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_word(debugContext, COMMAND_REGISTER_ESI, value);
        }
    }
    else if (!lib_text_compare(arg[1], "di"))
    {
        command_printf(debugContext, "DI ");
        command_printf(debugContext, "%04X", _di);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_word(debugContext, COMMAND_REGISTER_EDI, value);
        }
    }
    else if (!lib_text_compare(arg[1], "ss"))
    {
        command_printf(debugContext, "SS ");
        command_printf(debugContext, "%04X", _ss);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, COMMAND_REGISTER_SS, (lib_u16)value))
            {
                command_printf(debugContext, "debug: fail to load ss from %04X\n", (lib_u16)value);
            }
        }
    }
    else if (!lib_text_compare(arg[1], "cs"))
    {
        command_printf(debugContext, "CS ");
        command_printf(debugContext, "%04X", _cs);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, COMMAND_REGISTER_CS, (lib_u16)value))
            {
                command_printf(debugContext, "debug: fail to load cs from %04X\n", (lib_u16)value);
            }
        }
    }
    else if (!lib_text_compare(arg[1], "ds"))
    {
        command_printf(debugContext, "DS ");
        command_printf(debugContext, "%04X", _ds);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, COMMAND_REGISTER_DS, (lib_u16)value))
            {
                command_printf(debugContext, "debug: fail to load ds from %04X\n", (lib_u16)value);
            }
        }
    }
    else if (!lib_text_compare(arg[1], "es"))
    {
        command_printf(debugContext, "ES ");
        command_printf(debugContext, "%04X", _es);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, COMMAND_REGISTER_ES, (lib_u16)value))
            {
                command_printf(debugContext, "debug: fail to load es from %04X\n", (lib_u16)value);
            }
        }
    }
    else if (!lib_text_compare(arg[1], "ip"))
    {
        command_printf(debugContext, "IP ");
        command_printf(debugContext, "%04X", _ip);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_word(debugContext, COMMAND_REGISTER_EIP, value);
        }
    }
    else if (!lib_text_compare(arg[1], "f"))
    {
        rprintflags(debugContext);
        command_printf(debugContext, " -");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        command_text_lower(s);
        if (!lib_text_compare(s, "ov"))
        {
            debug_set_flag(debugContext, 0x0800u, 1);
        }
        else if (!lib_text_compare(s, "nv"))
        {
            debug_set_flag(debugContext, 0x0800u, 0);
        }
        else if (!lib_text_compare(s, "dn"))
        {
            debug_set_flag(debugContext, 0x0400u, 1);
        }
        else if (!lib_text_compare(s, "up"))
        {
            debug_set_flag(debugContext, 0x0400u, 0);
        }
        else if (!lib_text_compare(s, "ei"))
        {
            debug_set_flag(debugContext, 0x0200u, 1);
        }
        else if (!lib_text_compare(s, "di"))
        {
            debug_set_flag(debugContext, 0x0200u, 0);
        }
        else if (!lib_text_compare(s, "ng"))
        {
            debug_set_flag(debugContext, 0x0080u, 1);
        }
        else if (!lib_text_compare(s, "pl"))
        {
            debug_set_flag(debugContext, 0x0080u, 0);
        }
        else if (!lib_text_compare(s, "zr"))
        {
            debug_set_flag(debugContext, 0x0040u, 1);
        }
        else if (!lib_text_compare(s, "nz"))
        {
            debug_set_flag(debugContext, 0x0040u, 0);
        }
        else if (!lib_text_compare(s, "ac"))
        {
            debug_set_flag(debugContext, 0x0010u, 1);
        }
        else if (!lib_text_compare(s, "na"))
        {
            debug_set_flag(debugContext, 0x0010u, 0);
        }
        else if (!lib_text_compare(s, "pe"))
        {
            debug_set_flag(debugContext, 0x0004u, 1);
        }
        else if (!lib_text_compare(s, "po"))
        {
            debug_set_flag(debugContext, 0x0004u, 0);
        }
        else if (!lib_text_compare(s, "cy"))
        {
            debug_set_flag(debugContext, 0x0001u, 1);
        }
        else if (!lib_text_compare(s, "nc"))
        {
            debug_set_flag(debugContext, 0x0001u, 0);
        }
        else
        {
            command_printf(debugContext, "bf Error\n");
        }
    }
    else
    {
        command_printf(debugContext, "br Error\n");
    }
}
static void r(command_context *debugContext)
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
static void s(command_context *debugContext)
{
    lib_size next, length, j;
    lib_u32 i, count = scanrange(debugContext, _ds, 128u, &next);
    lib_u8 bytes[X86_DEBUG_LINE_CAPACITY], val;
    if (nErrPos) return;
    length = scanlist(debugContext, next, bytes);
    if (nErrPos || count < length) return;
    for (i = 0u; i <= count - length; ++i)
    {
        for (j = 0u; j < length; ++j)
        {
            if (command_read_real(debugContext, seg, (lib_u16)(ptr + i + j), &val, 1)) return;
            if (val != bytes[j]) break;
        }
        if (j == length) command_printf(debugContext, "%04X:%04X  \n", seg, (lib_u16)(ptr + i));
    }
}

/* trace */
static void t(command_context *debugContext)
{
    lib_u16 count;
    if ((!command_is_paused(debugContext)))
    {
        command_printf(debugContext, "Machine is already running.\n");
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
            command_printf(debugContext, "debug: fail to load cs from %04X\n", seg);
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
    command_resume(debugContext);
}
/* unassemble */
static void uprint(command_context *debugContext, lib_u16 segment, lib_u16 start, lib_u16 end)
{
    lib_u8 len;
    lib_u32 boundary;
    if (start > end)
    {
        end = 0xffff;
    }
    while (start <= end)
    {
        len = uprintins(debugContext, segment, start);
        if (len == 0u) break;
        boundary = (lib_u32)start + (lib_u32)len;
        start = (lib_u16)boundary;
        if (boundary > 0xffff)
        {
            break;
        }
    }
    uasmSegRec = segment;
    uasmPtrRec = start;
    return;
}
static void u(command_context *debugContext)
{
    lib_size next;
    lib_u32 count;
    if (narg == 1u)
        uprint(debugContext, uasmSegRec, uasmPtrRec,
            (lib_u16)(uasmPtrRec + 0x1fu));
    else
    {
        count = scanrange(debugContext, _cs, 32u, &next);
        if (nErrPos) return;
        if (next != narg) { seterr(debugContext, next); return; }
        uprint(debugContext, seg, ptr, (lib_u16)(ptr + count - 1u));
    }
}

/* verbal */
static void v(command_context *debugContext)
{
    lib_size i;
    char str[0x100];
    command_printf(debugContext, ":");
    if (!command_read_line(debugContext, str, sizeof(str))) return;
    if (lib_text_length(str) != 0u && str[lib_text_length(str) - 1u] == '\n')
        str[lib_text_length(str) - 1u] = '\0';
    for (i = 0; i < lib_text_length(str); ++i)
    {
        command_printf(debugContext, "%02X", str[i]);
        if (!((i + 1) % 0x10))
        {
            command_printf(debugContext, "\n");
        }
        else if (!((i + 1) % 0x08) && (str[i + 1] != '\0'))
        {
            command_printf(debugContext, "-");
        }
        else
        {
            command_printf(debugContext, " ");
        }
    }
    if (i % 0x10)
    {
        command_printf(debugContext, "\n");
    }
}
/* write */
static void w(command_context *debugContext)
{
    lib_size i = 0;
    lib_u32 len = (_bx << 16) + _cx;
    lib_u8 data[X86_DEBUG_BYTES];
    lib_storage_file_writer *writer = LIB_NULL;
    lib_status status;
    if (!lib_text_length(strFileName))
    {
        command_printf(debugContext, "(W)rite error, no destination defined\n");
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
        command_printf(debugContext, "File not found\n");
        return;
    }
    command_printf(debugContext, "Writing ");
    command_printf(debugContext, "%04X", _bx);
    command_printf(debugContext, "%04X", _cx);
    command_printf(debugContext, " bytes\n");
    while (i < len) {
        lib_size count = len - i < sizeof(data) ? len - i : sizeof(data);
        lib_size index;
        for (index = 0u; index < count; ++index)
            command_read_real(debugContext, seg, (lib_u16)(ptr + i + index), &data[index], 1);
        if (debugContext->access_status != LIB_STATUS_OK) break;
        if (lib_storage_file_writer_write(writer, data, count) !=
                LIB_STATUS_OK) {
            command_printf(debugContext, "File write failed\n");
            break;
        }
        i += count;
    }
    if (lib_storage_file_writer_close(writer) != LIB_STATUS_OK)
        command_printf(debugContext, "File write failed\n");
}
/* DEBUG CMD END */

/* EXTENDED DEBUG CMD BEGIN */
#define xalin debugContext->assemble_linear
#define xdlin debugContext->dump_linear
#define xulin debugContext->unassemble_linear
static lib_i32 xcheckrange(command_context *debugContext,
    lib_u32 linear, lib_u32 count)
{
    if (count != 0u && count - 1u > LIB_UINT32_MAX - linear) {
        seterr(debugContext, narg - 1);
        return 0;
    }
    return 1;
}
/* print */
static lib_u8 xuprintins(command_context *debugContext, lib_u32 linear)
{
    lib_size i;
    lib_size sbin_remaining;
    lib_i32 binary_failed = LIB_FALSE;
    lib_i32 format_result;
    lib_u8 len;
    /* The decoder needs 15 host bytes; accept only bytes read before address end. */
    lib_u8 ucode[15] = {0};
    lib_u8 available = linear > LIB_UINT32_MAX - 14u ?
        (lib_u8)(LIB_UINT32_MAX - linear + 1u) : 15u;
    char str[0x100], stmt[0x100], sbin[0x100];
    char *sbin_cursor;
    if (command_read_linear(debugContext, linear, (void *)ucode, available))
    {
        len = 0;
        (void)lib_c_snprintf(str, sizeof(str), "L%08X <ERROR>", linear);
    }
    else
    {
        lib_size instruction_bytes = 0u;
        if (x86_xasm32_disassemble(ucode, sizeof(ucode), stmt,
                sizeof(stmt), &i, &instruction_bytes,
                command_code_property(debugContext, X86_DEBUG_GET_CODE_DEFAULT_SIZE)) != LIB_STATUS_OK ||
            instruction_bytes > available) {
            len = 0u;
            (void)lib_c_snprintf(stmt, sizeof(stmt), "<ERROR>");
        } else {
            len = (lib_u8)instruction_bytes;
        }
        sbin[0] = 0;
        sbin_cursor = sbin;
        sbin_remaining = sizeof(sbin);
        for (i = 0; i < len; ++i)
        {
            format_result = command_format_append(&sbin_cursor, &sbin_remaining,
                "%02X", (lib_u8)ucode[i]);
            if (format_result < 0) {
                binary_failed = LIB_TRUE;
                len = 0;
                (void)lib_c_snprintf(str, sizeof(str), "L%08X <ERROR>", linear);
                break;
            }
        }
        if (!binary_failed) {
            (void)lib_c_snprintf(str, sizeof(str), "L%08X %s ", linear, sbin);
        }
        for (i = lib_text_length(str); i < 24; ++i)
        {
            if (!command_append_text_checked(str, sizeof(str), " ")) {
                len = 0u;
                (void)lib_c_snprintf(str, sizeof(str), "L%08X <ERROR>", linear);
                break;
            }
        }
        if (!command_append_text_checked(str, sizeof(str), stmt)) {
            len = 0u;
            (void)lib_c_snprintf(str, sizeof(str), "L%08X <ERROR>", linear);
        }
    }
    command_printf(debugContext, "%s\n", str);
    return len;
}
static void xrprintreg(command_context *debugContext)
{
    lib_u32 flags = _eflags;
    command_printf(debugContext, "EAX=%08X", _eax);
    command_printf(debugContext, " EBX=%08X", _ebx);
    command_printf(debugContext, " ECX=%08X", _ecx);
    command_printf(debugContext, " EDX=%08X", _edx);
    command_printf(debugContext, "\nESP=%08X", _esp);
    command_printf(debugContext, " EBP=%08X", _ebp);
    command_printf(debugContext, " ESI=%08X", _esi);
    command_printf(debugContext, " EDI=%08X", _edi);
    command_printf(debugContext, "\nEIP=%08X", _eip);
    command_printf(debugContext, " EFL=%08X", flags);
    command_printf(debugContext, ": ");
    command_printf(debugContext, "%s ", flags & 0x20000u ? "VM" : "vm");
    command_printf(debugContext, "%s ", flags & 0x10000u ? "RF" : "rf");
    command_printf(debugContext, "%s ", flags & 0x04000u ? "NT" : "nt");
    command_printf(debugContext, "IOPL=%01X ", (flags >> 12u) & 3u);
    command_printf(debugContext, "%s ", flags & 0x00800u ? "OF" : "of");
    command_printf(debugContext, "%s ", flags & 0x00400u ? "DF" : "df");
    command_printf(debugContext, "%s ", flags & 0x00200u ? "IF" : "if");
    command_printf(debugContext, "%s ", flags & 0x00100u ? "TF" : "tf");
    command_printf(debugContext, "%s ", flags & 0x00080u ? "SF" : "sf");
    command_printf(debugContext, "%s ", flags & 0x00040u ? "ZF" : "zf");
    command_printf(debugContext, "%s ", flags & 0x00010u ? "AF" : "af");
    command_printf(debugContext, "%s ", flags & 0x00004u ? "PF" : "pf");
    command_printf(debugContext, "%s ", flags & 0x00001u ? "CF" : "cf");
    command_printf(debugContext, "\n");
    xulin = command_code_property(debugContext, X86_DEBUG_GET_CODE_BASE) + _eip;
    xuprintins(debugContext, xulin);
}
/* assemble */
static void xaconsole(command_context *debugContext)
{
    lib_size i, len, errAsmPos;
    char astmt[0x100];
    lib_u8 acode[15];
    lib_i32 flagExitAsm = 0;
    while (!flagExitAsm)
    {
        command_printf(debugContext, "L%08X ", xalin);
        if (!command_read_line(debugContext, astmt, sizeof(astmt))) return;
        if (lib_text_length(astmt) != 0u && astmt[lib_text_length(astmt) - 1u] == '\n')
            astmt[lib_text_length(astmt) - 1u] = '\0';
        if (!lib_text_length(astmt))
        {
            flagExitAsm = 1;
            continue;
        }
        errAsmPos = 0;
        if (x86_xasm32_assemble(astmt, lib_text_length(astmt), acode,
                sizeof(acode), &len,
                command_code_property(debugContext, X86_DEBUG_GET_CODE_DEFAULT_SIZE)) != LIB_STATUS_OK) {
            len = 0u;
        }
        if (!len)
        {
            errAsmPos = lib_text_length(astmt) + 9;
        }
        else
        {
            if (!xcheckrange(debugContext, xalin, (lib_u32)len)) return;
            if (command_write_linear(debugContext, xalin, (void *)acode, (lib_u8)len))
            {
                command_printf(debugContext, "debug: fail to write to L%08X\n", xalin);
                return;
            }
            if (len > LIB_UINT32_MAX - xalin) return;
            xalin += (lib_u32)len;
        }
        if (errAsmPos)
        {
            for (i = 0; i < errAsmPos; ++i)
            {
                command_printf(debugContext, " ");
            }
            command_printf(debugContext, "^ Error\n");
        }
    }
}
static void xa(command_context *debugContext)
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
static void xc(command_context *debugContext)
{
    lib_size i, count;
    lib_u32 lin1, lin2;
    lib_u8 val1, val2;
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
        if (!xcheckrange(debugContext, lin1, (lib_u32)count) ||
            !xcheckrange(debugContext, lin2, (lib_u32)count)) return;
        for (i = 0; i < count; ++i)
        {
            if (command_read_linear(debugContext, (lib_u32)(lin1 + i), (void *)(&val1), 1))
            {
                command_printf(debugContext, "debug: fail to read from L%08X.\n", (lib_u32)(lin1 + i));
                return;
            }
            if (command_read_linear(debugContext, (lib_u32)(lin2 + i), (void *)(&val2), 1))
            {
                command_printf(debugContext, "debug: fail to read from L%08X.\n", (lib_u32)(lin2 + i));
                return;
            }
            if (val1 != val2)
                command_printf(debugContext, "L%08X  %02X  %02X  L%08X\n",
                           (lib_u32)(lin1 + i), val1, val2, (lib_u32)(lin2 + i));
        }
    }
}
/* dump */
static void xdprint(command_context *debugContext, lib_u32 linear, lib_u32 count)
{
    char t, c[0x11];
    lib_u32 ilinear;
    lib_u32 start = linear;
    lib_u32 end;
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
            command_printf(debugContext, "L%08X  ", ilinear);
        if (ilinear < start || ilinear > end)
        {
            command_printf(debugContext, "  ");
            c[ilinear % 0x10] = ' ';
        }
        else
        {
            if (command_read_linear(debugContext, ilinear, (void *)(&c[ilinear % 0x10]), 1))
            {
                command_printf(debugContext, "debug: fail to read from L%08X\n", ilinear);
                return;
            }
            else
            {
                command_printf(debugContext, "%02X", c[ilinear % 0x10] & 0xff);
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
        command_printf(debugContext, " ");
        if (ilinear % 0x10 == 7 && ilinear >= start && ilinear < end)
            command_printf(debugContext, "\b-");
        if (ilinear % 0x10 == 0x0f)
        {
            command_printf(debugContext, "  %s\n", c);
        }
        if (ilinear == 0xffffffff)
        {
            break;
        }
    }
    xdlin = ilinear;
}
static void xd(command_context *debugContext)
{
    lib_u32 count;
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
static void xe(command_context *debugContext)
{
    lib_size i;
    lib_u8 val;
    lib_u32 linear;
    char s[0x100];
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
        if (command_read_linear(debugContext, linear, (void *)(&val), 1))
        {
            command_printf(debugContext, "debug: fail to read from L%08X.\n", linear);
            return;
        }
        command_printf(debugContext, "L%08X  %02X.", linear, val);
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        command_text_lower(s);
        val = scannubit8(debugContext, s);
        if (nErrPos)
        {
            return;
        }
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (command_write_linear(debugContext, linear, (void *)(&val), 1))
            {
                command_printf(debugContext, "debug: fail to write to L%08X.\n", linear);
            }
        }
    }
    else
    {
        lib_u8 bytes[X86_DEBUG_LINE_CAPACITY];
        lib_size count;
        linear = scannubit32(debugContext, arg[1]);
        count = scanlist(debugContext, 2u, bytes);
        if (nErrPos || !xcheckrange(debugContext, linear, (lib_u32)count)) return;
        for (i = 0u; i < count; ++i)
            if (command_write_linear(debugContext, linear + (lib_u32)i,
                    &bytes[i], 1)) return;
    }
}
/* fill */
static void xf(command_context *debugContext)
{
    lib_size i, count, length;
    lib_u32 linear;
    lib_u8 bytes[X86_DEBUG_LINE_CAPACITY];
    if (narg < 4u) { seterr(debugContext, narg - 1u); return; }
    linear = scannubit32(debugContext, arg[1]);
    count = scannubit32(debugContext, arg[2]);
    length = scanlist(debugContext, 3u, bytes);
    if (nErrPos || !xcheckrange(debugContext, linear, (lib_u32)count)) return;
    for (i = 0u; i < count; ++i)
        if (command_write_linear(debugContext, linear + (lib_u32)i,
                &bytes[i % length], 1)) return;
}

/* go */
static void xg(command_context *debugContext)
{
    lib_size count = 0;
    lib_u32 linear;
    if ((!command_is_paused(debugContext)))
    {
        command_printf(debugContext, "Machine is already running.\n");
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
    if (command_set_break(debugContext, linear)) return;
    command_begin_break(debugContext, count);
    command_resume(debugContext);
}
/* move */
static void xm(command_context *debugContext)
{
    lib_u8 val;
    lib_size i;
    lib_u32 lin1, lin2, count, offset;
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
            offset = lin2 > lin1 ? count - 1u - (lib_u32)i : (lib_u32)i;
            if (command_read_linear(debugContext, lin1 + offset, (void *)(&val), 1))
            {
                command_printf(debugContext, "debug: fail to read from L%08X.\n", lin1 + offset);
                return;
            }
            if (command_write_linear(debugContext, lin2 + offset, (void *)(&val), 1))
            {
                command_printf(debugContext, "debug: fail to write to L%08X.\n", lin2 + offset);
                return;
            }
        }
    }
}
/* search */
static void xs(command_context *debugContext)
{
    lib_size i, count, bcount;
    lib_u32 linear;
    lib_u8 mem[256], line[256];
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
        if (nErrPos || !xcheckrange(debugContext, linear, (lib_u32)count)) return;
        if (count < bcount) return;
        for (i = 0; i <= count - bcount; ++i)
        {
            if (command_read_linear(debugContext, (lib_u32)(linear + i), (void *)mem, (lib_u8)bcount))
            {
                command_printf(debugContext, "debug: fail to read from L%08X.\n", linear + i);
                return;
            }
            if (!lib_memory_compare((void *)mem, (void *)line, bcount))
            {
                command_printf(debugContext, "L%08X\n", linear + i);
            }
        }
    }
}
/* trace */
static void xt(command_context *debugContext)
{
    lib_u32 count;
    if ((!command_is_paused(debugContext)))
    {
        command_printf(debugContext, "Machine is already running.\n");
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
    command_resume(debugContext);
}
/* register */
static void xrscanreg(command_context *debugContext)
{
    lib_u32 value;
    char s[0x100];
    if (!lib_text_compare(arg[1], "eax"))
    {
        command_printf(debugContext, "EAX ");
        command_printf(debugContext, "%08X", _eax);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_EAX, value);
        }
    }
    else if (!lib_text_compare(arg[1], "ecx"))
    {
        command_printf(debugContext, "ECX ");
        command_printf(debugContext, "%08X", _ecx);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_ECX, value);
        }
    }
    else if (!lib_text_compare(arg[1], "edx"))
    {
        command_printf(debugContext, "EDX ");
        command_printf(debugContext, "%08X", _edx);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_EDX, value);
        }
    }
    else if (!lib_text_compare(arg[1], "ebx"))
    {
        command_printf(debugContext, "EBX ");
        command_printf(debugContext, "%08X", _ebx);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_EBX, value);
        }
    }
    else if (!lib_text_compare(arg[1], "esp"))
    {
        command_printf(debugContext, "ESP ");
        command_printf(debugContext, "%08X", _esp);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_ESP, value);
        }
    }
    else if (!lib_text_compare(arg[1], "ebp"))
    {
        command_printf(debugContext, "EBP ");
        command_printf(debugContext, "%08X", _ebp);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_EBP, value);
        }
    }
    else if (!lib_text_compare(arg[1], "esi"))
    {
        command_printf(debugContext, "ESI ");
        command_printf(debugContext, "%08X", _esi);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_ESI, value);
        }
    }
    else if (!lib_text_compare(arg[1], "edi"))
    {
        command_printf(debugContext, "EDI ");
        command_printf(debugContext, "%08X", _edi);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_EDI, value);
        }
    }
    else if (!lib_text_compare(arg[1], "eip"))
    {
        command_printf(debugContext, "EIP ");
        command_printf(debugContext, "%08X", _eip);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_EIP, value);
        }
    }
    else if (!lib_text_compare(arg[1], "eflags"))
    {
        command_printf(debugContext, "EFLAGS ");
        command_printf(debugContext, "%08X", _eflags);
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_EFLAGS, value);
        }
    }
    else if (!lib_text_compare(arg[1], "es"))
    {
        command_print_segments(debugContext);
        command_printf(debugContext, ":");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, COMMAND_REGISTER_ES, (lib_u16)value))
            {
                command_printf(debugContext, "debug: fail to load es from %04X\n", (lib_u16)value);
            }
        }
    }
    else if (!lib_text_compare(arg[1], "cs"))
    {
        command_print_segments(debugContext);
        command_printf(debugContext, ":");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, COMMAND_REGISTER_CS, (lib_u16)value))
            {
                command_printf(debugContext, "debug: fail to load cs from %04X\n", (lib_u16)value);
            }
        }
    }
    else if (!lib_text_compare(arg[1], "ss"))
    {
        command_print_segments(debugContext);
        command_printf(debugContext, ":");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, COMMAND_REGISTER_SS, (lib_u16)value))
            {
                command_printf(debugContext, "debug: fail to load ss from %04X\n", (lib_u16)value);
            }
        }
    }
    else if (!lib_text_compare(arg[1], "ds"))
    {
        command_print_segments(debugContext);
        command_printf(debugContext, ":");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, COMMAND_REGISTER_DS, (lib_u16)value))
            {
                command_printf(debugContext, "debug: fail to load ds from %04X\n", (lib_u16)value);
            }
        }
    }
    else if (!lib_text_compare(arg[1], "fs"))
    {
        command_print_segments(debugContext);
        command_printf(debugContext, ":");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, COMMAND_REGISTER_FS, (lib_u16)value))
            {
                command_printf(debugContext, "debug: fail to load fs from %04X\n", (lib_u16)value);
            }
        }
    }
    else if (!lib_text_compare(arg[1], "gs"))
    {
        command_print_segments(debugContext);
        command_printf(debugContext, ":");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit16(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            if (debug_set_register(debugContext, COMMAND_REGISTER_GS, (lib_u16)value))
            {
                command_printf(debugContext, "debug: fail to load gs from %04X\n", (lib_u16)value);
            }
        }
    }
    else if (!lib_text_compare(arg[1], "cr0"))
    {
        command_printf(debugContext, "CR0 ");
        command_printf(debugContext, "%08X", _cr(0));
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_CR0, value);
        }
    }
    else if (!lib_text_compare(arg[1], "cr1"))
    {
        command_printf(debugContext, "CR1 ");
        command_printf(debugContext, "%08X", _cr(1));
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_CR1, value);
        }
    }
    else if (!lib_text_compare(arg[1], "cr2"))
    {
        command_printf(debugContext, "CR2 ");
        command_printf(debugContext, "%08X", _cr(2));
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_CR2, value);
        }
    }
    else if (!lib_text_compare(arg[1], "cr3"))
    {
        command_printf(debugContext, "CR3 ");
        command_printf(debugContext, "%08X", _cr(3));
        command_printf(debugContext, "\n:");
        if (!command_read_line(debugContext, s, sizeof(s))) return;
        value = scannubit32(debugContext, s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos)
        {
            debug_set_register(debugContext, COMMAND_REGISTER_CR3, value);
        }
    }
    else
    {
        command_printf(debugContext, "br Error\n");
    }
}
static void xr(command_context *debugContext)
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
static void xuprint(command_context *debugContext, lib_u32 linear, lib_u32 count)
{
    lib_u32 len = 0;
    lib_size i;
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
static void xu(command_context *debugContext)
{
    lib_u32 count;
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
static void xw(command_context *debugContext)
{
    lib_u32 linear;
    switch (narg)
    {
    case 1:
        command_print_watches(debugContext);
        break;
    case 2:
        switch (arg[1][0])
        {
        case 'r':
            if (command_clear_watch(debugContext, COMMAND_REGISTER_WATCH_READ)) return;
            command_printf(debugContext, "Watch-read point removed.\n");
            break;
        case 'w':
            if (command_clear_watch(debugContext, COMMAND_REGISTER_WATCH_WRITE)) return;
            command_printf(debugContext, "Watch-write point removed.\n");
            break;
        case 'e':
            if (command_clear_watch(debugContext, COMMAND_REGISTER_WATCH_EXECUTE)) return;
            command_printf(debugContext, "Watch-exec point removed.\n");
            break;
        case 'u':
            if (command_clear_watch(debugContext, COMMAND_REGISTER_WATCH_READ) ||
                command_clear_watch(debugContext, COMMAND_REGISTER_WATCH_WRITE) ||
                command_clear_watch(debugContext, COMMAND_REGISTER_WATCH_EXECUTE)) return;
            command_printf(debugContext, "All watch points removed.\n");
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
            command_set_watch(debugContext, COMMAND_REGISTER_WATCH_READ, linear);
            break;
        case 'w':
            linear = scannubit32(debugContext, arg[2]);
            if (nErrPos) return;
            command_set_watch(debugContext, COMMAND_REGISTER_WATCH_WRITE, linear);
            break;
        case 'e':
            linear = scannubit32(debugContext, arg[2]);
            if (nErrPos) return;
            command_set_watch(debugContext, COMMAND_REGISTER_WATCH_EXECUTE, linear);
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
static void xhelp(command_context *debugContext)
{
    command_printf(debugContext, "assemble        XA [address]\n");
    command_printf(debugContext, "compare         XC addr1 addr2 count_byte\n");
    command_printf(debugContext, "dump            XD [address [count_byte]]\n");
    command_printf(debugContext, "enter           XE address [byte_list]\n");
    command_printf(debugContext, "fill            XF address count_byte byte_list\n");
    command_printf(debugContext, "go              XG [address [count_instr]]\n");
    command_printf(debugContext, "move            XM addr1 addr2 count_byte\n");
    command_printf(debugContext, "register        XR [register]\n");
    command_printf(debugContext, "  regular         XREG\n");
    command_printf(debugContext, "  segment         XSREG\n");
    command_printf(debugContext, "  control         XCREG\n");
    command_printf(debugContext, "search          XS address count_byte byte_list\n");
    command_printf(debugContext, "trace           XT [count_instr]\n");
    command_printf(debugContext, "unassemble      XU [address [count_instr]]\n");
    command_printf(debugContext, "watch           XW r/w/e address\n");
}
static void x(command_context *debugContext)
{
    lib_size i;
    arg[narg] = arg[0];
    for (i = 1; i < narg; ++i)
    {
        arg[i - 1] = arg[i];
    }
    arg[narg - 1] = arg[narg];
    arg[narg] = LIB_NULL;
    narg--;
    if (!lib_text_compare(arg[0], "\?"))
    {
        xhelp(debugContext);
    }
    else if (!lib_text_compare(arg[0], "a"))
    {
        xa(debugContext);
    }
    else if (!lib_text_compare(arg[0], "c"))
    {
        xc(debugContext);
    }
    else if (!lib_text_compare(arg[0], "d"))
    {
        xd(debugContext);
    }
    else if (!lib_text_compare(arg[0], "e"))
    {
        xe(debugContext);
    }
    else if (!lib_text_compare(arg[0], "f"))
    {
        xf(debugContext);
    }
    else if (!lib_text_compare(arg[0], "g"))
    {
        xg(debugContext);
    }
    else if (!lib_text_compare(arg[0], "m"))
    {
        xm(debugContext);
    }
    else if (!lib_text_compare(arg[0], "r"))
    {
        xr(debugContext);
    }
    else if (!lib_text_compare(arg[0], "s"))
    {
        xs(debugContext);
    }
    else if (!lib_text_compare(arg[0], "t"))
    {
        xt(debugContext);
    }
    else if (!lib_text_compare(arg[0], "u"))
    {
        xu(debugContext);
    }
    else if (!lib_text_compare(arg[0], "w"))
    {
        xw(debugContext);
    }
    else if (!lib_text_compare(arg[0], "reg"))
    {
        xrprintreg(debugContext);
    }
    else if (!lib_text_compare(arg[0], "sreg"))
    {
        command_print_segments(debugContext);
    }
    else if (!lib_text_compare(arg[0], "creg"))
    {
        command_print_controls(debugContext);
    }
    else
    {
        arg[0] = arg[narg];
        seterr(debugContext, 0);
    }
}
/* EXTENDED DEBUG CMD END */

/* main routines */
static void help(command_context *debugContext)
{
    command_printf(debugContext, "assemble        A [address]\n");
    command_printf(debugContext, "compare         C range address\n");
    command_printf(debugContext, "dump            D [range]\n");
    command_printf(debugContext, "enter           E address [list]\n");
    command_printf(debugContext, "fill            F range list\n");
    command_printf(debugContext, "go              G [[address] breakpoint]\n");
    /* command_printf(debugContext, "go              G [=address] [addresses]\n"); */
    command_printf(debugContext, "hex             H value1 value2\n");
    command_printf(debugContext, "input           I port\n");
    command_printf(debugContext, "load            L [address]\n");
    /* command_printf(debugContext, "load            L [address] [drive] [firstsector] [number]\n"); */
    command_printf(debugContext, "move            M range address\n");
    command_printf(debugContext, "name            N pathname\n");
    /* command_printf(debugContext, "name            N [pathname] [arglist]\n"); */
    command_printf(debugContext, "output          O port byte\n");
    /* !command_printf(debugContext, "proceed           P [nx=address] [number]\n"); */
    command_printf(debugContext, "quit            Q \n");
    command_printf(debugContext, "register        R [register]\n");
    command_printf(debugContext, "search          S range list\n");
    command_printf(debugContext, "trace           T [[address] value]\n");
    /* command_printf(debugContext, "trace           T [=address] [value]\n"); */
    command_printf(debugContext, "unassemble      U [range]\n");
    command_printf(debugContext, "range           address [end | L length]\n");
    command_printf(debugContext, "list            hex bytes and quoted strings\n");
    command_printf(debugContext, "verbal          V \n");
    command_printf(debugContext, "write           W [address]\n");
    command_printf(debugContext, "debug32         X?\n");
    /* command_printf(debugContext, "write           W [address] [drive] [firstsector] [number]\n"); */
    /* command_printf(debugContext, "allocate expanded memory        XA [#pages]\n"); */
    /* command_printf(debugContext, "deallocate expanded memory      XD [handle]\n"); */
    /* command_printf(debugContext, "map expanded memory pages       XM [Lpage] [Ppage] [handle]\n"); */
    /* command_printf(debugContext, "display expanded memory status  XS\n"); */
}

static void parse(command_context *debugContext)
{
    char *p;
    if (!command_copy_text_checked(strCmdCopy, sizeof(strCmdCopy), strCmdBuff)) return;
    narg = 0u;
    nErrPos = 0u;
    p = strCmdCopy;
    while (*p)
    {
        char *start, quote = 0;
        while (*p && lib_text_find_character(" ,\t\n\r\f", *p)) ++p;
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
                if (lib_text_find_character(" ,\t\n\r\f", *p)) break;
                if (*p == '\'' || *p == '"') quote = *p;
                else if (*p >= 'A' && *p <= 'Z') *p += 'a' - 'A';
            }
            ++p;
        }
        if (*p) *p++ = '\0';
        arg[narg++] = start;
        if (narg == 1u && lib_text_length(start) > 1u) arg[narg++] = start + 1;
        if (quote) { seterr(debugContext, narg - 1u); break; }
    }
    arg[narg] = LIB_NULL;
}

static void exec(command_context *debugContext)
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

static void command_initialize(x86_debug *command,
    common_machine *machine)
{
    lib_release(command->output);
    lib_memory_set(command, 0, sizeof(*command));
    command->machine = machine;
}

lib_status x86_debug_create(x86_debug **out_command)
{
    x86_debug *command;

    if (out_command == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_command = LIB_NULL;
    command = (x86_debug *)lib_allocate_zero(1u, sizeof(*command));
    if (command == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    *out_command = command;
    return LIB_STATUS_OK;
}

void x86_debug_destroy(x86_debug *command)
{
    if (command == LIB_NULL) return;
    x86_debug_close(command);
    lib_release(command->output);
    lib_release(command);
}

lib_status x86_debug_open(x86_debug *command,
    common_machine *machine)
{
    if (command == LIB_NULL || machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    x86_debug_close(command);
    command_initialize(command, machine);
    return LIB_STATUS_OK;
}

void x86_debug_close(x86_debug *command)
{
    if (command == LIB_NULL) return;
    common_machine_debug_cancel(command->machine);
    command->machine = LIB_NULL;
    command->continuation = COMMAND_CONTINUATION_NONE;
}

static void command_prompt(x86_debug *command)
{
    const char *prompt = command->continuation == COMMAND_CONTINUATION_NONE ?
        "-" : command->input_prompt;
    if (command->result != LIB_NULL) {
        (void)command_copy_text_checked(command->result->prompt,
            sizeof(command->result->prompt), prompt);
        command->result->prompt_ready = LIB_TRUE;
    }
}

static void command_prepare_continuation(x86_debug *command)
{
    if (command->argument_count == 0u || command->arguments[0] == LIB_NULL)
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
        if (!lib_text_compare(command->arguments[1], "a"))
            command->continuation = COMMAND_CONTINUATION_XASSEMBLE;
        else if (!lib_text_compare(command->arguments[1], "e") &&
            command->argument_count == 3u)
            command->continuation = COMMAND_CONTINUATION_XENTER;
        else if (!lib_text_compare(command->arguments[1], "r") &&
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
    if (lib_text_find_character("acdefgilmorstuwx", name[0]) == NULL) return LIB_FALSE;
    return lib_text_compare(name, "?") != 0 && lib_text_compare(name, "h") != 0 &&
        lib_text_compare(name, "n") != 0 && lib_text_compare(name, "q") != 0 &&
        !(name[0] == 'x' && command->argument_count == 2u &&
            lib_text_compare(command->arguments[1], "?") == 0);
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
        dumpPtrRec = (lib_u16)(_ip) / 0x10 * 0x10;
        xulin = command_code_property(debugContext, X86_DEBUG_GET_CODE_BASE) + _eip;
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
    lib_size i;

    if (command == LIB_NULL || line == LIB_NULL || out_result == LIB_NULL ||
        command->machine == LIB_NULL)
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
        for (i = 0u; i < nErrPos; ++i) command_printf(debugContext, " ");
        command_printf(debugContext, "^ Error\n");
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
    lib_u32 executed;

    if (command == LIB_NULL || out_result == LIB_NULL)
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
