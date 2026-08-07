/* Frozen, profile-neutral guest media-provider contract. */
#ifndef CORE_MACHINE_MEDIA_INTERFACE_H
#define CORE_MACHINE_MEDIA_INTERFACE_H

#include "type.h"

#define CORE_MACHINE_MEDIA_MAX_DEVICES 4u
#define CORE_MACHINE_MEDIA_ID_INVALID 0u

typedef uint32_t core_machine_media_id;

typedef enum core_machine_media_result {
    CORE_MACHINE_MEDIA_RESULT_OK = 0,
    CORE_MACHINE_MEDIA_RESULT_ABSENT,
    CORE_MACHINE_MEDIA_RESULT_CHANGED,
    CORE_MACHINE_MEDIA_RESULT_READ_ONLY,
    CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE,
    CORE_MACHINE_MEDIA_RESULT_TRANSIENT,
    CORE_MACHINE_MEDIA_RESULT_PERMANENT,
    CORE_MACHINE_MEDIA_RESULT_UNSUPPORTED
} core_machine_media_result;

typedef enum core_machine_media_capability {
    CORE_MACHINE_MEDIA_CAPABILITY_REMOVABLE = 0x0001u,
    CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY = 0x0002u,
    CORE_MACHINE_MEDIA_CAPABILITY_FLUSHABLE = 0x0004u,
    CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN = 0x0008u,
    CORE_MACHINE_MEDIA_CAPABILITY_CHANGE_DETECTABLE = 0x0010u,
    CORE_MACHINE_MEDIA_CAPABILITY_FORMATTABLE = 0x0020u
} core_machine_media_capability;

typedef struct core_machine_media_geometry {
    uint64_t logical_sector_count;
    uint32_t bytes_per_sector;
    uint32_t cylinders;
    uint32_t heads;
    uint32_t sectors_per_track;
} core_machine_media_geometry;

typedef struct core_machine_media_info {
    core_machine_media_id id;
    uint64_t generation;
    uint32_t capabilities;
    type_bool present;
    core_machine_media_geometry geometry;
} core_machine_media_info;

typedef core_machine_media_result (*core_machine_media_query_provider)(
    C_VOID *context, core_machine_media_info *out_info);
typedef core_machine_media_result (*core_machine_media_read_provider)(
    C_VOID *context, uint64_t offset, C_VOID *buffer, uint32_t byte_count);
typedef core_machine_media_result (*core_machine_media_write_provider)(
    C_VOID *context, uint64_t offset, const C_VOID *buffer, uint32_t byte_count);
typedef core_machine_media_result (*core_machine_media_format_provider)(
    C_VOID *context, uint64_t logical_sector, uint32_t sector_count, uint8_t fill);
typedef core_machine_media_result (*core_machine_media_flush_provider)(
    C_VOID *context);

typedef struct core_machine_media_provider {
    core_machine_media_query_provider query;
    core_machine_media_read_provider read_bytes;
    core_machine_media_write_provider write_bytes;
    core_machine_media_format_provider format_sectors;
    core_machine_media_flush_provider flush;
} core_machine_media_provider;

typedef struct core_machine_media_binding {
    core_machine_media_id id;
    C_VOID *context;
    const core_machine_media_provider *provider;
} core_machine_media_binding;

typedef struct core_machine_media_registry {
    core_machine_media_binding bindings[CORE_MACHINE_MEDIA_MAX_DEVICES];
    uint32_t binding_count;
    type_bool frozen;
} core_machine_media_registry;

C_VOID core_machine_media_registry_initialize(core_machine_media_registry *registry);
type_status core_machine_media_registry_bind(core_machine_media_registry *registry,
    core_machine_media_id id, C_VOID *context,
    const core_machine_media_provider *provider);
type_status core_machine_media_registry_freeze(core_machine_media_registry *registry);
C_VOID core_machine_media_registry_finalize(core_machine_media_registry *registry);

type_status core_machine_media_query(const core_machine_media_registry *registry,
    core_machine_media_id id, core_machine_media_info *out_info,
    core_machine_media_result *out_result);
type_status core_machine_media_read_bytes(const core_machine_media_registry *registry,
    core_machine_media_id id, uint64_t offset, C_VOID *buffer, uint32_t byte_count,
    core_machine_media_result *out_result);
type_status core_machine_media_write_bytes(const core_machine_media_registry *registry,
    core_machine_media_id id, uint64_t offset, const C_VOID *buffer,
    uint32_t byte_count, core_machine_media_result *out_result);
type_status core_machine_media_read_sectors(const core_machine_media_registry *registry,
    core_machine_media_id id, uint64_t logical_sector, uint32_t sector_count,
    C_VOID *buffer, core_machine_media_result *out_result);
type_status core_machine_media_write_sectors(const core_machine_media_registry *registry,
    core_machine_media_id id, uint64_t logical_sector, uint32_t sector_count,
    const C_VOID *buffer, core_machine_media_result *out_result);
type_status core_machine_media_format_sectors(const core_machine_media_registry *registry,
    core_machine_media_id id, uint64_t logical_sector, uint32_t sector_count,
    uint8_t fill, core_machine_media_result *out_result);
type_status core_machine_media_flush(const core_machine_media_registry *registry,
    core_machine_media_id id, core_machine_media_result *out_result);

#endif
