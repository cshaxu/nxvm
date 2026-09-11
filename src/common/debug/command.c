/* DEBUG is the debug console for users to break, trace, lookup,
 * and print virtual machine devices. */

#include "common/debug/command.h"
#include "common/debug/command_runtime.h"
#include "common/xasm32/xasm32_interface.h"

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

struct common_debug_command {
    common_machine *machine;
    common_debug_file_service files;
    common_debug_result *result;
    STD_SIZE_T error_position;
    STD_SIZE_T argument_count;
    C_CHAR **arguments;
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
    C_CHAR pending_line[COMMON_DEBUG_LINE_CAPACITY];
    C_INT pending_line_available;
    command_continuation continuation;
    type_unsigned_32 breakpoint_linear;
    STD_SIZE_T breakpoint_remaining;
    STD_SIZE_T trace_remaining;
    C_INT awaiting_pause;
    command_run_kind run_kind;
    common_debug_memory_access
        memory_accesses[COMMON_DEBUG_MEMORY_ACCESS_CAPACITY];
    type_unsigned_8 memory_access_count;
};

typedef common_debug_command command_context;
typedef common_debug_register command_register;
typedef common_machine_debug_watch_kind command_machine_watch_kind;

static C_INT command_copy_text_checked(C_CHAR *destination,
    STD_SIZE_T destination_capacity, const C_CHAR *source);

#define COMMAND_REGISTER_EAX COMMON_DEBUG_EAX
#define COMMAND_REGISTER_ECX COMMON_DEBUG_ECX
#define COMMAND_REGISTER_EDX COMMON_DEBUG_EDX
#define COMMAND_REGISTER_EBX COMMON_DEBUG_EBX
#define COMMAND_REGISTER_ESP COMMON_DEBUG_ESP
#define COMMAND_REGISTER_EBP COMMON_DEBUG_EBP
#define COMMAND_REGISTER_ESI COMMON_DEBUG_ESI
#define COMMAND_REGISTER_EDI COMMON_DEBUG_EDI
#define COMMAND_REGISTER_EIP COMMON_DEBUG_EIP
#define COMMAND_REGISTER_EFLAGS COMMON_DEBUG_EFLAGS
#define COMMAND_REGISTER_ES COMMON_DEBUG_ES
#define COMMAND_REGISTER_CS COMMON_DEBUG_CS
#define COMMAND_REGISTER_SS COMMON_DEBUG_SS
#define COMMAND_REGISTER_DS COMMON_DEBUG_DS
#define COMMAND_REGISTER_FS COMMON_DEBUG_FS
#define COMMAND_REGISTER_GS COMMON_DEBUG_GS
#define COMMAND_REGISTER_CR0 COMMON_DEBUG_CR0
#define COMMAND_REGISTER_CR1 COMMON_DEBUG_CR1
#define COMMAND_REGISTER_CR2 COMMON_DEBUG_CR2
#define COMMAND_REGISTER_CR3 COMMON_DEBUG_CR3
#define COMMAND_REGISTER_CR4 COMMON_DEBUG_CR4
#define COMMAND_REGISTER_WATCH_READ COMMON_MACHINE_DEBUG_WATCH_READ
#define COMMAND_REGISTER_WATCH_WRITE COMMON_MACHINE_DEBUG_WATCH_WRITE
#define COMMAND_REGISTER_WATCH_EXECUTE COMMON_MACHINE_DEBUG_WATCH_EXECUTE

static C_INT command_printf(command_context *debugContext,
    const C_CHAR *format, ...)
{
    STD_SIZE_T used;
    C_INT written;
    va_list arguments;

    if (debugContext == STD_NULL || debugContext->result == STD_NULL ||
        format == STD_NULL) return -1;
    used = STD_STRLEN(debugContext->result->text);
    if (used >= sizeof(debugContext->result->text)) return -1;
    va_start(arguments, format);
    written = vsnprintf(debugContext->result->text + used,
        sizeof(debugContext->result->text) - used, format, arguments);
    va_end(arguments);
    return written;
}

static C_INT command_read_line(command_context *debugContext,
    C_CHAR *buffer, STD_SIZE_T buffer_size)
{
    if (debugContext == STD_NULL || buffer == STD_NULL || buffer_size == 0u ||
        !debugContext->pending_line_available) return 0;
    if (!command_copy_text_checked(buffer, buffer_size,
            debugContext->pending_line)) return 0;
    debugContext->pending_line_available = 0;
    return 1;
}

static C_INT command_execute(command_context *debugContext,
    const common_machine_debug_request *request,
    common_machine_debug_result *result)
{
    common_machine_debug_lease lease;

    if (debugContext == STD_NULL || request == STD_NULL || result == STD_NULL ||
        common_machine_debug_acquire(debugContext->machine, &lease) != LIB_STATUS_OK)
        return 1;
    return common_machine_debug_execute_with_lease(debugContext->machine, &lease,
        request, result) == LIB_STATUS_OK ? 0 : 1;
}

static C_INT command_read_register(command_context *debugContext,
    command_register register_id, type_unsigned_32 *value)
{
    common_machine_debug_result result;
    if (value == STD_NULL || command_execute(debugContext,
            &(common_machine_debug_request){
                COMMON_MACHINE_DEBUG_READ_REGISTER, (lib_u32)register_id },
            &result)) return 1;
    *value = result.value;
    return 0;
}

static C_INT command_write_register(command_context *debugContext,
    command_register register_id, type_unsigned_32 value)
{
    common_machine_debug_result result;
    return command_execute(debugContext, &(common_machine_debug_request){
        COMMON_MACHINE_DEBUG_WRITE_REGISTER, (lib_u32)register_id, value }, &result);
}

