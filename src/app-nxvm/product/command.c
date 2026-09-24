/* Copyright 2012-2014 Neko. */

/* The product owns its command words and their meaning.  Common owns the
 * monitor loop, lifecycle dispatch, presentation transitions and all host
 * queues; this file only supplies the injected command provider. */
#include "lib/types/types_interface.h"
#include <stdarg.h>
#include <stdio.h>

#include "x86/debug/debug_interface.h"
#include "common/session/session_interface.h"
#include "common/ui/ui_interface.h"
#include "app-nxvm/product/composition.h"
#include "app-nxvm/machine/machine_interface.h"
#include "app-nxvm/product/ini_interface.h"
#include "app-nxvm/product/command.h"
#include "app-nxvm/product/keyboard.h"

#define CONSOLE_MAXNARG 256u

struct vm_app_console_context {
    lib_u8 command_buffer[COMMON_SESSION_TEXT_CAPACITY];
    lib_u8 *arguments[CONSOLE_MAXNARG];
    lib_size argument_count;
    vm_app *session;
    common_session *control;
    x86_debug *debug;
    lib_i32 debug_active;
    lib_i32 debug_requested;
};

static vm_machine *vm_app_console_machine(
    const vm_app_console_context *context)
{ return context == LIB_NULL ? LIB_NULL : vm_app_machine(context->session); }

static common_machine *vm_app_console_common_machine(
    const vm_app_console_context *context)
{
    return context == LIB_NULL ? LIB_NULL : vm_app_common_machine(context->session);
}

static void vm_app_console_clear_result(common_session_command_result *result)
{
    if (result != LIB_NULL) *result = (common_session_command_result){0};
}

static void vm_app_console_append(common_session_command_result *result,
    const lib_u8 *format, ...)
{
    lib_size used;
    lib_i32 written;
    va_list arguments;

    if (result == LIB_NULL || format == LIB_NULL) return;
    used = lib_text_length(result->text);
    if (used >= sizeof(result->text)) return;
    va_start(arguments, format);
    written = vsnprintf(result->text + used, sizeof(result->text) - used,
        (const char *)format, arguments);
    va_end(arguments);
    if (written < 0) result->text[used] = '\0';
}

static void vm_app_console_prompt(common_session_command_result *result,
    const lib_u8 *prompt)
{
    if (result == LIB_NULL || prompt == LIB_NULL) return;
    (void)snprintf(result->prompt, sizeof(result->prompt), "%s", (const char *)prompt);
    result->arm_prompt = LIB_TRUE;
}

static void vm_app_console_lower(lib_u8 *text)
{
    while (*text != '\0') {
        if (*text >= 'A' && *text <= 'Z') *text = (lib_u8)(*text + ('a' - 'A'));
        ++text;
    }
}

static lib_bool vm_app_console_text_equal(const lib_u8 *left, const char *right)
{
    while (*left != '\0' && *right != '\0') {
        if (*left++ != (lib_u8)*right++) return LIB_FALSE;
    }
    return *left == '\0' && *right == '\0';
}

static void vm_app_console_parse(vm_app_console_context *context, lib_u8 *line)
{
    lib_u8 *token;

    context->argument_count = 0u;
    token = (lib_u8 *)strtok((char *)line, " \t\n\r\f");
    while (token != LIB_NULL && context->argument_count < CONSOLE_MAXNARG) {
        vm_app_console_lower(token);
        context->arguments[context->argument_count++] = token;
        token = (lib_u8 *)strtok(LIB_NULL, " \t\n\r\f");
    }
}

static lib_i32 vm_app_console_is_running(common_session_machine_state state)
{ return state == COMMON_SESSION_MACHINE_RUNNING; }

static void vm_app_console_help(common_session_command_result *result)
{
    vm_app_console_append(result,
        "VM Console Commands\n"
        "===================\n"
        "HELP    Show help information\n"
        "INFO    List device information\n"
        "SPEED   Show or select machine speed\n"
        "DEBUG   Launch hardware debugger\n"
        "FLOPPY  Insert or eject removable floppy media\n"
        "START | RESET | STOP | RESUME\n"
        "EXIT    Stop the machine and quit the console\n\n");
}

