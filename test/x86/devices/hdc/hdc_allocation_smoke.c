#include "lib/types/types_interface.h"
#include "lib/types/test.h"
#include "x86/devices/hdc/hdc_interface.h"

static lib_bool fail_allocation;
static lib_u32 attempts, live_allocations, signals;

static void *allocate(lib_size count, lib_size size)
{
    ++attempts;
    if (fail_allocation) return LIB_NULL;
    void *memory = lib_allocate_zero(count, size);
    if (memory != LIB_NULL) ++live_allocations;
    return memory;
}

static void release(void *memory)
{
    if (memory != LIB_NULL) {
        lib_test_assert(live_allocations != 0u);
        --live_allocations;
    }
    lib_release(memory);
}

static void signal_level(void *context, lib_bool asserted)
{
    (void)context;
    (void)asserted;
    ++signals;
}

/* Owner-local fault injection; no allocator API is added to the chip. */
#define lib_allocate_zero allocate
#define lib_release release
#include "x86/devices/hdc/hdc.c"
#undef lib_allocate_zero
#undef lib_release

lib_i32 main(void)
{
    const x86_hdc_connection connection = {.irq = signal_level, .drq = signal_level};
    for (lib_u32 protocol = X86_HDC_PROTOCOL_ATA_PIO;
        protocol <= X86_HDC_PROTOCOL_XEBEC_XT; ++protocol) {
        x86_hdc_config config = {.protocol = (x86_hdc_protocol)protocol};
        x86_hdc *hdc = (x86_hdc *)&config;
        fail_allocation = LIB_TRUE;
        attempts = signals = 0u;
        lib_test_assert(x86_hdc_create(&config, &connection, &hdc) == LIB_STATUS_NO_MEMORY);
        lib_test_assert(hdc == LIB_NULL && attempts == 1u && signals == 0u &&
            live_allocations == 0u);
        fail_allocation = LIB_FALSE;
        lib_test_assert(x86_hdc_create(&config, &connection, &hdc) == LIB_STATUS_OK);
        lib_test_assert(hdc != LIB_NULL && live_allocations == 1u);
        x86_hdc_destroy(hdc);
        lib_test_assert(live_allocations == 0u);
    }
    x86_hdc_config invalid = {.protocol = (x86_hdc_protocol)99};
    x86_hdc *hdc = (x86_hdc *)&invalid;
    attempts = signals = 0u;
    lib_test_assert(x86_hdc_create(&invalid, &connection, &hdc) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(hdc == LIB_NULL && attempts == 0u && signals == 0u);
    x86_hdc_destroy(LIB_NULL);
    return 0;
}
