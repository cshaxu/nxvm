#include "app-myob300/ob300_board_interface.h"
#include "lib/types/file.h"
#include "lib/types/test.h"
#include "x86-core/machine/debug_interface.h"
#include "x86-core/machine/memory_interface.h"

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

    write_port(core, 0xecu, 0x18u);
    expect_port(core, 0xecu, 0u);
    expect_port(core, 0xedu, 0x6au);
    write_port(core, 0xfbu, 0x39u);
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
}

static void test_refresh_and_ram(x86_core_machine *core)
{
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
    write_port(core, 0x64u, 0xaau);
    lib_test_assert(x86_core_machine_bus_read(core, 0x64u, &kbc_status) == LIB_STATUS_OK &&
        (kbc_status & 1u) == 0u);
    for (lib_u32 step = 0u; step < 1000u && (kbc_status & 1u) == 0u; ++step) {
        lib_test_assert(x86_core_machine_run(core, (x86_core_machine_run_budget){1u, 0u},
            &result) == LIB_STATUS_OK);
        lib_test_assert(x86_core_machine_bus_read(core, 0x64u, &kbc_status) == LIB_STATUS_OK);
    }
    lib_test_assert((kbc_status & 1u) != 0u);
    expect_port(core, 0x60u, 0x55u);
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

static void test_input(x86_core_machine *core, x86_nxvm_board_state *board)
{
    const lib_u8 keyboard_vector[] = {0x00u, 0x03u, 0u, 0u};
    const lib_u8 mouse_vector[] = {0x10u, 0x03u, 0u, 0u};
    const lib_u8 idle[] = {0xfbu, 0xf4u, 0xebu, 0xfdu};
    const lib_u8 keyboard_irq[] = {
        0xe4u, 0x60u, 0xa2u, 0x00u, 0x05u, 0xb0u, 0x20u, 0xe6u, 0x20u, 0xcfu
    };
    const lib_u8 mouse_irq[] = {
        0xe4u, 0x60u, 0xa2u, 0x01u, 0x05u, 0xb0u, 0x20u,
        0xe6u, 0xa0u, 0xe6u, 0x20u, 0xcfu
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
    lib_test_assert(x86_nxvm_board_keyboard_receive_native_byte(board, 0x1cu) == LIB_STATUS_OK);
    expect_input(core, 0x500u, 0x1cu);
    write_port(core, 0x64u, 0xd4u);
    write_port(core, 0x60u, 0xf4u);
    expect_input(core, 0x501u, 0xfau);
    lib_test_assert(x86_nxvm_board_mouse_receive_relative(board, 5, 3, 1u) == LIB_STATUS_OK);
    expect_input(core, 0x501u, 0x09u);
    expect_input(core, 0x501u, 0x05u);
    expect_input(core, 0x501u, 0x03u);
}

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

    write_port(core, 0x3d6u, 0u);
    expect_port(core, 0x3d7u, 0x90u);
    lib_test_assert(myob300_board_capture_display(board, &snapshot) == LIB_STATUS_UNSUPPORTED);

    /* Enable 640x480 planar scanout and write a pixel via the guest aperture. */
    write_indexed(core, 0x3d4u, 0x3d5u, 1u, 79u);
    write_indexed(core, 0x3d4u, 0x3d5u, 7u, 2u);
    write_indexed(core, 0x3d4u, 0x3d5u, 18u, 0xdfu);
    write_indexed(core, 0x3d4u, 0x3d5u, 19u, 40u);
    write_indexed(core, 0x3d4u, 0x3d5u, 0x17u, 0x80u);
    write_indexed(core, 0x3c4u, 0x3c5u, 2u, 1u);
    write_indexed(core, 0x3c4u, 0x3c5u, 4u, 4u);
    write_indexed(core, 0x3ceu, 0x3cfu, 6u, 5u);
    write_indexed(core, 0x3ceu, 0x3cfu, 8u, 0xffu);
    write_indexed(core, 0x3c0u, 0x3c0u, 0x12u, 15u);
    write_indexed(core, 0x3c0u, 0x3c0u, 1u, 1u);
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
    lib_test_assert(x86_core_machine_memory_write(core, 0xe0000u, &write, 1u) == LIB_STATUS_OK);
    expect_memory_pair(core, 0xe0000u, 0x55u, 0xaau);
    write_port(core, 0xfbu, 0u);
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
    write_indexed(core, 0xecu, 0xedu, 0x2fu, 0xf7u);
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

int main(void)
{
    static lib_u8 image[MYOB300_CARD_BOOT_BYTES + 0x8000u];
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
    image[0x7e8000u] = 0x55u;
    image[0x7e8001u] = 0xaau;
    image[0x7f7ff1u] = 0x1au;
    image[0x803fffu] = 0x11u;
    image[0x804000u] = 0x22u;
    image[0x800100u] = 0xb8u;
    image[0x800101u] = 0x34u;
    image[0x800102u] = 0x12u;
    lib_test_assert(x86_nxvm_board_plan_create(&myob300_board_configuration, &plan) == LIB_STATUS_OK);
    lib_test_assert(myob300_board_configure(plan, &setup) == LIB_STATUS_OK);
    lib_test_assert(x86_nxvm_board_create_from_plan(plan, &core, &board) == LIB_STATUS_OK);
    x86_nxvm_board_plan_destroy(plan);
    lib_test_assert(x86_core_machine_get_cpu_profile(core, &profile) == LIB_STATUS_OK &&
        profile == X86_CORE_CPU_PROFILE_AM386SXLV);
    lib_test_assert(x86_core_machine_get_memory_bytes(core, &memory) == LIB_STATUS_OK &&
        memory == 2u * 1024u * 1024u);
    lib_test_assert(x86_core_machine_freeze_execution_providers(core) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_reset(core) == LIB_STATUS_OK);
    test_chipset(core, setup.profile);
    test_support_devices(core, board);
    test_refresh_and_ram(core);
    test_ab_shadow(core);
    test_display(core, setup.profile);
    test_input(core, board);
    test_card(core);
    test_management_memory(core);
    lib_test_assert(x86_core_machine_reset(core) == LIB_STATUS_OK);
    expect_port(core, 0xecu, 0u);
    x86_core_machine_destroy(core);
    return 0;
}
