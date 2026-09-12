/* Copyright 2012-2014 Neko. */

/*
 * Console provides a command-line interface for users
 * to configure, debug and run the virtual machine.
 */

#include "type.h"

#include "vm/product/console.h"
#include "vm/app/app.h"
#include "common/ui/ui_interface.h"
#include "vm/product/catalog.h"
#include "vm/product/recorder.h"
#include "common/session/session_interface.h"

struct vm_product_console_context {
    STD_SIZE_T argument_count;
    C_CHAR **arguments;
    C_INT exit_requested;
    C_CHAR command_buffer[0x100];
    vm_app *session;
    vm_product_session_catalog *catalog;
    common_session *control;
    common_ui *presentation;
    vm_product_recorder *recorder;
    C_INT session_stopped;
};
#define CONSOLE_MAXNARG 256

#define consoleContext context

#define numArgs (consoleContext->argument_count)
#define argArray (consoleContext->arguments)
#define flagExit (consoleContext->exit_requested)
#define strCmdBuff (consoleContext->command_buffer)
#define currentSession (consoleContext->session)

static C_INT vm_product_console_printf(const vm_product_console_context *context,
    const C_CHAR *format, ...)
{
    C_CHAR text[4096];
    STD_VA_LIST arguments;
    C_INT written;

    if (context == STD_NULL || format == STD_NULL) return -1;
    va_start(arguments, format);
    written = vsnprintf(text, sizeof(text), format, arguments);
    va_end(arguments);
    if (written < 0 || (STD_SIZE_T)written >= sizeof(text)) return -1;
    return common_ui_write_console(context->presentation, text) ==
        LIB_STATUS_OK ? written : -1;
}

static lib_status vm_product_console_line_sink(void *context,
    const C_CHAR *line)
{
    return common_session_publish_console_line(context, line);
}

static lib_status vm_product_console_input_sink(void *context,
    const ui_input_event *event)
{
    return common_session_publish_ui_input(context, event);
}

static void vm_product_console_ui_failure(void *context, lib_u64 source_identity,
    lib_status status)
{
    vm_product_console_context *console = context;

    (void)source_identity;
    if (console != STD_NULL)
        (void)vm_product_console_printf(console, "UI failure: %d.\n", (C_INT)status);
}

static common_ui_target vm_product_console_ui_target(common_session_target target)
{
    return target == COMMON_SESSION_TARGET_WINDOW ? COMMON_UI_TARGET_WINDOW :
        target == COMMON_SESSION_TARGET_CONSOLE ? COMMON_UI_TARGET_CONSOLE :
        COMMON_UI_TARGET_NONE;
}

static void vm_product_console_apply_ui_plan(vm_product_console_context *context,
    const common_session_plan *plan)
{
    common_ui_plan ui_plan = {0};

    if (context == STD_NULL || context->presentation == STD_NULL || plan == STD_NULL)
        return;
    ui_plan.target_changed = plan->target_changed;
    ui_plan.target = vm_product_console_ui_target(plan->target);
    ui_plan.mouse_capturable_changed = plan->mouse_capturable_changed;
    ui_plan.mouse_capturable = plan->mouse_capturable;
    ui_plan.release_mouse = plan->release_mouse;
    ui_plan.frame_ready = plan->frame_ready;
    ui_plan.frame = plan->frame;
    (void)common_ui_apply(context->presentation, &ui_plan);
}

