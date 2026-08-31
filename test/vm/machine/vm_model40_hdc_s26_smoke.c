#include "type.h"

#include "core/machine/hdc.h"
#include "core/machine/machine.h"
#include "core/machine/port.h"
#include "vm/composition/session/media.h"
#include "vm/composition/session/session_private.h"

#define MODEL40_HDC_BYTES (925u * 5u * 17u * 512u)

static C_INT write_chip(const C_CHAR *path, type_unsigned_8 value)
{
    type_unsigned_8 bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    STD_FILE *file;

    STD_MEMSET(bytes, value, sizeof(bytes));
    file = STD_FOPEN(path, "wb");
    return file != STD_NULL && STD_FWRITE(bytes, 1u, sizeof(bytes), file) == sizeof(bytes) &&
        STD_FCLOSE(file) == 0;
}

static C_INT write_hdd(const C_CHAR *path, type_unsigned_8 first, type_unsigned_8 second)
{
    type_unsigned_8 sector[512] = {0};
    type_unsigned_8 final = 0u;
    STD_FILE *file;

    sector[0] = first;
    sector[1] = second;
    file = STD_FOPEN(path, "wb");
    if (file == STD_NULL || STD_FWRITE(sector, 1u, sizeof(sector), file) != sizeof(sector) ||
        STD_FSEEK_64(file, (type_signed_64)MODEL40_HDC_BYTES - 1, STD_SEEK_SET) != 0 ||
        STD_FWRITE(&final, 1u, 1u, file) != 1u) {
        if (file != STD_NULL) (C_VOID)STD_FCLOSE(file);
        return 0;
    }
    return STD_FCLOSE(file) == 0;
}

static C_INT write_short_hdd(const C_CHAR *path)
{
    type_unsigned_8 sector[512] = {0};
    STD_FILE *file = STD_FOPEN(path, "wb");

    return file != STD_NULL && STD_FWRITE(sector, 1u, sizeof(sector), file) == sizeof(sector) &&
        STD_FCLOSE(file) == 0;
}
static C_INT read_first_sector(vm_session *session, type_unsigned_8 drive_head,
    type_unsigned_16 expected_word)
{
    core_machine_hdc *hdc;
    type_unsigned_32 value;
    type_unsigned_16 index;

    if (session == STD_NULL || session->core_machine == STD_NULL) return 0;
    hdc = &session->core_machine->hdc;
    core_machine_port_write(&session->core_machine->executor_port, 0x01f2u, 1u);
    core_machine_port_write(&session->core_machine->executor_port, 0x01f3u, 1u);
    core_machine_port_write(&session->core_machine->executor_port, 0x01f4u, 0u);
    core_machine_port_write(&session->core_machine->executor_port, 0x01f5u, 0u);
    core_machine_port_write(&session->core_machine->executor_port, 0x01f6u, drive_head);
    core_machine_port_write(&session->core_machine->executor_port, 0x01f7u, 0x20u);
    core_machine_hdc_advance(hdc);
    value = core_machine_port_read(&session->core_machine->executor_port, 0x03f6u);
    if ((value & CORE_MACHINE_HDC_STATUS_DRQ) == 0u || !core_machine_hdc_irq_pending(hdc)) return 0;
    value = core_machine_port_read(&session->core_machine->executor_port, 0x01f7u);
    if ((value & CORE_MACHINE_HDC_STATUS_DRQ) == 0u || core_machine_hdc_irq_pending(hdc) ||
        core_machine_port_read(&session->core_machine->executor_port, 0x01f0u) != expected_word) return 0;
    for (index = 1u; index < 256u; ++index) {
        (C_VOID)core_machine_port_read(&session->core_machine->executor_port, 0x01f0u);
    }
    core_machine_hdc_advance(hdc);
    if (!core_machine_hdc_irq_pending(hdc)) return 0;
    (C_VOID)core_machine_port_read(&session->core_machine->executor_port, 0x01f7u);
    core_machine_port_write(&session->core_machine->executor_port, 0x01f7u, 0xecu);
    core_machine_hdc_advance(hdc);
    value = core_machine_port_read(&session->core_machine->executor_port, 0x01f7u);
    if ((value & CORE_MACHINE_HDC_STATUS_ERR) == 0u || core_machine_hdc_irq_pending(hdc)) return 0;
    core_machine_port_write(&session->core_machine->executor_port, 0x03f6u,
        CORE_MACHINE_HDC_DEVICE_CONTROL_SRST);
    core_machine_port_write(&session->core_machine->executor_port, 0x03f6u, 0u);
    return !core_machine_hdc_irq_pending(hdc) &&
        core_machine_port_read(&session->core_machine->executor_port, 0x01f1u) ==
            CORE_MACHINE_HDC_ERROR_DIAGNOSTIC_OK &&
        core_machine_port_read(&session->core_machine->executor_port, 0x03f6u) ==
            (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC);
}

