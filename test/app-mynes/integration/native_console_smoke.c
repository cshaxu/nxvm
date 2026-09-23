#include <assert.h>
#include <string.h>
#include <windows.h>

#include "product/command.h"
#include "common/machine/machine_interface.h"
#include "common/session/session_interface.h"
#include "common/ui/ui_interface.h"
#include "core/driver_interface.h"
#include "lib/storage/file_interface.h"

#define FIXTURE_PATH "mynes-native-console-fixture.nes"
#define CONSOLE_COLUMNS 80
#define CONSOLE_ROWS 25

typedef struct native_console_fixture {
    core_driver *driver;
    common_machine *machine;
    common_session *session;
    common_ui *ui;
    app_command_context command;
    HANDLE session_thread;
    int session_result;
} native_console_fixture;

typedef struct console_image {
    CHAR_INFO cells[CONSOLE_COLUMNS * CONSOLE_ROWS];
} console_image;

static void write_fixture(void)
{
    lib_u8 bytes[16u + 16384u];
    lib_storage_file_writer *writer = LIB_NULL;
    static const lib_u8 program[] = {
        0x78u, 0xa9u, 0u, 0x8du, 0u, 0x20u,
        0xa9u, 0x3fu, 0x8du, 6u, 0x20u, 0xa9u, 0u, 0x8du, 6u, 0x20u,
        0xa9u, 0x0fu, 0x8du, 7u, 0x20u, 0xa9u, 0x21u, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 6u, 0x20u, 0xa9u, 0u, 0x8du, 6u, 0x20u,
        0xa9u, 0xffu, 0x8du, 7u, 0x20u, 0xa9u, 0xffu, 0x8du, 7u, 0x20u,
        0xa9u, 0xffu, 0x8du, 7u, 0x20u, 0xa9u, 0xffu, 0x8du, 7u, 0x20u,
        0xa9u, 0xffu, 0x8du, 7u, 0x20u, 0xa9u, 0xffu, 0x8du, 7u, 0x20u,
        0xa9u, 0xffu, 0x8du, 7u, 0x20u, 0xa9u, 0xffu, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 7u, 0x20u, 0xa9u, 0u, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 7u, 0x20u, 0xa9u, 0u, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 7u, 0x20u, 0xa9u, 0u, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 7u, 0x20u, 0xa9u, 0u, 0x8du, 7u, 0x20u,
        0xa9u, 0x0au, 0x8du, 1u, 0x20u,
        0xa9u, 1u, 0x8du, 0x16u, 0x40u, 0xa9u, 0u, 0x8du, 0x16u, 0x40u,
        0xadu, 0x16u, 0x40u, 0x29u, 1u, 0xf0u, 4u,
        0xa9u, 0x2au, 0xd0u, 2u, 0xa9u, 0x21u, 0x85u, 0u,
        0xa9u, 0x3fu, 0x8du, 6u, 0x20u, 0xa9u, 1u, 0x8du, 6u, 0x20u,
        0xa5u, 0u, 0x8du, 7u, 0x20u, 0x4cu, 0x79u, 0x80u
    };

    memset(bytes, 0, sizeof(bytes));
    bytes[0] = 'N'; bytes[1] = 'E'; bytes[2] = 'S'; bytes[3] = 0x1au;
    bytes[4] = 1u;
    memcpy(bytes + 16u, program, sizeof(program));
    bytes[16u + 0x3ffcu] = 0u;
    bytes[16u + 0x3ffdu] = 0x80u;
    assert(lib_storage_file_writer_open(FIXTURE_PATH,
        LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_write(writer, bytes, sizeof(bytes)) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
}

static void state_sink(void *opaque, common_machine_state state, lib_u32 generation)
{
    native_console_fixture *fixture = opaque;
    common_session_machine_state session_state = COMMON_SESSION_MACHINE_ERROR;

    switch (state) {
    case COMMON_MACHINE_STOPPED: session_state = COMMON_SESSION_MACHINE_STOPPED; break;
    case COMMON_MACHINE_RUNNING: session_state = COMMON_SESSION_MACHINE_RUNNING; break;
    case COMMON_MACHINE_PAUSED: session_state = COMMON_SESSION_MACHINE_PAUSED; break;
    case COMMON_MACHINE_RESET_COMPLETED:
        session_state = COMMON_SESSION_MACHINE_RESET_COMPLETED; break;
    case COMMON_MACHINE_STARTING: session_state = COMMON_SESSION_MACHINE_INIT; break;
    case COMMON_MACHINE_ERROR: break;
    }
    assert(common_session_enqueue_runtime_completed(fixture->session, session_state,
        generation));
}

static void frame_sink(void *opaque, lib_u32 sequence, lib_bool graphics,
    lib_u32 generation)
{
    native_console_fixture *fixture = opaque;
    assert(common_session_enqueue_frame_completed(fixture->session, sequence,
        graphics, generation));
}

static DWORD WINAPI run_session(void *opaque)
{
    native_console_fixture *fixture = opaque;
    fixture->session_result = common_session_run(fixture->session);
    return 0u;
}

static void wait_for_machine_state(common_machine *machine,
    common_machine_state expected)
{
    DWORD started = GetTickCount();
    do {
        if (common_machine_state_get(machine) == expected) return;
        Sleep(10u);
    } while (GetTickCount() - started < 10000u);
    assert(!"machine did not reach the expected lifecycle state");
}

static void submit_line(native_console_fixture *fixture, const char *text)
{
    common_ui_event event = { 0 };
    lib_size length = strlen(text);
    assert(length < sizeof(event.value.line.text));
    event.kind = COMMON_UI_EVENT_MONITOR_LINE;
    memcpy(event.value.line.text, text, length);
    event.value.line.length = (lib_u32)length;
    assert(common_session_enqueue_ui_event(fixture->session, &event));
}

static void send_key_state(HANDLE input, WORD key, BOOL pressed)
{
    INPUT_RECORD record = { 0 };
    DWORD written = 0u;

    record.EventType = KEY_EVENT;
    record.Event.KeyEvent.bKeyDown = pressed;
    record.Event.KeyEvent.wRepeatCount = 1u;
    record.Event.KeyEvent.wVirtualKeyCode = key;
    record.Event.KeyEvent.wVirtualScanCode = (WORD)MapVirtualKeyW(key,
        MAPVK_VK_TO_VSC);
    record.Event.KeyEvent.uChar.UnicodeChar = (WCHAR)key;
    assert(WriteConsoleInputW(input, &record, 1u, &written));
    assert(written == 1u);
}

static void pause_with_escape(HANDLE input, common_machine *machine)
{
    DWORD started = GetTickCount();

    do {
        if (common_machine_state_get(machine) == COMMON_MACHINE_PAUSED) return;
        /* Resume reports RUNNING before the raw Console reader has completed
         * its broker takeover. Retry the physical Esc cycle until that reader
         * owns input, rather than assuming an arbitrary scheduling delay. */
        send_key_state(input, VK_ESCAPE, FALSE);
        send_key_state(input, VK_ESCAPE, TRUE);
        Sleep(100u);
    } while (GetTickCount() - started < 10000u);
    assert(!"Esc did not pause the native Console");
}

static void capture_console(HANDLE output, console_image *image)
{
    COORD size = { CONSOLE_COLUMNS, CONSOLE_ROWS };
    COORD origin = { 0, 0 };
    SMALL_RECT area = { 0, 0, CONSOLE_COLUMNS - 1, CONSOLE_ROWS - 1 };
    assert(ReadConsoleOutputW(output, image->cells, size, origin, &area));
}

static lib_bool image_differs(const console_image *left, const console_image *right)
{
    return memcmp(left->cells, right->cells, sizeof(left->cells)) != 0 ? LIB_TRUE : LIB_FALSE;
}

static void wait_for_frame(common_machine *machine, HANDLE output, console_image *image)
{
    DWORD started = GetTickCount();
    do {
        if (common_machine_published_frame_sequence(machine) != 0u) {
            capture_console(output, image);
            return;
        }
        Sleep(10u);
    } while (GetTickCount() - started < 3000u);
    assert(!"machine did not publish a Console frame");
}

static void wait_for_changed_image(HANDLE input, HANDLE output,
    const console_image *before)
{
    console_image after;
    DWORD started = GetTickCount();
    DWORD last_input = started;
    do {
        capture_console(output, &after);
        if (image_differs(before, &after)) return;
        /* The native reader may complete its raw-input cutover after the first
         * record.  Reissue a physical press cycle until a guest frame consumes
         * it; the 100 ms held interval exceeds the fixture's pad polling loop. */
        if (GetTickCount() - last_input >= 100u) {
            send_key_state(input, 'K', FALSE);
            send_key_state(input, 'K', TRUE);
            last_input = GetTickCount();
        }
        Sleep(10u);
    } while (GetTickCount() - started < 10000u);
    assert(!"native Console did not present changed text cells");
}

int main(void)
{
    native_console_fixture fixture = { 0 };
    common_machine_driver driver;
    common_session_options session_options;
    common_ui_options ui_options;
    kvm_hotkey_registry hotkeys;
    console_image idle;
    HANDLE input;
    HANDLE output;

    (void)FreeConsole();
    assert(AllocConsole());
    if (GetConsoleWindow() != NULL) ShowWindow(GetConsoleWindow(), SW_HIDE);
    input = CreateFileA("CONIN$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0u, NULL);
    output = INVALID_HANDLE_VALUE;
    assert(input != INVALID_HANDLE_VALUE);
    write_fixture();
    assert(core_driver_create(&fixture.driver, &(core_driver_options) {
        .text_output = LIB_TRUE }) == LIB_STATUS_OK);
    assert(core_driver_make_driver(fixture.driver, &driver) == LIB_STATUS_OK);
    assert(common_machine_create(&fixture.machine, &driver) == LIB_STATUS_OK);
    assert(common_machine_set_removable_media(fixture.machine, FIXTURE_PATH,
        LIB_STORAGE_MEDIUM_READONLY));
    app_command_initialize(&fixture.command, fixture.machine, LIB_TRUE, LIB_TRUE,
        COMMON_SESSION_DISPLAY_CONSOLE);
    session_options = (common_session_options) {
        .display = COMMON_SESSION_DISPLAY_CONSOLE,
        .console_control = LIB_TRUE,
        .machine = fixture.machine,
        .command = { .context = &fixture.command, .open = app_command_open,
            .reject_line = app_command_reject_line, .submit_line = app_command_submit_line,
            .begin_external = app_command_begin_external,
            .handle_hotkey = app_command_handle_hotkey,
            .note_runtime = app_command_note_runtime,
            .note_broker = app_command_note_broker,
            .note_monitor_current = app_command_note_monitor_current }
    };
    assert(common_session_create(&fixture.session, &session_options) == LIB_STATUS_OK);
    common_machine_set_state_sink(fixture.machine, state_sink, &fixture);
    common_machine_set_frame_sink(fixture.machine, frame_sink, &fixture);
    kvm_hotkey_registry_initialize(&hotkeys);
    assert(kvm_hotkey_registry_register(&hotkeys, KVM_KEY_ESCAPE, 0u,
        "pause-toggle") == LIB_STATUS_OK);
    ui_options = (common_ui_options) { .event_context = fixture.session,
        .event_sink = common_session_enqueue_ui_event, .hotkeys = hotkeys,
        .running_window_title = "MyNes native Console smoke",
        .paused_window_title = "MyNes native Console smoke (paused)",
        .graphics_console_status_text = "NES video requires a window." };
    assert(common_ui_create(&fixture.ui, &ui_options) == LIB_STATUS_OK);
    assert(common_session_bind_ui(fixture.session, fixture.ui) == LIB_STATUS_OK);
    fixture.session_thread = CreateThread(NULL, 0u, run_session, &fixture, 0u, NULL);
    assert(fixture.session_thread != NULL);

    wait_for_machine_state(fixture.machine, COMMON_MACHINE_PAUSED);
    Sleep(50u);
    submit_line(&fixture, "resume");
    wait_for_machine_state(fixture.machine, COMMON_MACHINE_RUNNING);
    /* CONOUT$ names the active screen buffer at open time.  The raw KVM
       binding has just selected its private buffer, so open after the
       cutover rather than observing the original cooked monitor buffer. */
    output = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0u, NULL);
    assert(output != INVALID_HANDLE_VALUE);
    wait_for_frame(fixture.machine, output, &idle);
    assert(FlushConsoleInputBuffer(input));
    /* Keep K down until a guest frame observes it; an immediate up record
       would let the serial pad poll see only the released state. */
    send_key_state(input, 'K', TRUE);
    wait_for_changed_image(input, output, &idle);
    send_key_state(input, 'K', FALSE);
    /* Retire any repeated held-key records before sending a one-shot hotkey. */
    Sleep(100u);
    assert(FlushConsoleInputBuffer(input));
    pause_with_escape(input, fixture.machine);
    submit_line(&fixture, "resume");
    wait_for_machine_state(fixture.machine, COMMON_MACHINE_RUNNING);
    submit_line(&fixture, "exit");
    assert(WaitForSingleObject(fixture.session_thread, 5000u) == WAIT_OBJECT_0);
    assert(fixture.session_result == 1);
    CloseHandle(fixture.session_thread);
    assert(common_machine_shutdown(fixture.machine) == LIB_STATUS_OK);
    assert(common_ui_destroy(fixture.ui) == LIB_STATUS_OK);
    assert(common_session_destroy(fixture.session) == LIB_STATUS_OK);
    assert(common_machine_destroy(fixture.machine) == LIB_STATUS_OK);
    assert(core_driver_destroy(fixture.driver) == LIB_STATUS_OK);
    CloseHandle(input);
    CloseHandle(output);
    assert(FreeConsole());
    return 0;
}
