#include "type.h"

#include "core/machine/media_interface.h"
#include "vm/composition/session/media.h"
#include "vm/composition/session/session_private.h"

#define MODEL40_FDD_BYTES (80u * 2u * 15u * 512u)

static C_INT write_chip(const C_CHAR *path, type_unsigned_8 value)
{
    type_unsigned_8 bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    STD_FILE *file;

    STD_MEMSET(bytes, value, sizeof(bytes));
    file = STD_FOPEN(path, "wb");
    return file != STD_NULL && STD_FWRITE(bytes, 1u, sizeof(bytes), file) ==
        sizeof(bytes) && STD_FCLOSE(file) == 0;
}

static C_INT write_floppy(const C_CHAR *path)
{
    static type_unsigned_8 image[MODEL40_FDD_BYTES];
    STD_FILE *file;

    image[0] = 0xebu;
    image[1] = 0x3cu;
    image[510] = 0x55u;
    image[511] = 0xaau;
    file = STD_FOPEN(path, "wb");
    return file != STD_NULL && STD_FWRITE(image, 1u, sizeof(image), file) ==
        sizeof(image) && STD_FCLOSE(file) == 0;
}

C_INT main(C_VOID)
{
    static const C_CHAR even_sha256[] =
        "4fe7b59af6de3b665b67788cc2f99892ab827efae3a467342b3bb4e3bc8e5bfe";
    static const C_CHAR odd_sha256[] =
        "111ce3c2a38d83a2e4706bde4abddd509d7f8248116c6832b06745bdc349e09f";
    vm_session_config config = {0};
    vm_session *session = STD_NULL;
    core_machine_media_info info;
    core_machine_media_result result;
    C_INT failed = 0;

    if (!write_chip("t390-s5-even.bin", 0u) ||
        !write_chip("t390-s5-odd.bin", 1u) ||
        !write_floppy("t390-s5-floppy.img")) failed = 1;
    config.profile_kind = VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40;
    config.fdd_image = "t390-s5-floppy.img";
    config.model40_firmware = (vm_profile_model40_byob_manifest) {
        .even_path = "t390-s5-even.bin", .even_sha256 = even_sha256,
        .odd_path = "t390-s5-odd.bin", .odd_sha256 = odd_sha256,
        .provenance = "project-owned synthetic test input" };
    failed |= !failed && (vm_session_create(&config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL || !session->fdd.connect.flagDiskExist ||
        session->retained_config.fdd_image == STD_NULL ||
        session->fdd.data.ncyl != 80u || session->fdd.data.nhead != 2u ||
        session->fdd.data.nsector != 15u || session->fdd.data.nbyte != 512u ||
        core_machine_media_query(session->media_registry, VM_SESSION_MEDIA_FDD_ID,
            &info, &result) != TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK ||
        !info.present || info.geometry.logical_sector_count != 2400u ||
        info.geometry.bytes_per_sector != 512u);
    vm_session_destroy(session);
    (C_VOID)STD_REMOVE("t390-s5-even.bin");
    (C_VOID)STD_REMOVE("t390-s5-odd.bin");
    (C_VOID)STD_REMOVE("t390-s5-floppy.img");
    if (!failed) STD_PRINTF("M5:T390:S5:MODEL40-BYOB-BOOT-MEDIA:OK\n");
    return failed;
}
