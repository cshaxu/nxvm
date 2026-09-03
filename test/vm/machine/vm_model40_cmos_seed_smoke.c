#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/port.h"
#include "core/machine/rtc.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_private.h"
#include "../support/rom/model40_session_assets.h"

static type_unsigned_8 vm_model40_cmos_read(vm_session *session,
    type_unsigned_8 index)
{
    t_port *port = &session->core_machine->executor_port;

    core_machine_port_write(port, 0x0070u, index);
    return (type_unsigned_8)core_machine_port_read(port, 0x0071u);
}

static C_VOID vm_model40_cmos_write(vm_session *session, type_unsigned_8 index,
    type_unsigned_8 value)
{
    t_port *port = &session->core_machine->executor_port;

    core_machine_port_write(port, 0x0070u, index);
    core_machine_port_write(port, 0x0071u, value);
}

static C_INT vm_model40_cmos_checksum_is_valid(vm_session *session)
{
    type_unsigned_16 checksum = 0u;
    type_unsigned_8 index;

    for (index = 0x10u; index < 0x2eu; ++index) {
        checksum = (type_unsigned_16)(checksum + vm_model40_cmos_read(session, index));
    }
    return vm_model40_cmos_read(session, 0x2eu) ==
        TYPE_MASK_UNSIGNED_8(checksum >> 8u) &&
        vm_model40_cmos_read(session, 0x2fu) == TYPE_MASK_UNSIGNED_8(checksum);
}

static C_INT vm_model40_cmos_seed_matches(vm_session *session)
{
    return vm_model40_cmos_read(session, CORE_MACHINE_RTC_TYPE_DISK_FLOPPY) == 0x22u &&
        vm_model40_cmos_read(session, CORE_MACHINE_RTC_TYPE_DISK_FIXED) == 0x80u &&
        vm_model40_cmos_read(session, CORE_MACHINE_RTC_TYPE_DISK_FIXED_EXTENDED_0) == 0u &&
        vm_model40_cmos_read(session, CORE_MACHINE_RTC_EQUIPMENT) == 0x41u &&
        vm_model40_cmos_read(session, CORE_MACHINE_RTC_BASEMEM_LSB) == 0x80u &&
        vm_model40_cmos_read(session, CORE_MACHINE_RTC_BASEMEM_MSB) == 0x02u &&
        vm_model40_cmos_read(session, CORE_MACHINE_RTC_EXTMEM_LSB) == 0x00u &&
        vm_model40_cmos_read(session, CORE_MACHINE_RTC_EXTMEM_MSB) == 0x04u &&
        vm_model40_cmos_checksum_is_valid(session);
}

static C_VOID vm_model40_cmos_seed_bytes(
    type_unsigned_8 bytes[VM_SESSION_CMOS_SEED_BYTES])
{
    STD_MEMSET(bytes, 0, VM_SESSION_CMOS_SEED_BYTES);
    bytes[CORE_MACHINE_RTC_TYPE_DISK_FLOPPY] = 0x22u;
    bytes[CORE_MACHINE_RTC_TYPE_DISK_FIXED] = 0x80u;
    /* 33h is Model-40 board NVRAM, not a topology-derived CMOS field. */
    bytes[0x33u] = 0x5au;
    bytes[CORE_MACHINE_RTC_BASEMEM_LSB] = 0x80u;
    bytes[CORE_MACHINE_RTC_BASEMEM_MSB] = 0x02u;
    bytes[CORE_MACHINE_RTC_EXTMEM_LSB] = 0x00u;
    bytes[CORE_MACHINE_RTC_EXTMEM_MSB] = 0x04u;
    {
        type_unsigned_16 checksum = 0u;
        type_unsigned_8 index;

        for (index = 0x10u; index < 0x2eu; ++index) {
            checksum = (type_unsigned_16)(checksum + bytes[index]);
        }
        bytes[0x2eu] = TYPE_MASK_UNSIGNED_8(checksum >> 8u);
        bytes[0x2fu] = TYPE_MASK_UNSIGNED_8(checksum);
    }
}

static type_status vm_model40_cmos_seed_session_create(
    const type_unsigned_8 seed[VM_SESSION_CMOS_SEED_BYTES], vm_session **out_session)
{
    type_unsigned_8 even_bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES] = {0};
    type_unsigned_8 odd_bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    type_unsigned_8 default_seed[VM_SESSION_CMOS_SEED_BYTES];
    vm_session_config config = {0};
    vm_session_assets assets = {0};

    STD_MEMSET(odd_bytes, 1, sizeof(odd_bytes));
    config.profile_kind = VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40;
    config.bios_count = 2u;
    assets.bios[0u] = (vm_session_asset_bytes) { even_bytes, sizeof(even_bytes) };
    assets.bios[1u] = (vm_session_asset_bytes) { odd_bytes, sizeof(odd_bytes) };
    vm_model40_fixture_cmos_seed(default_seed);
    assets.cmos_seed = (vm_session_asset_bytes) { seed == STD_NULL ? default_seed : seed,
        VM_SESSION_CMOS_SEED_BYTES };
    return vm_session_create_from_assets(&config, &assets, out_session);
}

C_INT main(C_VOID)
{
    vm_session *first = STD_NULL;
    vm_session *second = STD_NULL;
    vm_session *seeded = STD_NULL;
    type_unsigned_8 seed[VM_SESSION_CMOS_SEED_BYTES];
    C_INT failed = 0;

    failed |= vm_model40_cmos_seed_session_create(STD_NULL, &first) != TYPE_STATUS_OK ||
        first == STD_NULL;
    if (!failed) failed |= !vm_model40_cmos_seed_matches(first);
    if (!failed) {
        vm_model40_cmos_write(first, CORE_MACHINE_RTC_EQUIPMENT, 0x5au);
        failed |= vm_session_reset(first) != TYPE_STATUS_OK ||
            vm_model40_cmos_read(first, CORE_MACHINE_RTC_EQUIPMENT) != 0x5au;
    }
    failed |= vm_model40_cmos_seed_session_create(STD_NULL, &second) != TYPE_STATUS_OK ||
        second == STD_NULL;
    if (!failed) failed |= !vm_model40_cmos_seed_matches(second);
    vm_model40_cmos_seed_bytes(seed);
    failed |= !failed && vm_model40_cmos_seed_session_create(seed, &seeded) != TYPE_STATUS_OK;
    if (!failed) {
        failed |= vm_model40_cmos_read(seeded, CORE_MACHINE_RTC_EQUIPMENT) !=
                seed[CORE_MACHINE_RTC_EQUIPMENT] ||
            vm_model40_cmos_read(seeded, 0x33u) != seed[0x33u] ||
            vm_model40_cmos_read(seeded, 0x2eu) != seed[0x2eu] ||
            vm_model40_cmos_read(seeded, 0x2fu) != seed[0x2fu] ||
            !vm_model40_cmos_checksum_is_valid(seeded);
    }
    vm_session_destroy(seeded);
    vm_session_destroy(second);
    vm_session_destroy(first);
    if (failed) return 1;
    STD_PRINTF("M5:T513:S4:MODEL40-CMOS-SEED:OK\n");
    return 0;
}
