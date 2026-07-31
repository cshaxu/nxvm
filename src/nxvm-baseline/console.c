/* Copyright 2012-2014 Neko. */

/*
 * NXVM console provides a command-line interface for users
 * to configure, debug and run the virtual machine.
 */

#include "utils.h"

#include "device/device.h"
#include "platform/platform.h"
#include "machine.h"

#include "debug.h"
#include "console.h"

#define CONSOLE_MAXNARG 256

static size_t numArgs;
static char **argArray;
static int flagExit;
static char strCmdBuff[0x100];

/*
 * Parses command-line input.
 *
 * strCmdBuff [IN]  String buffer of command-line input
 * numArgs       [OUT] Number of argArrayuments
 * argArray        [OUT] Array of argArrayuments
 */
static void parse() {
    numArgs = 0;
    argArray[numArgs] = STRTOK(strCmdBuff, " \t\n\r\f");
    if (!argArray[numArgs]) {
        return;
    }
    utilsLowerStr(argArray[numArgs++]);
    while (numArgs < CONSOLE_MAXNARG) {
        argArray[numArgs] = STRTOK(NULL, " \t\n\r\f");
        if (argArray[numArgs]) {
            utilsLowerStr(argArray[numArgs++]);
        } else {
            break;
        }
    }
}

/* Prints help commands. */
#define GetHelp if (1) {doHelp();return;} else
static void doHelp() {
    if (STRCMP(argArray[0], "help")) {
        numArgs = 2;
        argArray[1] = argArray[0];
    }
    switch (numArgs) {
    case 2:
        if (!STRCMP(argArray[1], "help")) {
            PRINTF("Show help info\n");
            PRINTF("\nHELP\n");
            PRINTF("  show menu of all commands\n");
            PRINTF("\nHELP <command>\n");
            PRINTF("  show help info for command\n");
            break;
        } else if (!STRCMP(argArray[1], "exit")) {
            PRINTF("Quit the console\n");
            PRINTF("\nEXIT\n");
            break;
        } else if (!STRCMP(argArray[1], "info")) {
            PRINTF("List virtual machine status\n");
            PRINTF("\nINFO\n");
            break;
        } else if (!STRCMP(argArray[1], "debug")) {
            PRINTF("Launch NXVM hardware debugger\n");
            PRINTF("\nDEBUG\n");
            break;
        } else if (!STRCMP(argArray[1], "debug32")) {
            PRINTF("Launch NXVM 32-bit hardware debugger\n");
            PRINTF("\nDEBUG\n");
            break;
        } else if (!STRCMP(argArray[1], "record")) {
            PRINTF("Record cpu status in each iteration for futher dumping\n");
            PRINTF("\nRECORD start <file> | stop\n");
            PRINTF("  start: open output file for record writes\n");
            PRINTF("  stop:  close output file to finish recording\n");
            break;
        } else if (!STRCMP(argArray[1], "set")) {
            PRINTF("Change BIOS settings\n");
            PRINTF("\nSET <item> <value>\n");
            PRINTF("  available items and values\n");
            PRINTF("  boot   fdd, hdd\n");
            break;
        } else if (!STRCMP(argArray[1], "device")) {
            PRINTF("Change NXVM devices\n");
            PRINTF("\nDEVICE ram <size>\n");
            PRINTF("  change memory size (KB)\n");
            PRINTF("\nDEVICE display console | window\n");
            PRINTF("  change display type\n");
            PRINTF("\nDEVICE fdd create | (insert <file>) | (remove <file>)\n");
            PRINTF("  change floppy drive status:\n");
            PRINTF("  create: discard current floppy image\n");
            PRINTF("          and create a new one\n");
            PRINTF("  insert: load floppy image from file\n");
            PRINTF("  remove: remove floppy image and dump to file\n");
            PRINTF("\nDEVICE hdd (create [cyl <num>]) | (connect <file>) | (disconnect <file>)\n");
            PRINTF("  change hard disk drive status:\n");
            PRINTF("  create:     discard current hard disk image\n");
            PRINTF("              and create a new one of n cyls\n");
            PRINTF("  connect:    load hard disk image from file\n");
            PRINTF("  disconnect: remove hard disk image and dump to file\n");
            break;
        } else if (!STRCMP(argArray[1], "start")) {
            PRINTF("Start virtual machine\n");
            PRINTF("\nSTART\n");
            break;
        } else if (!STRCMP(argArray[1], "reset")) {
            PRINTF("Reset virtual machine\n");
            PRINTF("\nRESET\n");
            break;
        } else if (!STRCMP(argArray[1], "stop")) {
            PRINTF("Stop virtual machine\n");
            PRINTF("\nSTOP\n");
            break;
        } else if (!STRCMP(argArray[1], "resume")) {
            PRINTF("Resume virtual machine\n");
            PRINTF("\nRESUME\n");
            break;
        }
    case 1:
    default:
        PRINTF("NXVM Console Commands\n");
        PRINTF("=====================\n");
        PRINTF("HELP    Show help info\n");
        PRINTF("EXIT    Quit the console\n");
        PRINTF("INFO    List all NXVM info\n");
        PRINTF("\n");
        PRINTF("DEBUG   Launch NXVM hardware debugger\n");
        PRINTF("RECORD  Record cpu status for each instruction\n");
        PRINTF("\n");
        PRINTF("SET     Change BIOS settings\n");
        PRINTF("DEVICE  Change hardware parts\n");
        PRINTF("\n");
        PRINTF("START   Start virtual machine\n");
        PRINTF("RESET   Reset virtual machine\n");
        PRINTF("STOP    Stop virtual machine\n");
        PRINTF("RESUME  Resume virtual machine\n");
        PRINTF("\n");
        PRINTF("For command usage, type 'HELP <command>'.\n");
        break;
    }
}

