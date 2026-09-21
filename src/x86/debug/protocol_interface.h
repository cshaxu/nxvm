#ifndef X86_DEBUG_PROTOCOL_INTERFACE_H
#define X86_DEBUG_PROTOCOL_INTERFACE_H

#include "lib/types/types_interface.h"

/* Copied in-process x86 values shared by frontend and CPU adapters.
 * Not a serialized format and not a dependency on the command parser. */
#define X86_DEBUG_BYTES 32u

typedef enum x86_debug_operation {
    X86_DEBUG_READ_REGISTER,
    X86_DEBUG_WRITE_REGISTER,
    X86_DEBUG_READ_LINEAR,
    X86_DEBUG_WRITE_LINEAR,
    X86_DEBUG_READ_REAL,
    X86_DEBUG_WRITE_REAL,
    X86_DEBUG_READ_PORT,
    X86_DEBUG_WRITE_PORT,
    X86_DEBUG_GET_CODE_DEFAULT_SIZE,
    X86_DEBUG_GET_CODE_BASE,
    X86_DEBUG_GET_CPU_SNAPSHOT,
    X86_DEBUG_SET_WATCH,
    X86_DEBUG_CLEAR_WATCH,
    X86_DEBUG_GET_WATCH,
    X86_DEBUG_SET_EXECUTION_PLAN,
    X86_DEBUG_CLEAR_EXECUTION_PLAN,
    X86_DEBUG_GET_EXECUTION_RESULT
} x86_debug_operation;

typedef enum x86_debug_watch_kind {
    X86_DEBUG_WATCH_READ,
    X86_DEBUG_WATCH_WRITE,
    X86_DEBUG_WATCH_EXECUTE
} x86_debug_watch_kind;

typedef enum x86_debug_execution_plan_kind {
    X86_DEBUG_EXECUTION_NONE,
    X86_DEBUG_EXECUTION_TRACE,
    X86_DEBUG_EXECUTION_BREAK_REAL,
    X86_DEBUG_EXECUTION_BREAK_LINEAR
} x86_debug_execution_plan_kind;

typedef struct x86_debug_segment_snapshot {
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
} x86_debug_segment_snapshot;

typedef struct x86_debug_cpu_snapshot {
    x86_debug_segment_snapshot es;
    x86_debug_segment_snapshot cs;
    x86_debug_segment_snapshot ss;
    x86_debug_segment_snapshot ds;
    x86_debug_segment_snapshot fs;
    x86_debug_segment_snapshot gs;
    x86_debug_segment_snapshot tr;
    x86_debug_segment_snapshot ldtr;
    x86_debug_segment_snapshot gdtr;
    x86_debug_segment_snapshot idtr;
    lib_u32 cr0;
    lib_u32 cr2;
    lib_u32 cr3;
} x86_debug_cpu_snapshot;

typedef struct x86_debug_request {
    x86_debug_operation operation;
    lib_u32 register_id;
    lib_u32 address;
    lib_u16 segment;
    lib_u16 offset;
    lib_u16 port;
    x86_debug_watch_kind watch_kind;
    x86_debug_execution_plan_kind execution_kind;
    lib_u64 instruction_count;
    lib_u8 bytes; /* Memory payload size; port I/O explicitly requires 1 (byte). */
    lib_u8 data[X86_DEBUG_BYTES];
} x86_debug_request;

#define X86_DEBUG_ACCESS_CAPACITY 32u
typedef struct x86_debug_memory_access {
    lib_bool write;
    lib_u32 linear;
    lib_u32 bytes;
    lib_u64 data; /* Lowest-addressed up to eight bytes, little endian. */
} x86_debug_memory_access;

typedef struct x86_debug_observation {
    x86_debug_memory_access accesses[X86_DEBUG_ACCESS_CAPACITY];
    lib_u8 count;
    lib_bool truncated;
    lib_bool watch_hit;
    x86_debug_watch_kind watch_kind;
    lib_u32 watch_address;
} x86_debug_observation;

typedef struct x86_debug_response {
    lib_u32 value;
    lib_bool enabled;
    lib_u8 bytes;
    lib_u8 data[X86_DEBUG_BYTES];
    x86_debug_cpu_snapshot cpu;
    x86_debug_observation observation;
} x86_debug_response;

typedef enum x86_debug_register {
    X86_DEBUG_EAX, X86_DEBUG_ECX, X86_DEBUG_EDX, X86_DEBUG_EBX,
    X86_DEBUG_ESP, X86_DEBUG_EBP, X86_DEBUG_ESI, X86_DEBUG_EDI,
    X86_DEBUG_EIP, X86_DEBUG_EFLAGS, X86_DEBUG_ES, X86_DEBUG_CS,
    X86_DEBUG_SS, X86_DEBUG_DS, X86_DEBUG_FS, X86_DEBUG_GS,
    X86_DEBUG_CR0, X86_DEBUG_CR1, X86_DEBUG_CR2, X86_DEBUG_CR3,
    X86_DEBUG_CR4, X86_DEBUG_REGISTER_COUNT
} x86_debug_register;

#endif
