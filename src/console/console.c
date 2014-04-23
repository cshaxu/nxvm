/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"

#include "../vmachine/vapi.h"
#include "../vmachine/vmachine.h"
#include "../vmachine/debug/debug.h"

#include "console.h"

///*void Exec()
//{
//	char execmd[MAXLINE];
//	FILE *load;
//	t_nubit8 c;
//	t_nubit16 i = 0,end;
//	t_nubit32 len = 0;
//	if (!vmachine.flaginit || vmachine.flagrun) {
//		printc("Cannot execute binary file now.\n");
//		return;
//	}
//	printc(".COM File: ");
//	FGETS(execmd,MAXLINE,stdin);
//	parse(execmd);
//	if(!strlen(execmd)) return;
//	load = FOPEN(execmd,"rb");
//	if(!load) {
//		printc("File not found\n");
//		return;
//	} else {
//		vcpu.ax = vcpu.bx = vcpu.cx = vcpu.dx = 0x0000;
//		vcpu.si = vcpu.di = vcpu.bp = 0x0000;
//		vcpu.sp = 0xffee;	vcpu.ip = 0x0100;
//		vcpu.ds = vcpu.es = vcpu.ss = vcpu.cs = 0x0001;
//		vmachine.flagrun = 1;
//		c = fgetc(load);
//		while(!feof(load)) {
//			vramVarByte(vcpu.cs+i,vcpu.ip+((len++)%0x10000)) = c;
//			i = len / 0x10000;
//			c = fgetc(load);
//		}
//		end = vcpu.ip+len;
//		fclose(load);
//		while(vcpu.ip < end && vmachine.flagrun) vmachineRefresh();
//	}
//}*/
////#define _rec (vapirecord.rec[(i + vapirecord.start) % VAPI_RECORD_SIZE])
//void Dump()
//{
//	t_nubitcc i = 0;
////	Operand x;
////	FILE *fp;
//	char str[MAXLINE];
//	printc("Dump File: ");
//	FGETS(str, MAXLINE, stdin);
//	parse(str);
//	vmachineDumpRecordFile(str);
//	/*printc("Instruction Dump File: ");
//	FGETS(str, MAXLINE, stdin);
//	parse(str);
//	fp = FOPEN(str,"w");
//	if (!fp) {
//		printc("ERORR:\tfailed to dump instruction.\n");
//		return;
//	}
//	while (i < vapirecord.size) {
//		disassemble(str, &x, (void *)vram.base, _rec.cs, _rec.ip);
//		fprintf(fp,"%04X:%04X\t%s\n",_rec.cs,_rec.ip,str);
//		_rec.cs;
//		++i;
//	}
//	fclose(fp);*/
//}
//void Memory()
//{
//	unsigned int i;
//	unsigned int tempSize;
//	unsigned char testFlag = 0;
//	char str[MAXLINE];
//	if (!vmachine.flaginit || vmachine.flagrun) {
//		printc("Cannot modify memory size now.\n");
//		return;
//	}
//	fflush(stdin);
//	printc("Size(KB): ");
//	FGETS(str,MAXLINE,stdin);
//	tempSize = atoi(str);
//	if(tempSize > 0x400) {
//		vramAlloc(tempSize<<0x0a);
//	} else {
//		vramAlloc(1024<<0x0a);
//	}
//	for(i = 0;i < vram.size;++i)
//	{
//		if(i % 1024 == 0) printc("\rMemory Testing : %dK",i/1024);
//		if(d_nubit8(vram.base+i) != 0) {
//			printc("\nMemory test failed.\n");
//			testFlag = 1;
//			break;
//		}
//	}
//	if(!testFlag) printc("\rMemory Testing : %dK OK\n",i/1024);
//	return;
//
//}


#define MAXLINE 256
#define MAXNARG 256
#define MAXNASMARG 4

