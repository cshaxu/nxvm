#ifndef VM_PRODUCT_CONTROL_H
#define VM_PRODUCT_CONTROL_H

#include "type.h"

#include "vm/platform/session_state.h"

typedef struct vm_product_control vm_product_control;

typedef enum vm_product_control_fact_kind {
    VM_PRODUCT_CONTROL_FACT_MONITOR_LINE,
    VM_PRODUCT_CONTROL_FACT_LIFECYCLE
} vm_product_control_fact_kind;

typedef struct vm_product_control_fact {
    vm_product_control_fact_kind kind;
    type_unsigned_32 run_generation;
    union {
        C_CHAR line[1024];
        vm_session_lifecycle lifecycle;
    } value;
} vm_product_control_fact;

type_status vm_product_control_create(vm_product_control **out_control);
C_VOID vm_product_control_destroy(vm_product_control *control);
type_status vm_product_control_publish(vm_product_control *control,
    const vm_product_control_fact *fact);
type_status vm_product_control_take(vm_product_control *control,
    vm_product_control_fact *out_fact, C_UINT timeout_milliseconds);
C_VOID vm_product_control_close(vm_product_control *control);

#endif
