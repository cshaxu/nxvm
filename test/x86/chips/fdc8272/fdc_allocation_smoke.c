#include "fixture.h"
#include "lib/types/test.h"

static lib_bool fail_allocation;
static lib_u32 attempts, live_allocations;

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

/* Owner-local failure injection, no allocator seam in the product contract. */
#define lib_allocate_zero allocate
#define lib_release release
#include "x86/chips/fdc8272/fdc.c"
#undef lib_allocate_zero
#undef lib_release

int main(void)
{
    fixture state = {.present = LIB_TRUE, .ready_mask = 0x0fu};
    x86_fdc_timing timing = {0u, 1u, 1u, 1u, 1u};

    for (lib_u8 retry = 0u; retry < 3u; ++retry) {
        fail_allocation = LIB_TRUE;
        attempts = 0u;
        state.chip = (x86_fdc *)&state;
        lib_test_assert(fixture_create(&state, &timing) == LIB_STATUS_NO_MEMORY);
        lib_test_assert(state.chip == LIB_NULL && attempts == 1u && live_allocations == 0u);
        lib_test_assert(!state.irq && !state.drq);

        fail_allocation = LIB_FALSE;
        timing.step_denominator = 0u;
        lib_test_assert(fixture_create(&state, &timing) == LIB_STATUS_INVALID_ARGUMENT);
        lib_test_assert(state.chip == LIB_NULL && live_allocations == 0u);
        timing.step_denominator = 1u;
        lib_test_assert(fixture_create(&state, &timing) == LIB_STATUS_OK);
        lib_test_assert(state.chip != LIB_NULL && live_allocations == 1u);
        x86_fdc_set_service_enabled(state.chip, LIB_TRUE);
        x86_fdc_set_reset(state.chip, LIB_TRUE);
        fixture_command(&state, (const lib_u8[]){0x03u, 0xf0u, 0u}, 3u);
        fixture_command(&state, (const lib_u8[]){0x46u, 0u, 0u, 0u, 1u, 2u, 1u, 0u, 0u}, 9u);
        lib_test_assert(state.drq);
        x86_fdc_destroy(state.chip);
        state.chip = LIB_NULL;
        lib_test_assert(live_allocations == 0u && !state.irq && !state.drq);
    }
    x86_fdc_destroy(LIB_NULL);
    return 0;
}
