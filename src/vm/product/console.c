/* Copyright 2012-2014 Neko. */

/*
 * Console provides a command-line interface for users
 * to configure, debug and run the virtual machine.
 */

#include "type.h"

#include "core/product/session/command_interface.h"

#include "vm/product/console.h"

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
            STD_PRINTF("\nSESSION LIST | OPEN [--cpu <model>] [--fpu <model>] | SELECT <id> | CLOSE [id]\n");
            STD_PRINTF("  list:   show sessions; * marks the selected session\n");
            STD_PRINTF("  open:   create one stopped session (default: 80386, no FPU)\n");
            STD_PRINTF("          cpu: 8086, 80186, 80286, 80386; fpu: none\n");
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
        else if (!STD_STRCMP(argArray[1], "set"))
        {
            STD_PRINTF("Change BIOS settings\n");
            STD_PRINTF("\nSET <item> <value>\n");
            STD_PRINTF("  available items and values\n");
            STD_PRINTF("  boot   fdd, hdd\n");
            break;
        }
        else if (!STD_STRCMP(argArray[1], "device"))
        {
            STD_PRINTF("Change VM devices\n");
            STD_PRINTF("\nDEVICE ram <size>\n");
            STD_PRINTF("  change memory size (KB)\n");
            STD_PRINTF("\nDEVICE display console | window | auto\n");
            STD_PRINTF("  change display type\n");
            STD_PRINTF("\nDEVICE fdd create | (insert <file>) | (remove <file>)\n");
            STD_PRINTF("  change floppy drive status:\n");
            STD_PRINTF("  create: discard current floppy image\n");
            STD_PRINTF("          and create a new one\n");
            STD_PRINTF("  insert: load floppy image from file\n");
            STD_PRINTF("  remove: remove floppy image and dump to file\n");
            STD_PRINTF("\nDEVICE hdd (create [cyl <num>]) | (connect <file>) | (disconnect <file>)\n");
            STD_PRINTF("  change hard disk drive status:\n");
            STD_PRINTF("  create:     discard current hard disk image\n");
            STD_PRINTF("              and create a new one of n cyls\n");
            STD_PRINTF("  connect:    load hard disk image from file\n");
            STD_PRINTF("  disconnect: remove hard disk image and dump to file\n");
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
        STD_PRINTF("SET     Change BIOS settings\n");
        STD_PRINTF("DEVICE  Change hardware parts\n");
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

/* Sets BIOS settings */
static C_VOID doSet(vm_product_console_context *context)
{
    if (numArgs < 2)
    {
        GetHelp;
    }
    if (!STD_STRCMP(argArray[1], "boot"))
    {
        if (numArgs != 3)
        {
            GetHelp;
        }
        if (!STD_STRCMP(argArray[2], "fdd"))
        {
            machineProvider->set_boot_hdd(machineProvider->context, 0);
        }
        else if (!STD_STRCMP(argArray[2], "hdd"))
        {
            machineProvider->set_boot_hdd(machineProvider->context, 1);
        }
        else
        {
            GetHelp;
        }
    }
    else
    {
        GetHelp;
    }
}

/* Set hardware connections */
static C_VOID doDevice(vm_product_console_context *context)
{
    if (numArgs < 2)
    {
        GetHelp;
    }
    if (machineProvider->is_running(machineProvider->context))
    {
        STD_PRINTF("Cannot change device now.\n");
        return;
    }
    if (!STD_STRCMP(argArray[1], "ram"))
    {
        if (numArgs != 3)
        {
            GetHelp;
        }
        machineProvider->set_memory(machineProvider->context, (STD_SIZE_T)STD_ATOI(argArray[2]) << 10);
    }
    else if (!STD_STRCMP(argArray[1], "display"))
    {
        if (numArgs != 3)
        {
            GetHelp;
        }
        if (!STD_STRCMP(argArray[2], "console"))
        {
            machineProvider->set_display_mode(machineProvider->context,
                VM_PRODUCT_CONSOLE_DISPLAY_CONSOLE);
        }
        else if (!STD_STRCMP(argArray[2], "window"))
        {
            machineProvider->set_display_mode(machineProvider->context,
                VM_PRODUCT_CONSOLE_DISPLAY_WINDOW);
        }
        else if (!STD_STRCMP(argArray[2], "auto"))
        {
            machineProvider->set_display_mode(machineProvider->context,
                VM_PRODUCT_CONSOLE_DISPLAY_AUTO);
        }
        else
        {
            GetHelp;
        }
    }
    else if (!STD_STRCMP(argArray[1], "fdd"))
    {
        if (numArgs < 3)
        {
            GetHelp;
        }
        if (!STD_STRCMP(argArray[2], "create"))
        {
            machineProvider->create_fdd(machineProvider->context);
            STD_PRINTF("Floppy disk created.\n");
        }
        else if (!STD_STRCMP(argArray[2], "insert"))
        {
            if (numArgs < 4)
            {
                GetHelp;
            }
            if (!machineProvider->insert_fdd(machineProvider->context, argArray[3]))
            {
                STD_PRINTF("Floppy disk inserted.\n");
            }
            else
            {
                STD_PRINTF("Cannot read floppy disk from '%s'.\n", argArray[3]);
            }
        }
        else if (!STD_STRCMP(argArray[2], "remove"))
        {
            if (numArgs < 4)
            {
                argArray[3] = STD_NULL;
            }
            if (!machineProvider->remove_fdd(machineProvider->context, argArray[3]))
            {
                STD_PRINTF("Floppy disk removed.\n");
            }
            else
            {
                STD_PRINTF("Cannot write floppy disk to '%s'.\n", argArray[3]);
            }
        }
        else
        {
            GetHelp;
        }
    }
    else if (!STD_STRCMP(argArray[1], "hdd"))
    {
        if (numArgs < 3)
        {
            GetHelp;
        }
        if (!STD_STRCMP(argArray[2], "create"))
        {
            if (numArgs > 3)
            {
                if (numArgs == 5 && !STD_STRCMP(argArray[3], "cyl"))
                {
                    if (STD_ATOI(argArray[4]))
                    {
                        machineProvider->create_hdd(machineProvider->context, (uint16_t)STD_ATOI(argArray[4]));
                    }
                    else
                    {
                        GetHelp;
                    }
                }
                else
                {
                    GetHelp;
                }
            }
            else
            {
                machineProvider->create_hdd(machineProvider->context, 20);
            }
            STD_PRINTF("Hard disk created.\n");
        }
        else if (!STD_STRCMP(argArray[2], "connect"))
        {
            if (numArgs < 4)
            {
                GetHelp;
            }
            if (!machineProvider->insert_hdd(machineProvider->context, argArray[3]))
            {
                STD_PRINTF("Hard disk connected.\n");
            }
            else
            {
                STD_PRINTF("Cannot read hard disk from '%s'.\n", argArray[3]);
            }
        }
        else if (!STD_STRCMP(argArray[2], "disconnect"))
        {
            if (numArgs < 4)
            {
                argArray[3] = STD_NULL;
            }
            if (!machineProvider->remove_hdd(machineProvider->context, argArray[3]))
            {
                STD_PRINTF("Hard disk disconnected.\n");
            }
            else
            {
                STD_PRINTF("Cannot write hard disk to '%s'.\n", argArray[3]);
            }
        }
        else
        {
            GetHelp;
        }
    }
    else
    {
        GetHelp;
    }
}

/* Tests product: reset and start debugger */
static C_VOID doTest(vm_product_console_context *context)
{
    machineProvider->reset(machineProvider->context);
    machineProvider->debug(machineProvider->context);
}

static C_VOID vm_product_console_write_line(C_VOID *opaque, const C_CHAR *line)
{
    (C_VOID) opaque;
    STD_PRINTF("%s\n", line);
}

/* Executes commands */
static C_VOID execute(vm_product_console_context *context)
{
    if (!argArray[0] || !STD_STRLEN(argArray[0]))
    {
        return;
    }
    else if (!STD_STRCMP(argArray[0], "session"))
    {
        const core_product_session_output_provider output = {
            vm_product_console_write_line, STD_NULL};
        (C_VOID) core_product_session_command_execute(sessionManager,
                                                      (C_INT)numArgs, argArray, &output);
    }
    else if (!STD_STRCMP(argArray[0], "test"))
    {
        doTest(context);
    }
    else if (!STD_STRCMP(argArray[0], "help"))
    {
        doHelp(context);
    }
    else if (!STD_STRCMP(argArray[0], "exit"))
    {
        doExit(context);
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
    else if (!STD_STRCMP(argArray[0], "set"))
    {
        doSet(context);
    }
    else if (!STD_STRCMP(argArray[0], "device"))
    {
        doDevice(context);
    }
    else if (!STD_STRCMP(argArray[0], "mode"))
    {
        if (!machineProvider->is_running(machineProvider->context))
        {
            vm_product_console_display_mode mode = machineProvider->get_display_mode(
                machineProvider->context);
            machineProvider->set_display_mode(machineProvider->context,
                mode == VM_PRODUCT_CONSOLE_DISPLAY_CONSOLE ?
                VM_PRODUCT_CONSOLE_DISPLAY_WINDOW :
                mode == VM_PRODUCT_CONSOLE_DISPLAY_WINDOW ?
                VM_PRODUCT_CONSOLE_DISPLAY_AUTO :
                VM_PRODUCT_CONSOLE_DISPLAY_CONSOLE);
        }
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
static C_VOID vm_product_console_initialize(vm_product_console_context *context)
{
    argArray = (C_CHAR **)STD_MALLOC(CONSOLE_MAXNARG * sizeof(C_CHAR *));
    flagExit = 0;
}

/* Finalizes console */
static C_VOID vm_product_console_finalize(vm_product_console_context *context)
{
    if (argArray)
    {
        STD_FREE((C_VOID *)argArray);
    }
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
                               core_product_session_manager *session_manager)
{
    if (context == STD_NULL || machine_provider == STD_NULL ||
        session_manager == STD_NULL)
        return;
    vm_product_console_context_initialize(context);
    machineProvider = machine_provider;
    sessionManager = session_manager;
    vm_product_console_initialize(context);
    STD_PRINTF("\nPlease enter 'HELP' for information.\n\n");
    while (!flagExit)
    {
        STD_PRINTF("Console> ");
        STD_FGETS(strCmdBuff, 0x100, STD_STDIN);
        parse(context);
        execute(context);
    }
    vm_product_console_finalize(context);
}
