#ifndef VM_PROFILE_CONTRACT_INTERFACE_H
#define VM_PROFILE_CONTRACT_INTERFACE_H

#include "app-nxvm/devices/machine_interface.h"
#include "lib/types/types_interface.h"
#include "type.h"

#define VM_PROFILE_CONTRACT_PORT_LEAF_CAPACITY 96u
#define VM_PROFILE_CONTRACT_MEMORY_WINDOW_CAPACITY 8u
#define VM_PROFILE_CONTRACT_ROUTE_CAPACITY 8u

typedef enum vm_profile_contract_firmware_policy {
    VM_PROFILE_CONTRACT_FIRMWARE_POLICY_BUILTIN = 1u,
    VM_PROFILE_CONTRACT_FIRMWARE_POLICY_BYOB = 2u
} vm_profile_contract_firmware_policy;

typedef enum vm_profile_contract_media_policy {
    VM_PROFILE_CONTRACT_MEDIA_POLICY_NONE = 1u,
    VM_PROFILE_CONTRACT_MEDIA_POLICY_SESSION = 2u
} vm_profile_contract_media_policy;

typedef struct vm_profile_contract_window {
    lib_u32 first;
    lib_u32 last;
    lib_u32 device;
} vm_profile_contract_window;

typedef struct vm_profile_contract_port_leaf {
    lib_u32 device;
    lib_u16 port;
    type_bool read;
    type_bool write;
} vm_profile_contract_port_leaf;

typedef struct vm_profile_contract_route {
    lib_u32 device;
    lib_u8 line;
} vm_profile_contract_route;

typedef struct vm_profile_contract_core_input {
    lib_u32 id;
    core_machine_config configuration;
    core_machine_controller_timing_rules controller_timing_rules;
} vm_profile_contract_core_input;

/* A direct, frozen board result.  It contains effective values only: no
 * parent graph, per-field owner strings, or runtime provenance mirror. */
typedef struct vm_profile_contract_values {
    vm_profile_contract_core_input core;
    lib_u32 enabled_devices;
    vm_profile_contract_port_leaf port_leaves[VM_PROFILE_CONTRACT_PORT_LEAF_CAPACITY];
    lib_size port_leaf_count;
    vm_profile_contract_window memory_windows[VM_PROFILE_CONTRACT_MEMORY_WINDOW_CAPACITY];
    lib_size memory_window_count;
    vm_profile_contract_route irq_routes[VM_PROFILE_CONTRACT_ROUTE_CAPACITY];
    lib_size irq_route_count;
    vm_profile_contract_route drq_routes[VM_PROFILE_CONTRACT_ROUTE_CAPACITY];
    lib_size drq_route_count;
    lib_u32 firmware_policy;
    lib_u32 media_policy;
    lib_u32 allowed_session_options;
} vm_profile_contract_values;

typedef struct vm_profile_contract_catalog {
    const lib_u32 *ids;
    lib_size count;
} vm_profile_contract_catalog;

type_status vm_profile_contract_validate(const vm_profile_contract_values *values,
    const vm_profile_contract_catalog *catalog, lib_u32 requested_options);

#endif
