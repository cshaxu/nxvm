#include "type.h"

#include "vm/composition/session/session.h"

static C_INT write_chip(const C_CHAR *path, type_unsigned_8 value)
{
    type_unsigned_8 bytes[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    STD_FILE *file;
    STD_MEMSET(bytes, value, sizeof(bytes));
    file = STD_FOPEN(path, "wb");
    if (file == STD_NULL) return 0;
    return STD_FWRITE(bytes, 1u, sizeof(bytes), file) == sizeof(bytes) &&
        STD_FCLOSE(file) == 0;
}

C_INT main(C_VOID)
{
    static const C_CHAR even_sha256[] = "4fe7b59af6de3b665b67788cc2f99892ab827efae3a467342b3bb4e3bc8e5bfe";
    static const C_CHAR odd_sha256[] = "111ce3c2a38d83a2e4706bde4abddd509d7f8248116c6832b06745bdc349e09f";
    vm_session_config config = {0};
    vm_session *session = STD_NULL;
    C_INT failed = 0;

    if (!write_chip("t386-s20-even.bin", 0u) || !write_chip("t386-s20-odd.bin", 1u)) failed = 1;
    config.profile_kind = VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40;
    config.model40_firmware = (vm_profile_model40_byob_manifest) {
        "t386-s20-even.bin", even_sha256, "t386-s20-odd.bin", odd_sha256,
        "project-owned synthetic test input" };
    failed |= vm_session_create(&config, &session) != TYPE_STATUS_OK || session == STD_NULL ||
        !session->model40_private || session->core_machine_config.memory_bytes != 1024u * 1024u ||
        session->core_machine_config.cpu_profile != CORE_MACHINE_CPU_PROFILE_80386 ||
        session->core_machine_config.fpu_profile != CORE_MACHINE_FPU_PROFILE_NONE ||
        session->fdd.data.nsector != 15u || session->model40_rom.even_bytes[0] != 0u ||
        session->model40_rom.odd_bytes[0] != 1u;
    failed |= !failed && !write_chip("t386-s20-even.bin", 2u);
    failed |= !failed && (session->model40_rom.even_bytes[0] != 0u ||
        session->model40_rom.odd_bytes[0] != 1u);
    vm_session_destroy(session);
    session = STD_NULL;
    config.model40_firmware.even_sha256 = odd_sha256;
    failed |= vm_session_create(&config, &session) != TYPE_STATUS_FAULT || session != STD_NULL;
    (C_VOID)STD_REMOVE("t386-s20-even.bin");
    (C_VOID)STD_REMOVE("t386-s20-odd.bin");
    if (!failed) STD_PRINTF("M5:T386:S20:MODEL40-BYOB-MANIFEST:OK\nM5:T386:S20:MODEL40-BYOB-VALIDATION:OK\nM5:T386:S20:MODEL40-PUBLIC-COMPOSITION:OK\n");
    return failed;
}