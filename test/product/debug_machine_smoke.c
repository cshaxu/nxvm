#include "lib/types/test.h"
#include "lib/types/win32/test.h"
#include "lib/types/file.h"
#include "emulator/machine/machine_interface.h"
#include "product/debug/debug_interface.h"

/* Only the driver behavior needed by Debug's paused-lease integration.
 * Emulator owns the sole executor; this fixture creates no thread or runner. */
typedef struct machine_fake {
    lib_win32_handle stopped, wake, running, reset_completed;
    emulator_machine_executor_callback callback;
    void *callback_context;
    emulator_machine_debug_execute debug;
    void *debug_context;
    lib_win32_dword executor_thread;
    lib_win32_long debug_calls;
} machine_fake;

static lib_bool fixture_reset(void *opaque)
{
    machine_fake *fake = opaque;
    lib_win32_reset_event(fake->stopped);
    lib_win32_reset_event(fake->wake);
    return LIB_TRUE;
}

static lib_bool fixture_run(void *opaque)
{
    machine_fake *fake = opaque;
    lib_win32_handle events[] = { fake->stopped, fake->wake };
    fake->executor_thread = lib_win32_get_current_thread_id();
    if (fake->callback != LIB_NULL) fake->callback(fake->callback_context);
    for (;;) {
        lib_win32_dword result = lib_win32_wait_for_multiple_objects(2u, events,
            LIB_WIN32_FALSE, 5000u);
        if (result == LIB_WIN32_WAIT_OBJECT_0) return LIB_TRUE;
        if (result != LIB_WIN32_WAIT_OBJECT_0 + 1u) return LIB_FALSE;
        lib_win32_reset_event(fake->wake);
        if (fake->callback != LIB_NULL) fake->callback(fake->callback_context);
    }
}

static void fixture_stop(void *opaque)
{ lib_win32_set_event(((machine_fake *)opaque)->stopped); }
static void fixture_wake(void *opaque)
{ lib_win32_set_event(((machine_fake *)opaque)->wake); }
static void fixture_heartbeat(void *opaque, lib_bool enabled)
{ (void)opaque; (void)enabled; }
static void fixture_callback(void *opaque,
    emulator_machine_executor_callback callback, void *context)
{
    machine_fake *fake = opaque;
    fake->callback = callback;
    fake->callback_context = context;
}
static void fixture_input(void *opaque, const kvm_input_event *event)
{ (void)opaque; (void)event; }
static lib_status fixture_frame(void *opaque, emulator_machine_frame *frame)
{
    (void)opaque;
    frame->window.valid = LIB_FALSE;
    return LIB_STATUS_OK;
}
static lib_status fixture_debug(void *opaque, const void *request, lib_size size,
    void *response, lib_size capacity, lib_size *response_size)
{
    machine_fake *fake = opaque;
    lib_test_assert(lib_win32_get_current_thread_id() == fake->executor_thread);
    lib_win32_interlocked_increment(&fake->debug_calls);
    return fake->debug(fake->debug_context, request, size, response, capacity,
        response_size);
}
static void machine_fake_note_state(void *opaque, emulator_machine_state state,
    lib_u32 generation)
{
    machine_fake *fake = opaque;
    (void)generation;
    if (state == EMULATOR_MACHINE_RUNNING) lib_win32_set_event(fake->running);
    if (state == EMULATOR_MACHINE_RESET_COMPLETED)
        lib_win32_set_event(fake->reset_completed);
}
static void machine_fake_initialize(machine_fake *fake,
    emulator_machine_driver *driver)
{
    fake->stopped = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    fake->wake = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    fake->running = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    fake->reset_completed = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    lib_test_assert(fake->stopped && fake->wake && fake->running && fake->reset_completed);
    *driver = (emulator_machine_driver) { .context = fake, .reset = fixture_reset,
        .run = fixture_run, .request_stop = fixture_stop,
        .request_wake = fixture_wake, .set_heartbeat = fixture_heartbeat,
        .set_executor_callback = fixture_callback, .deliver_input = fixture_input,
        .copy_frame = fixture_frame, .execute_debug = fixture_debug };
}
static void machine_fake_dispose(machine_fake *fake)
{
    lib_win32_close_handle(fake->reset_completed);
    lib_win32_close_handle(fake->running);
    lib_win32_close_handle(fake->wake);
    lib_win32_close_handle(fake->stopped);
}

