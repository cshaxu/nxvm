#include "common/machine/machine_interface.h"
#include "common/debug/debug_interface.h"

#include <windows.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

typedef struct machine_fake {
    HANDLE stopped;
    HANDLE state_stopped;
    HANDLE reset_completed;
    HANDLE wake;
    HANDLE running;
    HANDLE frame;
    HANDLE input;
    common_machine_executor_callback callback;
    void *callback_context;
    LONG resets;
    LONG runs;
    LONG inputs;
    LONG debug_calls;
    DWORD executor_thread;
    HANDLE callback_entered;
    HANDLE callback_release;
    LONG notifications;
    lib_bool register_fixture;
    lib_u32 registers[COMMON_DEBUG_REGISTER_COUNT];
    unsigned real_reads, linear_reads;
    lib_bool memory_fixture;
    lib_u8 memory[8192];
} machine_fake;

static lib_bool fake_reset(void *opaque)
{
    machine_fake *fake = (machine_fake *)opaque;
    ResetEvent(fake->stopped);
    ResetEvent(fake->wake);
    InterlockedIncrement(&fake->resets);
    return LIB_TRUE;
}

static lib_bool fake_run(void *opaque)
{
    machine_fake *fake = (machine_fake *)opaque;
    HANDLE events[2] = { fake->stopped, fake->wake };
    InterlockedIncrement(&fake->runs);
    fake->executor_thread = GetCurrentThreadId();
    if (fake->callback != NULL) fake->callback(fake->callback_context);
    for (;;) {
        DWORD result = WaitForMultipleObjects(2u, events, FALSE, 5000u);
        if (result == WAIT_OBJECT_0) return LIB_TRUE;
        if (result != WAIT_OBJECT_0 + 1u) return LIB_FALSE;
        ResetEvent(fake->wake);
        if (fake->callback != NULL) fake->callback(fake->callback_context);
    }
}

static void fake_request_stop(void *opaque)
{ SetEvent(((machine_fake *)opaque)->stopped); }
static void fake_request_wake(void *opaque)
{ SetEvent(((machine_fake *)opaque)->wake); }
static lib_bool fake_set_media(void *opaque, const char *path)
{ (void)opaque; (void)path; return LIB_TRUE; }
static void fake_heartbeat(void *opaque, lib_bool enabled)
{ (void)opaque; (void)enabled; }
static void fake_set_callback(void *opaque, common_machine_executor_callback callback,
    void *context)
{
    machine_fake *fake = (machine_fake *)opaque;
    fake->callback = callback;
    fake->callback_context = context;
}
static void fake_deliver_input(void *opaque, const kvm_input_event *event)
{
    machine_fake *fake = (machine_fake *)opaque;
    (void)event;
    InterlockedIncrement(&fake->inputs);
    SetEvent(fake->input);
}
static lib_bool fake_copy_frame(void *opaque, kvm_frame *frame)
{
    (void)opaque;
    memset(frame, 0, sizeof(*frame));
    frame->valid = 1u;
    frame->text_columns = KVM_TEXT_COLUMNS;
    frame->text_rows = KVM_TEXT_ROWS;
    return LIB_TRUE;
}
static lib_status fake_execute_debug(void *opaque,
    const common_machine_debug_request *request,
    common_machine_debug_result *result)
{
    machine_fake *fake = (machine_fake *)opaque;
    if (request == NULL || result == NULL) return LIB_STATUS_INVALID_ARGUMENT;
    assert(GetCurrentThreadId() == fake->executor_thread);
    InterlockedIncrement(&fake->debug_calls);
    *result = (common_machine_debug_result) { .value = request->address };
    if (fake->memory_fixture && (request->operation == COMMON_MACHINE_DEBUG_READ_REAL ||
        request->operation == COMMON_MACHINE_DEBUG_WRITE_REAL ||
        request->operation == COMMON_MACHINE_DEBUG_READ_LINEAR ||
        request->operation == COMMON_MACHINE_DEBUG_WRITE_LINEAR)) {
        lib_u32 address = request->operation == COMMON_MACHINE_DEBUG_READ_REAL ||
            request->operation == COMMON_MACHINE_DEBUG_WRITE_REAL ?
            ((lib_u32)request->segment << 4) + request->offset : request->address;
        assert(address + request->bytes <= sizeof(fake->memory));
        if (request->operation == COMMON_MACHINE_DEBUG_WRITE_REAL ||
            request->operation == COMMON_MACHINE_DEBUG_WRITE_LINEAR)
            memcpy(fake->memory + address, request->data, request->bytes);
        else memcpy(result->data, fake->memory + address, request->bytes);
        return LIB_STATUS_OK;
    }
    if (fake->register_fixture) {
        switch (request->operation) {
        case COMMON_MACHINE_DEBUG_READ_REGISTER:
            result->value = fake->registers[request->register_id];
            break;
        case COMMON_MACHINE_DEBUG_WRITE_REGISTER:
            fake->registers[request->register_id] = request->address;
            break;
        case COMMON_MACHINE_DEBUG_READ_REAL:
            ++fake->real_reads;
            memset(result->data, 0x90, sizeof(result->data));
            break;
        case COMMON_MACHINE_DEBUG_READ_LINEAR:
            ++fake->linear_reads;
            memset(result->data, 0x90, sizeof(result->data));
            break;
        case COMMON_MACHINE_DEBUG_GET_CODE_BASE:
            result->value = 0x10000000u;
            break;
        case COMMON_MACHINE_DEBUG_GET_CODE_DEFAULT_SIZE:
            result->value = 1u;
            break;
        case COMMON_MACHINE_DEBUG_GET_EXECUTION_RESULT:
            result->enabled = LIB_TRUE;
            result->value = 1u;
            break;
        default:
            break;
        }
    }
    return LIB_STATUS_OK;
}