static void vm_app_console_info(const vm_app_console_context *context,
    common_session_command_result *result)
{
    vm_machine_information information;
    vm_machine *machine = vm_app_console_machine(context);

    if (machine == LIB_NULL || vm_machine_get_information(machine, &information) !=
        LIB_STATUS_OK) {
        vm_app_console_append(result, "Machine information unavailable.\n");
        return;
    }
    vm_app_console_append(result,
        "Device Info\n===========\nMachine:           %s\n"
        "CPU:               Intel %s\nRAM Size:          %u %s\n"
        "Floppy Disk Drive: %.2f MB, %s\n",
        vm_profile_name(information.profile_kind),
        core_machine_cpu_profile_name(information.cpu_profile),
        (lib_u32)(information.memory_bytes < (1u << 20) ?
            information.memory_bytes >> 10 : information.memory_bytes >> 20),
        information.memory_bytes < (1u << 20) ? "KB" : "MB",
        information.floppy_image_bytes * 1.0 / ((1 << 10) * 1000),
        information.floppy_media_inserted ? "inserted" : "not inserted");
    if (information.fixed_disk_present)
        vm_app_console_append(result, "Hard Disk Drive:   %u cylinders, %.2f MB, %s\n",
            (lib_u32)information.fixed_disk_cylinders,
            information.fixed_disk_image_bytes * 1.0 / (1 << 20),
            information.fixed_disk_media_connected ? "connected" : "disconnected");
    vm_app_console_append(result, "\nBIOS: %s\nRunning: %s\n",
        information.external_firmware ? "external ROM mapped at F0000h" :
            "profile ROM mapped", information.active ? "Yes" : "No");
}

static const lib_u8 *vm_app_console_speed_name(vm_machine_speed speed)
{ return speed == VM_MACHINE_SPEED_TURBO ? "turbo" : "standard"; }

static void vm_app_console_speed(const vm_app_console_context *context,
    common_session_command_result *result)
{
    vm_machine_speed speed;
    vm_machine *machine = vm_app_console_machine(context);

    if (machine == LIB_NULL) return;
    if (context->argument_count == 1u) {
        if (vm_machine_get_speed(machine, &speed) == LIB_STATUS_OK)
            vm_app_console_append(result, "Speed: %s\n",
                vm_app_console_speed_name(speed));
        return;
    }
    if (context->argument_count != 2u) {
        vm_app_console_append(result, "Usage: SPEED [STANDARD|TURBO]\n");
        return;
    }
    if (vm_app_console_text_equal(context->arguments[1], "standard")) speed = VM_MACHINE_SPEED_STANDARD;
    else if (vm_app_console_text_equal(context->arguments[1], "turbo")) speed = VM_MACHINE_SPEED_TURBO;
    else {
        vm_app_console_append(result, "Usage: SPEED [STANDARD|TURBO]\n");
        return;
    }
    if (vm_machine_set_speed(machine, speed) == LIB_STATUS_OK)
        vm_app_console_append(result, "Speed: %s\n", vm_app_console_speed_name(speed));
    else vm_app_console_append(result, "Cannot change speed while session is running.\n");
}

static void vm_app_console_floppy(const vm_app_console_context *context,
    common_session_machine_state state, common_session_command_result *result)
{
    vm_machine *machine = vm_app_console_machine(context);
    if (machine == LIB_NULL) return;
    if (vm_app_console_is_running(state)) {
        vm_app_console_append(result, "Cannot change floppy media now.\n");
        return;
    }
    if (context->argument_count == 3u && vm_app_console_text_equal(context->arguments[1], "insert")) {
        vm_app_console_append(result, vm_machine_insert_fdd(machine,
            (const char *)context->arguments[2]) ?
            "Cannot read floppy disk.\n" : "Floppy disk inserted.\n");
        return;
    }
    if (context->argument_count == 2u &&
        vm_app_console_text_equal(context->arguments[1], "eject")) {
        vm_app_console_append(result, vm_machine_eject_fdd(machine) ?
            "Cannot eject floppy disk.\n" : "Floppy disk ejected.\n");
        return;
    }
    vm_app_console_append(result, "Usage: FLOPPY INSERT <image> | EJECT\n");
}

static void vm_app_console_debug_result(common_session_command_result *destination,
    const x86_debug_result *source)
{
    if (destination == LIB_NULL || source == LIB_NULL) return;
    vm_app_console_append(destination, "%s", source->text);
    if (source->prompt_ready) vm_app_console_prompt(destination, source->prompt);
    if (source->lifecycle_request == X86_DEBUG_LIFECYCLE_RESUME ||
        source->lifecycle_request == X86_DEBUG_LIFECYCLE_STEP)
        destination->request = COMMON_SESSION_REQUEST_RESUME;
    if (source->lifecycle_request == X86_DEBUG_LIFECYCLE_STOP)
        destination->request = COMMON_SESSION_REQUEST_STOP;
}

