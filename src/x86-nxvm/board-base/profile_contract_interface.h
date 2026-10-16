#ifndef X86_NXVM_BOARD_CONTRACT_INTERFACE_H
#define X86_NXVM_BOARD_CONTRACT_INTERFACE_H

#include "x86-nxvm/board-base/machine_board_interface.h"
#include "lib/types/types_interface.h"

#define X86_NXVM_BOARD_CONTRACT_PORT_LEAF_CAPACITY 96u
#define X86_NXVM_BOARD_CONTRACT_MEMORY_WINDOW_CAPACITY 8u
#define X86_NXVM_BOARD_CONTRACT_ROUTE_CAPACITY 8u

typedef enum x86_nxvm_board_contract_firmware_policy {
    X86_NXVM_BOARD_CONTRACT_FIRMWARE_POLICY_BUILTIN = 1u,
    X86_NXVM_BOARD_CONTRACT_FIRMWARE_POLICY_BYOB = 2u
} x86_nxvm_board_contract_firmware_policy;

typedef enum x86_nxvm_board_contract_media_policy {
    X86_NXVM_BOARD_CONTRACT_MEDIA_POLICY_NONE = 1u,
    X86_NXVM_BOARD_CONTRACT_MEDIA_POLICY_SESSION = 2u
} x86_nxvm_board_contract_media_policy;

typedef struct x86_nxvm_board_contract_window {
    lib_u32 first;
    lib_u32 last;
    lib_u32 device;
} x86_nxvm_board_contract_window;

typedef struct x86_nxvm_board_contract_port_leaf {
    lib_u32 device;
    lib_u16 port;
    lib_u8 read;
    lib_u8 write;
} x86_nxvm_board_contract_port_leaf;

typedef struct x86_nxvm_board_contract_route {
    lib_u32 device;
    lib_u8 line;
} x86_nxvm_board_contract_route;

typedef struct x86_nxvm_board_contract_core_input {
    lib_u32 id;
    x86_nxvm_board_config configuration;
    x86_nxvm_board_controller_timing_rules controller_timing_rules;
} x86_nxvm_board_contract_core_input;

/* A direct, frozen board result.  It contains effective values only: no
 * parent graph, per-field owner strings, or runtime provenance mirror. */
typedef struct x86_nxvm_board_contract_values {
    x86_nxvm_board_contract_core_input core;
    lib_u32 enabled_devices;
    x86_nxvm_board_contract_port_leaf port_leaves[X86_NXVM_BOARD_CONTRACT_PORT_LEAF_CAPACITY];
    lib_size port_leaf_count;
    x86_nxvm_board_contract_window memory_windows[X86_NXVM_BOARD_CONTRACT_MEMORY_WINDOW_CAPACITY];
    lib_size memory_window_count;
    x86_nxvm_board_contract_route irq_routes[X86_NXVM_BOARD_CONTRACT_ROUTE_CAPACITY];
    lib_size irq_route_count;
    x86_nxvm_board_contract_route drq_routes[X86_NXVM_BOARD_CONTRACT_ROUTE_CAPACITY];
    lib_size drq_route_count;
    lib_u32 firmware_policy;
    lib_u32 media_policy;
    lib_u32 allowed_session_options;
} x86_nxvm_board_contract_values;

typedef struct x86_nxvm_board_contract_catalog {
    const lib_u32 *ids;
    lib_size count;
} x86_nxvm_board_contract_catalog;

lib_status x86_nxvm_board_contract_validate(const x86_nxvm_board_contract_values *values,
    const x86_nxvm_board_contract_catalog *catalog, lib_u32 requested_options);

#endif
