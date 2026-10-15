#ifndef X86_IBMPC_PRODUCT_MACHINE_INTERFACE_H
#define X86_IBMPC_PRODUCT_MACHINE_INTERFACE_H

#include "emulator/product/machine_interface.h"

/* The PC shell sees only an already composed machine lifetime. */
typedef struct app_composed_machine {
    emulator_product_machine composition;
} app_composed_machine;

#endif
