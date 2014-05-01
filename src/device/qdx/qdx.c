/* Copyright 2012-2014 Neko. */

/* QDX implements quick and dirty instruction loader */

#include "qdcga.h"
#include "qdkeyb.h"
#include "qddisk.h"

#include "../vmachine.h"
#include "qdx.h"

t_faddrcc qdxTable[0x100];

void qdxExecInt(t_nubit8 intid) {
	if (qdxTable[intid]) {
		ExecFun(qdxTable[intid]);
	}
}

static void init() {
	t_nubit16 i;
	for (i = 0x0000;i < 0x0100;++i) {
		qdxTable[i] = (t_faddrcc) NULL;
	}
	qdkeybInit();
	qdcgaInit();
	qddiskInit();
}

static void reset() {qdcgaReset();}

static void refresh() {}

static void final() {}

void qdxRegister() {vmachineAddMe;}
