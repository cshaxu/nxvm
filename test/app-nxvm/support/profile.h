#ifndef NXVM_TEST_PROFILE_H
#define NXVM_TEST_PROFILE_H

#include "app-nxvm/profiles/machine_plan_interface.h"
#include "x86/product/machine/machine_private.h"

/* App fixtures prepare this actual Profile context; the Shared adapter owns
 * its lifetime. This borrowed test view adds no registry or mirrored state. */
static inline vm_profile_machine_plan *vm_test_profile_plan(
    const vm_machine *machine)
{
    return machine == LIB_NULL ? LIB_NULL :
        (vm_profile_machine_plan *)machine->construction.profile.context;
}

#endif
