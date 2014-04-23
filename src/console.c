/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "vmachine/vmachine.h"
#include "vmachine/system/vapi.h"

#include "global.h"
#include "debug.h"
#include "console.h"

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

void NSTest()
{}

void NSExec()
{
	char execmd[MAXLINE];
	FILE *load;
	t_nubit8 c;
	t_nubit16 i = 0,end;
	t_nubit32 len = 0;
	fprintf(stdout,".COM File: ");
	fgets(execmd,MAXLINE,stdin);
	parse(execmd);
	if(!strlen(execmd)) return;
	load = fopen(execmd,"rb");
	if(!load) {
		fprintf(stdout,"File not found\n");
		return;
	} else {
		vcpu.ax = vcpu.bx = vcpu.cx = vcpu.dx = 0x0000;
		vcpu.si = vcpu.di = vcpu.bp = 0x0000;
		vcpu.sp = 0xffee;	vcpu.ip = 0x0100;
		vcpu.ds = vcpu.es = vcpu.ss = vcpu.cs = 0x0001;
		vmachinerunflag = 1;
		c = fgetc(load);
		while(!feof(load)) {
			vramSetByte(vcpu.cs+i,vcpu.ip+((len++)%0x10000),c);
			i = len / 0x10000;
			c = fgetc(load);
		}
		end = vcpu.ip+len;
		//fprintf(stdout,"File '%s' is loaded to 0001:0100, length is %d bytes.\n",execmd,len);
		fclose(load);
		while(vcpu.ip < end && vmachinerunflag) vmachineRefresh();
	}
}
void NSExit()
{
	exitFlag = 1;
}
void NSHelp()
{
	fprintf(stdout,"NXVM Console Commands\n");
	fprintf(stdout,"=====================\n");
	fprintf(stdout,"DEBUG\t\tRuns Debug, a program testing and editing tool.\n");
	fprintf(stdout,"EXEC\t\tRuns a binary of '.COM' format from host machine.\n");
	fprintf(stdout,"EXIT\t\tQuits the NXVM console (command interpreter).\n");
	fprintf(stdout,"HELP\t\tProvides Help information for NXVM console commands.\n\n");
	fprintf(stdout,"POWON\t\tPowers on Neko's x86 Virtual Machine.\n");
	//fprintf(stdout,"STATUS\t\tPrints the status of NXVM.\n");
	fprintf(stdout,"FLOPPY\t\tAssigns the name of floppy image.\n");
	fprintf(stdout,"MEMORY\t\tAssigns the memory size of NXVM.\n");
	fprintf(stdout,"MEMORYTEST\tTests the memory size of NXVM.\n");
/*	fprintf(stdout,"NXVM Operations\n");
	fprintf(stdout,"==============\n");
	fprintf(stdout,"OFF\t\tTurns off Neko's x86 Virtual Machine.\n");
	fprintf(stdout,"RESET\t\tRestarts Neko's x86 Virtual Machine.\n");*/
	
}

void NSFloppy()
{
	char str[MAXLINE];
	fprintf(stdout,"Floppy Image File: ");
	fgets(str, MAXLINE, stdin);
	parse(str);
	vapiInsertFloppyDisk(str);
}
void NSMemory()
{
	t_nubit32 tempSize;
	char str[MAXLINE];
	if(vmachinerunflag) {fprintf(stdout,"Cannot modify memory size now.\n");return;}
	fflush(stdin);
	fprintf(stdout,"Size(KB): ");
	fgets(str,MAXLINE,stdin);
	tempSize = atoi(str);
	if(tempSize > 0x400) {
		vramAlloc(tempSize<<0x0a);
	} else {
		vramAlloc(1024<<0x0a);
	}
}
void NSMemoryTest()
{
	t_bool testFlag = 0;
	t_nubit32 i;
	if(vmachinerunflag || !vmachineinitflag) {fprintf(stdout,"Cannot perform memory test now.\n");return;}
	for(i = 0;i < vram.size;++i)
	{
		if(i % 1024 == 0) fprintf(stdout,"\rMemory Testing : %dK",i/1024);
		if(*(t_nubit8 *)(vram.base+i) != 0) {
			fprintf(stdout,"\nMemory test failed.\n");
			testFlag = 1;
			break;
		}
	}
	if(!testFlag) fprintf(stdout,"\rMemory Testing : %dK OK\n",i/1024);
	return;
}

void NSPowOn()
{
	if(vmachineinitflag) vmachineFinal();
	vmachinePowerOn();
}
/*void NSOff()
{
	vmachinePowerOff();
	if(!vmachineinitflag) vmachineInit();
}
void NSReset()
{
	if(vmachinerunflag) {
		vmachinePowerOff();
		vmachinePowerOn();
	}
}*/

void NSConsole()
{
	char cmdl[MAXLINE];
	exitFlag = 0;
	if(!vmachineinitflag) vmachineInit();
	fprintf(stdout,"Please enter 'HELP' for information.\n");
	while(!exitFlag) {
		fflush(stdin);
		fprintf(stdout,"Console> ");
		fgets(cmdl,MAXLINE,stdin);
		parse(cmdl);
		if(!strlen(cmdl)) continue;
		else if(!strcmp(cmdl,"test")) NSTest();
		else if(!strcmp(cmdl,"debug")) NSDebug();
		else if(!strcmp(cmdl,"exec")) NSExec();
		else if(!strcmp(cmdl,"exit")) NSExit();
		else if(!strcmp(cmdl,"help")) NSHelp();
		//else if(!strcmp(cmdl,"pwd")) fprintf(stdout,"%s\n",argv[0]);

		else if(!strcmp(cmdl,"floppy")) NSFloppy();
		else if(!strcmp(cmdl,"memory")) NSMemory();
		else if(!strcmp(cmdl,"memorytest")) NSMemoryTest();

		else if(!strcmp(cmdl,"powon")) NSPowOn();
		/*else if(!strcmp(cmdl,"off")) NSOff();
		else if(!strcmp(cmdl,"reset")) NSReset();*/

		else fprintf(stdout,"Illegal command '%s'.\n",cmdl);
		fprintf(stdout,"\n");
	}
	if(vmachineinitflag) vmachineFinal();
}