typedef struct debug_fake {
    lib_bool register_fixture;
    lib_u32 registers[PRODUCT_DEBUG_REGISTER_COUNT];
    lib_u32 real_reads, linear_reads;
    lib_bool memory_fixture;
    lib_u8 memory[8192];
} debug_fake;

static lib_status fake_execute_x86(void *opaque,
    const product_debug_request *request,
    product_debug_response *result)
{
    debug_fake *fake = (debug_fake *)opaque;
    if (request == LIB_NULL || result == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *result = (product_debug_response) { .value = request->address };
    if (fake->memory_fixture && (request->operation == PRODUCT_DEBUG_READ_REAL ||
        request->operation == PRODUCT_DEBUG_WRITE_REAL ||
        request->operation == PRODUCT_DEBUG_READ_LINEAR ||
        request->operation == PRODUCT_DEBUG_WRITE_LINEAR)) {
        lib_u32 address = request->operation == PRODUCT_DEBUG_READ_REAL ||
            request->operation == PRODUCT_DEBUG_WRITE_REAL ?
            ((lib_u32)request->segment << 4) + request->offset : request->address;
        lib_test_assert(address + request->bytes <= sizeof(fake->memory));
        if (request->operation == PRODUCT_DEBUG_WRITE_REAL ||
            request->operation == PRODUCT_DEBUG_WRITE_LINEAR)
            lib_memory_copy(fake->memory + address, request->data, request->bytes);
        else lib_memory_copy(result->data, fake->memory + address, request->bytes);
        return LIB_STATUS_OK;
    }
    if (fake->register_fixture) {
        switch (request->operation) {
        case PRODUCT_DEBUG_READ_REGISTER:
            result->value = fake->registers[request->register_id];
            break;
        case PRODUCT_DEBUG_WRITE_REGISTER:
            fake->registers[request->register_id] = request->address;
            break;
        case PRODUCT_DEBUG_READ_REAL:
            ++fake->real_reads;
            lib_memory_set(result->data, 0x90, sizeof(result->data));
            break;
        case PRODUCT_DEBUG_READ_LINEAR:
            ++fake->linear_reads;
            lib_memory_set(result->data, 0x90, sizeof(result->data));
            break;
        case PRODUCT_DEBUG_GET_CODE_BASE:
            result->value = 0x10000000u;
            break;
        case PRODUCT_DEBUG_GET_CODE_DEFAULT_SIZE:
            result->value = 1u;
            break;
        case PRODUCT_DEBUG_GET_EXECUTION_RESULT:
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
    product_debug_request request;
    product_debug_response result;
    lib_status status;
    lib_test_assert(size == sizeof(request) && capacity == sizeof(result));
    lib_memory_copy(&request, bytes, size);
    status = fake_execute_x86(opaque, &request, &result);
    if (status == LIB_STATUS_OK) {
        lib_memory_copy(response, &result, sizeof(result));
        *response_size = sizeof(result);
    }
    return status;
}

static lib_status execute_x86(emulator_machine *machine,
    const emulator_machine_debug_lease *lease, const product_debug_request *request,
    product_debug_response *response)
{
    lib_size size;
    lib_status status = emulator_machine_debug_execute_with_lease(machine, lease,
        request, sizeof(*request), response, sizeof(*response), &size);
    lib_test_assert(size == (status == LIB_STATUS_OK ? sizeof(*response) : 0u));
    return status;
}

static void extended_registers(product_debug *debug, debug_fake *fake)
{
    product_debug_result result;
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
    fake->registers[PRODUCT_DEBUG_EFLAGS] = 0x37fd7u;
    for (index = 0; index < sizeof(commands) / sizeof(commands[0]); ++index) {
        fake->real_reads = fake->linear_reads = 0u;
        lib_test_assert(product_debug_submit_line(debug, commands[index], &result) == LIB_STATUS_OK);
        if (index >= 4u) {
            lib_test_assert(result.lifecycle_request == PRODUCT_DEBUG_LIFECYCLE_RESUME);
            lib_test_assert(product_debug_observe_machine(debug, PRODUCT_DEBUG_MACHINE_PAUSED,
                LIB_STATUS_OK, &result) == LIB_STATUS_OK);
        }
        lib_test_assert(lib_text_find_substring(result.text, expected) != LIB_NULL);
        lib_test_assert(lib_text_find_substring(result.text, "L22340008 90") != LIB_NULL);
        lib_test_assert(fake->real_reads == 0u && fake->linear_reads == 1u);
    }
    fake->registers[PRODUCT_DEBUG_EFLAGS] = 2u;
    lib_test_assert(product_debug_submit_line(debug, "xr", &result) == LIB_STATUS_OK);
    lib_test_assert(lib_text_find_substring(result.text,
        "EFL=00000002: vm rf nt IOPL=0 of df if tf sf zf af pf cf \n") != LIB_NULL);
    fake->real_reads = fake->linear_reads = 0u;
    lib_test_assert(product_debug_submit_line(debug, "r", &result) == LIB_STATUS_OK);
    lib_test_assert(lib_text_compare_n(result.text, "AX=0000  BX=0003", 15u) == 0);
    lib_test_assert(lib_text_find_substring(result.text, "EAX=") == LIB_NULL && fake->linear_reads == 0u &&
        fake->real_reads == 1u);
    lib_test_assert(lib_text_find_substring(result.text, "0000:0008 90") != LIB_NULL);
    /* Each original XR register continuation reads and writes the full value. */
    for (index = 0; index < 10u; ++index) {
        char line[32];
        char value[16];
        lib_c_snprintf(line, sizeof(line), "xr %s", names[index]);
        lib_c_snprintf(value, sizeof(value), "%08X", fake->registers[index]);
        lib_test_assert(product_debug_submit_line(debug, line, &result) == LIB_STATUS_OK);
        lib_test_assert(lib_text_find_substring(result.text, value) != LIB_NULL);
        lib_test_assert(product_debug_submit_line(debug, "89abcdef", &result) == LIB_STATUS_OK);
        lib_test_assert(fake->registers[index] == 0x89abcdefu);
    }
    lib_test_assert(product_debug_submit_line(debug, "xsreg", &result) == LIB_STATUS_OK);
    lib_test_assert(lib_text_find_substring(result.text, "Data, e, rw, big") != LIB_NULL);
    fake->register_fixture = LIB_FALSE;
}
static void transcript(product_debug *debug, const char *line,
    const char *text, const char *prompt)
{
    product_debug_result result;
    lib_size length = lib_text_length(text);

    while (length != 0u && (text[length - 1u] == '\r' || text[length - 1u] == '\n'))
        --length;
    lib_test_assert(product_debug_submit_line(debug, line, &result) == LIB_STATUS_OK);
    lib_test_assert(lib_text_compare_n(result.text, text, length) == 0 &&
        result.text[length] == '\0');
    lib_test_assert(result.prompt_ready && lib_text_compare(result.prompt, prompt) == 0);
    lib_test_assert(result.lifecycle_request == PRODUCT_DEBUG_LIFECYCLE_NONE);
}

static void original_cli(product_debug *debug, debug_fake *fake)
{
    product_debug_result result;
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
        lib_c_snprintf(line, sizeof(line), "r%s", names[i]);
        lib_c_snprintf(expected, sizeof(expected), "%c%c CDEF\n", names[i][0] - 32, names[i][1] - 32);
        transcript(debug, line, expected, ":");
        transcript(debug, "1234", "", "-");
        lib_test_assert(fake->registers[i] == 0x89ab1234u);
        lib_c_snprintf(expected, sizeof(expected), "%c%c 1234\n", names[i][0] - 32, names[i][1] - 32);
        transcript(debug, line, expected, ":");
        transcript(debug, "", "", "-");
        lib_test_assert(fake->registers[i] == 0x89ab1234u);
    }
    transcript(debug, "xr eax", "EAX 89AB1234\n", ":");
    transcript(debug, "12345678", "", "-");
    lib_test_assert(fake->registers[PRODUCT_DEBUG_EAX] == 0x12345678u);
    transcript(debug, "rf", "", "NV UP DI PL NZ NA PO NC  -");
    transcript(debug, "cy", "", "-");
    lib_test_assert((fake->registers[PRODUCT_DEBUG_EFLAGS] & 1u) != 0u);
    transcript(debug, "e0:500", "", "0000:0500  00.");
    transcript(debug, "ab", "", "-");
    lib_test_assert(fake->memory[0x500] == 0xab);
    transcript(debug, "xe 500", "", "L00000500  AB.");
    transcript(debug, "cd", "", "-");
    lib_test_assert(fake->memory[0x500] == 0xcd);
    transcript(debug, "xe 500", "", "L00000500  CD.");
    transcript(debug, "", "", "-");
    lib_test_assert(fake->memory[0x500] == 0xcd);
    transcript(debug, "v", "", ":");
    transcript(debug, "Ab", "41 62 \n", "-");
    transcript(debug, "a0:510", "", "0000:0510 ");
    transcript(debug, "nop", "", "0000:0511 ");
    transcript(debug, "; comment", "", "0000:0511 ");
    transcript(debug, "clc", "", "0000:0512 ");
    transcript(debug, "", "", "-");
    lib_test_assert(fake->memory[0x510] == 0x90 && fake->memory[0x511] == 0xf8);
    transcript(debug, "xa 520", "", "L00000520 ");
    transcript(debug, "nop", "", "L00000521 ");
    lib_test_assert(product_debug_submit_line(debug, "not_an_instruction", &result) == LIB_STATUS_OK);
    lib_test_assert(lib_text_find_substring(result.text, "^ Error") && lib_text_compare(result.prompt, "L00000521 ") == 0);
    transcript(debug, "clc", "", "L00000522 ");
    transcript(debug, "", "", "-");
    transcript(debug, "xa", "", "L00000522 ");
    transcript(debug, "", "", "-");
    lib_test_assert(fake->memory[0x520] == 0x90 && fake->memory[0x521] == 0xf8);
    lib_memory_set(fake->memory + 0x540, 0x66, 14u);
    fake->memory[0x54e] = 0x90;
    lib_test_assert(product_debug_submit_line(debug, "xu 540 1", &result) == LIB_STATUS_OK);
    lib_test_assert(lib_text_find_substring(result.text, "<ERROR>") == LIB_NULL);
    lib_test_assert(product_debug_submit_line(debug, "xu", &result) == LIB_STATUS_OK);
    lib_test_assert(lib_text_compare_n(result.text, "L0000054F", 9) == 0);
    lib_test_assert(product_debug_submit_line(debug, "u 0:540 54e", &result) == LIB_STATUS_OK);
    lib_test_assert(lib_text_find_substring(result.text, "<ERROR>") == LIB_NULL);
    transcript(debug, "r zz", "br Error\n", "-");
    transcript(debug, "t 0", "", "-");
    transcript(debug, "xt 0", "", "-");
    transcript(debug, "xg 500 0", "", "-");
    /* Original dump formatting, all 256 rows, including its backspace dash.
     * This exceeds both old copied buffers; no prefix/tail may disappear. */
    lib_memory_set(fake->memory, 'A', sizeof(fake->memory));
    lib_test_assert(product_debug_submit_line(debug, "xd 0 1000", &result) == LIB_STATUS_OK);
    const char *cursor = result.text;
    for (lib_u32 address = 0; address < 0x1000; address += 16) {
        char row[128];
        lib_size row_length;
        lib_c_snprintf(row, sizeof(row), "L%08X  41 41 41 41 41 41 41 41 \b-41 41 41 41 41 41 41 41   AAAAAAAAAAAAAAAA\n", address);
        row_length = lib_text_length(row);
        if (address + 16u == 0x1000u) --row_length;
        lib_test_assert(lib_text_compare_n(cursor, row, row_length) == 0);
        cursor += row_length;
    }
    lib_test_assert(*cursor == '\0' && lib_text_length(result.text) > 16384u);
    lib_test_assert(product_debug_submit_line(debug, "xd", &result) == LIB_STATUS_OK);
    lib_test_assert(lib_text_compare_n(result.text, "L00001000", 9) == 0);
    fake->memory_fixture = LIB_FALSE;
    lib_test_assert(product_debug_submit_line(debug, "g", &result) == LIB_STATUS_OK);
    lib_test_assert(result.lifecycle_request == PRODUCT_DEBUG_LIFECYCLE_RESUME);
    lib_test_assert(product_debug_observe_machine(debug, PRODUCT_DEBUG_MACHINE_PAUSED,
        LIB_STATUS_OK, &result) == LIB_STATUS_OK);
    lib_test_assert(lib_text_find_substring(result.text, "AX=") && !lib_text_find_substring(result.text, "EAX="));
    for (lib_u32 linear = 0; linear < 2u; ++linear) {
        lib_test_assert(product_debug_submit_line(debug, linear ? "xt 2" : "t 2", &result) == LIB_STATUS_OK);
        for (lib_u32 step = 0; step < 2u; ++step) {
            lib_test_assert(product_debug_observe_machine(debug, PRODUCT_DEBUG_MACHINE_PAUSED,
                LIB_STATUS_OK, &result) == LIB_STATUS_OK);
            lib_size length = lib_text_length(result.text);
            lib_test_assert(length != 0u && result.text[length - 1u] != '\r' &&
                result.text[length - 1u] != '\n');
            lib_test_assert(result.lifecycle_request == (step == 0u ?
                PRODUCT_DEBUG_LIFECYCLE_RESUME : PRODUCT_DEBUG_LIFECYCLE_NONE));
        }
    }
    fake->register_fixture = fake->memory_fixture = LIB_FALSE;
}

int main(void)
{
    machine_fake fake = { 0 };
    debug_fake protocol = { 0 };
    emulator_machine_driver driver = { 0 };
    emulator_machine *machine = LIB_NULL;
    emulator_machine_debug_lease lease = { 0 };
    product_debug_response debug_result = { 0 };
    product_debug *debug = LIB_NULL;
    product_debug_result debug_command_result = { 0 };

    machine_fake_initialize(&fake, &driver);
    fake.debug = fake_execute_debug;
    fake.debug_context = &protocol;
    lib_test_assert(emulator_machine_create(&machine, &driver) == LIB_STATUS_OK);
    emulator_machine_set_state_sink(machine, machine_fake_note_state, &fake);
    lib_test_assert(product_debug_create(&debug) == LIB_STATUS_OK);
    lib_test_assert(product_debug_open(debug, machine) == LIB_STATUS_OK);
    lib_test_assert(fake.debug_calls == 0);
    lib_test_assert(product_debug_submit_line(debug, "?", &debug_command_result) == LIB_STATUS_OK);
    lib_test_assert(debug_command_result.keep_active && fake.debug_calls == 0);
    lib_test_assert(product_debug_submit_line(debug, "h 1 2", &debug_command_result) == LIB_STATUS_OK);
    lib_test_assert(lib_text_find_substring(debug_command_result.text, "0003") != LIB_NULL && fake.debug_calls == 0);
    lib_test_assert(product_debug_submit_line(debug, "r", &debug_command_result) == LIB_STATUS_OK);
    lib_test_assert(lib_text_find_substring(debug_command_result.text, "must be paused") != LIB_NULL && fake.debug_calls == 0);
    lib_test_assert(emulator_machine_start(machine));
    lib_test_assert(lib_win32_wait_for_single_object(fake.running, 5000u) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(product_debug_submit_line(debug, "d", &debug_command_result) == LIB_STATUS_OK);
    lib_test_assert(lib_text_find_substring(debug_command_result.text, "must be paused") != LIB_NULL);
    lib_test_assert(product_debug_submit_line(debug, "q", &debug_command_result) == LIB_STATUS_OK);
    lib_test_assert(!debug_command_result.keep_active);
    lib_test_assert(emulator_machine_state_get(machine) == EMULATOR_MACHINE_RUNNING);
    lib_test_assert(product_debug_open(debug, machine) == LIB_STATUS_OK);
    lib_test_assert(fake.debug_calls == 0);
    lib_test_assert(emulator_machine_reset(machine));
    lib_test_assert(lib_win32_wait_for_single_object(fake.reset_completed, 5000u) == LIB_WIN32_WAIT_OBJECT_0);
    lib_test_assert(emulator_machine_state_get(machine) == EMULATOR_MACHINE_PAUSED);
    lib_test_assert(emulator_machine_debug_acquire(machine, &lease) == LIB_STATUS_OK);
    lib_test_assert(execute_x86(machine, &lease,
        &(product_debug_request) {
            .operation = PRODUCT_DEBUG_READ_REGISTER,
            .address = 0x1234u }, &debug_result) == LIB_STATUS_OK);
    lib_test_assert(debug_result.value == 0x1234u &&
        lib_win32_interlocked_compare_exchange(&fake.debug_calls, 0, 0) == 1);
    lib_test_assert(product_debug_open(debug, machine) == LIB_STATUS_OK);
    lib_test_assert(fake.debug_calls == 1);
    lib_test_assert(product_debug_submit_line(debug, "?", &debug_command_result) ==
        LIB_STATUS_OK);
    lib_test_assert(lib_text_find_substring(debug_command_result.text, "assemble") != LIB_NULL);
    extended_registers(debug, &protocol);
    original_cli(debug, &protocol);
    product_debug_close(debug);
    product_debug_destroy(debug);
    /* Immediate paused destruction must not need a caller-side STOP barrier. */
    lib_test_assert(emulator_machine_destroy(machine) == LIB_STATUS_OK);
    machine_fake_dispose(&fake);
    return 0;
}
