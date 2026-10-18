#ifndef X86_CORE_CPU_INTERFACE_H
#define X86_CORE_CPU_INTERFACE_H
#include "lib/types/types_interface.h"
#include "x86-core/fpu/fpu_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct x86_core_cpu_entry_plan_state {
    lib_u16 cs;
    lib_u16 ds;
    lib_u16 es;
    lib_u16 ss;
    lib_u16 ip;
    lib_u16 sp;
    lib_u32 eax;
    lib_u32 ebx;
    lib_u32 ecx;
    lib_u32 edx;
    lib_u32 esi;
    lib_u32 edi;
    lib_u32 ebp;
    lib_u32 eflags;
} x86_core_cpu_entry_plan_state;

typedef struct x86_core_cpu_execution_context x86_core_cpu_execution_context;
typedef struct x86_core_cpu_prepared_entry x86_core_cpu_prepared_entry;

/* The stopped caller finishes this candidate before executing, resetting or
 * destroying CPU. Preparation does not publish registers; finish consumes it. */
lib_status x86_core_cpu_prepare_entry(x86_core_cpu_execution_context *context,
    const x86_core_cpu_entry_plan_state *state,
    x86_core_cpu_prepared_entry **out_entry);
void x86_core_cpu_finish_entry(x86_core_cpu_prepared_entry *entry,
    lib_bool commit);


/* An external-cycle address is only comparable within its named CPU space.
 * This is shared by profile configuration and the Core CPU lifecycle. */
typedef enum x86_core_cpu_external_cycle_space {
    X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY = 0,
    X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT
} x86_core_cpu_external_cycle_space;

typedef enum x86_core_cpu_profile {
    X86_CORE_CPU_PROFILE_DEFAULT = 0,
    X86_CORE_CPU_PROFILE_8086,
    X86_CORE_CPU_PROFILE_8088,
    X86_CORE_CPU_PROFILE_80186,
    X86_CORE_CPU_PROFILE_80286,
    X86_CORE_CPU_PROFILE_80386,
    /* Selected embedded SX model: CPUID 042Bh, VME/SMM, no integrated FPU. */
    X86_CORE_CPU_PROFILE_80486SX,
    /* Native CPU work is incremental; Machine admission requires qualification. */
    X86_CORE_CPU_PROFILE_V30
} x86_core_cpu_profile;

/* CPU provenance is independent of the board's physical address decoder. */
typedef enum x86_core_cpu_memory_access_provenance {
    X86_CORE_CPU_MEMORY_ACCESS_DATA = 0,
    X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH,
    X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH,
    X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ,
    X86_CORE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE
} x86_core_cpu_memory_access_provenance;

typedef struct x86_core_cpu_cache_memory {
    lib_u32 address;
    lib_bool fill_enabled;
} x86_core_cpu_cache_memory;

typedef enum x86_core_cpu_cache_query {
    X86_CORE_CPU_CACHE_LOOKUP = 0,
    X86_CORE_CPU_CACHE_RESET_LOOKUP,
    X86_CORE_CPU_CACHE_FILL_PROBE
} x86_core_cpu_cache_query;

typedef enum x86_core_cpu_cache_operation {
    X86_CORE_CPU_CACHE_WRITEBACK = 0,
    X86_CORE_CPU_CACHE_FLUSH
} x86_core_cpu_cache_operation;

typedef enum x86_core_cpu_special_cycle {
    X86_CORE_CPU_SPECIAL_NONE = 0,
    X86_CORE_CPU_SPECIAL_HALT,
    X86_CORE_CPU_SPECIAL_SHUTDOWN,
    X86_CORE_CPU_SPECIAL_STOP_GRANT
} x86_core_cpu_special_cycle;

enum {
    X86_CORE_CPU_MEMORY_CACHE_PWT = 0x08u,
    X86_CORE_CPU_MEMORY_CACHE_PCD = 0x10u
};

/* Borrowed for the CPU lifetime; one execution owner serializes all calls.
 * Memory operations complete one logical transfer, not an entire instruction.
 * Observation reads must not consume device state or publish bus effects.
 * A successful port transfer is completed only after CPU-local publication;
 * a failed transfer must cancel its own admission before returning.
 * Memory writes may retain an accepted prefix on failure; there is no storage
 * rollback. Cache-control bits are the declared PWT/PCD flags of this cycle.
 * Callbacks may propagate signals, but cannot reenter CPU execution/destruction.
 * Widths are bytes; port widths are 1, 2 or 4. No board state is exposed. */
