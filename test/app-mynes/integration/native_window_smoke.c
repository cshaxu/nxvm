#include <assert.h>
#include <windows.h>

#include "product/command.h"
#include "common/machine/machine_interface.h"
#include "common/session/session_interface.h"
#include "common/ui/ui_interface.h"
#include "core/driver_interface.h"
#include "lib/base/sync_interface.h"
#include "lib/storage/file_interface.h"

#define FIXTURE_PATH "mynes-native-window-fixture.nes"

typedef struct native_window_fixture
{
    core_driver *driver;
    common_machine *machine;
    common_session *session;
    common_ui *ui;
    app_command_context command;
    HANDLE session_thread;
    lib_i32 session_result;
    base_sync_event *runtime_processed;
    lib_atomic_i32 expected_runtime;
    lib_atomic_i32 observed_runtime;
} native_window_fixture;

#define NATIVE_WINDOW_RUNTIME_NONE (-1)

static native_window_fixture *native_window_runtime_observer;

static void write_fixture(void)
{
    lib_u8 bytes[16u + 16384u];
    lib_storage_file_writer *writer = LIB_NULL;
    /* NROM-128 with CHR RAM.  Its all-background image changes colour when
       the standard K=A binding reaches the controller shift register. */
    static const lib_u8 program[] = {
        0x78u,
        0xa9u, 0u, 0x8du, 0u, 0x20u,
        0xa9u, 0x3fu, 0x8du, 6u, 0x20u,
        0xa9u, 0u, 0x8du, 6u, 0x20u,
        0xa9u, 0x0fu, 0x8du, 7u, 0x20u,
        0xa9u, 0x21u, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 6u, 0x20u,
        0xa9u, 0u, 0x8du, 6u, 0x20u,
        0xa9u, 0xffu, 0x8du, 7u, 0x20u,
        0xa9u, 0xffu, 0x8du, 7u, 0x20u,
        0xa9u, 0xffu, 0x8du, 7u, 0x20u,
        0xa9u, 0xffu, 0x8du, 7u, 0x20u,
        0xa9u, 0xffu, 0x8du, 7u, 0x20u,
        0xa9u, 0xffu, 0x8du, 7u, 0x20u,
        0xa9u, 0xffu, 0x8du, 7u, 0x20u,
        0xa9u, 0xffu, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 7u, 0x20u,
        0xa9u, 0u, 0x8du, 7u, 0x20u,
        0xa9u, 0x0au, 0x8du, 1u, 0x20u,
        0xa9u, 1u, 0x8du, 0x16u, 0x40u,
        0xa9u, 0u, 0x8du, 0x16u, 0x40u,
        0xadu, 0x16u, 0x40u, 0x29u, 1u, 0xf0u, 4u,
        0xa9u, 0x2au, 0xd0u, 2u, 0xa9u, 0x21u,
        0x85u, 0u,
        0xa9u, 0x3fu, 0x8du, 6u, 0x20u,
        0xa9u, 1u, 0x8du, 6u, 0x20u,
        0xa5u, 0u, 0x8du, 7u, 0x20u,
        0x4cu, 0x79u, 0x80u};

    lib_memory_set(bytes, 0, sizeof(bytes));
    bytes[0] = 'N';
    bytes[1] = 'E';
    bytes[2] = 'S';
    bytes[3] = 0x1au;
    bytes[4] = 1u;
    lib_memory_copy(bytes + 16u, program, sizeof(program));
    bytes[16u + 0x3ffcu] = 0u;
    bytes[16u + 0x3ffdu] = 0x80u;
    assert(lib_storage_file_writer_open(FIXTURE_PATH,
                                        LIB_STORAGE_FILE_WRITER_TRUNCATE, &writer) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_write(writer, bytes, sizeof(bytes)) == LIB_STATUS_OK);
    assert(lib_storage_file_writer_close(writer) == LIB_STATUS_OK);
}

