#ifndef VM_AT_WIRING_INTERFACE_H
#define VM_AT_WIRING_INTERFACE_H

#include "lib/types/types_interface.h"

typedef enum vm_at_device_role {
    VM_AT_DEVICE_PIC,
    VM_AT_DEVICE_PIT,
    VM_AT_DEVICE_DMA,
    VM_AT_DEVICE_KBC,
    VM_AT_DEVICE_VADP,
    VM_AT_DEVICE_VADP_SEQUENCER,
    VM_AT_DEVICE_VADP_GRAPHICS,
    VM_AT_DEVICE_VADP_ATTRIBUTE,
    VM_AT_DEVICE_CMOS,
    VM_AT_DEVICE_FDC,
    VM_AT_DEVICE_HDC,
    VM_AT_DEVICE_MEMORY_CONTROL,
    VM_AT_DEVICE_BOARD
} vm_at_device_role;

#define VM_AT_NO_DMA_CHANNEL 0xffu
#define VM_AT_DEVICE_MASK_ALL ((1u << (VM_AT_DEVICE_BOARD + 1u)) - 1u)

typedef struct vm_at_port_leaf {
    vm_at_device_role device;
    lib_u16 port;
    lib_u8 read;
    lib_u8 write;
} vm_at_port_leaf;

typedef enum vm_at_route_source {
    VM_AT_ROUTE_PIT_IRQ0,
    VM_AT_ROUTE_KBC_KEYBOARD_IRQ1,
    VM_AT_ROUTE_KBC_AUX_IRQ12,
    VM_AT_ROUTE_CMOS_IRQ8,
    VM_AT_ROUTE_FDC_IRQ6_DMA2
} vm_at_route_source;

typedef struct vm_at_route {
    vm_at_route_source source;
    lib_u8 irq;
    lib_u8 dma_channel;
} vm_at_route;

/* Immutable AT endpoint grammar; compositions select enabled device bits.
 * These are declarations, never live chip/register state or a model selector. */
#define VM_AT_PORT_LEAF_COUNT 79u
extern const vm_at_port_leaf vm_at_port_leaves[VM_AT_PORT_LEAF_COUNT];
extern const vm_at_route vm_at_routes_with_aux[5u];
extern const vm_at_route vm_at_routes_without_aux[4u];

/* Borrowed immutable results. Enabled roles are explicit composition input. */
const vm_at_port_leaf *vm_at_port_leaf_find(const vm_at_port_leaf *leaves,
    lib_size count, lib_u32 enabled, vm_at_device_role device, lib_u16 port);
const vm_at_port_leaf *vm_at_port_leaf_at(const vm_at_port_leaf *leaves,
    lib_size count, lib_u32 enabled, vm_at_device_role device, lib_size ordinal);
const vm_at_route *vm_at_route_find(const vm_at_route *routes,
    lib_size count, vm_at_route_source source);

#endif
