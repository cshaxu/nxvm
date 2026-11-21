#include "app-myob300/ob300_board_interface.h"
#include "app-myob300/ob300_card.h"
#include "app-myob300/ob300_machine_interface.h"
#include "lib/base/sync_interface.h"
#include "lib/types/file.h"
#include "lib/types/test.h"
#include "x86-core/machine/debug_interface.h"
#include "x86-core/machine/memory_interface.h"
#include "x86-debug/product/protocol_interface.h"
#include "x86-nxvm/machine/machine_interface.h"

static void write_port(x86_core_machine *core, lib_u16 port, lib_u8 value)
{
    lib_test_assert(x86_core_machine_bus_write(core, port, value) == LIB_STATUS_OK);
}

static void write_indexed(x86_core_machine *core, lib_u16 port, lib_u16 data_port,
    lib_u8 index, lib_u8 value)
{
    write_port(core, port, index);
    write_port(core, data_port, value);
}

static void expect_port(x86_core_machine *core, lib_u16 port, lib_u8 expected)
{
    lib_u32 value;

    lib_test_assert(x86_core_machine_bus_read(core, port, &value) == LIB_STATUS_OK);
    if ((lib_u8)value != expected)
        lib_c_fprintf(lib_c_stderr, "port %04x: expected %02x, received %02x\n",
            (lib_u32)port, (lib_u32)expected, value);
    lib_test_assert((lib_u8)value == expected);
}

static void test_chipset(x86_core_machine *core, myob300_board *profile)
{
    lib_u32 value;

    expect_port(core, 0xb4u, 128u);
    expect_port(core, 0xb6u, 180u);
    write_port(core, 0xecu, 0x18u);
    expect_port(core, 0xecu, 0u);
    expect_port(core, 0xedu, 0x6au);
    write_port(core, 0xfbu, 0x39u);
    write_port(core, 0xecu, 0x13u);
    expect_port(core, 0xedu, 0x05u);
    write_port(core, 0xedu, 0x83u);
    expect_port(core, 0xedu, 0x85u);
    write_port(core, 0xedu, 0x01u);
    expect_port(core, 0xedu, 0x05u);
    write_port(core, 0xecu, 0x18u);
    expect_port(core, 0xedu, 0x20u);
    write_port(core, 0xedu, 0x2fu);
    expect_port(core, 0xedu, 0x27u);
    write_port(core, 0xf9u, 0xa4u);
    write_port(core, 0xedu, 0u);
    expect_port(core, 0xedu, 0x27u);
    write_port(core, 0xfbu, 0u);
    write_port(core, 0xedu, 0x37u);
    write_port(core, 0xedu, 0x20u);
    expect_port(core, 0xedu, 0x37u);
    write_port(core, 0xecu, 0xffu);
    lib_test_assert(x86_core_machine_bus_read(core, 0xedu, &value) == LIB_STATUS_UNSUPPORTED);
    lib_test_assert(x86_core_machine_bus_write(core, 0xedu, 0u) == LIB_STATUS_UNSUPPORTED);
    lib_test_assert(x86_core_machine_bus_read(core, 0x3d8u, &value) == LIB_STATUS_UNSUPPORTED);

    write_port(core, 0xecu, 0x03u);
    expect_port(core, 0xedu, 0x03u);
    write_port(core, 0xedu, 0x13u);
    expect_port(core, 0xedu, 0x13u);

    write_port(core, 0xecu, 0x33u);
    expect_port(core, 0xedu, 0x08u);
    write_port(core, 0xedu, 0x20u);
    expect_port(core, 0xedu, 0x28u);
    write_indexed(core, 0xecu, 0xedu, 0x35u, 0x18u);
    expect_port(core, 0xedu, 0u);
    write_port(core, 0xecu, 0x3eu);
    expect_port(core, 0xedu, 0u);
    write_indexed(core, 0xecu, 0xedu, 0x3fu, 0x04u);
    expect_port(core, 0xedu, 0x04u);

    write_indexed(core, 0xecu, 0xedu, 0x3au, 0xfeu);
    write_port(core, 0xecu, 0x3bu);
    expect_port(core, 0xedu, 0xffu);
    lib_test_assert(myob300_board_set_matrix_key(profile, 0u, 4u, LIB_TRUE) == LIB_STATUS_OK);
    expect_port(core, 0xedu, 0xefu);
    lib_test_assert(myob300_board_set_matrix_key(profile, 0u, 4u, LIB_FALSE) == LIB_STATUS_OK);
    expect_port(core, 0xedu, 0xffu);
}

static void configure_pic(x86_core_machine *core)
{
    const struct {
        lib_u16 port;
        lib_u8 value;
    } pic_initialization[] = {
        {0x20u, 0x11u}, {0x21u, 0x08u}, {0x21u, 0x04u}, {0x21u, 0x01u},
        {0xa0u, 0x11u}, {0xa1u, 0x70u}, {0xa1u, 0x02u}, {0xa1u, 0x01u}
    };
    for (lib_size index = 0u; index < sizeof(pic_initialization) / sizeof(pic_initialization[0]);
         ++index)
        write_port(core, pic_initialization[index].port, pic_initialization[index].value);
}

