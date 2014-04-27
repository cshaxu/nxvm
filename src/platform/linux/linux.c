/* Copyright 2012-2014 Neko. */

/* LINUX provides linux platform interface. */

#include <unistd.h>

#include "linux.h"
#include "linuxcon.h"

void linuxSleep(unsigned int milisec) {usleep((milisec) * 1000);}

void linuxDisplaySetScreen(unsigned char window) {
	if (window) {
	} else {
		lnxcDisplaySetScreen();
	}
}

void linuxDisplayPaint(unsigned char window) {
	if (window) {
	} else {
		lnxcDisplayPaint();
	}
}

void linuxStartMachine(unsigned char window) {
	if (window) {
	} else {
		lnxcStartMachine();
	}
}
