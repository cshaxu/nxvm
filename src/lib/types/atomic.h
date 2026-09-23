#ifndef LIB_TYPES_ATOMIC_H
#define LIB_TYPES_ATOMIC_H

#include "lib/types/types_interface.h"

/* This is an implementation-only atomic boundary. Public library contracts
 * expose copied values and never name compiler or platform atomic types. */
#if defined(_MSC_VER)
#include <intrin.h>

typedef volatile long lib_atomic_flag;
typedef volatile long lib_atomic_i32;
typedef volatile long lib_atomic_u32;
typedef volatile long long lib_atomic_u64;
#if LIB_UPTR_IS_64_BIT
typedef volatile long long lib_atomic_uptr;
#else
typedef volatile long lib_atomic_uptr;
#endif

typedef enum lib_memory_order
{
    LIB_MEMORY_ORDER_RELAXED,
    LIB_MEMORY_ORDER_ACQUIRE,
    LIB_MEMORY_ORDER_RELEASE,
    LIB_MEMORY_ORDER_ACQ_REL,
    LIB_MEMORY_ORDER_SEQ_CST
} lib_memory_order;

#define LIB_ATOMIC_FLAG_INITIALIZER 0L

static inline void lib_atomic_flag_clear(lib_atomic_flag *object)
{
    (void)_InterlockedExchange(object, 0L);
}

static inline int lib_atomic_flag_test_and_set_explicit(lib_atomic_flag *object,
                                                        lib_memory_order order)
{
    (void)order;
    return _InterlockedExchange(object, 1L) != 0L;
}

static inline void lib_atomic_flag_clear_explicit(lib_atomic_flag *object,
                                                  lib_memory_order order)
{
    (void)order;
    (void)_InterlockedExchange(object, 0L);
}

static inline void lib_atomic_i32_initialize(lib_atomic_i32 *object, lib_i32 value)
{
    *object = (long)value;
}

static inline lib_i32 lib_atomic_i32_load_explicit(const lib_atomic_i32 *object,
                                                   lib_memory_order order)
{
    (void)order;
    return (lib_i32)_InterlockedCompareExchange((volatile long *)object, 0L, 0L);
}

static inline void lib_atomic_i32_store_explicit(lib_atomic_i32 *object,
                                                 lib_i32 value, lib_memory_order order)
{
    (void)order;
    (void)_InterlockedExchange(object, (long)value);
}

static inline lib_i32 lib_atomic_i32_exchange_explicit(lib_atomic_i32 *object,
    lib_i32 value, lib_memory_order order)
{
    (void)order;
    return (lib_i32)_InterlockedExchange(object, (long)value);
}

static inline lib_i32 lib_atomic_i32_fetch_add_explicit(lib_atomic_i32 *object,
    lib_i32 value, lib_memory_order order)
{
    (void)order;
    return (lib_i32)_InterlockedExchangeAdd(object, (long)value);
}

static inline lib_bool lib_atomic_i32_compare_exchange_strong_explicit(
    lib_atomic_i32 *object, lib_i32 *expected, lib_i32 desired,
    lib_memory_order success_order, lib_memory_order failure_order)
{
    long observed;
    (void)success_order;
    (void)failure_order;
    observed = _InterlockedCompareExchange(object, (long)desired, (long)*expected);
    if ((lib_i32)observed == *expected) return LIB_TRUE;
    *expected = (lib_i32)observed;
    return LIB_FALSE;
}

static inline void lib_atomic_u32_initialize(lib_atomic_u32 *object, lib_u32 value)
{
    *object = (long)value;
}

static inline lib_u32 lib_atomic_u32_load_explicit(const lib_atomic_u32 *object,
    lib_memory_order order)
{
    (void)order;
    return (lib_u32)_InterlockedCompareExchange((volatile long *)object, 0L, 0L);
}

static inline void lib_atomic_u32_store_explicit(lib_atomic_u32 *object,
    lib_u32 value, lib_memory_order order)
{
    (void)order;
    (void)_InterlockedExchange(object, (long)value);
}

static inline lib_u32 lib_atomic_u32_exchange_explicit(lib_atomic_u32 *object,
    lib_u32 value, lib_memory_order order)
{
    (void)order;
    return (lib_u32)_InterlockedExchange(object, (long)value);
}

static inline lib_u32 lib_atomic_u32_fetch_add_explicit(lib_atomic_u32 *object,
                                                        lib_u32 value, lib_memory_order order)
{
    (void)order;
    return (lib_u32)_InterlockedExchangeAdd(object, (long)value);
}

static inline lib_u32 lib_atomic_u32_fetch_sub_explicit(lib_atomic_u32 *object,
                                                        lib_u32 value, lib_memory_order order)
{
    (void)order;
    return (lib_u32)_InterlockedExchangeAdd(object, -(long)value);
}

