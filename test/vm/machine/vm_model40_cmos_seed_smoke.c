#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/port.h"
#include "core/machine/rtc.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_private.h"
#include "../support/vm_model40_byob_fixture.h"

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

static C_INT vm_model40_cmos_seed_file_write(const C_CHAR *path,
    type_unsigned_8 bytes[VM_SESSION_CMOS_SEED_BYTES])
{
    STD_FILE *file;

    STD_MEMSET(bytes, 0, VM_SESSION_CMOS_SEED_BYTES);
    bytes[CORE_MACHINE_RTC_TYPE_DISK_FLOPPY] = 0x22u;
    bytes[CORE_MACHINE_RTC_TYPE_DISK_FIXED] = 0x80u;
    bytes[CORE_MACHINE_RTC_EQUIPMENT] = 0x5au;
    bytes[CORE_MACHINE_RTC_BASEMEM_LSB] = 0x80u;
    bytes[CORE_MACHINE_RTC_BASEMEM_MSB] = 0x02u;
    bytes[CORE_MACHINE_RTC_EXTMEM_LSB] = 0x00u;
    bytes[CORE_MACHINE_RTC_EXTMEM_MSB] = 0x04u;
    bytes[0x2eu] = 0xffu;
    bytes[0x2fu] = 0xffu;
    file = STD_FOPEN(path, "wb");
    return file != STD_NULL && STD_FWRITE(bytes, 1u, VM_SESSION_CMOS_SEED_BYTES,
        file) == VM_SESSION_CMOS_SEED_BYTES && STD_FCLOSE(file) == 0;
}

static type_status vm_model40_cmos_seed_file_session_create(const C_CHAR *even_path,
    const C_CHAR *odd_path, const C_CHAR *cmos_path, vm_session **out_session)
{
    static const C_CHAR even_sha256[] =
        "4fe7b59af6de3b665b67788cc2f99892ab827efae3a467342b3bb4e3bc8e5bfe";
    static const C_CHAR odd_sha256[] =
        "111ce3c2a38d83a2e4706bde4abddd509d7f8248116c6832b06745bdc349e09f";
    type_unsigned_8 even_bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES] = {0};
    type_unsigned_8 odd_bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    vm_session_config config = {0};

    STD_MEMSET(odd_bytes, 1, sizeof(odd_bytes));
    if (!vm_model40_fixture_write_bytes(even_path, even_bytes) ||
        !vm_model40_fixture_write_bytes(odd_path, odd_bytes)) return TYPE_STATUS_FAULT;
    config.profile_kind = VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40;
    config.cmos_seed = cmos_path;
    (C_VOID)even_sha256; (C_VOID)odd_sha256;
    config.bios_path[0u] = even_path;
    config.bios_path[1u] = odd_path;
    config.bios_count = 2u;
    return (type_status)vm_session_create(&config, out_session);
}

C_INT main(C_VOID)
{
    vm_session *first = STD_NULL;
    vm_session *second = STD_NULL;
    vm_session *seeded = STD_NULL;
    type_unsigned_8 seed[VM_SESSION_CMOS_SEED_BYTES];
    C_INT failed = 0;

    failed |= vm_model40_fixture_create("t513-model40-cmos-even.bin",
        "t513-model40-cmos-odd.bin", &first) != TYPE_STATUS_OK || first == STD_NULL;
    if (!failed) failed |= !vm_model40_cmos_seed_matches(first);
    if (!failed) {
        vm_model40_cmos_write(first, CORE_MACHINE_RTC_EQUIPMENT, 0x5au);
        failed |= vm_session_reset(first) != TYPE_STATUS_OK ||
            vm_model40_cmos_read(first, CORE_MACHINE_RTC_EQUIPMENT) != 0x5au;
    }
    failed |= vm_model40_fixture_create("t513-model40-cmos-even.bin",
        "t513-model40-cmos-odd.bin", &second) != TYPE_STATUS_OK || second == STD_NULL;
    if (!failed) failed |= !vm_model40_cmos_seed_matches(second);
    failed |= !failed && !vm_model40_cmos_seed_file_write("t515-model40.cmos", seed);
    failed |= !failed && vm_model40_cmos_seed_file_session_create(
        "t515-model40-even.bin", "t515-model40-odd.bin", "t515-model40.cmos",
        &seeded) != TYPE_STATUS_OK;
    if (!failed) {
        failed |= vm_model40_cmos_read(seeded, CORE_MACHINE_RTC_EQUIPMENT) != 0x5au ||
            vm_model40_cmos_read(seeded, 0x2eu) == seed[0x2eu] ||
            vm_model40_cmos_read(seeded, 0x2fu) == seed[0x2fu] ||
            !vm_model40_cmos_checksum_is_valid(seeded);
    }
    vm_session_destroy(seeded);
    vm_session_destroy(second);
    vm_session_destroy(first);
    vm_model40_fixture_remove("t513-model40-cmos-even.bin",
        "t513-model40-cmos-odd.bin");
    vm_model40_fixture_remove("t515-model40-even.bin", "t515-model40-odd.bin");
    (C_VOID)STD_REMOVE("t515-model40.cmos");
    if (failed) return 1;
    STD_PRINTF("M5:T513:S4:MODEL40-CMOS-SEED:OK\n");
    return 0;
}
