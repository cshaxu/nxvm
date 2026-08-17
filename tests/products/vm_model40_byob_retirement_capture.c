#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/retirement_observation_interface.h"
#include "vm/composition/session/session.h"

#define MODEL40_CAPTURE_FORM_LIMIT 128u
#define MODEL40_CAPTURE_RETIREMENT_LIMIT 2000000u
#define MODEL40_BOOT_SECTOR_LINEAR_PC 0x00007c00u

typedef struct model40_retirement_capture_form {
    const C_CHAR *form;
    const C_CHAR *operand;
    type_unsigned_8 opcode;
    type_unsigned_8 escape_opcode;
    type_unsigned_8 group_extension;
    type_unsigned_32 source_timing_form_id;
    core_machine_retirement_timing_origin timing_origin;
    core_machine_retirement_modrm_form modrm_form;
    type_unsigned_8 modrm_extension;
    core_machine_retirement_control_outcome control_outcome;
    type_unsigned_8 next_lexeme_components;
    core_machine_retirement_repeat_phase repeat_phase;
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
    type_bool c1_checkpoint_reached;
    type_bool observation_seen;
    type_bool previous_protected_mode;
    type_bool protected_mode_seen;
    type_bool form_limit_reached;
    type_bool terminal_bytes_available;
} model40_retirement_capture;

