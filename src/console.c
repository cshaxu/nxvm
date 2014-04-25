/* Copyright 2012-2014 Neko. */

/*
 * NXVM console provides a command-line interface for users
 * to configure, debug and run the virtual machine.
 */

#include "vmachine/vapi.h"

#include "vmachine/vram.h"
#include "vmachine/vcpu.h"
#include "vmachine/vfdd.h"
#include "vmachine/vhdd.h"

#include "vmachine/vmachine.h"

#include "vmachine/debug/debug.h"
#include "vmachine/debug/record.h"

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
	lcase(args[narg++]);
	while(narg < CONSOLE_MAXNARG) {
		args[narg] = STRTOK(NULL, " \t\n\r\f");
		if (args[narg]) {
			lcase(args[narg++]);
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
			vapiPrint("Show help info\n");
			vapiPrint("\nHELP\n");
			vapiPrint("  show menu of all commands\n");
			vapiPrint("\nHELP <command>\n");
			vapiPrint("  show help info for command\n");
			break;
		} else if (!strcmp(args[1], "exit")) {
			vapiPrint("Quit the console\n");
			vapiPrint("\nEXIT\n");
			break;
		} else if (!strcmp(args[1], "info")) {
			vapiPrint("List virtual machine status\n");
			vapiPrint("\nINFO\n");
			break;
		} else if (!strcmp(args[1], "debug")) {
			vapiPrint("Launch NXVM hardware debugger\n");
			vapiPrint("\nDEBUG\n");
			break;
		} else if (!strcmp(args[1], "debug32")) {
			vapiPrint("Launch NXVM 32-bit hardware debugger\n");
			vapiPrint("\nDEBUG\n");
			break;
		} else if (!strcmp(args[1], "record")) {
			vapiPrint("Record cpu status in each iteration for futher dumping\n");
			vapiPrint("\nRECORD on | off | (dump <file>) | (now <file>)\n");
			break;
		} else if (!strcmp(args[1], "set")) {
			vapiPrint("Change BIOS settings\n");
			vapiPrint("\nSET <item> <value>\n");
			vapiPrint("  available items and values\n");
			vapiPrint("  boot   fdd, hdd\n");
			break;
		} else if (!strcmp(args[1], "device")) {
			vapiPrint("Change NXVM devices\n");
			vapiPrint("\nDEVICE ram <size>\n");
			vapiPrint("  change memory size (KB)\n");
			vapiPrint("\nDEVICE display console | window\n");
			vapiPrint("  change display type\n");
			vapiPrint("\nDEVICE fdd create | (insert <file>) | (remove <file>)\n");
			vapiPrint("  change floppy drive status:\n");
			vapiPrint("  create: discard current floppy image\n");
			vapiPrint("          and create a new one\n");
			vapiPrint("  insert: load floppy image from file\n");
			vapiPrint("  remove: remove floppy image and dump to file\n");
			vapiPrint("\nDEVICE hdd (create [cyl <num>]) | (connect <file>) | (disconnect <file>)\n");
			vapiPrint("  change hard disk drive status:\n");
			vapiPrint("  create:     discard current hard disk image\n");
			vapiPrint("              and create a new one of n cyls\n");
			vapiPrint("  connect:    load hard disk image from file\n");
			vapiPrint("  disconnect: remove hard disk image and dump to file\n");
			break;
		} else if (!strcmp(args[1], "nxvm")) {
			vapiPrint("Change virtual machine status\n");
			vapiPrint("\nNXVM start | reset | stop | resume\n");
			vapiPrint("  start:    start virtual machine\n");
			vapiPrint("  reset:    reset all device and restart machine\n");
			vapiPrint("  stop:     stop virtual machine\n");
			vapiPrint("  resume:   resume virtual macine\n");
			break;
		}
	case 1:
	default:
		vapiPrint("NXVM Console Commands\n");
		vapiPrint("=====================\n");
		vapiPrint("HELP    Show help info\n");
		vapiPrint("EXIT    Quit the console\n");
		vapiPrint("INFO    List all NXVM info\n");
		vapiPrint("\n");
		vapiPrint("DEBUG   Launch NXVM hardware debugger\n");
		vapiPrint("RECORD  Record cpu status for each instruction\n");
		vapiPrint("\n");
		vapiPrint("SET     Change BIOS settings\n");
		vapiPrint("DEVICE  Change hardware parts\n");
		vapiPrint("NXVM    Change virtual machine status\n");
		vapiPrint("\n");
		vapiPrint("For command usage, type 'HELP <command>'.\n");
		break;
	}
}

