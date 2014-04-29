/* Copyright 2012-2014 Neko. */

/*
 * NXVM console provides a command-line interface for users
 * to configure, debug and run the virtual machine.
 */

#include "utils.h"

#include "device/vram.h"
#include "device/vcpu.h"
#include "device/vfdd.h"
#include "device/vhdd.h"
#include "device/vdebug.h"
#include "device/vmachine.h"

#include "device/device.h"
#include "platform/platform.h"
#include "machine.h"

#include "debug.h"
#include "console.h"

#define CONSOLE_MAXNARG 256

static int narg;
static char **args;
static t_bool flagExit;
static t_string strCmdBuff;

/*
 * Parses command-line input.
 *
 * strCmdBuff [IN]  String buffer of command-line input
 * narg       [OUT] Number of argsuments
 * args        [OUT] Array of argsuments
 */
static void parse() {
	narg = 0;
	args[narg] = STRTOK(strCmdBuff, " \t\n\r\f");
	if (!args[narg]) {
		return;
	}
	utilsLowerStr(args[narg++]);
	while(narg < CONSOLE_MAXNARG) {
		args[narg] = STRTOK(NULL, " \t\n\r\f");
		if (args[narg]) {
			utilsLowerStr(args[narg++]);
		} else {
			break;
		}
	}
}

/* Prints help commands. */
#define GetHelp if (1) {doHelp();return;} else
static void doHelp() {
	if (strcmp(args[0], "help")) {
		narg = 2;
		args[1] = args[0];
	}
	switch (narg) {
	case 2:
		if (!strcmp(args[1], "help")) {
			utilsPrint("Show help info\n");
			utilsPrint("\nHELP\n");
			utilsPrint("  show menu of all commands\n");
			utilsPrint("\nHELP <command>\n");
			utilsPrint("  show help info for command\n");
			break;
		} else if (!strcmp(args[1], "exit")) {
			utilsPrint("Quit the console\n");
			utilsPrint("\nEXIT\n");
			break;
		} else if (!strcmp(args[1], "info")) {
			utilsPrint("List virtual machine status\n");
			utilsPrint("\nINFO\n");
			break;
		} else if (!strcmp(args[1], "debug")) {
			utilsPrint("Launch NXVM hardware debugger\n");
			utilsPrint("\nDEBUG\n");
			break;
		} else if (!strcmp(args[1], "debug32")) {
			utilsPrint("Launch NXVM 32-bit hardware debugger\n");
			utilsPrint("\nDEBUG\n");
			break;
		} else if (!strcmp(args[1], "record")) {
			utilsPrint("Record cpu status in each iteration for futher dumping\n");
			utilsPrint("\nRECORD start <file> | stop\n");
			utilsPrint("  start: open output file for record writes\n");
			utilsPrint("  stop:  close output file to finish recording\n");
			break;
		} else if (!strcmp(args[1], "set")) {
			utilsPrint("Change BIOS settings\n");
			utilsPrint("\nSET <item> <value>\n");
			utilsPrint("  available items and values\n");
			utilsPrint("  boot   fdd, hdd\n");
			break;
		} else if (!strcmp(args[1], "device")) {
			utilsPrint("Change NXVM devices\n");
			utilsPrint("\nDEVICE ram <size>\n");
			utilsPrint("  change memory size (KB)\n");
			utilsPrint("\nDEVICE display console | window\n");
			utilsPrint("  change display type\n");
			utilsPrint("\nDEVICE fdd create | (insert <file>) | (remove <file>)\n");
			utilsPrint("  change floppy drive status:\n");
			utilsPrint("  create: discard current floppy image\n");
			utilsPrint("          and create a new one\n");
			utilsPrint("  insert: load floppy image from file\n");
			utilsPrint("  remove: remove floppy image and dump to file\n");
			utilsPrint("\nDEVICE hdd (create [cyl <num>]) | (connect <file>) | (disconnect <file>)\n");
			utilsPrint("  change hard disk drive status:\n");
			utilsPrint("  create:     discard current hard disk image\n");
			utilsPrint("              and create a new one of n cyls\n");
			utilsPrint("  connect:    load hard disk image from file\n");
			utilsPrint("  disconnect: remove hard disk image and dump to file\n");
			break;
		} else if (!strcmp(args[1], "start")) {
			utilsPrint("Start virtual machine\n");
			utilsPrint("\nSTART\n");
			break;
		} else if (!strcmp(args[1], "reset")) {
			utilsPrint("Reset virtual machine\n");
			utilsPrint("\nRESET\n");
			break;
		} else if (!strcmp(args[1], "stop")) {
			utilsPrint("Stop virtual machine\n");
			utilsPrint("\nSTOP\n");
			break;
		} else if (!strcmp(args[1], "resume")) {
			utilsPrint("Resume virtual machine\n");
			utilsPrint("\nRESUME\n");
			break;
		}
	case 1:
	default:
		utilsPrint("NXVM Console Commands\n");
		utilsPrint("=====================\n");
		utilsPrint("HELP    Show help info\n");
		utilsPrint("EXIT    Quit the console\n");
		utilsPrint("INFO    List all NXVM info\n");
		utilsPrint("\n");
		utilsPrint("DEBUG   Launch NXVM hardware debugger\n");
		utilsPrint("RECORD  Record cpu status for each instruction\n");
		utilsPrint("\n");
		utilsPrint("SET     Change BIOS settings\n");
		utilsPrint("DEVICE  Change hardware parts\n");
		utilsPrint("\n");
		utilsPrint("START   Start virtual machine\n");
		utilsPrint("RESET   Reset virtual machine\n");
		utilsPrint("STOP    Stop virtual machine\n");
		utilsPrint("RESUME  Resume virtual machine\n");
		utilsPrint("\n");
		utilsPrint("For command usage, type 'HELP <command>'.\n");
		break;
	}
}