typedef struct x86_core_cpu_bus_provider {
    lib_status (*read_memory)(void *context, lib_u32 address, void *destination,
        lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
        lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control);
    lib_status (*write_memory)(void *context, lib_u32 address, const void *source,
        lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
        lib_u8 cache_control);
    lib_status (*transfer_port)(void *context, lib_u16 port, lib_u8 bytes,
        lib_bool write, lib_u32 *value);
    void (*complete_port)(void *context, lib_u16 port, lib_u8 bytes,
        lib_bool write);
    /* True when the board-level TEST condition keeps WAIT suspended. */
    lib_bool (*wait_test_asserted)(void *context);
    /* Optional 80186 integrated-peripheral escape trap selection. */
    lib_bool (*escape_trap_enabled)(void *context);
    lib_bool (*interrupt_pending)(void *context);
    lib_status (*acknowledge_interrupt)(void *context, lib_u8 *vector);
    void (*extension_command)(void *context, lib_u8 opcode, lib_u8 modrm);
    /* Lookup resolves live A20 only; reset lookup preserves the raw address.
     * Only a fill probe checks eligibility of the complete aligned 16-byte
     * line, without reading device data. Absent callback means identity
     * addressing with automatic cache fills disabled. */
    lib_status (*query_cache_memory)(void *context, lib_u32 address,
        x86_core_cpu_cache_query query, x86_core_cpu_cache_memory *out_memory);
    /* Complete one actual external special cycle. Missing implementation is
     * not successful delivery. WBINVD requests writeback, then flush. */
    lib_status (*cache_operation)(void *context,
        x86_core_cpu_cache_operation operation);
    /* Assert/release the CPU's memory LOCK signal. All CPU memory cycles
     * inside it use the existing callbacks, including split operands and
     * page-table updates. Complete it
     * exactly once after a successful begin, also on a CPU or provider fault.
     * Both callbacks may be absent only for a sole-owner isolated bus. */
    lib_status (*begin_memory_lock)(void *context);
    void (*complete_memory_lock)(void *context, lib_bool commit);
    /* Non-failing physical SMIACT level. Assert before the first state save,
     * deassert after the last restore (or processor reset). It is not an
     * automatic cache/A20 policy or permission to reenter CPU execution. */
    void (*system_management_active)(void *context, lib_bool active);
    /* One held physical request, sampled again only while unacknowledged.
     * acknowledged=false is normal RDY wait, not failure. An acknowledged
     * call releases the request; cancellation is null-admission-safe and
     * releases any outstanding request on reset/fault/destruction. Required
     * for selected-SX HALT/shutdown/Stop Grant, never used for observation. */
    lib_status (*special_cycle)(void *context, x86_core_cpu_special_cycle cycle,
        lib_bool *acknowledged);
    void (*cancel_special_cycle)(void *context, x86_core_cpu_special_cycle cycle);
} x86_core_cpu_bus_provider;

static inline lib_i32 x86_core_cpu_profile_has_8086_semantics(
    x86_core_cpu_profile profile)
{
    return profile == X86_CORE_CPU_PROFILE_8086 ||
        profile == X86_CORE_CPU_PROFILE_8088;
}

/* Level 2 costs are relative to one completed executor refresh. Zero keeps the
 * legacy ticks_per_instruction base and disables the corresponding surcharge. */
typedef struct x86_core_cpu_instruction_timing {
    lib_u32 base_ticks;
    lib_u32 prefix_surcharge;
    lib_u32 taken_branch_surcharge;
    lib_u32 data_memory_surcharge;
    lib_u32 io_surcharge;
    lib_u32 rep_iteration_surcharge;
} x86_core_cpu_instruction_timing;

typedef enum x86_core_cpu_retirement_timing_origin {
    X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED = 0,
    X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_STRING_IO,
    X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_80386_DYNAMIC_MULTIPLY,
    X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC,
    X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_80386_SECONDARY,
    X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_80386_PRIVILEGED,
    X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_PRIMARY,
    X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK,
    X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK,
    X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_80286_FALLBACK,
    X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_80386_FALLBACK,
    X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_COMPATIBILITY,
    X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL,
    X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_80486_SOURCE,
    X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_80486_MODEL,
    /* NEC section 12 prefetched, zero-wait subtotal, not complete elapsed time. */
    X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE
} x86_core_cpu_retirement_timing_origin;

typedef enum x86_core_cpu_retirement_repeat_phase {
    X86_CORE_CPU_RETIREMENT_REPEAT_NONE = 0,
    X86_CORE_CPU_RETIREMENT_REPEAT_PRIMITIVE,
    X86_CORE_CPU_RETIREMENT_REPEAT_ZERO_COUNT,
    X86_CORE_CPU_RETIREMENT_REPEAT_FIRST,
    X86_CORE_CPU_RETIREMENT_REPEAT_CONTINUATION
} x86_core_cpu_retirement_repeat_phase;

/* The timing form is an opaque Core-owned identifier. A classified path
 * without a ledger lookup intentionally reports this sentinel. */
#define X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ((lib_u32)-1)

