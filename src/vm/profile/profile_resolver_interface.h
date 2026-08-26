#ifndef VM_PROFILE_RESOLVER_INTERFACE_H
#define VM_PROFILE_RESOLVER_INTERFACE_H

#include "core/machine/machine_interface.h"
#include "type.h"

#define VM_PROFILE_RESOLVER_IDENTITY_CAPACITY 48u
#define VM_PROFILE_RESOLVER_PORT_LEAF_CAPACITY 96u
#define VM_PROFILE_RESOLVER_MEMORY_WINDOW_CAPACITY 8u
#define VM_PROFILE_RESOLVER_ROUTE_CAPACITY 8u
#define VM_PROFILE_RESOLVER_PARENT_DEPTH_CAPACITY 8u
#define VM_PROFILE_RESOLVER_FIELD_COUNT 7u

typedef enum vm_profile_resolver_field {
    VM_PROFILE_RESOLVER_FIELD_CORE = 1u << 0u,
    VM_PROFILE_RESOLVER_FIELD_DEVICES = 1u << 1u,
    VM_PROFILE_RESOLVER_FIELD_PORTS = 1u << 2u,
    VM_PROFILE_RESOLVER_FIELD_MEMORY = 1u << 3u,
    VM_PROFILE_RESOLVER_FIELD_IRQ = 1u << 4u,
    VM_PROFILE_RESOLVER_FIELD_DRQ = 1u << 5u,
    VM_PROFILE_RESOLVER_FIELD_POLICY = 1u << 6u,
    VM_PROFILE_RESOLVER_FIELD_ALL = (1u << 7u) - 1u
} vm_profile_resolver_field;

typedef enum vm_profile_resolver_firmware_policy {
    VM_PROFILE_RESOLVER_FIRMWARE_POLICY_BUILTIN = 1u,
    VM_PROFILE_RESOLVER_FIRMWARE_POLICY_BYOB = 2u
} vm_profile_resolver_firmware_policy;

typedef enum vm_profile_resolver_media_policy {
    VM_PROFILE_RESOLVER_MEDIA_POLICY_NONE = 1u,
    VM_PROFILE_RESOLVER_MEDIA_POLICY_SESSION = 2u
} vm_profile_resolver_media_policy;

typedef struct vm_profile_resolver_window {
    type_unsigned_32 first;
    type_unsigned_32 last;
    type_unsigned_32 device;
} vm_profile_resolver_window;

/* A port is a leaf, not an inferred interval: read/write direction is part of
 * the board contract and gaps must stay absent. */
typedef struct vm_profile_resolver_port_leaf {
    type_unsigned_32 device;
    type_unsigned_16 port;
    type_bool read;
    type_bool write;
} vm_profile_resolver_port_leaf;

typedef struct vm_profile_resolver_route {
    type_unsigned_32 device;
    type_unsigned_8 line;
} vm_profile_resolver_route;

typedef struct vm_profile_resolver_core_plan_input {
    type_unsigned_32 contract_id;
    core_machine_config configuration;
    core_machine_controller_timing_rules controller_timing_rules;
} vm_profile_resolver_core_plan_input;

typedef struct vm_profile_resolver_values {
    vm_profile_resolver_core_plan_input core;
    type_unsigned_32 enabled_devices;
    vm_profile_resolver_port_leaf port_leaves[VM_PROFILE_RESOLVER_PORT_LEAF_CAPACITY];
    STD_SIZE_T port_leaf_count;
    vm_profile_resolver_window memory_windows[VM_PROFILE_RESOLVER_MEMORY_WINDOW_CAPACITY];
    STD_SIZE_T memory_window_count;
    vm_profile_resolver_route irq_routes[VM_PROFILE_RESOLVER_ROUTE_CAPACITY];
    STD_SIZE_T irq_route_count;
    vm_profile_resolver_route drq_routes[VM_PROFILE_RESOLVER_ROUTE_CAPACITY];
    STD_SIZE_T drq_route_count;
    type_unsigned_32 firmware_policy;
    type_unsigned_32 media_policy;
    type_unsigned_32 allowed_session_options;
} vm_profile_resolver_values;

typedef struct vm_profile_resolver_declaration {
    const C_CHAR *identity;
    const struct vm_profile_resolver_declaration *parent;
    type_unsigned_32 provided_fields;
    type_unsigned_32 owned_fields;
    vm_profile_resolver_values values;
} vm_profile_resolver_declaration;

typedef struct vm_profile_resolver_contract_catalog {
    const type_unsigned_32 *ids;
    STD_SIZE_T count;
} vm_profile_resolver_contract_catalog;

typedef struct vm_profile_resolver_session_request {
    type_unsigned_32 requested_options;
} vm_profile_resolver_session_request;

typedef struct vm_resolved_profile {
    C_CHAR identity[VM_PROFILE_RESOLVER_IDENTITY_CAPACITY];
    C_CHAR parent_identity[VM_PROFILE_RESOLVER_IDENTITY_CAPACITY];
    C_CHAR field_owner[VM_PROFILE_RESOLVER_FIELD_COUNT]
        [VM_PROFILE_RESOLVER_IDENTITY_CAPACITY];
    type_unsigned_32 owned_fields;
    vm_profile_resolver_values values;
} vm_resolved_profile;

type_status vm_profile_resolver_resolve(
    const vm_profile_resolver_declaration *declaration,
    const vm_profile_resolver_contract_catalog *catalog,
    const vm_profile_resolver_session_request *request,
    vm_resolved_profile *out_profile);

#endif
