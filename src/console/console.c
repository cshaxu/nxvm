/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "../vmachine/vmachine.h"

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

#include "../vmachine/vapi.h"
void Test()
{
	INPUT_RECORD inRec;
	DWORD ckeyState = 0;
	DWORD res;
	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
	t_nubit16 ascii;
	t_nubit16 vkey;
	while(1) {
		ReadConsoleInput(hIn,&inRec,1,(LPDWORD)(&res));
		if (inRec.Event.KeyEvent.bKeyDown){
			switch (inRec.EventType) {
			case KEY_EVENT:
				vapiPrint("Key Pressed: %c\n",inRec.Event.KeyEvent.uChar.AsciiChar);

				ascii = inRec.Event.KeyEvent.wVirtualScanCode;
				vkey = inRec.Event.KeyEvent.wVirtualKeyCode;
				if (inRec.Event.KeyEvent.bKeyDown) {
					switch (vkey) {
					case VK_F1: vapiPrint("f1");break;
					case VK_F2: vapiPrint("f2");break;
					case VK_F3: vapiPrint("f3");break;
					case VK_F4: vapiPrint("f4");break;
					case VK_F5: vapiPrint("f5");break;
					case VK_F6: vapiPrint("f6");break;
					case VK_F7: vapiPrint("f7");break;
					case VK_F8: vapiPrint("f8");break;
					case VK_F9: vapiPrint("f9");break;
					case VK_F10: vapiPrint("f10");break;
					case VK_F11: vapiPrint("f11");break;
					case VK_F12: vapiPrint("f12");break;
					case VK_ESCAPE: vapiPrint("esc");break;
					case VK_PRIOR: vapiPrint("pageup");break;
					case VK_NEXT: vapiPrint("pagedn");break;
					case VK_END: vapiPrint("end");break;
					case VK_HOME: vapiPrint("home");break;
					case VK_LEFT: vapiPrint("left");break;
					case VK_UP: vapiPrint("right");break;
					case VK_RIGHT: vapiPrint("up");break;
					case VK_DOWN: vapiPrint("dn");break;
							break;
					default://剩下的字符可能是alt。。ctl与普通字符等，但是updateKBStatus会过滤掉普通字符
						if (vkey >= 0x41 && vkey <= 0x41+'Z'-'A') {
							if (vapiCallBackKeyboardGetAlt())
								vapiPrint("%c", ascii & 0xff);
							if (vapiCallBackKeyboardGetCtrl())
								vapiPrint("%c", (ascii + vkey - 0x0041) & 0xff);
							//如果不是按下ctrl，则是按下alt
						}
						break;
					}
				}

				break;
			default:
				break;
			}
		}
	}
}

void Help()
{
	fprintf(stdout,"NXVM Console Commands\n");
	fprintf(stdout,"=====================\n");
	fprintf(stdout,"HELP    Show this help menu\n");
	fprintf(stdout,"EXIT    Quit the console\n\n");

	fprintf(stdout,"DEBUG   Execute debug, a program testing and editing tool\n");
	fprintf(stdout,"EXEC    Execute a binary file\n");
	fprintf(stdout,"RECORD  Record cpu status for each instruction\n");
	fprintf(stdout,"TRACE   Trace cpu status for each instruction\n");
	fprintf(stdout,"FLOPPY  Load a floppy disk from image file\n");
	fprintf(stdout,"MEMORY  Assign the memory size\n");
	fprintf(stdout,"INFO    Print all virtual machine settings\n\n");

	fprintf(stdout,"START   Turn on virtual machine\n");
	fprintf(stdout,"STOP    Turn off virtual machine\n");
	fprintf(stdout,"RESET   Restart virtual machine\n\n");
}
void Exit()
{
	if (!vmachine.flagrun) {
		if (vmachine.flaginit) vmachineFinal();
		exitFlag = 1;
	}
}