/* A result is cleared per CPU round; selection fills successful retirement
 * timing and committed event entry adds its separate elapsed subtotal. The
 * decoder-form key is Core-private; the S3 result verifier maps it to the
 * corresponding manifest record without exposing machine storage. */
typedef struct x86_core_cpu_timing_result {
    lib_u64 ticks;
    lib_u32 key_id;
    lib_u32 formula_inputs;
    x86_core_cpu_retirement_timing_origin retirement_origin;
    /* V30 may expose a nominal subtotal in ticks/origin while total time is
     * unallocated; that partial result cannot qualify its Machine admission. */
    lib_u8 source_timing_unallocated;
    lib_u32 form_id;
    x86_core_cpu_retirement_repeat_phase repeat_phase;
    /* Entry time is separate from a successfully retired instruction. Zero
     * ticks with nonzero event_ticks is a valid event-only CPU round. */
    lib_u64 event_ticks;
} x86_core_cpu_timing_result;

#define X86_CORE_CPU_DEVICE_NAME "Intel 8086+"

const char *x86_core_cpu_profile_name(x86_core_cpu_profile profile);

typedef struct x86_core_cpu_state {
    lib_u16 cs;
    lib_u32 cs_base;
    lib_u32 eip;
    lib_u32 eflags;
    lib_u8 halted;
} x86_core_cpu_state;

#define X86_CORE_CPU_DIAGNOSTIC_WINDOW_CAPACITY 32u
#define X86_CORE_CPU_DIAGNOSTIC_BYTES 15u

typedef struct x86_core_cpu_execution_point {
    lib_u16 cs;
    lib_u32 cs_base;
    lib_u32 eip;
    lib_u32 linear_pc;
    lib_u8 bytes[X86_CORE_CPU_DIAGNOSTIC_BYTES];
    lib_u8 byte_count;
} x86_core_cpu_execution_point;

/* Transient instruction facts, copied on the serialized CPU execution owner.
 * point.bytes is the existing fetch observation window, not decoded length.
 * No pointer in this value refers to CPU or decoder storage. */
typedef struct x86_core_cpu_instruction_observation {
    x86_core_cpu_execution_point point;
    lib_u32 old_eip;
    lib_u32 eax;
    lib_u16 dx;
    lib_u8 cpl;
    lib_bool old_default_size_32;
    lib_bool protected_mode;
    lib_bool virtual_8086_mode;
    lib_bool operand_size_32;
    lib_bool address_size_32;
    lib_bool lock_prefix;
    lib_u8 repeat_prefix;
    /* Target encoding components captured before asynchronous entry; zero
     * when no control target is applicable or its encoding is unavailable. */
    lib_u8 next_lexeme_components;
} x86_core_cpu_instruction_observation;

/* Bit masks reported by CPU fault snapshots, including non-deliverable stops. */
#define VCPUINS_EXCEPT_DE  0x00000001 /* 00 - fault: divide error */
#define VCPUINS_EXCEPT_DB  0x00000002 /* 01 - trap/fault: debug exception */
#define VCPUINS_EXCEPT_NMI 0x00000004 /* 02 - n/a:   non-maskable interrupt */
#define VCPUINS_EXCEPT_BP  0x00000008 /* 03 - trap:  break point */
#define VCPUINS_EXCEPT_OF  0x00000010 /* 04 - trap:  overflow exception */
#define VCPUINS_EXCEPT_BR  0x00000020 /* 05 - fault: boundary check fail */
#define VCPUINS_EXCEPT_UD  0x00000040 /* 06 - fault: invalid opcode */
#define VCPUINS_EXCEPT_NM  0x00000080 /* 07 - fault: coprocessor not available */
#define VCPUINS_EXCEPT_DF  0x00000100 /* 08 - double fault abort */
#define VCPUINS_EXCEPT_09  0x00000200 /* 09 - protected coprocessor segment-overrun abort */
#define VCPUINS_EXCEPT_TS  0x00000400 /* 10 - fault: task state segment fail */
#define VCPUINS_EXCEPT_NP  0x00000800 /* 11 - fault: segment not present */
#define VCPUINS_EXCEPT_SS  0x00001000 /* 12 - fault: stack segment fault */
#define VCPUINS_EXCEPT_GP  0x00002000 /* 13 - fault: general protection */
#define VCPUINS_EXCEPT_PF  0x00004000 /* 14 - fault: page fault */
#define VCPUINS_EXCEPT_15  0x00008000 /* 15 - n/a:   reserved */
#define VCPUINS_EXCEPT_MF  0x00010000 /* 16 - fault: x87 fpu floating point error */
#define VCPUINS_EXCEPT_AC  0x00020000 /* 17 - fault: alignment check */

#define VCPUINS_EXCEPT_FPU_UNSUPPORTED 0x40000000 /* internal FPU model stop */

/* 80386 real-address stack-limit wrap is an architectural shutdown, not an
 * interrupt-deliverable exception. */