static void test_support_devices(x86_core_machine *core, x86_nxvm_board_state *board)
{
    lib_bool enabled = LIB_FALSE;
    lib_u32 value;

    expect_port(core, 0x378u, 0u);
    write_port(core, 0x378u, 0xa5u);
    expect_port(core, 0x378u, 0xa5u);
    write_port(core, 0x378u, 0x5au);
    expect_port(core, 0x378u, 0x5au);

    configure_pic(core);
    write_port(core, 0x21u, 0xf1u);
    write_port(core, 0xa1u, 0x1fu);
    expect_port(core, 0x21u, 0xf1u);
    expect_port(core, 0xa1u, 0x1fu);

    write_port(core, 0x43u, 0x30u);
    write_port(core, 0x40u, 0x56u);
    write_port(core, 0x40u, 0x34u);
    write_port(core, 0x43u, 0xe2u);
    expect_port(core, 0x40u, 0x70u);

    write_port(core, 0x81u, 0x17u);
    expect_port(core, 0x81u, 0x17u);
    write_port(core, 0x70u, 0x20u);
    write_port(core, 0x71u, 0x65u);
    expect_port(core, 0x71u, 0x65u);
    write_port(core, 0x70u, 0xf8u);
    write_port(core, 0x71u, 0x5cu);
    write_port(core, 0x70u, 0x38u);
    expect_port(core, 0x71u, 0u);
    write_port(core, 0x70u, 0x78u);
    expect_port(core, 0x71u, 0x5cu);

    lib_test_assert(x86_nxvm_board_observe_a20(board, &enabled) == LIB_STATUS_OK && !enabled);
    write_port(core, 0x92u, 2u);
    lib_test_assert(x86_nxvm_board_observe_a20(board, &enabled) == LIB_STATUS_OK && enabled);
    write_port(core, 0x92u, 0u);
    lib_test_assert(x86_nxvm_board_observe_a20(board, &enabled) == LIB_STATUS_OK && !enabled);

    expect_port(core, 0xeeu, 0xffu);
    expect_port(core, 0x92u, 2u);
    lib_test_assert(x86_nxvm_board_observe_a20(board, &enabled) == LIB_STATUS_OK && enabled);
    write_port(core, 0xeeu, 0xffu);
    expect_port(core, 0x92u, 0u);
    lib_test_assert(x86_nxvm_board_observe_a20(board, &enabled) == LIB_STATUS_OK && !enabled);

    write_port(core, 0x64u, 0xd1u);
    write_port(core, 0x60u, 3u);
    expect_port(core, 0xeeu, 0xffu);
    write_port(core, 0xeeu, 0u);
    expect_port(core, 0x92u, 0u);
    lib_test_assert(x86_nxvm_board_observe_a20(board, &enabled) == LIB_STATUS_OK && enabled);
    write_port(core, 0x64u, 0xd1u);
    write_port(core, 0x60u, 1u);
    lib_test_assert(x86_nxvm_board_observe_a20(board, &enabled) == LIB_STATUS_OK && !enabled);

    write_indexed(core, 0xecu, 0xedu, 0x14u, 0x80u);
    lib_test_assert(x86_core_machine_bus_read(core, 0xeeu, &value) == LIB_STATUS_UNSUPPORTED);
    expect_port(core, 0x92u, 0u);
    write_port(core, 0x92u, 2u);
    lib_test_assert(x86_core_machine_bus_write(core, 0xeeu, 0u) == LIB_STATUS_UNSUPPORTED);
    expect_port(core, 0x92u, 2u);
    write_port(core, 0x92u, 0u);
    write_port(core, 0xedu, 0u);
}

static void test_refresh_and_ram(x86_core_machine *core)
{
    static const lib_u8 controller_replies[][2] = {{0xaau, 0x55u}, {0xc0u, 0x84u}};
    const lib_u8 idle[] = {0xebu, 0xfeu};
    const lib_u8 written = 0x74u;
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP)
    };
    x86_core_machine_run_result result;
    lib_bool refresh_low = LIB_FALSE;
    lib_bool refresh_high = LIB_FALSE;
    lib_u8 actual;
    lib_u32 kbc_status;

    lib_test_assert(x86_core_machine_reset(core) == LIB_STATUS_OK);
    write_port(core, 0x92u, 2u);
    lib_test_assert(x86_core_machine_memory_write(core, 0x1fffffu, &written, 1u) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_memory_write(core, 0x7fffffu, &written, 1u) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_memory_read(core, 0x1fffffu, &actual, 1u) == LIB_STATUS_OK &&
        actual == written);
    lib_test_assert(x86_core_machine_memory_read(core, 0x7fffffu, &actual, 1u) == LIB_STATUS_OK &&
        actual == 0xffu);

    lib_test_assert(x86_core_machine_memory_write(core, 0u, idle, sizeof(idle)) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_debug_patch_registers(core, &entry) == LIB_STATUS_OK);
    for (lib_u32 step = 0u; step < 1000u && (!refresh_low || !refresh_high); ++step) {
        lib_u32 port;
        lib_test_assert(x86_core_machine_bus_read(core, 0x61u, &port) == LIB_STATUS_OK);
        if ((port & 0x10u) != 0u) refresh_high = LIB_TRUE;
        else refresh_low = LIB_TRUE;
        lib_test_assert(x86_core_machine_run(core, (x86_core_machine_run_budget){1u, 0u},
            &result) == LIB_STATUS_OK);
    }
    lib_test_assert(refresh_low && refresh_high);
    for (lib_size i = 0u; i < sizeof(controller_replies) / sizeof(controller_replies[0]); ++i) {
        write_port(core, 0x64u, controller_replies[i][0]);
        lib_test_assert(x86_core_machine_bus_read(core, 0x64u, &kbc_status) == LIB_STATUS_OK &&
            (kbc_status & 1u) == 0u);
        for (lib_u32 step = 0u; step < 1000u && (kbc_status & 1u) == 0u; ++step) {
            lib_test_assert(x86_core_machine_run(core, (x86_core_machine_run_budget){1u, 0u},
                &result) == LIB_STATUS_OK);
            lib_test_assert(x86_core_machine_bus_read(core, 0x64u, &kbc_status) == LIB_STATUS_OK);
        }
        lib_test_assert((kbc_status & 1u) != 0u);
        expect_port(core, 0x60u, controller_replies[i][1]);
    }
    lib_test_assert(x86_core_machine_reset(core) == LIB_STATUS_OK);
}

