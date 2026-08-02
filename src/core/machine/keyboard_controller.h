/* Copyright 2012-2014 Neko. */

#ifndef NXVM_CORE_KEYBOARD_CONTROLLER_H
#define NXVM_CORE_KEYBOARD_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "core/machine/vglobal.h"

#define NXVM_DEVICE_KBC "Intel 8042"

#define VKBC_STATUS_KE 0x10 /* keyboard enabled(1) or not(0) */

typedef struct { t_nubit8 reserved; } t_kbc;
t_kbc *core_machine_keyboard_controller_current(void);
void core_machine_keyboard_controller_bind_live(t_kbc *controller);
void core_machine_keyboard_controller_unbind_live(void);
#define vkbc (*core_machine_keyboard_controller_current())

void vkbcInit();
void vkbcReset();
void vkbcRefresh();
void vkbcFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