C_INT main(C_VOID)
{
    static const C_CHAR even_sha256[] = "4fe7b59af6de3b665b67788cc2f99892ab827efae3a467342b3bb4e3bc8e5bfe";
    static const C_CHAR odd_sha256[] = "111ce3c2a38d83a2e4706bde4abddd509d7f8248116c6832b06745bdc349e09f";
    vm_session_config config = {0};
    vm_session *session = STD_NULL;
    C_INT failed = 0;

    if (!write_chip("t386-s26-even.bin", 0u)) { failed = 1; }
    if (!write_chip("t386-s26-odd.bin", 1u)) { failed = 1; }
    if (!write_hdd("t386-s26-hdd.img", 0xa5u, 0x5au)) { failed = 1; }
    config.profile_kind = VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40;
    config.hdd_image = "t386-s26-hdd.img";
    config.model40_firmware = (vm_profile_model40_byob_manifest) {
        .even_path = "t386-s26-even.bin", .even_sha256 = even_sha256,
        .odd_path = "t386-s26-odd.bin", .odd_sha256 = odd_sha256,
        .provenance = "project-owned synthetic test input" };
    failed |= !failed && (vm_session_create(&config, &session) != TYPE_STATUS_OK || session == STD_NULL);
    failed |= !failed && (session == STD_NULL || !session->hdd.connect.flagDiskExist ||
        session->core_machine->hdc.connect.config.service.command_ticks != 0u ||
        session->core_machine->hdc.connect.config.service.next_sector_ticks != 0u ||
        session->hdd.data.ncyl != 925u || session->hdd.data.nhead != 5u ||
        session->hdd.data.nsector != 17u || session->hdd.data.nbyte != 512u ||
        session->core_machine->hdc.connect.slave_media_id != CORE_MACHINE_MEDIA_ID_INVALID ||
        vm_session_insert_hdd(session, "t386-s26-hdd.img") == 0 ||
        !read_first_sector(session, 0x20u, 0x5aa5u) ||
        !read_first_sector(session, 0xa0u, 0x5aa5u));
    if (!failed) core_machine_port_write(&session->core_machine->executor_port, 0x01f6u, 0x20u);
    failed |= !failed && core_machine_port_read(&session->core_machine->executor_port, 0x03f6u) !=
        (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC);
    if (!failed) core_machine_port_write(&session->core_machine->executor_port, 0x01f6u, 0x00u);
    failed |= !failed && core_machine_port_read(&session->core_machine->executor_port, 0x03f6u) !=
        (CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC);
    vm_session_destroy(session);
    session = STD_NULL;
    config.hdd_image = STD_NULL;
    failed |= !failed && (vm_session_create(&config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL);
    if (!failed) core_machine_port_write(&session->core_machine->executor_port, 0x01f6u, 0x20u);
    failed |= !failed && (
        core_machine_port_read(&session->core_machine->executor_port, 0x03f6u) != 0u);
    vm_session_destroy(session);
    session = STD_NULL;
    if (!write_short_hdd("t386-s26-bad-hdd.img")) failed = 1;
    config.hdd_image = "t386-s26-bad-hdd.img";
    failed |= !failed && (vm_session_create(&config, &session) != TYPE_STATUS_FAULT ||
        session != STD_NULL);
    (C_VOID)STD_REMOVE("t386-s26-even.bin");
    (C_VOID)STD_REMOVE("t386-s26-odd.bin");
    (C_VOID)STD_REMOVE("t386-s26-hdd.img");
    (C_VOID)STD_REMOVE("t386-s26-bad-hdd.img");
    if (!failed) {
        STD_PRINTF("M5:T386:S26:MODEL40-HDC-STARTUP:OK\n");
        STD_PRINTF("M5:T386:S26:MODEL40-HDC-FIXED-MEDIA:OK\n");
    }
    return failed;
}
