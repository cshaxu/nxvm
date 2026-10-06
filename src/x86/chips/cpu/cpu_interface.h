#ifndef CORE_MACHINE_CPU_INTERFACE_H
#define CORE_MACHINE_CPU_INTERFACE_H
#include "lib/types/types_interface.h"
#include "x86/chips/fpu/fpu_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine_entry_plan_state {
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
} core_machine_entry_plan_state;

typedef struct core_machine_cpu_execution_context core_machine_cpu_execution_context;
typedef struct core_machine_cpu_prepared_entry core_machine_cpu_prepared_entry;

/* The stopped caller finishes this candidate before executing, resetting or
 * destroying CPU. Preparation does not publish registers; finish consumes it. */
lib_status core_machine_cpu_prepare_entry(core_machine_cpu_execution_context *context,
    const core_machine_entry_plan_state *state,
    core_machine_cpu_prepared_entry **out_entry);
void core_machine_cpu_finish_entry(core_machine_cpu_prepared_entry *entry,
    lib_bool commit);


/* An external-cycle address is only comparable within its named CPU space.
 * This is shared by profile configuration and the Core CPU lifecycle. */
typedef enum core_machine_cpu_external_cycle_space {
    CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY = 0,
    CORE_MACHINE_CPU_EXTERNAL_CYCLE_SPACE_PORT
} core_machine_cpu_external_cycle_space;

typedef enum core_machine_cpu_profile {
    CORE_MACHINE_CPU_PROFILE_DEFAULT = 0,
    CORE_MACHINE_CPU_PROFILE_8086,
    CORE_MACHINE_CPU_PROFILE_8088,
    CORE_MACHINE_CPU_PROFILE_80186,
    CORE_MACHINE_CPU_PROFILE_80286,
    CORE_MACHINE_CPU_PROFILE_80386
} core_machine_cpu_profile;

/* CPU provenance is independent of the board's physical address decoder. */
typedef enum core_machine_cpu_memory_access_provenance {
    CORE_MACHINE_CPU_MEMORY_ACCESS_DATA = 0,
    CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH,
    CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH,
    CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ,
    CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE
} core_machine_cpu_memory_access_provenance;

/* Borrowed for the CPU lifetime; one execution owner serializes all calls.
 * Memory operations complete one logical transfer, not an entire instruction.
 * Observation reads must not consume device state or publish bus effects.
 * A successful port transfer is completed only after CPU-local publication;
 * a failed transfer must cancel its own admission before returning.
 * Callbacks may propagate signals, but cannot reenter CPU execution/destruction.
 * Widths are bytes; port widths are 1, 2 or 4. No board state is exposed. */
typedef struct core_machine_cpu_bus_provider {
    lib_status (*read_memory)(void *context, lib_u32 address, void *destination,
        lib_u8 bytes, core_machine_cpu_memory_access_provenance provenance,
        lib_bool observe_only, lib_bool reset_fetch);
    lib_status (*write_memory)(void *context, lib_u32 address, const void *source,
        lib_u8 bytes, core_machine_cpu_memory_access_provenance provenance);
    lib_status (*transfer_port)(void *context, lib_u16 port, lib_u8 bytes,
        lib_bool write, lib_u32 *value);
    void (*complete_port)(void *context, lib_u16 port, lib_u8 bytes,
        lib_bool write);
    lib_bool (*interrupt_pending)(void *context);
    lib_status (*acknowledge_interrupt)(void *context, lib_u8 *vector);
    void (*extension_command)(void *context, lib_u8 opcode, lib_u8 modrm);
} core_machine_cpu_bus_provider;

static inline lib_i32 core_machine_cpu_profile_has_8086_semantics(
    core_machine_cpu_profile profile)
{
    return profile == CORE_MACHINE_CPU_PROFILE_8086 ||
        profile == CORE_MACHINE_CPU_PROFILE_8088;
}

/* Level 2 costs are relative to one completed executor refresh. Zero keeps the
 * legacy ticks_per_instruction base and disables the corresponding surcharge. */