static C_INT command_access_memory(command_context *debugContext,
    common_machine_debug_operation operation, type_unsigned_32 address,
    type_unsigned_16 segment, type_unsigned_16 offset, C_VOID *data,
    type_unsigned_8 bytes)
{
    common_machine_debug_request request = {0};
    common_machine_debug_result result;
    request.operation = operation;
    request.address = address;
    request.segment = segment;
    request.offset = offset;
    request.bytes = bytes;
    if (operation == COMMON_MACHINE_DEBUG_WRITE_LINEAR ||
        operation == COMMON_MACHINE_DEBUG_WRITE_REAL)
        STD_MEMCPY(request.data, data, bytes);
    if (command_execute(debugContext, &request, &result)) return 1;
    if (operation == COMMON_MACHINE_DEBUG_READ_LINEAR ||
        operation == COMMON_MACHINE_DEBUG_READ_REAL)
        STD_MEMCPY(data, result.data, bytes);
    return 0;
}

static C_INT command_read_port(command_context *debugContext,
    type_unsigned_16 port, type_unsigned_32 *value)
{
    common_machine_debug_result result;
    if (command_execute(debugContext, &(common_machine_debug_request){
            COMMON_MACHINE_DEBUG_READ_PORT, 0u, 0u, 0u, 0u, port }, &result))
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
    common_machine_debug_request request = {0};
    common_machine_debug_result result;
    request.operation = COMMON_MACHINE_DEBUG_WRITE_PORT;
    request.port = port;
    request.address = value;
    return command_execute(debugContext, &request, &result);
}

static type_unsigned_32 command_code_property(command_context *debugContext,
    common_machine_debug_operation operation)
{
    common_machine_debug_result result;
    return command_execute(debugContext,
        &(common_machine_debug_request){ .operation = operation }, &result) ? 0u : result.value;
}

static C_INT command_set_watch(command_context *debugContext,
    command_machine_watch_kind kind, type_unsigned_32 address)
{
    common_machine_debug_result result;
    return command_execute(debugContext, &(common_machine_debug_request){
        COMMON_MACHINE_DEBUG_SET_WATCH, 0u, address, 0u, 0u, 0u, kind }, &result);
}

static C_INT command_clear_watch(command_context *debugContext,
    command_machine_watch_kind kind)
{
    common_machine_debug_result result;
    return command_execute(debugContext, &(common_machine_debug_request){
        COMMON_MACHINE_DEBUG_CLEAR_WATCH, 0u, 0u, 0u, 0u, 0u, kind }, &result);
}

static C_INT command_get_watch(command_context *debugContext,
    command_machine_watch_kind kind, type_unsigned_32 *out_address)
{
    common_machine_debug_result result;

    if (out_address == STD_NULL || command_execute(debugContext,
            &(common_machine_debug_request){ COMMON_MACHINE_DEBUG_GET_WATCH,
                0u, 0u, 0u, 0u, 0u, kind }, &result)) return -1;
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
        debugContext->result->lifecycle_request = COMMON_DEBUG_LIFECYCLE_RESUME;
}

static C_INT command_set_break(command_context *debugContext,
    type_unsigned_32 linear)
{
    common_machine_debug_request request = {0};
    common_machine_debug_result result;

    if (debugContext == STD_NULL) return 1;
    request.operation = COMMON_MACHINE_DEBUG_SET_EXECUTION_PLAN;
    request.execution_kind = COMMON_MACHINE_DEBUG_EXECUTION_BREAK_LINEAR;
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
    common_machine_debug_result result;

    if (debugContext == STD_NULL) return;
    (C_VOID)command_execute(debugContext, &(common_machine_debug_request) {
        .operation = COMMON_MACHINE_DEBUG_CLEAR_EXECUTION_PLAN }, &result);
    debugContext->breakpoint_linear = 0u;
    debugContext->breakpoint_remaining = 0u;
}

static C_INT command_set_trace(command_context *debugContext, STD_SIZE_T count)
{
    common_machine_debug_request request = {0};
    common_machine_debug_result result;

    if (debugContext == STD_NULL) return 1;
    request.operation = COMMON_MACHINE_DEBUG_SET_EXECUTION_PLAN;
    request.execution_kind = COMMON_MACHINE_DEBUG_EXECUTION_TRACE;
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
    common_machine_debug_result result;

    if (out_executed == STD_NULL || command_execute(debugContext,
            &(common_machine_debug_request) {
                .operation = COMMON_MACHINE_DEBUG_GET_EXECUTION_RESULT },
            &result) || !result.enabled) return 1;
    *out_executed = result.value;
    return 0;
}

static C_INT command_begin_trace(command_context *debugContext,
    command_run_kind kind, STD_SIZE_T count)
{
    if (debugContext == STD_NULL || command_set_trace(debugContext,
            count < 0x100u ? 1u : count)) return 1;
    debugContext->run_kind = kind;
    debugContext->trace_remaining = count < 0x100u ? count : 1u;
    debugContext->awaiting_pause = 1;
    return 0;
}

