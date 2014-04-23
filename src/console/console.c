/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "../vmachine/vmachine.h"
#include "../vmachine/vapi.h"

#include "asm86.h"
#include "debug.h"
#include "console.h"

#define MAXLINE 256
static int exitFlag;

static void parse(char *s)
{
	int i = 0;
	while(s[i] != '\0') {
		if(s[i] == '\n') {
			s[i] = '\0';
			break;
		}
		if(s[i] > 0x40 && s[i] < 0x5b)
			s[i] += 0x20;
		i++;
	}
}

#include "windows.h"

void Test()
{
	qdbiosInit();
}

void Help()
{
	fprintf(stdout,"NXVM Console Commands\n");
	fprintf(stdout,"=====================\n");
	fprintf(stdout,"HELP    Show this help menu\n");
	fprintf(stdout,"EXIT    Quit the console\n\n");

	fprintf(stdout,"DEBUG   Execute virtual debugger\n");
//	fprintf(stdout,"EXEC    Execute a binary file\n");
	fprintf(stdout,"RECORD  Record cpu status for each instruction\n");
	fprintf(stdout,"DUMP    Dump records into log file\n\n");
//	fprintf(stdout,"TRACE   Trace cpu status for each instruction\n");
	fprintf(stdout,"FLOPPY  Load a floppy disk from image file\n");
	fprintf(stdout,"MEMORY  Assign the memory size\n");
	fprintf(stdout,"INFO    Print all virtual machine settings\n\n");

	fprintf(stdout,"START   Turn on virtual machine\n");
	fprintf(stdout,"STOP    Turn off virtual machine; hotkey is F10\n");
	fprintf(stdout,"RESET   Restart virtual machine\n\n");
}
void Exit()
{
	if (!vmachine.flagrun) {
		if (vmachine.flaginit) vmachineFinal();
		exitFlag = 1;
	}
}

void Debug() {debug();}
//void Exec()
//{
//	char execmd[MAXLINE];
//	FILE *load;
//	t_nubit8 c;
//	t_nubit16 i = 0,end;
//	t_nubit32 len = 0;
//	if (!vmachine.flaginit || vmachine.flagrun) {
//		fprintf(stdout,"Cannot execute binary file now.\n");
//		return;
//	}
//	fprintf(stdout,".COM File: ");
//	fgets(execmd,MAXLINE,stdin);
//	parse(execmd);
//	if(!strlen(execmd)) return;
//	load = FOPEN(execmd,"rb");
//	if(!load) {
//		fprintf(stdout,"File not found\n");
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
//		//fprintf(stdout,"File '%s' is loaded to 0001:0100, length is %d bytes.\n",execmd,len);
//		fclose(load);
//		while(vcpu.ip < end && vmachine.flagrun) vmachineRefresh();
//	}
//}
void Record()
{
	if (!vmachine.flaginit || vmachine.flagrun) {
		fprintf(stdout,"Cannot change record status now.\n");
		return;
	}
	if (vmachine.flagrecord) {
		vmachine.flagrecord = 0x00;
		fprintf(stdout,"Recorder turned off.\n");
	} else {
		vmachine.flagrecord = 0x01;
		fprintf(stdout,"Recorder turned on.\n");
	}
}
#define _rec (vapirecord.rec[(i + vapirecord.start) % VAPI_RECORD_SIZE])
void Dump()
{
	t_nubitcc i = 0;
	Operand x;
	FILE *fp;
	char str[MAXLINE];
	fprintf(stdout,"Dump File: ");
	fgets(str, MAXLINE, stdin);
	parse(str);
	vmachineDumpRecordFile(str);
	fprintf(stdout,"Instruction Dump File: ");
	fgets(str, MAXLINE, stdin);
	parse(str);
	fp = fopen(str,"w");
	if (!fp) {
		fprintf(stdout,"ERORR:\tfailed to dump instruction.\n");
		return;
	}
	while (i < vapirecord.size) {
		disassemble(str, &x, (void *)vram.base, _rec.cs, _rec.ip);
		fprintf(fp,"%04X:%04X\t%s\n",_rec.cs,_rec.ip,str);
		_rec.cs;
		++i;
	}
	fclose(fp);
}