static void test_ab_shadow(x86_core_machine *core)
{
    const lib_u8 value = 0xa5u;
    lib_u8 actual;

    write_port(core, 0xfbu, 0u);
    write_indexed(core, 0xecu, 0xedu, 0x0eu, 1u);
    lib_test_assert(x86_core_machine_memory_write(core, 0xa0000u, &value, 1u) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_memory_read(core, 0xa0000u, &actual, 1u) == LIB_STATUS_OK &&
        actual != value);
    write_port(core, 0xedu, 3u);
    lib_test_assert(x86_core_machine_memory_read(core, 0xa0000u, &actual, 1u) == LIB_STATUS_OK &&
        actual == value);
    write_port(core, 0xedu, 0u);
}

static void expect_input(x86_core_machine *core, lib_u32 address, lib_u8 expected)
{
    x86_core_machine_run_result result;
    lib_u8 actual = 0u;

    for (lib_u32 step = 0u; step < 4000u; ++step) {
        lib_test_assert(x86_core_machine_run(core, (x86_core_machine_run_budget){1u, 0u},
            &result) == LIB_STATUS_OK);
        lib_test_assert(x86_core_machine_memory_read(core, address, &actual, 1u) == LIB_STATUS_OK);
        if (actual == expected) return;
        if (result.reason == X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            lib_u8 advanced;
            lib_test_assert(x86_core_machine_advance_to_next_deadline(core, &advanced) ==
                LIB_STATUS_OK);
        }
    }
    lib_c_fprintf(lib_c_stderr, "input at %04x: expected %02x, received %02x, PC=%08x stop=%u\n",
        address, (lib_u32)expected, (lib_u32)actual, result.linear_pc, (lib_u32)result.reason);
    lib_test_assert(actual == expected);
}