static inline void lib_atomic_u64_initialize(lib_atomic_u64 *object, lib_u64 value)
{
    *object = (long long)value;
}

static inline lib_u64 lib_atomic_u64_load_explicit(const lib_atomic_u64 *object,
                                                   lib_memory_order order)
{
    (void)order;
    return (lib_u64)_InterlockedCompareExchange64((volatile long long *)object,
                                                  0LL, 0LL);
}

static inline lib_bool lib_atomic_u64_compare_exchange_weak_explicit(
    lib_atomic_u64 *object, lib_u64 *expected, lib_u64 desired,
    lib_memory_order success_order, lib_memory_order failure_order)
{
    long long observed;
    (void)success_order;
    (void)failure_order;
    observed = _InterlockedCompareExchange64(object, (long long)desired,
                                             (long long)*expected);
    if ((lib_u64)observed == *expected)
        return LIB_TRUE;
    *expected = (lib_u64)observed;
    return LIB_FALSE;
}

static inline void lib_atomic_uptr_initialize(lib_atomic_uptr *object,
    lib_uptr value)
{
    *object = (intptr_t)value;
}

static inline lib_uptr lib_atomic_uptr_load_explicit(const lib_atomic_uptr *object,
    lib_memory_order order)
{
    (void)order;
#if LIB_UPTR_IS_64_BIT
    return (lib_uptr)_InterlockedCompareExchange64((volatile long long *)object,
        0LL, 0LL);
#else
    return (lib_uptr)_InterlockedCompareExchange((volatile long *)object, 0L, 0L);
#endif
}

static inline lib_bool lib_atomic_uptr_compare_exchange_strong_explicit(
    lib_atomic_uptr *object, lib_uptr *expected, lib_uptr desired,
    lib_memory_order success_order, lib_memory_order failure_order)
{
    lib_uptr observed;
    (void)success_order;
    (void)failure_order;
#if LIB_UPTR_IS_64_BIT
    observed = (lib_uptr)_InterlockedCompareExchange64((volatile long long *)object,
        (long long)desired, (long long)*expected);
#else
    observed = (lib_uptr)_InterlockedCompareExchange((volatile long *)object,
        (long)desired, (long)*expected);
#endif
    if (observed == *expected) return LIB_TRUE;
    *expected = observed;
    return LIB_FALSE;
}

#else
#include <stdatomic.h>

typedef atomic_flag lib_atomic_flag;
typedef atomic_int lib_atomic_i32;
typedef atomic_uint lib_atomic_u32;
typedef atomic_uint_fast64_t lib_atomic_u64;
typedef atomic_uintptr_t lib_atomic_uptr;
typedef memory_order lib_memory_order;

#define LIB_ATOMIC_FLAG_INITIALIZER ATOMIC_FLAG_INIT

#define LIB_MEMORY_ORDER_RELAXED memory_order_relaxed
#define LIB_MEMORY_ORDER_ACQUIRE memory_order_acquire
#define LIB_MEMORY_ORDER_RELEASE memory_order_release
#define LIB_MEMORY_ORDER_ACQ_REL memory_order_acq_rel
#define LIB_MEMORY_ORDER_SEQ_CST memory_order_seq_cst

#define lib_atomic_flag_clear atomic_flag_clear
#define lib_atomic_flag_test_and_set_explicit atomic_flag_test_and_set_explicit
#define lib_atomic_flag_clear_explicit atomic_flag_clear_explicit
#define lib_atomic_i32_initialize atomic_init
#define lib_atomic_i32_load_explicit atomic_load_explicit
#define lib_atomic_i32_store_explicit atomic_store_explicit
#define lib_atomic_i32_exchange_explicit atomic_exchange_explicit
#define lib_atomic_i32_fetch_add_explicit atomic_fetch_add_explicit
#define lib_atomic_i32_compare_exchange_strong_explicit atomic_compare_exchange_strong_explicit
#define lib_atomic_u32_initialize atomic_init
#define lib_atomic_u32_load_explicit atomic_load_explicit
#define lib_atomic_u32_store_explicit atomic_store_explicit
#define lib_atomic_u32_exchange_explicit atomic_exchange_explicit
#define lib_atomic_u32_fetch_add_explicit atomic_fetch_add_explicit
#define lib_atomic_u32_fetch_sub_explicit atomic_fetch_sub_explicit
#define lib_atomic_u64_initialize atomic_init
#define lib_atomic_u64_load_explicit atomic_load_explicit
#define lib_atomic_u64_compare_exchange_weak_explicit \
    atomic_compare_exchange_weak_explicit
#define lib_atomic_uptr_initialize atomic_init
#define lib_atomic_uptr_load_explicit atomic_load_explicit
#define lib_atomic_uptr_compare_exchange_strong_explicit \
    atomic_compare_exchange_strong_explicit
#endif

#endif