/* Quits NXVM. */
static void doExit() {
	if (narg != 1) {
		GetHelp;
	}
	if (!vmachine.flagrun) {
		flagExit = 1;
	} else {
		vapiPrint("Please stop NXVM before exit.\n");
	}
}

/* Prints virtual machine status */
static void doInfo() {
	if (narg != 1) {
		GetHelp;
	}
	vapiPrint("NXVM Device Info\n");
	vapiPrint("================\n");
	vapiPrint("Machine:           %s\n", NXVM_DEVICE_MACHINE);
	vapiPrint("CPU:               %s\n", NXVM_DEVICE_CPU);
	vapiPrint("RAM Size:          %d MB\n", vram.size >> 20);
	vapiPrint("Floppy Disk Drive: %s, %.2f MB, %s\n", NXVM_DEVICE_FDD,
		vfddGetImageSize * 1. / 0xfa000,
		vfdd.flagexist ? "inserted" : "not inserted");
	vapiPrint("Hard Disk Drive:   %s, %.2f MB, %s\n", NXVM_DEVICE_HDD,
		vhddGetImageSize * 1. / 0x100000,
		vhdd.flagexist ? "connected" : "disconnected");
	vapiPrint("Display Type:      %s\n", vmachine.flagmode ? "Window" : "Console");
	vapiPrint("\n");
	vapiPrint("NXVM BIOS Settings\n");
	vapiPrint("==================\n");
	vapiPrint("Boot Disk: %s\n", vmachine.flagboot ? "Hard Drive" : "Floppy");
	vapiPrint("\n");
	vapiPrint("NXVM Debug Status\n");
	vapiPrint("=================\n");
	vapiPrint("Recorder:    %s\n", vrecord.flagrecord ? (vrecord.flagnow ? "Now" : "On") : "Off");
	vapiPrint("Trace:       %s\n", vdebug.tracecnt ? "On" : "Off");
	vapiPrint("Break Point: ");
	if (vdebug.flagbreak) {
		vapiPrint("%04X:%04X\n", vdebug.breakcs,vdebug.breakip);
	} else if (vdebug.flagbreakx) {
		vapiPrint("L%08X\n", vdebug.breaklinear);
	} else {
		vapiPrint("Off\n");
	}
	vapiPrint("\n");
	vapiPrint("NXVM Running Status\n");
	vapiPrint("===================\n");
	vapiPrint("Running: %s\n", vmachine.flagrun  ? "Yes" : "No");
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
	if (vmachine.flagrun) {
		vapiPrint("Cannot change record status or dump record now.\n");
		return;
	}
	if (!strcmp(args[1], "on")) {
		vrecord.flagrecord = 1;
		vrecord.flagnow = 0;
		if (vrecord.fp) {
			fclose(vrecord.fp);
		}
	} else if (!strcmp(args[1], "off")) {
		vrecord.flagrecord = 0;
		vrecord.flagnow = 0;
		if (vrecord.fp) {
			fclose(vrecord.fp);
		}
	} else if (!strcmp(args[1], "dump")) {
		if (narg < 3) {
			GetHelp;
		}
		recordDump(args[2]);
		vrecord.flagnow = 0;
		vrecord.flagrecord = 0;
		if (vrecord.fp) {
			fclose(vrecord.fp);
		}
	} else if (!strcmp(args[1], "now")) {
		if (narg < 3) {
			GetHelp;
		}
		vrecord.flagrecord = 1;
		vrecord.flagnow = 1;
		recordNow(args[2]);
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
			vmachine.flagboot = 0;
		} else if(!strcmp(args[2], "hdd")) {
			vmachine.flagboot = 1;
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
	if (vmachine.flagrun) {
		vapiPrint("Cannot change device now.\n");
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
			vmachine.flagmode = 0;
		} else if (!strcmp(args[2], "window")) {
			vmachine.flagmode = 1;
		} else {
			GetHelp;
		}
	} else if(!strcmp(args[1], "fdd")) {
		if (narg < 3) {
			GetHelp;
		}
		if (!strcmp(args[2], "create")) {
			vfdd.flagexist = 1;
			vapiPrint("Floppy disk created.\n");
		} else if (!strcmp(args[2], "insert")) {
			if (narg < 4) {
				GetHelp;
			}
			if (!vapiFloppyInsert(args[3])) {
				vapiPrint("Floppy disk inserted.\n");
			} else {
				vapiPrint("Cannot read floppy disk from '%s'.\n", args[3]);
			}
		} else if (!strcmp(args[2], "remove")) {
			if (narg < 4) {
				args[3] = NULL;
			}
			if (!vapiFloppyRemove(args[3])) {
				vapiPrint("Floppy disk removed.\n");
			} else {
				vapiPrint("Cannot write floppy disk to '%s'.\n", args[3]);
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
			vapiPrint("Hard disk created.\n");
		} else if (!strcmp(args[2], "connect")) {
			if (narg < 4) {
				GetHelp;
			}
			if (!vapiHardDiskInsert(args[3])) {
				vapiPrint("Hard disk connected.\n");
			} else {
				vapiPrint("Cannot read hard disk from '%s'.\n", args[3]);
			}
		} else if (!strcmp(args[2], "disconnect")) {
			if (narg < 4) {
				args[3] = NULL;
			}
			if (!vapiHardDiskRemove(args[3])) {
				vapiPrint("Hard disk disconnected.\n");
			} else {
				vapiPrint("Cannot write hard disk to '%s'.\n", args[3]);
			}
		} else {
			GetHelp;
		}
	} else {
		GetHelp;
	}
}

/* Controls NXVM Running status */
static void doNxvm() {
	if (narg != 2) {
		GetHelp;
	}
	if (!strcmp(args[1], "start")) {
		if (!vmachine.flagrun) {
			vmachineStart();
		} else {
			vapiPrint("Virtual machine is already running.\n");
		}
	} else if (!strcmp(args[1], "reset")) {
		vmachineReset();
	} else if (!strcmp(args[1], "stop")) {
		vmachineStop();
	} else if (!strcmp(args[1], "resume")) {
		if (!vmachine.flagrun) {
			vmachineResume();
		} else {
			vapiPrint("Virtual machine is already running.\n");
		}
	} else {
		GetHelp;
	}
}

/* Tests NXVM: reset and start debugger */
static void doTest() {
	vmachine.flagboot = 1;
	vmachine.flagmode = 1;
	vmachineReset();
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
	} else if (!strcmp(args[0], "nxvm")) {
		doNxvm();
	/* support for convenient commands */
	} else if(!strcmp(args[0], "mode")) {
		if (!vmachine.flagrun) {
			vmachine.flagmode = !vmachine.flagmode;
		}
	} else if (!strcmp(args[0], "start")) {
		vmachineStart();
	} else if (!strcmp(args[0], "reset")) {
		vmachineReset();
	} else if (!strcmp(args[0], "stop")) {
		vmachineStop();
	} else if (!strcmp(args[0], "resume")) {
		vmachineResume();
	} else {
		vapiPrint("Illegal command '%s'.\n",args[0]);
	}
	vapiPrint("\n");
}

/* Initialize console */
static void consoleInit() {
	args = (char **) malloc(CONSOLE_MAXNARG * sizeof(char *));
	flagExit = 0;
	vmachineInit();
	vapiFloppyInsert("d:/fd.img");
}

/* Finalize console */
static void consoleFinal() {
	vmachineFinal();
	if (args) {
		free(args);
	}
}

/* Entry point of NXVM console */
void consoleMain() {
	consoleInit();
	vapiPrint("\nPlease enter 'HELP' for information.\n\n");
	while (!flagExit) {
		vapiPrint("Console> ");
		FGETS(strCmdBuff, MAXLINE, stdin);
		parse();
		execute();
	}
	consoleFinal();
}
