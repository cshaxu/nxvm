#include "lib/types/types_interface.h"
#include "lib/host/clock_interface.h"

#define CHECK(expression) do { if (!(expression)) return __LINE__; } while (0)

int main(void)
{
    lib_atomic_flag lock = LIB_ATOMIC_FLAG_INITIALIZER;
    lib_atomic_i32 state;
    lib_atomic_u32 references;
    lib_atomic_u64 identity;
    lib_u64 expected;
    lib_u64 before, after, frequency, next_frequency, milliseconds;
    char bytes[4];
    char *copy = lib_allocate_zero(4u, 1u);

    CHECK(copy != LIB_NULL);
    lib_memory_copy(bytes, "abc", 4u);
    lib_memory_copy(copy, bytes, 4u);
    CHECK(lib_memory_compare(copy, bytes, 4u) == 0);
    CHECK(lib_text_length(copy) == 3u);
    CHECK(lib_memory_find(copy, 'b', 4u) == copy + 1);
    lib_release(copy);

    CHECK(!lib_atomic_flag_test_and_set_explicit(&lock, LIB_MEMORY_ORDER_ACQUIRE));
    CHECK(lib_atomic_flag_test_and_set_explicit(&lock, LIB_MEMORY_ORDER_ACQUIRE));
    lib_atomic_flag_clear_explicit(&lock, LIB_MEMORY_ORDER_RELEASE);
    CHECK(!lib_atomic_flag_test_and_set_explicit(&lock, LIB_MEMORY_ORDER_ACQUIRE));
    lib_atomic_flag_clear(&lock);
    lib_atomic_i32_initialize(&state, -1);
    CHECK(lib_atomic_i32_load_explicit(&state, LIB_MEMORY_ORDER_RELAXED) == -1);
    lib_atomic_i32_store_explicit(&state, 9, LIB_MEMORY_ORDER_RELEASE);
    CHECK(lib_atomic_i32_load_explicit(&state, LIB_MEMORY_ORDER_ACQUIRE) == 9);
    lib_atomic_u32_initialize(&references, 1u);
    CHECK(lib_atomic_u32_fetch_add_explicit(&references, 2u, LIB_MEMORY_ORDER_RELAXED) == 1u);
    CHECK(lib_atomic_u32_fetch_sub_explicit(&references, 1u, LIB_MEMORY_ORDER_ACQ_REL) == 3u);
    lib_atomic_u64_initialize(&identity, 0x100000001ULL);
    expected = 0u;
    CHECK(!lib_atomic_u64_compare_exchange_weak_explicit(&identity, &expected,
        7u, LIB_MEMORY_ORDER_RELAXED, LIB_MEMORY_ORDER_RELAXED));
    CHECK(expected == 0x100000001ULL);
    while (!lib_atomic_u64_compare_exchange_weak_explicit(&identity, &expected,
            7u, LIB_MEMORY_ORDER_ACQ_REL, LIB_MEMORY_ORDER_ACQUIRE)) { }
    CHECK(lib_atomic_u64_load_explicit(&identity, LIB_MEMORY_ORDER_ACQUIRE) == 7u);

    CHECK(host_clock_monotonic_counter(LIB_NULL, &frequency) == LIB_STATUS_INVALID_ARGUMENT);
    CHECK(host_clock_monotonic_counter(&before, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    CHECK(host_clock_monotonic_counter(&before, &frequency) == LIB_STATUS_OK);
    CHECK(host_clock_monotonic_counter(&after, &next_frequency) == LIB_STATUS_OK);
    CHECK(frequency > 0u && frequency == next_frequency && after >= before);
    CHECK(host_clock_milliseconds(&milliseconds) == LIB_STATUS_OK);
    return 0;
}
