#ifndef VM_APP_RECORDER_H
#define VM_APP_RECORDER_H

#include "type.h"

#include "vm/machine/machine_interface.h"

typedef struct vm_app_recorder vm_app_recorder;

type_status vm_app_recorder_create(vm_app_recorder **out_recorder);
void vm_app_recorder_destroy(vm_app_recorder *recorder);
type_status vm_app_recorder_start(vm_app_recorder *recorder,
    const C_CHAR *path);
type_status vm_app_recorder_stop(vm_app_recorder *recorder);
void vm_app_recorder_observe(void *context,
    const vm_machine_debug_observation *observation);

#endif