typedef struct core_machine_instruction_timing {
    lib_u32 base_ticks;
    lib_u32 prefix_surcharge;
    lib_u32 taken_branch_surcharge;
    lib_u32 data_memory_surcharge;
    lib_u32 io_surcharge;
    lib_u32 rep_iteration_surcharge;
} core_machine_instruction_timing;

typedef enum core_machine_retirement_timing_origin {
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED = 0,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80386_DYNAMIC_MULTIPLY,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80386_SECONDARY,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80386_PRIVILEGED,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80286_FALLBACK,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80386_FALLBACK,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_COMPATIBILITY,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_CONTROL_MODEL
} core_machine_retirement_timing_origin;

typedef enum core_machine_retirement_repeat_phase {
    CORE_MACHINE_RETIREMENT_REPEAT_NONE = 0,
    CORE_MACHINE_RETIREMENT_REPEAT_PRIMITIVE,
    CORE_MACHINE_RETIREMENT_REPEAT_ZERO_COUNT,
    CORE_MACHINE_RETIREMENT_REPEAT_FIRST,
    CORE_MACHINE_RETIREMENT_REPEAT_CONTINUATION
} core_machine_retirement_repeat_phase;

/* The timing form is an opaque Core-owned identifier. A classified path
 * without a ledger lookup intentionally reports this sentinel. */
#define CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ((lib_u32)-1)

/* A result is built once for every successful CPU timing selection.  The
 * decoder-form key is Core-private; the S3 result verifier maps it to the
 * corresponding manifest record without exposing machine storage. */
typedef struct core_machine_cpu_timing_result {
    lib_u64 ticks;
    lib_u32 key_id;
    lib_u32 formula_inputs;
    core_machine_retirement_timing_origin retirement_origin;
    lib_u8 source_timing_unallocated;
    lib_u32 form_id;
    core_machine_retirement_repeat_phase repeat_phase;
} core_machine_cpu_timing_result;

#define CORE_MACHINE_CPU_DEVICE_NAME "Intel 8086+"

const char *core_machine_cpu_profile_name(core_machine_cpu_profile profile);

typedef struct core_machine_cpu_state {
    lib_u16 cs;
    lib_u32 cs_base;
    lib_u32 eip;
    lib_u32 eflags;
    lib_u8 halted;
} core_machine_cpu_state;

#define CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY 32u
#define CORE_MACHINE_CPU_DIAGNOSTIC_BYTES 15u

typedef struct core_machine_cpu_execution_point {
    lib_u16 cs;
    lib_u32 cs_base;
    lib_u32 eip;
    lib_u32 linear_pc;
    lib_u8 bytes[CORE_MACHINE_CPU_DIAGNOSTIC_BYTES];
    lib_u8 byte_count;
} core_machine_cpu_execution_point;

/* Transient instruction facts, copied on the serialized CPU execution owner.
 * point.bytes is the existing fetch observation window, not decoded length.
 * No pointer in this value refers to CPU or decoder storage. */
typedef struct core_machine_cpu_instruction_observation {
    core_machine_cpu_execution_point point;
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
} core_machine_cpu_instruction_observation;

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
#define VCPUINS_EXCEPT_09  0x00000200 /* 09 - abort: reserved */
#define VCPUINS_EXCEPT_TS  0x00000400 /* 10 - fault: task state segment fail */
#define VCPUINS_EXCEPT_NP  0x00000800 /* 11 - fault: segment not present */
#define VCPUINS_EXCEPT_SS  0x00001000 /* 12 - fault: stack segment fault */
#define VCPUINS_EXCEPT_GP  0x00002000 /* 13 - fault: general protection */
#define VCPUINS_EXCEPT_PF  0x00004000 /* 14 - fault: page fault */
#define VCPUINS_EXCEPT_15  0x00008000 /* 15 - n/a:   reserved */
#define VCPUINS_EXCEPT_MF  0x00010000 /* 16 - fault: x87 fpu floating point error */

#define VCPUINS_EXCEPT_FPU_UNSUPPORTED 0x40000000 /* internal FPU model stop */

/* 80386 real-address stack-limit wrap is an architectural shutdown, not an
 * interrupt-deliverable exception. */
