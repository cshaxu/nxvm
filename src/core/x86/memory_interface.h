#ifndef CORE_MACHINE_MEMORY_INTERFACE_H
#define CORE_MACHINE_MEMORY_INTERFACE_H
#include "lib/types/types_interface.h"




#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

/* Copied physical decoding into the single installed RAM backing. */
typedef struct core_machine_memory_alias_config {
    lib_u32 physical_start;
    lib_u32 backing_start;
    lib_size bytes;
} core_machine_memory_alias_config;

/* Construction-only, owning executor thread. Publish the entire batch or none;
 * selected aliases take precedence over unselected fallback decoding. */
lib_status core_machine_install_memory_aliases(core_machine *machine,
    const core_machine_memory_alias_config *aliases, lib_size count,
    lib_bool selected);
/* Reconfigure one already-installed board alias.  This is for a board-owned
 * runtime decode register; it neither adds storage nor changes the backing. */
lib_status core_machine_set_memory_alias_decode(core_machine *machine,
    lib_u32 physical_start, lib_size bytes, lib_bool selected,
    lib_bool writable);

typedef void (*core_machine_memory_parity_fault_observer)(void *owner,
    lib_u32 physical);

typedef void (*core_machine_memory_write_observer)(void *owner,
    lib_u32 physical, lib_uptr bytes);

/* Inclusive physical window and its reset-selected suppressed address lines.
 * Zero clear bits give identity decoding. Board composition supplies any
 * hardware-specific window; raw reset overlays are resolved before this. */
typedef struct core_machine_address_decode {
    lib_u32 first;
    lib_u32 last;
    lib_u32 reset_clear_mask;
} core_machine_address_decode;
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
/* UNSUPPORTED declines without effects. Other write errors may retain an
 * accepted prefix; Core cancellation cannot undo device publication. */
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
 * as present; reset overlays decode before ordinary address-line decoding. An
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

lib_status core_machine_set_address_clear_mask(
    core_machine *machine,
    lib_u32 clear_mask);

/* Board signals may change address lines while the guest runs. The paused setter
 * above keeps its separate lifecycle restriction. */
lib_status core_machine_signal_address_clear_mask(core_machine *machine,
    lib_u32 clear_mask);
lib_status core_machine_observe_address_clear_mask(const core_machine *machine,
    lib_u32 *out_clear_mask);

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
