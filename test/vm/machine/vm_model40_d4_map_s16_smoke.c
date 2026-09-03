#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/session_private.h"
#include "../support/rom/model40_session_assets.h"

static C_INT read_byte(core_machine *machine, type_unsigned_32 physical,
    type_unsigned_8 expected)
{
    type_unsigned_8 observed = 0u;

    return core_machine_memory_read(machine, physical, &observed,
        sizeof(observed)) == TYPE_STATUS_OK && observed == expected;
}

static C_INT write_byte(core_machine *machine, type_unsigned_32 physical,
    type_unsigned_8 value, type_status expected)
{
    return core_machine_memory_write(machine, physical, &value, sizeof(value)) == expected;
}

static C_INT write_word(core_machine *machine, type_unsigned_32 physical,
    type_unsigned_16 value)
{
    return core_machine_memory_write(machine, physical, &value, sizeof(value)) ==
        TYPE_STATUS_OK;
}

static C_INT read_word(core_machine *machine, type_unsigned_32 physical,
    type_unsigned_16 expected)
{
    type_unsigned_16 observed = 0u;

    return core_machine_memory_read(machine, physical, &observed,
        sizeof(observed)) == TYPE_STATUS_OK && observed == expected;
}

C_INT main(C_VOID)
{
    static type_unsigned_8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static type_unsigned_8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    vm_session *session = STD_NULL;
    core_machine_d4_platform_observation d4;
    type_unsigned_32 port_value = 0u;
    C_INT failed = 0;

    even[0u] = 0x11u;
    odd[0u] = 0x22u;
#define CHECK(expression) do { ++step; if (!(expression)) { \
    failed = step; STD_PRINTF("D4-MAP failed step=%d line=%d\n", step, __LINE__); \
} } while (0)
    C_INT step = 0;

    CHECK(vm_model40_fixture_create_bytes(even, odd, &session) == TYPE_STATUS_OK &&
        session != STD_NULL);
    if (!failed) {
        CHECK(read_byte(session->core_machine,
            VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START, 0x11u));
        CHECK(read_byte(session->core_machine,
            VM_PROFILE_MODEL40_ROM_LOW_PHYSICAL_START + 1u, 0x22u));
        CHECK(read_byte(session->core_machine,
            VM_PROFILE_MODEL40_ROM_COMPATIBILITY_ALIAS_START, 0x11u));
        CHECK(read_byte(session->core_machine,
            VM_PROFILE_MODEL40_ROM_COMPATIBILITY_ALIAS_START + 1u, 0x22u));
        CHECK(read_byte(session->core_machine,
            VM_PROFILE_MODEL40_ROM_HIGH_ALIAS_START, 0x11u));
        CHECK(read_byte(session->core_machine,
            VM_PROFILE_MODEL40_ROM_HIGH_ALIAS_START + 1u, 0x22u));
        CHECK(read_byte(session->core_machine,
            VM_PROFILE_MODEL40_ROM_HIGH_RESET_ALIAS_START, 0x11u));
        CHECK(read_byte(session->core_machine,
            VM_PROFILE_MODEL40_ROM_HIGH_RESET_ALIAS_START + 1u, 0x22u));
        CHECK(read_byte(session->core_machine, 0x00f40000u, 0xffu));
        CHECK(read_byte(session->core_machine, 0x00f80000u, 0xffu));
        /* The DeskPro ROM POST tests its relocated C0000h--EFFFFh RAM at
         * FC0000h--FEFFFFh. These are Core's one backing through the selected
         * board decode; the low C0000h--EFFFFh hole remains ROM/open bus. */
        CHECK(write_byte(session->core_machine, 0x00fc1234u, 0x3cu,
            TYPE_STATUS_OK));
        CHECK(read_byte(session->core_machine, 0x00fc1234u, 0x3cu));
        CHECK(write_byte(session->core_machine, 0x00fd5678u, 0xa5u,
            TYPE_STATUS_OK));
        CHECK(read_byte(session->core_machine, 0x00fd5678u, 0xa5u));
        CHECK(write_byte(session->core_machine, 0x00fe9abcu, 0x5au,
            TYPE_STATUS_OK));
        CHECK(read_byte(session->core_machine, 0x00fe9abcu, 0x5au));
        /* FF0000h is also selected RAM, despite the ordinary F0000h BIOS ROM
         * route underneath it.  The alias must win for this board decode. */
        CHECK(write_byte(session->core_machine, 0x00ff1234u, 0x73u,
            TYPE_STATUS_OK));
        CHECK(read_byte(session->core_machine, 0x00ff1234u, 0x73u));
        CHECK(core_machine_get_d4_platform_observation(session->core_machine,
            &d4) == TYPE_STATUS_OK && !d4.iochk_latched && !d4.failsafe_latched);
        CHECK(write_byte(session->core_machine, 0x00fa0000u, 0x3cu,
            TYPE_STATUS_OK));
        CHECK(read_byte(session->core_machine, 0x00fa0000u, 0x3cu));
        /* POST also probes the relocated B0000h page through FB0000h. */
        CHECK(write_byte(session->core_machine, 0x00fb1234u, 0x96u,
            TYPE_STATUS_OK));
        /* B0000h is an unpopulated low-memory window while B8000h is the
         * selected EGA aperture.  It must not silently fall through into the
         * same Core backing as the relocated FB0000h RAM page. */
        CHECK(write_byte(session->core_machine, 0x000b1234u, 0x69u,
            TYPE_STATUS_OK));
        CHECK(read_byte(session->core_machine, 0x000b1234u, 0xffu));
        CHECK(read_byte(session->core_machine, 0x00fb1234u, 0x96u));
        /* The 64 KiB board-bank seam is one contiguous RAM decode. */
        CHECK(write_word(session->core_machine, 0x00fbffffu, 0xffffu));
        CHECK(read_word(session->core_machine, 0x00fbffffu, 0xffffu));
        CHECK(write_byte(session->core_machine, 0x000b5678u, 0x69u,
            TYPE_STATUS_OK));
        CHECK(read_byte(session->core_machine, 0x000b5678u, 0xffu));
        CHECK(write_byte(session->core_machine,
            VM_PROFILE_MODEL40_ROM_COMPATIBILITY_ALIAS_START, 0x5au,
            TYPE_STATUS_OK));
        CHECK(read_byte(session->core_machine,
            VM_PROFILE_MODEL40_ROM_COMPATIBILITY_ALIAS_START, 0x11u));
        CHECK(write_byte(session->core_machine, 0x00000020u, 0xa5u,
            TYPE_STATUS_OK));
        CHECK(read_byte(session->core_machine, 0x00100020u, 0xa5u));
        CHECK(core_machine_set_a20(session->core_machine, TYPE_TRUE) == TYPE_STATUS_OK);
        CHECK(write_byte(session->core_machine, 0x00100020u, 0x5au,
            TYPE_STATUS_OK));
        CHECK(read_byte(session->core_machine, 0x00100020u, 0x5au));
        CHECK(core_machine_set_a20(session->core_machine, TYPE_FALSE) == TYPE_STATUS_OK);

        /* AT spare DMA page latches are board-visible state, not unknown
         * ports.  DeskPro firmware uses 86h during POST. */
        CHECK(core_machine_bus_write(session->core_machine, 0x0086u, 0x5au) == TYPE_STATUS_OK);
        CHECK(core_machine_bus_read(session->core_machine, 0x0086u, &port_value) == TYPE_STATUS_OK &&
            port_value == 0x5au);
        CHECK(core_machine_bus_write(session->core_machine, 0x0080u, 0xa5u) == TYPE_STATUS_OK);
        CHECK(core_machine_bus_read(session->core_machine, 0x0080u, &port_value) == TYPE_STATUS_OK &&
            port_value == 0xa5u);

        CHECK(read_byte(session->core_machine, VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL,
            0x8fu));
        CHECK(read_byte(session->core_machine, VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL + 1u,
            0xfdu));
        CHECK(read_byte(session->core_machine, VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL + 2u,
            0x42u));
        CHECK(read_byte(session->core_machine, VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL + 3u,
            0xfcu));
        CHECK(read_byte(session->core_machine, VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL + 4u,
            0xffu));
        CHECK(write_byte(session->core_machine, VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL + 4u,
            0x5au, TYPE_STATUS_OK));
        CHECK(read_byte(session->core_machine, VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL + 4u,
            0xffu));
        CHECK(write_byte(session->core_machine, VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL + 2u,
            0x4au, TYPE_STATUS_OK));
        CHECK(read_byte(session->core_machine, VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL + 2u,
            0x4au));
        CHECK(core_machine_set_a20(session->core_machine, TYPE_TRUE) == TYPE_STATUS_OK);
        CHECK(write_byte(session->core_machine, 0x00100020u, 0x96u,
            TYPE_STATUS_OK));
        CHECK(write_byte(session->core_machine, VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL + 2u,
            0x41u, TYPE_STATUS_OK));
        CHECK(read_byte(session->core_machine, 0x00100020u, 0xffu));
        CHECK(write_byte(session->core_machine, 0x00100020u, 0x69u,
            TYPE_STATUS_OK));
        CHECK(write_byte(session->core_machine, VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL + 2u,
            0x42u, TYPE_STATUS_OK));
        CHECK(read_byte(session->core_machine, 0x00100020u, 0x96u));
        CHECK(write_byte(session->core_machine, 0x000f0000u, 0xa5u,
            TYPE_STATUS_OK));
        CHECK(read_byte(session->core_machine, 0x000f0000u, 0x11u));
        CHECK(core_machine_reset(session->core_machine) == TYPE_STATUS_OK);
        CHECK(core_machine_bus_read(session->core_machine, 0x0086u, &port_value) == TYPE_STATUS_OK &&
            port_value == 0u);
        CHECK(read_byte(session->core_machine, 0x000f0000u, 0x11u));
        CHECK(read_byte(session->core_machine, VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL,
            0x8fu));
        CHECK(read_byte(session->core_machine, VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL + 1u,
            0xfdu));
        CHECK(read_byte(session->core_machine, VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL + 2u,
            0x42u));
        CHECK(read_byte(session->core_machine, VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL + 3u,
            0xfcu));
        CHECK(read_byte(session->core_machine, VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL + 4u,
            0xffu));
        CHECK(write_byte(session->core_machine, VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL + 4u,
            0x5au, TYPE_STATUS_OK));
        CHECK(read_byte(session->core_machine, VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL + 4u,
            0xffu));
        CHECK(write_byte(session->core_machine, VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL + 2u,
            0x4au, TYPE_STATUS_OK));
        CHECK(read_byte(session->core_machine, VM_PROFILE_MODEL40_D4_CONTROL_PHYSICAL + 2u,
            0x4au));
        CHECK(core_machine_set_a20(session->core_machine, 1) == TYPE_STATUS_OK);
    }
#undef CHECK
    if (!failed) STD_PRINTF("M5:T386:S16:D4-ROM-MAP:OK\n");
    if (!failed) STD_PRINTF("M5:T386:S16:D4-SOLE-ROM-OWNER:OK\n");
    if (!failed) STD_PRINTF("M5:T386:S16:D4-RESET-ALIAS:OK\n");
    if (!failed) STD_PRINTF("M5:T390:S29:MODEL40-ROM-DECODE:OK\n");
    vm_session_destroy(session);
    return failed ? 1 : 0;
}
