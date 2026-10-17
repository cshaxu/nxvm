#include "lib/types/test.h"
#include "lib/types/types_interface.h"

/* Run the Linux owner with a controlled native counter, on either build host. */
#define LIB_TYPES_LINUX_CLOCK_H
typedef struct { lib_i64 tv_sec, tv_nsec; } lib_linux_timespec;
#define LIB_LINUX_CLOCK_MONOTONIC 1
static lib_linux_timespec value = { 2, 123 };
static lib_i32 failure;
static lib_u32 queries;
static lib_i32 counter(lib_i32 clock, lib_linux_timespec *out)
{
    lib_test_assert(clock == LIB_LINUX_CLOCK_MONOTONIC);
    ++queries;
    *out = value;
    return failure;
}
#define lib_linux_clock_gettime counter
#include "lib/base/linux/clock.c"

int main(void)
{
    lib_u64 units = 999u, frequency = 888u;
    lib_test_assert(base_clock_platform_counter(LIB_NULL, &frequency) == LIB_STATUS_IO_ERROR);
    lib_test_assert(base_clock_platform_counter(&units, LIB_NULL) == LIB_STATUS_IO_ERROR);
    lib_test_assert(queries == 0u);
    for (lib_u32 stage = 0u; stage < 3u; ++stage) {
        failure = stage == 0u ? -1 : 0;
        value = (lib_linux_timespec){ stage == 1u ? -1 : 2, stage == 2u ? -1 : 123 };
        lib_test_assert(base_clock_platform_counter(&units, &frequency) == LIB_STATUS_IO_ERROR);
        lib_test_assert(units == 999u && frequency == 888u);
    }
    value = (lib_linux_timespec){ 2, 123 };
    lib_test_assert(base_clock_platform_counter(&units, &frequency) == LIB_STATUS_OK);
    lib_test_assert(units == 2000000123u && frequency == 1000000000u);
    return 0;
}
