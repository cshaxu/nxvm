/* Copyright 2012-2026 Neko. */
#include "core/chips/ps2mouse/ps2mouse_interface.h"

typedef enum mouse_parameter {
    MOUSE_PARAMETER_NONE,
    MOUSE_PARAMETER_SAMPLE_RATE,
    MOUSE_PARAMETER_RESOLUTION
} mouse_parameter;

struct x86_ps2_mouse {
    mouse_parameter pending_parameter;
    lib_bool reporting_enabled;
    lib_u8 buttons;
    lib_u8 resolution;
    lib_u8 sample_rate;
};

lib_status x86_ps2_mouse_create(x86_ps2_mouse **out_mouse)
{
    x86_ps2_mouse *mouse;

    if (out_mouse == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_mouse = LIB_NULL;
    mouse = lib_allocate_zero(1u, sizeof(*mouse));
    if (mouse == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    x86_ps2_mouse_reset(mouse);
    *out_mouse = mouse;
    return LIB_STATUS_OK;
}

void x86_ps2_mouse_destroy(x86_ps2_mouse *mouse)
{
    lib_release(mouse);
}

void x86_ps2_mouse_reset(x86_ps2_mouse *mouse)
{
    if (mouse == LIB_NULL) return;
    mouse->reporting_enabled = LIB_FALSE;
    mouse->buttons = 0u;
    mouse->resolution = 2u;
    mouse->sample_rate = 100u;
    mouse->pending_parameter = MOUSE_PARAMETER_NONE;
}

static lib_bool mouse_is_sample_rate(lib_u8 value)
{
    switch (value) {
    case 10u: case 20u: case 40u: case 60u: case 80u: case 100u: case 200u:
        return LIB_TRUE;
    default:
        return LIB_FALSE;
    }
}

lib_status x86_ps2_mouse_write(x86_ps2_mouse *mouse, lib_u8 value,
    x86_ps2_mouse_reply *out_reply)
{
    if (mouse == LIB_NULL || out_reply == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    *out_reply = (x86_ps2_mouse_reply){ { 0xfau }, 1u };
    if (mouse->pending_parameter == MOUSE_PARAMETER_SAMPLE_RATE) {
        mouse->pending_parameter = MOUSE_PARAMETER_NONE;
        if (mouse_is_sample_rate(value)) mouse->sample_rate = value;
        else out_reply->bytes[0] = 0xfeu;
        return LIB_STATUS_OK;
    }
    if (mouse->pending_parameter == MOUSE_PARAMETER_RESOLUTION) {
        mouse->pending_parameter = MOUSE_PARAMETER_NONE;
        if (value <= 3u) mouse->resolution = value;
        else out_reply->bytes[0] = 0xfeu;
        return LIB_STATUS_OK;
    }
    switch (value) {
    case 0xffu:
        x86_ps2_mouse_reset(mouse);
        out_reply->bytes[1] = 0xaau;
        out_reply->bytes[2] = 0x00u;
        out_reply->count = 3u;
        break;
    case 0xf6u:
        x86_ps2_mouse_reset(mouse);
        break;
    case 0xf5u:
        mouse->reporting_enabled = LIB_FALSE;
        break;
    case 0xf4u:
        mouse->reporting_enabled = LIB_TRUE;
        break;
    case 0xf2u:
        out_reply->bytes[1] = 0x00u;
        out_reply->count = 2u;
        break;
    case 0xf3u:
        mouse->pending_parameter = MOUSE_PARAMETER_SAMPLE_RATE;
        break;
    case 0xe8u:
        mouse->pending_parameter = MOUSE_PARAMETER_RESOLUTION;
        break;
    case 0xe9u:
        out_reply->bytes[1] = (lib_u8)(mouse->buttons |
            (mouse->reporting_enabled ? 0x20u : 0u));
        out_reply->bytes[2] = mouse->resolution;
        out_reply->bytes[3] = mouse->sample_rate;
        out_reply->count = 4u;
        break;
    default:
        out_reply->bytes[0] = 0xfeu;
        break;
    }
    return LIB_STATUS_OK;
}

static void mouse_encode_delta(lib_i16 delta, lib_u8 sign_bit,
    lib_u8 overflow_bit, lib_u8 *packet_first, lib_u8 *packet_data)
{
    if (delta > 255) {
        *packet_first |= overflow_bit;
        *packet_data = 0xffu;
    } else if (delta < -256) {
        *packet_first |= sign_bit | overflow_bit;
        *packet_data = 0x00u;
    } else {
        *packet_data = (lib_u8)delta;
        if (delta < 0) *packet_first |= sign_bit;
    }
}

lib_status x86_ps2_mouse_report(x86_ps2_mouse *mouse,
    lib_i16 delta_x, lib_i16 delta_y, lib_u8 buttons,
    x86_ps2_mouse_receive receive, void *context)
{
    lib_u8 packet[3];
    lib_status status;

    if (mouse == LIB_NULL || receive == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (!mouse->reporting_enabled) return LIB_STATUS_INVALID_STATE;
    buttons &= 0x07u;
    if (delta_x == 0 && delta_y == 0 && buttons == mouse->buttons) {
        return LIB_STATUS_OK;
    }
    packet[0] = (lib_u8)(0x08u | buttons);
    mouse_encode_delta(delta_x, 0x10u, 0x40u, &packet[0], &packet[1]);
    mouse_encode_delta(delta_y, 0x20u, 0x80u, &packet[0], &packet[2]);
    status = receive(context, packet);
    if (status == LIB_STATUS_OK) mouse->buttons = buttons;
    return status;
}
