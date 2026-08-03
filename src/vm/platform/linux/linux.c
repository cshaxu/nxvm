/* Copyright 2012-2014 Neko. */

/* LINUX provides linux platform interface. */

#include <unistd.h>

#include "vm/platform/linux/linuxcon.h"
#include "vm/platform/linux/linux.h"

void vm_platform_linux_sleep(uint32_t milisec) {
    usleep((milisec) * 1000);
}

void vm_platform_linux_display_set_screen(int window, const vm_platform_run_context *context) {
    if (window) {
    } else {
        lnxcDisplaySetScreen(context);
    }
}

void vm_platform_linux_display_paint(int window, const vm_platform_run_context *context) {
    if (window) {
    } else {
        lnxcDisplayPaint(context);
    }
}

void vm_platform_linux_start_machine(int window, const vm_platform_run_context *context) {
    if (window) {
    } else {
        lnxcStartMachine(context);
    }
}