#define VCPUINS_EXCEPT_SHUTDOWN 0x20000000

#define VCPUINS_EXCEPT_CE  0x80000000 /* 31 - internal case error */

typedef struct core_machine_cpu_fault_snapshot {
    lib_i32 valid;
    lib_u32 exception_mask;
    lib_u32 exception_code;
    core_machine_cpu_execution_point point;
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
} core_machine_cpu_fault_snapshot;

/* Callback values are borrowed only for the call; consumers may copy them.
 * A null member disables that observation without constructing its snapshot. */
typedef struct core_machine_cpu_execution_diagnostic_provider {
    void (*record_instruction)(void *context,
        const core_machine_cpu_instruction_observation *observation);
    void (*record_delivered_exception)(void *context,
        const core_machine_cpu_fault_snapshot *snapshot);
    void (*record_fault)(void *context,
        const core_machine_cpu_fault_snapshot *snapshot);
} core_machine_cpu_execution_diagnostic_provider;

typedef struct core_machine_cpu_diagnostic {
    core_machine_cpu_fault_snapshot first_fault;
    core_machine_cpu_fault_snapshot first_delivered_exception;
    core_machine_cpu_fault_snapshot last_delivered_exception;
    lib_u32 delivered_exception_count;
    core_machine_cpu_execution_point recent[CORE_MACHINE_CPU_DIAGNOSTIC_WINDOW_CAPACITY];
    lib_size recent_count;
} core_machine_cpu_diagnostic;

#define CORE_MACHINE_DEBUG_INSTRUCTION_BYTES 15u
#define VCPU_CR0_PE 0x00000001
#define VCPU_CR0_MP 0x00000002
#define VCPU_CR0_EM 0x00000004
#define VCPU_CR0_TS 0x00000008
#define VCPU_CR0_ET 0x00000010
#define VCPU_CR0_PG 0x80000000
#define CORE_MACHINE_DEBUG_MEMORY_ACCESS_CAPACITY 32u
#define CORE_MACHINE_DEBUG_EFLAGS_CF 0x00000001u
#define CORE_MACHINE_DEBUG_EFLAGS_PF 0x00000004u
#define CORE_MACHINE_DEBUG_EFLAGS_AF 0x00000010u
#define CORE_MACHINE_DEBUG_EFLAGS_ZF 0x00000040u
#define CORE_MACHINE_DEBUG_EFLAGS_SF 0x00000080u
#define CORE_MACHINE_DEBUG_EFLAGS_TF 0x00000100u
#define CORE_MACHINE_DEBUG_EFLAGS_IF 0x00000200u
#define CORE_MACHINE_DEBUG_EFLAGS_DF 0x00000400u
#define CORE_MACHINE_DEBUG_EFLAGS_OF 0x00000800u
#define CORE_MACHINE_DEBUG_EFLAGS_NT 0x00004000u
#define CORE_MACHINE_DEBUG_EFLAGS_RF 0x00010000u
#define CORE_MACHINE_DEBUG_EFLAGS_VM 0x00020000u

typedef enum core_machine_debug_watch_kind {
    CORE_MACHINE_DEBUG_WATCH_READ,
    CORE_MACHINE_DEBUG_WATCH_WRITE,
    CORE_MACHINE_DEBUG_WATCH_EXECUTE
} core_machine_debug_watch_kind;

typedef struct core_machine_debug_memory_access {
    lib_i32 write;
    lib_u32 linear;
    lib_u8 bytes;
    lib_u64 data;
} core_machine_debug_memory_access;

typedef struct core_machine_debug_segment_snapshot {
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
} core_machine_debug_segment_snapshot;

typedef struct core_machine_debug_cpu_snapshot {
    core_machine_debug_segment_snapshot es, cs, ss, ds, fs, gs;
    core_machine_debug_segment_snapshot tr, ldtr, gdtr, idtr;
    lib_u32 cr0, cr2, cr3;
    lib_u32 eax, ecx, edx, ebx, esp, ebp, esi, edi, eip, eflags;
} core_machine_debug_cpu_snapshot;

