#include <windows.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "lib/kvm-window/window_interface.h"

static HANDLE entered, exited, retired, done, ticked;
static WNDPROC original;
static HWND target;
static LONG retire_count;

static LRESULT CALLBACK observe(HWND w, UINT m, WPARAM a, LPARAM b)
{
    if (m == WM_ENTERSIZEMOVE || m == WM_ENTERMENULOOP) SetEvent(entered);
    if (m == WM_EXITSIZEMOVE || m == WM_EXITMENULOOP) SetEvent(exited);
    if (m == WM_TIMER) SetEvent(ticked);
    return CallWindowProcW(original, w, m, a, b);
}
static BOOL CALLBACK find_window(HWND w, LPARAM unused)
{
    DWORD pid;
    wchar_t name[64];
    (void)unused;
    GetWindowThreadProcessId(w, &pid);
    GetClassNameW(w, name, 64);
    if (pid == GetCurrentProcessId() && wcscmp(name, L"LibKvmWindow") == 0)
        target = w;
    return TRUE;
}
static lib_i32 input(void *p, const kvm_input_event *e)
{
    (void)p;
    if (e->type == KVM_EVENT_SOURCE_RETIRED) {
        InterlockedIncrement(&retire_count);
        SetEvent(retired);
    }
    return 1;
}
static void failed(void *p, lib_u64 id, lib_status s)
{ (void)p; (void)id; (void)s; assert(!"unexpected component failure"); }
static DWORD WINAPI watchdog(void *p)
{
    (void)p;
    if (WaitForSingleObject(done, 15000) != WAIT_OBJECT_0)
        TerminateProcess(GetCurrentProcess(), 99);
    return 0;
}
int main(void)
{
    const WPARAM commands[] = { SC_SIZE | WMSZ_RIGHT, SC_MOVE, SC_KEYMENU };
    entered = CreateEventA(NULL, TRUE, FALSE, NULL);
    exited = CreateEventA(NULL, TRUE, FALSE, NULL);
    retired = CreateEventA(NULL, TRUE, FALSE, NULL);
    done = CreateEventA(NULL, TRUE, FALSE, NULL);
    ticked = CreateEventA(NULL, TRUE, FALSE, NULL);
    HANDLE guard = CreateThread(NULL, 0, watchdog, NULL, 0, NULL);
    assert(entered && exited && retired && done && ticked && guard);
    for (lib_u32 i = 0; i < sizeof(commands)/sizeof(commands[0]); ++i) {
        kvm_window *w = NULL;
        kvm_window_options o = { 0 };
        char title[128];
        DWORD_PTR result;
        ResetEvent(entered); ResetEvent(exited); ResetEvent(retired);
        retire_count = 0; target = NULL;
        o.component.input_sink = input; o.component.failure_sink = failed;
        o.initial_title = "modal-before";
        assert(kvm_window_create(&w, &o) == LIB_STATUS_OK);
        EnumWindows(find_window, 0); assert(target);
        original = (WNDPROC)SetWindowLongPtrW(target, GWLP_WNDPROC, (LONG_PTR)observe);
        printf("modal case %u\n", i); fflush(stdout);
        assert(PostMessageW(target, WM_SYSCOMMAND, commands[i], i == 2 ? ' ' : 0));
        assert(WaitForSingleObject(entered, 3000) == WAIT_OBJECT_0);
        ResetEvent(ticked);
        assert(WaitForSingleObject(ticked, 3000) == WAIT_OBJECT_0);
        assert(WaitForSingleObject(exited, 0) == WAIT_TIMEOUT);
        assert(kvm_window_freeze(w) == LIB_STATUS_OK);
        assert(kvm_window_set_title(w, "modal-after") == LIB_STATUS_OK);
        assert(SendMessageTimeoutW(target, WM_NULL, 0, 0, SMTO_ABORTIFHUNG, 3000, &result));
        GetWindowTextA(target, title, sizeof(title));
        assert(strcmp(title, "modal-after") == 0);
        /* STOP must unwind the nested loop without an external cancel/Enter. */
        assert(kvm_window_destroy(w)==LIB_STATUS_OK);
        assert(WaitForSingleObject(exited, 0) == WAIT_OBJECT_0);
        assert(WaitForSingleObject(retired, 0) == WAIT_OBJECT_0);
        assert(retire_count == 1 && !IsWindow(target));
    }
    SetEvent(done); assert(WaitForSingleObject(guard, 3000) == WAIT_OBJECT_0);
    CloseHandle(guard); CloseHandle(entered); CloseHandle(exited);
    CloseHandle(retired); CloseHandle(done); CloseHandle(ticked);
    return 0;
}
