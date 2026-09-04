/* Copyright 2012-2014 Neko. */

/* LINUX provides linux platform interface. */

#include "type.h"

#include "vm/platform/platform.h"


#include "vm/platform/linux/linuxcon.h"

#include "vm/platform/linux/linux.h"

C_VOID vm_platform_linux_display_set_screen(C_INT window, const vm_platform_run_context *context) {
    if (window) {
    } else {
        vm_platform_linuxcon_display_set_screen(context);
    }
}

C_VOID vm_platform_linux_display_paint(C_INT window, const vm_platform_run_context *context) {
    if (window) {
    } else {
        vm_platform_linuxcon_display_paint(context);
    }
}

type_status vm_platform_linux_run_handle_start(
    const vm_platform_run_context *context, vm_platform_run_handle *handle) {
    if (vm_platform_run_context_get_display_mode(context) ==
        VM_PLATFORM_DISPLAY_WINDOW) {
        return TYPE_STATUS_UNSUPPORTED;
    }
    return vm_platform_linuxcon_run_handle_start(context, handle);
}

C_VOID vm_platform_linux_run_handle_request_stop(vm_platform_run_handle *handle) {
    vm_platform_linuxcon_run_handle_request_stop(handle);
}

C_VOID vm_platform_linux_run_handle_join(vm_platform_run_handle *handle) {
    vm_platform_linuxcon_run_handle_join(handle);
}

C_VOID vm_platform_linux_run_handle_finalize(vm_platform_run_handle *handle) {
    vm_platform_linuxcon_run_handle_finalize(handle);
}
