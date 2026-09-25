#include "lib/types/test.h"
#include "lib/types/file.h"
#include "lib/types/win32/test.h"
#include "lib/kvm-window/window_interface.h"

static lib_win32_handle entered, exited, retired, done, ticked;
static lib_win32_wndproc original;
static lib_win32_hwnd target;
static lib_win32_long retire_count;

static lib_win32_lresult LIB_WIN32_CALLBACK observe(lib_win32_hwnd w, lib_win32_uint m, lib_win32_wparam a, lib_win32_lparam b)
{
    if (m == LIB_WIN32_WM_ENTERSIZEMOVE || m == LIB_WIN32_WM_ENTERMENULOOP) lib_win32_set_event(entered);
    if (m == LIB_WIN32_WM_EXITSIZEMOVE || m == LIB_WIN32_WM_EXITMENULOOP) lib_win32_set_event(exited);
    if (m == LIB_WIN32_WM_TIMER) lib_win32_set_event(ticked);
    return lib_win32_call_window_proc_w(original, w, m, a, b);
}
static lib_win32_bool LIB_WIN32_CALLBACK find_window(lib_win32_hwnd w, lib_win32_lparam unused)
{
    lib_win32_dword pid;
    wchar_t name[64];
    (void)unused;
    lib_win32_get_window_thread_process_id(w, &pid);
    lib_win32_get_class_name_w(w, name, 64);
    if (pid == lib_win32_get_current_process_id() && wcscmp(name, L"LibKvmWindow") == 0)
        target = w;
    return LIB_WIN32_TRUE;
}
static lib_bool input(void *p, const kvm_input_event *e)
{
    (void)p;
    if (e->type == KVM_EVENT_SOURCE_RETIRED) {
        lib_win32_interlocked_increment(&retire_count);
        lib_win32_set_event(retired);
    }
    return LIB_TRUE;
}
static void failed(void *p, lib_u64 id, lib_status s)
{ (void)p; (void)id; (void)s; lib_test_assert(!"unexpected component failure"); }
static lib_win32_dword LIB_WIN32_WINAPI watchdog(void *p)
{
    (void)p;
    if (lib_win32_wait_for_single_object(done, 15000) != LIB_WIN32_WAIT_OBJECT_0)
        lib_win32_terminate_process(lib_win32_get_current_process(), 99);
    return 0;
}
int main(void)
{
    const lib_win32_wparam commands[] = { LIB_WIN32_SC_SIZE | LIB_WIN32_WMSZ_RIGHT, LIB_WIN32_SC_MOVE, LIB_WIN32_SC_KEYMENU };
    entered = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    exited = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    retired = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    done = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    ticked = lib_win32_create_event_a(LIB_NULL, LIB_WIN32_TRUE, LIB_WIN32_FALSE, LIB_NULL);
    lib_win32_handle guard = lib_win32_create_thread(LIB_NULL, 0, watchdog, LIB_NULL, 0, LIB_NULL);
    lib_test_assert(entered && exited && retired && done && ticked && guard);
    for (lib_u32 i = 0; i < sizeof(commands)/sizeof(commands[0]); ++i) {
        kvm_window *w = LIB_NULL;
        kvm_window_options o = { 0 };
        char title[128];
        lib_win32_dword_ptr result;
        lib_win32_reset_event(entered); lib_win32_reset_event(exited); lib_win32_reset_event(retired);
        retire_count = 0; target = LIB_NULL;
        o.component.input_sink = input; o.component.failure_sink = failed;
        o.initial_title = "modal-before";
        lib_test_assert(kvm_window_create(&w, &o) == LIB_STATUS_OK);
        lib_win32_enum_windows(find_window, 0); lib_test_assert(target);
        original = (lib_win32_wndproc)lib_win32_set_window_long_ptr_w(target, LIB_WIN32_GWLP_WNDPROC, (lib_win32_long_ptr)observe);
        lib_c_printf("modal case %u\n", i); lib_c_fflush(lib_c_stdout);
        lib_test_assert(lib_win32_post_message_w(target, LIB_WIN32_WM_SYSCOMMAND, commands[i], i == 2 ? ' ' : 0));
        lib_test_assert(lib_win32_wait_for_single_object(entered, 3000) == LIB_WIN32_WAIT_OBJECT_0);
        lib_win32_reset_event(ticked);
        lib_test_assert(lib_win32_wait_for_single_object(ticked, 3000) == LIB_WIN32_WAIT_OBJECT_0);
        lib_test_assert(lib_win32_wait_for_single_object(exited, 0) == LIB_WIN32_WAIT_TIMEOUT);
        lib_test_assert(kvm_window_freeze(w) == LIB_STATUS_OK);
        lib_test_assert(kvm_window_set_title(w, "modal-after") == LIB_STATUS_OK);
        lib_test_assert(lib_win32_send_message_timeout_w(target, LIB_WIN32_WM_NULL, 0, 0, LIB_WIN32_SMTO_ABORT_IF_HUNG, 3000, &result));
        lib_win32_get_window_text_a(target, title, sizeof(title));
        lib_test_assert(lib_text_compare(title, "modal-after") == 0);
        /* STOP must unwind the nested loop without an external cancel/Enter. */
        lib_test_assert(kvm_window_destroy(w)==LIB_STATUS_OK);
        lib_test_assert(lib_win32_wait_for_single_object(exited, 0) == LIB_WIN32_WAIT_OBJECT_0);
        lib_test_assert(lib_win32_wait_for_single_object(retired, 0) == LIB_WIN32_WAIT_OBJECT_0);
        lib_test_assert(retire_count == 1 && !lib_win32_is_window(target));
    }
    lib_win32_set_event(done); lib_test_assert(lib_win32_wait_for_single_object(guard, 3000) == LIB_WIN32_WAIT_OBJECT_0);
    lib_win32_close_handle(guard); lib_win32_close_handle(entered); lib_win32_close_handle(exited);
    lib_win32_close_handle(retired); lib_win32_close_handle(done); lib_win32_close_handle(ticked);
    return 0;
}