static void extended_registers(common_debug *debug, machine_fake *fake)
{
    common_debug_result result;
    const char *names[] = { "eax", "ecx", "edx", "ebx", "esp", "ebp",
        "esi", "edi", "eip", "eflags" };
    const char *commands[] = { "xr", "xreg", "x r", "x reg", "xt", "xg 12345678" };
    const char *expected =
        "EAX=12340000 EBX=12340003 ECX=12340001 EDX=12340002\n"
        "ESP=12340004 EBP=12340005 ESI=12340006 EDI=12340007\n"
        "EIP=12340008 EFL=00037FD7: VM RF NT IOPL=3 OF DF IF TF SF ZF AF PF CF \n";
    unsigned index;
    fake->register_fixture = LIB_TRUE;
    for (index = 0; index < 10u; ++index)
        fake->registers[index] = 0x12340000u + index;
    fake->registers[COMMON_DEBUG_EFLAGS] = 0x37fd7u;
    for (index = 0; index < sizeof(commands) / sizeof(commands[0]); ++index) {
        fake->real_reads = fake->linear_reads = 0u;
        assert(common_debug_submit_line(debug, commands[index], &result) == LIB_STATUS_OK);
        if (index >= 4u) {
            assert(result.lifecycle_request == COMMON_DEBUG_LIFECYCLE_RESUME);
            assert(common_debug_observe_machine(debug, COMMON_DEBUG_MACHINE_PAUSED,
                LIB_STATUS_OK, &result) == LIB_STATUS_OK);
        }
        assert(strstr(result.text, expected) != NULL);
        assert(strstr(result.text, "L22340008 90") != NULL);
        assert(fake->real_reads == 0u && fake->linear_reads == 1u);
    }
    fake->registers[COMMON_DEBUG_EFLAGS] = 2u;
    assert(common_debug_submit_line(debug, "xr", &result) == LIB_STATUS_OK);
    assert(strstr(result.text,
        "EFL=00000002: vm rf nt IOPL=0 of df if tf sf zf af pf cf \n") != NULL);
    fake->linear_reads = 0u;
    assert(common_debug_submit_line(debug, "r", &result) == LIB_STATUS_OK);
    assert(strncmp(result.text, "AX=0000  BX=0003", 15u) == 0);
    assert(strstr(result.text, "EAX=") == NULL && fake->linear_reads == 1u);
    assert(strstr(result.text, "0000:0008 90") != NULL);
    /* Each original XR register continuation reads and writes the full value. */
    for (index = 0; index < 10u; ++index) {
        char line[32];
        char value[16];
        snprintf(line, sizeof(line), "xr %s", names[index]);
        snprintf(value, sizeof(value), "%08X", fake->registers[index]);
        assert(common_debug_submit_line(debug, line, &result) == LIB_STATUS_OK);
        assert(strstr(result.text, value) != NULL);
        assert(common_debug_submit_line(debug, "89abcdef", &result) == LIB_STATUS_OK);
        assert(fake->registers[index] == 0x89abcdefu);
    }
    assert(common_debug_submit_line(debug, "xsreg", &result) == LIB_STATUS_OK);
    assert(strstr(result.text, "Data, e, rw, big") != NULL);
    fake->register_fixture = LIB_FALSE;
}
static void transcript(common_debug *debug, const char *line,
    const char *text, const char *prompt)
{
    common_debug_result result;
    assert(common_debug_submit_line(debug, line, &result) == LIB_STATUS_OK);
    assert(strcmp(result.text, text) == 0);
    assert(result.prompt_ready && strcmp(result.prompt, prompt) == 0);
    assert(result.lifecycle_request == COMMON_DEBUG_LIFECYCLE_NONE);
}

