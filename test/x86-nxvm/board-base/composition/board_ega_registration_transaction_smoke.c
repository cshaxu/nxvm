#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86-core/machine/machine_interface.h"
#include "../video_registration_fixture.h"
#include "bus_fixture.h"

static lib_status sentinel_read(void *owner, lib_u16 address, lib_u64 tick,
    lib_u32 *out_value)
{
    (void)owner;
    (void)address;
    (void)tick;
    *out_value = 0x5au;
    return LIB_STATUS_OK;
}

static lib_status sentinel_memory_read(void *owner, lib_u32 physical,
    lib_uptr destination, lib_uptr bytes, lib_bool observe_only)
{
    (void)owner;
    (void)observe_only;
    if (physical != 0x1000u || bytes != 1u) return LIB_STATUS_UNSUPPORTED;
    *(lib_u8 *)destination = 0x5au;
    return LIB_STATUS_OK;
}

static lib_status sentinel_memory_query(void *owner, lib_u32 physical,
    lib_uptr bytes, x86_core_machine_memory_access access)
{
    (void)owner;
    return physical == 0x1000u && bytes == 1u &&
        access == X86_CORE_MACHINE_MEMORY_ACCESS_READ ?
        LIB_STATUS_OK : LIB_STATUS_UNSUPPORTED;
}

static lib_status sentinel_memory_write(void *owner, lib_u32 physical,
    lib_uptr source, lib_uptr bytes)
{
    (void)owner;
    (void)physical;
    (void)source;
    (void)bytes;
    return LIB_STATUS_UNSUPPORTED;
}

static void sentinel_observer(void *owner, lib_u32 physical, lib_uptr bytes)
{
    (void)physical;
    (void)bytes;
    ++*(lib_u32 *)owner;
}

static lib_i32 create_machine(x86_core_machine **out_machine, lib_u32 *writes)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 16u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_8086
    };
    const x86_core_machine_port_route port = {
        .address = 0x80u, .read = sentinel_read, .owner = writes
    };
    const x86_core_machine_memory_device_route memory = {
        0x1000u, 1u, {sentinel_memory_read, sentinel_memory_write, sentinel_memory_query},
        X86_CORE_MACHINE_MEMORY_PROVIDER_STANDARD, LIB_NULL
    };

    return x86_core_machine_neutral_create(&config, out_machine) != LIB_STATUS_OK ||
        x86_core_machine_install_port_routes(*out_machine, &port, 1u) != LIB_STATUS_OK ||
        x86_core_machine_install_memory_device_routes(*out_machine, &memory, 1u,
            sentinel_observer, LIB_NULL, writes) != LIB_STATUS_OK;
}

static lib_i32 expect_routes(x86_core_machine *machine, lib_bool cga,
    lib_bool configured, lib_u32 variant)
{
    const struct {lib_u16 address; lib_bool write; lib_bool present;} routes[] = {
        {0x80u, LIB_FALSE, LIB_TRUE},
        {0x3d4u, LIB_TRUE, cga},
        {0x3c4u, LIB_TRUE, configured},
        {0x3c9u, LIB_TRUE, configured && variant == 1u},
        {0x7c6u, LIB_FALSE, configured && variant == 2u}
    };
    lib_i32 failed = 0;

    for (lib_size index = 0u; index < sizeof(routes) / sizeof(routes[0]); ++index)
        failed |= test_x86_nxvm_machine_fixture_expect_port(machine, routes[index].address,
            routes[index].write, routes[index].present);
    return failed;
}

static lib_i32 retained_sentinel(x86_core_machine *machine, lib_u32 *writes)
{
    lib_u32 port = 0u;
    lib_u8 memory = 0u;
    const lib_u8 value = 0xa5u;

    const lib_i32 failed = x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(machine, 0x80u, &port) != LIB_STATUS_OK || port != 0x5au ||
        x86_core_machine_memory_read(machine, 0x1000u, &memory, 1u) != LIB_STATUS_OK ||
        memory != 0x5au ||
        x86_core_machine_memory_write(machine, 0x2000u, &value, 1u) != LIB_STATUS_OK ||
        *writes != 1u;

    if (failed) lib_c_printf("Video sentinel failed: port=%u memory=%u writes=%u\n",
        port, memory, *writes);
    return failed;
}

