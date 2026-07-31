/* Copyright 2012-2014 Neko. */

/* LINUX provides linux platform interface. */

#include <unistd.h>

#include "linuxcon.h"
#include "linux.h"

void linuxSleep(uint32_t milisec) {
    usleep((milisec) * 1000);
}

void linuxDisplaySetScreen(int window) {
    if (window) {
    } else {
        lnxcDisplaySetScreen();
    }
}

void linuxDisplayPaint(int window) {
    if (window) {
    } else {
        lnxcDisplayPaint();
    }
}

void linuxStartMachine(int window) {
    if (window) {
    } else {
        lnxcStartMachine();
    }
}
