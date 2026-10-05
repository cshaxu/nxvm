#ifndef VM_PROFILE_SELECTION_INTERFACE_H
#define VM_PROFILE_SELECTION_INTERFACE_H
#include "lib/types/types_interface.h"


#include "ibmpc/machine/input_interface.h"

typedef enum vm_machine_profile_kind {
    VM_MACHINE_PROFILE_DEFAULT_PC_AT,
    VM_MACHINE_PROFILE_IBM_5170_MODEL_339,
    VM_MACHINE_PROFILE_IBM_5160_MODEL_268,
    VM_MACHINE_PROFILE_COMPAQ_DESKPRO_386_MODEL_40
} vm_machine_profile_kind;

const char *vm_profile_name(vm_machine_profile_kind kind);

#endif
