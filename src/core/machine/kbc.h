/* Copyright 2012-2014 Neko. */

#ifndef NXVM_CORE_KBC_H
#define NXVM_CORE_KBC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "core/machine/vglobal.h"
#include "core/machine/port.h"

#define NXVM_DEVICE_KBC "Intel 8042"

#define VKBC_STATUS_KE 0x10 /* keyboard enabled(1) or not(0) */

typedef struct { t_nubit8 reserved; } t_kbc;
t_kbc *core_machine_kbc_current(void);
void core_machine_kbc_bind_live(t_kbc *controller);
void core_machine_kbc_unbind_live(void);
#define vkbc (*core_machine_kbc_current())

void vkbcInit();
void vkbcReset();
void vkbcRefresh();
void vkbcFinal();
void core_machine_kbc_register_ports(t_kbc *controller, t_port *port);
void core_machine_kbc_initialize(t_kbc *controller, t_port *port);
void core_machine_kbc_reset(t_kbc *controller);
void core_machine_kbc_refresh(t_kbc *controller);
void core_machine_kbc_finalize(t_kbc *controller);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