static C_VOID vm_product_console_apply_plan(vm_product_console_context *context,
    const common_session_plan *plan, C_INT restore_prompt)
{
    const C_CHAR *state = STD_NULL;

    if (context == STD_NULL || plan == STD_NULL) return;
    vm_product_console_apply_ui_plan(context, plan);
    switch (plan->notice) {
    case COMMON_SESSION_NOTICE_STARTED: state = "started"; break;
    case COMMON_SESSION_NOTICE_RESUMED: state = "resumed"; break;
    case COMMON_SESSION_NOTICE_PAUSED: state = "paused"; break;
    case COMMON_SESSION_NOTICE_RESET: state = "reset"; break;
    case COMMON_SESSION_NOTICE_STOPPED: state = "stopped"; break;
    default: break;
    }
    if (context->presentation != STD_NULL && state != STD_NULL &&
        (plan->notice == COMMON_SESSION_NOTICE_STARTED ||
         plan->notice == COMMON_SESSION_NOTICE_RESUMED ||
         plan->notice == COMMON_SESSION_NOTICE_PAUSED))
        (void)common_ui_set_window_title(context->presentation,
            plan->notice == COMMON_SESSION_NOTICE_PAUSED ? "NXVM (Paused)" :
            "NXVM (Running)");
    if (state != STD_NULL) (C_VOID)vm_product_console_printf(context, restore_prompt ?
        "\r\nMachine %s.\nConsole> " : "Machine %s.\n", state);
    if (plan->console_text[0] != '\0')
        (C_VOID)vm_product_console_printf(context, "%s", plan->console_text);
    if (plan->console_prompt_ready)
        (C_VOID)vm_product_console_printf(context, "%s", plan->console_prompt);
}

static C_VOID vm_product_console_drain_lifecycle(vm_product_console_context *context,
    C_INT restore_prompt)
{
    common_session_fact fact;
    ui_frame display;
    common_session_plan plan;

    if (context == STD_NULL) return;
    while (common_session_take(context->control, &fact, &display, 0u) == TYPE_STATUS_OK) {
        if (common_session_reduce_fact(context->control, &fact, &display, &plan) ==
            LIB_STATUS_OK) vm_product_console_apply_plan(context, &plan, restore_prompt);
        if (fact.kind == COMMON_SESSION_FACT_UI_INPUT &&
            vm_app_reduce_input(context->session, &fact.value.input, &plan) == TYPE_STATUS_OK)
            vm_product_console_apply_plan(context, &plan, restore_prompt);
    }
}


#define STD_PRINTF(...) vm_product_console_printf(context, __VA_ARGS__)

/*
 * Parses command-line input.
 *
 * strCmdBuff [IN]  String buffer of command-line input
 * numArgs       [OUT] Number of argArrayuments
 * argArray        [OUT] Array of argArrayuments
 */
static C_VOID parse(vm_product_console_context *context)
{
    numArgs = 0;
    argArray[numArgs] = STD_STRTOK(strCmdBuff, " \t\n\r\f");
    if (!argArray[numArgs])
    {
        return;
    }
    type_string_lower(argArray[numArgs++]);
    while (numArgs < CONSOLE_MAXNARG)
    {
        argArray[numArgs] = STD_STRTOK(STD_NULL, " \t\n\r\f");
        if (argArray[numArgs])
        {
            type_string_lower(argArray[numArgs++]);
        }
        else
        {
            break;
        }
    }
}

static C_INT vm_product_console_read_line(vm_product_console_context *context,
    C_CHAR *buffer, STD_SIZE_T buffer_size)
{
    common_session_fact fact;
    ui_frame display;
    common_session_plan plan;

    if (context == STD_NULL || buffer == STD_NULL || buffer_size == 0u ||
        context->presentation == STD_NULL || context->control == STD_NULL)
        return 0;
    buffer[0] = '\0';
    if (common_ui_request_console_line(context->presentation) != LIB_STATUS_OK) return 0;
    for (;;) {
        if (common_session_take(context->control, &fact, &display, 0xffffffffu) !=
                TYPE_STATUS_OK) return 0;
        if (fact.kind != COMMON_SESSION_FACT_CONSOLE_LINE) {
            if (common_session_reduce_fact(context->control, &fact, &display, &plan) ==
                LIB_STATUS_OK) vm_product_console_apply_plan(context, &plan, TYPE_TRUE);
            if (fact.kind == COMMON_SESSION_FACT_UI_INPUT &&
                vm_app_reduce_input(context->session, &fact.value.input, &plan) == TYPE_STATUS_OK)
                vm_product_console_apply_plan(context, &plan, TYPE_TRUE);
            continue;
        }
        if (common_session_has_cli_provider(context->control)) {
            if (common_session_reduce_fact(context->control, &fact, &display, &plan) ==
                LIB_STATUS_OK) vm_product_console_apply_plan(context, &plan, TYPE_FALSE);
            continue;
        }
        if (fact.kind != COMMON_SESSION_FACT_CONSOLE_LINE) return 0;
        if (STD_STRLEN(fact.value.line) >= buffer_size) return 0;
        STD_MEMCPY(buffer, fact.value.line, STD_STRLEN(fact.value.line) + 1u);
        return 1;
    }
}

