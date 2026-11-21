#include "app-myob300/ob300_mouse.h"
#include "lib/base/input_interface.h"

#define MYOB300_MOUSE_PORT 0x238u
#define MYOB300_MOUSE_IRQ 12u
#define MYOB300_MOUSE_COUNTER_MASK 0xfffu
#define MYOB300_MOUSE_BUTTON_IRQ 0x10u
#define MYOB300_MOUSE_MOTION_IRQ 0x20u
#define MYOB300_MOUSE_LEFT_RELEASED 0x80u
#define MYOB300_MOUSE_RIGHT_RELEASED 0x40u

void myob300_mouse_reset(myob300_mouse *mouse)
{
    x86_nxvm_board_pic_irq_source_deassert(mouse->irq);
    *mouse = (myob300_mouse){.irq = mouse->irq};
}

static lib_status myob300_mouse_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *value)
{
    myob300_mouse *mouse = owner;

    (void)tick;
    switch (port - MYOB300_MOUSE_PORT) {
    case 0u: *value = mouse->x & 0xffu; break;
    case 1u: *value = mouse->x >> 8u; break;
    case 2u: *value = mouse->y & 0xffu; break;
    case 3u:
        *value = (mouse->y >> 8u) |
            ((mouse->buttons & BASE_MOUSE_BUTTON_LEFT) == 0u ? MYOB300_MOUSE_LEFT_RELEASED : 0u) |
            ((mouse->buttons & BASE_MOUSE_BUTTON_RIGHT) == 0u ? MYOB300_MOUSE_RIGHT_RELEASED : 0u);
        x86_nxvm_board_pic_irq_source_deassert(mouse->irq);
        break;
    default: return LIB_STATUS_UNSUPPORTED;
    }
    return LIB_STATUS_OK;
}

static lib_status myob300_mouse_write(void *owner, lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    myob300_mouse *mouse = owner;

    if (access_bytes != 1u) return LIB_STATUS_UNSUPPORTED;
    if (port == MYOB300_MOUSE_PORT) {
        mouse->sensing = value != 0u;
        x86_nxvm_board_pic_irq_source_deassert(mouse->irq);
    } else {
        mouse->control = (lib_u8)value & (MYOB300_MOUSE_BUTTON_IRQ | MYOB300_MOUSE_MOTION_IRQ);
        if (mouse->control == 0u) x86_nxvm_board_pic_irq_source_deassert(mouse->irq);
    }
    return LIB_STATUS_OK;
}

lib_status myob300_mouse_install(myob300_mouse *mouse,
    const x86_nxvm_board_profile_services *services)
{
    const x86_core_machine_port_route routes[] = {
        {MYOB300_MOUSE_PORT, myob300_mouse_read, myob300_mouse_write, mouse, LIB_FALSE, 0u},
        {MYOB300_MOUSE_PORT + 1u, myob300_mouse_read, LIB_NULL, mouse, LIB_FALSE, 0u},
        {MYOB300_MOUSE_PORT + 2u, myob300_mouse_read, LIB_NULL, mouse, LIB_FALSE, 0u},
        {MYOB300_MOUSE_PORT + 3u, myob300_mouse_read, myob300_mouse_write, mouse, LIB_FALSE, 0u}
    };
    const lib_status status = x86_nxvm_board_pic_irq_source_bind(&mouse->irq,
        services->pic_master, services->pic_slave, MYOB300_MOUSE_IRQ);

    if (status != LIB_STATUS_OK) return status;
    return x86_core_machine_install_port_routes(services->core, routes,
        sizeof(routes) / sizeof(routes[0]));
}

void myob300_mouse_receive(myob300_mouse *mouse, lib_i32 dx, lib_i32 dy, lib_u32 buttons)
{
    const lib_u8 next_buttons = (lib_u8)(buttons & (BASE_MOUSE_BUTTON_LEFT | BASE_MOUSE_BUTTON_RIGHT));
    const lib_u8 causes = (dx != 0 || dy != 0 ? MYOB300_MOUSE_MOTION_IRQ : 0u) |
        (next_buttons != mouse->buttons ? MYOB300_MOUSE_BUTTON_IRQ : 0u);

    /* L2: relative host movement drives the OEM controller's wrapping counters. */
    mouse->x = (lib_u16)((mouse->x - (lib_u32)dx) & MYOB300_MOUSE_COUNTER_MASK);
    mouse->y = (lib_u16)((mouse->y + (lib_u32)dy) & MYOB300_MOUSE_COUNTER_MASK);
    mouse->buttons = next_buttons;
    if (mouse->sensing && (mouse->control & causes) != 0u)
        x86_nxvm_board_pic_irq_source_assert(mouse->irq);
}
