#ifndef NTVDM64_VM_COMPOSITION_CONTROL_H
#define NTVDM64_VM_COMPOSITION_CONTROL_H

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

void vm_composition_control_start(void);
void vm_composition_control_reset(void);
void vm_composition_control_stop(void);
void vm_composition_control_initialize(void);
void vm_composition_control_finalize(void);
int vm_composition_control_is_running(void);
int vm_composition_control_get_flip(void);
void vm_composition_control_print_status(void);
void vm_composition_control_bind_command_boundary(
    void (*callback)(void *opaque), void *opaque);

#ifdef __cplusplus
}
#endif

#endif
