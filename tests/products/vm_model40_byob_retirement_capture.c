#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/retirement_observation_interface.h"
#include "vm/composition/session/session.h"

#define MODEL40_CAPTURE_FORM_LIMIT 128u
#define MODEL40_CAPTURE_RETIREMENT_LIMIT 250000u
#define MODEL40_BOOT_SECTOR_LINEAR_PC 0x00007c00u

typedef struct model40_retirement_capture_form {
    const C_CHAR *form;
    const C_CHAR *operand;
    type_unsigned_64 ticks;
    type_unsigned_8 cpl;
    type_bool protected_mode;
    type_bool virtual_8086_mode;
    type_bool operand_size_32;
    type_bool address_size_32;
    type_bool lock_prefix;
    type_unsigned_8 repeat_prefix;
    core_machine_retirement_timing_disposition disposition;
    type_unsigned_32 count;
} model40_retirement_capture_form;

typedef struct model40_retirement_capture {
    model40_retirement_capture_form forms[MODEL40_CAPTURE_FORM_LIMIT];
    type_unsigned_32 count;
    type_unsigned_32 classified;
    type_unsigned_32 unallocated;
    type_unsigned_32 form_count;
    type_unsigned_8 terminal_bytes[CORE_MACHINE_CPU_DIAGNOSTIC_BYTES];
    type_unsigned_8 terminal_byte_count;
    type_bool checkpoint_reached;
    type_bool form_limit_reached;
    type_bool terminal_bytes_available;
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
    case 0x06u: case 0x0eu: case 0x16u: case 0x1eu:
        return "push-segment";
    case 0x07u: case 0x17u: case 0x1fu:
        return "pop-segment";
    case 0x32u: case 0x33u:
        return "xor-register-register";
    case 0x40u: case 0x41u: case 0x42u: case 0x43u:
    case 0x44u: case 0x45u: case 0x46u: case 0x47u:
        return "inc-register";
    case 0x48u: case 0x49u: case 0x4au: case 0x4bu:
    case 0x4cu: case 0x4du: case 0x4eu: case 0x4fu:
        return "dec-register";
    case 0x50u: case 0x51u: case 0x52u: case 0x53u:
    case 0x54u: case 0x55u: case 0x56u: case 0x57u:
        return "push-register";
    case 0x58u: case 0x59u: case 0x5au: case 0x5bu:
    case 0x5cu: case 0x5du: case 0x5eu: case 0x5fu:
        return "pop-register";
    case 0x75u:
        return "jcc";
    case 0x80u:
        return "cmp-register-immediate";
    case 0x8au: case 0x8bu:
        return (observation->point.bytes[1] & 0xc0u) == 0xc0u ?
            "mov-register-register" : "mov-register-rm";
    case 0x8eu:
        return "mov-sreg";
    case 0xd0u:
        return (observation->point.bytes[1] & 0xf8u) == 0xe0u ?
            "sal-register-one" : "group2-one";
    case 0x9cu:
        return "pushf";
    case 0x9du:
        return "popf";
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
    case 0xf5u:
        return "cmc";
    case 0xf8u:
        return "clc";
    case 0xf9u:
        return "stc";
    case 0xfau:
        return "cli";
    case 0xfbu:
        return "sti";
    case 0xfcu:
        return "cld";
    case 0xfdu:
        return "std";
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

static C_INT model40_capture_form_matches(
    const model40_retirement_capture_form *form, const C_CHAR *name,
    const C_CHAR *operand, const core_machine_retirement_observation *observation)
{
    return form != STD_NULL && observation != STD_NULL &&
        !STD_STRCMP(form->form, name) && !STD_STRCMP(form->operand, operand) &&
        form->ticks == observation->source_ticks && form->cpl == observation->cpl &&
        form->protected_mode == observation->protected_mode &&
        form->virtual_8086_mode == observation->virtual_8086_mode &&
        form->operand_size_32 == observation->operand_size_32 &&
        form->address_size_32 == observation->address_size_32 &&
        form->lock_prefix == observation->lock_prefix &&
        form->repeat_prefix == observation->repeat_prefix &&
        form->disposition == observation->timing_disposition;
}

static C_VOID model40_capture_observe(C_VOID *opaque,
    const core_machine_retirement_observation *observation)
{
    model40_retirement_capture *capture =
        (model40_retirement_capture *)opaque;
    const C_CHAR *name;
    const C_CHAR *operand;
    type_unsigned_32 index;

    if (capture == STD_NULL || observation == STD_NULL) return;
    ++capture->count;
    if ((capture->count & 1023u) == 0u) {
        STD_PRINTF("M5:T390:S5:BYOB-BOOT-PROGRESS:count=%u\n",
            (unsigned)capture->count);
        (C_VOID)fflush(STD_STDOUT);
    }
    if (observation->timing_disposition ==
        CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED) {
        ++capture->classified;
    } else {
        ++capture->unallocated;
        if (!capture->terminal_bytes_available) {
            capture->terminal_byte_count = observation->point.byte_count;
            STD_MEMCPY(capture->terminal_bytes, observation->point.bytes,
                sizeof(capture->terminal_bytes));
            capture->terminal_bytes_available = TYPE_TRUE;
        }
    }
    if (observation->point.linear_pc == MODEL40_BOOT_SECTOR_LINEAR_PC) {
        capture->checkpoint_reached = TYPE_TRUE;
    }
    name = model40_capture_form_name(observation);
    operand = model40_capture_operand_name(observation);
    for (index = 0u; index < capture->form_count; ++index) {
        if (model40_capture_form_matches(&capture->forms[index], name, operand,
                observation)) {
            ++capture->forms[index].count;
            return;
        }
    }
    if (capture->form_count == MODEL40_CAPTURE_FORM_LIMIT) {
        capture->form_limit_reached = TYPE_TRUE;
        return;
    }
    capture->forms[capture->form_count++] = (model40_retirement_capture_form) {
        name, operand, observation->source_ticks, observation->cpl,
        observation->protected_mode, observation->virtual_8086_mode,
        observation->operand_size_32, observation->address_size_32,
        observation->lock_prefix, observation->repeat_prefix,
        observation->timing_disposition, 1u };
}

static C_VOID model40_capture_emit_terminal_bytes(
    const model40_retirement_capture *capture)
{
    type_unsigned_8 index;

    if (capture == STD_NULL || !capture->terminal_bytes_available) return;
    STD_PRINTF("T390 terminal-bytes=");
    for (index = 0u; index < capture->terminal_byte_count &&
        index < sizeof(capture->terminal_bytes); ++index) {
        STD_PRINTF("%02X", (unsigned)capture->terminal_bytes[index]);
    }
    STD_PRINTF("\n");
}

static C_VOID model40_capture_emit(const model40_retirement_capture *capture)
{
    type_unsigned_32 index;

    if (capture == STD_NULL) return;
    for (index = 0u; index < capture->form_count; ++index) {
        const model40_retirement_capture_form *form = &capture->forms[index];

        STD_PRINTF("T390 form=%s operand=%s ticks=%llu cpl=%u pm=%u vm=%u os32=%u "
            "as32=%u lock=%u rep=%u disposition=%u count=%u\n",
            form->form, form->operand, (unsigned long long)form->ticks,
            (unsigned)form->cpl, (unsigned)form->protected_mode,
            (unsigned)form->virtual_8086_mode, (unsigned)form->operand_size_32,
            (unsigned)form->address_size_32, (unsigned)form->lock_prefix,
            (unsigned)form->repeat_prefix, (unsigned)form->disposition,
            (unsigned)form->count);
    }
}

static C_INT model40_capture_create_session(C_INT argc, C_CHAR **argv,
    vm_session **out_session)
{
    vm_session_config config = { 0 };

    if (argv == STD_NULL || out_session == STD_NULL ||
        (argc != 7 && (argc != 8 || STD_STRCMP(argv[7], "--terminal-bytes")))) return 0;
    config.profile_kind = VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40;
    config.fdd_image = argv[6];
    config.model40_firmware = (vm_profile_model40_byob_manifest) {
        argv[1], argv[2], argv[3], argv[4], argv[5] };
    return vm_session_create(&config, out_session) == TYPE_STATUS_OK &&
        *out_session != STD_NULL;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_retirement_observation_provider provider;
    core_machine_run_result result = { 0 };
    model40_retirement_capture capture = { 0 };
    vm_session *session = STD_NULL;
    type_status status = TYPE_STATUS_OK;
    type_unsigned_32 index;
    const C_CHAR *terminal = "retirement-budget";
    C_INT emit_terminal_bytes = argc == 8 && argv != STD_NULL &&
        !STD_STRCMP(argv[7], "--terminal-bytes");

    if (!model40_capture_create_session(argc, argv, &session)) {
        STD_FPRINTF(STD_STDERR, "usage: capture even-image even-digest "
            "odd-image odd-digest provenance floppy-image [--terminal-bytes]\n");
        return 2;
    }
    provider.callback = model40_capture_observe;
    provider.context = &capture;
    status = core_machine_set_retirement_observation_provider(
        session->core_machine, &provider);
    if (status == TYPE_STATUS_OK) status = core_machine_reset(session->core_machine);
    for (index = 0u; status == TYPE_STATUS_OK && index <
        MODEL40_CAPTURE_RETIREMENT_LIMIT && !capture.checkpoint_reached &&
        capture.unallocated == 0u && !capture.form_limit_reached; ++index) {
        status = core_machine_run(session->core_machine, budget, &result);
        if (status != TYPE_STATUS_OK || result.reason == CORE_MACHINE_STOP_FAULT) break;
    }
    if (capture.checkpoint_reached) terminal = "boot-sector-linear-7c00";
    else if (capture.unallocated != 0u) terminal = "source-timing-unallocated";
    else if (capture.form_limit_reached) terminal = "form-capacity";
    else if (status != TYPE_STATUS_OK) terminal = "run-status";
    else if (result.reason == CORE_MACHINE_STOP_FAULT) terminal = "fault";
    model40_capture_emit(&capture);
    if (emit_terminal_bytes) model40_capture_emit_terminal_bytes(&capture);
    STD_PRINTF("M5:T390:S7:BYOB-BOOT-CAPTURE:terminal=%s count=%u classified=%u "
        "unallocated=%u forms=%u status=%u\n", terminal, (unsigned)capture.count,
        (unsigned)capture.classified, (unsigned)capture.unallocated,
        (unsigned)capture.form_count, (unsigned)status);
    vm_session_destroy(session);
    return capture.checkpoint_reached && capture.unallocated == 0u &&
        !capture.form_limit_reached && status == TYPE_STATUS_OK ? 0 : 1;
}