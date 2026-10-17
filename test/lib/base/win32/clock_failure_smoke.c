#include "lib/types/test.h"
#include "lib/types/types_interface.h"
#include "lib/types/win32/scalar.h"
#include "lib/types/win32/clock.h"
static lib_i32 counter_ok = 1, frequency_ok = 1;
static lib_win32_longlong counter_value = 123, frequency_value = 1000;
static lib_u32 query_count;
static lib_win32_bool fake_counter(lib_win32_counter *out)
{ ++query_count; out->QuadPart = counter_value; return counter_ok; }
static lib_win32_bool fake_frequency(lib_win32_counter *out)
{ ++query_count; out->QuadPart = frequency_value; return frequency_ok; }
#undef lib_win32_query_performance_counter
#undef lib_win32_query_performance_frequency
#define lib_win32_query_performance_counter fake_counter
#define lib_win32_query_performance_frequency fake_frequency
#include "lib/base/win32/clock.c"
#define CHECK(expression) do { if (!(expression)) return __LINE__; } while (0)

int main(void)
{
    lib_u64 units = 999u, frequency = 888u;
    CHECK(base_clock_platform_counter(LIB_NULL, &frequency) == LIB_STATUS_IO_ERROR);
    CHECK(base_clock_platform_counter(&units, LIB_NULL) == LIB_STATUS_IO_ERROR);
    CHECK(query_count == 0u);
    counter_ok = 0;
    CHECK(base_clock_platform_counter(&units, &frequency) == LIB_STATUS_IO_ERROR);
    CHECK(units == 999u && frequency == 888u && query_count == 1u);
    counter_ok = 1; frequency_ok = 0;
    CHECK(base_clock_platform_counter(&units, &frequency) == LIB_STATUS_IO_ERROR);
    frequency_ok = 1; frequency_value = 0;
    CHECK(base_clock_platform_counter(&units, &frequency) == LIB_STATUS_IO_ERROR);
    frequency_value = -1;
    CHECK(base_clock_platform_counter(&units, &frequency) == LIB_STATUS_IO_ERROR);
    frequency_value = 1000; counter_value = -1;
    CHECK(base_clock_platform_counter(&units, &frequency) == LIB_STATUS_IO_ERROR);
    CHECK(units == 999u && frequency == 888u);
    counter_value = 123;
    CHECK(base_clock_platform_counter(&units, &frequency) == LIB_STATUS_OK);
    CHECK(units == 123u && frequency == 1000u);
    return 0;
}
