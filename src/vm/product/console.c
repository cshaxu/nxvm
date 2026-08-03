/* Copyright 2012-2014 Neko. */

/*
 * NXVM console provides a command-line interface for users
 * to configure, debug and run the virtual machine.
 */

#include "type.h"

#include "core/product/utils.h"


#include "vm/product/console.h"

#define CONSOLE_MAXNARG 256

#define consoleContext context

#define numArgs (consoleContext->argument_count)
#define argArray (consoleContext->arguments)
#define flagExit (consoleContext->exit_requested)
#define strCmdBuff (consoleContext->command_buffer)
#define consoleTarget (consoleContext->target)

/*
 * Parses command-line input.
 *
 * strCmdBuff [IN]  String buffer of command-line input
 * numArgs       [OUT] Number of argArrayuments
 * argArray        [OUT] Array of argArrayuments
 */
static C_VOID parse(nxvm_product_console_context *context) {
    numArgs = 0;
    argArray[numArgs] = STD_STRTOK(strCmdBuff, " \t\n\r\f");
    if (!argArray[numArgs]) {
        return;
    }
    ntvdm64_type_string_lower(argArray[numArgs++]);
    while (numArgs < CONSOLE_MAXNARG) {
        argArray[numArgs] = STD_STRTOK(STD_NULL, " \t\n\r\f");
        if (argArray[numArgs]) {
            ntvdm64_type_string_lower(argArray[numArgs++]);
        } else {
            break;
        }
    }
}

