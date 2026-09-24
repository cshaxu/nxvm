#include "machine_fixture.h"
#include "x86/debug/debug_interface.h"

#include <assert.h>
#include <stdio.h>

typedef struct debug_fake {
    lib_bool register_fixture;
    lib_u32 registers[X86_DEBUG_REGISTER_COUNT];
    lib_u32 real_reads, linear_reads;
    lib_bool memory_fixture;
    lib_u8 memory[8192];
} debug_fake;

static lib_status fake_execute_x86(void *opaque,
    const x86_debug_request *request,
    x86_debug_response *result)
{
    debug_fake *fake = (debug_fake *)opaque;
    if (request == NULL || result == NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *result = (x86_debug_response) { .value = request->address };
    if (fake->memory_fixture && (request->operation == X86_DEBUG_READ_REAL ||
        request->operation == X86_DEBUG_WRITE_REAL ||
        request->operation == X86_DEBUG_READ_LINEAR ||
        request->operation == X86_DEBUG_WRITE_LINEAR)) {
        lib_u32 address = request->operation == X86_DEBUG_READ_REAL ||
            request->operation == X86_DEBUG_WRITE_REAL ?
            ((lib_u32)request->segment << 4) + request->offset : request->address;
        assert(address + request->bytes <= sizeof(fake->memory));
        if (request->operation == X86_DEBUG_WRITE_REAL ||
            request->operation == X86_DEBUG_WRITE_LINEAR)
            lib_memory_copy(fake->memory + address, request->data, request->bytes);
        else lib_memory_copy(result->data, fake->memory + address, request->bytes);
        return LIB_STATUS_OK;
    }
    if (fake->register_fixture) {
        switch (request->operation) {
        case X86_DEBUG_READ_REGISTER:
            result->value = fake->registers[request->register_id];
            break;
        case X86_DEBUG_WRITE_REGISTER:
            fake->registers[request->register_id] = request->address;
            break;
        case X86_DEBUG_READ_REAL:
            ++fake->real_reads;
            lib_memory_set(result->data, 0x90, sizeof(result->data));
            break;
        case X86_DEBUG_READ_LINEAR:
            ++fake->linear_reads;
            lib_memory_set(result->data, 0x90, sizeof(result->data));
            break;
        case X86_DEBUG_GET_CODE_BASE:
            result->value = 0x10000000u;
            break;
        case X86_DEBUG_GET_CODE_DEFAULT_SIZE:
            result->value = 1u;
            break;
        case X86_DEBUG_GET_EXECUTION_RESULT:
            result->enabled = LIB_TRUE;
            result->value = 1u;
            break;
        default:
            break;
        }
    }
    return LIB_STATUS_OK;
}

static lib_status fake_execute_debug(void *opaque, const void *bytes, lib_size size,
    void *response, lib_size capacity, lib_size *response_size)
{
    x86_debug_request request;
    x86_debug_response result;
    lib_status status;
    assert(size == sizeof(request) && capacity == sizeof(result));
    lib_memory_copy(&request, bytes, size);
    status = fake_execute_x86(opaque, &request, &result);
    if (status == LIB_STATUS_OK) {
        lib_memory_copy(response, &result, sizeof(result));
        *response_size = sizeof(result);
    }
    return status;
}

static lib_status execute_x86(common_machine *machine,
    const common_machine_debug_lease *lease, const x86_debug_request *request,
    x86_debug_response *response)
{
    lib_size size;
    lib_status status = common_machine_debug_execute_with_lease(machine, lease,
        request, sizeof(*request), response, sizeof(*response), &size);
    assert(size == (status == LIB_STATUS_OK ? sizeof(*response) : 0u));
    return status;
}

static void extended_registers(x86_debug *debug, debug_fake *fake)
{
    x86_debug_result result;
    const char *names[] = { "eax", "ecx", "edx", "ebx", "esp", "ebp",
        "esi", "edi", "eip", "eflags" };
    const char *commands[] = { "xr", "xreg", "x r", "x reg", "xt", "xg 12345678" };
    const char *expected =
        "EAX=12340000 EBX=12340003 ECX=12340001 EDX=12340002\n"
        "ESP=12340004 EBP=12340005 ESI=12340006 EDI=12340007\n"
        "EIP=12340008 EFL=00037FD7: VM RF NT IOPL=3 OF DF IF TF SF ZF AF PF CF \n";
    lib_u32 index;
    fake->register_fixture = LIB_TRUE;
    for (index = 0; index < 10u; ++index)
        fake->registers[index] = 0x12340000u + index;
    fake->registers[X86_DEBUG_EFLAGS] = 0x37fd7u;
    for (index = 0; index < sizeof(commands) / sizeof(commands[0]); ++index) {
        fake->real_reads = fake->linear_reads = 0u;
        assert(x86_debug_submit_line(debug, commands[index], &result) == LIB_STATUS_OK);
        if (index >= 4u) {
            assert(result.lifecycle_request == X86_DEBUG_LIFECYCLE_RESUME);
            assert(x86_debug_observe_machine(debug, X86_DEBUG_MACHINE_PAUSED,
                LIB_STATUS_OK, &result) == LIB_STATUS_OK);
        }
        assert(lib_text_find_substring(result.text, expected) != NULL);
        assert(lib_text_find_substring(result.text, "L22340008 90") != NULL);
        assert(fake->real_reads == 0u && fake->linear_reads == 1u);
    }
    fake->registers[X86_DEBUG_EFLAGS] = 2u;
    assert(x86_debug_submit_line(debug, "xr", &result) == LIB_STATUS_OK);
    assert(lib_text_find_substring(result.text,
        "EFL=00000002: vm rf nt IOPL=0 of df if tf sf zf af pf cf \n") != NULL);
    fake->real_reads = fake->linear_reads = 0u;
    assert(x86_debug_submit_line(debug, "r", &result) == LIB_STATUS_OK);
    assert(lib_text_compare_n(result.text, "AX=0000  BX=0003", 15u) == 0);
    assert(lib_text_find_substring(result.text, "EAX=") == NULL && fake->linear_reads == 0u &&
        fake->real_reads == 1u);
    assert(lib_text_find_substring(result.text, "0000:0008 90") != NULL);
    /* Each original XR register continuation reads and writes the full value. */
    for (index = 0; index < 10u; ++index) {
        char line[32];
        char value[16];
        snprintf(line, sizeof(line), "xr %s", names[index]);
        snprintf(value, sizeof(value), "%08X", fake->registers[index]);
        assert(x86_debug_submit_line(debug, line, &result) == LIB_STATUS_OK);
        assert(lib_text_find_substring(result.text, value) != NULL);
        assert(x86_debug_submit_line(debug, "89abcdef", &result) == LIB_STATUS_OK);
        assert(fake->registers[index] == 0x89abcdefu);
    }
    assert(x86_debug_submit_line(debug, "xsreg", &result) == LIB_STATUS_OK);
    assert(lib_text_find_substring(result.text, "Data, e, rw, big") != NULL);
    fake->register_fixture = LIB_FALSE;
}
static void transcript(x86_debug *debug, const char *line,
    const char *text, const char *prompt)
{
    x86_debug_result result;
    assert(x86_debug_submit_line(debug, line, &result) == LIB_STATUS_OK);
    assert(lib_text_compare(result.text, text) == 0);
    assert(result.prompt_ready && lib_text_compare(result.prompt, prompt) == 0);
    assert(result.lifecycle_request == X86_DEBUG_LIFECYCLE_NONE);
}

static void original_cli(x86_debug *debug, debug_fake *fake)
{
    x86_debug_result result;
    const char *names[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di", "ip" };
    fake->register_fixture = fake->memory_fixture = LIB_TRUE;
    lib_memory_set(fake->registers, 0, sizeof(fake->registers));
    lib_memory_set(fake->memory, 0, sizeof(fake->memory));
    transcript(debug, "h 1 2", "0003  FFFF\n", "-");
    transcript(debug, "", "", "-");
    transcript(debug, " \t ", "", "-");
    /* All original low-word assignments must retain their 32-bit aliases. */
    for (lib_u32 i = 0; i < 9u; ++i) {
        char line[16], expected[32];
        fake->registers[i] = 0x89abcdefu;
        snprintf(line, sizeof(line), "r%s", names[i]);
        snprintf(expected, sizeof(expected), "%c%c CDEF\n", names[i][0] - 32, names[i][1] - 32);
        transcript(debug, line, expected, ":");
        transcript(debug, "1234", "", "-");
        assert(fake->registers[i] == 0x89ab1234u);
        snprintf(expected, sizeof(expected), "%c%c 1234\n", names[i][0] - 32, names[i][1] - 32);
        transcript(debug, line, expected, ":");
        transcript(debug, "", "", "-");
        assert(fake->registers[i] == 0x89ab1234u);
    }
    transcript(debug, "xr eax", "EAX 89AB1234\n", ":");
    transcript(debug, "12345678", "", "-");
    assert(fake->registers[X86_DEBUG_EAX] == 0x12345678u);
    transcript(debug, "rf", "", "NV UP DI PL NZ NA PO NC  -");
    transcript(debug, "cy", "", "-");
    assert((fake->registers[X86_DEBUG_EFLAGS] & 1u) != 0u);
    transcript(debug, "e0:500", "", "0000:0500  00.");
    transcript(debug, "ab", "", "-");
    assert(fake->memory[0x500] == 0xab);
    transcript(debug, "xe 500", "", "L00000500  AB.");
    transcript(debug, "cd", "", "-");
    assert(fake->memory[0x500] == 0xcd);
    transcript(debug, "xe 500", "", "L00000500  CD.");
    transcript(debug, "", "", "-");
    assert(fake->memory[0x500] == 0xcd);
    transcript(debug, "v", "", ":");
    transcript(debug, "Ab", "41 62 \n", "-");
    transcript(debug, "a0:510", "", "0000:0510 ");
    transcript(debug, "nop", "", "0000:0511 ");
    transcript(debug, "; comment", "", "0000:0511 ");
    transcript(debug, "clc", "", "0000:0512 ");
    transcript(debug, "", "", "-");
    assert(fake->memory[0x510] == 0x90 && fake->memory[0x511] == 0xf8);
    transcript(debug, "xa 520", "", "L00000520 ");
    transcript(debug, "nop", "", "L00000521 ");
    assert(x86_debug_submit_line(debug, "not_an_instruction", &result) == LIB_STATUS_OK);
    assert(lib_text_find_substring(result.text, "^ Error\n") && lib_text_compare(result.prompt, "L00000521 ") == 0);
    transcript(debug, "clc", "", "L00000522 ");
    transcript(debug, "", "", "-");
    transcript(debug, "xa", "", "L00000522 ");
    transcript(debug, "", "", "-");
    assert(fake->memory[0x520] == 0x90 && fake->memory[0x521] == 0xf8);
    lib_memory_set(fake->memory + 0x540, 0x66, 14u);
    fake->memory[0x54e] = 0x90;
    assert(x86_debug_submit_line(debug, "xu 540 1", &result) == LIB_STATUS_OK);
    assert(lib_text_find_substring(result.text, "<ERROR>") == NULL);
    assert(x86_debug_submit_line(debug, "xu", &result) == LIB_STATUS_OK);
    assert(lib_text_compare_n(result.text, "L0000054F", 9) == 0);
    assert(x86_debug_submit_line(debug, "u 0:540 54e", &result) == LIB_STATUS_OK);
    assert(lib_text_find_substring(result.text, "<ERROR>") == NULL);
    transcript(debug, "r zz", "br Error\n", "-");
    transcript(debug, "t 0", "", "-");
    transcript(debug, "xt 0", "", "-");
    transcript(debug, "xg 500 0", "", "-");
    /* Original dump formatting, all 256 rows, including its backspace dash.
     * This exceeds both old copied buffers; no prefix/tail may disappear. */
    lib_memory_set(fake->memory, 'A', sizeof(fake->memory));
    assert(x86_debug_submit_line(debug, "xd 0 1000", &result) == LIB_STATUS_OK);
    const char *cursor = result.text;
    for (lib_u32 address = 0; address < 0x1000; address += 16) {
        char row[128];
        snprintf(row, sizeof(row), "L%08X  41 41 41 41 41 41 41 41 \b-41 41 41 41 41 41 41 41   AAAAAAAAAAAAAAAA\n", address);
        assert(lib_text_compare_n(cursor, row, lib_text_length(row)) == 0);
        cursor += lib_text_length(row);
    }
    assert(*cursor == '\0' && lib_text_length(result.text) > 16384u);
    assert(x86_debug_submit_line(debug, "xd", &result) == LIB_STATUS_OK);
    assert(lib_text_compare_n(result.text, "L00001000", 9) == 0);
    fake->memory_fixture = LIB_FALSE;
    assert(x86_debug_submit_line(debug, "g", &result) == LIB_STATUS_OK);
    assert(result.lifecycle_request == X86_DEBUG_LIFECYCLE_RESUME);
    assert(x86_debug_observe_machine(debug, X86_DEBUG_MACHINE_PAUSED,
        LIB_STATUS_OK, &result) == LIB_STATUS_OK);
    assert(lib_text_find_substring(result.text, "AX=") && !lib_text_find_substring(result.text, "EAX="));
    for (lib_u32 linear = 0; linear < 2u; ++linear) {
        assert(x86_debug_submit_line(debug, linear ? "xt 2" : "t 2", &result) == LIB_STATUS_OK);
        for (lib_u32 step = 0; step < 2u; ++step) {
            assert(x86_debug_observe_machine(debug, X86_DEBUG_MACHINE_PAUSED,
                LIB_STATUS_OK, &result) == LIB_STATUS_OK);
            lib_size length = lib_text_length(result.text);
            assert(length >= 2u && result.text[length - 1u] == '\n');
            assert((result.text[length - 2u] == '\n') == (step == 0u));
            assert(result.lifecycle_request == (step == 0u ?
                X86_DEBUG_LIFECYCLE_RESUME : X86_DEBUG_LIFECYCLE_NONE));
        }
    }
    fake->register_fixture = fake->memory_fixture = LIB_FALSE;
}

int main(void)
{
    machine_fake fake = { 0 };
    debug_fake protocol = { 0 };
    common_machine_driver driver = { 0 };
    common_machine *machine = NULL;
    common_machine_debug_lease lease = { 0 };
    x86_debug_response debug_result = { 0 };
    x86_debug *debug = NULL;
    x86_debug_result debug_command_result = { 0 };

    machine_fake_initialize(&fake, &driver);
    fake.debug = fake_execute_debug;
    fake.debug_context = &protocol;
    assert(common_machine_create(&machine, &driver) == LIB_STATUS_OK);
    common_machine_set_state_sink(machine, machine_fake_note_state, &fake);
    assert(x86_debug_create(&debug) == LIB_STATUS_OK);
    assert(x86_debug_open(debug, machine) == LIB_STATUS_OK);
    assert(fake.debug_calls == 0);
    assert(x86_debug_submit_line(debug, "?", &debug_command_result) == LIB_STATUS_OK);
    assert(debug_command_result.keep_active && fake.debug_calls == 0);
    assert(x86_debug_submit_line(debug, "h 1 2", &debug_command_result) == LIB_STATUS_OK);
    assert(lib_text_find_substring(debug_command_result.text, "0003") != NULL && fake.debug_calls == 0);
    assert(x86_debug_submit_line(debug, "r", &debug_command_result) == LIB_STATUS_OK);
    assert(lib_text_find_substring(debug_command_result.text, "must be paused") != NULL && fake.debug_calls == 0);
    assert(common_machine_start(machine));
    assert(WaitForSingleObject(fake.running, 5000u) == WAIT_OBJECT_0);
    assert(x86_debug_submit_line(debug, "d", &debug_command_result) == LIB_STATUS_OK);
    assert(lib_text_find_substring(debug_command_result.text, "must be paused") != NULL);
    assert(x86_debug_submit_line(debug, "q", &debug_command_result) == LIB_STATUS_OK);
    assert(!debug_command_result.keep_active);
    assert(common_machine_state_get(machine) == COMMON_MACHINE_RUNNING);
    assert(x86_debug_open(debug, machine) == LIB_STATUS_OK);
    assert(fake.debug_calls == 0);
    assert(common_machine_reset(machine));
    assert(WaitForSingleObject(fake.reset_completed, 5000u) == WAIT_OBJECT_0);
    assert(common_machine_state_get(machine) == COMMON_MACHINE_PAUSED);
    assert(common_machine_debug_acquire(machine, &lease) == LIB_STATUS_OK);
    assert(execute_x86(machine, &lease,
        &(x86_debug_request) {
            .operation = X86_DEBUG_READ_REGISTER,
            .address = 0x1234u }, &debug_result) == LIB_STATUS_OK);
    assert(debug_result.value == 0x1234u &&
        InterlockedCompareExchange(&fake.debug_calls, 0, 0) == 1);
    assert(x86_debug_open(debug, machine) == LIB_STATUS_OK);
    assert(fake.debug_calls == 1);
    assert(x86_debug_submit_line(debug, "?", &debug_command_result) ==
        LIB_STATUS_OK);
    assert(lib_text_find_substring(debug_command_result.text, "assemble") != NULL);
    extended_registers(debug, &protocol);
    original_cli(debug, &protocol);
    x86_debug_close(debug);
    x86_debug_destroy(debug);
    /* Immediate paused destruction must not need a caller-side STOP barrier. */
    assert(common_machine_destroy(machine) == LIB_STATUS_OK);
    machine_fake_dispose(&fake);
    return 0;
}
