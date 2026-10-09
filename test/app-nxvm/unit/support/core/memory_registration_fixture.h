#ifndef TEST_CORE_MEMORY_REGISTRATION_FIXTURE_H
#define TEST_CORE_MEMORY_REGISTRATION_FIXTURE_H
#include "core/x86/machine_interface.h"

typedef lib_status (*test_core_memory_registration_configure)(
    core_machine *machine, void *context);
typedef lib_i32 (*test_core_memory_registration_exercise)(
    core_machine *machine, void *context);

/* Retain capacity failure, rollback, retry, ownership and removal assertions
 * at the real registry owner. Callbacks exercise the installing component. */
lib_i32 test_core_memory_registration_case(lib_u32 mode, void *owner,
    test_core_memory_registration_configure configure,
    test_core_memory_registration_exercise exercise, void *context);
#endif
