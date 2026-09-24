#include "lib/types/test.h"
#include "lib/types/file.h"
#include "lib/types/types_interface.h"

_Static_assert(sizeof(lib_i8) == 1u, "lib_i8 must remain one byte");
_Static_assert(sizeof(lib_u8) == 1u, "lib_u8 must remain one byte");
_Static_assert(sizeof(lib_uptr) == sizeof(void *),
    "lib_uptr must preserve every object-pointer bit");
#include "lib/base/clock_interface.h"

#define CHECK(expression) do { if (!(expression)) return __LINE__; } while (0)

int main(void)
{
    lib_atomic_flag lock = LIB_ATOMIC_FLAG_INITIALIZER;
    lib_atomic_i32 state;
    lib_atomic_u32 references;
    lib_atomic_u64 identity;
    lib_atomic_uptr token;
    lib_u64 expected;
    lib_uptr expected_token;
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
    CHECK(lib_pointer_to_uptr(bytes) != 0u);
    CHECK(lib_uptr_to_pointer(lib_pointer_to_uptr(bytes)) == bytes);
    CHECK(LIB_STATUS_OK == 0 && LIB_STATUS_INVALID_ARGUMENT == 1 &&
        LIB_STATUS_INVALID_STATE == 2 && LIB_STATUS_UNSUPPORTED == 3 &&
        LIB_STATUS_NO_MEMORY == 4 && LIB_STATUS_IO_ERROR == 5 &&
        LIB_STATUS_INTERNAL_ERROR == 6 && LIB_STATUS_LIMIT_EXCEEDED == 7);

    CHECK(!lib_atomic_flag_test_and_set_explicit(&lock, LIB_MEMORY_ORDER_ACQUIRE));
    CHECK(lib_atomic_flag_test_and_set_explicit(&lock, LIB_MEMORY_ORDER_ACQUIRE));
    lib_atomic_flag_clear_explicit(&lock, LIB_MEMORY_ORDER_RELEASE);
    CHECK(!lib_atomic_flag_test_and_set_explicit(&lock, LIB_MEMORY_ORDER_ACQUIRE));
    lib_atomic_i32_initialize(&state, -1);
    CHECK(lib_atomic_i32_load_explicit(&state, LIB_MEMORY_ORDER_RELAXED) == -1);
    lib_atomic_i32_store_explicit(&state, 9, LIB_MEMORY_ORDER_RELEASE);
    CHECK(lib_atomic_i32_load_explicit(&state, LIB_MEMORY_ORDER_ACQUIRE) == 9);
    lib_atomic_u32_initialize(&references, 1u);
    CHECK(lib_atomic_u32_load_explicit(&references, LIB_MEMORY_ORDER_RELAXED) == 1u);
    lib_atomic_u32_store_explicit(&references, LIB_UINT32_MAX,
        LIB_MEMORY_ORDER_RELEASE);
    CHECK(lib_atomic_u32_fetch_add_explicit(&references, 2u, LIB_MEMORY_ORDER_RELAXED) == LIB_UINT32_MAX);
    CHECK(lib_atomic_u32_fetch_sub_explicit(&references, 1u, LIB_MEMORY_ORDER_ACQ_REL) == 1u);
    identity = 0x100000001ULL;
    expected = 0u;
    CHECK(!lib_atomic_u64_compare_exchange_weak_explicit(&identity, &expected,
        7u, LIB_MEMORY_ORDER_RELAXED, LIB_MEMORY_ORDER_RELAXED));
    CHECK(expected == 0x100000001ULL);
    while (!lib_atomic_u64_compare_exchange_weak_explicit(&identity, &expected,
            7u, LIB_MEMORY_ORDER_ACQ_REL, LIB_MEMORY_ORDER_ACQUIRE)) { }
    CHECK(lib_atomic_u64_load_explicit(&identity, LIB_MEMORY_ORDER_ACQUIRE) == 7u);
    lib_atomic_uptr_initialize(&token, 17u);
    expected_token = 0u;
    CHECK(!lib_atomic_uptr_compare_exchange_strong_explicit(&token,
        &expected_token, 18u, LIB_MEMORY_ORDER_ACQ_REL,
        LIB_MEMORY_ORDER_ACQUIRE));
    CHECK(expected_token == 17u);
    CHECK(lib_atomic_uptr_compare_exchange_strong_explicit(&token,
        &expected_token, LIB_UPTR_MAX, LIB_MEMORY_ORDER_SEQ_CST,
        LIB_MEMORY_ORDER_SEQ_CST));
    CHECK(lib_atomic_uptr_load_explicit(&token, LIB_MEMORY_ORDER_ACQUIRE) ==
        LIB_UPTR_MAX);

    CHECK(base_clock_monotonic_counter(LIB_NULL, &frequency) == LIB_STATUS_INVALID_ARGUMENT);
    CHECK(base_clock_monotonic_counter(&before, LIB_NULL) == LIB_STATUS_INVALID_ARGUMENT);
    CHECK(base_clock_monotonic_counter(&before, &frequency) == LIB_STATUS_OK);
    CHECK(base_clock_monotonic_counter(&after, &next_frequency) == LIB_STATUS_OK);
    CHECK(frequency > 0u && frequency == next_frequency && after >= before);
    CHECK(base_clock_milliseconds(&milliseconds) == LIB_STATUS_OK);
    return 0;
}
