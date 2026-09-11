#ifndef VM_PRODUCT_RECORDER_H
#define VM_PRODUCT_RECORDER_H

#include "type.h"

#include "vm/machine/runtime/machine_interface.h"

typedef struct vm_product_recorder vm_product_recorder;

type_status vm_product_recorder_create(vm_product_recorder **out_recorder);
void vm_product_recorder_destroy(vm_product_recorder *recorder);
type_status vm_product_recorder_start(vm_product_recorder *recorder,
    const C_CHAR *path);
type_status vm_product_recorder_stop(vm_product_recorder *recorder);
void vm_product_recorder_observe(void *context,
    const vm_machine_debug_observation *observation);

#endif