static void test_input(x86_core_machine *core, x86_nxvm_board_state *board,
    myob300_board *profile)
{
    const lib_u8 keyboard_vector[] = {0x00u, 0x03u, 0u, 0u};
    const lib_u8 mouse_vector[] = {0x10u, 0x03u, 0u, 0u};
    const lib_u8 oem_mouse_vector[] = {0x20u, 0x03u, 0u, 0u};
    /* Match the BIOS order: poll SLP with IF clear, then service IRQ. */
    const lib_u8 idle[] = {
        0xe4u, 0xedu, 0xa8u, 0x80u, 0x75u, 0xfau, 0xfbu, 0xf4u, 0xebu, 0xfdu
    };
    const lib_u8 keyboard_irq[] = {
        0xe4u, 0x60u, 0xa2u, 0x00u, 0x05u, 0xb0u, 0x20u, 0xe6u, 0x20u, 0xcfu
    };
    const lib_u8 mouse_irq[] = {
        0xe4u, 0x60u, 0xa2u, 0x01u, 0x05u, 0xb0u, 0x20u,
        0xe6u, 0xa0u, 0xe6u, 0x20u, 0xcfu
    };
    const lib_u8 oem_mouse_irq[] = {
        0xbau, 0x38u, 0x02u, 0xecu, 0xa2u, 0x02u, 0x05u,
        0xb0u, 0x20u, 0xe6u, 0xa0u, 0xe6u, 0x20u, 0xcfu
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_SS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_ESP),
        .values = {[X86_CORE_CPU_DEBUG_EIP] = 0x200u, [X86_CORE_CPU_DEBUG_ESP] = 0x800u}
    };

    lib_test_assert(x86_core_machine_reset(core) == LIB_STATUS_OK);
    configure_pic(core);
    write_port(core, 0x21u, 0xf9u);
    write_port(core, 0xa1u, 0xefu);
    write_port(core, 0x64u, 0x60u);
    write_port(core, 0x60u, 0x03u);
    lib_test_assert(x86_core_machine_memory_write(core, 0x24u,
        keyboard_vector, sizeof(keyboard_vector)) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_memory_write(core, 0x1d0u,
        mouse_vector, sizeof(mouse_vector)) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_memory_write(core, 0x200u, idle, sizeof(idle)) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_memory_write(core, 0x300u,
        keyboard_irq, sizeof(keyboard_irq)) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_memory_write(core, 0x310u,
        mouse_irq, sizeof(mouse_irq)) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_debug_patch_registers(core, &entry) == LIB_STATUS_OK);
    write_port(core, 0xfbu, 0u);
    write_indexed(core, 0xecu, 0xedu, 0x13u, 0x81u);
    expect_port(core, 0xedu, 0x85u);
    lib_test_assert(x86_nxvm_board_keyboard_receive_native_byte(board, 0x1cu) == LIB_STATUS_OK);
    expect_input(core, 0x500u, 0x1cu);
    expect_port(core, 0xedu, 0x05u);
    write_port(core, 0x64u, 0xd4u);
    write_port(core, 0x60u, 0xf4u);
    expect_input(core, 0x501u, 0xfau);
    lib_test_assert(x86_nxvm_board_mouse_receive_relative(board, 5, 3, 1u) == LIB_STATUS_OK);
    expect_input(core, 0x501u, 0x09u);
    expect_input(core, 0x501u, 0x05u);
    expect_input(core, 0x501u, 0x03u);

    lib_test_assert(x86_core_machine_memory_write(core, 0x1d0u,
        oem_mouse_vector, sizeof(oem_mouse_vector)) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_memory_write(core, 0x320u,
        oem_mouse_irq, sizeof(oem_mouse_irq)) == LIB_STATUS_OK);
    write_port(core, 0x238u, 0x08u);
    write_port(core, 0x23bu, 0x30u);
    myob300_board_receive_mouse(profile, 5, 3, BASE_MOUSE_BUTTON_LEFT);
    expect_input(core, 0x502u, 0xfbu);
    expect_port(core, 0x239u, 0x0fu);
    expect_port(core, 0x23au, 0x03u);
    expect_port(core, 0x23bu, 0x40u);
    myob300_board_receive_mouse(profile, 0, 0, 0u);
    expect_port(core, 0x23bu, 0xc0u);
}

static const struct {
    lib_u16 port;
    lib_u8 value;
} planar_mode_writes[] = {
    {0x3d4u, 1u}, {0x3d5u, 79u},
    {0x3d4u, 7u}, {0x3d5u, 2u},
    {0x3d4u, 18u}, {0x3d5u, 0xdfu},
    {0x3d4u, 19u}, {0x3d5u, 40u},
    {0x3d4u, 0x17u}, {0x3d5u, 0x80u},
    {0x3c4u, 2u}, {0x3c5u, 1u},
    {0x3c4u, 4u}, {0x3c5u, 4u},
    {0x3ceu, 6u}, {0x3cfu, 5u},
    {0x3ceu, 8u}, {0x3cfu, 0xffu},
    {0x3c0u, 0x12u}, {0x3c0u, 15u},
    {0x3c0u, 1u}, {0x3c0u, 1u},
    {0x3c8u, 1u}, {0x3c9u, 63u}, {0x3c9u, 63u}, {0x3c9u, 63u}
};

static void test_display(x86_core_machine *core, myob300_board *board)
{
    static x86_video_snapshot snapshot;
    /* Exercise the firmware's word access to the extension index/data pair. */
    const lib_u8 program[] = {
        0xbau, 0xd6u, 0x03u, 0xb8u, 0x10u, 0x20u, 0xefu, 0xedu, 0xf4u
    };
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP)
    };
    const lib_u8 pixel = 0x80u;
    const lib_u8 hidden_write = 0x40u;
    lib_u32 eax;
    x86_core_machine_run_result result;
    x86_core_machine_memory_route route;

    lib_test_assert(x86_core_machine_memory_write(core, 0u, program,
        sizeof(program)) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_debug_patch_registers(core, &entry) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_run(core, (x86_core_machine_run_budget){16u, 0u},
        &result) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_debug_read_register(core, X86_CORE_CPU_DEBUG_EAX,
        &eax) == LIB_STATUS_OK && (eax & 0xffffu) == 0x2010u);

    write_indexed(core, 0x3b4u, 0x3b5u, 9u, 15u);
    expect_port(core, 0x3b4u, 9u);
    expect_port(core, 0x3d5u, 15u);

    write_port(core, 0x3d6u, 0u);
    expect_port(core, 0x3d7u, 0x90u);
    lib_test_assert(myob300_board_capture_display(board, &snapshot) == LIB_STATUS_UNSUPPORTED);

    /* Enable 640x480 planar scanout and write a pixel via the guest aperture. */
    for (lib_size i = 0u; i < sizeof(planar_mode_writes) / sizeof(planar_mode_writes[0]); ++i)
        write_port(core, planar_mode_writes[i].port, planar_mode_writes[i].value);
    lib_test_assert(x86_core_machine_memory_query(core, 0xa0000u, 1u,
        X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, &route) == LIB_STATUS_OK &&
        route == X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER);
    lib_test_assert(x86_core_machine_memory_write(core, 0xa0000u, &pixel,
        sizeof(pixel)) == LIB_STATUS_OK);
    lib_test_assert(myob300_board_capture_display(board, &snapshot) == LIB_STATUS_OK &&
        snapshot.pixel_width == 640u && snapshot.pixel_height == 480u &&
        snapshot.pixels[0] == 1u && snapshot.pixels[1] == 0u);
    write_indexed(core, 0x3d6u, 0x3d7u, 0x0bu, 0x10u);
    lib_test_assert(x86_core_machine_memory_query(core, 0xa0000u, 1u,
        X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, &route) == LIB_STATUS_OK &&
        route == X86_CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM);
    lib_test_assert(x86_core_machine_memory_write(core, 0xa0000u, &hidden_write,
        sizeof(hidden_write)) == LIB_STATUS_OK);
    write_port(core, 0x3d7u, 0u);
    lib_test_assert(myob300_board_capture_display(board, &snapshot) == LIB_STATUS_OK &&
        snapshot.pixels[0] == 1u && snapshot.pixels[1] == 0u);
}

