#include "core/chips/ps2mouse/ps2mouse_interface.h"

typedef struct packet_probe {
    lib_u8 packet[3];
    lib_u32 calls;
    lib_status result;
} packet_probe;

static lib_status receive_packet(void *context, const lib_u8 packet[3])
{
    packet_probe *probe = context;
    lib_memory_copy(probe->packet, packet, sizeof(probe->packet));
    ++probe->calls;
    return probe->result;
}

static lib_bool reply_is(x86_ps2_mouse *mouse, lib_u8 value,
    lib_u8 count, lib_u8 first, lib_u8 second, lib_u8 third, lib_u8 fourth)
{
    const lib_u8 expected[] = { first, second, third, fourth };
    x86_ps2_mouse_reply reply;
    return x86_ps2_mouse_write(mouse, value, &reply) == LIB_STATUS_OK &&
        reply.count == count && lib_memory_compare(reply.bytes, expected, count) == 0;
}

static lib_bool commands(x86_ps2_mouse *mouse)
{
    lib_u16 value;
    lib_bool failed = LIB_FALSE;
    for (value = 0u; value < 256u; ++value) {
        lib_u8 count = 1u;
        lib_u8 first = 0xfau;
        lib_u8 second = 0u;
        lib_u8 third = 0u;
        lib_u8 fourth = 0u;
        x86_ps2_mouse_reset(mouse);
        switch (value) {
        case 0xffu: count = 3u; second = 0xaau; break;
        case 0xf2u: count = 2u; break;
        case 0xe9u: count = 4u; third = 2u; fourth = 100u; break;
        case 0xf6u: case 0xf5u: case 0xf4u: case 0xf3u: case 0xe8u: break;
        default: first = 0xfeu; break;
        }
        failed |= !reply_is(mouse, (lib_u8)value, count, first, second, third, fourth);
    }
    for (value = 0u; value < 256u; ++value) {
        const lib_bool valid_rate = value == 10u || value == 20u ||
            value == 40u || value == 60u || value == 80u ||
            value == 100u || value == 200u;
        x86_ps2_mouse_reset(mouse);
        failed |= !reply_is(mouse, 0xf3u, 1u, 0xfau, 0u, 0u, 0u);
        failed |= !reply_is(mouse, (lib_u8)value, 1u,
            valid_rate ? 0xfau : 0xfeu, 0u, 0u, 0u);
        failed |= !reply_is(mouse, 0xe9u, 4u, 0xfau, 0u, 2u,
            valid_rate ? (lib_u8)value : 100u);
        failed |= !reply_is(mouse, 0xe8u, 1u, 0xfau, 0u, 0u, 0u);
        failed |= !reply_is(mouse, (lib_u8)value, 1u,
            value <= 3u ? 0xfau : 0xfeu, 0u, 0u, 0u);
        failed |= !reply_is(mouse, 0xe9u, 4u, 0xfau, 0u,
            value <= 3u ? (lib_u8)value : 2u,
            valid_rate ? (lib_u8)value : 100u);
    }
    return failed;
}

static lib_bool reports(x86_ps2_mouse *mouse)
{
    static const struct {
        lib_i16 delta;
        lib_u8 byte;
        lib_bool negative;
        lib_bool overflow;
    } cases[] = {
        { -32768, 0x00u, LIB_TRUE, LIB_TRUE },
        { -257, 0x00u, LIB_TRUE, LIB_TRUE },
        { -256, 0x00u, LIB_TRUE, LIB_FALSE },
        { -1, 0xffu, LIB_TRUE, LIB_FALSE },
        { 0, 0x00u, LIB_FALSE, LIB_FALSE },
        { 1, 0x01u, LIB_FALSE, LIB_FALSE },
        { 255, 0xffu, LIB_FALSE, LIB_FALSE },
        { 256, 0xffu, LIB_FALSE, LIB_TRUE },
        { 32767, 0xffu, LIB_FALSE, LIB_TRUE }
    };
    packet_probe probe = { { 0u }, 0u, LIB_STATUS_OK };
    lib_size x;
    lib_size y;
    lib_u8 buttons;
    lib_bool failed = LIB_FALSE;
    for (x = 0u; x < sizeof(cases) / sizeof(cases[0]); ++x) {
        for (y = 0u; y < sizeof(cases) / sizeof(cases[0]); ++y) {
            for (buttons = 0u; buttons < 8u; ++buttons) {
                const lib_u8 first = (lib_u8)(0x08u | buttons |
                    (cases[x].negative ? 0x10u : 0u) |
                    (cases[y].negative ? 0x20u : 0u) |
                    (cases[x].overflow ? 0x40u : 0u) |
                    (cases[y].overflow ? 0x80u : 0u));
                probe.calls = 0u;
                x86_ps2_mouse_reset(mouse);
                failed |= !reply_is(mouse, 0xf4u, 1u, 0xfau, 0u, 0u, 0u);
                failed |= x86_ps2_mouse_report(mouse, cases[x].delta,
                    cases[y].delta, (lib_u8)(buttons | 0xf8u), receive_packet,
                    &probe) != LIB_STATUS_OK;
                if (cases[x].delta == 0 && cases[y].delta == 0 && buttons == 0u) {
                    failed |= probe.calls != 0u;
                } else {
                    failed |= probe.calls != 1u || probe.packet[0] != first ||
                        probe.packet[1] != cases[x].byte || probe.packet[2] != cases[y].byte;
                }
                failed |= !reply_is(mouse, 0xe9u, 4u, 0xfau,
                    (lib_u8)(0x20u | buttons), 2u, 100u);
            }
        }
    }
    return failed;
}