/* Quits NXVM. */
static void doExit() {
	if (narg != 1) {
		GetHelp;
	}
	if (!device.flagRun) {
		flagExit = 1;
	} else {
		utilsPrint("Please stop NXVM before exit.\n");
	}
}

/* Prints virtual machine status */
static void doInfo() {
	if (narg != 1) {
		GetHelp;
	}
	utilsPrint("NXVM Device Info\n");
	utilsPrint("================\n");
	utilsPrint("Machine:           %s\n", NXVM_DEVICE_MACHINE);
	utilsPrint("CPU:               %s\n", NXVM_DEVICE_CPU);
	utilsPrint("RAM Size:          %d MB\n", vram.size >> 20);
	utilsPrint("Floppy Disk Drive: %s, %.2f MB, %s\n", NXVM_DEVICE_FDD,
		vfddGetImageSize * 1. / 0xfa000,
		vfdd.flagexist ? "inserted" : "not inserted");
	utilsPrint("Hard Disk Drive:   %s, %.2f MB, %s\n", NXVM_DEVICE_HDD,
		vhddGetImageSize * 1. / 0x100000,
		vhdd.flagexist ? "connected" : "disconnected");
	utilsPrint("Display Type:      %s\n", platform.flagMode ? "Window" : "Console");
	utilsPrint("\n");
	utilsPrint("NXVM BIOS Settings\n");
	utilsPrint("==================\n");
	utilsPrint("Boot Disk: %s\n", deviceConnectBiosGetBoot() ? "Hard Drive" : "Floppy");
	utilsPrint("\n");
	utilsPrint("NXVM Debug Status\n");
	utilsPrint("=================\n");
	utilsPrint("Recorder:    %s\n", vdebug.recordFile ? "On" : "Off");
	utilsPrint("Trace:       %s\n", vdebug.tracecnt ? "On" : "Off");
	utilsPrint("Break Point: ");
	if (vdebug.flagbreak) {
		utilsPrint("%04X:%04X\n", vdebug.breakcs,vdebug.breakip);
	} else if (vdebug.flagbreakx) {
		utilsPrint("L%08X\n", vdebug.breaklinear);
	} else {
		utilsPrint("Off\n");
	}
	utilsPrint("\n");
	utilsPrint("NXVM Running Status\n");
	utilsPrint("===================\n");
	utilsPrint("Running: %s\n", device.flagRun  ? "Yes" : "No");
}

/* Starts internal debugger */
static void doDebug() {
	if (narg != 1) {
		GetHelp;
	}
	debugMain();
}

/* Executes cpu instruction recorder */
static void doRecord() {
	if (narg < 2) {
		GetHelp;
	}
	if (device.flagRun) {
		utilsPrint("Cannot change record status or dump record now.\n");
		return;
	}
	if (!strcmp(args[1], "start")) {
		if (narg != 3) {
			GetHelp;
		}
		debugRecordStart(args[2]);
	} else if (!strcmp(args[1], "stop")) {
		debugRecordStop();
	} else {
		GetHelp;
	}
}

