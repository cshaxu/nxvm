#ifndef VM_PLATFORM_EXECUTION_WAIT_H
#define VM_PLATFORM_EXECUTION_WAIT_H

#include "lib/session/executor.h"

#define VM_PLATFORM_EXECUTION_FLIP_TIMEOUT_MILLISECONDS 5000u

C_INT vm_platform_wait_for_execution_flip(const lib_session_executor *executor,
    C_INT initial_flip, type_unsigned_32 timeout_milliseconds);

#endif