#define VCPUINS_EXCEPT_SHUTDOWN 0x20000000

#define VCPUINS_EXCEPT_CE  0x80000000 /* 31 - internal case error */

typedef struct x86_core_cpu_fault_snapshot {
    lib_i32 valid;
    lib_u32 exception_mask;
    lib_u32 exception_code;
    x86_core_cpu_execution_point point;
    lib_u32 eax;
    lib_u32 ebx;
    lib_u32 ecx;
    lib_u32 edx;
    lib_u32 cr2;
    lib_u32 esp;
    lib_u16 ss;
    lib_u32 ss_base;
    lib_u32 ebp;
    lib_u32 esi;
    lib_u32 edi;
    lib_u32 eflags;
} x86_core_cpu_fault_snapshot;

/* Callback values are borrowed only for the call; consumers may copy them.
 * A null member disables that observation without constructing its snapshot. */
typedef struct x86_core_cpu_execution_diagnostic_provider {
    void (*record_instruction)(void *context,
        const x86_core_cpu_instruction_observation *observation);
    void (*record_delivered_exception)(void *context,
        const x86_core_cpu_fault_snapshot *snapshot);
    void (*record_fault)(void *context,
        const x86_core_cpu_fault_snapshot *snapshot);
} x86_core_cpu_execution_diagnostic_provider;

typedef struct x86_core_cpu_diagnostic {
    x86_core_cpu_fault_snapshot first_fault;
    x86_core_cpu_fault_snapshot first_delivered_exception;
    x86_core_cpu_fault_snapshot last_delivered_exception;
    lib_u32 delivered_exception_count;
    x86_core_cpu_execution_point recent[X86_CORE_CPU_DIAGNOSTIC_WINDOW_CAPACITY];
    lib_size recent_count;
} x86_core_cpu_diagnostic;

#define X86_CORE_CPU_DEBUG_INSTRUCTION_BYTES 15u
#define VCPU_CR0_PE 0x00000001
#define VCPU_CR0_MP 0x00000002
#define VCPU_CR0_EM 0x00000004
#define VCPU_CR0_TS 0x00000008
#define VCPU_CR0_ET 0x00000010
#define VCPU_CR0_NE 0x00000020u
#define VCPU_CR0_WP 0x00010000u
#define VCPU_CR0_AM 0x00040000u
#define VCPU_CR0_NW 0x20000000u
#define VCPU_CR0_CD 0x40000000u
#define VCPU_CR0_PG 0x80000000
#define X86_CORE_CPU_DEBUG_MEMORY_ACCESS_CAPACITY 32u
#define X86_CORE_CPU_DEBUG_EFLAGS_CF 0x00000001u
#define X86_CORE_CPU_DEBUG_EFLAGS_PF 0x00000004u
#define X86_CORE_CPU_DEBUG_EFLAGS_AF 0x00000010u
#define X86_CORE_CPU_DEBUG_EFLAGS_ZF 0x00000040u
#define X86_CORE_CPU_DEBUG_EFLAGS_SF 0x00000080u
#define X86_CORE_CPU_DEBUG_EFLAGS_TF 0x00000100u
#define X86_CORE_CPU_DEBUG_EFLAGS_IF 0x00000200u
#define X86_CORE_CPU_DEBUG_EFLAGS_DF 0x00000400u
#define X86_CORE_CPU_DEBUG_EFLAGS_OF 0x00000800u
#define X86_CORE_CPU_DEBUG_EFLAGS_NT 0x00004000u
#define X86_CORE_CPU_DEBUG_EFLAGS_RF 0x00010000u
#define X86_CORE_CPU_DEBUG_EFLAGS_VM 0x00020000u

typedef enum x86_core_cpu_debug_watch_kind {
    X86_CORE_CPU_DEBUG_WATCH_READ,
    X86_CORE_CPU_DEBUG_WATCH_WRITE,
    X86_CORE_CPU_DEBUG_WATCH_EXECUTE
} x86_core_cpu_debug_watch_kind;

typedef struct x86_core_cpu_debug_memory_access {
    lib_i32 write;
    lib_u32 linear;
    lib_u8 bytes;
    lib_u64 data;
} x86_core_cpu_debug_memory_access;

typedef struct x86_core_cpu_debug_segment_snapshot {
    lib_u16 selector;
    lib_u32 base;
    lib_u32 limit;
    lib_u8 dpl;
    lib_u8 type;
    lib_u8 accessed;
    lib_u8 executable;
    lib_u8 conform;
    lib_u8 readable;
    lib_u8 defsize;
    lib_u8 big;
    lib_u8 expdown;
    lib_u8 writable;
} x86_core_cpu_debug_segment_snapshot;

