#ifndef X86_CORE_MACHINE_MEMORY_INTERFACE_H
#define X86_CORE_MACHINE_MEMORY_INTERFACE_H
#include "lib/types/types_interface.h"




#ifdef __cplusplus
extern "C" {
#endif

typedef struct x86_core_machine x86_core_machine;

/* Copied physical decoding into the single installed RAM backing. */
typedef struct x86_core_machine_memory_alias_config {
    lib_u32 physical_start;
    lib_u32 backing_start;
    lib_size bytes;
} x86_core_machine_memory_alias_config;

/* Construction-only, owning executor thread. Publish the entire batch or none;
 * selected aliases take precedence over unselected fallback decoding. */
lib_status x86_core_machine_install_memory_aliases(x86_core_machine *machine,
    const x86_core_machine_memory_alias_config *aliases, lib_size count,
    lib_bool selected);
/* Reconfigure one already-installed board alias.  This is for a board-owned
 * runtime decode register; it neither adds storage nor changes the backing. */
lib_status x86_core_machine_set_memory_alias_decode(x86_core_machine *machine,
    lib_u32 physical_start, lib_size bytes, lib_bool selected,
    lib_bool writable);

typedef void (*x86_core_machine_memory_parity_fault_observer)(void *owner,
    lib_u32 physical);

typedef void (*x86_core_machine_memory_write_observer)(void *owner,
    lib_u32 physical, lib_uptr bytes);

/* Inclusive physical window and its reset-selected suppressed address lines.
 * Zero clear bits give identity decoding. Board composition supplies any
 * hardware-specific window; raw reset overlays are resolved before this. */
typedef struct x86_core_machine_address_decode {
    lib_u32 first;
    lib_u32 last;
    lib_u32 reset_clear_mask;
} x86_core_machine_address_decode;
typedef enum x86_core_machine_memory_access {
    X86_CORE_MACHINE_MEMORY_ACCESS_READ = 0,
    X86_CORE_MACHINE_MEMORY_ACCESS_WRITE
} x86_core_machine_memory_access;

/* Called only after this route has claimed an access and its query, read or
 * write callback returns a terminal status. UNSUPPORTED is a normal decline
 * and never reaches this observer. */
typedef void (*x86_core_machine_memory_device_failure_observer)(void *owner,
    lib_u32 physical, lib_uptr bytes,
    x86_core_machine_memory_access access, lib_status status);

typedef enum x86_core_machine_memory_route {
    X86_CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM = 0,
    X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER
} x86_core_machine_memory_route;

/* A composition-owned device may claim a configured physical range.  Returning
 * LIB_STATUS_UNSUPPORTED from query leaves the range available to the next
 * registered device or ordinary RAM; other failures are terminal. Read callbacks
 * with observe_only return the same bytes without changing device state. Query
 * is always observational; inspection never falls back to an operational read. */
typedef lib_status (*x86_core_machine_memory_device_read)(void *owner,
    lib_u32 physical, lib_uptr destination,
    lib_uptr bytes, lib_bool observe_only);
/* Optional instruction-fetch source for a decoded memory provider.  When it
 * is absent, instruction fetches use read.  It exists for hardware whose ROM
 * remains executable while the same physical window exposes writable shadow
 * RAM to data accesses. */
typedef lib_status (*x86_core_machine_memory_device_instruction_read)(void *owner,
    lib_u32 physical, lib_uptr destination,
    lib_uptr bytes, lib_bool observe_only);
/* UNSUPPORTED declines without effects. Other write errors may retain an
 * accepted prefix; Core cancellation cannot undo device publication. */
typedef lib_status (*x86_core_machine_memory_device_write)(void *owner,
    lib_u32 physical, lib_uptr source,
    lib_uptr bytes);
typedef lib_status (*x86_core_machine_memory_device_query)(void *owner,
    lib_u32 physical, lib_uptr bytes,
    x86_core_machine_memory_access access);

typedef struct x86_core_machine_memory_device_callbacks {
    x86_core_machine_memory_device_read read;
    x86_core_machine_memory_device_write write;
    x86_core_machine_memory_device_query query;
} x86_core_machine_memory_device_callbacks;

typedef enum x86_core_machine_memory_provider_mode {
    X86_CORE_MACHINE_MEMORY_PROVIDER_STANDARD = 0,
    X86_CORE_MACHINE_MEMORY_PROVIDER_OVERLAY,
    X86_CORE_MACHINE_MEMORY_PROVIDER_RESET_OVERLAY,
    X86_CORE_MACHINE_MEMORY_PROVIDER_REPLACEMENT,
    X86_CORE_MACHINE_MEMORY_PROVIDER_FALLBACK,
    /* CPU physical overlay before ordinary aliases/address-line decoding.
     * DMA never sees this provider; its ordinary RAM/device route is retained. */
    X86_CORE_MACHINE_MEMORY_PROVIDER_CPU_OVERLAY
} x86_core_machine_memory_provider_mode;

typedef struct x86_core_machine_memory_device_route {
    lib_u32 physical_start;
    lib_size bytes;
    x86_core_machine_memory_device_callbacks callbacks;
    x86_core_machine_memory_provider_mode mode;
    x86_core_machine_memory_device_failure_observer failure;
} x86_core_machine_memory_device_route;

typedef struct x86_core_machine_memory_parity_config {
    lib_size bytes;
    x86_core_machine_memory_parity_fault_observer fault;
} x86_core_machine_memory_parity_config;

typedef void (*x86_core_machine_dma_device_effect)(void *owner,
    lib_u8 channel, lib_u16 *value);

/* Publish routes, optional parity and optional write observer as one owner
 * transaction. Replacement routes override lower ROM/RAM only when queried
 * as present; reset overlays decode before ordinary address-line decoding. An
 * observer-only registration has zero routes. */
lib_status x86_core_machine_install_memory_device_routes(x86_core_machine *machine,
    const x86_core_machine_memory_device_route *routes, lib_size count,
    x86_core_machine_memory_write_observer observer,
    const x86_core_machine_memory_parity_config *parity, void *owner);
lib_status x86_core_machine_remove_memory_device_routes(x86_core_machine *machine,
    const void *owner);
/* Select an optional immutable executable source for all routes owned by this
 * composition.  Returning UNSUPPORTED falls back to the route's ordinary
 * read callback.  The setter is construction-only. */
lib_status x86_core_machine_set_memory_instruction_read(x86_core_machine *machine,
    const void *owner, x86_core_machine_memory_device_instruction_read read);
/* Copy bytes through the observational route without changing guest state. */
lib_status x86_core_machine_memory_inspect(const x86_core_machine *machine,
    lib_u32 physical, void *out_data, lib_size size);
lib_status x86_core_machine_memory_read(
    const x86_core_machine *machine,
    lib_u32 physical,
    void *out_data,
    lib_size size);

lib_status x86_core_machine_memory_write(
    x86_core_machine *machine,
    lib_u32 physical,
    const void *data,
    lib_size size);

/* Queries one complete physical range without exposing storage or invoking a
 * provider data callback. It is valid only at a stopped or paused boundary. */
lib_status x86_core_machine_memory_query(
    const x86_core_machine *machine,
    lib_u32 physical,
    lib_size size,
    x86_core_machine_memory_access access,
    x86_core_machine_memory_route *out_route);

lib_status x86_core_machine_set_address_clear_mask(
    x86_core_machine *machine,
    lib_u32 clear_mask);

/* Board signals may change address lines while the guest runs. The paused setter
 * above keeps its separate lifecycle restriction. */
lib_status x86_core_machine_signal_address_clear_mask(x86_core_machine *machine,
    lib_u32 clear_mask);
lib_status x86_core_machine_observe_address_clear_mask(const x86_core_machine *machine,
    lib_u32 *out_clear_mask);

/* Owning execution thread, including a board output callback. Invalidate old
 * CPU cache/prefetch bytes when a provider changes its physical memory view.
 * This is not a guest write or a DMA snoop and does not advance guest time. */
void x86_core_machine_signal_memory_decode_change(x86_core_machine *machine,
    lib_u32 physical, lib_size bytes);

/* One Core-owned DMA memory transaction. Device effects occur only between
 * begin and memory, or between memory and commit, respectively. */
lib_status x86_core_machine_dma_memory_cycle(x86_core_machine *machine,
    lib_u32 physical, lib_u8 bytes, lib_u8 channel,
    x86_core_machine_memory_access access, lib_u16 *value,
    x86_core_machine_dma_device_effect before_memory,
    x86_core_machine_dma_device_effect after_memory, void *device_owner);

#ifdef __cplusplus
}
#endif

#endif