/* Quits NXVM. */
static void doExit() {
    if (numArgs != 1) {
        GetHelp;
    }
    if (!device.flagRun) {
        flagExit = 1;
    } else {
        PRINTF("Please stop NXVM before exit.\n");
    }
}

/* Prints virtual machine status */
static void doInfo() {
    if (numArgs != 1) {
        GetHelp;
    }
    PRINTF("NXVM Device Info\n");
    PRINTF("================\n");
    devicePrintMachine();
    PRINTF("\n");
    PRINTF("NXVM Platform Info\n");
    PRINTF("==================\n");
    PRINTF("Display Type: %s\n", platform.flagMode ? "Window" : "Console");
    PRINTF("\n");
    PRINTF("NXVM BIOS Settings\n");
    PRINTF("==================\n");
    devicePrintBios();
    PRINTF("\n");
    PRINTF("NXVM Device Status\n");
    PRINTF("==================\n");
    devicePrintStatus();
}

/* Starts internal debugger */
static void doDebug() {
    if (numArgs != 1) {
        GetHelp;
    }
    debugMain();
}

/* Executes cpu instruction recorder */
static void doRecord() {
    if (numArgs < 2) {
        GetHelp;
    }
    if (device.flagRun) {
        PRINTF("Cannot change record status or dump record now.\n");
        return;
    }
    if (!STRCMP(argArray[1], "start")) {
        if (numArgs != 3) {
            GetHelp;
        }
        deviceConnectDebugRecordStart(argArray[2]);
    } else if (!STRCMP(argArray[1], "stop")) {
        deviceConnectDebugRecordStop();
    } else {
        GetHelp;
    }
}

/* Sets BIOS settings */
static void doSet() {
    if (numArgs < 2) {
        GetHelp;
    }
    if (!STRCMP(argArray[1], "boot")) {
        if (numArgs != 3) {
            GetHelp;
        }
        if (!STRCMP(argArray[2], "fdd")) {
            deviceConnectBiosSetBoot(0);
        } else if (!STRCMP(argArray[2], "hdd")) {
            deviceConnectBiosSetBoot(1);
        } else {
            GetHelp;
        }
    } else {
        GetHelp;
    }
}

