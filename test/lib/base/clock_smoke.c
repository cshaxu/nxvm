#include "lib/types/test.h"
#include "lib/base/clock_interface.h"
#define CHECK(expression) do { if (!(expression)) return __LINE__; } while (0)
int main(void)
{
    lib_u64 before, after, frequency, next_frequency, milliseconds;
    CHECK(base_clock_monotonic_counter(LIB_NULL, &frequency) == LIB_STATUS_INVALID_ARGUMENT);
    CHECK(base_clock_monotonic_counter(&before, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    CHECK(base_clock_monotonic_counter(&before, &frequency) == LIB_STATUS_OK);
    CHECK(base_clock_monotonic_counter(&after, &next_frequency) == LIB_STATUS_OK);
    CHECK(frequency > 0u && frequency == next_frequency && after >= before);
    CHECK(base_clock_milliseconds(&milliseconds) == LIB_STATUS_OK);
    return 0;
}