typedef struct x86_core_cpu_debug_cpu_snapshot {
    x86_core_cpu_debug_segment_snapshot es, cs, ss, ds, fs, gs;
    x86_core_cpu_debug_segment_snapshot tr, ldtr, gdtr, idtr;
    lib_u32 cr0, cr2, cr3;
    lib_u32 eax, ecx, edx, ebx, esp, ebp, esi, edi, eip, eflags;
} x86_core_cpu_debug_cpu_snapshot;

typedef enum x86_core_cpu_snapshot_point {
    X86_CORE_CPU_SNAPSHOT_CURRENT,
    /* Saved decoder entry for the current/last instruction; meaningful only
     * after instruction entry, not after construction or reset alone. */
    X86_CORE_CPU_SNAPSHOT_INSTRUCTION_ENTRY
} x86_core_cpu_snapshot_point;

/* A copied debugger record names only the fields consumed by the retained
 * debugger. It is not a CPU, decoder, or executor layout. */
typedef struct x86_core_cpu_debug_instruction_observation {
    lib_u16 cs;
    lib_u16 ss;
    lib_u16 ds;
    lib_u16 es;
    lib_u16 fs;
    lib_u16 gs;
    lib_u32 cs_base;
    lib_u32 ss_base;
    lib_u32 eip;
    lib_u32 esp;
    lib_u32 eax;
    lib_u32 ecx;
    lib_u32 edx;
    lib_u32 ebx;
    lib_u32 ebp;
    lib_u32 esi;
    lib_u32 edi;
    lib_u32 eflags;
    lib_i32 code_default_size;
    lib_u16 instruction_cs;
    lib_u32 instruction_eip;
    lib_u32 instruction_linear;
    lib_u8 instruction_bytes[X86_CORE_CPU_DEBUG_INSTRUCTION_BYTES];
    lib_u8 instruction_byte_count;
    x86_core_cpu_debug_memory_access
        memory_accesses[X86_CORE_CPU_DEBUG_MEMORY_ACCESS_CAPACITY];
    lib_u8 memory_access_count;
    lib_u8 watch_hit;
    x86_core_cpu_debug_watch_kind watch_kind;
    lib_u32 watch_address;
} x86_core_cpu_debug_instruction_observation;

typedef enum x86_core_cpu_debug_register {
    X86_CORE_CPU_DEBUG_EAX, X86_CORE_CPU_DEBUG_ECX, X86_CORE_CPU_DEBUG_EDX,
    X86_CORE_CPU_DEBUG_EBX, X86_CORE_CPU_DEBUG_ESP, X86_CORE_CPU_DEBUG_EBP,
    X86_CORE_CPU_DEBUG_ESI, X86_CORE_CPU_DEBUG_EDI, X86_CORE_CPU_DEBUG_EIP,
    X86_CORE_CPU_DEBUG_EFLAGS, X86_CORE_CPU_DEBUG_ES, X86_CORE_CPU_DEBUG_CS,
    X86_CORE_CPU_DEBUG_SS, X86_CORE_CPU_DEBUG_DS, X86_CORE_CPU_DEBUG_FS,
    X86_CORE_CPU_DEBUG_GS, X86_CORE_CPU_DEBUG_CR0, X86_CORE_CPU_DEBUG_CR1,
    X86_CORE_CPU_DEBUG_CR2, X86_CORE_CPU_DEBUG_CR3, X86_CORE_CPU_DEBUG_CR4,
    X86_CORE_CPU_DEBUG_REGISTER_COUNT
} x86_core_cpu_debug_register;

#define X86_CORE_CPU_DEBUG_REGISTER_MASK(register_id) \
    (1u << (register_id))

/* A patch names precisely the fields it may alter. Core validates the complete
 * requested set against a candidate CPU and commits it only on success. */
typedef struct x86_core_cpu_debug_register_patch {
    lib_u32 mask;
    lib_u32 values[X86_CORE_CPU_DEBUG_REGISTER_COUNT];
} x86_core_cpu_debug_register_patch;

typedef enum x86_core_cpu_instruction_space {
    X86_CORE_CPU_INSTRUCTION_PRIMARY,
    X86_CORE_CPU_INSTRUCTION_0F,
    X86_CORE_CPU_INSTRUCTION_FPU_ESCAPE,
    X86_CORE_CPU_INSTRUCTION_NEC_0F
} x86_core_cpu_instruction_space;

typedef enum x86_core_cpu_external_cycle_phase {
    X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN = 1,
    X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT,
    X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL,
    /* The Core CPU owner issued this named sequential request while the
     * preceding prefetch cycle was still in flight. */
    X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_OVERLAP_DECLARE
} x86_core_cpu_external_cycle_phase;


/* Return this matching COMMIT's extra wait clocks, not its baseline bus
 * duration. Other phases and unmatched COMMIT return zero. CPU records the
 * completed-access fact; the executor remains the sole clock publisher. */
