/* Copyright 2012-2014 Neko. */

#ifndef NXVM_LINUX_H
#define NXVM_LINUX_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "type.h"

typedef struct vm_platform_run_context vm_platform_run_context;

void vm_platform_linux_sleep(uint32_t milisec);
void vm_platform_linux_display_set_screen(int window, const vm_platform_run_context *context);
void vm_platform_linux_display_paint(int window, const vm_platform_run_context *context);
void vm_platform_linux_start_machine(int window, const vm_platform_run_context *context);

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