/* Prints help commands. */
#define GetHelp do { doHelp(context); return; } while (0)
static C_VOID doHelp(vm_product_console_context *context)
{
    if (STD_STRCMP(argArray[0], "help"))
    {
        numArgs = 2;
        argArray[1] = argArray[0];
    }
    switch (numArgs)
    {
    case 2:
        if (!STD_STRCMP(argArray[1], "help"))
        {
            STD_PRINTF("Show help info\n");
            STD_PRINTF("\nHELP\n");
            STD_PRINTF("  show menu of all commands\n");
            STD_PRINTF("\nHELP <command>\n");
            STD_PRINTF("  show help info for command\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "exit"))
        {
            STD_PRINTF("Stop the machine and quit the console\n");
            STD_PRINTF("\nEXIT\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "info"))
        {
            STD_PRINTF("List virtual machine status\n");
            STD_PRINTF("\nINFO\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "session"))
        {
            STD_PRINTF("NXVM runs one startup-selected virtual machine.\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "speed"))
        {
            STD_PRINTF("Show or select machine speed\n");
            STD_PRINTF("\nSPEED [STANDARD|TURBO]\n");
            STD_PRINTF("  standard: retain L2 HLT host-load backoff\n");
            STD_PRINTF("  turbo:    reserved for Core-deadline fast-forward\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "debug"))
        {
            STD_PRINTF("Launch VM hardware debugger\n");
            STD_PRINTF("\nDEBUG\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "debug32"))
        {
            STD_PRINTF("Launch VM 32-bit hardware debugger\n");
            STD_PRINTF("\nDEBUG\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "record"))
        {
            STD_PRINTF("Record cpu status in each iteration for futher dumping\n");
            STD_PRINTF("\nRECORD start <file> | stop\n");
            STD_PRINTF("  start: open output file for record writes\n");
            STD_PRINTF("  stop:  close output file to finish recording\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "floppy"))
        {
            STD_PRINTF("Change removable floppy media while stopped\n");
            STD_PRINTF("\nFLOPPY INSERT <image> | EJECT\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "start"))
        {
            STD_PRINTF("Start virtual machine\n");
            STD_PRINTF("\nSTART\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "reset"))
        {
            STD_PRINTF("Reset virtual machine\n");
            STD_PRINTF("\nRESET\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "stop"))
        {
            STD_PRINTF("Stop virtual machine\n");
            STD_PRINTF("\nSTOP\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "resume"))
        {
            STD_PRINTF("Resume virtual machine\n");
            STD_PRINTF("\nRESUME\n");
            break;
        }
        break;
    case 1:
    default:
        STD_PRINTF("VM Console Commands\n");
        STD_PRINTF("=====================\n");
        STD_PRINTF("HELP    Show help info\n");
        STD_PRINTF("EXIT    Stop the machine and quit the console\n");
        STD_PRINTF("INFO    List all device info\n");
        STD_PRINTF("SPEED   Show or select machine speed\n");
        STD_PRINTF("\n");
        STD_PRINTF("DEBUG   Launch hardware debugger\n");
        STD_PRINTF("RECORD  Record cpu status for each instruction\n");
        STD_PRINTF("\n");
        STD_PRINTF("FLOPPY  Insert or eject removable floppy media\n");
        STD_PRINTF("\n");
        STD_PRINTF("START   Start virtual machine\n");
        STD_PRINTF("RESET   Reset virtual machine\n");
        STD_PRINTF("STOP    Stop virtual machine\n");
        STD_PRINTF("RESUME  Resume virtual machine\n");
        STD_PRINTF("\n");
        STD_PRINTF("For command usage, type 'HELP <command>'.\n");
        break;
    }
}

/* Quits product. */
static C_VOID doExit(vm_product_console_context *context)
{
    if (numArgs != 1)
    {
        GetHelp;
    }
    if (vm_app_stop(currentSession) != TYPE_STATUS_OK) {
        STD_PRINTF("Unable to stop machine.\n");
        return;
    }
    context->session_stopped = TYPE_TRUE;
    flagExit = 1;
}

/* Prints virtual machine status */
static C_VOID doInfo(vm_product_console_context *context)
{
    if (numArgs != 1)
    {
        GetHelp;
    }
    STD_PRINTF("Device Info\n");
    STD_PRINTF("================\n");
    vm_app_print_machine(currentSession);
    STD_PRINTF("\n");
    STD_PRINTF("Platform Info\n");
    STD_PRINTF("==================\n");
    switch (context->presentation == STD_NULL ? COMMON_UI_TARGET_NONE :
        common_ui_get_target(context->presentation)) {
    case COMMON_UI_TARGET_WINDOW:
        STD_PRINTF("Display Type: Window\n");
        break;
    default:
        STD_PRINTF("Display Type: Console\n");
        break;
    }
    STD_PRINTF("\n");
    STD_PRINTF("BIOS Settings\n");
    STD_PRINTF("==================\n");
    vm_app_print_bios(currentSession);
    STD_PRINTF("\n");
    STD_PRINTF("Device Status\n");
    STD_PRINTF("==================\n");
    vm_app_print_status(currentSession);
}

/* Starts internal debugger */
static C_VOID doDebug(vm_product_console_context *context)
{
    if (numArgs != 1)
    {
        GetHelp;
    }
    (C_VOID)vm_app_debug(currentSession);
}

/* Executes cpu instruction recorder */
static C_VOID doRecord(vm_product_console_context *context)
{
    if (numArgs < 2)
    {
        GetHelp;
    }
    if (vm_app_is_running(currentSession))
    {
        STD_PRINTF("Cannot change record status or dump record now.\n");
        return;
    }
    if (!STD_STRCMP(argArray[1], "start"))
    {
        if (numArgs != 3)
        {
            GetHelp;
        }
        if (vm_product_recorder_start(context->recorder, argArray[2]) ==
            TYPE_STATUS_OK) STD_PRINTF("Record started.\n");
        else STD_PRINTF("ERROR:\trecorder cannot open output file.\n");
    }
    else if (!STD_STRCMP(argArray[1], "stop"))
    {
        if (vm_product_recorder_stop(context->recorder) == TYPE_STATUS_OK)
            STD_PRINTF("Record finished.\n");
        else STD_PRINTF("ERROR:\trecorder not turned on.\n");
    }
    else
    {
        GetHelp;
    }
}

static const C_CHAR *vm_product_console_speed_name(vm_app_speed speed)
{
    return speed == VM_APP_SPEED_TURBO ? "turbo" : "standard";
}

static C_VOID doSpeed(vm_product_console_context *context)
{
    vm_app_speed speed;
    type_status status;

    if (numArgs == 1u) {
        if (vm_app_get_speed(currentSession, &speed) == TYPE_STATUS_OK) {
            STD_PRINTF("Speed: %s\n", vm_product_console_speed_name(speed));
        }
        return;
    }
    if (numArgs != 2u) { GetHelp; }
    if (!STD_STRCMP(argArray[1], "standard")) speed = VM_APP_SPEED_STANDARD;
    else if (!STD_STRCMP(argArray[1], "turbo")) speed = VM_APP_SPEED_TURBO;
    else { GetHelp; }
    status = vm_app_set_speed(currentSession, speed);
    if (status == TYPE_STATUS_OK) {
        STD_PRINTF("Speed: %s\n", vm_product_console_speed_name(speed));
    } else if (status == TYPE_STATUS_INVALID_STATE) {
        STD_PRINTF("Cannot change speed while session is running.\n");
    } else {
        STD_PRINTF("Cannot change speed.\n");
    }
}

/* Set hardware connections */
static C_VOID doFloppy(vm_product_console_context *context)
{
    if (numArgs == 3u && !STD_STRCMP(argArray[1], "insert")) {
        if (vm_app_is_running(currentSession)) {
            STD_PRINTF("Cannot change floppy media now.\n");
        } else if (vm_app_insert_fdd(currentSession, argArray[2])) {
            STD_PRINTF("Cannot read floppy disk from '%s'.\n", argArray[2]);
        } else {
            STD_PRINTF("Floppy disk inserted.\n");
        }
        return;
    }
    if (numArgs == 2u && !STD_STRCMP(argArray[1], "eject")) {
        if (vm_app_is_running(currentSession)) {
            STD_PRINTF("Cannot change floppy media now.\n");
        } else if (vm_app_remove_fdd(currentSession, STD_NULL)) {
            STD_PRINTF("Cannot eject floppy disk.\n");
        } else {
            STD_PRINTF("Floppy disk ejected.\n");
        }
        return;
    }
    STD_PRINTF("Usage: FLOPPY INSERT <image> | EJECT\n");
}

static C_INT vm_product_console_choose_profile(const vm_product_console_context *context,
    vm_session_request *out_entry)
{
    C_CHAR selection[32];
    STD_SIZE_T index;
    STD_SIZE_T count;
    C_INT choice;

    if (context == STD_NULL || out_entry == STD_NULL ||
        (count = vm_product_session_catalog_count(context->catalog)) == 0u) {
        STD_PRINTF("No session configuration files found.\n");
        return 0;
    }
    STD_PRINTF("Available session profiles:\n");
    for (index = 0u; index < count; ++index) {
        vm_session_request entry;

        if (vm_product_session_catalog_get_request(context->catalog, index, &entry) !=
            TYPE_STATUS_OK) return 0;
        STD_PRINTF("  %u  %s\n", (unsigned int)(index + 1u), entry.file_name);
    }
    STD_PRINTF("Select profile [1-%u, Enter to cancel]: ",
        (unsigned int)count);
    if (!vm_product_console_read_line((vm_product_console_context *)context,
            selection, sizeof(selection))) return 0;
    if (selection[0] == '\n' || selection[0] == '\r' || selection[0] == '\0') return 0;
    choice = STD_ATOI(selection);
    if (choice > 0 && (STD_SIZE_T)choice <= count) {
        return vm_product_session_catalog_get_request(context->catalog,
            (STD_SIZE_T)(choice - 1), out_entry) == TYPE_STATUS_OK;
    }
    STD_PRINTF("Unknown profile selection.\n");
    return 0;
}

static C_VOID vm_product_console_open_profile(vm_product_console_context *context)
{
    vm_session_request selected_entry;
    common_session_target target;

    if (context == STD_NULL || !vm_product_console_choose_profile(context,
            &selected_entry)) return;
    if (vm_app_open_profile(currentSession, &selected_entry) != TYPE_STATUS_OK) {
        STD_PRINTF("Unable to create session from '%s'.\n", selected_entry.file_name);
        return;
    }
    target = !STD_STRCMP(selected_entry.display, "window") ?
        COMMON_SESSION_TARGET_WINDOW : COMMON_SESSION_TARGET_CONSOLE;
    (C_VOID)vm_app_set_presentation_target(currentSession, target);
}

/* Executes commands */
static C_VOID execute(vm_product_console_context *context)
{
    if (!argArray[0] || !STD_STRLEN(argArray[0]))
    {
        return;
    }
    else if (!STD_STRCMP(argArray[0], "help") || !STD_STRCMP(argArray[0], "exit"))
    {
        if (!STD_STRCMP(argArray[0], "help")) doHelp(context);
        else doExit(context);
    }
    else if (!STD_STRCMP(argArray[0], "info"))
    {
        doInfo(context);
    }
    else if (!STD_STRCMP(argArray[0], "debug"))
    {
        doDebug(context);
    }
    else if (!STD_STRCMP(argArray[0], "record"))
    {
        doRecord(context);
    }
    else if (!STD_STRCMP(argArray[0], "speed"))
    {
        doSpeed(context);
    }
    else if (!STD_STRCMP(argArray[0], "floppy"))
    {
        doFloppy(context);
    }
    else if (!STD_STRCMP(argArray[0], "start"))
    {
        common_session_plan plan;
        type_status status = vm_app_start(currentSession, &plan);
        if (status == TYPE_STATUS_OK)
            vm_product_console_apply_plan(context, &plan, TYPE_FALSE);
        if (status != TYPE_STATUS_OK) {
            STD_PRINTF("START failed: %d.\n", (C_INT)status);
        }
    }
    else if (!STD_STRCMP(argArray[0], "reset"))
    {
        (C_VOID)vm_app_reset(currentSession);
    }
    else if (!STD_STRCMP(argArray[0], "stop"))
    {
        (C_VOID)vm_app_stop(currentSession);
    }
    else if (!STD_STRCMP(argArray[0], "resume"))
    {
        common_session_plan plan;
        type_status status = vm_app_resume(currentSession, &plan);
        if (status == TYPE_STATUS_OK)
            vm_product_console_apply_plan(context, &plan, TYPE_FALSE);
        if (status != TYPE_STATUS_OK) {
            STD_PRINTF("RESUME failed: %d.\n", (C_INT)status);
        }
    }
    else
    {
        STD_PRINTF("Illegal command '%s'.\n", argArray[0]);
    }
    STD_PRINTF("\n");
}

/* Initializes console */
static C_INT vm_product_console_initialize(vm_product_console_context *context,
    const C_CHAR *profile_directory)
{
    common_ui_options ui_options = {0};

    argArray = (C_CHAR **)STD_MALLOC(CONSOLE_MAXNARG * sizeof(C_CHAR *));
    if (argArray == STD_NULL) return TYPE_FALSE;
    flagExit = 0;
    context->session_stopped = TYPE_FALSE;
    context->control = vm_app_session(currentSession);
    if (context->control == STD_NULL) {
        context->control = STD_NULL;
        return TYPE_FALSE;
    }
    ui_options.input_context = context->control;
    ui_options.input_sink = vm_product_console_input_sink;
    ui_options.console_line_context = context->control;
    ui_options.console_line_sink = vm_product_console_line_sink;
    ui_options.failure_context = context;
    ui_options.failure_sink = vm_product_console_ui_failure;
    ui_hotkey_registry_initialize(&ui_options.hotkeys);
    (void)ui_hotkey_registry_register(&ui_options.hotkeys, 'P',
        UI_HOTKEY_MODIFIER_CONTROL | UI_HOTKEY_MODIFIER_ALT, "pause");
    (void)ui_hotkey_registry_register(&ui_options.hotkeys, 'D',
        UI_HOTKEY_MODIFIER_CONTROL | UI_HOTKEY_MODIFIER_ALT, "cad");
    (void)ui_hotkey_registry_register(&ui_options.hotkeys, 'F',
        UI_HOTKEY_MODIFIER_CONTROL | UI_HOTKEY_MODIFIER_ALT, "alt-enter");
    (void)ui_hotkey_registry_register(&ui_options.hotkeys, 'M',
        UI_HOTKEY_MODIFIER_CONTROL | UI_HOTKEY_MODIFIER_ALT, "release-mouse");
    ui_options.initial_window_title = "NXVM (Running)";
    if (common_ui_create(&context->presentation, &ui_options) != LIB_STATUS_OK) {
        context->control = STD_NULL;
        return TYPE_FALSE;
    }
    if (vm_product_recorder_create(&context->recorder) != TYPE_STATUS_OK) {
        common_ui_destroy(context->presentation);
        context->presentation = STD_NULL;
        context->control = STD_NULL;
        return TYPE_FALSE;
    }
    vm_app_bind_product_debug_observer(context->session,
        vm_product_recorder_observe, context->recorder);
    if (vm_product_session_catalog_create(profile_directory, &context->catalog) ==
            TYPE_STATUS_OK) return TYPE_TRUE;
    vm_product_session_catalog_destroy(context->catalog);
    context->catalog = STD_NULL;
    common_ui_destroy(context->presentation);
    context->presentation = STD_NULL;
    context->control = STD_NULL;
    vm_app_bind_product_debug_observer(context->session, STD_NULL, STD_NULL);
    vm_product_recorder_destroy(context->recorder);
    context->recorder = STD_NULL;
    STD_FREE(argArray);
    argArray = STD_NULL;
    return TYPE_FALSE;
}

/* Finalizes console */
static C_VOID vm_product_console_finalize(vm_product_console_context *context)
{
    if (argArray)
    {
        STD_FREE((C_VOID *)argArray);
    }
    argArray = STD_NULL;
    if (!context->session_stopped && vm_app_stop(currentSession) == TYPE_STATUS_OK) {
        context->session_stopped = TYPE_TRUE;
    }
    vm_product_console_drain_lifecycle(context, TYPE_FALSE);
    vm_product_session_catalog_destroy(context->catalog);
    context->catalog = STD_NULL;
    vm_app_bind_product_debug_observer(context->session, STD_NULL, STD_NULL);
    common_ui_destroy(context->presentation);
    vm_product_recorder_destroy(context->recorder);
    context->recorder = STD_NULL;
    context->presentation = STD_NULL;
    common_session_close(context->control);
    context->control = STD_NULL;
}

type_status vm_product_console_context_create(
    vm_product_console_context **out_context)
{
    vm_product_console_context *context;

    if (out_context == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_context = STD_NULL;
    context = (vm_product_console_context *)STD_CALLOC(1u, sizeof(*context));
    if (context == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    *out_context = context;
    return TYPE_STATUS_OK;
}

C_VOID vm_product_console_context_destroy(vm_product_console_context *context)
{
    if (context == STD_NULL) return;
    vm_app_bind_product_debug_observer(context->session, STD_NULL, STD_NULL);
    common_ui_destroy(context->presentation);
    vm_product_session_catalog_destroy(context->catalog);
    vm_product_recorder_destroy(context->recorder);
    STD_FREE(context);
}

C_VOID vm_product_console_main(vm_product_console_context *context,
                               vm_app *machine_session,
                               const C_CHAR *profile_directory)
{
    if (context == STD_NULL || machine_session == STD_NULL ||
        profile_directory == STD_NULL)
        return;
    context->session = machine_session;
    if (!vm_product_console_initialize(context, profile_directory)) return;
    STD_PRINTF("\nType HELP for help.\n\n");
    vm_product_console_open_profile(context);
    while (!flagExit)
    {
        STD_PRINTF("Console> ");
        if (!vm_product_console_read_line(context, strCmdBuff,
                sizeof(strCmdBuff))) break;
        parse(context);
        execute(context);
        vm_product_console_drain_lifecycle(context, TYPE_FALSE);
    }
    vm_product_console_finalize(context);
}
