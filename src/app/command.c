/* Copyright 2012-2014 Neko. */

/* The product owns its command words and their meaning.  Common owns the
 * monitor loop, lifecycle dispatch, presentation transitions and all host
 * queues; this file only supplies the injected command provider. */

#include "type.h"

#include <limits.h>

#include "x86/debug/debug_interface.h"
#include "common/session/session_interface.h"
#include "common/ui/ui_interface.h"
#include "app/composition.h"
#include "core/machine/machine_interface.h"
#include "app/ini_interface.h"
#include "app/command.h"
#include "app/keyboard.h"
#include "app/recorder.h"

#define CONSOLE_MAXNARG 256u

struct vm_app_console_context {
    C_CHAR command_buffer[0x100];
    C_CHAR *arguments[CONSOLE_MAXNARG];
    STD_SIZE_T argument_count;
    vm_app *session;
    common_session *control;
    x86_debug *debug;
    vm_app_recorder *recorder;
    C_INT debug_active;
    C_INT debug_requested;
};

static vm_machine *vm_app_console_machine(
    const vm_app_console_context *context)
{ return context == STD_NULL ? STD_NULL : vm_app_machine(context->session); }

static common_machine *vm_app_console_common_machine(
    const vm_app_console_context *context)
{
    return context == STD_NULL ? LIB_NULL : vm_app_common_machine(context->session);
}

static C_VOID vm_app_console_clear_result(common_session_command_result *result)
{
    if (result != LIB_NULL) *result = (common_session_command_result){0};
}

static C_VOID vm_app_console_append(common_session_command_result *result,
    const C_CHAR *format, ...)
{
    STD_SIZE_T used;
    C_INT written;
    STD_VA_LIST arguments;

    if (result == LIB_NULL || format == STD_NULL) return;
    used = STD_STRLEN(result->text);
    if (used >= sizeof(result->text)) return;
    va_start(arguments, format);
    written = vsnprintf(result->text + used, sizeof(result->text) - used,
        format, arguments);
    va_end(arguments);
    if (written < 0) result->text[used] = '\0';
}

static C_VOID vm_app_console_prompt(common_session_command_result *result,
    const C_CHAR *prompt)
{
    if (result == LIB_NULL || prompt == STD_NULL) return;
    (void)snprintf(result->prompt, sizeof(result->prompt), "%s", prompt);
    result->arm_prompt = LIB_TRUE;
}

static C_VOID vm_app_console_parse(vm_app_console_context *context,
    C_CHAR *line)
{
    C_CHAR *token;

    context->argument_count = 0u;
    token = STD_STRTOK(line, " \t\n\r\f");
    while (token != STD_NULL && context->argument_count < CONSOLE_MAXNARG) {
        type_string_lower(token);
        context->arguments[context->argument_count++] = token;
        token = STD_STRTOK(STD_NULL, " \t\n\r\f");
    }
}

static C_INT vm_app_console_is_running(common_session_machine_state state)
{ return state == COMMON_SESSION_MACHINE_RUNNING; }

static C_VOID vm_app_console_help(common_session_command_result *result)
{
    vm_app_console_append(result,
        "VM Console Commands\n"
        "===================\n"
        "HELP    Show help information\n"
        "INFO    List device information\n"
        "SPEED   Show or select machine speed\n"
        "DEBUG   Launch hardware debugger\n"
        "RECORD  Record cpu status for each instruction\n"
        "FLOPPY  Insert or eject removable floppy media\n"
        "START | RESET | STOP | RESUME\n"
        "EXIT    Stop the machine and quit the console\n\n");
}