static const C_CHAR *model40_capture_form_name(
    const core_machine_retirement_observation *observation,
    type_unsigned_8 opcode_index)
{
    if (observation == STD_NULL || opcode_index >= observation->point.byte_count) {
        return "unavailable";
    }
    switch (observation->point.bytes[opcode_index]) {
    case 0x0fu:
        return opcode_index + 2u < observation->point.byte_count &&
            observation->point.bytes[opcode_index + 1u] == 0x01u &&
            observation->point.bytes[opcode_index + 2u] == 0xf0u ? "lmsw-register" :
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
        return observation->point.bytes[opcode_index] >= 0xb0u &&
            observation->point.bytes[opcode_index] <= 0xbfu ? "mov-immediate" :
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

static C_INT model40_capture_is_prefix(type_unsigned_8 opcode)
{
    switch (opcode) {
    case 0x26u: case 0x2eu: case 0x36u: case 0x3eu: case 0x64u: case 0x65u:
    case 0x66u: case 0x67u: case 0xf0u: case 0xf2u: case 0xf3u:
        return 1;
    default:
        return 0;
    }
}

static type_unsigned_8 model40_capture_opcode_index(
    const core_machine_retirement_observation *observation)
{
    type_unsigned_8 index = 0u;

    if (observation == STD_NULL) return 0u;
    while (index < observation->point.byte_count &&
        model40_capture_is_prefix(observation->point.bytes[index])) ++index;
    return index;
}

static type_unsigned_8 model40_capture_group_extension(
    const core_machine_retirement_observation *observation, type_unsigned_8 index,
    type_unsigned_8 opcode)
{
    type_unsigned_8 modrm_index = (type_unsigned_8)(index + 1u);

    if (observation == STD_NULL || modrm_index >= observation->point.byte_count) return 0xffu;
    switch (opcode) {
    case 0x80u: case 0x81u: case 0x82u: case 0x83u: case 0xc0u: case 0xc1u:
    case 0xd0u: case 0xd1u: case 0xd2u: case 0xd3u: case 0xf6u: case 0xf7u:
    case 0xfeu: case 0xffu:
        return (type_unsigned_8)((observation->point.bytes[modrm_index] >> 3u) & 7u);
    default:
        return 0xffu;
    }
}
static C_INT model40_capture_form_matches(
    const model40_retirement_capture_form *form, const C_CHAR *name,
    const C_CHAR *operand, type_unsigned_8 opcode, type_unsigned_8 escape_opcode,
    type_unsigned_8 group_extension, type_unsigned_32 source_timing_form_id,
    core_machine_retirement_timing_origin timing_origin,
    const core_machine_retirement_observation *observation)
{
    return form != STD_NULL && observation != STD_NULL &&
        !STD_STRCMP(form->form, name) && !STD_STRCMP(form->operand, operand) &&
        form->opcode == opcode && form->escape_opcode == escape_opcode &&
        form->group_extension == group_extension &&
        form->source_timing_form_id == source_timing_form_id &&
        form->timing_origin == timing_origin &&
        form->modrm_form == observation->modrm_form &&
        form->modrm_extension == observation->modrm_extension &&
        form->control_outcome == observation->control_outcome &&
        form->next_lexeme_components == observation->next_lexeme_components &&
        form->repeat_phase == observation->repeat_phase &&
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
    type_unsigned_8 opcode_index;
    type_unsigned_8 opcode;
    type_unsigned_8 escape_opcode = 0xffu;
    type_unsigned_8 group_extension;

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
    if (!capture->observation_seen) {
        capture->observation_seen = TYPE_TRUE;
    } else if (capture->previous_protected_mode && !observation->protected_mode) {
        capture->checkpoint_reached = TYPE_TRUE;
    } else if (!capture->previous_protected_mode && observation->protected_mode &&
        capture->checkpoint_reached) {
        capture->c1_checkpoint_reached = TYPE_TRUE;
    }
    if (observation->protected_mode) capture->protected_mode_seen = TYPE_TRUE;
    capture->previous_protected_mode = observation->protected_mode;
    opcode_index = model40_capture_opcode_index(observation);
    opcode = opcode_index < observation->point.byte_count ?
        observation->point.bytes[opcode_index] : 0xffu;
    if (opcode == 0x0fu && opcode_index + 1u < observation->point.byte_count) {
        escape_opcode = observation->point.bytes[opcode_index + 1u];
    }
    group_extension = model40_capture_group_extension(observation, opcode_index, opcode);
    name = model40_capture_form_name(observation, opcode_index);
    operand = model40_capture_operand_name(observation);
    for (index = 0u; index < capture->form_count; ++index) {
        if (model40_capture_form_matches(&capture->forms[index], name, operand,
                opcode, escape_opcode, group_extension, observation->source_timing_form_id,
                observation->timing_origin, observation)) {
            ++capture->forms[index].count;
            return;
        }
    }
    if (capture->form_count == MODEL40_CAPTURE_FORM_LIMIT) {
        capture->form_limit_reached = TYPE_TRUE;
        return;
    }
    capture->forms[capture->form_count++] = (model40_retirement_capture_form) {
        name, operand, opcode, escape_opcode, group_extension,
        observation->source_timing_form_id, observation->timing_origin,
        observation->modrm_form, observation->modrm_extension,
        observation->control_outcome, observation->next_lexeme_components,
        observation->repeat_phase,
        observation->source_ticks, observation->cpl,
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

        STD_PRINTF("T390 form=%s operand=%s opcode=%02X escape=%02X group=%u source-form=%u origin=%u modrm=%u modrm-ext=%u control=%u next=%u repeat-phase=%u ticks=%llu cpl=%u pm=%u vm=%u os32=%u "
            "as32=%u lock=%u rep=%u disposition=%u count=%u\n",
            form->form, form->operand, (unsigned)form->opcode,
            (unsigned)form->escape_opcode, (unsigned)form->group_extension,
            (unsigned)form->source_timing_form_id, (unsigned)form->timing_origin,
            (unsigned)form->modrm_form, (unsigned)form->modrm_extension,
            (unsigned)form->control_outcome, (unsigned)form->next_lexeme_components,
            (unsigned)form->repeat_phase,
            (unsigned long long)form->ticks, (unsigned)form->cpl, (unsigned)form->protected_mode,
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
        (argc != 7 && (argc != 8 || (STD_STRCMP(argv[7], "--terminal-bytes") &&
        STD_STRCMP(argv[7], "--c1-diagnostic"))))) return 0;
    config.profile_kind = VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40;
    config.fdd_image = argv[6];
    config.model40_firmware = (vm_profile_model40_byob_manifest) {
        argv[1], argv[2], argv[3], argv[4], argv[5] };
    return vm_session_create(&config, out_session) == TYPE_STATUS_OK &&
        *out_session != STD_NULL;
}

static C_INT model40_capture_synthetic_c0_smoke(C_VOID)
{
    model40_retirement_capture capture = { 0 };
    core_machine_retirement_observation observation = { 0 };

    observation.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    observation.timing_disposition = CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    observation.timing_origin =
        CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80386_FALLBACK;
    observation.point.byte_count = 2u;
    observation.point.bytes[0] = 0x66u;
    observation.point.bytes[1] = 0xb8u;
    observation.source_ticks = 3u;
    model40_capture_observe(&capture, &observation);
    observation.protected_mode = TYPE_TRUE;
    observation.timing_origin =
        CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80386_SECONDARY;
    observation.point.bytes[0] = 0x0fu;
    observation.point.bytes[1] = 0x20u;
    observation.point.byte_count = 2u;
    observation.modrm_form = CORE_MACHINE_RETIREMENT_MODRM_REGISTER;
    observation.modrm_extension = 0u;
    model40_capture_observe(&capture, &observation);
    observation.modrm_extension = 2u;
    model40_capture_observe(&capture, &observation);
    observation.modrm_extension = CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE;
    observation.protected_mode = TYPE_FALSE;
    observation.timing_origin =
        CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK;
    observation.point.bytes[0] = 0xeau;
    observation.point.byte_count = 1u;
    model40_capture_observe(&capture, &observation);
    observation.timing_origin =
        CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY;
    observation.point.bytes[0] = 0x90u;
    model40_capture_observe(&capture, &observation);
    observation.timing_disposition =
        CORE_MACHINE_RETIREMENT_TIMING_SOURCE_UNALLOCATED;
    model40_capture_observe(&capture, &observation);
    observation.timing_disposition = CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    observation.protected_mode = TYPE_TRUE;
    model40_capture_observe(&capture, &observation);
    if (!capture.checkpoint_reached || !capture.c1_checkpoint_reached ||
        capture.count != 7u || capture.classified != 6u ||
        capture.unallocated != 1u || capture.form_count != 7u ||
        STD_STRCMP(capture.forms[0].form, "mov-immediate")) return 1;
    STD_PRINTF("M5:T390:S17:M40-C0-CAPTURE:OK\n");
    STD_PRINTF("M5:T390:S29:M40-C1-DIAGNOSTIC:OK\n");
    STD_PRINTF("M5:T390:S32:C1-TRANSITION:OK\n");
    return 0;
}
C_INT main(C_INT argc, C_CHAR **argv)
{
    if (argc == 2 && argv != STD_NULL &&
        !STD_STRCMP(argv[1], "--synthetic-c0-smoke")) {
        return model40_capture_synthetic_c0_smoke();
    }
    const core_machine_run_budget budget = { 1u, 0u };
    core_machine_retirement_observation_provider provider;
    core_machine_run_result result = { 0 };
    model40_retirement_capture capture = { 0 };
    vm_session *session = STD_NULL;
    type_status status = TYPE_STATUS_OK;
    type_unsigned_32 index;
    type_unsigned_64 elapsed_before_terminal = 0u;
    const C_CHAR *terminal = "retirement-budget-exhausted";
    C_INT emit_terminal_bytes = argc == 8 && argv != STD_NULL &&
        !STD_STRCMP(argv[7], "--terminal-bytes");
    C_INT c1_diagnostic = argc == 8 && argv != STD_NULL &&
        !STD_STRCMP(argv[7], "--c1-diagnostic");

    if (!model40_capture_create_session(argc, argv, &session)) {
        STD_FPRINTF(STD_STDERR, "usage: capture even-image even-digest "
            "odd-image odd-digest provenance floppy-image [--terminal-bytes|--c1-diagnostic]\n");
        return 2;
    }
    provider.callback = model40_capture_observe;
    provider.context = &capture;
    status = core_machine_set_retirement_observation_provider(
        session->core_machine, &provider);
    if (status == TYPE_STATUS_OK) status = core_machine_reset(session->core_machine);
    for (index = 0u; status == TYPE_STATUS_OK && index <
        MODEL40_CAPTURE_RETIREMENT_LIMIT &&
        (!capture.checkpoint_reached || (c1_diagnostic && !capture.c1_checkpoint_reached)) &&
        capture.unallocated == 0u && !capture.form_limit_reached; ++index) {
        elapsed_before_terminal = result.elapsed_ticks;
        status = core_machine_run(session->core_machine, budget, &result);
        if (status != TYPE_STATUS_OK || result.reason == CORE_MACHINE_STOP_FAULT) break;
    }
    if (c1_diagnostic && capture.c1_checkpoint_reached) terminal = "c1-protected-entry";
    else if (c1_diagnostic && capture.unallocated != 0u) terminal =
        "c1-source-timing-unallocated";
    else if (capture.form_limit_reached) terminal = "form-capacity";
    else if (status != TYPE_STATUS_OK) terminal = "run-status";
    else if (result.reason == CORE_MACHINE_STOP_FAULT) terminal = "fault";
    else if (c1_diagnostic) terminal = "c1-retirement-budget-exhausted";
    else if (capture.checkpoint_reached) terminal = "protected-return-c0";
    else if (capture.unallocated != 0u) terminal = "source-timing-unallocated";
    model40_capture_emit(&capture);
    if (emit_terminal_bytes) model40_capture_emit_terminal_bytes(&capture);
    STD_PRINTF("M5:T390:S8:BYOB-BOOT-CAPTURE:terminal=%s count=%u classified=%u "
        "unallocated=%u forms=%u protected=%u checkpoint=%u c1=%u status=%u\n", terminal,
        (unsigned)capture.count, (unsigned)capture.classified,
        (unsigned)capture.unallocated, (unsigned)capture.form_count,
        (unsigned)capture.protected_mode_seen, (unsigned)capture.checkpoint_reached,
        (unsigned)capture.c1_checkpoint_reached, (unsigned)status);
    if (c1_diagnostic) {
        STD_PRINTF("M5:T390:S29:M40-C1-DIAGNOSTIC:terminal=%s unallocated=%u c1=%u "
            "executed=%u ticks=%llu elapsed-before=%llu elapsed=%llu status=%u\n", terminal,
            (unsigned)capture.unallocated, (unsigned)capture.c1_checkpoint_reached,
            (unsigned)result.executed,
            (unsigned long long)elapsed_before_terminal,
            (unsigned long long)result.ticks,
            (unsigned long long)result.elapsed_ticks, (unsigned)status);
    }
    vm_session_destroy(session);
    if (c1_diagnostic) {
        return capture.checkpoint_reached && capture.c1_checkpoint_reached &&
            capture.unallocated == 0u && !capture.form_limit_reached &&
            status == TYPE_STATUS_OK ? 0 : 1;
    }
    return capture.checkpoint_reached && capture.unallocated == 0u &&
        !capture.form_limit_reached && status == TYPE_STATUS_OK ? 0 : 1;
}