/* Prints help commands. */
#define GetHelp if (1) {doHelp(context);return;} else
static C_VOID doHelp(nxvm_product_console_context *context) {
    if (STD_STRCMP(argArray[0], "help")) {
        numArgs = 2;
        argArray[1] = argArray[0];
    }
    switch (numArgs) {
    case 2:
        if (!STD_STRCMP(argArray[1], "help")) {
            STD_PRINTF("Show help info\n");
            STD_PRINTF("\nHELP\n");
            STD_PRINTF("  show menu of all commands\n");
            STD_PRINTF("\nHELP <command>\n");
            STD_PRINTF("  show help info for command\n");
            break;
        } else if (!STD_STRCMP(argArray[1], "exit")) {
            STD_PRINTF("Quit the console\n");
            STD_PRINTF("\nEXIT\n");
            break;
        } else if (!STD_STRCMP(argArray[1], "info")) {
            STD_PRINTF("List virtual machine status\n");
            STD_PRINTF("\nINFO\n");
            break;
        } else if (!STD_STRCMP(argArray[1], "debug")) {
            STD_PRINTF("Launch NXVM hardware debugger\n");
            STD_PRINTF("\nDEBUG\n");
            break;
        } else if (!STD_STRCMP(argArray[1], "debug32")) {
            STD_PRINTF("Launch NXVM 32-bit hardware debugger\n");
            STD_PRINTF("\nDEBUG\n");
            break;
        } else if (!STD_STRCMP(argArray[1], "record")) {
            STD_PRINTF("Record cpu status in each iteration for futher dumping\n");
            STD_PRINTF("\nRECORD start <file> | stop\n");
            STD_PRINTF("  start: open output file for record writes\n");
            STD_PRINTF("  stop:  close output file to finish recording\n");
            break;
        } else if (!STD_STRCMP(argArray[1], "set")) {
            STD_PRINTF("Change BIOS settings\n");
            STD_PRINTF("\nSET <item> <value>\n");
            STD_PRINTF("  available items and values\n");
            STD_PRINTF("  boot   fdd, hdd\n");
            break;
        } else if (!STD_STRCMP(argArray[1], "device")) {
            STD_PRINTF("Change NXVM devices\n");
            STD_PRINTF("\nDEVICE ram <size>\n");
            STD_PRINTF("  change memory size (KB)\n");
            STD_PRINTF("\nDEVICE display console | window\n");
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
        } else if (!STD_STRCMP(argArray[1], "start")) {
            STD_PRINTF("Start virtual machine\n");
            STD_PRINTF("\nSTART\n");
            break;
        } else if (!STD_STRCMP(argArray[1], "reset")) {
            STD_PRINTF("Reset virtual machine\n");
            STD_PRINTF("\nRESET\n");
            break;
        } else if (!STD_STRCMP(argArray[1], "stop")) {
            STD_PRINTF("Stop virtual machine\n");
            STD_PRINTF("\nSTOP\n");
            break;
        } else if (!STD_STRCMP(argArray[1], "resume")) {
            STD_PRINTF("Resume virtual machine\n");
            STD_PRINTF("\nRESUME\n");
            break;
        }
    case 1:
    default:
        STD_PRINTF("NXVM Console Commands\n");
        STD_PRINTF("=====================\n");
        STD_PRINTF("HELP    Show help info\n");
        STD_PRINTF("EXIT    Quit the console\n");
        STD_PRINTF("INFO    List all NXVM info\n");
        STD_PRINTF("\n");
        STD_PRINTF("DEBUG   Launch NXVM hardware debugger\n");
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

/* Quits NXVM. */
static C_VOID doExit(nxvm_product_console_context *context) {
    if (numArgs != 1) {
        GetHelp;
    }
    if (!consoleTarget->is_running(consoleTarget->context)) {
        flagExit = 1;
    } else {
        STD_PRINTF("Please stop NXVM before exit.\n");
    }
}

/* Prints virtual machine status */
static C_VOID doInfo(nxvm_product_console_context *context) {
    if (numArgs != 1) {
        GetHelp;
    }
    STD_PRINTF("NXVM Device Info\n");
    STD_PRINTF("================\n");
    consoleTarget->print_machine(consoleTarget->context);
    STD_PRINTF("\n");
    STD_PRINTF("NXVM Platform Info\n");
    STD_PRINTF("==================\n");
    STD_PRINTF("Display Type: %s\n", consoleTarget->get_window_display(consoleTarget->context) ? "Window" : "Console");
    STD_PRINTF("\n");
    STD_PRINTF("NXVM BIOS Settings\n");
    STD_PRINTF("==================\n");
    consoleTarget->print_bios(consoleTarget->context);
    STD_PRINTF("\n");
    STD_PRINTF("NXVM Device Status\n");
    STD_PRINTF("==================\n");
    consoleTarget->print_status(consoleTarget->context);
}

/* Starts internal debugger */
static C_VOID doDebug(nxvm_product_console_context *context) {
    if (numArgs != 1) {
        GetHelp;
    }
    consoleTarget->debug(consoleTarget->context);
}

/* Executes cpu instruction recorder */
static C_VOID doRecord(nxvm_product_console_context *context) {
    if (numArgs < 2) {
        GetHelp;
    }
    if (consoleTarget->is_running(consoleTarget->context)) {
        STD_PRINTF("Cannot change record status or dump record now.\n");
        return;
    }
    if (!STD_STRCMP(argArray[1], "start")) {
        if (numArgs != 3) {
            GetHelp;
        }
        consoleTarget->record_start(consoleTarget->context, argArray[2]);
    } else if (!STD_STRCMP(argArray[1], "stop")) {
        consoleTarget->record_stop(consoleTarget->context);
    } else {
        GetHelp;
    }
}

/* Sets BIOS settings */
static C_VOID doSet(nxvm_product_console_context *context) {
    if (numArgs < 2) {
        GetHelp;
    }
    if (!STD_STRCMP(argArray[1], "boot")) {
        if (numArgs != 3) {
            GetHelp;
        }
        if (!STD_STRCMP(argArray[2], "fdd")) {
            consoleTarget->set_boot_hdd(consoleTarget->context, 0);
        } else if (!STD_STRCMP(argArray[2], "hdd")) {
            consoleTarget->set_boot_hdd(consoleTarget->context, 1);
        } else {
            GetHelp;
        }
    } else {
        GetHelp;
    }
}

/* Set hardware connections */
static C_VOID doDevice(nxvm_product_console_context *context) {
    if (numArgs < 2) {
        GetHelp;
    }
    if (consoleTarget->is_running(consoleTarget->context)) {
        STD_PRINTF("Cannot change device now.\n");
        return;
    }
    if (!STD_STRCMP(argArray[1], "ram")) {
        if (numArgs != 3) {
            GetHelp;
        }
        consoleTarget->set_memory(consoleTarget->context, (STD_SIZE_T)STD_ATOI(argArray[2]) << 10);
    } else if (!STD_STRCMP(argArray[1], "display")) {
        if (numArgs != 3) {
            GetHelp;
        }
        if (!STD_STRCMP(argArray[2], "console")) {
            consoleTarget->set_window_display(consoleTarget->context, 0);
        } else if (!STD_STRCMP(argArray[2], "window")) {
            consoleTarget->set_window_display(consoleTarget->context, 1);
        } else {
            GetHelp;
        }
    } else if (!STD_STRCMP(argArray[1], "fdd")) {
        if (numArgs < 3) {
            GetHelp;
        }
        if (!STD_STRCMP(argArray[2], "create")) {
            consoleTarget->create_fdd(consoleTarget->context);
            STD_PRINTF("Floppy disk created.\n");
        } else if (!STD_STRCMP(argArray[2], "insert")) {
            if (numArgs < 4) {
                GetHelp;
            }
            if (!consoleTarget->insert_fdd(consoleTarget->context, argArray[3])) {
                STD_PRINTF("Floppy disk inserted.\n");
            } else {
                STD_PRINTF("Cannot read floppy disk from '%s'.\n", argArray[3]);
            }
        } else if (!STD_STRCMP(argArray[2], "remove")) {
            if (numArgs < 4) {
                argArray[3] = STD_NULL;
            }
            if (!consoleTarget->remove_fdd(consoleTarget->context, argArray[3])) {
                STD_PRINTF("Floppy disk removed.\n");
            } else {
                STD_PRINTF("Cannot write floppy disk to '%s'.\n", argArray[3]);
            }
        } else {
            GetHelp;
        }
    } else if (!STD_STRCMP(argArray[1], "hdd")) {
        if (numArgs < 3) {
            GetHelp;
        }
        if (!STD_STRCMP(argArray[2], "create")) {
            if (numArgs > 3) {
                if (numArgs == 5 && !STD_STRCMP(argArray[3], "cyl")) {
                    if (STD_ATOI(argArray[4])) {
                        consoleTarget->create_hdd(consoleTarget->context, (uint16_t)STD_ATOI(argArray[4]));
                    } else {
                        GetHelp;
                    }
                } else {
                    GetHelp;
                }
            } else {
                consoleTarget->create_hdd(consoleTarget->context, 20);
            }
            STD_PRINTF("Hard disk created.\n");
        } else if (!STD_STRCMP(argArray[2], "connect")) {
            if (numArgs < 4) {
                GetHelp;
            }
            if (!consoleTarget->insert_hdd(consoleTarget->context, argArray[3])) {
                STD_PRINTF("Hard disk connected.\n");
            } else {
                STD_PRINTF("Cannot read hard disk from '%s'.\n", argArray[3]);
            }
        } else if (!STD_STRCMP(argArray[2], "disconnect")) {
            if (numArgs < 4) {
                argArray[3] = STD_NULL;
            }
            if (!consoleTarget->remove_hdd(consoleTarget->context, argArray[3])) {
                STD_PRINTF("Hard disk disconnected.\n");
            } else {
                STD_PRINTF("Cannot write hard disk to '%s'.\n", argArray[3]);
            }
        } else {
            GetHelp;
        }
    } else {
        GetHelp;
    }
}

/* Tests NXVM: reset and start debugger */
static C_VOID doTest(nxvm_product_console_context *context) {
    consoleTarget->reset(consoleTarget->context);
    consoleTarget->debug(consoleTarget->context);
}

/* Executes commands */
static C_VOID execute(nxvm_product_console_context *context) {
    if (!argArray[0] || !STD_STRLEN(argArray[0])) {
        return;
    } else if (!STD_STRCMP(argArray[0], "test")) {
        doTest(context);
    } else if (!STD_STRCMP(argArray[0], "help")) {
        doHelp(context);
    } else if (!STD_STRCMP(argArray[0], "exit")) {
        doExit(context);
    } else if (!STD_STRCMP(argArray[0], "info")) {
        doInfo(context);
    } else if (!STD_STRCMP(argArray[0], "debug")) {
        doDebug(context);
    } else if (!STD_STRCMP(argArray[0], "record")) {
        doRecord(context);
    } else if (!STD_STRCMP(argArray[0], "set")) {
        doSet(context);
    } else if (!STD_STRCMP(argArray[0], "device")) {
        doDevice(context);
    } else if (!STD_STRCMP(argArray[0], "mode")) {
        if (!consoleTarget->is_running(consoleTarget->context)) {
            consoleTarget->set_window_display(consoleTarget->context,
                !consoleTarget->get_window_display(consoleTarget->context));
        }
    } else if (!STD_STRCMP(argArray[0], "start")) {
        consoleTarget->start(consoleTarget->context);
    } else if (!STD_STRCMP(argArray[0], "reset")) {
        consoleTarget->reset(consoleTarget->context);
    } else if (!STD_STRCMP(argArray[0], "stop")) {
        consoleTarget->stop(consoleTarget->context);
    } else if (!STD_STRCMP(argArray[0], "resume")) {
        consoleTarget->resume(consoleTarget->context);
    } else {
        STD_PRINTF("Illegal command '%s'.\n",argArray[0]);
    }
    STD_PRINTF("\n");
}

/* Initializes console */
static C_VOID vm_product_console_initialize(nxvm_product_console_context *context) {
    argArray = (C_CHAR **) STD_MALLOC(CONSOLE_MAXNARG * sizeof(C_CHAR *));
    flagExit = 0;
    consoleTarget->initialize(consoleTarget->context);
}

/* Finalizes console */
static C_VOID vm_product_console_finalize(nxvm_product_console_context *context) {
    consoleTarget->finalize(consoleTarget->context);
    if (argArray) {
        STD_FREE((C_VOID *) argArray);
    }
}

/* Entry point of NXVM console */
C_VOID nxvm_product_console_context_initialize(
    nxvm_product_console_context *context)
{
    if (context != STD_NULL) STD_MEMSET(context, 0, sizeof(*context));
}

C_VOID vm_product_console_main(nxvm_product_console_context *context,
                 const nxvm_product_console_target *target) {
    if (context == STD_NULL || target == STD_NULL) return;
    nxvm_product_console_context_initialize(context);
    consoleTarget = target;
    vm_product_console_initialize(context);
    STD_PRINTF("\nPlease enter 'HELP' for information.\n\n");
    while (!flagExit) {
        STD_PRINTF("Console> ");
        STD_FGETS(strCmdBuff, 0x100, STD_STDIN);
        parse(context);
        execute(context);
    }
    vm_product_console_finalize(context);
}
