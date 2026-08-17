#ifndef CORE_MACHINE_RETIREMENT_OBSERVATION_INTERFACE_H
#define CORE_MACHINE_RETIREMENT_OBSERVATION_INTERFACE_H

#include "core/machine/cpu_interface.h"
#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

typedef enum core_machine_retirement_timing_disposition {
    CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED = 1,
    CORE_MACHINE_RETIREMENT_TIMING_SOURCE_UNALLOCATED
} core_machine_retirement_timing_disposition;

/* The timing form is an opaque Core-owned identifier. A classified path
 * without a ledger lookup intentionally reports this sentinel. */
#define CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ((type_unsigned_32)-1)

typedef struct core_machine_retirement_observation {
    type_unsigned_64 sequence;
    type_unsigned_64 elapsed_ticks;
    type_unsigned_64 timeline_ticks;
    type_unsigned_64 source_ticks;
    core_machine_cpu_execution_point point;
    core_machine_cpu_profile cpu_profile;
    core_machine_retirement_timing_disposition timing_disposition;
    type_unsigned_32 source_timing_form_id;
    type_unsigned_8 cpl;
    type_bool protected_mode;
    type_bool virtual_8086_mode;
    type_bool operand_size_32;
    type_bool address_size_32;
    type_bool lock_prefix;
    type_unsigned_8 repeat_prefix;
} core_machine_retirement_observation;

typedef C_VOID (*core_machine_retirement_observation_callback)(
    C_VOID *context, const core_machine_retirement_observation *observation);

typedef struct core_machine_retirement_observation_provider {
    core_machine_retirement_observation_callback callback;
    C_VOID *context;
} core_machine_retirement_observation_provider;

/* Installs or removes the optional copied retirement observer while stopped or
 * paused. The callback observes only Core-owned copied state and is invoked
 * before physical-contract rejection or elapsed-time publication. */
type_status core_machine_set_retirement_observation_provider(
    core_machine *machine,
    const core_machine_retirement_observation_provider *provider);

#ifdef __cplusplus
}
#endif

#endif