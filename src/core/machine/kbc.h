/* Copyright 2012-2014 Neko. */

#ifndef NXVM_CORE_KBC_H
#define NXVM_CORE_KBC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"
#include "core/machine/port.h"

#define NXVM_DEVICE_KBC "Intel 8042"

#define VKBC_STATUS_KE 0x10 /* keyboard enabled(1) or not(0) */

typedef struct { ntvdm64_type_unsigned_8 reserved; } t_kbc;
void core_machine_kbc_register_ports(t_kbc *controller, t_port *port);
void core_machine_kbc_initialize(t_kbc *controller, t_port *port);
void core_machine_kbc_reset(t_kbc *controller);
void core_machine_kbc_refresh(t_kbc *controller);
void core_machine_kbc_finalize(t_kbc *controller);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