void Debug()
{
	debug();
}
void Exec()
{
	/* NOTE: vmachine run environment */
	char execmd[MAXLINE];
	FILE *load;
	t_nubit8 c;
	t_nubit16 i = 0,end;
	t_nubit32 len = 0;
	if (!vmachine.flaginit || vmachine.flagrun) {
		fprintf(stdout,"Cannot execute binary file now.\n");
		return;
	}
	fprintf(stdout,".COM File: ");
	fgets(execmd,MAXLINE,stdin);
	parse(execmd);
	if(!strlen(execmd)) return;
	load = FOPEN(execmd,"rb");
	if(!load) {
		fprintf(stdout,"File not found\n");
		return;
	} else {
		vcpu.ax = vcpu.bx = vcpu.cx = vcpu.dx = 0x0000;
		vcpu.si = vcpu.di = vcpu.bp = 0x0000;
		vcpu.sp = 0xffee;	vcpu.ip = 0x0100;
		vcpu.ds = vcpu.es = vcpu.ss = vcpu.cs = 0x0001;
		vmachine.flagrun = 1;
		c = fgetc(load);
		while(!feof(load)) {
			vramVarByte(vcpu.cs+i,vcpu.ip+((len++)%0x10000)) = c;
			i = len / 0x10000;
			c = fgetc(load);
		}
		end = vcpu.ip+len;
		//fprintf(stdout,"File '%s' is loaded to 0001:0100, length is %d bytes.\n",execmd,len);
		fclose(load);
		while(vcpu.ip < end && vmachine.flagrun) vmachineRefresh();
	}
}
void Record()
{
	char str[MAXLINE];
	if (!vmachine.flaginit || vmachine.flagrun) {
		fprintf(stdout,"Cannot change record status now.\n");
		return;
	}
	if (vmachine.flagrecord) {
		vmachine.flagrecord = 0x00;
		fprintf(stdout,"Recorder turned off.\n");
	} else {
		fprintf(stdout,"Record File: ");
		fgets(str, MAXLINE, stdin);
		parse(str);
		vmachine.flagrecord = 0x01;
		vmachineSetRecordFile(str);
		fprintf(stdout,"Recorder turned on.\n");
	}
}
void Trace()
{
	if (!vmachine.flaginit || vmachine.flagrun) {
		fprintf(stdout,"Cannot change trace status now.\n");
		return;
	}
	if (vmachine.flagtrace) {
		vmachine.flagtrace = 0x00;
		fprintf(stdout,"Tracer turned off.\n");
	} else {
		vmachine.flagtrace = 0x01;
		fprintf(stdout,"Tracer turned on.\n");
	}
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
		vmachineRemoveFloppy(str);
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
		if(*(t_nubit8 *)(vram.base+i) != 0) {
			fprintf(stdout,"\nMemory test failed.\n");
			testFlag = 1;
			break;
		}
	}
	if(!testFlag) fprintf(stdout,"\rMemory Testing : %dK OK\n",i/1024);
	return;

}
void Info()
{}

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
	if (vmachine.flagrun)
		vmachine.flagrun = 0x00;
	else
		fprintf(stdout, "Virtual machine is already turned off.\n");
}
void Reset()
{
/*	if (vmachine.flaginit && vmachine.flagrun)
		vkbc.flagreset = 0x01;*/
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
		else if(!strcmp(cmdl,"exec"))   Exec();
		else if(!strcmp(cmdl,"record")) Record();
		else if(!strcmp(cmdl,"trace"))  Trace();

		else if(!strcmp(cmdl,"floppy")) Floppy();
		else if(!strcmp(cmdl,"memory")) Memory();
		else if(!strcmp(cmdl,"info"))   Info();

		else if(!strcmp(cmdl,"start"))  Start();
		else if(!strcmp(cmdl,"stop"))   Stop();
		else if(!strcmp(cmdl,"reset"))  Reset();

		else fprintf(stdout,"Illegal command '%s'.\n",cmdl);

		fflush(stdin);
		fprintf(stdout,"\n");
	}
	if(vmachine.flaginit) vmachineFinal();
}