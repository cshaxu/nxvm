/* Copyright 2012-2014 Neko. */

/*
 * Console provides a command-line interface for users
 * to configure, debug and run the virtual machine.
 */

#include "type.h"

#include "core/product/session/command_interface.h"

#include "vm/product/console.h"

#include "core/product/session/command_interface.h"

#define CONSOLE_MAXNARG 256

#define consoleContext context

#define numArgs (consoleContext->argument_count)
#define argArray (consoleContext->arguments)
#define flagExit (consoleContext->exit_requested)
#define strCmdBuff (consoleContext->command_buffer)
#define machineProvider (consoleContext->machine_provider)
#define sessionManager (consoleContext->session_manager)

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

static C_INT vm_product_console_read_line(C_CHAR *buffer, STD_SIZE_T buffer_size)
{
    return buffer != STD_NULL && buffer_size != 0u && buffer_size <= 0x7fffffffu &&
        STD_FGETS(buffer, (C_INT)buffer_size, STD_STDIN) != STD_NULL;
}

/* Prints help commands. */
#define GetHelp          \
    if (1)               \
    {                    \
        doHelp(context); \
        return;          \
    }                    \
    else
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
            STD_PRINTF("Quit the console\n");
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
    case 1:
    default:
        STD_PRINTF("VM Console Commands\n");
        STD_PRINTF("=====================\n");
        STD_PRINTF("HELP    Show help info\n");
        STD_PRINTF("EXIT    Quit the console\n");
        STD_PRINTF("INFO    List all device info\n");
        STD_PRINTF("SESSION Manage sessions\n");
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
    core_product_session_snapshot *snapshots;
    STD_SIZE_T count;
    STD_SIZE_T index;

    if (numArgs != 1)
    {
        GetHelp;
    }
    if (core_product_session_manager_get_count(sessionManager, &count) !=
        TYPE_STATUS_OK)
        return;
    snapshots = (core_product_session_snapshot *)STD_CALLOC(count, sizeof(*snapshots));
    if (snapshots == STD_NULL || core_product_session_manager_list(sessionManager,
                                                                   snapshots, count, &count) != TYPE_STATUS_OK)
    {
        STD_FREE(snapshots);
        return;
    }
    for (index = 0u; index < count; ++index)
    {
        if (snapshots[index].state == CORE_PRODUCT_SESSION_STATE_RUNNING ||
            snapshots[index].state == CORE_PRODUCT_SESSION_STATE_PAUSED)
        {
            STD_PRINTF("Please stop session %u before exit.\n",
                       (unsigned int)snapshots[index].id);
            STD_FREE(snapshots);
            return;
        }
    }
    STD_FREE(snapshots);
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
    case VM_PRODUCT_CONSOLE_DISPLAY_WINDOW:
        STD_PRINTF("Display Type: Window\n");
        break;
    case VM_PRODUCT_CONSOLE_DISPLAY_AUTO:
        STD_PRINTF("Display Type: Auto\n");
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

static const vm_product_session_catalog_entry *vm_product_console_choose_profile(
    const vm_product_console_context *context)
{
    C_CHAR selection[32];
    STD_SIZE_T index;
    C_INT choice;

    if (context == STD_NULL || context->catalog.count == 0u) {
        STD_PRINTF("No session configuration files found.\n");
        return STD_NULL;
    }
    STD_PRINTF("Available session profiles:\n");
    for (index = 0u; index < context->catalog.count; ++index) {
        STD_PRINTF("  %u  %s\n", (unsigned int)(index + 1u),
            context->catalog.entries[index].file_name);
    }
    STD_PRINTF("Select profile [1-%u, Enter to cancel]: ",
        (unsigned int)context->catalog.count);
    if (!vm_product_console_read_line(selection, sizeof(selection))) return STD_NULL;
    if (selection[0] == '\n' || selection[0] == '\r' || selection[0] == '\0') return STD_NULL;
    choice = STD_ATOI(selection);
    if (choice > 0 && (STD_SIZE_T)choice <= context->catalog.count) {
        return vm_product_session_catalog_get(&context->catalog,
            (STD_SIZE_T)(choice - 1));
    }
    STD_PRINTF("Unknown profile selection.\n");
    return STD_NULL;
}

static C_VOID vm_product_console_write_line(C_VOID *context, const C_CHAR *line)
{
    (C_VOID)context;
    STD_PRINTF("%s\n", line);
}