static C_VOID vm_app_console_info(vm_app_console_context *context,
    common_session_command_result *result)
{
    vm_machine_information information;
    vm_machine *machine = vm_app_console_machine(context);

    if (machine == STD_NULL || vm_machine_get_information(machine, &information) !=
        TYPE_STATUS_OK) {
        vm_app_console_append(result, "Machine information unavailable.\n");
        return;
    }
    vm_app_console_append(result,
        "Device Info\n===========\nMachine:           IBM PC/AT\nProfile:           %s\n"
        "CPU:               Intel %s\nRAM Size:          %u %s\n"
        "Floppy Disk Drive: %.2f MB, %s\n",
        vm_machine_profile_name(information.profile_kind),
        core_machine_cpu_profile_name(information.cpu_profile),
        (unsigned int)(information.memory_bytes < (1u << 20) ?
            information.memory_bytes >> 10 : information.memory_bytes >> 20),
        information.memory_bytes < (1u << 20) ? "KB" : "MB",
        information.floppy_image_bytes * 1.0 / ((1 << 10) * 1000),
        information.floppy_media_inserted ? "inserted" : "not inserted");
    if (information.fixed_disk_present)
        vm_app_console_append(result, "Hard Disk Drive:   %u cylinders, %.2f MB, %s\n",
            (unsigned int)information.fixed_disk_cylinders,
            information.fixed_disk_image_bytes * 1.0 / (1 << 20),
            information.fixed_disk_media_connected ? "connected" : "disconnected");
    vm_app_console_append(result, "\nBIOS: %s\nRunning: %s\n",
        information.external_firmware ? "external ROM mapped at F0000h" :
            "profile ROM mapped", information.active ? "Yes" : "No");
}

static const C_CHAR *vm_app_console_speed_name(vm_machine_speed speed)
{ return speed == VM_MACHINE_SPEED_TURBO ? "turbo" : "standard"; }

static C_VOID vm_app_console_speed(vm_app_console_context *context,
    common_session_command_result *result)
{
    vm_machine_speed speed;
    vm_machine *machine = vm_app_console_machine(context);

    if (machine == STD_NULL) return;
    if (context->argument_count == 1u) {
        if (vm_machine_get_speed(machine, &speed) == TYPE_STATUS_OK)
            vm_app_console_append(result, "Speed: %s\n",
                vm_app_console_speed_name(speed));
        return;
    }
    if (context->argument_count != 2u) {
        vm_app_console_append(result, "Usage: SPEED [STANDARD|TURBO]\n");
        return;
    }
    if (!STD_STRCMP(context->arguments[1], "standard")) speed = VM_MACHINE_SPEED_STANDARD;
    else if (!STD_STRCMP(context->arguments[1], "turbo")) speed = VM_MACHINE_SPEED_TURBO;
    else {
        vm_app_console_append(result, "Usage: SPEED [STANDARD|TURBO]\n");
        return;
    }
    if (vm_machine_set_speed(machine, speed) == TYPE_STATUS_OK)
        vm_app_console_append(result, "Speed: %s\n", vm_app_console_speed_name(speed));
    else vm_app_console_append(result, "Cannot change speed while session is running.\n");
}

static C_VOID vm_app_console_record(vm_app_console_context *context,
    common_session_machine_state state, common_session_command_result *result)
{
    if (context->argument_count < 2u) {
        vm_app_console_append(result, "Usage: RECORD start <file> | stop\n");
        return;
    }
    if (vm_app_console_is_running(state)) {
        vm_app_console_append(result, "Cannot change record status while running.\n");
        return;
    }
    if (!STD_STRCMP(context->arguments[1], "start") && context->argument_count == 3u) {
        vm_app_console_append(result, vm_app_recorder_start(context->recorder,
            context->arguments[2]) == TYPE_STATUS_OK ? "Record started.\n" :
            "ERROR:\trecorder cannot open output file.\n");
    } else if (!STD_STRCMP(context->arguments[1], "stop") && context->argument_count == 2u) {
        vm_app_console_append(result, vm_app_recorder_stop(context->recorder) ==
            TYPE_STATUS_OK ? "Record finished.\n" : "ERROR:\trecorder not turned on.\n");
    } else vm_app_console_append(result, "Usage: RECORD start <file> | stop\n");
}

static C_VOID vm_app_console_floppy(vm_app_console_context *context,
    common_session_machine_state state, common_session_command_result *result)
{
    vm_machine *machine = vm_app_console_machine(context);
    if (machine == STD_NULL) return;
    if (vm_app_console_is_running(state)) {
        vm_app_console_append(result, "Cannot change floppy media now.\n");
        return;
    }
    if (context->argument_count == 3u && !STD_STRCMP(context->arguments[1], "insert")) {
        vm_app_console_append(result, vm_machine_insert_fdd(machine, context->arguments[2]) ?
            "Cannot read floppy disk.\n" : "Floppy disk inserted.\n");
        return;
    }
    if (context->argument_count == 2u && !STD_STRCMP(context->arguments[1], "eject")) {
        vm_app_console_append(result, vm_machine_remove_fdd(machine, STD_NULL) ?
            "Cannot eject floppy disk.\n" : "Floppy disk ejected.\n");
        return;
    }
    vm_app_console_append(result, "Usage: FLOPPY INSERT <image> | EJECT\n");
}