/* Set hardware connections */
static void doDevice() {
    if (numArgs < 2) {
        GetHelp;
    }
    if (device.flagRun) {
        PRINTF("Cannot change device now.\n");
        return;
    }
    if (!STRCMP(argArray[1], "ram")) {
        if (numArgs != 3) {
            GetHelp;
        }
        deviceConnectRamAllocate(atoi(argArray[2]) << 10);
    } else if (!STRCMP(argArray[1], "display")) {
        if (numArgs != 3) {
            GetHelp;
        }
        if (!STRCMP(argArray[2], "console")) {
            platform.flagMode = 0;
        } else if (!STRCMP(argArray[2], "window")) {
            platform.flagMode = 1;
        } else {
            GetHelp;
        }
    } else if (!STRCMP(argArray[1], "fdd")) {
        if (numArgs < 3) {
            GetHelp;
        }
        if (!STRCMP(argArray[2], "create")) {
            deviceConnectFloppyCreate();
            PRINTF("Floppy disk created.\n");
        } else if (!STRCMP(argArray[2], "insert")) {
            if (numArgs < 4) {
                GetHelp;
            }
            if (!deviceConnectFloppyInsert(argArray[3])) {
                PRINTF("Floppy disk inserted.\n");
            } else {
                PRINTF("Cannot read floppy disk from '%s'.\n", argArray[3]);
            }
        } else if (!STRCMP(argArray[2], "remove")) {
            if (numArgs < 4) {
                argArray[3] = NULL;
            }
            if (!deviceConnectFloppyRemove(argArray[3])) {
                PRINTF("Floppy disk removed.\n");
            } else {
                PRINTF("Cannot write floppy disk to '%s'.\n", argArray[3]);
            }
        } else {
            GetHelp;
        }
    } else if (!STRCMP(argArray[1], "hdd")) {
        if (numArgs < 3) {
            GetHelp;
        }
        if (!STRCMP(argArray[2], "create")) {
            if (numArgs > 3) {
                if (numArgs == 5 && !STRCMP(argArray[3], "cyl")) {
                    if (atoi(argArray[4])) {
                        deviceConnectHardDiskCreate(atoi(argArray[4]));
                    } else {
                        GetHelp;
                    }
                } else {
                    GetHelp;
                }
            } else {
                deviceConnectHardDiskCreate(20);
            }
            PRINTF("Hard disk created.\n");
        } else if (!STRCMP(argArray[2], "connect")) {
            if (numArgs < 4) {
                GetHelp;
            }
            if (!deviceConnectHardDiskInsert(argArray[3])) {
                PRINTF("Hard disk connected.\n");
            } else {
                PRINTF("Cannot read hard disk from '%s'.\n", argArray[3]);
            }
        } else if (!STRCMP(argArray[2], "disconnect")) {
            if (numArgs < 4) {
                argArray[3] = NULL;
            }
            if (!deviceConnectHardDiskRemove(argArray[3])) {
                PRINTF("Hard disk disconnected.\n");
            } else {
                PRINTF("Cannot write hard disk to '%s'.\n", argArray[3]);
            }
        } else {
            GetHelp;
        }
    } else {
        GetHelp;
    }
}

/* Tests NXVM: reset and start debugger */
static void doTest() {
    deviceReset();
    debugMain();
}

/* Executes commands */
static void execute() {
    if (!argArray[0] || !STRLEN(argArray[0])) {
        return;
    } else if (!STRCMP(argArray[0], "test")) {
        doTest();
    } else if (!STRCMP(argArray[0], "help")) {
        doHelp();
    } else if (!STRCMP(argArray[0], "exit")) {
        doExit();
    } else if (!STRCMP(argArray[0], "info")) {
        doInfo();
    } else if (!STRCMP(argArray[0], "debug")) {
        doDebug();
    } else if (!STRCMP(argArray[0], "record")) {
        doRecord();
    } else if (!STRCMP(argArray[0], "set")) {
        doSet();
    } else if (!STRCMP(argArray[0], "device")) {
        doDevice();
    } else if (!STRCMP(argArray[0], "mode")) {
        if (!device.flagRun) {
            platform.flagMode = !platform.flagMode;
        }
    } else if (!STRCMP(argArray[0], "start")) {
        machineStart();
    } else if (!STRCMP(argArray[0], "reset")) {
        machineReset();
    } else if (!STRCMP(argArray[0], "stop")) {
        machineStop();
    } else if (!STRCMP(argArray[0], "resume")) {
        machineResume();
    } else {
        PRINTF("Illegal command '%s'.\n",argArray[0]);
    }
    PRINTF("\n");
}

/* Initializes console */
static void consoleInit() {
    argArray = (char **) MALLOC(CONSOLE_MAXNARG * sizeof(char *));
    flagExit = 0;
    machineInit();
#if GLOBAL_PLATFORM == GLOBAL_VAR_WIN32
    deviceConnectFloppyInsert("d:/fd.img");
    deviceConnectHardDiskInsert("d:/hd.img");
#else
    deviceConnectFloppyInsert("/Users/xha/fd.img");
    deviceConnectHardDiskInsert("/Users/xha/hd.img");
#endif
}

/* Finalizes console */
static void consoleFinal() {
    machineFinal();
    if (argArray) {
        FREE((void *) argArray);
    }
}

/* Entry point of NXVM console */
void consoleMain() {
    consoleInit();
    PRINTF("\nPlease enter 'HELP' for information.\n\n");
    while (!flagExit) {
        PRINTF("Console> ");
        FGETS(strCmdBuff, 0x100, stdin);
        parse();
        execute();
    }
    consoleFinal();
}