lib_i32 test_video_registration_routes(t_vadp *adapter)
{
    const x86_nxvm_board_display_config config = {
        .text_timing = {3u, 2u, 1u},
        .ega_present = LIB_TRUE,
        .ega_personality = X86_VIDEO_EGA_PERSONALITY_GENERIC,
        .ega_sequencer = {X86_NXVM_BOARD_VADP_EGA_APERTURE_BASE,
            X86_NXVM_BOARD_VADP_EGA_APERTURE_BYTES,
            0x03u, 0x00u, 0x0fu, 0x02u, LIB_TRUE}
    };
    const x86_video_cecg_config compaq = {
        0x41u, 0x00u, 0x30u, 0x01u, LIB_TRUE, LIB_FALSE, LIB_TRUE,
        0x06u, 0x01u, LIB_FALSE, LIB_FALSE, LIB_FALSE
    };
    static const lib_size port_counts[] = {15u, 27u, 22u};
    x86_core_machine *machine = LIB_NULL;
    lib_u32 writes = 0u;
    lib_i32 failed = create_machine(&machine, &writes);

    if (!failed) {
        failed |= test_video_registration_initial_failure(adapter, machine) ||
            expect_routes(machine, LIB_FALSE, LIB_FALSE, 0u);
        failed |= test_video_registration_initialize(adapter, machine, LIB_STATUS_OK) ||
            expect_routes(machine, LIB_TRUE, LIB_FALSE, 0u);
        test_video_registration_finalize(adapter);
        failed |= expect_routes(machine, LIB_FALSE, LIB_FALSE, 0u) ||
            retained_sentinel(machine, &writes);
    }
    if (failed) lib_c_printf("Video initial registration failed.\n");
    x86_core_machine_destroy(machine);

    for (lib_u32 variant = 0u; variant < 3u; ++variant) {
        x86_nxvm_board_display_config selected = config;

        selected.vga_present = variant == 1u;
        if (variant == 2u) {
            selected.ega_personality = X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR;
            selected.cecg = compaq;
        }
        for (lib_u32 dependency = 0u; dependency < 2u; ++dependency) {
            lib_i32 case_failed = 0;
            machine = LIB_NULL;
            writes = 0u;
            if (create_machine(&machine, &writes)) {
                x86_core_machine_destroy(machine);
                return 1;
            }
            case_failed |= test_video_registration_initialize(adapter, machine, LIB_STATUS_OK);
            case_failed |= test_video_registration_dependency_failure(adapter, &selected,
                dependency == 0u, port_counts[variant]) ||
                !test_video_registration_unconfigured(adapter) ||
                expect_routes(machine, LIB_TRUE, LIB_FALSE, variant);
            case_failed |= x86_nxvm_board_vadp_configure(adapter, &selected) != LIB_STATUS_OK ||
                test_video_registration_unconfigured(adapter) ||
                expect_routes(machine, LIB_TRUE, LIB_TRUE, variant);
            test_video_registration_finalize(adapter);
            case_failed |= expect_routes(machine, LIB_FALSE, LIB_FALSE, variant) ||
                retained_sentinel(machine, &writes);
            if (case_failed) lib_c_printf("Video variant %u dependency %u failed.\n",
                variant, dependency);
            failed |= case_failed;
            x86_core_machine_destroy(machine);
        }
    }

    /* The actual last Compaq endpoint collides; its preceding endpoints and
     * memory publication must roll back without replacing the CGA chip. */
    {
        x86_nxvm_board_display_config selected = config;
        lib_u8 collision_owner = 0u;
        const x86_core_machine_port_route collision = {
            .address = 0xfc6u, .read = sentinel_read, .owner = &collision_owner
        };

        selected.ega_personality = X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR;
        selected.cecg = compaq;
        machine = LIB_NULL;
        writes = 0u;
        if (create_machine(&machine, &writes)) {
            x86_core_machine_destroy(machine);
            return 1;
        }
        failed |= test_video_registration_initialize(adapter, machine, LIB_STATUS_OK) ||
            x86_core_machine_install_port_routes(machine, &collision, 1u) != LIB_STATUS_OK;
        failed |= test_video_registration_configure_rollback(adapter, &selected,
            LIB_STATUS_INVALID_STATE) || !test_video_registration_unconfigured(adapter) ||
            expect_routes(machine, LIB_TRUE, LIB_FALSE, 2u);
        failed |= x86_core_machine_remove_port_routes(machine, &collision_owner) != LIB_STATUS_OK ||
            x86_nxvm_board_vadp_configure(adapter, &selected) != LIB_STATUS_OK ||
            expect_routes(machine, LIB_TRUE, LIB_TRUE, 2u);
        test_video_registration_finalize(adapter);
        failed |= expect_routes(machine, LIB_FALSE, LIB_FALSE, 2u) ||
            retained_sentinel(machine, &writes);
        x86_core_machine_destroy(machine);
    }

    if (failed) return 1;
    lib_c_printf("VIDEO-REGISTRATION-ROLLBACK:OK\n");
    return 0;
}