static C_VOID vm_product_console_open_profile(vm_product_console_context *context)
{
    const vm_product_session_catalog_entry *entry = vm_product_console_choose_profile(context);
    C_CHAR option[] = "--profile";
    C_CHAR option_fdd[] = "--fdd";
    C_CHAR option_hdd[] = "--hdd";
    C_CHAR option_boot[] = "--boot";
    C_CHAR option_display[] = "--display";
    C_CHAR option_cpu[] = "--cpu";
    C_CHAR option_fpu[] = "--fpu";
    C_CHAR option_memory[] = "--memory-kib";
    C_CHAR option_even_path[] = "--model40-rom-even-path";
    C_CHAR option_even_sha256[] = "--model40-rom-even-sha256";
    C_CHAR option_odd_path[] = "--model40-rom-odd-path";
    C_CHAR option_odd_sha256[] = "--model40-rom-odd-sha256";
    C_CHAR option_provenance[] = "--model40-provenance";
    C_CHAR memory[32];
    const C_CHAR *arguments[] = { option, STD_NULL,
        option_fdd, STD_NULL, option_hdd, STD_NULL, option_boot, STD_NULL,
        option_display, STD_NULL, STD_NULL, STD_NULL, STD_NULL, STD_NULL,
        STD_NULL, STD_NULL, option_even_path, STD_NULL, option_even_sha256, STD_NULL,
        option_odd_path, STD_NULL, option_odd_sha256, STD_NULL, option_provenance, STD_NULL };
    const C_CHAR *command_arguments[28] = { "session", "open" };
    core_product_session_output_provider output;
    C_INT argument_count = 10;

    if (entry == STD_NULL || context == STD_NULL) return;
    arguments[1] = entry->profile;
    arguments[3] = entry->floppy[0] ? entry->floppy : "null";
    arguments[5] = entry->hard_disk[0] ? entry->hard_disk : "null";
    arguments[7] = entry->boot;
    arguments[9] = entry->display;
    if (!STD_STRCMP(entry->profile, "default-pc-at")) {
        arguments[argument_count++] = option_cpu;
        arguments[argument_count++] = entry->cpu[0] ? entry->cpu : "80386";
        arguments[argument_count++] = option_fpu;
        arguments[argument_count++] = entry->fpu[0] ? entry->fpu : "none";
        if (entry->memory_bytes != 0u) {
            if (STD_SNPRINTF(memory, sizeof(memory), "%zu",
                    entry->memory_bytes >> 10) < 0) return;
            arguments[argument_count++] = option_memory;
            arguments[argument_count++] = memory;
        }
    }
    if (!STD_STRCMP(entry->profile, "compaq-deskpro-386-model-40")) {
        arguments[argument_count++] = option_even_path;
        arguments[argument_count++] = entry->model40_even_path;
        arguments[argument_count++] = option_even_sha256;
        arguments[argument_count++] = entry->model40_even_sha256;
        arguments[argument_count++] = option_odd_path;
        arguments[argument_count++] = entry->model40_odd_path;
        arguments[argument_count++] = option_odd_sha256;
        arguments[argument_count++] = entry->model40_odd_sha256;
        arguments[argument_count++] = option_provenance;
        arguments[argument_count++] = entry->model40_provenance;
    }
    for (C_INT index = 0; index < argument_count; ++index) {
        command_arguments[index + 2] = arguments[index];
    }
    output.write_line = vm_product_console_write_line;
    output.context = STD_NULL;
    if (!core_product_session_command_execute(context->session_manager,
            argument_count + 2, command_arguments, &output)) return;
    machineProvider->set_display_mode(machineProvider->context,
        !STD_STRCMP(entry->display, "window") ? VM_PRODUCT_CONSOLE_DISPLAY_WINDOW :
        !STD_STRCMP(entry->display, "auto") ? VM_PRODUCT_CONSOLE_DISPLAY_AUTO :
        VM_PRODUCT_CONSOLE_DISPLAY_CONSOLE);
}

static C_VOID vm_product_console_session(vm_product_console_context *context)
{
    const C_CHAR *arguments[CONSOLE_MAXNARG];
    core_product_session_output_provider output;
    C_INT index;

    if (context == STD_NULL || numArgs < 2u) { GetHelp; return; }
    if (!STD_STRCMP(argArray[1], "open") && numArgs == 2u) {
        vm_product_console_open_profile(context);
        return;
    }
    for (index = 0; index < numArgs; ++index) arguments[index] = argArray[index];
    output.write_line = vm_product_console_write_line;
    output.context = STD_NULL;
    (C_VOID)core_product_session_command_execute(context->session_manager,
        numArgs, arguments, &output);
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
    vm_product_session_catalog_initialize(&context->catalog, profile_directory);
    return TYPE_TRUE;
}

/* Finalizes console */
static C_VOID vm_product_console_finalize(vm_product_console_context *context)
{
    if (argArray)
    {
        STD_FREE((C_VOID *)argArray);
    }
    argArray = STD_NULL;
}

/* Entry point of product console */
C_VOID vm_product_console_context_initialize(
    vm_product_console_context *context)
{
    if (context != STD_NULL)
        STD_MEMSET(context, 0, sizeof(*context));
}

C_VOID vm_product_console_main(vm_product_console_context *context,
                               const vm_product_console_machine_provider *machine_provider,
                               core_product_session_manager *session_manager,
                               const C_CHAR *profile_directory)
{
    if (context == STD_NULL || machine_provider == STD_NULL ||
        session_manager == STD_NULL)
        return;
    vm_product_console_context_initialize(context);
    machineProvider = machine_provider;
    sessionManager = session_manager;
    if (!vm_product_console_initialize(context, profile_directory)) return;
    STD_PRINTF("\nType HELP for help.\n\n");
    vm_product_console_open_profile(context);
    while (!flagExit)
    {
        STD_PRINTF("Console> ");
        if (!vm_product_console_read_line(strCmdBuff, sizeof(strCmdBuff))) break;
        parse(context);
        execute(context);
    }
    vm_product_console_finalize(context);
}
