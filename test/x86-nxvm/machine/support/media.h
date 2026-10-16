#include "lib/types/test.h"
#ifndef NXVM_TEST_MEDIA_H
#define NXVM_TEST_MEDIA_H

#include "x86-nxvm/machine/media/fdd_interface.h"
#include "x86-nxvm/machine/media/hdd_interface.h"

/* Composition tests observe the same copied provider contract as controllers. */
static inline x86_nxvm_board_media_info vm_test_media_info(
    const x86_nxvm_board_media_provider *provider, void *context)
{
    x86_nxvm_board_media_info info = {0};
    x86_nxvm_board_media_result result = provider->query(context, &info);
    lib_test_assert(result == X86_NXVM_BOARD_MEDIA_RESULT_OK ||
        result == X86_NXVM_BOARD_MEDIA_RESULT_ABSENT);
    (void)result;
    return info;
}

static inline x86_nxvm_board_media_info vm_test_fdd_info(t_fdd *fdd)
{ return vm_test_media_info(x86_nxvm_machine_fdd_media_provider(), fdd); }

static inline x86_nxvm_board_media_info vm_test_hdd_info(t_hdd *hdd)
{ return vm_test_media_info(x86_nxvm_machine_hdd_media_provider(), hdd); }

#endif