typedef enum core_machine_cpu_snapshot_point {
    CORE_MACHINE_CPU_SNAPSHOT_CURRENT,
    /* Saved decoder entry for the current/last instruction; meaningful only
     * after instruction entry, not after construction or reset alone. */
    CORE_MACHINE_CPU_SNAPSHOT_INSTRUCTION_ENTRY
} core_machine_cpu_snapshot_point;

/* A copied debugger record names only the fields consumed by the retained
 * debugger. It is not a CPU, decoder, or executor layout. */
typedef struct core_machine_debug_instruction_observation {
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
    lib_u8 instruction_bytes[CORE_MACHINE_DEBUG_INSTRUCTION_BYTES];
    lib_u8 instruction_byte_count;
    core_machine_debug_memory_access
        memory_accesses[CORE_MACHINE_DEBUG_MEMORY_ACCESS_CAPACITY];
    lib_u8 memory_access_count;
    lib_u8 watch_hit;
    core_machine_debug_watch_kind watch_kind;
    lib_u32 watch_address;
} core_machine_debug_instruction_observation;

typedef enum core_machine_debug_register {
    CORE_MACHINE_DEBUG_EAX, CORE_MACHINE_DEBUG_ECX, CORE_MACHINE_DEBUG_EDX,
    CORE_MACHINE_DEBUG_EBX, CORE_MACHINE_DEBUG_ESP, CORE_MACHINE_DEBUG_EBP,
    CORE_MACHINE_DEBUG_ESI, CORE_MACHINE_DEBUG_EDI, CORE_MACHINE_DEBUG_EIP,
    CORE_MACHINE_DEBUG_EFLAGS, CORE_MACHINE_DEBUG_ES, CORE_MACHINE_DEBUG_CS,
    CORE_MACHINE_DEBUG_SS, CORE_MACHINE_DEBUG_DS, CORE_MACHINE_DEBUG_FS,
    CORE_MACHINE_DEBUG_GS, CORE_MACHINE_DEBUG_CR0, CORE_MACHINE_DEBUG_CR1,
    CORE_MACHINE_DEBUG_CR2, CORE_MACHINE_DEBUG_CR3, CORE_MACHINE_DEBUG_CR4,
    CORE_MACHINE_DEBUG_REGISTER_COUNT
} core_machine_debug_register;

#define CORE_MACHINE_DEBUG_REGISTER_MASK(register_id) \
    (1u << (register_id))

/* A patch names precisely the fields it may alter. Core validates the complete
 * requested set against a candidate CPU and commits it only on success. */
typedef struct core_machine_debug_register_patch {
    lib_u32 mask;
    lib_u32 values[CORE_MACHINE_DEBUG_REGISTER_COUNT];
} core_machine_debug_register_patch;

typedef enum core_machine_cpu_instruction_space {
    CORE_MACHINE_CPU_INSTRUCTION_PRIMARY,
    CORE_MACHINE_CPU_INSTRUCTION_0F,
    CORE_MACHINE_CPU_INSTRUCTION_FPU_ESCAPE
} core_machine_cpu_instruction_space;

typedef enum core_machine_cpu_external_cycle_phase {
    CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN = 1,
    CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT,
    CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL,
    /* The Core CPU owner issued this named sequential request while the
     * preceding prefetch cycle was still in flight. */
    CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_OVERLAP_DECLARE
} core_machine_cpu_external_cycle_phase;


typedef void (*core_machine_cpu_external_cycle_provider)(void *context,
    core_machine_cpu_external_cycle_phase phase,
    core_machine_cpu_external_cycle_space space, lib_u32 address,
    lib_u8 bytes, lib_u8 write,
    core_machine_cpu_memory_access_provenance provenance);

typedef struct core_machine_cpu_instruction_metadata {
    core_machine_cpu_profile minimum_cpu;
    x86_fpu_profile minimum_fpu;
    lib_i32 valid;
} core_machine_cpu_instruction_metadata;

/* A lexical result is intentionally narrower than instruction decoding: it
 * names only the byte-layout components used by 80386 Jcc's `m` timing term.
 * It never validates operands or applies instruction semantics. */