static void expect_memory_pair(x86_core_machine *core, lib_u32 address,
    lib_u8 first, lib_u8 second)
{
    lib_u8 pair[2];

    lib_test_assert(x86_core_machine_memory_read(core, address, pair, sizeof(pair)) == LIB_STATUS_OK);
    lib_test_assert(pair[0] == first && pair[1] == second);
}

static void test_card(x86_core_machine *core)
{
    const x86_core_cpu_debug_register_patch entry = {
        .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_CS) |
            X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EIP),
        .values = {[X86_CORE_CPU_DEBUG_CS] = 0xf000u, [X86_CORE_CPU_DEBUG_EIP] = 0x100u}
    };
    const lib_u8 write = 0xffu;
    x86_core_machine_run_result result;
    lib_u32 eax;

    expect_memory_pair(core, 0xf7ff0u, 0xa1u, 0x1au);
    expect_memory_pair(core, 0xe0000u, 0x55u, 0xaau);
    expect_memory_pair(core, 0xe8000u, 0x55u, 0xaau);
    expect_memory_pair(core, 0xe0002u, 0x40u, 0x00u);
    expect_memory_pair(core, 0xe8002u, 0x20u, 0x00u);
    lib_test_assert(x86_core_machine_memory_write(core, 0xe0000u, &write, 1u) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_memory_write(core, 0xe8000u, &write, 1u) == LIB_STATUS_OK);
    expect_memory_pair(core, 0xe0000u, 0x55u, 0xaau);
    expect_memory_pair(core, 0xe8000u, 0x55u, 0xaau);
    write_port(core, 0xfbu, 0u);
    /* ROM 1.0 ABA maps the System Card directory through its first window. */
    write_indexed(core, 0xecu, 0xedu, 0x26u, 0xf4u);
    write_indexed(core, 0xecu, 0xedu, 0x27u, 0x81u);
    write_indexed(core, 0xecu, 0xedu, 0x28u, 0xfdu);
    expect_memory_pair(core, 0xd3fd0u, 'O', 'B');
    expect_memory_pair(core, 0x00802070u, 0x36u, 0xc9u);
    lib_test_assert(x86_core_machine_memory_write(core, 0x00802070u, &write, 1u) ==
        LIB_STATUS_OK);
    expect_memory_pair(core, 0x00802070u, 0x36u, 0xc9u);
    expect_memory_pair(core, 0x00c2ce34u, 0x17u, 0xd4u);
    write_indexed(core, 0xecu, 0xedu, 0x2cu, 0xbcu);
    write_indexed(core, 0xecu, 0xedu, 0x2du, 0xc2u);
    write_indexed(core, 0xecu, 0xedu, 0x2fu, 0xbdu);
    write_indexed(core, 0xecu, 0xedu, 0x30u, 0xc2u);
    write_indexed(core, 0xecu, 0xedu, 0x31u, 1u);
    expect_port(core, 0xedu, 1u);
    expect_memory_pair(core, 0xf3fffu, 0x11u, 0x22u);
    lib_test_assert(x86_core_machine_memory_write(core, 0xf4000u, &write,
        sizeof(write)) == LIB_STATUS_OK);
    expect_memory_pair(core, 0xf3fffu, 0x11u, 0x22u);
    lib_test_assert(x86_core_machine_debug_patch_registers(core, &entry) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_run(core, (x86_core_machine_run_budget){1u, 0u},
        &result) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_debug_read_register(core, X86_CORE_CPU_DEBUG_EAX,
        &eax) == LIB_STATUS_OK && (eax & 0xffffu) == 0x1234u);

    /* Repoint the first window to the independent byte-wide attribute space. */
    write_indexed(core, 0xecu, 0xedu, 0x2du, 0x80u);
    expect_memory_pair(core, 0xf0000u, 0x61u, 0xffu);
    expect_memory_pair(core, 0xf0002u, 0x62u, 0xffu);
    expect_memory_pair(core, 0xf0004u, 0xffu, 0xffu);
    /* A disabled window reveals the immutable boot window again. */
    write_indexed(core, 0xecu, 0xedu, 0x2fu, 0x3du);
    expect_memory_pair(core, 0xf7ff0u, 0xa1u, 0x1au);
    lib_test_assert(x86_core_machine_reset(core) == LIB_STATUS_OK);
    expect_memory_pair(core, 0xf0000u, 0u, 0u);
}