static void state_sink(void *opaque, common_machine_state state, lib_u32 generation)
{
    native_window_fixture *fixture = opaque;
    common_session_machine_state session_state = COMMON_SESSION_MACHINE_ERROR;

    switch (state)
    {
    case COMMON_MACHINE_STOPPED:
        session_state = COMMON_SESSION_MACHINE_STOPPED;
        break;
    case COMMON_MACHINE_RUNNING:
        session_state = COMMON_SESSION_MACHINE_RUNNING;
        break;
    case COMMON_MACHINE_PAUSED:
        session_state = COMMON_SESSION_MACHINE_PAUSED;
        break;
    case COMMON_MACHINE_RESET_COMPLETED:
        if (fixture->command.suppress_window_after_reset)
        {
            fixture->command.suppress_window_after_reset = LIB_FALSE;
            fixture->command.report_suppressed_reset = LIB_TRUE;
            session_state = COMMON_SESSION_MACHINE_PAUSED;
        }
        else
            session_state = COMMON_SESSION_MACHINE_RESET_COMPLETED;
        break;
    case COMMON_MACHINE_STARTING:
        session_state = COMMON_SESSION_MACHINE_INIT;
        break;
    case COMMON_MACHINE_ERROR:
        break;
    }
    assert(common_session_enqueue_runtime_completed(fixture->session, session_state,
                                                    generation));
}

static void frame_sink(void *opaque, lib_u32 sequence, lib_bool graphics,
                       lib_u32 generation)
{
    native_window_fixture *fixture = opaque;
    assert(common_session_enqueue_frame_completed(fixture->session, sequence,
                                                  graphics, generation));
}

static void note_runtime(void *opaque, common_session_machine_state prior,
                         common_session_machine_state completed,
                         common_session_command_result *out_result)
{
    native_window_fixture *fixture = native_window_runtime_observer;

    app_command_note_runtime(opaque, prior, completed, out_result);
    if (fixture != NULL)
        lib_atomic_i32_store_explicit(&fixture->observed_runtime, (lib_i32)completed,
                                      LIB_MEMORY_ORDER_SEQ_CST);
}

static void note_monitor_current(void *opaque, lib_bool current,
                                 common_session_command_result *out_result)
{
    native_window_fixture *fixture = native_window_runtime_observer;

    app_command_note_monitor_current(opaque, current, out_result);
    if (fixture != NULL &&
        lib_atomic_i32_load_explicit(&fixture->expected_runtime,
                                     LIB_MEMORY_ORDER_SEQ_CST) ==
            lib_atomic_i32_load_explicit(&fixture->observed_runtime,
                                         LIB_MEMORY_ORDER_SEQ_CST))
    {
        lib_atomic_i32_store_explicit(&fixture->expected_runtime,
                                      NATIVE_WINDOW_RUNTIME_NONE,
                                      LIB_MEMORY_ORDER_SEQ_CST);
        assert(base_sync_event_signal(fixture->runtime_processed) == LIB_STATUS_OK);
    }
}

static DWORD WINAPI run_session(void *opaque)
{
    native_window_fixture *fixture = opaque;
    fixture->session_result = common_session_run(fixture->session);
    return 0u;
}

static HWND window_handle;

static BOOL CALLBACK find_mynes_window(HWND window, LPARAM unused)
{
    wchar_t name[64];
    DWORD process = 0u;
    (void)unused;
    GetWindowThreadProcessId(window, &process);
    GetClassNameW(window, name, (int)(sizeof(name) / sizeof(name[0])));
    if (process == GetCurrentProcessId() && wcscmp(name, L"LibKvmWindow") == 0)
        window_handle = window;
    return TRUE;
}

static HWND wait_for_window(void)
{
    DWORD started = GetTickCount();
    do
    {
        window_handle = NULL;
        EnumWindows(find_mynes_window, 0);
        if (window_handle != NULL)
            return window_handle;
        Sleep(10u);
    } while (GetTickCount() - started < 3000u);
    return NULL;
}

static void assert_no_window(void)
{
    window_handle = NULL;
    EnumWindows(find_mynes_window, 0);
    assert(window_handle == NULL);
}

static void expect_runtime(native_window_fixture *fixture,
                           common_session_machine_state expected)
{
    assert(base_sync_event_reset(fixture->runtime_processed) == LIB_STATUS_OK);
    lib_atomic_i32_store_explicit(&fixture->expected_runtime, (lib_i32)expected,
                                  LIB_MEMORY_ORDER_SEQ_CST);
}

static void wait_for_runtime(native_window_fixture *fixture,
                             common_session_machine_state expected,
                             common_machine_state machine_expected)
{
    assert(base_sync_event_wait(fixture->runtime_processed, 3000u) ==
           BASE_SYNC_WAIT_SIGNALED);
    assert(lib_atomic_i32_load_explicit(&fixture->observed_runtime,
                                        LIB_MEMORY_ORDER_SEQ_CST) == (lib_i32)expected);
    assert(common_machine_state_get(fixture->machine) == machine_expected);
}

