#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/retirement_observation_interface.h"
#include "vm/composition/session/session.h"

typedef struct model40_retirement_capture {
    type_unsigned_32 count;
    type_unsigned_32 classified;
    type_unsigned_32 unallocated;
} model40_retirement_capture;

static const C_CHAR *model40_capture_form_name(
    const core_machine_retirement_observation *observation)
{
    if (observation == STD_NULL) return "unavailable";
    switch (observation->point.bytes[0]) {
    case 0x0fu:
        return observation->point.bytes[1] == 0x01u &&
            observation->point.bytes[2] == 0xf0u ? "lmsw-register" :
            "other";
    case 0x32u: case 0x33u:
        return "xor-register-register";
    case 0x75u:
        return "jcc";
    case 0x80u:
        return "cmp-register-immediate";
    case 0x8au: case 0x8bu:
        return (observation->point.bytes[1] & 0xc0u) == 0xc0u ?
            "mov-register-register" : "mov-register-rm";
    case 0x8eu:
        return "mov-sreg";
    case 0x9eu:
        return "sahf";
    case 0xa8u:
        return "test-accumulator-immediate";
    case 0xacu:
        return "lods";
    case 0xe2u:
        return "loop";
    case 0xe4u:
        return "in-immediate";
    case 0xe6u:
        return "out-immediate";
    case 0xeau:
        return "jmp-far-direct";
    case 0xebu:
        return "jmp-direct";
    case 0xeeu:
        return "out-dx";
    case 0xf8u:
        return "clc";
    case 0xfau:
        return "cli";
    default:
        return observation->point.bytes[0] >= 0xb0u &&
            observation->point.bytes[0] <= 0xbfu ? "mov-immediate" :
            "other";
    }
}

static const C_CHAR *model40_capture_operand_name(
    const core_machine_retirement_observation *observation)
{
    if (observation == STD_NULL || observation->point.bytes[0] != 0x8eu) {
        return "not-applicable";
    }
    return (observation->point.bytes[1] & 0xc0u) == 0xc0u ?
        "register" : "memory";
}

static C_VOID model40_capture_observe(C_VOID *opaque,
    const core_machine_retirement_observation *observation)
{
    model40_retirement_capture *capture =
        (model40_retirement_capture *)opaque;

    if (capture == STD_NULL || observation == STD_NULL) return;
    ++capture->count;
    if (observation->timing_disposition ==
        CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED) {
        ++capture->classified;
    } else {
        ++capture->unallocated;
    }
    STD_PRINTF("T390 form=%s operand=%s ticks=%llu cpl=%u pm=%u vm=%u os32=%u as32=%u "
        "lock=%u rep=%u disposition=%u\n",
        model40_capture_form_name(observation),
        model40_capture_operand_name(observation),
        (unsigned long long)observation->source_ticks,
        (unsigned)observation->cpl, (unsigned)observation->protected_mode,
        (unsigned)observation->virtual_8086_mode,
        (unsigned)observation->operand_size_32,
        (unsigned)observation->address_size_32,
        (unsigned)observation->lock_prefix,
        (unsigned)observation->repeat_prefix,
        (unsigned)observation->timing_disposition);
}

static C_INT model40_capture_create_session(C_INT argc, C_CHAR **argv,
    vm_session **out_session)
{
    vm_session_config config = { 0 };

    if (argv == STD_NULL || out_session == STD_NULL || argc != 6) return 0;
    config.profile_kind = VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40;
    config.model40_firmware = (vm_profile_model40_byob_manifest) {
        argv[1], argv[2], argv[3], argv[4], argv[5]
    };
    return vm_session_create(&config, out_session) == TYPE_STATUS_OK &&
        *out_session != STD_NULL;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_retirement_observation_provider provider;
    core_machine_run_result result;
    model40_retirement_capture capture = { 0u, 0u, 0u };
    vm_session *session = STD_NULL;
    type_status status = TYPE_STATUS_OK;
    type_unsigned_32 index;

    if (!model40_capture_create_session(argc, argv, &session)) {
        STD_FPRINTF(STD_STDERR, "usage: capture even-image even-digest "
            "odd-image odd-digest provenance\n");
        return 2;
    }
    provider.callback = model40_capture_observe;
    provider.context = &capture;
    status = core_machine_set_retirement_observation_provider(
        session->core_machine, &provider);
    if (status == TYPE_STATUS_OK) status = core_machine_reset(session->core_machine);
    for (index = 0u; status == TYPE_STATUS_OK && index < 64u &&
        capture.count < 64u && capture.unallocated == 0u; ++index) {
        status = core_machine_run(session->core_machine, budget, &result);
        if (status != TYPE_STATUS_OK ||
            result.reason == CORE_MACHINE_STOP_FAULT) break;
    }
    STD_PRINTF("M5:T390:S4:BYOB-CAPTURE:count=%u classified=%u "
        "unallocated=%u status=%u\n", (unsigned)capture.count,
        (unsigned)capture.classified, (unsigned)capture.unallocated,
        (unsigned)status);
    vm_session_destroy(session);
    return capture.count == 0u || capture.unallocated != 0u ||
        status != TYPE_STATUS_OK ? 1 : 0;
}