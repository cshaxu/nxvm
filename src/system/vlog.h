
#ifndef NXVM_VLOG_H
#define NXVM_VLOG_H

#include "stdio.h"
#include "../vmachine/vglobal.h"

#define VLOG_COUNT_MAX 0xffff

typedef struct {
	FILE *fp;
	t_nubitcc line;
} t_log;

extern t_log vlog;

void vlogInit();
void vlogFinal();
void vlogExec();

#endif