static COLORREF wait_for_pixel(HWND window, COLORREF previous)
{
    DWORD started = GetTickCount();
    do
    {
        RECT client;
        HDC dc = GetDC(window);
        COLORREF pixel = CLR_INVALID;
        if (dc != NULL && GetClientRect(window, &client))
            pixel = GetPixel(dc, (client.right - client.left) / 2,
                             (client.bottom - client.top) / 2);
        if (dc != NULL)
            ReleaseDC(window, dc);
        /* A newly created native client area is black before its first Core
         * frame is painted.  It proves only window creation, not gameplay
         * presentation, so wait for the fixture's nonblack first frame. */
        if (pixel != CLR_INVALID && pixel != previous &&
            (previous != CLR_INVALID || pixel != RGB(0u, 0u, 0u)))
            return pixel;
        Sleep(10u);
    } while (GetTickCount() - started < 3000u);
    return CLR_INVALID;
}

static void wait_for_title(HWND window, const char *expected)
{
    DWORD started = GetTickCount();
    do
    {
        char title[128];
        GetWindowTextA(window, title, (int)sizeof(title));
        if (lib_text_compare(title, expected) == 0)
            return;
        Sleep(10u);
    } while (GetTickCount() - started < 3000u);
    assert(!"Window did not reach its expected title");
}

static void send_hotkey(HWND window, WPARAM key)
{
    assert(PostMessageW(window, WM_KEYDOWN, key, 0));
    assert(PostMessageW(window, WM_KEYUP, key, 0));
}

static void wait_for_window_retirement(HWND window)
{
    DWORD started = GetTickCount();
    do
    {
        if (!IsWindow(window))
            return;
        Sleep(10u);
    } while (GetTickCount() - started < 3000u);
    assert(!"Window close did not retire the native receiver");
}

static void submit_line(native_window_fixture *fixture, const char *text)
{
    common_ui_event event = {0};
    lib_size length = lib_text_length(text);
    assert(length < sizeof(event.value.line.text));
    event.kind = COMMON_UI_EVENT_MONITOR_LINE;
    lib_memory_copy(event.value.line.text, text, length);
    event.value.line.length = (lib_u32)length;
    assert(common_session_enqueue_ui_event(fixture->session, &event));
}

