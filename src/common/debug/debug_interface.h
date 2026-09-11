#ifndef COMMON_DEBUG_INTERFACE_H
#define COMMON_DEBUG_INTERFACE_H

#include "lib/types/types_interface.h"
#include "common/machine/machine_interface.h"

typedef struct common_debug common_debug;

/* This is the copied x86 register vocabulary exposed by common/machine.  It
 * intentionally follows the machine contract rather than Core's private
 * enum, so a second product can map its own CPU implementation to it. */
typedef enum common_debug_register {
    COMMON_DEBUG_EAX, COMMON_DEBUG_ECX, COMMON_DEBUG_EDX, COMMON_DEBUG_EBX,
    COMMON_DEBUG_ESP, COMMON_DEBUG_EBP, COMMON_DEBUG_ESI, COMMON_DEBUG_EDI,
    COMMON_DEBUG_EIP, COMMON_DEBUG_EFLAGS, COMMON_DEBUG_ES, COMMON_DEBUG_CS,
    COMMON_DEBUG_SS, COMMON_DEBUG_DS, COMMON_DEBUG_FS, COMMON_DEBUG_GS,
    COMMON_DEBUG_CR0, COMMON_DEBUG_CR1, COMMON_DEBUG_CR2, COMMON_DEBUG_CR3,
    COMMON_DEBUG_CR4, COMMON_DEBUG_REGISTER_COUNT
} common_debug_register;

#define COMMON_DEBUG_LINE_CAPACITY 256u
#define COMMON_DEBUG_TEXT_CAPACITY 8192u
#define COMMON_DEBUG_PROMPT_CAPACITY 64u
#define COMMON_DEBUG_MEMORY_ACCESS_CAPACITY 32u

typedef enum common_debug_lifecycle_request {
    COMMON_DEBUG_LIFECYCLE_NONE,
    COMMON_DEBUG_LIFECYCLE_RESUME,
    COMMON_DEBUG_LIFECYCLE_STEP,
    COMMON_DEBUG_LIFECYCLE_STOP
} common_debug_lifecycle_request;

typedef struct common_debug_result {
    char text[COMMON_DEBUG_TEXT_CAPACITY];
    char prompt[COMMON_DEBUG_PROMPT_CAPACITY];
    lib_bool prompt_ready;
    lib_bool keep_active;
    common_debug_lifecycle_request lifecycle_request;
} common_debug_result;

typedef struct common_debug_memory_access {
    lib_bool write;
    lib_u32 linear;
    lib_u8 bytes;
    lib_u64 data;
} common_debug_memory_access;

typedef struct common_debug_instruction_observation {
    common_debug_memory_access
        memory_accesses[COMMON_DEBUG_MEMORY_ACCESS_CAPACITY];
    lib_u8 memory_access_count;
} common_debug_instruction_observation;

typedef enum common_debug_machine_state {
    COMMON_DEBUG_MACHINE_RUNNING,
    COMMON_DEBUG_MACHINE_PAUSED,
    COMMON_DEBUG_MACHINE_RESET,
    COMMON_DEBUG_MACHINE_STOPPED,
    COMMON_DEBUG_MACHINE_FAULT
} common_debug_machine_state;

typedef lib_status (*common_debug_file_read)(void *context, const char *path,
    lib_u64 offset, lib_u8 *out_data, lib_size capacity, lib_size *out_bytes);
typedef lib_status (*common_debug_file_write)(void *context, const char *path,
    lib_u64 offset, const lib_u8 *data, lib_size bytes);

typedef struct common_debug_file_service {
    common_debug_file_read read;
    common_debug_file_write write;
    void *context;
} common_debug_file_service;

lib_status common_debug_create(common_debug **out_debug);
void common_debug_destroy(common_debug *debug);
lib_status common_debug_open(common_debug *debug, common_machine *machine,
    const common_debug_file_service *files);
void common_debug_close(common_debug *debug);
lib_status common_debug_submit_line(common_debug *debug, const char *line,
    common_debug_result *out_result);
void common_debug_observe_instruction(common_debug *debug,
    const common_debug_instruction_observation *observation);
lib_status common_debug_observe_machine(common_debug *debug,
    common_debug_machine_state state, lib_status status,
    common_debug_result *out_result);

#endif
