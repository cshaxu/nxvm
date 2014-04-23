/* This file is a part of NXVM project. */

#include "vmachine/vapi.h"
#include "vmachine/vmachine.h"
#include "vmachine/debug/debug.h"
#include "vmachine/debug/record.h"

#include "console.h"

#define CONSOLE_MAXNARG 256

static int narg;
static char **arg;
static t_bool   flagexit;
static t_string strCmdBuff, strCmdCopy;

static void parse()
{
	STRCPY(strCmdCopy,strCmdBuff);
	narg = 0;
	arg[narg] = STRTOK(strCmdCopy," \t\n\r\f");
	if(arg[narg]) {
		lcase(arg[narg]);
		narg++;
	} else return;
	while(narg < CONSOLE_MAXNARG) {
		arg[narg] = STRTOK(NULL," \t\n\r\f");
		if(arg[narg]) {
			lcase(arg[narg]);
			narg++;
		} else break;
	}
}

#define GetHelp if(1) {Help();return;} else
static void Help()
{
	if (strcmp(arg[0], "help")) {
		narg = 2;
		arg[1] = arg[0];
	}
	switch (narg) {
	case 2:
		if (!strcmp(arg[1], "help")) {
			vapiPrint("Show help info\n");
			vapiPrint("\nHELP\n");
			vapiPrint("  show menu of all commands\n");
			vapiPrint("\nHELP [command]\n");
			vapiPrint("  show help info for command\n");
			break;
		} else if (!strcmp(arg[1], "exit")) {
			vapiPrint("Quit the console\n");
			vapiPrint("\nEXIT\n");
			break;
		} else if (!strcmp(arg[1], "info")) {
			vapiPrint("List all NXVM info\n");
			vapiPrint("\nINFO\n");
			break;
		} else if (!strcmp(arg[1], "debug")) {
			vapiPrint("Launch NXVM hardware debugger\n");
			vapiPrint("\nDEBUG\n");
			break;
		} else if (!strcmp(arg[1], "debug32")) {
			vapiPrint("Launch NXVM 32-bit hardware debugger\n");
			vapiPrint("\nDEBUG\n");
			break;
		} else if (!strcmp(arg[1], "record")) {
			vapiPrint("Record cpu status in each iteration for futher dumping\n");
			vapiPrint("\nRECORD [on | off | dump <file> | now <file>]\n");
			break;
		} else if (!strcmp(arg[1], "set")) {
			vapiPrint("Change BIOS settings\n");
			vapiPrint("\nSET <item> <value>\n");
			vapiPrint("  available items and values\n");
			vapiPrint("  boot   fdd, hdd\n");
			break;
		} else if (!strcmp(arg[1], "device")) {
			vapiPrint("Change NXVM devices\n");
			vapiPrint("\nDEVICE ram <size>\n");
			vapiPrint("  change memory size (KB)\n");
			vapiPrint("\nDEVICE display [console | window]\n");
			vapiPrint("  change display type\n");
			vapiPrint("\nDEVICE fdd [create | insert <file>| remove <file>]\n");
			vapiPrint("  change floppy drive status:\n");
			vapiPrint("  create: discard current floppy image\n");
			vapiPrint("          and create a new one\n");
			vapiPrint("  insert: load floppy image from file\n");
			vapiPrint("  remove: remove floppy image and dump to file\n");
			vapiPrint("\nDEVICE hdd [create [cyl <num>]| connect <file>| disconnect <file>]\n");
			vapiPrint("  change hard disk drive status:\n");
			vapiPrint("  create:     discard current hard disk image\n");
			vapiPrint("              and create a new one of n cyls\n");
			vapiPrint("  connect:    load hard disk image from file\n");
			vapiPrint("  disconnect: remove hard disk image and dump to file\n");
			break;
		} else if (!strcmp(arg[1], "nxvm")) {
			vapiPrint("Change virtual machine status\n");
			vapiPrint("\nNXVM [start | reset | stop | continue]\n");
			vapiPrint("  start:    start virtual machine\n");
			vapiPrint("  reset:    reset all device and restart machine\n");
			vapiPrint("  stop:     stop virtual machine\n");
			vapiPrint("  resume:   resume virtual macine\n");
			break;
		} else ;
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
static void Exit()
{
	if (narg != 1) GetHelp;
	if (!vmachine.flagrun) {
		flagexit = 1;
	} else {
		vapiPrint("Please stop NXVM before exit.\n");
	}
}
static void Info()
{
	if (narg != 1) GetHelp;
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
	vapiPrint("Display Type:      ");
	if (vmachine.flagmode) vapiPrint("Window\n");
	else vapiPrint("Console\n");
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
	if (vdebug.flagbreak) vapiPrint("%04X:%04X\n",vdebug.breakcs,vdebug.breakip);
	else vapiPrint("Off\n");
	vapiPrint("\n");
	vapiPrint("NXVM Running Status\n");
	vapiPrint("===================\n");
	vapiPrint("Running: %s\n", vmachine.flagrun  ? "Yes" : "No");
}
static void Debug()
{
	if (narg != 1) GetHelp;
	debug();
}
static void Record()
{
	if (narg < 2) GetHelp;
	if (vmachine.flagrun) {
		vapiPrint("Cannot change record status or dump record now.\n");
		return;
	}
	if (!strcmp(arg[1], "on")) {
		vrecord.flagrecord = 1;
		vrecord.flagnow = 0;
		if (vrecord.fp) fclose(vrecord.fp);
	} else if (!strcmp(arg[1], "off")) {
		vrecord.flagrecord = 0;
		vrecord.flagnow = 0;
		if (vrecord.fp) fclose(vrecord.fp);
	} else if (!strcmp(arg[1], "dump")) {
		if (narg < 3) GetHelp;
		recordDump(arg[2]);
		vrecord.flagnow = 0;
		vrecord.flagrecord = 0;
		if (vrecord.fp) fclose(vrecord.fp);
	} else if (!strcmp(arg[1], "now")) {
		if (narg < 3) GetHelp;
		vrecord.flagrecord = 1;
		vrecord.flagnow = 1;
		recordNow(arg[2]);
	} else GetHelp;
}
static void Set()
{
	if (narg < 2) GetHelp;
	if (!strcmp(arg[1], "boot")) {
		if (narg != 3) GetHelp;
		if (!strcmp(arg[2], "fdd"))
			vmachine.flagboot = 0;
		else if(!strcmp(arg[2], "hdd"))
			vmachine.flagboot = 1;
		else GetHelp;
	} else GetHelp;
}
static void Device()
{
	if (narg < 2) GetHelp;
	if (vmachine.flagrun) {
		vapiPrint("Cannot change device now.\n");
		return;
	}
	if (!strcmp(arg[1], "ram")) {
		if (narg != 3) GetHelp;
		vramAlloc(atoi(arg[2]) << 10);
	} else if(!strcmp(arg[1], "display")) {
		if (narg != 3) GetHelp;
		if (!strcmp(arg[2], "console"))
			vmachine.flagmode = 0;
		else if (!strcmp(arg[2], "window"))
			vmachine.flagmode = 1;
		else GetHelp;
	} else if(!strcmp(arg[1], "fdd")) {
		if (narg < 3) GetHelp;
		if (!strcmp(arg[2], "create")) {
			vfdd.flagexist = 1;
		} else if (!strcmp(arg[2], "insert")) {
			if (narg < 4) GetHelp;
			if (vapiFloppyInsert(arg[3]))
				vapiPrint("Floppy disk inserted.\n");
			else
				vapiPrint("Cannot read floppy disk from '%s'.\n", arg[3]);
		} else if (!strcmp(arg[2], "remove")) {
			if (narg < 4) arg[3] = NULL;
			if (vapiFloppyRemove(arg[3]))
				vapiPrint("Floppy disk removed.\n");
			else
				vapiPrint("Cannot write floppy disk to '%s'.\n", arg[3]);
		} else GetHelp;
	} else if(!strcmp(arg[1], "hdd")) {
		if (narg < 3) GetHelp;
		if (!strcmp(arg[2], "create")) {
			vhdd.flagexist = 1;
			if (narg > 3) {
				if (narg == 5 && !strcmp(arg[3], "cyl")) {
					vhdd.ncyl = atoi(arg[4]);
				} else GetHelp;
			}
			vhddAlloc();
		} else if (!strcmp(arg[2], "connect")) {
			if (narg < 4) GetHelp;
			if (vapiHardDiskInsert(arg[3]))
				vapiPrint("Hard disk connected.\n");
			else
				vapiPrint("Cannot read hard disk from '%s'.\n", arg[3]);
		} else if (!strcmp(arg[2], "disconnect")) {
			if (narg < 4) arg[3] = NULL;
			if (vapiHardDiskRemove(arg[3]))
				vapiPrint("Hard disk disconnected.\n");
			else
				vapiPrint("Cannot write hard disk from '%s'.\n", arg[3]);
		} else GetHelp;
	} else GetHelp;
}
static void Nxvm()
{
	if (narg != 2) GetHelp;
	if (!strcmp(arg[1], "start")) {
		if (!vmachine.flagrun) {
			vmachineStart();
		} else
			vapiPrint("Virtual machine is already running.\n");
	} else if (!strcmp(arg[1], "reset")) {
		vapiCallBackMachineReset();
	} else if (!strcmp(arg[1], "stop")) {
		vmachineStop();
	} else if (!strcmp(arg[1], "resume")) {
		if (!vmachine.flagrun)
			vmachineResume();
		else
			vapiPrint("Virtual machine is already running.\n");
	} else GetHelp;
}

static void Test()
{
	vmachine.flagboot = 1;
	vmachine.flagmode = 1;
	vmachineReset();
	debug();
}

static void exec()
{
	if (!arg[0] || !strlen(arg[0])) return;
	else if(!strcmp(arg[0],"test"))    Test();
	else if(!strcmp(arg[0],"help"))    Help();
	else if(!strcmp(arg[0],"exit"))    Exit();
	else if(!strcmp(arg[0],"info"))    Info();
	else if(!strcmp(arg[0],"debug"))   Debug();
	else if(!strcmp(arg[0],"record"))  Record();
	else if(!strcmp(arg[0],"set"))     Set();
	else if(!strcmp(arg[0],"device"))  Device();
	else if(!strcmp(arg[0],"nxvm"))    Nxvm();
	/* support for convenient commands */
	else if(!strcmp(arg[0],"mode"))  {if (!vmachine.flagrun) vmachine.flagmode = !vmachine.flagmode;}
	else if(!strcmp(arg[0],"start"))  vmachineStart();
	else if(!strcmp(arg[0],"reset"))  vapiCallBackMachineReset();
	else if(!strcmp(arg[0],"stop"))   vmachineStop();
	else if(!strcmp(arg[0],"resume")) vmachineResume();
	else vapiPrint("Illegal command '%s'.\n",arg[0]);
	vapiPrint("\n");
}
static void init()
{
	arg = (char **)malloc(CONSOLE_MAXNARG * sizeof(char *));
	flagexit = 0;
	vmachineInit();
	if (!vfdd.flagexist) vapiFloppyInsert("fd.img");
	if (!vhdd.flagexist) vapiHardDiskInsert("hd.img");
#if VGLOBAL_PLATFORM == VGLOBAL_VAR_WIN32
	if (!vfdd.flagexist) vapiFloppyInsert("d:/fd.img");
	if (!vhdd.flagexist) vapiHardDiskInsert("d:/hd.img");
#else
	if (!vfdd.flagexist) vapiFloppyInsert("/mnt/hgfs/D/fd.img");
	if (!vhdd.flagexist) vapiHardDiskInsert("/mnt/hgfs/D/hd.img");
	if (!vfdd.flagexist) vapiFloppyInsert("~/fd.img");
	if (!vhdd.flagexist) vapiHardDiskInsert("~/hd.img");
#endif
}
static void final()
{
	vmachineFinal();
	if (arg) free(arg);
}

void console()
{
	init();
	vapiPrint("\nPlease enter 'HELP' for information.\n\n");
	while(!flagexit) {
		vapiPrint("Console> ");
		FGETS(strCmdBuff,MAXLINE,stdin);
		parse();
		exec();
	}
	final();
}
