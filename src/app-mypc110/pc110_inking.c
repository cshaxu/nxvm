#include "app-mypc110/pc110_inking_interface.h"

/* L2 register and packet model based on Open-Source-PC110 Discovery/Inking.
 * This does not claim the timing of the physical digitizer. */
static lib_status mypc110_inking_selection(const mypc110_inking *inking,
    lib_u16 *out_base, lib_u8 *out_irq)
{
    static const lib_u8 irqs[] = {3u, 4u, 10u, 11u, 12u, 15u, 5u, 7u};
    lib_u8 value;
    lib_status status;
    if (inking->configuration == LIB_NULL) {
        *out_base = inking->base;
        *out_irq = 0u;
        return LIB_STATUS_OK;
    }
    status = x86_vl82c420_read_register(inking->configuration,
        X86_VL82C420_BANK_SCAMP, 0x16u, &value);
    if (status != LIB_STATUS_OK) return status;
    if ((value & 0x80u) == 0u) return LIB_STATUS_INVALID_STATE;
    *out_base = (lib_u16)(0x05e0u + (value & 3u) * 0x1000u);
    *out_irq = irqs[(value >> 4u) & 7u];
    return LIB_STATUS_OK;
}

static void mypc110_inking_start_packet(mypc110_inking *inking,
    lib_u8 x, lib_u8 y, lib_bool down)
{
    inking->packet[0] = (lib_u8)(0x80u | ((x & 0x80u) >> 3u) |
        ((y & 0x80u) >> 4u) | (down ? 1u : 0u));
    inking->packet[1] = x & 0x7fu;
    inking->packet[2] = y & 0x7fu;
    inking->position = 0u;
    inking->pending = LIB_TRUE;
    x86_nxvm_board_pic_irq_source_assert(inking->irq);
}

static lib_status mypc110_inking_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    mypc110_inking *inking = owner;
    lib_u16 base;
    lib_u8 irq;
    lib_status status;
    (void)tick;
    if (inking == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = mypc110_inking_selection(inking, &base, &irq);
    if (status == LIB_STATUS_INVALID_STATE ||
        (status == LIB_STATUS_OK && (port < base || port > base + 2u))) {
        *out_value = 0xffu;
        return LIB_STATUS_OK;
    }
    if (status != LIB_STATUS_OK) return status;
    switch (port - base) {
    case 0u: *out_value = inking->pending ? inking->packet[inking->position] : 0u; break;
    case 1u: *out_value = (inking->control & 0xfeu) == 0x38u ?
        (inking->pending ? 0xffu : 0x7fu) : 0xffu; break;
    case 2u: *out_value = inking->control; break;
    default: return LIB_STATUS_UNSUPPORTED;
    }
    return LIB_STATUS_OK;
}

static lib_status mypc110_inking_write(void *owner, lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    mypc110_inking *inking = owner;
    lib_u8 previous;
    lib_u16 base;
    lib_u8 irq;
    lib_status status;
    if (inking == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = mypc110_inking_selection(inking, &base, &irq);
    if (status == LIB_STATUS_INVALID_STATE ||
        (status == LIB_STATUS_OK && port != base + 2u)) return LIB_STATUS_OK;
    if (status != LIB_STATUS_OK) return status;
    if (value > 0xffu || access_bytes != 1u) return LIB_STATUS_INVALID_ARGUMENT;
    previous = inking->control;
    inking->control = (lib_u8)value;
    if ((inking->control & 0xfeu) != 0x38u) {
        inking->pending = LIB_FALSE;
        inking->waiting_count = 0u;
        x86_nxvm_board_pic_irq_source_deassert(inking->irq);
    } else if (inking->pending && (previous & 1u) == 0u && (value & 1u) != 0u) {
        x86_nxvm_board_pic_irq_source_deassert(inking->irq);
    } else if (inking->pending && (previous & 1u) != 0u && (value & 1u) == 0u) {
        ++inking->position;
        if (inking->position == 3u && inking->waiting_count != 0u) {
            const mypc110_inking_point next = inking->waiting[0];
            --inking->waiting_count;
            for (lib_u8 i = 0u; i < inking->waiting_count; ++i)
                inking->waiting[i] = inking->waiting[i + 1u];
            mypc110_inking_start_packet(inking, next.x, next.y, next.down);
        } else if (inking->position == 3u) inking->pending = LIB_FALSE;
        else x86_nxvm_board_pic_irq_source_assert(inking->irq);
    }
    return LIB_STATUS_OK;
}

void mypc110_inking_reset(mypc110_inking *inking)
{
    if (inking == LIB_NULL) return;
    x86_nxvm_board_pic_irq_source_deassert(inking->irq);
    inking->control = 0xc0u;
    inking->position = 0u;
    inking->pending = LIB_FALSE;
    inking->waiting_count = 0u;
}

lib_status mypc110_inking_sample(mypc110_inking *inking,
    lib_u8 x, lib_u8 y, lib_bool down)
{
    lib_u16 base;
    lib_u8 irq;
    lib_status status;
    if (inking == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if ((inking->control & 0xfeu) != 0x38u)
        return LIB_STATUS_INVALID_STATE;
    status = mypc110_inking_selection(inking, &base, &irq);
    if (status != LIB_STATUS_OK) return status;
    if (inking->pending) {
        if (inking->waiting_count != 0u &&
            inking->waiting[inking->waiting_count - 1u].down == down) {
            inking->waiting[inking->waiting_count - 1u] = (mypc110_inking_point){x, y, down};
            return LIB_STATUS_OK;
        }
        if (inking->waiting_count == sizeof(inking->waiting) / sizeof(inking->waiting[0]))
            return LIB_STATUS_LIMIT_EXCEEDED;
        inking->waiting[inking->waiting_count++] = (mypc110_inking_point){x, y, down};
        return LIB_STATUS_OK;
    }
    if (inking->configuration != LIB_NULL) {
        status = x86_nxvm_board_pic_irq_source_bind(&inking->irq,
            inking->pic_master, inking->pic_slave, irq);
        if (status != LIB_STATUS_OK) return status;
    } else if (inking->irq == LIB_NULL) return LIB_STATUS_INVALID_STATE;
    mypc110_inking_start_packet(inking, x, y, down);
    return LIB_STATUS_OK;
}

lib_status mypc110_inking_install_routes(x86_core_machine *machine,
    mypc110_inking *inking, lib_u16 base)
{
    if (machine == LIB_NULL || inking == LIB_NULL || base > 0xfffdu)
        return LIB_STATUS_INVALID_ARGUMENT;
    x86_core_machine_port_route routes[12];
    lib_size count = 0u;
    const lib_size windows = inking->configuration == LIB_NULL ? 1u : 4u;
    inking->base = base;
    mypc110_inking_reset(inking);
    for (lib_size index = 0u; index < windows; ++index) {
        const lib_u16 address = inking->configuration == LIB_NULL ? base :
            (lib_u16)(0x05e0u + index * 0x1000u);
        routes[count++] = (x86_core_machine_port_route){address,
            mypc110_inking_read, LIB_NULL, inking, LIB_FALSE, 0u};
        routes[count++] = (x86_core_machine_port_route){(lib_u16)(address + 1u),
            mypc110_inking_read, LIB_NULL, inking, LIB_FALSE, 0u};
        routes[count++] = (x86_core_machine_port_route){(lib_u16)(address + 2u),
            mypc110_inking_read, mypc110_inking_write, inking, LIB_FALSE, 0u};
    }
    return x86_core_machine_install_port_routes(machine, routes, count);
}