static int narg;
static char **arg;
static int exitFlag;
static char cmdBuff[MAXLINE];
static char cmdCopy[MAXLINE];
static char filename[MAXLINE];
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
			printc("\n");
			printc("HELP\n");
			printc("  show menu of all commands\n");
			printc("HELP [command]\n");
			printc("  show help info for command\n");
			break;
		} else if (!strcmp(arg[1], "exit")) {
			printc("Quit the console\n");
			printc("\n");
			printc("EXIT\n");
			break;
		} else if (!strcmp(arg[1], "info")) {
			printc("List all NXVM info\n");
			printc("\n");
			printc("INFO\n");
			break;
		} else if (!strcmp(arg[1], "debug")) {
			printc("Launch NXVM hardware debugger\n");
			printc("\n");
			printc("DEBUG\n");
			break;
		} else if (!strcmp(arg[1], "record")) {
			printc("Record cpu status in each iteration for futher dumping\n");
			printc("\n");
			printc("RECORD [on | off | dump <file>]\n");
			break;
		} else if (!strcmp(arg[1], "set")) {
			printc("Change BIOS settings\n");
			printc("\n");
			printc("SET <item> <value>\n");
			printc("  available items and values\n");
			printc("  bootdisk   fdd, hdd\n");
			break;
		} else if (!strcmp(arg[1], "device")) {
			printc("Change nxvm devices\n");
			printc("\n");
			printc("DEVICE memory <size>\n");
			printc("  change memory size (KB)\n");
			printc("DEVICE display [console | window]\n");
			printc("  change display type\n");
			printc("DEVICE fdd [create | insert <file>| remove <file>]");
			printc("  change floppy drive status:\n");
			printc("  create: discard previous floppy image and create a new one\n");
			printc("  insert: load floppy image from file\n");
			printc("  remove: remove floppy image and dump to file\n");
			printc("DEVICE hdd [create | connect <file>| disconnect <file>]\n");
			printc("  change hard disk drive status:\n");
			printc("  create:     discard previous hard disk image and create a new one\n");
			printc("  connect:    load hard disk image from file\n");
			printc("  disconnect: remove hard disk image and dump to file\n");
			break;
		} else if (!strcmp(arg[1], "nxvm")) {
			printc("Change virtual machine status\n");
			printc("\n");
			printc("NXVM [start | reset | stop | continue]\n");
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
/*		printc("RUN     Execute a binary file\n");*/
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
	printc("IBM PC/AT");
	printc("CPU: Intel 8086, 16-bit\n");
	printc("RAM Size:          %d KB\n", vram.size >> 10);
	printc("Floppy Disk Drive: 3.5\", 1.44 MB, %s\n",
		vfdd.flagexist ? "inserted" : "not inserted");
	printc("Hard Disk Drive:   %d cylinders, 10 MB, %s\n",
		vhdd.ncyl, vhdd.flagexist ? "connected" : "disconnected");
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
	printc("Trace:       %s\n", vmachine.flagtrace ? "On" : "Off");
	printc("Break Point: ");
	if (vmachine.flagtrace) printc("%04X:%04X\n",vmachine.breakcs,vmachine.breakip);
	else printc("Off\n");
	printc("\n");
	printc("NXVM Running Status\n");
	printc("===================\n");
	printc("Running: %s\n",	vmachine.flagrun  ? "Yes" : "No");
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
		vmachine.flagrecord = 0x01;
		printc("Recorder turned on.\n");
	} else if (!strcmp(arg[1], "off")) {
		vmachine.flagrecord = 0x00;
		printc("Recorder turned off.\n");
	} else if (!strcmp(arg[1], "dump")) {
		if (narg < 3) GetHelp;
		vapiRecordDump(arg[2]);
	} else GetHelp;
}
static void Set()
{
	if (narg < 2) GetHelp;
	if (!strcmp(arg[1], "bootdisk")) {
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
		vramAlloc(atoi(arg[2]));
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
		vmachineReset();
	} else if (!strcmp(arg[1], "stop")) {
		vmachineStop();
	} else if (!strcmp(arg[1], "resume")) {
		if (!vmachine.flagrun)
			vmachineResume();
		else
			printc("Virtual machine is already running.\n");
	} else GetHelp;
}

#include "../vmachine/debug/aasm.h"
static void Test()
{
	vmachine.flagmode = 1;
	vmachineStart();
/*	if (aasm("add [bp+si-81],7f\n"
		"add [bp+si-7f],80\n"
		"add [-ff99],81\n",
		0, 0)) printf("succ\n");
	else printf("fail\n");
	debug();*/
	//cs:\nss:\nds:\n mov ah,[1234]\n@label1:\n\nmov bx, [bp+si]\n\n\njmp @label1", 0, 0);
}

static void exec()
{
	if (!arg[0] || !strlen(arg[0])) return;
	else if(!strcmp(arg[0],"test"))   Test();
	else if(!strcmp(arg[0],"help"))   Help();
	else if(!strcmp(arg[0],"exit"))   Exit();
	else if(!strcmp(arg[0],"info"))   Info();
/*	else if(!strcmp(arg[0],"run"))    Run();*/
	else if(!strcmp(arg[0],"debug"))  Debug();
	else if(!strcmp(arg[0],"record")) Record();
	else if(!strcmp(arg[0],"set"))    Set();
	else if(!strcmp(arg[0],"device")) Device();
	else if(!strcmp(arg[0],"nxvm"))   Nxvm();
	/* support for old commands */
	else if(!strcmp(arg[0],"mode"))  {if (!vmachine.flagrun) vmachine.flagmode = !vmachine.flagmode;}
	else if(!strcmp(arg[0],"start"))  vmachineStart();
	else if(!strcmp(arg[0],"reset"))  vmachineReset();
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
#if VGLOBAL_PLATFORM == VGLOBAL_VAR_WIN32
	vapiFloppyInsert("d:/fd.img");
	vapiHardDiskInsert("d:/hd.img");
#else
	vapiFloppyInsert("fd.img");
	vapiHardDiskInsert("hd.img");
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
