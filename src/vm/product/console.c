/* Copyright 2012-2014 Neko. */

/*
 * Console provides a command-line interface for users
 * to configure, debug and run the virtual machine.
 */

#include "type.h"

#include "vm/product/console.h"
#include "vm/product/console_host.h"
#include "lib/host/sync_interface.h"

#define VM_PRODUCT_CONSOLE_EVENT_CAPACITY 64u

typedef enum vm_product_console_event_kind {
    VM_PRODUCT_CONSOLE_EVENT_LINE,
    VM_PRODUCT_CONSOLE_EVENT_LIFECYCLE
} vm_product_console_event_kind;

typedef struct vm_product_console_event {
    vm_product_console_event_kind kind;
    core_product_session_id id;
    vm_session_lifecycle lifecycle;
    C_CHAR line[0x100];
} vm_product_console_event;

struct vm_product_console_context {
    STD_SIZE_T argument_count;
    C_CHAR **arguments;
    C_INT exit_requested;
    C_CHAR command_buffer[0x100];
    const vm_session_machine_provider *machine_provider;
    core_product_session_manager *session_manager;
    vm_product_session_catalog *catalog;
    vm_product_console_host *console_host;
    host_sync_event *event_ready;
    STD_ATOMIC_FLAG lifecycle_lock;
    vm_product_console_event events[VM_PRODUCT_CONSOLE_EVENT_CAPACITY];
    STD_SIZE_T event_first;
    STD_SIZE_T event_count;
    C_INT accepting_events;
    C_INT event_delivery_failed;
    C_INT sessions_stopped;
};
#include "core/product/session/command_interface.h"

#define CONSOLE_MAXNARG 256

#define consoleContext context

#define numArgs (consoleContext->argument_count)
#define argArray (consoleContext->arguments)
#define flagExit (consoleContext->exit_requested)
#define strCmdBuff (consoleContext->command_buffer)
#define machineProvider (consoleContext->machine_provider)
#define sessionManager (consoleContext->session_manager)

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
    return vm_product_console_host_write((vm_product_console_host *)
        context->console_host, text) ==
        TYPE_STATUS_OK ? written : -1;
}

static C_VOID vm_product_console_lifecycle_lock(
    vm_product_console_context *context)
{
    while (STD_ATOMIC_FLAG_TEST_AND_SET_EXPLICIT(&context->lifecycle_lock,
        STD_MEMORY_ORDER_ACQUIRE)) {
    }
}

static C_VOID vm_product_console_lifecycle_unlock(
    vm_product_console_context *context)
{
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&context->lifecycle_lock,
        STD_MEMORY_ORDER_RELEASE);
}

static C_VOID vm_product_console_push(vm_product_console_context *context,
    const vm_product_console_event *event)
{
    STD_SIZE_T index;

    if (context == STD_NULL || event == STD_NULL || context->event_ready == STD_NULL) return;
    vm_product_console_lifecycle_lock(context);
    if (context->accepting_events) {
        if (context->event_count == VM_PRODUCT_CONSOLE_EVENT_CAPACITY) {
            context->event_delivery_failed = TYPE_TRUE;
        } else {
            index = (context->event_first + context->event_count) %
                VM_PRODUCT_CONSOLE_EVENT_CAPACITY;
            context->events[index] = *event;
            ++context->event_count;
        }
        host_sync_event_signal(context->event_ready);
    }
    vm_product_console_lifecycle_unlock(context);
}

static C_VOID vm_product_console_report_lifecycle(C_VOID *opaque,
    core_product_session_id id, vm_session_lifecycle lifecycle)
{
    vm_product_console_event event = {0};

    event.kind = VM_PRODUCT_CONSOLE_EVENT_LIFECYCLE;
    event.id = id;
    event.lifecycle = lifecycle;
    vm_product_console_push((vm_product_console_context *)opaque, &event);
}