static C_VOID command_begin_break(command_context *debugContext,
    STD_SIZE_T count)
{
    if (debugContext == STD_NULL) return;
    debugContext->run_kind = COMMAND_RUN_BREAK_LINEAR;
    debugContext->breakpoint_remaining = count;
    debugContext->awaiting_pause = 1;
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
#define command_machine_get_code_default_size() command_code_property(debugContext, COMMON_MACHINE_DEBUG_GET_CODE_DEFAULT_SIZE)
#define command_machine_get_code_base() command_code_property(debugContext, COMMON_MACHINE_DEBUG_GET_CODE_BASE)
#define command_machine_read_linear(address, out, size) command_access_memory(debugContext, COMMON_MACHINE_DEBUG_READ_LINEAR, address, 0u, 0u, out, size)
#define command_machine_write_linear(address, in, size) command_access_memory(debugContext, COMMON_MACHINE_DEBUG_WRITE_LINEAR, address, 0u, 0u, in, size)
#define command_machine_read_real(segment, offset, out, size) command_access_memory(debugContext, COMMON_MACHINE_DEBUG_READ_REAL, 0u, segment, offset, out, size)
#define command_machine_write_real(segment, offset, in, size) command_access_memory(debugContext, COMMON_MACHINE_DEBUG_WRITE_REAL, 0u, segment, offset, in, size)
#define command_machine_read_port(port) command_read_port_value(debugContext, port)
#define command_machine_write_port(port, value) command_write_port(debugContext, port, value)
#define command_machine_set_break_real(segment, offset) command_set_break(debugContext, ((type_unsigned_32)(segment) << 4u) + (offset))
#define command_machine_set_break_linear(address) command_set_break(debugContext, address)
#define command_machine_clear_break(linear) command_clear_break(debugContext)
#define command_machine_set_watch(kind, address) command_set_watch(debugContext, kind, address)
#define command_machine_clear_watch(kind) command_clear_watch(debugContext, kind)
#define command_machine_print_registers() rprintregs(debugContext)
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
static C_INT debug_flag(command_context *debugContext, type_unsigned_32 mask)
{
    return (debug_register(debugContext, COMMAND_REGISTER_EFLAGS) & mask) != 0;
}
static C_VOID debug_set_flag(command_context *debugContext, type_unsigned_32 mask, C_INT set)
{
    type_unsigned_32 flags = debug_register(debugContext, COMMAND_REGISTER_EFLAGS);
    debug_set_register(debugContext, COMMAND_REGISTER_EFLAGS, set ? flags | mask : flags & ~mask);
}

static C_VOID command_print_segments(command_context *debugContext)
{
    command_printf(debugContext, "ES=%04X CS=%04X SS=%04X DS=%04X FS=%04X GS=%04X\n",
        (type_unsigned_16)debug_register(debugContext, COMMAND_REGISTER_ES),
        (type_unsigned_16)debug_register(debugContext, COMMAND_REGISTER_CS),
        (type_unsigned_16)debug_register(debugContext, COMMAND_REGISTER_SS),
        (type_unsigned_16)debug_register(debugContext, COMMAND_REGISTER_DS),
        (type_unsigned_16)debug_register(debugContext, COMMAND_REGISTER_FS),
        (type_unsigned_16)debug_register(debugContext, COMMAND_REGISTER_GS));
}

static C_VOID command_print_controls(command_context *debugContext)
{
    command_printf(debugContext, "CR0=%08X CR1=%08X CR2=%08X CR3=%08X CR4=%08X\n",
        debug_register(debugContext, COMMAND_REGISTER_CR0),
        debug_register(debugContext, COMMAND_REGISTER_CR1),
        debug_register(debugContext, COMMAND_REGISTER_CR2),
        debug_register(debugContext, COMMAND_REGISTER_CR3),
        debug_register(debugContext, COMMAND_REGISTER_CR4));
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

    for (index = 0u; index < debugContext->memory_access_count; ++index) {
        const common_debug_memory_access *access =
            &debugContext->memory_accesses[index];
        command_printf(debugContext, "%s: Lin=%08x, Data=%08x, Bytes=%1x\n",
            access->write ? "Write" : "Read", access->linear,
            (type_unsigned_32)access->data, access->bytes);
    }
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

static C_VOID command_machine_finalize_arguments(command_context *context)
{
    STD_FREE((C_VOID *)context->arguments);
    context->arguments = STD_NULL;
}

static C_VOID seterr(command_context *debugContext, STD_SIZE_T pos)
{
    nErrPos = (STD_SIZE_T)(arg[pos] - strCmdCopy + STD_STRLEN(arg[pos]) + 1);
}
static type_unsigned_8 scannubit8(command_context *debugContext, C_CHAR *s)
{
    type_unsigned_8 ans = 0;
    STD_SIZE_T i = 0;
    if (s[0] == '\'' && s[2] == '\'')
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
    C_CHAR *cseg, *cptr;
    C_CHAR ccopy[0x100];
    if (!command_copy_text_checked(ccopy, sizeof(ccopy), addr)) {
        seterr(debugContext, narg - 1u);
        return;
    }
    cseg = STD_STRTOK(ccopy, ":");
    cptr = STD_STRTOK(STD_NULL, "");
    if (!cptr)
    {
        seg = defseg;
        ptr = scannubit16(debugContext, cseg);
    }
    else
    {
        if (!STD_STRCMP(cseg, "es"))
        {
            seg = _es;
        }
        else if (!STD_STRCMP(cseg, "cs"))
        {
            seg = _cs;
        }
        else if (!STD_STRCMP(cseg, "ss"))
        {
            seg = _ss;
        }
        else if (!STD_STRCMP(cseg, "ds"))
        {
            seg = _ds;
        }
        else
        {
            seg = scannubit16(debugContext, cseg);
        }
        ptr = scannubit16(debugContext, cptr);
    }
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
        if (common_xasm32_assemble(cmdAsmBuff, STD_STRLEN(cmdAsmBuff),
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
    STD_SIZE_T i;
    type_unsigned_8 val1, val2;
    type_unsigned_16 seg1, ptr1, seg2, ptr2, range;
    if (narg != 4)
    {
        seterr(debugContext, narg - 1);
    }
    else
    {
        addrparse(debugContext, _ds, arg[1]);
        seg1 = seg;
        ptr1 = ptr;
        addrparse(debugContext, _ds, arg[3]);
        seg2 = seg;
        ptr2 = ptr;
        range = scannubit16(debugContext, arg[2]) - ptr1;
        if (!nErrPos)
        {
            for (i = 0; i <= range; ++i)
            {
                command_machine_read_real(seg1, (type_unsigned_16)(ptr1 + i), (C_VOID *)(&val1), 1);
                command_machine_read_real(seg2, (type_unsigned_16)(ptr2 + i), (C_VOID *)(&val2), 1);
                if (val1 != val2)
                {
                    STD_PRINTF("%04X:%04X  ", seg1, (type_unsigned_16)(ptr1 + i));
                    STD_PRINTF("%02X  %02X", val1, val2);
                    STD_PRINTF("  %04X:%04X\n", seg2, (type_unsigned_16)(ptr2 + i));
                }
            }
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
    if ((type_unsigned_32)((segment << 4) + end) > 0x000fffff)
    {
        end = (type_unsigned_16)(0x000fffff - (segment << 4));
    }
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
    type_unsigned_16 ptr2;
    if (narg == 1)
    {
        dprint(debugContext, dumpSegRec, dumpPtrRec, dumpPtrRec + 0x7f);
    }
    else if (narg == 2)
    {
        addrparse(debugContext, _ds, arg[1]);
        if (nErrPos)
        {
            return;
        }
        dprint(debugContext, seg, ptr, ptr + 0x7f);
    }
    else if (narg == 3)
    {
        addrparse(debugContext, _ds, arg[1]);
        ptr2 = scannubit16(debugContext, arg[2]);
        if (nErrPos)
        {
            return;
        }
        if (ptr > ptr2)
        {
            seterr(debugContext, 2);
        }
        else
        {
            dprint(debugContext, seg, ptr, ptr2);
        }
    }
    else
    {
        seterr(debugContext, 3);
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
    else if (narg > 2)
    {
        addrparse(debugContext, _ds, arg[1]);
        if (nErrPos)
        {
            return;
        }
        for (i = 2; i < narg; ++i)
        {
            val = scannubit8(debugContext, arg[i]); /* MARK */
            if (!nErrPos)
            {
                command_machine_write_real(seg, ptr, (C_VOID *)(&val), 1);
            }
            else
            {
                break;
            }
            ptr++;
        }
    }
}
/* fill */
static C_VOID f(command_context *debugContext)
{
    type_unsigned_8 nbyte;
    type_unsigned_8 val;
    STD_SIZE_T i, j;
    type_unsigned_16 end;
    if (narg < 4)
    {
        seterr(debugContext, narg - 1);
    }
    else
    {
        addrparse(debugContext, _ds, arg[1]);
        end = scannubit16(debugContext, arg[2]);
        if (end < ptr)
        {
            seterr(debugContext, 2);
        }
        if (!nErrPos)
        {
            nbyte = (type_unsigned_8)narg - 3;
            for (i = ptr, j = 0; i <= end; ++i, ++j)
            {
                val = scannubit8(debugContext, arg[j % nbyte + 3]);
                if (!nErrPos)
                {
                    command_machine_write_real(seg, (type_unsigned_16)i, (C_VOID *)(&val), 1);
                }
                else
                {
                    return;
                }
            }
        }
    }
}
/* go */
static C_VOID rprintregs(command_context *debugContext);
static C_VOID g(command_context *debugContext)
{
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
        if (command_machine_set_break_real(seg, ptr)) return;
        break;
    case 3:
        addrparse(debugContext, _cs, arg[1]);
        if (debug_set_register(debugContext, COMMAND_REGISTER_CS, seg))
        {
            STD_PRINTF("debug: fail to load cs from %04X\n", seg);
            return;
        }
        debug_set_register(debugContext, COMMAND_REGISTER_EIP, ptr);
        addrparse(debugContext, _cs, arg[2]);
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
    if (narg != 1u) {
        debugContext->run_kind = COMMAND_RUN_BREAK_REAL;
        debugContext->awaiting_pause = 1;
    }
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
    lib_size bytes;
    type_unsigned_8 data[COMMON_MACHINE_DEBUG_BYTES];

    if (debugContext->files.read == STD_NULL || !STD_STRLEN(strFileName)) {
        STD_PRINTF("File not found\n");
        return;
    }
    switch (narg) {
    case 1: seg = _cs; ptr = 0x100; break;
    case 2: addrparse(debugContext, _cs, arg[1]); break;
    default: seterr(debugContext, narg - 1); break;
    }
    while (!nErrPos && debugContext->files.read(debugContext->files.context,
        strFileName, len, data, sizeof(data), &bytes) == LIB_STATUS_OK && bytes != 0u) {
        lib_size index;
        for (index = 0u; index < bytes; ++index) {
            command_machine_write_real(seg + i, ptr + len++, &data[index], 1);
            i = (type_unsigned_16)(len / 0x10000u);
        }
    }
    debug_set_register(debugContext, COMMAND_REGISTER_ECX, (type_unsigned_16)(len & 0xffffu));
    debug_set_register(debugContext, COMMAND_REGISTER_EBX, (type_unsigned_16)(len >> 16u));
}
/* move */
static C_VOID m(command_context *debugContext)
{
    STD_SIZE_T i;
    type_unsigned_8 val;
    type_unsigned_16 seg1, ptr1, range, seg2, ptr2;
    if (narg != 4)
        seterr(debugContext, narg - 1);
    else
    {
        addrparse(debugContext, _ds, arg[1]);
        seg1 = seg;
        ptr1 = ptr;
        addrparse(debugContext, _ds, arg[3]);
        seg2 = seg;
        ptr2 = ptr;
        range = scannubit16(debugContext, arg[2]) - ptr1;
        if (!nErrPos)
        {
            if (((seg1 << 4) + ptr1) < ((seg2 << 4) + ptr2))
            {
                for (i = 0; i <= range; ++i)
                {
                    command_machine_read_real(seg1, (type_unsigned_16)(ptr1 + range - i), (C_VOID *)(&val), 1);
                    command_machine_write_real(seg2, (type_unsigned_16)(ptr2 + range - i), (C_VOID *)(&val), 1);
                }
            }
            else if (((seg1 << 4) + ptr1) > ((seg2 << 4) + ptr2))
            {
                for (i = 0; i <= range; ++i)
                {
                    command_machine_read_real(seg1, (type_unsigned_16)(ptr1 + i), (C_VOID *)(&val), 1);
                    command_machine_write_real(seg2, (type_unsigned_16)(ptr2 + i), (C_VOID *)(&val), 1);
                }
            }
        }
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
    STD_SIZE_T sbin_remaining;
    C_INT binary_failed = TYPE_FALSE;
    C_INT format_result;
    type_unsigned_8 len;
    type_unsigned_8 ucode[15];
    C_CHAR str[0x100], stmt[0x100], sbin[0x100];
    C_CHAR *sbin_cursor;
    if (command_machine_read_linear((segment << 4) + off, (C_VOID *)ucode, 15))
    {
        len = 0;
        (C_VOID)STD_SNPRINTF(str, sizeof(str), "%04X:%04X <ERROR>", segment, off);
    }
    else
    {
        if (common_xasm32_disassemble(ucode, sizeof(ucode), stmt,
                sizeof(stmt), &i,
                command_machine_get_code_default_size()) != TYPE_STATUS_OK) {
            len = 0u;
        } else {
            len = (type_unsigned_8)i;
        }
        sbin[0] = 0;
        sbin_cursor = sbin;
        sbin_remaining = sizeof(sbin);
        for (i = 0; i < len; ++i)
        {
            format_result = STD_SNPRINTF_APPEND(&sbin_cursor, &sbin_remaining,
                "%02X", (type_unsigned_8)ucode[i]);
            if (format_result < 0 || (STD_SIZE_T)format_result >= sbin_remaining) {
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
            debug_set_register(debugContext, COMMAND_REGISTER_EAX, (type_unsigned_16)value);
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
            debug_set_register(debugContext, COMMAND_REGISTER_EBX, (type_unsigned_16)value);
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
            debug_set_register(debugContext, COMMAND_REGISTER_ECX, (type_unsigned_16)value);
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
            debug_set_register(debugContext, COMMAND_REGISTER_EDX, (type_unsigned_16)value);
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
            debug_set_register(debugContext, COMMAND_REGISTER_EBP, (type_unsigned_16)value);
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
            debug_set_register(debugContext, COMMAND_REGISTER_ESP, (type_unsigned_16)value);
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
            debug_set_register(debugContext, COMMAND_REGISTER_ESI, (type_unsigned_16)value);
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
            debug_set_register(debugContext, COMMAND_REGISTER_EDI, (type_unsigned_16)value);
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
            debug_set_register(debugContext, COMMAND_REGISTER_EIP, value);
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
    STD_SIZE_T i;
    C_INT flagFound = 0;
    type_unsigned_16 p, pfront, start, end;
    type_unsigned_8 cstart, val;
    if (narg < 4)
    {
        seterr(debugContext, narg - 1);
    }
    else
    {
        addrparse(debugContext, _ds, arg[1]);
        start = ptr;
        end = scannubit16(debugContext, arg[2]);
        if (!nErrPos)
        {
            p = start;
            cstart = scannubit8(debugContext, arg[3]);
            while (p <= end)
            {
                command_machine_read_real(seg, p, (C_VOID *)(&val), 1);
                if (val == cstart)
                {
                    pfront = p;
                    flagFound = 1;
                    for (i = 3; i < narg; ++i)
                    {
                        command_machine_read_real(seg, p, (C_VOID *)(&val), 1);
                        if (val != scannubit8(debugContext, arg[i]))
                        {
                            flagFound = 0;
                            p = pfront + 1;
                            break;
                        }
                        else
                        {
                            ++p;
                        }
                    }
                    if (flagFound)
                    {
                        STD_PRINTF("%04X:%04X  ", seg, pfront);
                        STD_PRINTF("\n");
                    }
                }
                else
                    ++p;
            }
        }
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
        if (debug_set_register(debugContext, COMMAND_REGISTER_CS, seg))
        {
            STD_PRINTF("debug: fail to load cs from %04X\n", seg);
            return;
        }
        debug_set_register(debugContext, COMMAND_REGISTER_EIP, ptr);
        count = scannubit16(debugContext, arg[2]);
        break;
    default:
        seterr(debugContext, narg - 1);
        break;
    }
    if (nErrPos)
    {
        return;
    }
    /* The original loop synchronously waited after each instruction.  The
     * common/session boundary is asynchronous: one trace budget and one
     * resume request preserve the requested stop point without polling. */
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
    if ((type_unsigned_32)((segment << 4) + end) > 0xfffff)
    {
        end = (0xfffff - (segment << 4));
    }
    while (start <= end)
    {
        len = uprintins(debugContext, segment, start);
        start += len;
        boundary = (type_unsigned_32)start + (type_unsigned_32)len;
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
    type_unsigned_16 ptr2;
    if (narg == 1)
    {
        uprint(debugContext, uasmSegRec, uasmPtrRec, uasmPtrRec + 0x1f);
    }
    else if (narg == 2)
    {
        addrparse(debugContext, _cs, arg[1]);
        if (nErrPos)
        {
            return;
        }
        uprint(debugContext, seg, ptr, ptr + 0x1f);
    }
    else if (narg == 3)
    {
        addrparse(debugContext, _ds, arg[1]);
        ptr2 = scannubit16(debugContext, arg[2]);
        if (nErrPos)
        {
            return;
        }
        if (ptr > ptr2)
        {
            seterr(debugContext, 2);
        }
        else
        {
            uprint(debugContext, seg, ptr, ptr2);
        }
    }
    else
    {
        seterr(debugContext, 3);
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
    type_unsigned_8 data[COMMON_MACHINE_DEBUG_BYTES];
    if (!STD_STRLEN(strFileName))
    {
        STD_PRINTF("(W)rite error, no destination defined\n");
        return;
    }
    if (debugContext->files.write == STD_NULL)
    {
        STD_PRINTF("File not found\n");
    }
    else
    {
        STD_PRINTF("Writing ");
        STD_PRINTF("%04X", _bx);
        STD_PRINTF("%04X", _cx);
        STD_PRINTF(" bytes\n");
        switch (narg)
        {
        case 1:
            seg = _cs;
            ptr = 0x100;
            break;
        case 2:
            addrparse(debugContext, _cs, arg[1]);
            break;
        default:
            seterr(debugContext, narg - 1);
            break;
        }
        while (!nErrPos && i < len) {
            lib_size count = len - i < sizeof(data) ? len - i : sizeof(data);
            lib_size index;
            for (index = 0u; index < count; ++index)
                command_machine_read_real(seg, (type_unsigned_16)(ptr + i + index), &data[index], 1);
            if (debugContext->files.write(debugContext->files.context, strFileName,
                    i, data, count) != LIB_STATUS_OK) {
                STD_PRINTF("File write failed\n");
                break;
            }
            i += count;
        }
    }
}
/* DEBUG CMD END */

/* EXTENDED DEBUG CMD BEGIN */
type_unsigned_32 xalin;
type_unsigned_32 xdlin;
type_unsigned_32 xulin;
/* print */
static type_unsigned_8 xuprintins(command_context *debugContext, type_unsigned_32 linear)
{
    STD_SIZE_T i;
    STD_SIZE_T sbin_remaining;
    C_INT binary_failed = TYPE_FALSE;
    C_INT format_result;
    type_unsigned_8 len;
    type_unsigned_8 ucode[15];
    C_CHAR str[0x100], stmt[0x100], sbin[0x100];
    C_CHAR *sbin_cursor;
    if (command_machine_read_linear(linear, (C_VOID *)ucode, 15))
    {
        len = 0;
        (C_VOID)STD_SNPRINTF(str, sizeof(str), "L%08X <ERROR>", linear);
    }
    else
    {
        if (common_xasm32_disassemble(ucode, sizeof(ucode), stmt,
                sizeof(stmt), &i,
                command_machine_get_code_default_size()) != TYPE_STATUS_OK) {
            len = 0u;
        } else {
            len = (type_unsigned_8)i;
        }
        sbin[0] = 0;
        sbin_cursor = sbin;
        sbin_remaining = sizeof(sbin);
        for (i = 0; i < len; ++i)
        {
            format_result = STD_SNPRINTF_APPEND(&sbin_cursor, &sbin_remaining,
                "%02X", (type_unsigned_8)ucode[i]);
            if (format_result < 0 || (STD_SIZE_T)format_result >= sbin_remaining) {
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
    command_machine_print_registers();
    xulin = command_machine_get_code_base() + _eip;
    xuprintins(debugContext, xulin);
}
/* assemble */
static C_VOID xaconsole(command_context *debugContext, type_unsigned_32 linear)
{
    STD_SIZE_T i, len, errAsmPos;
    C_CHAR astmt[0x100];
    type_unsigned_8 acode[15];
    C_INT flagExitAsm = 0;
    while (!flagExitAsm)
    {
        STD_PRINTF("L%08X ", linear);
        if (!command_machine_read_line(astmt, sizeof(astmt))) return;
        if (STD_STRLEN(astmt) != 0u && astmt[STD_STRLEN(astmt) - 1u] == '\n')
            astmt[STD_STRLEN(astmt) - 1u] = '\0';
        if (!STD_STRLEN(astmt))
        {
            flagExitAsm = 1;
            continue;
        }
        errAsmPos = 0;
        if (common_xasm32_assemble(astmt, STD_STRLEN(astmt), acode,
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
            if (command_machine_write_linear(linear, (C_VOID *)acode, (type_unsigned_8)len))
            {
                STD_PRINTF("debug: fail to write to L%08X\n", linear);
                return;
            }
            linear += (type_unsigned_32)len;
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
    xalin = linear;
}
static C_VOID xa(command_context *debugContext)
{
    if (narg == 1)
    {
        xaconsole(debugContext, xalin);
    }
    else if (narg == 2)
    {
        xalin = scannubit32(debugContext, arg[1]);
        if (nErrPos)
        {
            return;
        }
        xaconsole(debugContext, xalin);
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
    type_unsigned_32 end = linear + count - 1;
    c[0x10] = '\0';
    if (!count)
    {
        return;
    }
    if (end < start)
        end = 0xffffffff;
    for (ilinear = start - (start % 0x10); ilinear <= end + 0x0f - (end % 0x10); ++ilinear)
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
        if ((ilinear + 1) % 0x10 == 0)
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
    else if (narg > 2)
    {
        linear = scannubit32(debugContext, arg[1]);
        if (nErrPos)
        {
            return;
        }
        for (i = 2; i < narg; ++i)
        {
            val = scannubit8(debugContext, arg[i]);
            if (!nErrPos)
            {
                if (command_machine_write_linear(linear, (C_VOID *)(&val), 1))
                {
                    STD_PRINTF("debug: fail to write to L%08X.\n", linear);
                    return;
                }
            }
            else
            {
                break;
            }
            linear++;
        }
    }
}
/* fill */
static C_VOID xf(command_context *debugContext)
{
    type_unsigned_8 val;
    STD_SIZE_T i, j, count, bcount;
    type_unsigned_32 linear;
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
        bcount = narg - 3;
        for (i = 0, j = 0; i < count; ++i, ++j)
        {
            val = scannubit8(debugContext, arg[j % bcount + 3]);
            if (nErrPos)
            {
                return;
            }
            if (command_machine_write_linear((type_unsigned_32)(linear + i), (C_VOID *)(&val), 1))
            {
                STD_PRINTF("debug: fail to write to L%08X.\n", (type_unsigned_32)(linear + i));
                return;
            }
        }
    }
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
    if (command_machine_set_break_linear(linear)) return;
    command_begin_break(debugContext, count);
    command_machine_resume();
}
/* move */
static C_VOID xm(command_context *debugContext)
{
    type_unsigned_8 val;
    STD_SIZE_T i;
    type_unsigned_32 lin1, lin2, count;
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
        for (i = 0; i < count; ++i)
        {
            if (command_machine_read_linear((type_unsigned_32)(lin1 + i), (C_VOID *)(&val), 1))
            {
                STD_PRINTF("debug: fail to read from L%08X.\n", lin1 + i);
                return;
            }
            if (command_machine_write_linear((type_unsigned_32)(lin2 + i), (C_VOID *)(&val), 1))
            {
                STD_PRINTF("debug: fail to write to L%08X.\n", lin2 + i);
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
    type_unsigned_8 val, mem[256], line[256];
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
        addrparse(debugContext, _ds, arg[1]);
        bcount = narg - 3;
        for (i = 0; i < bcount; ++i)
        {
            val = scannubit8(debugContext, arg[i + 3]);
            if (nErrPos)
            {
                return;
            }
            line[i] = val;
        }
        for (i = 0; i < count; ++i)
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
        break;
    }
    if (nErrPos)
    {
        return;
    }
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
static C_VOID xuprint(command_context *debugContext, type_unsigned_32 linear, type_unsigned_8 count)
{
    type_unsigned_32 len = 0;
    STD_SIZE_T i;
    for (i = 0; i < count; ++i)
    {
        len = xuprintins(debugContext, linear);
        if (!len)
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
            command_machine_clear_watch(COMMAND_REGISTER_WATCH_READ);
            STD_PRINTF("Watch-read point removed.\n");
            break;
        case 'w':
            command_machine_clear_watch(COMMAND_REGISTER_WATCH_WRITE);
            STD_PRINTF("Watch-write point removed.\n");
            break;
        case 'e':
            command_machine_clear_watch(COMMAND_REGISTER_WATCH_EXECUTE);
            STD_PRINTF("Watch-exec point removed.\n");
            break;
        case 'u':
            command_machine_clear_watch(COMMAND_REGISTER_WATCH_READ);
            command_machine_clear_watch(COMMAND_REGISTER_WATCH_WRITE);
            command_machine_clear_watch(COMMAND_REGISTER_WATCH_EXECUTE);
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
            command_machine_set_watch(COMMAND_REGISTER_WATCH_READ, linear);
            break;
        case 'w':
            linear = scannubit32(debugContext, arg[2]);
            command_machine_set_watch(COMMAND_REGISTER_WATCH_WRITE, linear);
            break;
        case 'e':
            linear = scannubit32(debugContext, arg[2]);
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
    if (!command_copy_text_checked(strCmdCopy, sizeof(strCmdCopy),
            strCmdBuff)) return;
    narg = 0;
    arg[0] = STD_STRTOK(strCmdCopy, " ,\t\n\r\f");
    if (arg[narg])
    {
        type_string_lower(arg[narg]);
        narg++;
    }
    else
    {
        return;
    }
    if (STD_STRLEN(arg[narg - 1]) != 1)
    {
        arg[narg] = arg[narg - 1] + 1;
        narg++;
    }
    while (narg < DEBUG_MAXNARG)
    {
        arg[narg] = STD_STRTOK(STD_NULL, " ,\t\n\r\f");
        if (arg[narg])
        {
            type_string_lower(arg[narg]);
            narg++;
        }
        else
        {
            break;
        }
    }
}

static C_VOID exec(command_context *debugContext)
{
    nErrPos = 0;
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

static C_VOID command_initialize(common_debug_command *command,
    common_machine *machine, const common_debug_file_service *files)
{
    STD_MEMSET(command, 0, sizeof(*command));
    command->machine = machine;
    if (files != STD_NULL) command->files = *files;
    command->arguments = (C_CHAR **)STD_CALLOC(DEBUG_MAXNARG,
        sizeof(*command->arguments));
}

lib_status common_debug_command_create(common_debug_command **out_command)
{
    common_debug_command *command;

    if (out_command == STD_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_command = STD_NULL;
    command = (common_debug_command *)STD_CALLOC(1u, sizeof(*command));
    if (command == STD_NULL) return LIB_STATUS_NO_MEMORY;
    *out_command = command;
    return LIB_STATUS_OK;
}

void common_debug_command_destroy(common_debug_command *command)
{
    if (command == STD_NULL) return;
    command_machine_finalize_arguments(command);
    STD_FREE(command);
}

lib_status common_debug_command_open(common_debug_command *command,
    common_machine *machine, const common_debug_file_service *files)
{
    command_context *debugContext = command;

    if (command == STD_NULL || machine == STD_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    command_machine_finalize_arguments(command);
    command_initialize(command, machine, files);
    if (command->arguments == STD_NULL) return LIB_STATUS_NO_MEMORY;
    strFileName[0] = '\0';
    asmSegRec = uasmSegRec = _cs;
    asmPtrRec = uasmPtrRec = _ip;
    dumpSegRec = _ds;
    dumpPtrRec = (type_unsigned_16)(_ip) / 0x10 * 0x10;
    xalin = 0u;
    xdlin = 0u;
    xulin = command_machine_get_code_base() + _eip;
    return LIB_STATUS_OK;
}

void common_debug_command_close(common_debug_command *command)
{
    if (command == STD_NULL) return;
    command_machine_finalize_arguments(command);
    command->machine = STD_NULL;
    command->continuation = COMMAND_CONTINUATION_NONE;
}

static void command_prompt(common_debug_command *command)
{
    const char *prompt = "-";
    switch (command->continuation) {
    case COMMAND_CONTINUATION_ASSEMBLE: prompt = "assemble> "; break;
    case COMMAND_CONTINUATION_ENTER: prompt = "enter> "; break;
    case COMMAND_CONTINUATION_REGISTER:
    case COMMAND_CONTINUATION_XREGISTER: prompt = ":"; break;
    default: break;
    }
    if (command->result != STD_NULL) {
        (void)command_copy_text_checked(command->result->prompt,
            sizeof(command->result->prompt), prompt);
        command->result->prompt_ready = LIB_TRUE;
    }
}

static void command_prepare_continuation(common_debug_command *command)
{
    if (command->argument_count == 0u || command->arguments[0] == STD_NULL)
        return;
    if (!STD_STRCMP(command->arguments[0], "a"))
        command->continuation = COMMAND_CONTINUATION_ASSEMBLE;
    else if (!STD_STRCMP(command->arguments[0], "e") &&
        command->argument_count == 2u)
        command->continuation = COMMAND_CONTINUATION_ENTER;
    else if (!STD_STRCMP(command->arguments[0], "r") &&
        command->argument_count == 2u)
        command->continuation = COMMAND_CONTINUATION_REGISTER;
    else if (!STD_STRCMP(command->arguments[0], "v"))
        command->continuation = COMMAND_CONTINUATION_VERBAL;
    else if (command->arguments[0][0] == 'x' &&
        command->argument_count >= 2u) {
        if (!STD_STRCMP(command->arguments[1], "a"))
            command->continuation = COMMAND_CONTINUATION_XASSEMBLE;
        else if (!STD_STRCMP(command->arguments[1], "e"))
            command->continuation = COMMAND_CONTINUATION_XENTER;
        else if (!STD_STRCMP(command->arguments[1], "r") &&
            command->argument_count >= 3u)
            command->continuation = COMMAND_CONTINUATION_XREGISTER;
    }
}

lib_status common_debug_command_submit_line(common_debug_command *command,
    const char *line, common_debug_result *out_result)
{
    command_context *debugContext = command;
    STD_SIZE_T i;

    if (command == STD_NULL || line == STD_NULL || out_result == STD_NULL ||
        command->machine == STD_NULL || command->arguments == STD_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    STD_MEMSET(out_result, 0, sizeof(*out_result));
    out_result->keep_active = LIB_TRUE;
    command->result = out_result;
    if (command->continuation == COMMAND_CONTINUATION_NONE) {
        if (!command_copy_text_checked(strCmdBuff, sizeof(strCmdBuff), line))
            return LIB_STATUS_INVALID_ARGUMENT;
        parse(debugContext);
        command_prepare_continuation(command);
        exec(debugContext);
    } else {
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
            xaconsole(debugContext, xalin);
            if (line[0] == '\0') command->continuation = COMMAND_CONTINUATION_NONE;
            break;
        case COMMAND_CONTINUATION_XENTER: xe(debugContext); command->continuation = COMMAND_CONTINUATION_NONE; break;
        case COMMAND_CONTINUATION_XREGISTER: xrscanreg(debugContext); command->continuation = COMMAND_CONTINUATION_NONE; break;
        default: break;
        }
    }
    if (nErrPos) {
        command->continuation = COMMAND_CONTINUATION_NONE;
        for (i = 0u; i < nErrPos; ++i) STD_PRINTF(" ");
        STD_PRINTF("^ Error\n");
    }
    command_prompt(command);
    if (flagExit) out_result->keep_active = LIB_FALSE;
    command->result = STD_NULL;
    return LIB_STATUS_OK;
}

void common_debug_command_observe_instruction(common_debug_command *command,
    const common_debug_instruction_observation *observation)
{
    if (command == STD_NULL || observation == STD_NULL) return;
    command->memory_access_count = observation->memory_access_count <
        COMMON_DEBUG_MEMORY_ACCESS_CAPACITY ? observation->memory_access_count :
        COMMON_DEBUG_MEMORY_ACCESS_CAPACITY;
    STD_MEMCPY(command->memory_accesses, observation->memory_accesses,
        command->memory_access_count * sizeof(command->memory_accesses[0]));
}

lib_status common_debug_command_observe_machine(common_debug_command *command,
    common_debug_machine_state state, lib_status status,
    common_debug_result *out_result)
{
    type_unsigned_32 executed;

    if (command == STD_NULL || out_result == STD_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    STD_MEMSET(out_result, 0, sizeof(*out_result));
    out_result->keep_active = LIB_TRUE;
    if (state != COMMON_DEBUG_MACHINE_PAUSED || status != LIB_STATUS_OK ||
        !command->awaiting_pause) return LIB_STATUS_OK;
    if (command_get_execution_result(command, &executed)) return LIB_STATUS_OK;
    command->result = out_result;
    command->awaiting_pause = 0;
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
        if (command_set_trace(command, 1u)) {
            command->run_kind = COMMAND_RUN_NONE;
            command_prompt(command);
            command->result = STD_NULL;
            return LIB_STATUS_OK;
        }
        command->awaiting_pause = 1;
        out_result->lifecycle_request = COMMON_DEBUG_LIFECYCLE_RESUME;
    } else if (command->run_kind == COMMAND_RUN_BREAK_LINEAR &&
        command->breakpoint_remaining != 0u) {
        if (command_set_break(command, command->breakpoint_linear)) {
            command->run_kind = COMMAND_RUN_NONE;
            command_prompt(command);
            command->result = STD_NULL;
            return LIB_STATUS_OK;
        }
        command->awaiting_pause = 1;
        out_result->lifecycle_request = COMMON_DEBUG_LIFECYCLE_RESUME;
    } else {
        command->run_kind = COMMAND_RUN_NONE;
        command_clear_break(command);
    }
    command_prompt(command);
    command->result = STD_NULL;
    return LIB_STATUS_OK;
}