static void test_management_memory(x86_core_machine *core)
{
    const lib_u8 ordinary = 0xa5u;
    x86_core_machine_run_result result = {0};
    lib_status status;
    lib_u8 value = 0u;

    lib_test_assert(x86_core_machine_reset(core) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_memory_write(core, 0x60210u,
        &ordinary, sizeof(ordinary)) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_run(core, (x86_core_machine_run_budget){32u, 0u},
        &result) == LIB_STATUS_OK && result.reason == X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT);
    expect_port(core, 0xecu, 0x18u);
    write_port(core, 0xedu, 0xa0u);
    lib_test_assert(x86_core_machine_set_smi(core, LIB_TRUE) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_run(core, (x86_core_machine_run_budget){1u, 0u},
        &result) == LIB_STATUS_OK && result.linear_pc == 0x00fffff0u);
    expect_port(core, 0xedu, 0xa8u);
    lib_test_assert(x86_core_machine_set_smi(core, LIB_FALSE) == LIB_STATUS_OK);
    status = x86_core_machine_run(core, (x86_core_machine_run_budget){64u, 0u}, &result);
    if (status != LIB_STATUS_OK || result.linear_pc != 0x000f800fu)
        lib_c_fprintf(lib_c_stderr, "SMM: status=%u stop=%u pc=%08x detail=%08x\n",
            (lib_u32)status, (lib_u32)result.reason, result.linear_pc, result.detail);
    lib_test_assert(status == LIB_STATUS_OK && result.linear_pc == 0x000f800fu);
    /* The SMI handler executes RES3 only if its private read returns 5Ah. */
    expect_port(core, 0xedu, 0xa0u);
    lib_test_assert(x86_core_machine_memory_read(core, 0x60210u,
        &value, sizeof(value)) == LIB_STATUS_OK && value == ordinary);
}

static x86_debug_response machine_debug(const emulator_machine_driver *driver,
    x86_debug_request request)
{
    x86_debug_response response;
    lib_size used = 0u;

    lib_test_assert(driver->execute_debug(driver->context, &request, sizeof(request), &response,
        sizeof(response), &used) == LIB_STATUS_OK && used == sizeof(response));
    return response;
}

static void test_machine_display(const emulator_machine_driver *driver)
{
    kvm_window_frame_storage frame = {0};
    kvm_console_text_frame console = {0};
    lib_bool produced = LIB_TRUE;

    lib_test_assert(driver->copy_frame(driver->context, &frame, &console, &produced) ==
        LIB_STATUS_OK && !produced);
    for (lib_size i = 0u; i < sizeof(planar_mode_writes) / sizeof(planar_mode_writes[0]); ++i)
        (void)machine_debug(driver, (x86_debug_request){.operation = X86_DEBUG_WRITE_PORT,
            .port = planar_mode_writes[i].port, .address = planar_mode_writes[i].value, .bytes = 1u});
    (void)machine_debug(driver, (x86_debug_request){.operation = X86_DEBUG_WRITE_LINEAR,
        .address = 0xa0000u, .bytes = 1u, .data = {0x80u}});
    /* Wait past the Machine's existing host display cadence. */
    base_sync_sleep_milliseconds(20u);
    lib_test_assert(driver->copy_frame(driver->context, &frame, &console, &produced) ==
        LIB_STATUS_OK && produced);
    lib_test_assert(frame.frame.graphics != LIB_NULL && frame.frame.text == LIB_NULL &&
        frame.frame.graphics->width == 640u && frame.frame.graphics->height == 480u);
    lib_test_assert(frame.frame.graphics->pixels[0] == 0xffffffu &&
        frame.frame.graphics->pixels[1] == 0u);
    kvm_window_frame_release(&frame);
}

static void stop_machine_on_debug_completion(void *owner)
{
    const emulator_machine_driver *driver = owner;

    if (driver->take_debug_stop(driver->context))
        driver->request_stop(driver->context);
}

