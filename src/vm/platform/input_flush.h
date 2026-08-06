#ifndef VM_PLATFORM_INPUT_FLUSH_H
#define VM_PLATFORM_INPUT_FLUSH_H

#include "type.h"

/* Retained VM Console hygiene, not a neutral host-input contract. */
C_VOID vm_platform_input_flush_console_input(C_VOID);

#endif
