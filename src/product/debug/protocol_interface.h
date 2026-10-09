#ifndef PRODUCT_DEBUG_PROTOCOL_INTERFACE_H
#define PRODUCT_DEBUG_PROTOCOL_INTERFACE_H

#include "lib/types/types_interface.h"

/* Copied in-process x86 values shared by frontend and CPU adapters.
 * Not a serialized format and not a dependency on the command parser. */
#define PRODUCT_DEBUG_BYTES 32u

typedef enum product_debug_operation {
    PRODUCT_DEBUG_READ_REGISTER,
    PRODUCT_DEBUG_WRITE_REGISTER,
    PRODUCT_DEBUG_READ_LINEAR,
    PRODUCT_DEBUG_WRITE_LINEAR,
    PRODUCT_DEBUG_READ_REAL,
    PRODUCT_DEBUG_WRITE_REAL,
    PRODUCT_DEBUG_READ_PORT,
    PRODUCT_DEBUG_WRITE_PORT,
    PRODUCT_DEBUG_GET_CODE_DEFAULT_SIZE,
    PRODUCT_DEBUG_GET_CODE_BASE,
    PRODUCT_DEBUG_GET_CPU_SNAPSHOT,
    PRODUCT_DEBUG_SET_WATCH,
    PRODUCT_DEBUG_CLEAR_WATCH,
    PRODUCT_DEBUG_GET_WATCH,
    PRODUCT_DEBUG_SET_EXECUTION_PLAN,
    PRODUCT_DEBUG_CLEAR_EXECUTION_PLAN,
    PRODUCT_DEBUG_GET_EXECUTION_RESULT
} product_debug_operation;

typedef enum product_debug_watch_kind {
    PRODUCT_DEBUG_WATCH_READ,
    PRODUCT_DEBUG_WATCH_WRITE,
    PRODUCT_DEBUG_WATCH_EXECUTE
} product_debug_watch_kind;

typedef enum product_debug_execution_plan_kind {
    PRODUCT_DEBUG_EXECUTION_NONE,
    PRODUCT_DEBUG_EXECUTION_TRACE,
    PRODUCT_DEBUG_EXECUTION_BREAK_REAL,
    PRODUCT_DEBUG_EXECUTION_BREAK_LINEAR
} product_debug_execution_plan_kind;

typedef struct product_debug_segment_snapshot {
    lib_u16 selector;
    lib_u32 base;
    lib_u32 limit;
    lib_u8 dpl;
    lib_u8 type;
    lib_bool accessed;
    lib_bool executable;
    lib_bool conform;
    lib_bool readable;
    lib_bool defsize;
    lib_bool big;
    lib_bool expdown;
    lib_bool writable;
} product_debug_segment_snapshot;

typedef struct product_debug_cpu_snapshot {
    product_debug_segment_snapshot es;
    product_debug_segment_snapshot cs;
    product_debug_segment_snapshot ss;
    product_debug_segment_snapshot ds;
    product_debug_segment_snapshot fs;
    product_debug_segment_snapshot gs;
    product_debug_segment_snapshot tr;
    product_debug_segment_snapshot ldtr;
    product_debug_segment_snapshot gdtr;
    product_debug_segment_snapshot idtr;
    lib_u32 cr0;
    lib_u32 cr2;
    lib_u32 cr3;
} product_debug_cpu_snapshot;

typedef struct product_debug_request {
    product_debug_operation operation;
    lib_u32 register_id;
    lib_u32 address;
    lib_u16 segment;
    lib_u16 offset;
    lib_u16 port;
    product_debug_watch_kind watch_kind;
    product_debug_execution_plan_kind execution_kind;
    lib_u64 instruction_count;
    lib_u8 bytes; /* Memory payload size; port I/O explicitly requires 1 (byte). */
    lib_u8 data[PRODUCT_DEBUG_BYTES];
} product_debug_request;

#define PRODUCT_DEBUG_ACCESS_CAPACITY 32u
typedef struct product_debug_memory_access {
    lib_bool write;
    lib_u32 linear;
    lib_u32 bytes;
    lib_u64 data; /* Lowest-addressed up to eight bytes, little endian. */
} product_debug_memory_access;

typedef struct product_debug_observation {
    product_debug_memory_access accesses[PRODUCT_DEBUG_ACCESS_CAPACITY];
    lib_u8 count;
    lib_bool truncated;
    lib_bool watch_hit;
    product_debug_watch_kind watch_kind;
    lib_u32 watch_address;
} product_debug_observation;

typedef struct product_debug_response {
    lib_u32 value;
    lib_bool enabled;
    lib_u8 bytes;
    lib_u8 data[PRODUCT_DEBUG_BYTES];
    product_debug_cpu_snapshot cpu;
    product_debug_observation observation;
} product_debug_response;

typedef enum product_debug_register {
    PRODUCT_DEBUG_EAX, PRODUCT_DEBUG_ECX, PRODUCT_DEBUG_EDX, PRODUCT_DEBUG_EBX,
    PRODUCT_DEBUG_ESP, PRODUCT_DEBUG_EBP, PRODUCT_DEBUG_ESI, PRODUCT_DEBUG_EDI,
    PRODUCT_DEBUG_EIP, PRODUCT_DEBUG_EFLAGS, PRODUCT_DEBUG_ES, PRODUCT_DEBUG_CS,
    PRODUCT_DEBUG_SS, PRODUCT_DEBUG_DS, PRODUCT_DEBUG_FS, PRODUCT_DEBUG_GS,
    PRODUCT_DEBUG_CR0, PRODUCT_DEBUG_CR1, PRODUCT_DEBUG_CR2, PRODUCT_DEBUG_CR3,
    PRODUCT_DEBUG_CR4, PRODUCT_DEBUG_REGISTER_COUNT
} product_debug_register;

#endif
