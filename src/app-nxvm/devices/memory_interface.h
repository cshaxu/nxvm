#ifndef CORE_MACHINE_MEMORY_INTERFACE_H
#define CORE_MACHINE_MEMORY_INTERFACE_H
#include "lib/types/types_interface.h"




#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

typedef void (*core_machine_memory_parity_fault_observer)(void *owner,
    lib_u32 physical);

typedef void (*core_machine_memory_write_observer)(void *owner,
    lib_u32 physical, lib_uptr bytes);

typedef enum core_machine_a20_wrap_policy {
    CORE_MACHINE_A20_WRAP_GLOBAL_MASK = 0,
    CORE_MACHINE_A20_WRAP_FIRST_TO_SECOND_MIB = 1
} core_machine_a20_wrap_policy;
typedef enum core_machine_memory_access {
    CORE_MACHINE_MEMORY_ACCESS_READ = 0,
    CORE_MACHINE_MEMORY_ACCESS_WRITE
} core_machine_memory_access;

typedef enum core_machine_memory_route {
    CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM = 0,
    CORE_MACHINE_MEMORY_ROUTE_PROVIDER
} core_machine_memory_route;

/* A composition-owned device may claim a configured physical range.  Returning
 * LIB_STATUS_UNSUPPORTED from query leaves the range available to the next
 * registered device or ordinary RAM; other failures are terminal. Read callbacks
 * with observe_only return the same bytes without changing device state. Query
 * is always observational; inspection never falls back to an operational read. */
typedef lib_status (*core_machine_memory_device_read)(void *owner,
    lib_u32 physical, lib_uptr destination,
    lib_uptr bytes, lib_bool observe_only);
typedef lib_status (*core_machine_memory_device_write)(void *owner,
    lib_u32 physical, lib_uptr source,
    lib_uptr bytes);
typedef lib_status (*core_machine_memory_device_query)(void *owner,
    lib_u32 physical, lib_uptr bytes,
    core_machine_memory_access access);

typedef struct core_machine_memory_device_callbacks {
    core_machine_memory_device_read read;
    core_machine_memory_device_write write;
    core_machine_memory_device_query query;
} core_machine_memory_device_callbacks;

typedef enum core_machine_memory_provider_mode {
    CORE_MACHINE_MEMORY_PROVIDER_STANDARD = 0,
    CORE_MACHINE_MEMORY_PROVIDER_OVERLAY,
    CORE_MACHINE_MEMORY_PROVIDER_RESET_OVERLAY,
    CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT,
    CORE_MACHINE_MEMORY_PROVIDER_FALLBACK
} core_machine_memory_provider_mode;

typedef struct core_machine_memory_device_route {
    lib_u32 physical_start;
    lib_size bytes;
    core_machine_memory_device_callbacks callbacks;
    core_machine_memory_provider_mode mode;
} core_machine_memory_device_route;

typedef struct core_machine_memory_parity_config {
    lib_size bytes;
    core_machine_memory_parity_fault_observer fault;
} core_machine_memory_parity_config;

typedef void (*core_machine_dma_device_effect)(void *owner,
    lib_u8 channel, lib_u16 *value);

/* Publish routes, optional parity and optional write observer as one owner
 * transaction. Replacement routes override lower ROM/RAM only when queried
 * as present; reset overlays decode before ordinary A20 routing. An
 * observer-only registration has zero routes. */
lib_status core_machine_install_memory_device_routes(core_machine *machine,
    const core_machine_memory_device_route *routes, lib_size count,
    core_machine_memory_write_observer observer,
    const core_machine_memory_parity_config *parity, void *owner);
lib_status core_machine_remove_memory_device_routes(core_machine *machine,
    const void *owner);
/* Copy bytes through the observational route without changing guest state. */
lib_status core_machine_memory_inspect(const core_machine *machine,
    lib_u32 physical, void *out_data, lib_size size);
lib_status core_machine_memory_read(
    const core_machine *machine,
    lib_u32 physical,
    void *out_data,
    lib_size size);

lib_status core_machine_memory_write(
    core_machine *machine,
    lib_u32 physical,
    const void *data,
    lib_size size);

/* Queries one complete physical range without exposing storage or invoking a
 * provider data callback. It is valid only at a stopped or paused boundary. */
lib_status core_machine_memory_query(
    const core_machine *machine,
    lib_u32 physical,
    lib_size size,
    core_machine_memory_access access,
    core_machine_memory_route *out_route);

lib_status core_machine_set_a20(
    core_machine *machine,
    lib_i32 enabled);

/* Board signals may change A20 while the guest runs. The paused-debug setter
 * above keeps its separate lifecycle restriction. */
lib_status core_machine_signal_a20(core_machine *machine, lib_bool enabled);
lib_status core_machine_observe_a20(const core_machine *machine,
    lib_bool *out_enabled);

/* One Core-owned DMA memory transaction. Device effects occur only between
 * begin and memory, or between memory and commit, respectively. */
lib_status core_machine_dma_memory_cycle(core_machine *machine,
    lib_u32 physical, lib_u8 bytes, lib_u8 channel,
    core_machine_memory_access access, lib_u16 *value,
    core_machine_dma_device_effect before_memory,
    core_machine_dma_device_effect after_memory, void *device_owner);

#ifdef __cplusplus
}
#endif

#endif