typedef lib_u64 (*x86_core_cpu_external_cycle_provider)(void *context,
    x86_core_cpu_external_cycle_phase phase,
    x86_core_cpu_external_cycle_space space, lib_u32 address,
    lib_u8 bytes, lib_u8 write,
    x86_core_cpu_memory_access_provenance provenance);

typedef struct x86_core_cpu_instruction_metadata {
    x86_core_cpu_profile minimum_cpu;
    x86_fpu_profile minimum_fpu;
    lib_i32 valid;
} x86_core_cpu_instruction_metadata;

/* A lexical result is intentionally narrower than instruction decoding: it
 * names only the byte-layout components used by 80386 Jcc's `m` timing term.
 * It never validates operands or applies instruction semantics. */
typedef struct x86_core_cpu_instruction_lexeme {
    lib_u8 byte_count;
    lib_u8 component_count;
    lib_u8 available;
} x86_core_cpu_instruction_lexeme;

typedef enum x86_core_cpu_watchpoint {
    X86_CORE_CPU_WATCH_READ,
    X86_CORE_CPU_WATCH_WRITE,
    X86_CORE_CPU_WATCH_EXECUTE
} x86_core_cpu_watchpoint;

lib_status x86_core_cpu_create(const x86_core_cpu_bus_provider *bus,
    void *bus_context, x86_core_cpu_execution_context **out_context);
void x86_core_cpu_destroy(x86_core_cpu_execution_context *context);
/* Quiesce physical outputs before their borrowed board context is destroyed.
 * Idempotent; CPU destruction also performs it for isolated chip owners. */
void x86_core_cpu_execution_finalize(x86_core_cpu_execution_context *context);

void x86_core_cpu_execution_request_stop(
    x86_core_cpu_execution_context *context);
lib_u8 x86_core_cpu_execution_consume_stop_request(
    x86_core_cpu_execution_context *context);
void x86_core_cpu_execution_request_debug_pause(
    x86_core_cpu_execution_context *context);
lib_u8 x86_core_cpu_execution_consume_debug_pause_request(
    x86_core_cpu_execution_context *context);
void x86_core_cpu_execution_request_reset(
    x86_core_cpu_execution_context *context);
typedef enum x86_core_cpu_reset_kind {
    X86_CORE_CPU_RESET_NONE = 0,
    X86_CORE_CPU_RESET_HARD,
    X86_CORE_CPU_RESET_SOFT
} x86_core_cpu_reset_kind;

/* Serialized qualified reset requests, not raw pin pulse sampling. HARD has
 * priority over SOFT. Only the selected SL-enhanced SX accepts SRESET. */
lib_status x86_core_cpu_execution_request_soft_reset(
    x86_core_cpu_execution_context *context);
x86_core_cpu_reset_kind x86_core_cpu_execution_consume_reset_request(
    x86_core_cpu_execution_context *context);
void x86_core_cpu_execution_request_shutdown(
    x86_core_cpu_execution_context *context);
/* Resident architectural shutdown; consuming its board signal does not clear it.
 * Cleared only by processor reset or successful source-qualified NMI entry. */
lib_bool x86_core_cpu_is_shutdown(
    const x86_core_cpu_execution_context *context);
lib_u8 x86_core_cpu_execution_consume_shutdown_request(
    x86_core_cpu_execution_context *context);
void x86_core_cpu_state_initialize(
    x86_core_cpu_execution_context *context);
void x86_core_cpu_state_reset(x86_core_cpu_execution_context *context);
/* Same reset owner and effects as RESET, except SRESET retains SMBASE. */
lib_status x86_core_cpu_state_soft_reset(x86_core_cpu_execution_context *context);
void x86_core_cpu_capture_state(const x86_core_cpu_execution_context *context,
    x86_core_cpu_state *out_state);
lib_u32 x86_core_cpu_linear_pc(const x86_core_cpu_execution_context *context);
lib_bool x86_core_cpu_is_halted(const x86_core_cpu_execution_context *context);
/* A restarted automatic coprocessor wait did not retire its ESC instruction.
 * The sole Core runner consumes this one execution-round outcome before it
 * publishes any instruction time or retirement observation. */
lib_bool x86_core_cpu_execution_consume_wait_stall(
    x86_core_cpu_execution_context *context);
/* Poll the architectural TEST input after Core has advanced the one guest
 * timeline.  This only releases a completed WAIT/ESC wait; it does not fetch
 * or execute an instruction. */
void x86_core_cpu_execution_poll_wait(
    x86_core_cpu_execution_context *context);
void x86_core_cpu_execution_advance_wait(
    x86_core_cpu_execution_context *context, lib_u64 elapsed_ticks);
/* Owning executor thread: advance only from the existing guest timeline.
 * Unavailable bus or stopped CLK preserves pending write service. This L2
 * timing model never repeats a functional memory/port provider operation.
 * invalidation_cycles counts actual external cycles during pending CPU work,
 * not invalidated lines or alias notifications. Idle advancement passes zero. */
