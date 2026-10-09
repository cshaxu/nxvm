#include "core/board-at/wiring_interface.h"

const vm_at_port_leaf *vm_at_port_leaf_find(const vm_at_port_leaf *leaves,
    lib_size count, lib_u32 enabled, vm_at_device_role device, lib_u16 port)
{
    if (leaves == LIB_NULL || device < VM_AT_DEVICE_PIC || device > VM_AT_DEVICE_BOARD ||
        (enabled & (1u << device)) == 0u) return LIB_NULL;
    for (lib_size index = 0u; index < count; ++index) {
        if (leaves[index].device == device && leaves[index].port == port)
            return &leaves[index];
    }
    return LIB_NULL;
}

const vm_at_port_leaf *vm_at_port_leaf_at(const vm_at_port_leaf *leaves,
    lib_size count, lib_u32 enabled, vm_at_device_role device, lib_size ordinal)
{
    if (leaves == LIB_NULL || device < VM_AT_DEVICE_PIC || device > VM_AT_DEVICE_BOARD ||
        (enabled & (1u << device)) == 0u) return LIB_NULL;
    for (lib_size index = 0u; index < count; ++index) {
        if (leaves[index].device == device) {
            if (ordinal == 0u) return &leaves[index];
            --ordinal;
        }
    }
    return LIB_NULL;
}

const vm_at_route *vm_at_route_find(const vm_at_route *routes,
    lib_size count, vm_at_route_source source)
{
    if (routes == LIB_NULL) return LIB_NULL;
    for (lib_size index = 0u; index < count; ++index) {
        if (routes[index].source == source) return &routes[index];
    }
    return LIB_NULL;
}

const vm_at_port_leaf vm_at_port_leaves[] = {
    { VM_AT_DEVICE_PIC, 0x0020u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_PIC, 0x0021u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_PIC, 0x00a0u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_PIC, 0x00a1u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_PIT, 0x0040u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_PIT, 0x0041u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_PIT, 0x0042u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_PIT, 0x0043u, LIB_FALSE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x0000u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x0001u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x0002u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x0003u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x0004u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x0005u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x0006u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x0007u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x0008u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x0009u, LIB_FALSE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x000au, LIB_FALSE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x000bu, LIB_FALSE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x000cu, LIB_FALSE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x000du, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x000eu, LIB_FALSE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x000fu, LIB_FALSE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x0081u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x0082u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x0083u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x0087u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x0089u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x008au, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x008bu, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x008fu, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x00c0u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x00c2u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x00c4u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x00c6u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x00c8u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x00cau, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x00ccu, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x00ceu, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x00d0u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x00d2u, LIB_FALSE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x00d4u, LIB_FALSE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x00d6u, LIB_FALSE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x00d8u, LIB_FALSE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x00dau, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x00dcu, LIB_FALSE, LIB_TRUE },
    { VM_AT_DEVICE_DMA, 0x00deu, LIB_FALSE, LIB_TRUE },
    { VM_AT_DEVICE_KBC, 0x0060u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_KBC, 0x0064u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_BOARD, 0x0061u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_MEMORY_CONTROL, 0x0092u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_VADP_ATTRIBUTE, 0x03c0u, LIB_FALSE, LIB_TRUE },
    { VM_AT_DEVICE_VADP_ATTRIBUTE, 0x03c1u, LIB_TRUE, LIB_FALSE },
    { VM_AT_DEVICE_VADP_SEQUENCER, 0x03c4u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_VADP_SEQUENCER, 0x03c5u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_VADP_GRAPHICS, 0x03ceu, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_VADP_GRAPHICS, 0x03cfu, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_VADP, 0x03d4u, LIB_FALSE, LIB_TRUE },
    { VM_AT_DEVICE_VADP, 0x03d5u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_VADP, 0x03d8u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_VADP, 0x03d9u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_VADP, 0x03dau, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_VADP, 0x03c2u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_CMOS, 0x0070u, LIB_FALSE, LIB_TRUE },
    { VM_AT_DEVICE_CMOS, 0x0071u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_FDC, 0x03f2u, LIB_FALSE, LIB_TRUE },
    { VM_AT_DEVICE_FDC, 0x03f4u, LIB_TRUE, LIB_FALSE },
    { VM_AT_DEVICE_FDC, 0x03f5u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_FDC, 0x03f7u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_HDC, 0x01f0u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_HDC, 0x01f1u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_HDC, 0x01f2u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_HDC, 0x01f3u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_HDC, 0x01f4u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_HDC, 0x01f5u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_HDC, 0x01f6u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_HDC, 0x01f7u, LIB_TRUE, LIB_TRUE },
    { VM_AT_DEVICE_HDC, 0x03f6u, LIB_TRUE, LIB_TRUE }
};

const vm_at_route vm_at_routes_with_aux[] = {
    { VM_AT_ROUTE_PIT_IRQ0, 0u,
        VM_AT_NO_DMA_CHANNEL },
    { VM_AT_ROUTE_KBC_KEYBOARD_IRQ1, 1u,
        VM_AT_NO_DMA_CHANNEL },
    { VM_AT_ROUTE_KBC_AUX_IRQ12, 12u,
        VM_AT_NO_DMA_CHANNEL },
    { VM_AT_ROUTE_CMOS_IRQ8, 8u,
        VM_AT_NO_DMA_CHANNEL },
    { VM_AT_ROUTE_FDC_IRQ6_DMA2, 6u, 2u }
};

const vm_at_route vm_at_routes_without_aux[] = {
    { VM_AT_ROUTE_PIT_IRQ0, 0u,
        VM_AT_NO_DMA_CHANNEL },
    { VM_AT_ROUTE_KBC_KEYBOARD_IRQ1, 1u,
        VM_AT_NO_DMA_CHANNEL },
    { VM_AT_ROUTE_CMOS_IRQ8, 8u,
        VM_AT_NO_DMA_CHANNEL },
    { VM_AT_ROUTE_FDC_IRQ6_DMA2, 6u, 2u }
};
