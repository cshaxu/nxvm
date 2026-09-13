#include "type.h"

#include "lib/types/types_interface.h"

#include "common/ui/console_host.h"
#include "lib/kvm-console/console_interface.h"
#include "lib/kvm-window/window_interface.h"

struct kvm_console { lib_console *logical_console; };
struct kvm_window {
    lib_bool frozen;
    kvm_component_options component;
};

static lib_status common_ui_fake_window_create_status;
static lib_status common_ui_fake_claim_status;
static kvm_window *common_ui_fake_window;

void common_ui_fake_reset(void)
{
    common_ui_fake_window_create_status = LIB_STATUS_OK;
    common_ui_fake_claim_status = LIB_STATUS_OK;
    common_ui_fake_window = LIB_NULL;
}

void common_ui_fake_fail_window_create(lib_status status)
{ common_ui_fake_window_create_status = status; }

void common_ui_fake_fail_claim(lib_status status)
{ common_ui_fake_claim_status = status; }

struct common_ui_console_host {
    void *line_context;
    common_ui_console_line_sink line_sink;
    void *failure_context;
    common_ui_failure_sink failure_sink;
};

lib_status common_ui_console_host_create(common_ui_console_host **out_host,
    void *line_context, common_ui_console_line_sink line_sink,
    void *failure_context, common_ui_failure_sink failure_sink)
{
    common_ui_console_host *host;

    if (out_host == LIB_NULL || line_sink == LIB_NULL || failure_sink == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_host = LIB_NULL;
    host = lib_allocate_zero(1u, sizeof(*host));
    if (host == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    host->line_context = line_context;
    host->line_sink = line_sink;
    host->failure_context = failure_context;
    host->failure_sink = failure_sink;
    *out_host = host;
    return LIB_STATUS_OK;
}

void common_ui_console_host_destroy(common_ui_console_host *host)
{ lib_release(host); }

lib_status common_ui_console_host_request_line(common_ui_console_host *host)
{
    char line[256];

    if (host == LIB_NULL || host->line_sink == LIB_NULL ||
        STD_FGETS(line, (C_INT)sizeof(line), STD_STDIN) == LIB_NULL)
        return LIB_STATUS_INVALID_STATE;
    return host->line_sink(host->line_context, line);
}

lib_status common_ui_console_host_write(common_ui_console_host *host, const char *text)
{
    (void)host;
    return text == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        STD_FPUTS(text, STD_STDOUT) < 0 ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
}

lib_status common_ui_console_host_claim_guest(common_ui_console_host *host,
    lib_console *guest_console)
{
    return host == LIB_NULL || guest_console == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        common_ui_fake_claim_status;
}

lib_status common_ui_console_host_release_guest(common_ui_console_host *host,
    lib_console *guest_console)
{
    return host == LIB_NULL || guest_console == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        LIB_STATUS_OK;
}

lib_status kvm_console_create(kvm_console **out_console,
    const kvm_console_options *options)
{
    kvm_console *console;

    if (out_console == LIB_NULL || options == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_console = LIB_NULL;
    console = lib_allocate_zero(1u, sizeof(*console));
    if (console == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    console->logical_console = (lib_console *)console;
    *out_console = console;
    return LIB_STATUS_OK;
}

lib_status kvm_console_publish_frame(kvm_console *console, const kvm_frame *frame)
{ return console == LIB_NULL || frame == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_OK; }

lib_status kvm_console_destroy(kvm_console *console)
{
    lib_release(console);
    return LIB_STATUS_OK;
}

lib_console *kvm_console_get_console(const kvm_console *console)
{ return console == LIB_NULL ? LIB_NULL : console->logical_console; }

lib_status kvm_window_create(kvm_window **out_window, const kvm_window_options *options)
{
    kvm_window *window;

    if (out_window == LIB_NULL || options == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_window = LIB_NULL;
    if (common_ui_fake_window_create_status != LIB_STATUS_OK)
        return common_ui_fake_window_create_status;
    window = lib_allocate_zero(1u, sizeof(*window));
    if (window == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    window->frozen = options->initial_frozen;
    window->component = options->component;
    common_ui_fake_window = window;
    *out_window = window;
    return LIB_STATUS_OK;
}

lib_status kvm_window_publish_frame(kvm_window *window, const kvm_frame *frame)
{ return window == LIB_NULL || frame == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_OK; }

lib_status kvm_window_destroy(kvm_window *window)
{
    if (common_ui_fake_window == window) common_ui_fake_window = LIB_NULL;
    lib_release(window);
    return LIB_STATUS_OK;
}

lib_status kvm_window_set_title(kvm_window *window, const char *title)
{ return window == LIB_NULL || title == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_OK; }

lib_status kvm_window_freeze(kvm_window *window)
{
    if (window == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    window->frozen = LIB_TRUE;
    return LIB_STATUS_OK;
}

lib_status kvm_window_unfreeze(kvm_window *window)
{
    if (window == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    window->frozen = LIB_FALSE;
    return LIB_STATUS_OK;
}

lib_status kvm_window_release_mouse(kvm_window *window)
{ return window == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_OK; }

int common_ui_fake_emit_window_input(const kvm_input_event *event)
{
    return common_ui_fake_window == LIB_NULL ||
        common_ui_fake_window->component.input_sink == LIB_NULL ? 0 :
        common_ui_fake_window->component.input_sink(
            common_ui_fake_window->component.input_context, event);
}
