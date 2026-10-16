#ifndef X86_NXVM_BOARD_AT_WIRING_INTERFACE_H
#define X86_NXVM_BOARD_AT_WIRING_INTERFACE_H

#include "lib/types/types_interface.h"

typedef enum x86_nxvm_board_at_device_role {
    X86_NXVM_BOARD_AT_DEVICE_PIC,
    X86_NXVM_BOARD_AT_DEVICE_PIT,
    X86_NXVM_BOARD_AT_DEVICE_DMA,
    X86_NXVM_BOARD_AT_DEVICE_KBC,
    X86_NXVM_BOARD_AT_DEVICE_VADP,
    X86_NXVM_BOARD_AT_DEVICE_VADP_SEQUENCER,
    X86_NXVM_BOARD_AT_DEVICE_VADP_GRAPHICS,
    X86_NXVM_BOARD_AT_DEVICE_VADP_ATTRIBUTE,
    X86_NXVM_BOARD_AT_DEVICE_CMOS,
    X86_NXVM_BOARD_AT_DEVICE_FDC,
    X86_NXVM_BOARD_AT_DEVICE_HDC,
    X86_NXVM_BOARD_AT_DEVICE_MEMORY_CONTROL,
    X86_NXVM_BOARD_AT_DEVICE_BOARD
} x86_nxvm_board_at_device_role;

#define X86_NXVM_BOARD_AT_NO_DMA_CHANNEL 0xffu
#define X86_NXVM_BOARD_AT_DEVICE_MASK_ALL ((1u << (X86_NXVM_BOARD_AT_DEVICE_BOARD + 1u)) - 1u)

typedef struct x86_nxvm_board_at_port_leaf {
    x86_nxvm_board_at_device_role device;
    lib_u16 port;
    lib_u8 read;
    lib_u8 write;
} x86_nxvm_board_at_port_leaf;

typedef enum x86_nxvm_board_at_route_source {
    X86_NXVM_BOARD_AT_ROUTE_PIT_IRQ0,
    X86_NXVM_BOARD_AT_ROUTE_KBC_KEYBOARD_IRQ1,
    X86_NXVM_BOARD_AT_ROUTE_KBC_AUX_IRQ12,
    X86_NXVM_BOARD_AT_ROUTE_CMOS_IRQ8,
    X86_NXVM_BOARD_AT_ROUTE_FDC_IRQ6_DMA2
} x86_nxvm_board_at_route_source;

typedef struct x86_nxvm_board_at_route {
    x86_nxvm_board_at_route_source source;
    lib_u8 irq;
    lib_u8 dma_channel;
} x86_nxvm_board_at_route;

/* Immutable AT endpoint grammar; compositions select enabled device bits.
 * These are declarations, never live chip/register state or a model selector. */
#define X86_NXVM_BOARD_AT_PORT_LEAF_COUNT 78u
extern const x86_nxvm_board_at_port_leaf x86_nxvm_board_at_port_leaves[X86_NXVM_BOARD_AT_PORT_LEAF_COUNT];
extern const x86_nxvm_board_at_route x86_nxvm_board_at_routes_with_aux[5u];
extern const x86_nxvm_board_at_route x86_nxvm_board_at_routes_without_aux[4u];

/* Borrowed immutable results. Enabled roles are explicit composition input. */
const x86_nxvm_board_at_port_leaf *x86_nxvm_board_at_port_leaf_find(const x86_nxvm_board_at_port_leaf *leaves,
    lib_size count, lib_u32 enabled, x86_nxvm_board_at_device_role device, lib_u16 port);
const x86_nxvm_board_at_port_leaf *x86_nxvm_board_at_port_leaf_at(const x86_nxvm_board_at_port_leaf *leaves,
    lib_size count, lib_u32 enabled, x86_nxvm_board_at_device_role device, lib_size ordinal);
const x86_nxvm_board_at_route *x86_nxvm_board_at_route_find(const x86_nxvm_board_at_route *routes,
    lib_size count, x86_nxvm_board_at_route_source source);

#endif