static void vm_app_console_submit_debug(vm_app_console_context *context,
    const lib_u8 *line, common_session_command_result *result)
{
    x86_debug_result debug_result = {0};
    if (x86_debug_submit_line(context->debug, (const char *)line, &debug_result) != LIB_STATUS_OK) {
        vm_app_console_append(result, "Debugger command failed.\n");
        return;
    }
    vm_app_console_debug_result(result, &debug_result);
    if (!debug_result.keep_active) {
        x86_debug_close(context->debug);
        context->debug_active = LIB_FALSE;
    }
}

static void vm_app_console_submit_line(void *opaque,
    common_session_machine_state state, const char *line,
    common_session_command_result *result)
{
    vm_app_console_context *context = opaque;

    vm_app_console_clear_result(result);
    const lib_u8 *text = (const lib_u8 *)line;
    if (context == LIB_NULL || text == LIB_NULL) return;
    if (context->debug_active) {
        vm_app_console_submit_debug(context, text, result);
        return;
    }
    (void)snprintf((char *)context->command_buffer, sizeof(context->command_buffer), "%s", line);
    vm_app_console_parse(context, context->command_buffer);
    if (context->argument_count == 0u) return;
    if (vm_app_console_text_equal(context->arguments[0], "help")) vm_app_console_help(result);
    else if (vm_app_console_text_equal(context->arguments[0], "info")) vm_app_console_info(context, result);
    else if (vm_app_console_text_equal(context->arguments[0], "speed")) vm_app_console_speed(context, result);
    else if (vm_app_console_text_equal(context->arguments[0], "floppy"))
        vm_app_console_floppy(context, state, result);
    else if (vm_app_console_text_equal(context->arguments[0], "start")) result->request = COMMON_SESSION_REQUEST_START;
    else if (vm_app_console_text_equal(context->arguments[0], "reset")) result->request = COMMON_SESSION_REQUEST_RESET;
    else if (vm_app_console_text_equal(context->arguments[0], "stop")) result->request = COMMON_SESSION_REQUEST_STOP;
    else if (vm_app_console_text_equal(context->arguments[0], "resume")) result->request = COMMON_SESSION_REQUEST_RESUME;
    else if (vm_app_console_text_equal(context->arguments[0], "exit")) result->exit_requested = LIB_TRUE;
    else if (vm_app_console_text_equal(context->arguments[0], "debug")) {
        if (state == COMMON_SESSION_MACHINE_PAUSED) {
            if (x86_debug_open(context->debug, vm_app_console_common_machine(context)) ==
                LIB_STATUS_OK) {
                context->debug_active = LIB_TRUE;
                vm_app_console_prompt(result, "- ");
            } else vm_app_console_append(result, "Unable to enter debugger.\n");
        } else if (state == COMMON_SESSION_MACHINE_RUNNING) {
            context->debug_requested = LIB_TRUE;
            result->request = COMMON_SESSION_REQUEST_PAUSE;
        } else vm_app_console_append(result, "Machine must be paused to enter debugger.\n");
    } else vm_app_console_append(result, "Illegal command '%s'.\n", context->arguments[0]);
}

static void vm_app_console_open(void *opaque,
    common_session_command_result *result)
{
    (void)opaque;
    vm_app_console_clear_result(result);
    vm_app_console_append(result, "\nType HELP for help.\n\n");
}

static void vm_app_console_reject_line(void *opaque,
    common_session_command_result *result)
{
    (void)opaque;
    vm_app_console_clear_result(result);
    vm_app_console_append(result, "Invalid console input.\n");
}

static void vm_app_console_note_runtime(void *opaque,
    common_session_machine_state prior, common_session_machine_state completed,
    common_session_command_result *result)
{
    vm_app_console_context *context = opaque;
    const lib_u8 *message = LIB_NULL;
    (void)prior;
    vm_app_console_clear_result(result);
    switch (completed) {
    case COMMON_SESSION_MACHINE_RUNNING: message = "Machine started.\n"; break;
    case COMMON_SESSION_MACHINE_PAUSED: message = "Machine paused.\n"; break;
    case COMMON_SESSION_MACHINE_STOPPED: message = "Machine stopped.\n"; break;
    case COMMON_SESSION_MACHINE_RESET_COMPLETED: message = "Machine reset.\n"; break;
    case COMMON_SESSION_MACHINE_ERROR: message = "Machine faulted.\n"; break;
    default: break;
    }
    if (message != LIB_NULL) vm_app_console_append(result, "%s", message);
    if (context != LIB_NULL && context->debug_requested &&
        completed == COMMON_SESSION_MACHINE_PAUSED &&
        x86_debug_open(context->debug, vm_app_console_common_machine(context)) ==
            LIB_STATUS_OK) {
        context->debug_requested = LIB_FALSE;
        context->debug_active = LIB_TRUE;
        vm_app_console_prompt(result, "- ");
    }
}