static C_VOID vm_app_console_debug_result(common_session_command_result *destination,
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

static C_VOID vm_app_console_submit_debug(vm_app_console_context *context,
    const C_CHAR *line, common_session_command_result *result)
{
    x86_debug_result debug_result = {0};
    if (x86_debug_submit_line(context->debug, line, &debug_result) != LIB_STATUS_OK) {
        vm_app_console_append(result, "Debugger command failed.\n");
        return;
    }
    vm_app_console_debug_result(result, &debug_result);
    if (!debug_result.keep_active) {
        x86_debug_close(context->debug);
        context->debug_active = TYPE_FALSE;
    }
}

static C_VOID vm_app_console_submit_line(C_VOID *opaque,
    common_session_machine_state state, const C_CHAR *line,
    common_session_command_result *result)
{
    vm_app_console_context *context = opaque;

    vm_app_console_clear_result(result);
    if (context == STD_NULL || line == STD_NULL) return;
    if (context->debug_active) {
        vm_app_console_submit_debug(context, line, result);
        return;
    }
    (void)snprintf(context->command_buffer, sizeof(context->command_buffer), "%s", line);
    vm_app_console_parse(context, context->command_buffer);
    if (context->argument_count == 0u) return;
    if (!STD_STRCMP(context->arguments[0], "help")) vm_app_console_help(result);
    else if (!STD_STRCMP(context->arguments[0], "info")) vm_app_console_info(context, result);
    else if (!STD_STRCMP(context->arguments[0], "speed")) vm_app_console_speed(context, result);
    else if (!STD_STRCMP(context->arguments[0], "record"))
        vm_app_console_record(context, state, result);
    else if (!STD_STRCMP(context->arguments[0], "floppy"))
        vm_app_console_floppy(context, state, result);
    else if (!STD_STRCMP(context->arguments[0], "start")) result->request = COMMON_SESSION_REQUEST_START;
    else if (!STD_STRCMP(context->arguments[0], "reset")) result->request = COMMON_SESSION_REQUEST_RESET;
    else if (!STD_STRCMP(context->arguments[0], "stop")) result->request = COMMON_SESSION_REQUEST_STOP;
    else if (!STD_STRCMP(context->arguments[0], "resume")) result->request = COMMON_SESSION_REQUEST_RESUME;
    else if (!STD_STRCMP(context->arguments[0], "exit")) result->exit_requested = LIB_TRUE;
    else if (!STD_STRCMP(context->arguments[0], "debug")) {
        if (state == COMMON_SESSION_MACHINE_PAUSED) {
            if (x86_debug_open(context->debug, vm_app_console_common_machine(context)) ==
                LIB_STATUS_OK) {
                context->debug_active = TYPE_TRUE;
                vm_app_console_prompt(result, "- ");
            } else vm_app_console_append(result, "Unable to enter debugger.\n");
        } else if (state == COMMON_SESSION_MACHINE_RUNNING) {
            context->debug_requested = TYPE_TRUE;
            result->request = COMMON_SESSION_REQUEST_PAUSE;
        } else vm_app_console_append(result, "Machine must be paused to enter debugger.\n");
    } else vm_app_console_append(result, "Illegal command '%s'.\n", context->arguments[0]);
}

static C_VOID vm_app_console_open(C_VOID *opaque,
    common_session_command_result *result)
{
    (C_VOID)opaque;
    vm_app_console_clear_result(result);
    vm_app_console_append(result, "\nType HELP for help.\n\n");
}

static C_VOID vm_app_console_reject_line(C_VOID *opaque,
    common_session_command_result *result)
{
    (C_VOID)opaque;
    vm_app_console_clear_result(result);
    vm_app_console_append(result, "Invalid console input.\n");
}

static C_VOID vm_app_console_note_runtime(C_VOID *opaque,
    common_session_machine_state prior, common_session_machine_state completed,
    common_session_command_result *result)
{
    vm_app_console_context *context = opaque;
    const C_CHAR *message = STD_NULL;
    (C_VOID)prior;
    vm_app_console_clear_result(result);
    switch (completed) {
    case COMMON_SESSION_MACHINE_RUNNING: message = "Machine started.\n"; break;
    case COMMON_SESSION_MACHINE_PAUSED: message = "Machine paused.\n"; break;
    case COMMON_SESSION_MACHINE_STOPPED: message = "Machine stopped.\n"; break;
    case COMMON_SESSION_MACHINE_RESET_COMPLETED: message = "Machine reset.\n"; break;
    case COMMON_SESSION_MACHINE_ERROR: message = "Machine faulted.\n"; break;
    default: break;
    }
    if (message != STD_NULL) vm_app_console_append(result, "%s", message);
    if (context != STD_NULL && context->debug_requested &&
        completed == COMMON_SESSION_MACHINE_PAUSED &&
        x86_debug_open(context->debug, vm_app_console_common_machine(context)) ==
            LIB_STATUS_OK) {
        context->debug_requested = TYPE_FALSE;
        context->debug_active = TYPE_TRUE;
        vm_app_console_prompt(result, "- ");
    }
}

static C_VOID vm_app_console_note_monitor_current(C_VOID *opaque,
    lib_bool current, common_session_command_result *result)
{
    vm_app_console_context *context = opaque;
    vm_app_console_clear_result(result);
    if (!current) return;
    vm_app_console_prompt(result,
        context != STD_NULL && context->debug_active ? "- " : "Console> ");
}

static lib_bool vm_app_console_begin_external(C_VOID *opaque,
    common_session_machine_state state, common_session_request request)
{
    (C_VOID)opaque;
    (C_VOID)state;
    (C_VOID)request;
    return LIB_TRUE;
}

static lib_bool vm_app_console_hotkey(C_VOID *opaque,
    common_session_machine_state state, const C_CHAR *identifier,
    common_session_command_result *result)
{
    vm_app_console_context *context = opaque;
    if (context == STD_NULL) {
        vm_app_console_clear_result(result);
        return LIB_FALSE;
    }
    return vm_app_keyboard_handle_hotkey(vm_app_console_machine(context), state,
        identifier, result);
}

static C_INT vm_app_console_compose(vm_app_console_context *context,
    const vm_session_request *request)
{
    common_session_options session_options = {0};
    common_ui_options ui_options = {0};

    if (vm_app_compose_machine(context->session, request) != TYPE_STATUS_OK) return 0;
    if (vm_app_recorder_create(&context->recorder) != TYPE_STATUS_OK ||
        x86_debug_create(&context->debug) != LIB_STATUS_OK) return 0;
    session_options.display = !STD_STRCMP(request->display, "window") ?
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
    if (vm_app_compose_control(context->session, &session_options) != TYPE_STATUS_OK) return 0;
    context->control = vm_app_session(context->session);
    ui_options.event_context = context->control;
    ui_options.event_sink = common_session_enqueue_ui_event;
    ui_options.running_window_title = "NXVM (Running)";
    ui_options.paused_window_title = "NXVM (Paused)";
    ui_options.graphics_console_status_text = "NXVM graphics output is active in a Window.";
    vm_app_keyboard_register_hotkeys(&ui_options.hotkeys);
    return vm_app_compose_ui(context->session, &ui_options) == TYPE_STATUS_OK;
}

type_status vm_app_console_context_create(vm_app_console_context **out_context)
{
    vm_app_console_context *context;
    if (out_context == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_context = STD_NULL;
    context = STD_CALLOC(1u, sizeof(*context));
    if (context == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    *out_context = context;
    return TYPE_STATUS_OK;
}

C_VOID vm_app_console_context_destroy(vm_app_console_context *context)
{
    if (context == STD_NULL) return;
    x86_debug_destroy(context->debug);
    vm_app_recorder_destroy(context->recorder);
    STD_FREE(context);
}

C_VOID vm_app_console_main(vm_app_console_context *context,
    vm_app *session, const C_CHAR *ini_path)
{
    vm_session_request request;
    if (context == STD_NULL || session == STD_NULL || ini_path == STD_NULL) return;
    context->session = session;
    if (vm_app_ini_load(ini_path, &request) != TYPE_STATUS_OK) {
        STD_PRINTF("Unable to load NXVM.ini.\n");
        return;
    }
    if (!vm_app_console_compose(context, &request)) {
        STD_PRINTF("Unable to create session from '%s'.\n", request.file_name);
        return;
    }
    (C_VOID)common_session_run(context->control);
}