static void original_cli(common_debug *debug, machine_fake *fake)
{
    common_debug_result result;
    const char *names[] = { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di", "ip" };
    fake->register_fixture = fake->memory_fixture = LIB_TRUE;
    memset(fake->registers, 0, sizeof(fake->registers));
    memset(fake->memory, 0, sizeof(fake->memory));
    transcript(debug, "h 1 2", "0003  FFFF\n", "-");
    transcript(debug, "", "", "-");
    transcript(debug, " \t ", "", "-");
    /* All original low-word assignments must retain their 32-bit aliases. */
    for (unsigned i = 0; i < 9u; ++i) {
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
    assert(fake->registers[COMMON_DEBUG_EAX] == 0x12345678u);
    transcript(debug, "rf", "", "NV UP DI PL NZ NA PO NC  -");
    transcript(debug, "cy", "", "-");
    assert((fake->registers[COMMON_DEBUG_EFLAGS] & 1u) != 0u);
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
    assert(common_debug_submit_line(debug, "not_an_instruction", &result) == LIB_STATUS_OK);
    assert(strstr(result.text, "^ Error\n") && strcmp(result.prompt, "L00000521 ") == 0);
    transcript(debug, "clc", "", "L00000522 ");
    transcript(debug, "", "", "-");
    transcript(debug, "xa", "", "L00000522 ");
    transcript(debug, "", "", "-");
    assert(fake->memory[0x520] == 0x90 && fake->memory[0x521] == 0xf8);
    memset(fake->memory + 0x540, 0x66, 14u);
    fake->memory[0x54e] = 0x90;
    assert(common_debug_submit_line(debug, "xu 540 1", &result) == LIB_STATUS_OK);
    assert(strstr(result.text, "<ERROR>") == NULL);
    assert(common_debug_submit_line(debug, "xu", &result) == LIB_STATUS_OK);
    assert(strncmp(result.text, "L0000054F", 9) == 0);
    assert(common_debug_submit_line(debug, "u 0:540 54e", &result) == LIB_STATUS_OK);
    assert(strstr(result.text, "<ERROR>") == NULL);
    transcript(debug, "r zz", "br Error\n", "-");
    transcript(debug, "t 0", "", "-");
    transcript(debug, "xt 0", "", "-");
    transcript(debug, "xg 500 0", "", "-");
    /* Original dump formatting, all 256 rows, including its backspace dash.
     * This exceeds both old copied buffers; no prefix/tail may disappear. */
    memset(fake->memory, 'A', sizeof(fake->memory));
    assert(common_debug_submit_line(debug, "xd 0 1000", &result) == LIB_STATUS_OK);
    const char *cursor = result.text;
    for (unsigned address = 0; address < 0x1000; address += 16) {
        char row[128];
        snprintf(row, sizeof(row), "L%08X  41 41 41 41 41 41 41 41 \b-41 41 41 41 41 41 41 41   AAAAAAAAAAAAAAAA\n", address);
        assert(strncmp(cursor, row, strlen(row)) == 0);
        cursor += strlen(row);
    }
    assert(*cursor == '\0' && strlen(result.text) > 16384u);
    assert(common_debug_submit_line(debug, "xd", &result) == LIB_STATUS_OK);
    assert(strncmp(result.text, "L00001000", 9) == 0);
    fake->memory_fixture = LIB_FALSE;
    assert(common_debug_submit_line(debug, "g", &result) == LIB_STATUS_OK);
    assert(result.lifecycle_request == COMMON_DEBUG_LIFECYCLE_RESUME);
    assert(common_debug_observe_machine(debug, COMMON_DEBUG_MACHINE_PAUSED,
        LIB_STATUS_OK, &result) == LIB_STATUS_OK);
    assert(strstr(result.text, "AX=") && !strstr(result.text, "EAX="));
    for (unsigned linear = 0; linear < 2u; ++linear) {
        assert(common_debug_submit_line(debug, linear ? "xt 2" : "t 2", &result) == LIB_STATUS_OK);
        for (unsigned step = 0; step < 2u; ++step) {
            assert(common_debug_observe_machine(debug, COMMON_DEBUG_MACHINE_PAUSED,
                LIB_STATUS_OK, &result) == LIB_STATUS_OK);
            lib_size length = strlen(result.text);
            assert(length >= 2u && result.text[length - 1u] == '\n');
            assert((result.text[length - 2u] == '\n') == (step == 0u));
            assert(result.lifecycle_request == (step == 0u ?
                COMMON_DEBUG_LIFECYCLE_RESUME : COMMON_DEBUG_LIFECYCLE_NONE));
        }
    }
    fake->register_fixture = fake->memory_fixture = LIB_FALSE;
}

static void note_state(void *opaque, common_machine_state state,
    lib_u32 generation)
{
    machine_fake *fake = (machine_fake *)opaque;
    (void)generation;
    InterlockedIncrement(&fake->notifications);
    if (state == COMMON_MACHINE_STOPPED && fake->callback_entered != NULL) {
        SetEvent(fake->callback_entered);
        assert(WaitForSingleObject(fake->callback_release, 5000u) == WAIT_OBJECT_0);
    }
    if (state == COMMON_MACHINE_RUNNING) SetEvent(fake->running);
    if (state == COMMON_MACHINE_STOPPED) SetEvent(fake->state_stopped);
    if (state == COMMON_MACHINE_RESET_COMPLETED) SetEvent(fake->reset_completed);
}
static void note_frame(void *opaque, lib_u32 sequence, lib_bool graphics,
    lib_u32 generation)
{
    machine_fake *fake = (machine_fake *)opaque;
    (void)sequence; (void)graphics; (void)generation;
    InterlockedIncrement(&fake->notifications);
    SetEvent(fake->frame);
}

static DWORD WINAPI shutdown_machine(void *opaque)
{
    common_machine_shutdown(opaque);
    return 0;
}

/* A blocked final callback must prevent shutdown returning. A thread handle
 * is the completion barrier; no timing sleep guesses at worker quiescence. */
static void shutdown_active(common_machine *machine, machine_fake *fake)
{
    HANDLE thread;
    LONG notifications;
    fake->callback_entered = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake->callback_release = CreateEventA(NULL, TRUE, FALSE, NULL);
    assert(fake->callback_entered && fake->callback_release);
    thread = CreateThread(NULL, 0u, shutdown_machine, machine, 0u, NULL);
    assert(thread != NULL);
    assert(WaitForSingleObject(fake->callback_entered, 5000u) == WAIT_OBJECT_0);
    assert(WaitForSingleObject(thread, 0u) == WAIT_TIMEOUT);
    SetEvent(fake->callback_release);
    assert(WaitForSingleObject(thread, 5000u) == WAIT_OBJECT_0);
    assert(fake->callback == NULL && fake->callback_context == NULL);
    notifications = fake->notifications;
    assert(!common_machine_start(machine));
    assert(!common_machine_reset(machine));
    assert(!common_machine_set_removable_media(machine, NULL));
    common_machine_shutdown(machine);
    common_machine_destroy(machine);
    assert(fake->notifications == notifications);
    CloseHandle(thread);
    CloseHandle(fake->callback_entered); CloseHandle(fake->callback_release);
    fake->callback_entered = fake->callback_release = NULL;
}

int main(void)
{
    machine_fake fake = { 0 };
    common_machine_driver driver = { 0 };
    common_machine *machine = NULL;
    kvm_input_event input = { 0 };
    kvm_frame frame = { 0 };
    lib_u32 generation = 0u;
    common_machine_debug_lease lease = { 0 };
    common_machine_debug_result debug_result = { 0 };
    common_debug *debug = NULL;
    common_debug_result debug_command_result = { 0 };

    fake.stopped = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake.state_stopped = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake.reset_completed = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake.wake = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake.running = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake.frame = CreateEventA(NULL, TRUE, FALSE, NULL);
    fake.input = CreateEventA(NULL, TRUE, FALSE, NULL);
    assert(fake.stopped != NULL && fake.state_stopped != NULL &&
        fake.reset_completed != NULL && fake.wake != NULL && fake.running != NULL &&
        fake.frame != NULL && fake.input != NULL);
    driver.context = &fake;
    driver.reset = fake_reset;
    driver.run = fake_run;
    driver.request_stop = fake_request_stop;
    driver.request_wake = fake_request_wake;
    driver.set_heartbeat = fake_heartbeat;
    driver.set_executor_callback = fake_set_callback;
    driver.deliver_input = fake_deliver_input;
    driver.copy_frame = fake_copy_frame;
    driver.set_removable_media = fake_set_media;
    driver.execute_debug = fake_execute_debug;
    assert(common_machine_create(&machine, &driver) == LIB_STATUS_OK);
    frame.valid = 1u;
    frame.sequence = 77u;
    assert(!common_machine_copy_published_frame(machine, &frame,
        common_machine_run_generation(machine)));
    assert(frame.valid == 1u && frame.sequence == 77u);
    common_machine_set_state_sink(machine, note_state, &fake);
    common_machine_set_frame_sink(machine, note_frame, &fake);
    assert(common_debug_create(&debug) == LIB_STATUS_OK);
    assert(common_debug_open(debug, machine) == LIB_STATUS_OK);
    assert(fake.debug_calls == 0);
    assert(common_debug_submit_line(debug, "?", &debug_command_result) == LIB_STATUS_OK);
    assert(debug_command_result.keep_active && fake.debug_calls == 0);
    assert(common_debug_submit_line(debug, "h 1 2", &debug_command_result) == LIB_STATUS_OK);
    assert(strstr(debug_command_result.text, "0003") != NULL && fake.debug_calls == 0);
    assert(common_debug_submit_line(debug, "r", &debug_command_result) == LIB_STATUS_OK);
    assert(strstr(debug_command_result.text, "must be paused") != NULL && fake.debug_calls == 0);
    assert(common_machine_start(machine));
    assert(WaitForSingleObject(fake.running, 5000u) == WAIT_OBJECT_0);
    assert(WaitForSingleObject(fake.frame, 5000u) == WAIT_OBJECT_0);
    assert(InterlockedCompareExchange(&fake.resets, 0, 0) == 1);
    generation = common_machine_run_generation(machine);
    assert(common_machine_copy_published_frame(machine, &frame, generation));
    assert(frame.valid == 1u && generation == common_machine_run_generation(machine));
    {
        lib_u32 sequence = frame.sequence;
        assert(!common_machine_copy_published_frame(machine, &frame, generation + 1u));
        assert(frame.valid == 1u && frame.sequence == sequence);
    }
    input.type = KVM_EVENT_KEY;
    input.data.key.pressed = 1u;
    assert(common_machine_enqueue_input(machine, &input));
    assert(WaitForSingleObject(fake.input, 5000u) == WAIT_OBJECT_0);
    assert(InterlockedCompareExchange(&fake.inputs, 0, 0) == 1);
    assert(common_debug_submit_line(debug, "d", &debug_command_result) == LIB_STATUS_OK);
    assert(strstr(debug_command_result.text, "must be paused") != NULL);
    assert(common_debug_submit_line(debug, "q", &debug_command_result) == LIB_STATUS_OK);
    assert(!debug_command_result.keep_active);
    assert(common_machine_state_get(machine) == COMMON_MACHINE_RUNNING);
    assert(common_debug_open(debug, machine) == LIB_STATUS_OK);
    assert(fake.debug_calls == 0);
    ResetEvent(fake.running);
    assert(common_machine_reset(machine));
    assert(WaitForSingleObject(fake.reset_completed, 5000u) == WAIT_OBJECT_0);
    assert(InterlockedCompareExchange(&fake.resets, 0, 0) == 2);
    assert(common_machine_state_get(machine) == COMMON_MACHINE_PAUSED);
    assert(common_machine_debug_acquire(machine, &lease) == LIB_STATUS_OK);
    assert(common_machine_debug_execute_with_lease(machine, &lease,
        &(common_machine_debug_request) {
            .operation = COMMON_MACHINE_DEBUG_READ_REGISTER,
            .address = 0x1234u }, &debug_result) == LIB_STATUS_OK);
    assert(debug_result.value == 0x1234u &&
        InterlockedCompareExchange(&fake.debug_calls, 0, 0) == 1);
    assert(common_debug_open(debug, machine) == LIB_STATUS_OK);
    assert(fake.debug_calls == 1);
    assert(common_debug_submit_line(debug, "?", &debug_command_result) ==
        LIB_STATUS_OK);
    assert(strstr(debug_command_result.text, "assemble") != NULL);
    extended_registers(debug, &fake);
    original_cli(debug, &fake);
    common_debug_close(debug);
    common_debug_destroy(debug);
    assert(common_machine_resume(machine));
    assert(common_machine_debug_execute_with_lease(machine, &lease,
        &(common_machine_debug_request) { 0 }, &debug_result) ==
        LIB_STATUS_INVALID_STATE);
    assert(WaitForSingleObject(fake.running, 5000u) == WAIT_OBJECT_0);
    assert(common_machine_stop(machine));
    assert(WaitForSingleObject(fake.state_stopped, 5000u) == WAIT_OBJECT_0);
    assert(common_machine_state_get(machine) == COMMON_MACHINE_STOPPED);
    {
        lib_u32 sequence = frame.sequence;
        assert(!common_machine_copy_published_frame(machine, &frame, generation));
        assert(frame.valid == 1u && frame.sequence == sequence);
    }
    common_machine_shutdown(machine);
    common_machine_shutdown(machine);
    common_machine_destroy(machine);
    /* A never-started worker uses the same terminal path. */
    assert(common_machine_create(&machine, &driver) == LIB_STATUS_OK);
    common_machine_shutdown(machine);
    assert(!common_machine_start(machine));
    common_machine_destroy(machine);
    for (int paused = 0; paused != 2; ++paused) {
        ResetEvent(fake.running); ResetEvent(fake.reset_completed);
        assert(common_machine_create(&machine, &driver) == LIB_STATUS_OK);
        common_machine_set_state_sink(machine, note_state, &fake);
        common_machine_set_frame_sink(machine, note_frame, &fake);
        assert(paused ? common_machine_reset(machine) : common_machine_start(machine));
        assert(WaitForSingleObject(paused ? fake.reset_completed : fake.running,
            5000u) == WAIT_OBJECT_0);
        shutdown_active(machine, &fake);
    }
    common_machine_shutdown(NULL);
    CloseHandle(fake.input); CloseHandle(fake.frame); CloseHandle(fake.running);
    CloseHandle(fake.wake); CloseHandle(fake.reset_completed);
    CloseHandle(fake.state_stopped); CloseHandle(fake.stopped);
    return 0;
}