static void test_machine_disk(emulator_machine_driver *driver, lib_u8 first_byte)
{
    static const struct {
        lib_u16 port;
        lib_u8 value;
    } read_chs[] = {
        {0x1f6u, 0xa0u}, {0x1f2u, 1u}, {0x1f3u, 1u},
        {0x1f4u, 0u}, {0x1f5u, 0u}, {0x1f7u, 0x20u}
    };
    x86_debug_response response;

    for (lib_size i = 0u; i < sizeof(read_chs) / sizeof(read_chs[0]); ++i)
        (void)machine_debug(driver, (x86_debug_request){.operation = X86_DEBUG_WRITE_PORT,
            .port = read_chs[i].port, .address = read_chs[i].value, .bytes = 1u});
    (void)machine_debug(driver, (x86_debug_request){.operation = X86_DEBUG_SET_EXECUTION_PLAN,
        .execution_kind = X86_DEBUG_EXECUTION_TRACE, .instruction_count = 1u});
    driver->set_executor_callback(driver->context, stop_machine_on_debug_completion, driver);
    lib_test_assert(driver->run(driver->context));
    driver->set_executor_callback(driver->context, LIB_NULL, LIB_NULL);
    response = machine_debug(driver, (x86_debug_request){.operation = X86_DEBUG_GET_EXECUTION_RESULT});
    lib_test_assert(response.enabled && response.value == 1u);
    response = machine_debug(driver, (x86_debug_request){.operation = X86_DEBUG_READ_PORT,
        .port = 0x3f6u, .bytes = 1u});
    lib_test_assert((response.value & 0x09u) == 0x08u);
    response = machine_debug(driver, (x86_debug_request){.operation = X86_DEBUG_READ_PORT,
        .port = 0x1f0u, .bytes = 1u});
    lib_test_assert((response.value & 0xffu) == first_byte);
}

static void test_machine(const lib_u8 *card)
{
    const char *path = "myob300-machine-smoke-card.bin";
    const char *disk_path = "myob300-machine-smoke-disk.img";
    const lib_u8 sector[512] = {0x5au, 0xa5u};
    const x86_nxvm_machine_config config = {
        .fixed_disk_image = {disk_path},
        .fixed_disk_mode = {LIB_STORAGE_MEDIUM_READONLY}
    };
    const x86_debug_request request = {
        .operation = X86_DEBUG_READ_LINEAR, .address = 0xe0000u, .bytes = 2u
    };
    x86_nxvm_machine_construction construction;
    x86_nxvm_machine *machine = LIB_NULL;
    x86_nxvm_machine_reset_vector reset;
    emulator_machine_driver driver;
    x86_debug_response response;
    lib_u8 cmos[X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES];
    lib_c_file *file = lib_c_fopen(path, "wb");

    lib_test_assert(file != LIB_NULL);
    lib_test_assert(lib_c_fwrite(card, 1u, 1u, file) == 1u);
    lib_test_assert(lib_c_fclose(file) == 0);
    lib_test_assert(myob300_machine_prepare(&config, path, &construction) ==
        LIB_STATUS_INVALID_ARGUMENT && construction.profile.context == LIB_NULL);
    file = lib_c_fopen(path, "wb");
    lib_test_assert(file != LIB_NULL);
    lib_test_assert(lib_c_fwrite(card, 1u, MYOB300_SYSTEM_CARD_BYTES, file) ==
        MYOB300_SYSTEM_CARD_BYTES);
    lib_test_assert(lib_c_fclose(file) == 0);
    lib_test_assert(myob300_machine_prepare(&config, path, &construction) == LIB_STATUS_OK);
    lib_test_assert(lib_c_remove(path) == 0);
    file = lib_c_fopen(disk_path, "wb");
    lib_test_assert(file != LIB_NULL);
    lib_test_assert(lib_c_fwrite(sector, 1u, sizeof(sector), file) == sizeof(sector));
    lib_test_assert(lib_c_fclose(file) == 0);
    lib_test_assert(x86_nxvm_machine_create(&config, &construction, &machine) == LIB_STATUS_OK);
    lib_test_assert(x86_nxvm_machine_get_reset_vector(machine, &reset) == LIB_STATUS_OK &&
        reset.cs == 0xf000u && reset.ip == 0xfff0u);
    lib_test_assert(x86_nxvm_machine_describe_emulator_driver(machine, &driver) == LIB_STATUS_OK);
    lib_test_assert(driver.reset(driver.context));
    driver.deliver_input(driver.context, &(kvm_input_event){.type = KVM_EVENT_MOUSE,
        .data.mouse = {.delta_x = -4, .delta_y = -2, .relative = LIB_TRUE}});
    response = machine_debug(&driver, (x86_debug_request){.operation = X86_DEBUG_READ_PORT,
        .port = 0x238u, .bytes = 1u});
    lib_test_assert(response.value == 4u);
    response = machine_debug(&driver, (x86_debug_request){.operation = X86_DEBUG_READ_PORT,
        .port = 0x23au, .bytes = 1u});
    lib_test_assert(response.value == 0xfeu);
    response = machine_debug(&driver, request);
    lib_test_assert(response.bytes == 2u && response.data[0] == 0x55u && response.data[1] == 0xaau);
    (void)machine_debug(&driver, (x86_debug_request){.operation = X86_DEBUG_WRITE_PORT,
        .port = 0xfbu, .bytes = 1u});
    (void)machine_debug(&driver, (x86_debug_request){.operation = X86_DEBUG_WRITE_PORT,
        .port = 0xecu, .address = 0x35u, .bytes = 1u});
    response = machine_debug(&driver, (x86_debug_request){.operation = X86_DEBUG_READ_PORT,
        .port = 0xedu, .bytes = 1u});
    lib_test_assert((response.value & 0x18u) == 0x18u);
    test_machine_disk(&driver, sector[0]);
    (void)machine_debug(&driver, (x86_debug_request){.operation = X86_DEBUG_WRITE_PORT,
        .port = 0x70u, .address = 0xf8u, .bytes = 1u});
    (void)machine_debug(&driver, (x86_debug_request){.operation = X86_DEBUG_WRITE_PORT,
        .port = 0x71u, .address = 0x5cu, .bytes = 1u});
    lib_test_assert(x86_nxvm_machine_capture_cmos(machine, cmos, sizeof(cmos)) == LIB_STATUS_OK &&
        cmos[0x78u] == 0x5cu && cmos[0x38u] == 0u);
    test_machine_display(&driver);
    x86_nxvm_machine_destroy(machine);
    lib_test_assert(lib_c_remove(disk_path) == 0);
}