static lib_bool failure_and_reset(x86_ps2_mouse *mouse)
{
    packet_probe probe = { { 0u }, 0u, LIB_STATUS_INVALID_STATE };
    x86_ps2_mouse_reply reply;
    lib_bool failed = LIB_FALSE;
    x86_ps2_mouse_reset(mouse);
    failed |= x86_ps2_mouse_write(mouse, 0xf4u, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= x86_ps2_mouse_report(mouse, 1, 1, 0u, receive_packet,
        &probe) != LIB_STATUS_INVALID_STATE || probe.calls != 0u;
    failed |= !reply_is(mouse, 0xf4u, 1u, 0xfau, 0u, 0u, 0u);
    failed |= x86_ps2_mouse_report(mouse, 1, 1, 1u, receive_packet,
        &probe) != LIB_STATUS_INVALID_STATE || probe.calls != 1u;
    failed |= !reply_is(mouse, 0xe9u, 4u, 0xfau, 0x20u, 2u, 100u);
    failed |= x86_ps2_mouse_report(mouse, 0, 0, 0u, receive_packet,
        &probe) != LIB_STATUS_OK || probe.calls != 1u;
    probe.result = LIB_STATUS_OK;
    failed |= x86_ps2_mouse_report(mouse, 0, 0, 1u, receive_packet,
        &probe) != LIB_STATUS_OK || probe.calls != 2u;
    failed |= x86_ps2_mouse_report(mouse, 0, 0, 1u, receive_packet,
        &probe) != LIB_STATUS_OK || probe.calls != 2u;
    failed |= !reply_is(mouse, 0xe9u, 4u, 0xfau, 0x21u, 2u, 100u);
    failed |= !reply_is(mouse, 0xf3u, 1u, 0xfau, 0u, 0u, 0u);
    x86_ps2_mouse_reset(mouse);
    failed |= !reply_is(mouse, 0xe9u, 4u, 0xfau, 0u, 2u, 100u);
    failed |= !reply_is(mouse, 0xe8u, 1u, 0xfau, 0u, 0u, 0u);
    x86_ps2_mouse_reset(mouse);
    failed |= !reply_is(mouse, 0xe9u, 4u, 0xfau, 0u, 2u, 100u);
    failed |= x86_ps2_mouse_write(LIB_NULL, 0xf4u, &reply) != LIB_STATUS_INVALID_ARGUMENT;
    failed |= x86_ps2_mouse_report(mouse, 0, 0, 0u, LIB_NULL, LIB_NULL) !=
        LIB_STATUS_INVALID_ARGUMENT;
    failed |= x86_ps2_mouse_report(LIB_NULL, 0, 0, 0u, receive_packet, &probe) !=
        LIB_STATUS_INVALID_ARGUMENT;
    return failed;
}

int main(void)
{
    x86_ps2_mouse *mouse = LIB_NULL;
    lib_bool failed;
    if (x86_ps2_mouse_create(LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT ||
        x86_ps2_mouse_create(&mouse) != LIB_STATUS_OK) return 1;
    failed = commands(mouse);
    failed |= reports(mouse);
    failed |= failure_and_reset(mouse);
    x86_ps2_mouse_destroy(mouse);
    x86_ps2_mouse_reset(LIB_NULL);
    x86_ps2_mouse_destroy(LIB_NULL);
    return failed ? 1 : 0;
}