static void vm_app_console_note_monitor_current(void *opaque,
    lib_bool current, common_session_command_result *result)
{
    vm_app_console_context *context = opaque;
    vm_app_console_clear_result(result);
    if (!current) return;
    vm_app_console_prompt(result,
        context != LIB_NULL && context->debug_active ? "- " : "Console> ");
}

static lib_bool vm_app_console_begin_external(void *opaque,
    common_session_machine_state state, common_session_request request)
{
    (void)opaque;
    (void)state;
    (void)request;
    return LIB_TRUE;
}

static lib_bool vm_app_console_hotkey(void *opaque,
    common_session_machine_state state, const lib_u8 *identifier,
    common_session_command_result *result)
{
    vm_app_console_context *context = opaque;
    if (context == LIB_NULL) {
        vm_app_console_clear_result(result);
        return LIB_FALSE;
    }
    return vm_app_keyboard_handle_hotkey(vm_app_console_machine(context), state,
        identifier, result);
}

static lib_status vm_app_console_compose(vm_app_console_context *context,
    const vm_session_request *request)
{
    common_session_options session_options = {0};
    common_ui_options ui_options = {0};

    lib_status status = vm_app_compose_machine(context->session, request);

    if (status != LIB_STATUS_OK) return status;
    if (x86_debug_create(&context->debug) != LIB_STATUS_OK)
        return LIB_STATUS_NO_MEMORY;
    session_options.display = vm_app_console_text_equal(request->display, "window") ?
        COMMON_SESSION_DISPLAY_WINDOW : COMMON_SESSION_DISPLAY_CONSOLE;
    session_options.console_control = request->console_control ? LIB_TRUE : LIB_FALSE;
    session_options.command.context = context;
    session_options.command.open = vm_app_console_open;
    session_options.command.reject_line = vm_app_console_reject_line;
    session_options.command.submit_line = vm_app_console_submit_line;
    session_options.command.begin_external = vm_app_console_begin_external;
    session_options.command.note_runtime = vm_app_console_note_runtime;
    session_options.command.note_monitor_current = vm_app_console_note_monitor_current;
    session_options.command.handle_hotkey = vm_app_console_hotkey;
    status = vm_app_compose_control(context->session, &session_options);
    if (status != LIB_STATUS_OK) return status;
    context->control = vm_app_session(context->session);
    ui_options.event_context = context->control;
    ui_options.event_sink = common_session_enqueue_ui_event;
    ui_options.running_window_title = "NXVM (Running)";
    ui_options.paused_window_title = "NXVM (Paused)";
    ui_options.graphics_console_status_text = "NXVM graphics output is active in a Window.";
    vm_app_keyboard_register_hotkeys(&ui_options.hotkeys);
    return vm_app_compose_ui(context->session, &ui_options);
}

lib_status vm_app_console_context_create(vm_app_console_context **out_context)
{
    vm_app_console_context *context;
    if (out_context == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_context = LIB_NULL;
    context = lib_allocate_zero(1u, sizeof(*context));
    if (context == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    *out_context = context;
    return LIB_STATUS_OK;
}

void vm_app_console_context_destroy(vm_app_console_context *context)
{
    if (context == LIB_NULL) return;
    x86_debug_destroy(context->debug);
    lib_release(context);
}

lib_status vm_app_console_main(vm_app_console_context *context,
    vm_app *session, const lib_u8 *ini_path)
{
    vm_session_request request;
    lib_status status;

    if (context == LIB_NULL || session == LIB_NULL || ini_path == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    context->session = session;
    status = vm_app_ini_load(ini_path, &request);
    if (status != LIB_STATUS_OK) {
        printf("Unable to load NXVM.ini.\n");
        return status;
    }
    status = vm_app_console_compose(context, &request);
    if (status != LIB_STATUS_OK) {
        printf("Unable to create session from '%s'.\n", (const char *)request.file_name);
        return status;
    }
    return common_session_run(context->control) ? LIB_STATUS_OK : LIB_STATUS_INTERNAL_ERROR;
}