static C_INT vm_product_console_take(vm_product_console_context *context,
    vm_product_console_event *out_event)
{
    C_INT result = 0;

    if (context == STD_NULL || out_event == STD_NULL) return -1;
    vm_product_console_lifecycle_lock(context);
    if (context->event_delivery_failed) {
        result = -1;
    } else if (context->event_count != 0u) {
        *out_event = context->events[context->event_first];
        context->event_first = (context->event_first + 1u) % VM_PRODUCT_CONSOLE_EVENT_CAPACITY;
        --context->event_count;
        result = 1;
    }
    if (context->event_count == 0u && !context->event_delivery_failed) {
        host_sync_event_reset(context->event_ready);
    }
    vm_product_console_lifecycle_unlock(context);
    return result;
}

static C_VOID vm_product_console_write_lifecycle(vm_product_console_context *context,
    const vm_product_console_event *event, C_INT restore_prompt)
{
    const C_CHAR *state = event->lifecycle == VM_SESSION_RUNNING ? "running" :
        event->lifecycle == VM_SESSION_PAUSED ? "paused" : "stopped";
    (C_VOID)vm_product_console_printf(context, restore_prompt ?
        "\r\nSession %u %s.\nConsole> " : "Session %u %s.\n",
        (unsigned int)event->id, state);
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

static C_VOID vm_product_console_line_received(C_VOID *opaque,
    const C_CHAR *text)
{
    vm_product_console_context *context = opaque;
    vm_product_console_event event = {0};
    STD_SIZE_T length;

    if (context == STD_NULL || text == STD_NULL) return;
    length = STD_STRLEN(text);
    if (length >= sizeof(event.line)) length = sizeof(event.line) - 1u;
    event.kind = VM_PRODUCT_CONSOLE_EVENT_LINE;
    STD_MEMCPY(event.line, text, length);
    event.line[length] = '\0';
    vm_product_console_push(context, &event);
}

static C_INT vm_product_console_read_line(vm_product_console_context *context,
    C_CHAR *buffer, STD_SIZE_T buffer_size)
{
    vm_product_console_event event;

    if (context == STD_NULL || buffer == STD_NULL || buffer_size == 0u ||
        context->console_host == STD_NULL || context->event_ready == STD_NULL)
        return 0;
    buffer[0] = '\0';
    if (vm_product_console_host_request_line(context->console_host) !=
        TYPE_STATUS_OK) return 0;
    for (;;) {
        if (host_sync_event_wait(context->event_ready, 0xffffffffu) !=
                HOST_SYNC_WAIT_SIGNALED) return 0;
        if (vm_product_console_take(context, &event) < 0) return 0;
        if (event.kind == VM_PRODUCT_CONSOLE_EVENT_LIFECYCLE) {
            vm_product_console_write_lifecycle(context, &event, TYPE_TRUE);
            continue;
        }
        if (event.kind != VM_PRODUCT_CONSOLE_EVENT_LINE) return 0;
        if (STD_STRLEN(event.line) >= buffer_size) return 0;
        STD_MEMCPY(buffer, event.line, STD_STRLEN(event.line) + 1u);
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
            STD_PRINTF("Stop all sessions and quit the console\n");
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
            STD_PRINTF("Manage VM sessions\n");
            STD_PRINTF("\nSESSION LIST | OPEN | SELECT <id> | CLOSE [id]\n");
            STD_PRINTF("  list:   show sessions; * marks the selected session\n");
            STD_PRINTF("  open:   show the startup-frozen YAML profile file names\n");
            STD_PRINTF("  select: choose the session for machine commands\n");
            STD_PRINTF("  close:  destroy one stopped session; the final session stays\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "speed"))
        {
            STD_PRINTF("Show or select the selected session speed\n");
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
        STD_PRINTF("EXIT    Stop all sessions and quit the console\n");
        STD_PRINTF("INFO    List all device info\n");
        STD_PRINTF("SESSION Manage sessions\n");
        STD_PRINTF("SPEED   Show or select selected-session speed\n");
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
    if (machineProvider->stop_all == STD_NULL ||
        machineProvider->stop_all(machineProvider->context) != TYPE_STATUS_OK) {
        STD_PRINTF("Unable to stop all sessions.\n");
        return;
    }
    context->sessions_stopped = TYPE_TRUE;
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
    machineProvider->print_machine(machineProvider->context);
    STD_PRINTF("\n");
    STD_PRINTF("Platform Info\n");
    STD_PRINTF("==================\n");
    switch (machineProvider->get_display_mode(machineProvider->context)) {
    case VM_SESSION_DISPLAY_WINDOW:
        STD_PRINTF("Display Type: Window\n");
        break;
    default:
        STD_PRINTF("Display Type: Console\n");
        break;
    }
    STD_PRINTF("\n");
    STD_PRINTF("BIOS Settings\n");
    STD_PRINTF("==================\n");
    machineProvider->print_bios(machineProvider->context);
    STD_PRINTF("\n");
    STD_PRINTF("Device Status\n");
    STD_PRINTF("==================\n");
    machineProvider->print_status(machineProvider->context);
}

/* Starts internal debugger */
static C_VOID doDebug(vm_product_console_context *context)
{
    if (numArgs != 1)
    {
        GetHelp;
    }
    machineProvider->debug(machineProvider->context);
}

/* Executes cpu instruction recorder */
static C_VOID doRecord(vm_product_console_context *context)
{
    if (numArgs < 2)
    {
        GetHelp;
    }
    if (machineProvider->is_running(machineProvider->context))
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
        machineProvider->record_start(machineProvider->context, argArray[2]);
    }
    else if (!STD_STRCMP(argArray[1], "stop"))
    {
        machineProvider->record_stop(machineProvider->context);
    }
    else
    {
        GetHelp;
    }
}

static const C_CHAR *vm_product_console_speed_name(vm_product_console_speed speed)
{
    return speed == VM_PRODUCT_CONSOLE_SPEED_TURBO ? "turbo" : "standard";
}

static C_VOID doSpeed(vm_product_console_context *context)
{
    vm_product_console_speed speed;
    type_status status;

    if (machineProvider->get_speed == STD_NULL || machineProvider->set_speed == STD_NULL) {
        STD_PRINTF("Speed control is unavailable.\n");
        return;
    }
    if (numArgs == 1u) {
        if (machineProvider->get_speed(machineProvider->context, &speed) == TYPE_STATUS_OK) {
            STD_PRINTF("Speed: %s\n", vm_product_console_speed_name(speed));
        }
        return;
    }
    if (numArgs != 2u) { GetHelp; }
    if (!STD_STRCMP(argArray[1], "standard")) speed = VM_PRODUCT_CONSOLE_SPEED_STANDARD;
    else if (!STD_STRCMP(argArray[1], "turbo")) speed = VM_PRODUCT_CONSOLE_SPEED_TURBO;
    else { GetHelp; }
    status = machineProvider->set_speed(machineProvider->context, speed);
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
        if (machineProvider->is_running(machineProvider->context)) {
            STD_PRINTF("Cannot change floppy media now.\n");
        } else if (machineProvider->insert_fdd(machineProvider->context, argArray[2])) {
            STD_PRINTF("Cannot read floppy disk from '%s'.\n", argArray[2]);
        } else {
            STD_PRINTF("Floppy disk inserted.\n");
        }
        return;
    }
    if (numArgs == 2u && !STD_STRCMP(argArray[1], "eject")) {
        if (machineProvider->is_running(machineProvider->context)) {
            STD_PRINTF("Cannot change floppy media now.\n");
        } else if (machineProvider->remove_fdd(machineProvider->context, STD_NULL)) {
            STD_PRINTF("Cannot eject floppy disk.\n");
        } else {
            STD_PRINTF("Floppy disk ejected.\n");
        }
        return;
    }
    STD_PRINTF("Usage: FLOPPY INSERT <image> | EJECT\n");
}

static C_INT vm_product_console_choose_profile(const vm_product_console_context *context,
    vm_product_session_request *out_entry)
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
        vm_product_session_request entry;

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

static C_VOID vm_product_console_write_line(C_VOID *context, const C_CHAR *line)
{
    (C_VOID)context;
    STD_PRINTF("%s\n", line);
}

static C_VOID vm_product_console_open_profile(vm_product_console_context *context)
{
    vm_product_session_request selected_entry;
    const vm_product_session_request *entry = &selected_entry;
    const core_product_session_open_options options = {
        0, STD_NULL, entry, sizeof(*entry)
    };
    const C_CHAR *arguments[] = { "session", "open" };
    core_product_session_output_provider output;

    if (context == STD_NULL || !vm_product_console_choose_profile(context,
            &selected_entry)) return;
    output.write_line = vm_product_console_write_line;
    output.context = STD_NULL;
    if (!core_product_session_command_execute(context->session_manager, 2,
            arguments, &options, &output)) return;
    machineProvider->set_console_binding(machineProvider->context,
        vm_product_console_host_binding(context->console_host));
    if (machineProvider->set_lifecycle_reporter != STD_NULL) {
        machineProvider->set_lifecycle_reporter(machineProvider->context,
            vm_product_console_report_lifecycle, context);
    }
    machineProvider->set_display_mode(machineProvider->context,
        !STD_STRCMP(entry->display, "window") ? VM_SESSION_DISPLAY_WINDOW :
        VM_SESSION_DISPLAY_CONSOLE);
}

static C_VOID vm_product_console_session(vm_product_console_context *context)
{
    const C_CHAR *arguments[CONSOLE_MAXNARG];
    core_product_session_output_provider output;
    STD_SIZE_T index;

    if (context == STD_NULL || numArgs < 2u) { GetHelp; return; }
    if (!STD_STRCMP(argArray[1], "open") && numArgs == 2u) {
        vm_product_console_open_profile(context);
        return;
    }
    for (index = 0u; index < numArgs; ++index) arguments[index] = argArray[index];
    output.write_line = vm_product_console_write_line;
    output.context = STD_NULL;
    (C_VOID)core_product_session_command_execute(context->session_manager,
        numArgs, arguments, STD_NULL, &output);
    if (!STD_STRCMP(argArray[1], "open")) {
        machineProvider->set_console_binding(machineProvider->context,
            vm_product_console_host_binding(context->console_host));
        if (machineProvider->set_lifecycle_reporter != STD_NULL) {
            machineProvider->set_lifecycle_reporter(machineProvider->context,
                vm_product_console_report_lifecycle, context);
        }
    }
}

/* Executes commands */
static C_VOID execute(vm_product_console_context *context)
{
    core_product_session_id selected;
    if (!argArray[0] || !STD_STRLEN(argArray[0]))
    {
        return;
    }
    else if (!STD_STRCMP(argArray[0], "session"))
    {
        vm_product_console_session(context);
    }
    else if (!STD_STRCMP(argArray[0], "help") || !STD_STRCMP(argArray[0], "exit"))
    {
        if (!STD_STRCMP(argArray[0], "help")) doHelp(context);
        else doExit(context);
    }
    else if (core_product_session_manager_get_selected_id(sessionManager, &selected) !=
        TYPE_STATUS_OK)
    {
        STD_PRINTF("No session selected. Use SESSION OPEN.\n");
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
        type_status status = machineProvider->start(machineProvider->context);
        if (status != TYPE_STATUS_OK) {
            STD_PRINTF("START failed: %d.\n", (C_INT)status);
        }
    }
    else if (!STD_STRCMP(argArray[0], "reset"))
    {
        machineProvider->reset(machineProvider->context);
    }
    else if (!STD_STRCMP(argArray[0], "stop"))
    {
        machineProvider->stop(machineProvider->context);
    }
    else if (!STD_STRCMP(argArray[0], "resume"))
    {
        type_status status = machineProvider->resume(machineProvider->context);
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
    argArray = (C_CHAR **)STD_MALLOC(CONSOLE_MAXNARG * sizeof(C_CHAR *));
    if (argArray == STD_NULL) return TYPE_FALSE;
    flagExit = 0;
    context->lifecycle_lock = (STD_ATOMIC_FLAG)ATOMIC_FLAG_INIT;
    STD_ATOMIC_FLAG_CLEAR_EXPLICIT(&context->lifecycle_lock,
        STD_MEMORY_ORDER_RELEASE);
    context->event_first = 0u;
    context->event_count = 0u;
    context->accepting_events = TYPE_TRUE;
    context->event_delivery_failed = TYPE_FALSE;
    context->sessions_stopped = TYPE_FALSE;
    if (host_sync_event_create(&context->event_ready) != LIB_STATUS_OK ||
        vm_product_console_host_create(&context->console_host, context,
            vm_product_console_line_received) != TYPE_STATUS_OK) {
        host_sync_event_destroy(context->event_ready);
        context->event_ready = STD_NULL;
        return TYPE_FALSE;
    }
    if (vm_product_session_catalog_create(profile_directory, &context->catalog) ==
            TYPE_STATUS_OK) return TYPE_TRUE;
    vm_product_session_catalog_destroy(context->catalog);
    context->catalog = STD_NULL;
    vm_product_console_host_destroy(context->console_host);
    host_sync_event_destroy(context->event_ready);
    context->console_host = STD_NULL;
    context->event_ready = STD_NULL;
    STD_FREE(argArray);
    argArray = STD_NULL;
    return TYPE_FALSE;
}

/* Finalizes console */
static C_VOID vm_product_console_finalize(vm_product_console_context *context)
{
    vm_product_console_event event;
    if (argArray)
    {
        STD_FREE((C_VOID *)argArray);
    }
    argArray = STD_NULL;
    if (!context->sessions_stopped && machineProvider != STD_NULL &&
        machineProvider->stop_all != STD_NULL &&
        machineProvider->stop_all(machineProvider->context) == TYPE_STATUS_OK) {
        context->sessions_stopped = TYPE_TRUE;
    }
    while (vm_product_console_take(context, &event) > 0) {
        if (event.kind == VM_PRODUCT_CONSOLE_EVENT_LIFECYCLE)
            vm_product_console_write_lifecycle(context, &event, TYPE_FALSE);
    }
    if (machineProvider != STD_NULL && machineProvider->set_lifecycle_reporter != STD_NULL) {
        machineProvider->set_lifecycle_reporter(machineProvider->context,
            STD_NULL, STD_NULL);
    }
    vm_product_console_lifecycle_lock(context);
    context->accepting_events = TYPE_FALSE;
    vm_product_console_lifecycle_unlock(context);
    vm_product_session_catalog_destroy(context->catalog);
    context->catalog = STD_NULL;
    vm_product_console_host_destroy(context->console_host);
    host_sync_event_destroy(context->event_ready);
    context->console_host = STD_NULL;
    context->event_ready = STD_NULL;
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
    vm_product_session_catalog_destroy(context->catalog);
    STD_FREE(context);
}

C_VOID vm_product_console_main(vm_product_console_context *context,
                               const vm_session_machine_provider *machine_provider,
                               core_product_session_manager *session_manager,
                               const C_CHAR *profile_directory)
{
    if (context == STD_NULL || machine_provider == STD_NULL ||
        session_manager == STD_NULL)
        return;
    machineProvider = machine_provider;
    sessionManager = session_manager;
    if (!vm_product_console_initialize(context, profile_directory)) return;
    if (machineProvider->set_lifecycle_reporter != STD_NULL) {
        machineProvider->set_lifecycle_reporter(machineProvider->context,
            vm_product_console_report_lifecycle, context);
    }
    STD_PRINTF("\nType HELP for help.\n\n");
    vm_product_console_open_profile(context);
    while (!flagExit)
    {
        STD_PRINTF("Console> ");
        if (!vm_product_console_read_line(context, strCmdBuff,
                sizeof(strCmdBuff))) break;
        parse(context);
        execute(context);
    }
    vm_product_console_finalize(context);
}