/* Sets BIOS settings */
static void doSet() {
	if (narg < 2) {
		GetHelp;
	}
	if (!strcmp(args[1], "boot")) {
		if (narg != 3) {
			GetHelp;
		}
		if (!strcmp(args[2], "fdd")) {
			deviceConnectBiosSetBoot(0);
		} else if(!strcmp(args[2], "hdd")) {
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
	if (narg < 2) {
		GetHelp;
	}
	if (device.flagRun) {
		utilsPrint("Cannot change device now.\n");
		return;
	}
	if (!strcmp(args[1], "ram")) {
		if (narg != 3) {
			GetHelp;
		}
		vramAlloc(atoi(args[2]) << 10);
	} else if(!strcmp(args[1], "display")) {
		if (narg != 3) {
			GetHelp;
		}
		if (!strcmp(args[2], "console")) {
			platform.flagMode = 0;
		} else if (!strcmp(args[2], "window")) {
			platform.flagMode = 1;
		} else {
			GetHelp;
		}
	} else if(!strcmp(args[1], "fdd")) {
		if (narg < 3) {
			GetHelp;
		}
		if (!strcmp(args[2], "create")) {
			vfdd.flagexist = 1;
			utilsPrint("Floppy disk created.\n");
		} else if (!strcmp(args[2], "insert")) {
			if (narg < 4) {
				GetHelp;
			}
			if (!deviceConnectFloppyInsert(args[3])) {
				utilsPrint("Floppy disk inserted.\n");
			} else {
				utilsPrint("Cannot read floppy disk from '%s'.\n", args[3]);
			}
		} else if (!strcmp(args[2], "remove")) {
			if (narg < 4) {
				args[3] = NULL;
			}
			if (!deviceConnectFloppyRemove(args[3])) {
				utilsPrint("Floppy disk removed.\n");
			} else {
				utilsPrint("Cannot write floppy disk to '%s'.\n", args[3]);
			}
		} else {
			GetHelp;
		}
	} else if(!strcmp(args[1], "hdd")) {
		if (narg < 3) {
			GetHelp;
		}
		if (!strcmp(args[2], "create")) {
			vhdd.flagexist = 1;
			if (narg > 3) {
				if (narg == 5 && !strcmp(args[3], "cyl")) {
					vhdd.ncyl = atoi(args[4]);
				} else {
					GetHelp;
				}
			} else {
				vhdd.ncyl = 20;
			}
			vhddAlloc();
			utilsPrint("Hard disk created.\n");
		} else if (!strcmp(args[2], "connect")) {
			if (narg < 4) {
				GetHelp;
			}
			if (!deviceConnectHardDiskInsert(args[3])) {
				utilsPrint("Hard disk connected.\n");
			} else {
				utilsPrint("Cannot read hard disk from '%s'.\n", args[3]);
			}
		} else if (!strcmp(args[2], "disconnect")) {
			if (narg < 4) {
				args[3] = NULL;
			}
			if (!deviceConnectHardDiskRemove(args[3])) {
				utilsPrint("Hard disk disconnected.\n");
			} else {
				utilsPrint("Cannot write hard disk to '%s'.\n", args[3]);
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
	if (!args[0] || !strlen(args[0])) {
		return;
	} else if (!strcmp(args[0], "test")) {
		doTest();
	} else if (!strcmp(args[0], "help")) {
		doHelp();
	} else if (!strcmp(args[0], "exit")) {
		doExit();
	} else if (!strcmp(args[0], "info")) {
		doInfo();
	} else if (!strcmp(args[0], "debug")) {
		doDebug();
	} else if (!strcmp(args[0], "record")) {
		doRecord();
	} else if (!strcmp(args[0], "set")) {
		doSet();
	} else if (!strcmp(args[0], "device")) {
		doDevice();
	} else if(!strcmp(args[0], "mode")) {
		if (!device.flagRun) {
			platform.flagMode = !platform.flagMode;
		}
	} else if (!strcmp(args[0], "start")) {
		machineStart();
	} else if (!strcmp(args[0], "reset")) {
		machineReset();
	} else if (!strcmp(args[0], "stop")) {
		machineStop();
	} else if (!strcmp(args[0], "resume")) {
		machineResume();
	} else {
		utilsPrint("Illegal command '%s'.\n",args[0]);
	}
	utilsPrint("\n");
}

/* Initializes console */
static void consoleInit() {
	args = (char **) malloc(CONSOLE_MAXNARG * sizeof(char *));
	flagExit = 0;
	machineInit();
#if GLOBAL_PLATFORM == GLOBAL_VAR_WIN32
	deviceConnectFloppyInsert("d:/fd.img");
#else
	deviceConnectFloppyInsert("/Users/xha/fd.img");
#endif
}

/* Finalizes console */
static void consoleFinal() {
	machineFinal();
	if (args) {
		free(args);
	}
}

/* Entry point of NXVM console */
void consoleMain() {
	consoleInit();
	utilsPrint("\nPlease enter 'HELP' for information.\n\n");
	while (!flagExit) {
		utilsPrint("Console> ");
		FGETS(strCmdBuff, MAXLINE, stdin);
		parse();
		execute();
	}
	consoleFinal();
}
