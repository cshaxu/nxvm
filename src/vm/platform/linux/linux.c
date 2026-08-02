/* Copyright 2012-2014 Neko. */

/* LINUX provides linux platform interface. */

#include <unistd.h>

#include "vm/platform/linux/linuxcon.h"
#include "vm/platform/linux/linux.h"

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

void linuxStartMachine(int window, const vm_platform_run_context *context) {
    if (window) {
    } else {
        lnxcStartMachine(context);
    }
}
