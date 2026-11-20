#include "app-myob300/ob300_board_interface.h"
#include "lib/types/file.h"
#include "lib/types/test.h"
#include "x86-core/machine/memory_interface.h"

static void write_port(x86_core_machine *core, lib_u16 port, lib_u8 value)
{
    lib_test_assert(x86_core_machine_bus_write(core, port, value) == LIB_STATUS_OK);
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

static void test_chipset(x86_core_machine *core)
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
    lib_test_assert(x86_core_machine_bus_read(core, 0x3d6u, &value) == LIB_STATUS_UNSUPPORTED);
}

static void test_support_devices(x86_core_machine *core, x86_nxvm_board_state *board)
{
    const struct {
        lib_u16 port;
        lib_u8 value;
    } pic_initialization[] = {
        {0x20u, 0x11u}, {0x21u, 0x08u}, {0x21u, 0x04u}, {0x21u, 0x01u},
        {0xa0u, 0x11u}, {0xa1u, 0x70u}, {0xa1u, 0x02u}, {0xa1u, 0x01u}
    };
    lib_bool enabled = LIB_FALSE;

    for (lib_size index = 0u; index < sizeof(pic_initialization) / sizeof(pic_initialization[0]);
         ++index)
        write_port(core, pic_initialization[index].port, pic_initialization[index].value);
    write_port(core, 0x21u, 0xf1u);
    write_port(core, 0xa1u, 0x1fu);
    expect_port(core, 0x21u, 0xf1u);
    expect_port(core, 0xa1u, 0x1fu);

    write_port(core, 0x43u, 0x30u);
    write_port(core, 0x40u, 0x56u);
    write_port(core, 0x40u, 0x34u);
    write_port(core, 0x43u, 0xecu);
    expect_port(core, 0x40u, 0x70u);

    write_port(core, 0x81u, 0x17u);
    expect_port(core, 0x81u, 0x17u);
    write_port(core, 0x70u, 0x20u);
    write_port(core, 0x71u, 0x65u);
    expect_port(core, 0x71u, 0x65u);

    lib_test_assert(x86_nxvm_board_observe_a20(board, &enabled) == LIB_STATUS_OK && !enabled);
    write_port(core, 0x92u, 2u);
    lib_test_assert(x86_nxvm_board_observe_a20(board, &enabled) == LIB_STATUS_OK && enabled);
    write_port(core, 0x92u, 0u);
    lib_test_assert(x86_nxvm_board_observe_a20(board, &enabled) == LIB_STATUS_OK && !enabled);
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
    static const lib_u8 image[MYOB300_SYSTEM_ROM_BYTES] = {
        /* Select SMMCTL. A cold reset halts. SMI enters the data test. */
        0x31u, 0xc0u, 0xe6u, 0xfbu, 0xb0u, 0x18u, 0xe6u, 0xecu,
        0xe4u, 0xedu, 0xa8u, 0x08u, 0x75u, 0x03u, 0xf4u, 0xebu, 0xfdu,
        /* Set DS and ES to SMRAM, point DI at the save area, then write and
         * read a private byte. RES3 executes only on a match. */
        0xb8u, 0x00u, 0x60u, 0x8eu, 0xd8u, 0x8eu, 0xc0u, 0x31u, 0xffu,
        0xc6u, 0x06u, 0x10u, 0x02u, 0x5au, 0xa0u, 0x10u, 0x02u,
        0x3cu, 0x5au, 0x75u, 0x02u, 0x0fu, 0x07u, 0xf4u,
        [MYOB300_SYSTEM_ROM_BYTES - 16u] = 0xeau, 0x00u, 0x80u, 0x00u, 0xf0u
    };
    x86_nxvm_board_plan *plan = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    x86_core_machine *core = LIB_NULL;
    x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_DEFAULT;
    lib_size memory = 0u;

    lib_test_assert(x86_nxvm_board_plan_create(&myob300_board_configuration, &plan) == LIB_STATUS_OK);
    lib_test_assert(myob300_board_configure(plan, image, sizeof(image)) == LIB_STATUS_OK);
    lib_test_assert(x86_nxvm_board_create_from_plan(plan, &core, &board) == LIB_STATUS_OK);
    x86_nxvm_board_plan_destroy(plan);
    lib_test_assert(x86_core_machine_get_cpu_profile(core, &profile) == LIB_STATUS_OK &&
        profile == X86_CORE_CPU_PROFILE_AM386SXLV);
    lib_test_assert(x86_core_machine_get_memory_bytes(core, &memory) == LIB_STATUS_OK &&
        memory == 2u * 1024u * 1024u);
    lib_test_assert(x86_core_machine_freeze_execution_providers(core) == LIB_STATUS_OK);
    lib_test_assert(x86_core_machine_reset(core) == LIB_STATUS_OK);
    test_chipset(core);
    test_support_devices(core, board);
    test_management_memory(core);
    lib_test_assert(x86_core_machine_reset(core) == LIB_STATUS_OK);
    expect_port(core, 0xecu, 0u);
    x86_core_machine_destroy(core);
    return 0;
}
