#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "core/machine/memory.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"

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

static C_INT write_video_rom(const C_CHAR *path)
{
    type_unsigned_8 bytes[VM_PROFILE_MODEL40_VIDEO_ROM_BYTES] = {0};
    STD_FILE *file;

    bytes[0u] = 0x55u;
    bytes[1u] = 0xaau;
    bytes[2u] = 0x20u;
    bytes[sizeof(bytes) - 1u] = 0xe1u;
    file = STD_FOPEN(path, "wb");
    return file != STD_NULL && STD_FWRITE(bytes, 1u, sizeof(bytes), file) ==
        sizeof(bytes) && STD_FCLOSE(file) == 0;
}

C_INT main(C_VOID)
{
    static const C_CHAR even_sha256[] = "4fe7b59af6de3b665b67788cc2f99892ab827efae3a467342b3bb4e3bc8e5bfe";
    static const C_CHAR odd_sha256[] = "111ce3c2a38d83a2e4706bde4abddd509d7f8248116c6832b06745bdc349e09f";
    static const C_CHAR video_sha256[] = "a78e2f3536f04d7c4b9b8a729659bce3242ca1ddc1da93820271ebd4781030a5";
    vm_session_config config = {0};
    vm_session *session = STD_NULL;
    vm_session_reset_vector reset_vector = {0};
    core_machine_run_result result = {0};
    core_machine_time_observation time_observation = {0};
    STD_SIZE_T memory_bytes = 0u;
    STD_SIZE_T retained_memory_bytes;
    type_unsigned_8 observed_memory = 0u;
    type_unsigned_8 video_body_byte = 0u;
    STD_SIZE_T mapping;
    C_INT failed = 0;

    if (!write_chip("t386-s20-even.bin", 0u) || !write_chip("t386-s20-odd.bin", 1u) ||
        !write_video_rom("t386-s20-video.bin")) failed = 1;
    config.profile_kind = VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40;
    (C_VOID)even_sha256; (C_VOID)odd_sha256; (C_VOID)video_sha256;
    config.bios_path[0u] = "t386-s20-even.bin";
    config.bios_path[1u] = "t386-s20-odd.bin";
    config.bios_count = 2u;
    config.video_path = "t386-s20-video.bin";
    failed |= vm_session_create(&config, &session) != TYPE_STATUS_OK || session == STD_NULL ||
        !session->model40_private || session->core_machine_config.memory_bytes != 2u * 1024u * 1024u ||
        session->core_machine_config.retirement_time_contract !=
            CORE_MACHINE_RETIREMENT_TIME_DETERMINISTIC ||
        session->core_machine_config.l1_compatibility_policy !=
            CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS ||
        session->core_machine_config.cpu_profile != CORE_MACHINE_CPU_PROFILE_80386 ||
        session->core_machine_config.fpu_profile != CORE_MACHINE_FPU_PROFILE_NONE ||
        !session->core_machine_config.cpu_80386_cr_mov_ignores_mod ||
        core_machine_capture_time_observation(session->core_machine,
            &time_observation) != TYPE_STATUS_OK || !time_observation.pacing_time_available ||
        time_observation.pacing_ticks_per_second != 16000000u || time_observation.physical_time_available ||
        time_observation.physical_ticks_per_second != 0u ||
        session->fdd.data.nsector != 15u || session->model40_rom.even_bytes[0] != 0u ||
        session->model40_rom.odd_bytes[0] != 1u ||
        session->model40_rom.video_bytes == STD_NULL ||
        session->model40_rom.video_bytes[0u] != 0x55u ||
        core_machine_memory_read(session->core_machine,
            VM_PROFILE_MODEL40_VIDEO_ROM_PHYSICAL_START, &observed_memory,
            sizeof(observed_memory)) != TYPE_STATUS_OK || observed_memory != 0x55u;
    for (mapping = 0u; mapping < session->core_machine->immutable_rom_mapping_count;
        ++mapping) {
        if (session->core_machine->immutable_rom_mappings[mapping].physical_start ==
            VM_PROFILE_MODEL40_VIDEO_ROM_COMPATIBILITY_ALIAS_START +
                VM_PROFILE_MODEL40_VIDEO_ROM_ALIAS_SKIP_BYTES) break;
    }
    failed |= mapping == session->core_machine->immutable_rom_mapping_count;
    failed |= !failed && (core_machine_memory_read(session->core_machine,
        VM_PROFILE_MODEL40_VIDEO_ROM_PHYSICAL_START +
            VM_PROFILE_MODEL40_VIDEO_ROM_ALIAS_SKIP_BYTES, &video_body_byte,
        sizeof(video_body_byte)) != TYPE_STATUS_OK || core_machine_memory_read(session->core_machine,
        VM_PROFILE_MODEL40_VIDEO_ROM_COMPATIBILITY_ALIAS_START +
            VM_PROFILE_MODEL40_VIDEO_ROM_ALIAS_SKIP_BYTES, &observed_memory,
        sizeof(observed_memory)) != TYPE_STATUS_OK || observed_memory != video_body_byte);
    failed |= !failed && (vm_session_get_reset_vector(session, &reset_vector) != TYPE_STATUS_OK ||
        reset_vector.cs != 0xf000u || reset_vector.ip != 0xfff0u);
    retained_memory_bytes = session->retained_config.memory_bytes;
    failed |= !failed && vm_session_reconfigure_memory(session, 2u * 1024u * 1024u) !=
        TYPE_STATUS_INVALID_STATE;
    failed |= !failed && (core_machine_get_memory_bytes(session->core_machine,
        &memory_bytes) != TYPE_STATUS_OK || memory_bytes != 2u * 1024u * 1024u ||
        session->core_machine_config.memory_bytes != 2u * 1024u * 1024u ||
        session->retained_config.memory_bytes != retained_memory_bytes);
    failed |= !failed && (core_machine_run(session->core_machine,
        (core_machine_run_budget) {1u, 0u}, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u);
    failed |= !failed && (core_machine_reset(session->core_machine) != TYPE_STATUS_OK ||
        core_machine_advance_time(session->core_machine, 1u) != TYPE_STATUS_OK ||
        session->core_machine->shared_pit.data.reload[1u] != 18u);
    failed |= !failed && (vm_session_get_reset_vector(session, &reset_vector) != TYPE_STATUS_OK ||
        reset_vector.cs != 0xf000u || reset_vector.ip != 0xfff0u);
    failed |= !failed && !write_chip("t386-s20-even.bin", 2u);
    failed |= !failed && (session->model40_rom.even_bytes[0] != 0u ||
        session->model40_rom.odd_bytes[0] != 1u);
    vm_session_destroy(session);
    session = STD_NULL;
    config.memory_bytes = 2u * 1024u * 1024u;
    failed |= vm_session_create(&config, &session) != TYPE_STATUS_INVALID_ARGUMENT ||
        session != STD_NULL;
    config.memory_bytes = 0u;
    (C_VOID)STD_REMOVE("t386-s20-even.bin");
    (C_VOID)STD_REMOVE("t386-s20-odd.bin");
    (C_VOID)STD_REMOVE("t386-s20-video.bin");
    if (!failed) STD_PRINTF("M5:T386:S20:MODEL40-BYOB-MANIFEST:OK\nM5:T386:S20:MODEL40-BYOB-VALIDATION:OK\nM5:T386:S20:MODEL40-PUBLIC-COMPOSITION:OK\nM5:T424:S1:MODEL40-BYOB-RESET-LIFECYCLE:OK\nM5:T440:S1:MODEL40-IMMUTABLE-CONFIGURATION:OK\n");
    return failed;
}
