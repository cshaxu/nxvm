#include "lib/types/win32/window.h"
#include "lib/types/win32/scalar.h"
#include "lib/kvm-window/win32/mouse.h"

/* Raw mouse registration is process-wide. Never replace an existing consumer. */
static lib_status mouse_registration(lib_win32_raw_input_device *mouse)
{
    lib_win32_uint count = 0u, received;
    lib_win32_raw_input_device *devices;
    lib_memory_set(mouse, 0, sizeof(*mouse));
    if (lib_win32_get_registered_raw_input_devices(LIB_NULL, &count,
            sizeof(*devices)) == (lib_win32_uint)-1) return LIB_STATUS_IO_ERROR;
    if (count == 0u) return LIB_STATUS_OK;
    devices = lib_allocate_zero(count, sizeof(*devices));
    if (devices == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    received = lib_win32_get_registered_raw_input_devices(devices, &count, sizeof(*devices));
    if (received != (lib_win32_uint)-1) {
        for (lib_win32_uint i = 0; i < received; ++i)
            if (devices[i].usUsagePage == 1u && devices[i].usUsage == 2u)
                *mouse = devices[i];
    }
    lib_release(devices);
    return received == (lib_win32_uint)-1 ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
}

static int same_bounds(const lib_win32_rect *a, const lib_win32_rect *b)
{
    return a->left == b->left && a->top == b->top &&
        a->right == b->right && a->bottom == b->bottom;
}

void kvm_win32_mouse_reset(kvm_win32_mouse *mouse)
{
    if (mouse == LIB_NULL) return;
    lib_memory_set(mouse, 0, sizeof(*mouse));
}

lib_status kvm_win32_mouse_release(kvm_win32_mouse *mouse)
{
    lib_status status = LIB_STATUS_OK;
    lib_win32_rect clipped;
    lib_win32_raw_input_device binding;
    if (mouse == LIB_NULL || !mouse->captured) return LIB_STATUS_OK;
    /* Clear ownership before ReleaseCapture synchronously notifies the window. */
    mouse->captured = LIB_FALSE;
    status = mouse_registration(&binding);
    if (status == LIB_STATUS_OK && binding.usUsage == 2u &&
        binding.hwndTarget == mouse->window) {
        binding.dwFlags = LIB_WIN32_RIDEV_REMOVE;
        binding.hwndTarget = LIB_NULL;
        if (!lib_win32_register_raw_input_devices(&binding, 1u, sizeof(binding)))
            status = LIB_STATUS_IO_ERROR;
    }
    /* Do not remove another window's replacement clipping rectangle. */
    if (!lib_win32_get_clip_cursor(&clipped) ||
        (same_bounds(&clipped, &mouse->bounds) && !lib_win32_clip_cursor(LIB_NULL)))
        status = LIB_STATUS_IO_ERROR;
    if (lib_win32_get_capture() == mouse->window && !lib_win32_release_capture())
        status = LIB_STATUS_IO_ERROR;
    mouse->window = LIB_NULL;
    lib_win32_set_cursor(lib_win32_load_cursor_a(LIB_NULL, LIB_WIN32_IDC_ARROW));
    lib_memory_set(&mouse->motion, 0, sizeof(mouse->motion));
    return status;
}

int kvm_win32_mouse_refresh_bounds(kvm_win32_mouse *mouse)
{
    lib_win32_rect client, bounds;
    lib_win32_point upper_left, lower_right;
    if (mouse == LIB_NULL) return 0;
    if (!mouse->captured) return 1;
    if (lib_win32_get_capture() != mouse->window ||
        !lib_win32_get_client_rect(mouse->window, &client) ||
        client.right <= client.left || client.bottom <= client.top) return 0;
    upper_left.x = client.left; upper_left.y = client.top;
    lower_right.x = client.right; lower_right.y = client.bottom;
    if (!lib_win32_client_to_screen(mouse->window, &upper_left) ||
        !lib_win32_client_to_screen(mouse->window, &lower_right)) return 0;
    bounds.left = upper_left.x; bounds.top = upper_left.y;
    bounds.right = lower_right.x; bounds.bottom = lower_right.y;
    if (!lib_win32_clip_cursor(&bounds)) return 0;
    mouse->bounds = bounds;
    lib_memory_set(&mouse->motion, 0, sizeof(mouse->motion));
    return 1;
}

lib_status kvm_win32_mouse_capture(kvm_win32_mouse *mouse,
    lib_win32_hwnd window)
{
    lib_win32_raw_input_device binding;
    lib_status status;
    if (mouse == LIB_NULL || window == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (mouse->captured) return LIB_STATUS_INVALID_STATE;
    status = mouse_registration(&binding);
    if (status != LIB_STATUS_OK) return status;
    if (binding.usUsage == 2u) return LIB_STATUS_INVALID_STATE;
    lib_win32_set_focus(window);
    if (lib_win32_get_focus() != window) return LIB_STATUS_INVALID_STATE;
    lib_win32_set_capture(window);
    if (lib_win32_get_capture() != window) return LIB_STATUS_INVALID_STATE;
    mouse->window = window;
    mouse->captured = LIB_TRUE;
    binding = (lib_win32_raw_input_device){ 1u, 2u, 0u, window };
    /* Keep legacy clicks/keyboard; only raw records supply captured motion. */
    if (!kvm_win32_mouse_refresh_bounds(mouse) ||
        !lib_win32_register_raw_input_devices(&binding, 1u, sizeof(binding))) {
        (void)kvm_win32_mouse_release(mouse);
        return LIB_STATUS_IO_ERROR;
    }
    return LIB_STATUS_OK;
}

int kvm_win32_mouse_move(kvm_win32_mouse *mouse,
    lib_win32_lparam record, int client_width, int client_height,
    unsigned int content_width, unsigned int content_height, int *dx, int *dy)
{
    lib_win32_raw_input input;
    lib_win32_uint size = sizeof(input), received;
    lib_win32_rect clipped;
    lib_win32_word flags;
    int width = 0, height = 0;
    if (mouse == LIB_NULL || dx == LIB_NULL || dy == LIB_NULL) return 0;
    *dx = *dy = 0;
    if (!mouse->captured || lib_win32_get_capture() != mouse->window ||
        !lib_win32_get_clip_cursor(&clipped) ||
        !same_bounds(&clipped, &mouse->bounds)) return 0;
    received = lib_win32_get_raw_input_data((lib_win32_hraw_input)record,
        LIB_WIN32_RID_INPUT, &input, &size, sizeof(lib_win32_raw_input_header));
    if (received == (lib_win32_uint)-1 || received < sizeof(input.header)) return 0;
    if (input.header.dwType != LIB_WIN32_RIM_TYPEMOUSE) return 1;
    if (received < sizeof(input)) return 0;
    flags = input.data.mouse.usFlags &
        (LIB_WIN32_MOUSE_MOVE_ABSOLUTE | LIB_WIN32_MOUSE_VIRTUAL_DESKTOP);
    if (flags & LIB_WIN32_MOUSE_MOVE_ABSOLUTE) {
        width = lib_win32_get_system_metrics((flags & LIB_WIN32_MOUSE_VIRTUAL_DESKTOP) ?
            LIB_WIN32_SM_CXVIRTUALSCREEN : LIB_WIN32_SM_CXSCREEN);
        height = lib_win32_get_system_metrics((flags & LIB_WIN32_MOUSE_VIRTUAL_DESKTOP) ?
            LIB_WIN32_SM_CYVIRTUALSCREEN : LIB_WIN32_SM_CYSCREEN);
        if (width <= 0 || height <= 0 ||
            input.data.mouse.lLastX < 0 || input.data.mouse.lLastX > 65535 ||
            input.data.mouse.lLastY < 0 || input.data.mouse.lLastY > 65535) return 0;
    }
    /* A new device/coordinate space starts a new absolute baseline. */
    if (mouse->device != input.header.hDevice || mouse->coordinate_flags != flags ||
        mouse->desktop_width != width || mouse->desktop_height != height)
        lib_memory_set(&mouse->motion, 0, sizeof(mouse->motion));
    mouse->device = input.header.hDevice;
    mouse->coordinate_flags = flags;
    mouse->desktop_width = width; mouse->desktop_height = height;
    if (flags & LIB_WIN32_MOUSE_MOVE_ABSOLUTE) {
        /* Desktop origin cancels in differences; do not mix client coordinates. */
        int x = (int)((lib_i64)input.data.mouse.lLastX * width / 65535);
        int y = (int)((lib_i64)input.data.mouse.lLastY * height / 65535);
        return kvm_window_motion_move(&mouse->motion, x, y, client_width, client_height,
            content_width, content_height, dx, dy);
    }
    return kvm_window_motion_scale(&mouse->motion,
        input.data.mouse.lLastX, input.data.mouse.lLastY,
        client_width, client_height, content_width, content_height, dx, dy);
}

int kvm_win32_mouse_captured(const kvm_win32_mouse *mouse)
{
    return mouse != LIB_NULL && mouse->captured;
}
