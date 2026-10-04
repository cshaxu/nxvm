#ifndef NXVM_TEST_MEDIA_H
#define NXVM_TEST_MEDIA_H

#include <assert.h>
#include "x86/product/machine/media/fdd_interface.h"
#include "x86/product/machine/media/hdd_interface.h"

/* Composition tests observe the same copied provider contract as controllers. */
static inline core_machine_media_info vm_test_media_info(
    const core_machine_media_provider *provider, void *context)
{
    core_machine_media_info info = {0};
    core_machine_media_result result = provider->query(context, &info);
    assert(result == CORE_MACHINE_MEDIA_RESULT_OK ||
        result == CORE_MACHINE_MEDIA_RESULT_ABSENT);
    (void)result;
    return info;
}

static inline core_machine_media_info vm_test_fdd_info(t_fdd *fdd)
{ return vm_test_media_info(vm_machine_fdd_media_provider(), fdd); }

static inline core_machine_media_info vm_test_hdd_info(t_hdd *hdd)
{ return vm_test_media_info(vm_machine_hdd_media_provider(), hdd); }

#endif
