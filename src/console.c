/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"

#include "vmachine/vapi.h"
#include "vmachine/vmachine.h"
#include "vmachine/debug/debug.h"
#include "vmachine/debug/record.h"

#include "console.h"

#define MAXLINE 256
#define MAXNARG 256
#define MAXNASMARG 4

static int narg;
static char **arg;
static int exitFlag;
static char cmdBuff[MAXLINE];
static char cmdCopy[MAXLINE];
static t_nubit32 printc(const t_string format, ...)
{
	t_nubit32 nWrittenBytes = 0;
	va_list arg_ptr;
	va_start(arg_ptr, format);
	nWrittenBytes = vfprintf(stdout,format,arg_ptr);
	//nWrittenBytes = vsprintf(stringBuffer,format,arg_ptr);
	va_end(arg_ptr);
	fflush(stdout);
	return nWrittenBytes;
}
static void cgets(char *_Buf, int _MaxCount, FILE *_File)
{
	FGETS(_Buf,_MaxCount,_File);
	fflush(stdin);
}
static void lcase(char *s)
{
	int i = 0;
	if(s[0] == '\'') return;
	while(s[i] != '\0') {
		if(s[i] == '\n') s[i] = '\0';
		else if(s[i] > 0x40 && s[i] < 0x5b)
			s[i] += 0x20;
		i++;
	}
}
static void parse()
{
	STRCPY(cmdCopy,cmdBuff);
	narg = 0;
	arg[narg] = STRTOK(cmdCopy," \t\n\r\f");
	if(arg[narg]) {
		lcase(arg[narg]);
		narg++;
	} else return;
	while(narg < MAXNARG) {
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
			printc("Show help info\n");
			printc("\nHELP\n");
			printc("  show menu of all commands\n");
			printc("\nHELP [command]\n");
			printc("  show help info for command\n");
			break;
		} else if (!strcmp(arg[1], "exit")) {
			printc("Quit the console\n");
			printc("\nEXIT\n");
			break;
		} else if (!strcmp(arg[1], "info")) {
			printc("List all NXVM info\n");
			printc("\nINFO\n");
			break;
		} else if (!strcmp(arg[1], "debug")) {
			printc("Launch NXVM hardware debugger\n");
			printc("\nDEBUG\n");
			break;
		} else if (!strcmp(arg[1], "debug32")) {
			printc("Launch NXVM 32-bit hardware debugger\n");
			printc("\nDEBUG\n");
			break;
		} else if (!strcmp(arg[1], "record")) {
			printc("Record cpu status in each iteration for futher dumping\n");
			printc("\nRECORD [on | off | dump <file> | now <file>]\n");
			break;
		} else if (!strcmp(arg[1], "set")) {
			printc("Change BIOS settings\n");
			printc("\nSET <item> <value>\n");
			printc("  available items and values\n");
			printc("  boot   fdd, hdd\n");
			break;
		} else if (!strcmp(arg[1], "device")) {
			printc("Change NXVM devices\n");
			printc("\nDEVICE ram <size>\n");
			printc("  change memory size (KB)\n");
			printc("\nDEVICE display [console | window]\n");
			printc("  change display type\n");
			printc("\nDEVICE fdd [create | insert <file>| remove <file>]\n");
			printc("  change floppy drive status:\n");
			printc("  create: discard current floppy image\n");
			printc("          and create a new one\n");
			printc("  insert: load floppy image from file\n");
			printc("  remove: remove floppy image and dump to file\n");
			printc("\nDEVICE hdd [create [cyl <num>]| connect <file>| disconnect <file>]\n");
			printc("  change hard disk drive status:\n");
			printc("  create:     discard current hard disk image\n");
			printc("              and create a new one of n cyls\n");
			printc("  connect:    load hard disk image from file\n");
			printc("  disconnect: remove hard disk image and dump to file\n");
			break;
		} else if (!strcmp(arg[1], "nxvm")) {
			printc("Change virtual machine status\n");
			printc("\nNXVM [start | reset | stop | continue]\n");
			printc("  start:    start virtual machine\n");
			printc("  reset:    reset all device and restart machine\n");
			printc("  stop:     stop virtual machine\n");
			printc("  resume:   resume virtual macine\n");
			break;
		} else ;
	case 1:
	default:
		printc("NXVM Console Commands\n");
		printc("=====================\n");
		printc("HELP    Show help info\n");
		printc("EXIT    Quit the console\n");
		printc("INFO    List all NXVM info\n");
		printc("\n");
		printc("DEBUG   Launch NXVM hardware debugger\n");
		printc("RECORD  Record cpu status for each instruction\n");
		printc("\n");
		printc("SET     Change BIOS settings\n");
		printc("DEVICE  Change hardware parts\n");
		printc("NXVM    Change virtual machine status\n");
		printc("\n");
		printc("For command usage, type 'HELP <command>'.\n");
		break;
	}
}
static void Exit()
{
	if (narg != 1) GetHelp;
	if (!vmachine.flagrun) {
		exitFlag = 1;
	} else {
		printc("Please stop NXVM before exit.\n");
	}
}
static void Info()
{
	if (narg != 1) GetHelp;
	printc("NXVM Device Info\n");
	printc("================\n");
	printc("Machine:           IBM PC/AT\n");
	printc("CPU:               Intel 8086+\n");
	printc("RAM Size:          %d MB\n", vram.size >> 20);
	printc("Floppy Disk Drive: 3.5\", %.2f MB, %s\n",
		vfddGetImageSize * 1. / 0xfa000,
		vfdd.flagexist ? "inserted" : "not inserted");
	printc("Hard Disk Drive:   %d cylinders, %.2f MB, %s\n",
		vhdd.ncyl, vhddGetImageSize * 1. / 0x100000,
		vhdd.flagexist ? "connected" : "disconnected");
	printc("Display Type:      ");
	if (vmachine.flagmode) printc("Window\n");
	else printc("Console\n");
	printc("\n");
	printc("NXVM BIOS Settings\n");
	printc("==================\n");
	printc("Boot Disk: %s\n", vmachine.flagboot ? "Hard Drive" : "Floppy");
	printc("\n");
	printc("NXVM Debug Status\n");
	printc("=================\n");
	printc("Recorder:    %s\n", vmachine.flagrecord ? "On" : "Off");
	printc("Trace:       %s\n", vmachine.tracecnt ? "On" : "Off");
	printc("Break Point: ");
	if (vmachine.flagbreak) printc("%04X:%04X\n",vmachine.breakcs,vmachine.breakip);
	else printc("Off\n");
	printc("\n");
	printc("NXVM Running Status\n");
	printc("===================\n");
	printc("Running: %s\n", vmachine.flagrun  ? "Yes" : "No");
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
		printc("Cannot change record status or dump record now.\n");
		return;
	}
	if (!strcmp(arg[1], "on")) {
		vmachine.flagrecord = 1;
		vrecord.flagnow = 0;
		if (vrecord.fp) fclose(vrecord.fp);
	} else if (!strcmp(arg[1], "off")) {
		vmachine.flagrecord = 0;
		vrecord.flagnow = 0;
		if (vrecord.fp) fclose(vrecord.fp);
	} else if (!strcmp(arg[1], "dump")) {
		if (narg < 3) GetHelp;
		recordDump(arg[2]);
		vrecord.flagnow = 0;
		vmachine.flagrecord = 0;
		if (vrecord.fp) fclose(vrecord.fp);
	} else if (!strcmp(arg[1], "now")) {
		if (narg < 3) GetHelp;
		vmachine.flagrecord = 1;
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
			vmachine.flagboot = 0x00;
		else if(!strcmp(arg[2], "hdd"))
			vmachine.flagboot = 0x01;
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
			vmachine.flagmode = 0x00;
		else if (!strcmp(arg[2], "window"))
			vmachine.flagmode = 0x01;
		else GetHelp;
	} else if(!strcmp(arg[1], "fdd")) {
		if (narg < 3) GetHelp;
		if (!strcmp(arg[2], "create")) {
			vfdd.flagexist = 0x01;
		} else if (!strcmp(arg[2], "insert")) {
			if (narg < 4) GetHelp;
			vapiFloppyInsert(arg[3]);
		} else if (!strcmp(arg[2], "remove")) {
			if (narg < 4) arg[3] = NULL;
			vapiFloppyRemove(arg[3]);
		} else GetHelp;
	} else if(!strcmp(arg[1], "hdd")) {
		if (narg < 3) GetHelp;
		if (!strcmp(arg[2], "create")) {
			vhdd.flagexist = 0x01;
			if (narg > 3) {
				if (narg == 5 && !strcmp(arg[3], "cyl")) {
					vhdd.ncyl = atoi(arg[4]);
				} else GetHelp;
			}
			vhddAlloc();
		} else if (!strcmp(arg[2], "connect")) {
			if (narg < 4) GetHelp;
			vapiHardDiskInsert(arg[3]);
		} else if (!strcmp(arg[2], "disconnect")) {
			if (narg < 4) arg[3] = NULL;
			vapiHardDiskRemove(arg[3]);
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
			printc("Virtual machine is already running.\n");
	} else if (!strcmp(arg[1], "reset")) {
		vapiCallBackMachineReset();
	} else if (!strcmp(arg[1], "stop")) {
		vmachineStop();
	} else if (!strcmp(arg[1], "resume")) {
		if (!vmachine.flagrun)
			vmachineResume();
		else
			printc("Virtual machine is already running.\n");
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
	else printc("Illegal command '%s'.\n",arg[0]);
	printc("\n");
}
static void init()
{
	arg = (char **)malloc(MAXNARG * sizeof(char *));
	exitFlag = 0x00;
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
	printc("Please enter 'HELP' for information.\n");
	while(!exitFlag) {
		printc("Console> ");
		cgets(cmdBuff,MAXLINE,stdin);
		parse();
		exec();
	}
	final();
}