void x86_core_cpu_execution_advance_bus(
    x86_core_cpu_execution_context *context, lib_u64 elapsed_ticks,
    lib_bool bus_available, lib_u64 invalidation_cycles);
/* WAIT re-samples an asserted board condition after five clocks.  This is the
 * 8086/80186 manual formula and an explicit L2 control model for later CPUs;
 * a valid result is a Core deadline, never a host sleep or synthetic opcode. */
lib_status x86_core_cpu_execution_wait_poll_ticks(
    const x86_core_cpu_execution_context *context, lib_u64 *out_ticks);
/* External NMI mask input gates both admission and delivery. Unmasking does
 * not manufacture an edge; the board refreshes its own pending sources. */
void x86_core_cpu_set_nmi_mask(x86_core_cpu_execution_context *context,
    lib_bool masked);
lib_bool x86_core_cpu_nmi_is_masked(const x86_core_cpu_execution_context *context);
lib_bool x86_core_cpu_request_nmi(x86_core_cpu_execution_context *context);
/* Serialized, qualified SMI level; inactive-to-active latches one request.
 * The board owns pulse/synchronizer timing. Only the selected SX accepts it;
 * a bus without the SMIACT output cannot admit the signal. */
lib_status x86_core_cpu_set_smi(x86_core_cpu_execution_context *context,
    lib_bool asserted);
/* Qualified logical levels on the serialized execution owner. Board logic
 * owns pin pulse/synchronizer timing. CLK=true means restarted and PLL stable;
 * clock changes require Stop Grant and STPCLK release requires a stable CLK.
 * NMI/SMI may change while CLK is stopped; other pins must remain stable.
 * No instruction or physical output is issued by these setters. */
lib_status x86_core_cpu_set_stop_clock(x86_core_cpu_execution_context *context,
    lib_bool asserted);
lib_status x86_core_cpu_set_clock_running(x86_core_cpu_execution_context *context,
    lib_bool running);
lib_status x86_core_cpu_set_flush(x86_core_cpu_execution_context *context,
    lib_bool asserted);
lib_status x86_core_cpu_set_nmi(x86_core_cpu_execution_context *context,
    lib_bool asserted);
/* A physical system wait, not product pause, HALT or a retired instruction. */
lib_bool x86_core_cpu_is_system_waiting(const x86_core_cpu_execution_context *context);
void x86_core_cpu_execution_context_bind_profiles(
    x86_core_cpu_execution_context *context,
    x86_core_cpu_profile cpu_profile,
    x86_fpu_profile fpu_profile,
    lib_u8 cpu_80386_cr_mov_ignores_mod, const x86_core_cpu_instruction_timing *timing);

lib_i32 x86_core_cpu_read_linear(x86_core_cpu_execution_context *context,
    lib_u32 linear, void *out_data, lib_u8 size);
/* Caller serializes debug operations against execution. Machine lifecycle
 * admission belongs to the board; register validation belongs to the CPU. */
lib_status x86_core_cpu_debug_capture_snapshot(
    const x86_core_cpu_execution_context *context,
    x86_core_cpu_snapshot_point point,
    x86_core_cpu_debug_cpu_snapshot *out_snapshot);
lib_status x86_core_cpu_debug_capture_instruction(
    const x86_core_cpu_execution_context *context,
    x86_core_cpu_debug_instruction_observation *out_observation);
lib_status x86_core_cpu_debug_read_register(
    const x86_core_cpu_execution_context *context,
    x86_core_cpu_debug_register register_id, lib_u32 *out_value);
/* Register publication is atomic after read-only validation. A physical
 * provider failure during commit retains its accepted write prefix; callers
 * must not replay it. Descriptor effects use the live CPU cache owner. */
lib_status x86_core_cpu_debug_patch_registers(
    x86_core_cpu_execution_context *context,
    const x86_core_cpu_debug_register_patch *patch);
lib_i32 x86_core_cpu_write_linear(x86_core_cpu_execution_context *context,
    lib_u32 linear, const void *in_data, lib_u8 size);
lib_i32 x86_core_cpu_get_code_default_size(
    const x86_core_cpu_execution_context *context);
lib_u32 x86_core_cpu_get_code_base(
    const x86_core_cpu_execution_context *context);
void x86_core_cpu_set_watchpoint(x86_core_cpu_execution_context *context,
    x86_core_cpu_watchpoint kind, lib_u32 linear);
void x86_core_cpu_clear_watchpoint(x86_core_cpu_execution_context *context,
    x86_core_cpu_watchpoint kind);
void x86_core_cpu_get_watchpoint(const x86_core_cpu_execution_context *context,
    x86_core_cpu_watchpoint kind, lib_u8 *out_enabled,
    lib_u32 *out_linear);

