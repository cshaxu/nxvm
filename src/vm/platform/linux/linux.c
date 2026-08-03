/* Copyright 2012-2014 Neko. */

/* LINUX provides linux platform interface. */

#include "type.h"

#include <unistd.h>


#include "vm/platform/linux/linuxcon.h"

#include "vm/platform/linux/linux.h"

C_VOID vm_platform_linux_sleep(uint32_t milisec) {
    usleep((milisec) * 1000);
}

C_VOID vm_platform_linux_display_set_screen(C_INT window, const vm_platform_run_context *context) {
    if (window) {
    } else {
        lnxcDisplaySetScreen(context);
    }
}

C_VOID vm_platform_linux_display_paint(C_INT window, const vm_platform_run_context *context) {
    if (window) {
    } else {
        lnxcDisplayPaint(context);
    }
}

C_VOID vm_platform_linux_start_machine(C_INT window, const vm_platform_run_context *context) {
    if (window) {
    } else {
        lnxcStartMachine(context);
    }
}