int main(void)
{
    native_window_fixture fixture = {0};
    common_machine_driver driver;
    common_session_options session_options;
    common_ui_options ui_options;
    kvm_hotkey_registry hotkeys;
    HWND window;
    COLORREF idle;
    COLORREF active;

    write_fixture();
    assert(core_driver_create(&fixture.driver, &(core_driver_options){0}) == LIB_STATUS_OK);
    assert(core_driver_make_driver(fixture.driver, &driver) == LIB_STATUS_OK);
    assert(common_machine_create(&fixture.machine, &driver) == LIB_STATUS_OK);
    assert(common_machine_set_removable_media(fixture.machine, FIXTURE_PATH,
                                              LIB_STORAGE_MEDIUM_READONLY));
    app_command_initialize(&fixture.command, fixture.machine, LIB_TRUE,
                           COMMON_SESSION_DISPLAY_WINDOW);
    assert(base_sync_event_create(BASE_SYNC_EVENT_AUTO_RESET,
                                  &fixture.runtime_processed) == LIB_STATUS_OK);
    lib_atomic_i32_initialize(&fixture.expected_runtime,
                              NATIVE_WINDOW_RUNTIME_NONE);
    lib_atomic_i32_initialize(&fixture.observed_runtime,
                              NATIVE_WINDOW_RUNTIME_NONE);
    session_options = (common_session_options){
        .display = COMMON_SESSION_DISPLAY_WINDOW,
        .console_control = LIB_TRUE,
        .machine = fixture.machine,
        .command = {
            .context = &fixture.command,
            .open = app_command_open,
            .reject_line = app_command_reject_line,
            .submit_line = app_command_submit_line,
            .begin_external = app_command_begin_external,
            .handle_hotkey = app_command_handle_hotkey,
            .note_runtime = note_runtime,
            .note_broker = app_command_note_broker,
            .note_monitor_current = note_monitor_current}};
    assert(common_session_create(&fixture.session, &session_options) == LIB_STATUS_OK);
    common_machine_set_state_sink(fixture.machine, state_sink, &fixture);
    common_machine_set_frame_sink(fixture.machine, frame_sink, &fixture);
    state_sink(&fixture, COMMON_MACHINE_STOPPED,
        common_machine_run_generation(fixture.machine));
    kvm_hotkey_registry_initialize(&hotkeys);
    assert(kvm_hotkey_registry_register(&hotkeys, KVM_KEY_ESCAPE, 0u,
                                        "pause-toggle") == LIB_STATUS_OK);
    ui_options = (common_ui_options){
        .event_context = fixture.session,
        .event_sink = common_session_enqueue_ui_event,
        .hotkeys = hotkeys,
        .running_window_title = "MyNes native smoke (Running)",
        .paused_window_title = "MyNes native smoke (Paused)",
        .graphics_console_status_text = "NES video requires a window."};
    assert(common_ui_create(&fixture.ui, &ui_options) == LIB_STATUS_OK);
    assert(common_session_bind_ui(fixture.session, fixture.ui) == LIB_STATUS_OK);
    native_window_runtime_observer = &fixture;
    expect_runtime(&fixture, COMMON_SESSION_MACHINE_STOPPED);
    fixture.session_thread = CreateThread(NULL, 0u, run_session, &fixture, 0u, NULL);
    assert(fixture.session_thread != NULL);

    wait_for_runtime(&fixture, COMMON_SESSION_MACHINE_STOPPED,
                     COMMON_MACHINE_STOPPED);
    assert_no_window();
    expect_runtime(&fixture, COMMON_SESSION_MACHINE_RUNNING);
    submit_line(&fixture, "start");
    wait_for_runtime(&fixture, COMMON_SESSION_MACHINE_RUNNING,
                     COMMON_MACHINE_RUNNING);
    window = wait_for_window();
    assert(window != NULL);
    idle = wait_for_pixel(window, CLR_INVALID);
    assert(idle != CLR_INVALID && idle != RGB(0u, 0u, 0u));
    expect_runtime(&fixture, COMMON_SESSION_MACHINE_PAUSED);
    send_hotkey(window, VK_ESCAPE);
    wait_for_runtime(&fixture, COMMON_SESSION_MACHINE_PAUSED,
                     COMMON_MACHINE_PAUSED);
    wait_for_title(window, "MyNes native smoke (Paused)");
    expect_runtime(&fixture, COMMON_SESSION_MACHINE_RUNNING);
    send_hotkey(window, VK_ESCAPE);
    wait_for_runtime(&fixture, COMMON_SESSION_MACHINE_RUNNING,
                     COMMON_MACHINE_RUNNING);
    wait_for_title(window, "MyNes native smoke (Running)");
    expect_runtime(&fixture, COMMON_SESSION_MACHINE_PAUSED);
    send_hotkey(window, VK_ESCAPE);
    wait_for_runtime(&fixture, COMMON_SESSION_MACHINE_PAUSED,
                     COMMON_MACHINE_PAUSED);
    wait_for_title(window, "MyNes native smoke (Paused)");
    expect_runtime(&fixture, COMMON_SESSION_MACHINE_RUNNING);
    send_hotkey(window, VK_ESCAPE);
    wait_for_runtime(&fixture, COMMON_SESSION_MACHINE_RUNNING,
                     COMMON_MACHINE_RUNNING);
    wait_for_title(window, "MyNes native smoke (Running)");
    assert(PostMessageW(window, WM_KEYDOWN, 'K', 0));
    active = wait_for_pixel(window, idle);
    assert(active != CLR_INVALID && active != idle);
    assert(PostMessageW(window, WM_KEYUP, 'K', 0));

    expect_runtime(&fixture, COMMON_SESSION_MACHINE_PAUSED);
    assert(PostMessageW(window, WM_CLOSE, 0u, 0));
    wait_for_window_retirement(window);
    wait_for_runtime(&fixture, COMMON_SESSION_MACHINE_PAUSED,
                     COMMON_MACHINE_PAUSED);

    submit_line(&fixture, "exit");
    assert(WaitForSingleObject(fixture.session_thread, 5000u) == WAIT_OBJECT_0);
    assert(fixture.session_result == 1);
    CloseHandle(fixture.session_thread);
    assert(common_machine_shutdown(fixture.machine) == LIB_STATUS_OK);
    assert(common_ui_destroy(fixture.ui) == LIB_STATUS_OK);
    native_window_runtime_observer = NULL;
    base_sync_event_destroy(fixture.runtime_processed);
    assert(common_session_destroy(fixture.session) == LIB_STATUS_OK);
    assert(common_machine_destroy(fixture.machine) == LIB_STATUS_OK);
    assert(core_driver_destroy(fixture.driver) == LIB_STATUS_OK);
    return 0;
}