/* Within a refresh this reports live instruction preparation; after successful
 * selection it preserves the completed facts across asynchronous entry until
 * the next refresh/reset. It does not expose live architectural storage. */
void x86_core_cpu_execution_copy_observation(
    const x86_core_cpu_execution_context *context,
    x86_core_cpu_instruction_observation *observation);

void x86_core_cpu_execution_context_bind_diagnostic_provider(
    x86_core_cpu_execution_context *context,
    const x86_core_cpu_execution_diagnostic_provider *provider,
    void *provider_context);

void x86_core_cpu_execution_context_bind_fpu(
    x86_core_cpu_execution_context *context, x86_fpu *fpu);

void x86_core_cpu_execution_context_bind_external_cycle_provider(
    x86_core_cpu_execution_context *context,
    x86_core_cpu_external_cycle_provider provider, void *provider_context);

void x86_core_cpu_execution_reserve_prefetch(
    x86_core_cpu_execution_context *context);

void x86_core_cpu_execution_advance_prefetch_reservation(
    x86_core_cpu_execution_context *context);

void x86_core_cpu_execution_invalidate_prefetch(
    x86_core_cpu_execution_context *context);

/* Owning executor thread, CLK running only. Physical external-write snoop
 * ignores A20M, respects NW and changes only matching cache valid bits, not
 * tags/data. Board logic must not issue EADS while CLK is stopped. */
void x86_core_cpu_signal_cache_write(x86_core_cpu_execution_context *context,
    lib_u32 physical, lib_size bytes);

void x86_core_cpu_execution_refresh(
    x86_core_cpu_execution_context *context);

lib_u8 x86_core_cpu_execution_consume_instruction_fault_delivery(
    x86_core_cpu_execution_context *context);

x86_core_cpu_instruction_metadata x86_core_cpu_instruction_metadata_get(
    x86_core_cpu_instruction_space space, lib_u8 opcode, lib_u8 modrm);

lib_u8 x86_core_cpu_instruction_lexeme_scan(
    const lib_u8 *bytes, lib_u8 available_bytes,
    x86_core_cpu_profile profile, lib_u8 code_32,
    x86_core_cpu_instruction_lexeme *out_lexeme);

lib_u8 x86_core_cpu_execution_preview_lexeme(
    const x86_core_cpu_execution_context *context,
    x86_core_cpu_instruction_lexeme *out_lexeme);

#define X86_CORE_CPU_TIMING_INPUT_MODRM      (1u << 0)
#define X86_CORE_CPU_TIMING_INPUT_CONTROL    (1u << 1)
#define X86_CORE_CPU_TIMING_INPUT_REPEAT     (1u << 2)
#define X86_CORE_CPU_TIMING_INPUT_MODE       (1u << 3)
#define X86_CORE_CPU_TIMING_INPUT_SIZE       (1u << 4)
#define X86_CORE_CPU_TIMING_INPUT_LOCK       (1u << 5)
#define X86_CORE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS (1u << 6)
#define X86_CORE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE (1u << 7)
#define X86_CORE_CPU_TIMING_INPUT_ODD_WORD   (1u << 8)
#define X86_CORE_CPU_TIMING_INPUT_REPEAT_PHASE (1u << 9)
#define X86_CORE_CPU_TIMING_INPUT_GROUP3_OPERAND (1u << 10)
#define X86_CORE_CPU_TIMING_INPUT_BRANCH     (1u << 11)
#define X86_CORE_CPU_TIMING_INPUT_BRANCH_TAKEN (1u << 12)
/* Actual SX timing participation, not hardware accuracy or eligibility. */
#define X86_CORE_CPU_TIMING_INPUT_CACHE      (1u << 13)
#define X86_CORE_CPU_TIMING_INPUT_FETCH      (1u << 14)
#define X86_CORE_CPU_TIMING_INPUT_MEMORY     (1u << 15)
#define X86_CORE_CPU_TIMING_INPUT_TRANSLATION (1u << 16)
#define X86_CORE_CPU_TIMING_INPUT_DECODE     (1u << 17)
#define X86_CORE_CPU_TIMING_INPUT_BUS        (1u << 18)

/* Timing is selected before asynchronous interrupt entry overwrites decode
 * state. Zero ticks means this refresh has no classified retirement. */
x86_core_cpu_timing_result x86_core_cpu_capture_timing(
    const x86_core_cpu_execution_context *context);
/* Shared checked accumulation for timing selection and the retained run loop. */
lib_i32 x86_core_cpu_timing_add_ticks(lib_u64 *value,
    lib_u64 delta);
/* Zero means no finite static bound, including SX provider/FIFO latency.
 * The executor must transport actual completion through its budget owner. */
lib_u64 x86_core_cpu_timing_maximum_ticks(
    x86_core_cpu_profile profile,
    const x86_core_cpu_instruction_timing *timing);

#ifdef __cplusplus
}
#endif

#endif