typedef struct core_machine_cpu_instruction_lexeme {
    lib_u8 byte_count;
    lib_u8 component_count;
    lib_u8 available;
} core_machine_cpu_instruction_lexeme;

typedef enum core_machine_cpu_watchpoint {
    CORE_MACHINE_CPU_WATCH_READ,
    CORE_MACHINE_CPU_WATCH_WRITE,
    CORE_MACHINE_CPU_WATCH_EXECUTE
} core_machine_cpu_watchpoint;

lib_status core_machine_cpu_create(const core_machine_cpu_bus_provider *bus,
    void *bus_context, core_machine_cpu_execution_context **out_context);
void core_machine_cpu_destroy(core_machine_cpu_execution_context *context);

void core_machine_cpu_execution_request_stop(
    core_machine_cpu_execution_context *context);
lib_u8 core_machine_cpu_execution_consume_stop_request(
    core_machine_cpu_execution_context *context);
void core_machine_cpu_execution_request_debug_pause(
    core_machine_cpu_execution_context *context);
lib_u8 core_machine_cpu_execution_consume_debug_pause_request(
    core_machine_cpu_execution_context *context);
void core_machine_cpu_execution_request_reset(
    core_machine_cpu_execution_context *context);
lib_u8 core_machine_cpu_execution_consume_reset_request(
    core_machine_cpu_execution_context *context);
void core_machine_cpu_execution_request_shutdown(
    core_machine_cpu_execution_context *context);
lib_u8 core_machine_cpu_execution_consume_shutdown_request(
    core_machine_cpu_execution_context *context);
void core_machine_cpu_state_initialize(
    core_machine_cpu_execution_context *context);
void core_machine_cpu_state_reset(core_machine_cpu_execution_context *context);
void core_machine_cpu_capture_state(const core_machine_cpu_execution_context *context,
    core_machine_cpu_state *out_state);
lib_u32 core_machine_cpu_linear_pc(const core_machine_cpu_execution_context *context);
lib_bool core_machine_cpu_is_halted(const core_machine_cpu_execution_context *context);
/* External NMI mask input gates both admission and delivery. Unmasking does
 * not manufacture an edge; the board refreshes its own pending sources. */
void core_machine_cpu_set_nmi_mask(core_machine_cpu_execution_context *context,
    lib_bool masked);
lib_bool core_machine_cpu_nmi_is_masked(const core_machine_cpu_execution_context *context);
lib_bool core_machine_cpu_request_nmi(core_machine_cpu_execution_context *context);
void core_machine_cpu_execution_context_bind_profiles(
    core_machine_cpu_execution_context *context,
    core_machine_cpu_profile cpu_profile,
    x86_fpu_profile fpu_profile,
    lib_u8 cpu_80386_cr_mov_ignores_mod, const core_machine_instruction_timing *timing);

lib_i32 core_machine_cpu_read_linear(core_machine_cpu_execution_context *context,
    lib_u32 linear, void *out_data, lib_u8 size);
/* Caller serializes debug operations against execution. Machine lifecycle
 * admission belongs to the board; register validation belongs to the CPU. */
lib_status core_machine_cpu_debug_capture_snapshot(
    const core_machine_cpu_execution_context *context,
    core_machine_cpu_snapshot_point point,
    core_machine_debug_cpu_snapshot *out_snapshot);
lib_status core_machine_cpu_debug_capture_instruction(
    const core_machine_cpu_execution_context *context,
    core_machine_debug_instruction_observation *out_observation);
lib_status core_machine_cpu_debug_read_register(
    const core_machine_cpu_execution_context *context,
    core_machine_debug_register register_id, lib_u32 *out_value);
lib_status core_machine_cpu_debug_patch_registers(
    core_machine_cpu_execution_context *context,
    const core_machine_debug_register_patch *patch);
lib_i32 core_machine_cpu_write_linear(core_machine_cpu_execution_context *context,
    lib_u32 linear, const void *in_data, lib_u8 size);
lib_i32 core_machine_cpu_get_code_default_size(
    const core_machine_cpu_execution_context *context);
lib_u32 core_machine_cpu_get_code_base(
    const core_machine_cpu_execution_context *context);