int main(void)
{
    static lib_u8 image[MYOB300_SYSTEM_CARD_BYTES];
    static const lib_u8 attribute[] = {0x61u, 0x62u};
    static const lib_u8 reset_vector[] = {0xeau, 0x00u, 0x80u, 0x00u, 0xf0u};
    static const lib_u8 reset_program[] = {
        /* Select SMMCTL. A cold reset halts. SMI enters the data test. */
        0x31u, 0xc0u, 0xe6u, 0xfbu, 0xb0u, 0x18u, 0xe6u, 0xecu,
        0xe4u, 0xedu, 0xa8u, 0x08u, 0x75u, 0x03u, 0xf4u, 0xebu, 0xfdu,
        /* Set DS and ES to SMRAM, point DI at the save area, then write and
         * read a private byte. RES3 executes only on a match. */
        0xb8u, 0x00u, 0x60u, 0x8eu, 0xd8u, 0x8eu, 0xc0u, 0x31u, 0xffu,
        0xc6u, 0x06u, 0x10u, 0x02u, 0x5au, 0xa0u, 0x10u, 0x02u,
        0x3cu, 0x5au, 0x75u, 0x02u, 0x0fu, 0x07u, 0xf4u
    };
    x86_nxvm_board_plan *plan = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    x86_core_machine *core = LIB_NULL;
    x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_DEFAULT;
    myob300_board_setup setup = {
        .common = {image, sizeof(image)},
        .attribute = {attribute, sizeof(attribute)}
    };
    lib_size memory = 0u;

    lib_memory_copy(image + MYOB300_CARD_BOOT_BYTES - 0x8000u,
        reset_program, sizeof(reset_program));
    lib_memory_copy(image + MYOB300_CARD_BOOT_BYTES - 16u, reset_vector, sizeof(reset_vector));
    image[0x7f7ff0u] = 0xa1u;
    image[0x7e0000u] = 0x55u;
    image[0x7e0001u] = 0xaau;
    image[0x7e0002u] = 0x40u;
    image[0x7e8000u] = 0x55u;
    image[0x7e8002u] = 0x20u;
    image[0x7e8001u] = 0xaau;
    image[0x7f7fd0u] = 'O';
    image[0x7f7fd1u] = 'B';
    image[0x7f7ff1u] = 0x1au;
    image[0x2070u] = 0x36u;
    image[0x2071u] = 0xc9u;
    image[0x42ce34u] = 0x17u;
    image[0x42ce35u] = 0xd4u;
    image[0x802070u] = 0xa5u;
    image[0x802071u] = 0x5au;
    image[0x803fffu] = 0x11u;
    image[0x804000u] = 0x22u;
    image[0x800100u] = 0xb8u;
    image[0x800101u] = 0x34u;
    image[0x800102u] = 0x12u;
    lib_test_assert(x86_nxvm_board_plan_create(&myob300_board_configuration, &plan) == LIB_STATUS_OK);
    lib_test_assert(x86_nxvm_board_plan_set_topology(plan, &myob300_board_topology) == LIB_STATUS_OK);
    lib_test_assert(myob300_board_configure(&setup, plan) == LIB_STATUS_OK);
    lib_test_assert(x86_nxvm_board_create_from_plan(plan, &core, &board) == LIB_STATUS_OK);
    x86_nxvm_board_plan_destroy(plan);
    lib_test_assert(x86_core_machine_get_cpu_profile(core, &profile) == LIB_STATUS_OK &&
        profile == X86_CORE_CPU_PROFILE_AM386SXLV);
    lib_test_assert(x86_core_machine_get_memory_bytes(core, &memory) == LIB_STATUS_OK &&
        memory == 2u * 1024u * 1024u);
    lib_test_assert(x86_core_machine_bind_firmware_provider(core, &myob300_card_firmware,
        &setup.common) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_freeze_execution_providers(core) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_reset(core) == LIB_STATUS_OK);
    test_chipset(core, setup.profile);
    test_support_devices(core, board);
    test_refresh_and_ram(core);
    test_ab_shadow(core);
    test_display(core, setup.profile);
    test_input(core, board, setup.profile);
    test_card(core);
    test_management_memory(core);
    lib_test_assert(x86_core_machine_reset(core) == LIB_STATUS_OK);
    expect_port(core, 0xecu, 0u);
    expect_port(core, 0x378u, 0u);
    x86_core_machine_destroy(core);
    test_machine(image);
    return 0;
}
