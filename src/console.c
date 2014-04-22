/* This file is a part of NekoVM project. */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "nvm/vmachine.h"

#include "global.h"
#include "debug.h"
#include "console.h"

int exitFlag;

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

void NSExec()
{
	char execmd[MAXLINE];
	fprintf(stdout,"Command: ");
	fgets(execmd,MAXLINE,stdin);
	parse(execmd);
	if(!strlen(execmd)) return;
	fprintf(stdout,"DEBUG:\tFile to be executed: '%s'\n",execmd);
}

void NSExit()
{
	exitFlag = 1;
}

void NSHelp()
{
	fprintf(stdout,"DEBUG\tRuns Debug, a program testing and editing tool.\n");
	fprintf(stdout,"EXEC\tRuns a binary of '.COM' format from host machine.\n");
	fprintf(stdout,"EXIT\tQuits the NVM console (command interpreter).\n");
	fprintf(stdout,"HELP\tProvides Help information for NVM console commands.\n");
	fprintf(stdout,"RESET\tRestarts Neko's Virtual Machine.\n");
}

void NSReset()
{
	NVMTerm();
	NVMInit();
}

void NSConsole()
{
#ifndef DEBUG
	char cmdl[MAXLINE];
	exitFlag = 0;
	fprintf(stdout,"\nPlease enter 'HELP' for information.\n\n");
	while(!exitFlag) {
		fflush(stdin);
		fprintf(stdout,"Console> ");
		fgets(cmdl,MAXLINE,stdin);
		parse(cmdl);
		if(!strlen(cmdl)) continue;
		else if(!strcmp(cmdl,"debug")) 
#endif
			NSDebug();
#ifndef DEBUG
		else if(!strcmp(cmdl,"exit")) NSExit();
		else if(!strcmp(cmdl,"help")) NSHelp();
		else if(!strcmp(cmdl,"reset")) NSReset();
		else if(!strcmp(cmdl,"exec")) NSExec();
		else fprintf(stdout,"Illegal command '%s'.\n",cmdl);
		fprintf(stdout,"\n");
	}
#endif
}