void core_machine_cpu_set_watchpoint(core_machine_cpu_execution_context *context,
    core_machine_cpu_watchpoint kind, lib_u32 linear);
void core_machine_cpu_clear_watchpoint(core_machine_cpu_execution_context *context,
    core_machine_cpu_watchpoint kind);
void core_machine_cpu_get_watchpoint(const core_machine_cpu_execution_context *context,
    core_machine_cpu_watchpoint kind, lib_u8 *out_enabled,
    lib_u32 *out_linear);

void core_machine_cpu_execution_copy_observation(
    const core_machine_cpu_execution_context *context,
    core_machine_cpu_instruction_observation *observation);

void core_machine_cpu_execution_context_bind_diagnostic_provider(
    core_machine_cpu_execution_context *context,
    const core_machine_cpu_execution_diagnostic_provider *provider,
    void *provider_context);

void core_machine_cpu_execution_context_bind_fpu(
    core_machine_cpu_execution_context *context, x86_fpu *fpu);

void core_machine_cpu_execution_context_bind_external_cycle_provider(
    core_machine_cpu_execution_context *context,
    core_machine_cpu_external_cycle_provider provider, void *provider_context);

void core_machine_cpu_execution_reserve_prefetch(
    core_machine_cpu_execution_context *context);

void core_machine_cpu_execution_advance_prefetch_reservation(
    core_machine_cpu_execution_context *context);

void core_machine_cpu_execution_invalidate_prefetch(
    core_machine_cpu_execution_context *context);

void core_machine_cpu_execution_refresh(
    core_machine_cpu_execution_context *context);

lib_u8 core_machine_cpu_execution_consume_instruction_fault_delivery(
    core_machine_cpu_execution_context *context);

core_machine_cpu_instruction_metadata core_machine_cpu_instruction_metadata_get(
    core_machine_cpu_instruction_space space, lib_u8 opcode, lib_u8 modrm);

lib_u8 core_machine_cpu_instruction_lexeme_scan(
    const lib_u8 *bytes, lib_u8 available_bytes,
    core_machine_cpu_profile profile, lib_u8 code_32,
    core_machine_cpu_instruction_lexeme *out_lexeme);

lib_u8 core_machine_cpu_execution_preview_lexeme(
    const core_machine_cpu_execution_context *context,
    core_machine_cpu_instruction_lexeme *out_lexeme);

#define CORE_MACHINE_CPU_TIMING_INPUT_MODRM      (1u << 0)
#define CORE_MACHINE_CPU_TIMING_INPUT_CONTROL    (1u << 1)
#define CORE_MACHINE_CPU_TIMING_INPUT_REPEAT     (1u << 2)
#define CORE_MACHINE_CPU_TIMING_INPUT_MODE       (1u << 3)
#define CORE_MACHINE_CPU_TIMING_INPUT_SIZE       (1u << 4)
#define CORE_MACHINE_CPU_TIMING_INPUT_LOCK       (1u << 5)
#define CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS (1u << 6)
#define CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE (1u << 7)
#define CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD   (1u << 8)
#define CORE_MACHINE_CPU_TIMING_INPUT_REPEAT_PHASE (1u << 9)
#define CORE_MACHINE_CPU_TIMING_INPUT_GROUP3_OPERAND (1u << 10)
#define CORE_MACHINE_CPU_TIMING_INPUT_WAIT_TICKS      (1u << 11)

/* B0's only successful-retirement CPU timing selection entry. */
lib_i32 core_machine_cpu_timing_select(core_machine_cpu_execution_context *context,
    core_machine_cpu_timing_result *out_result);
core_machine_cpu_timing_result core_machine_cpu_capture_timing(
    const core_machine_cpu_execution_context *context);
/* Shared checked accumulation for timing selection and the retained run loop. */
lib_i32 core_machine_timing_add_ticks(lib_u64 *value,
    lib_u64 delta);
lib_u64 core_machine_cpu_timing_maximum_ticks(
    core_machine_cpu_profile profile,
    const core_machine_instruction_timing *timing);

#ifdef __cplusplus
}
#endif

#endif