void Floppy()
{
	char str[MAXLINE];
	if (!vmachine.flaginit || vmachine.flagrun) {
		fprintf(stdout,"Cannot change floppy disk now.\n");
		return;
	}
	fprintf(stdout,"Floppy Image File: ");
	fgets(str, MAXLINE, stdin);
	parse(str);
	if (!vfdd.flagexist)
		vmachineInsertFloppy(str);
	else
		if (strlen(str))
			vmachineRemoveFloppy(str);
		else
			vmachineRemoveFloppy(NULL);
}
void Memory()
{
	unsigned int i;
	unsigned int tempSize;
	unsigned char testFlag = 0;
	char str[MAXLINE];
	if (!vmachine.flaginit || vmachine.flagrun) {
		fprintf(stdout,"Cannot modify memory size now.\n");
		return;
	}
	fflush(stdin);
	fprintf(stdout,"Size(KB): ");
	fgets(str,MAXLINE,stdin);
	tempSize = atoi(str);
	if(tempSize > 0x400) {
		vramAlloc(tempSize<<0x0a);
	} else {
		vramAlloc(1024<<0x0a);
	}
	for(i = 0;i < vram.size;++i)
	{
		if(i % 1024 == 0) fprintf(stdout,"\rMemory Testing : %dK",i/1024);
		if(d_nubit8(vram.base+i) != 0) {
			fprintf(stdout,"\nMemory test failed.\n");
			testFlag = 1;
			break;
		}
	}
	if(!testFlag) fprintf(stdout,"\rMemory Testing : %dK OK\n",i/1024);
	return;

}
void Info()
{
	fprintf(stdout,"init = %d\nrun = %d\nrecord=%d\nbreak = %d\ntrace = %d\n\n",
		vmachine.flaginit,vmachine.flagrun,vmachine.flagrecord,vmachine.flagbreak,vmachine.flagtrace);
}

void Mode()
{
	if (!vmachine.flaginit || vmachine.flagrun) {
		fprintf(stdout,"Cannot change display mode now.\n");
		return;
	}
	if (vmachine.flagmode) {
		vmachine.flagmode = 0x00;
		fprintf(stdout,"Display in console.\n");
	} else {
		vmachine.flagmode = 0x01;
		fprintf(stdout,"Display in window.\n");
	}
}
void Start()
{
	if (!vmachine.flagrun) {
		if (!vmachine.flaginit) vmachineInit();
		vmachineStart();
	} else
		fprintf(stdout, "Virtual machine is already running.\n");
}
void Stop()
{
	if (vmachine.flagrun) {
		vmachineStop();
	} else
		fprintf(stdout, "Virtual machine is already turned off.\n");
}
void Reset()
{
	if (vmachine.flagrun) {
		fprintf(stdout, "Virtual machine should be turned off first.\n");
		return;
	}
	if (vmachine.flaginit) vmachineFinal();
	vmachineInit();
}

void console()
{
	char cmdl[MAXLINE];
	exitFlag = 0;
	vmachineInit();
	fprintf(stdout,"Please enter 'HELP' for information.\n");
	while(!exitFlag) {
		fflush(stdin);
		fprintf(stdout,"Console> ");
		fgets(cmdl,MAXLINE,stdin);
		parse(cmdl);
		if(!strlen(cmdl)) continue;

		else if(!strcmp(cmdl,"test"))   Test();
		else if(!strcmp(cmdl,"help"))   Help();
		else if(!strcmp(cmdl,"exit"))   Exit();

		else if(!strcmp(cmdl,"debug"))  Debug();
		//else if(!strcmp(cmdl,"exec"))   Exec();
		else if(!strcmp(cmdl,"record")) Record();
		else if(!strcmp(cmdl,"dump"))   Dump();

		else if(!strcmp(cmdl,"floppy")) Floppy();
		else if(!strcmp(cmdl,"memory")) Memory();
		else if(!strcmp(cmdl,"info"))   Info();

		else if(!strcmp(cmdl,"mode"))   Mode();
		else if(!strcmp(cmdl,"start"))  Start();
		else if(!strcmp(cmdl,"stop"))   Stop();
		else if(!strcmp(cmdl,"reset"))  Reset();

		else fprintf(stdout,"Illegal command '%s'.\n",cmdl);

		fflush(stdin);
		fprintf(stdout,"\n");
	}
	if(vmachine.flaginit) vmachineFinal();
}