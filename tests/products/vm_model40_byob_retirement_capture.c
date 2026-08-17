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

typedef struct model40_retirement_capture_key {
    core_machine_retirement_eligibility_key value;
    type_unsigned_32 count;
} model40_retirement_capture_key;

typedef struct model40_retirement_capture {
    model40_retirement_capture_form forms[MODEL40_CAPTURE_FORM_LIMIT];
    model40_retirement_capture_key keys[MODEL40_CAPTURE_FORM_LIMIT];
    type_unsigned_32 count;
    type_unsigned_32 classified;
    type_unsigned_32 unallocated;
    type_unsigned_32 form_count;
    type_unsigned_32 key_count;
    type_bool c0a_diagnostic;
    type_bool c0a_collecting;
    type_bool c1_transfer_diagnostic;
    type_bool c1_collecting;
    type_bool c1_transfer_reached;
    type_bool fdc_read_data_reached;
    type_bool fdc_read_data_baseline_valid;
    type_unsigned_64 fdc_terminal_sequence_at_c0a;
    type_unsigned_8 terminal_bytes[CORE_MACHINE_CPU_DIAGNOSTIC_BYTES];
    type_unsigned_8 terminal_byte_count;
    type_bool checkpoint_reached;
    type_bool c1_checkpoint_reached;
    type_bool post_c0_io_seen;
    type_bool post_c0_io_read;
    type_bool post_c0_io_port_known;
    type_unsigned_16 post_c0_io_port;
    type_bool observation_seen;
    type_bool previous_protected_mode;
    type_bool protected_mode_seen;
    type_bool form_limit_reached;
    type_bool key_limit_reached;
    type_bool terminal_bytes_available;
} model40_retirement_capture;

static C_INT model40_capture_key_matches(
    const core_machine_retirement_eligibility_key *left,
    const core_machine_retirement_eligibility_key *right)
{
    return left != STD_NULL && right != STD_NULL &&
        left->cpu_profile == right->cpu_profile &&
        left->timing_origin == right->timing_origin &&
        left->source_timing_form_id == right->source_timing_form_id &&
        left->opcode == right->opcode && left->escape_opcode == right->escape_opcode &&
        left->modrm_form == right->modrm_form &&
        left->modrm_extension == right->modrm_extension &&
        left->control_outcome == right->control_outcome &&
        left->next_lexeme_components == right->next_lexeme_components &&
        left->repeat_phase == right->repeat_phase && left->cpl == right->cpl &&
        left->protected_mode == right->protected_mode &&
        left->virtual_8086_mode == right->virtual_8086_mode &&
        left->operand_size_32 == right->operand_size_32 &&
        left->address_size_32 == right->address_size_32 &&
        left->lock_prefix == right->lock_prefix &&
        left->repeat_prefix == right->repeat_prefix;
}

static C_VOID model40_capture_record_key(model40_retirement_capture *capture,
    const core_machine_retirement_eligibility_key *key)
{
    type_unsigned_32 index;

    if (capture == STD_NULL || key == STD_NULL) return;
    for (index = 0u; index < capture->key_count; ++index) {
        if (model40_capture_key_matches(&capture->keys[index].value, key)) {
            ++capture->keys[index].count;
            return;
        }
    }
    if (capture->key_count == MODEL40_CAPTURE_FORM_LIMIT) {
        capture->key_limit_reached = TYPE_TRUE;
        return;
    }
    capture->keys[capture->key_count].value = *key;
    capture->keys[capture->key_count].count = 1u;
    ++capture->key_count;
}
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

static C_VOID model40_capture_record_post_c0_io(
    model40_retirement_capture *capture,
    const core_machine_retirement_observation *observation)
{
    type_unsigned_8 opcode_index;
    type_unsigned_8 opcode;

    if (capture == STD_NULL || observation == STD_NULL ||
        !capture->checkpoint_reached || capture->post_c0_io_seen) return;
    opcode_index = model40_capture_opcode_index(observation);
    opcode = opcode_index < observation->point.byte_count ?
        observation->point.bytes[opcode_index] : 0xffu;
    if ((opcode == 0xe4u || opcode == 0xe5u || opcode == 0xe6u ||
        opcode == 0xe7u) && opcode_index + 1u < observation->point.byte_count) {
        capture->post_c0_io_seen = TYPE_TRUE;
        capture->post_c0_io_port_known = TYPE_TRUE;
        capture->post_c0_io_port = observation->point.bytes[opcode_index + 1u];
        capture->post_c0_io_read = opcode == 0xe4u || opcode == 0xe5u;
    } else if (opcode == 0xecu || opcode == 0xedu || opcode == 0xeeu ||
        opcode == 0xefu) {
        capture->post_c0_io_seen = TYPE_TRUE;
        capture->post_c0_io_read = opcode == 0xecu || opcode == 0xedu;
    }
}

static type_bool model40_capture_c0a_reached(
    const model40_retirement_capture *capture)
{
    return capture != STD_NULL && capture->checkpoint_reached &&
        capture->post_c0_io_seen && capture->post_c0_io_read &&
        capture->post_c0_io_port_known && capture->post_c0_io_port == 0x0061u;
}

static type_bool model40_capture_has_fdc_read_data(
    const model40_retirement_capture *capture, const vm_session *session)
{
    const core_machine_fdc_terminal_observation *observation;

    if (capture == STD_NULL || session == STD_NULL ||
        !capture->fdc_read_data_baseline_valid ||
        !model40_capture_c0a_reached(capture) ||
        !session->model40_fdc_terminal_observation_valid) return TYPE_FALSE;
    observation = &session->model40_fdc_terminal_observation;
    return observation->sequence > capture->fdc_terminal_sequence_at_c0a &&
        observation->command == 0xe6u && observation->drive == 0u &&
        observation->successful;
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
    type_bool aggregate;

    if (capture == STD_NULL || observation == STD_NULL) return;
    aggregate = capture->c1_transfer_diagnostic ? capture->c1_collecting :
        (!capture->c0a_diagnostic || capture->c0a_collecting);
    if (!aggregate) {
        if (!capture->observation_seen) capture->observation_seen = TYPE_TRUE;
        else if (capture->previous_protected_mode && !observation->protected_mode) {
            capture->checkpoint_reached = TYPE_TRUE;
            capture->c0a_collecting = TYPE_TRUE;
        }
        if (observation->protected_mode) capture->protected_mode_seen = TYPE_TRUE;
        capture->previous_protected_mode = observation->protected_mode;
        if (capture->c1_transfer_diagnostic) {
            model40_capture_record_post_c0_io(capture, observation);
            if (capture->post_c0_io_seen) capture->c1_collecting = TYPE_TRUE;
        }
        return;
    }
    ++capture->count;
    if ((capture->count & 1023u) == 0u) {
        STD_PRINTF("M5:T390:S5:BYOB-BOOT-PROGRESS:count=%u\n",
            (unsigned)capture->count);
        (C_VOID)fflush(STD_STDOUT);
    }
    if (observation->timing_disposition ==
        CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED) {
        ++capture->classified;
        model40_capture_record_key(capture, &observation->eligibility_key);
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
    model40_capture_record_post_c0_io(capture, observation);
    if (capture->c1_transfer_diagnostic &&
        observation->point.linear_pc == MODEL40_BOOT_SECTOR_LINEAR_PC) {
        capture->c1_transfer_reached = TYPE_TRUE;
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
        STD_STRCMP(argv[7], "--c1-diagnostic") &&
        STD_STRCMP(argv[7], "--post-c0-io-diagnostic") &&
        STD_STRCMP(argv[7], "--c0a-diagnostic") &&
        STD_STRCMP(argv[7], "--c1-transfer-diagnostic") &&
        STD_STRCMP(argv[7], "--fdc-read-data-diagnostic"))))) return 0;
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
    observation.point.bytes[0] = 0xe6u;
    observation.point.bytes[1] = 0x84u;
    observation.point.byte_count = 2u;
    model40_capture_observe(&capture, &observation);
    observation.point.bytes[0] = 0x90u;
    model40_capture_observe(&capture, &observation);
    observation.timing_disposition =
        CORE_MACHINE_RETIREMENT_TIMING_SOURCE_UNALLOCATED;
    model40_capture_observe(&capture, &observation);
    observation.timing_disposition = CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    observation.protected_mode = TYPE_TRUE;
    model40_capture_observe(&capture, &observation);
    if (!capture.checkpoint_reached || !capture.c1_checkpoint_reached ||
        !capture.post_c0_io_seen || !capture.post_c0_io_port_known ||
        capture.post_c0_io_read ||
        capture.post_c0_io_port != 0x0084u || capture.count != 8u ||
        capture.classified != 7u || capture.unallocated != 1u ||
        capture.form_count != 8u ||
        STD_STRCMP(capture.forms[0].form, "mov-immediate")) return 1;
    STD_PRINTF("M5:T390:S17:M40-C0-CAPTURE:OK\n");
    STD_PRINTF("M5:T390:S29:M40-C1-DIAGNOSTIC:OK\n");
    STD_PRINTF("M5:T390:S32:C1-TRANSITION:OK\n");
    STD_PRINTF("M5:T390:S33:POST-C0-IO:OK\n");
    return 0;
}
static C_INT model40_capture_synthetic_key_mapping_smoke(C_VOID)
{
    model40_retirement_capture capture = { 0 };
    core_machine_retirement_observation observation = { 0 };

    observation.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    observation.timing_disposition = CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    observation.point.byte_count = 1u;
    observation.point.bytes[0] = 0x90u;
    observation.eligibility_key.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    observation.eligibility_key.timing_origin =
        CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY;
    observation.eligibility_key.source_timing_form_id = 1u;
    observation.eligibility_key.opcode = 0x90u;
    observation.eligibility_key.escape_opcode = 0xffu;
    observation.eligibility_key.modrm_form =
        CORE_MACHINE_RETIREMENT_MODRM_UNAVAILABLE;
    observation.eligibility_key.modrm_extension =
        CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE;
    observation.eligibility_key.next_lexeme_components =
        CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE;
    model40_capture_observe(&capture, &observation);
    model40_capture_observe(&capture, &observation);
    observation.eligibility_key.opcode = 0x91u;
    model40_capture_observe(&capture, &observation);
    if (capture.key_count != 2u || capture.keys[0].count != 2u ||
        capture.keys[1].count != 1u || capture.key_limit_reached) return 1;
    STD_PRINTF("M5:T394:S5:C0-KEY-MAPPING:OK\n");
    return 0;
}
static C_INT model40_capture_synthetic_c0a_smoke(C_VOID)
{
    model40_retirement_capture capture = { 0 };
    core_machine_retirement_observation observation = { 0 };

    capture.c0a_diagnostic = TYPE_TRUE;
    observation.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    observation.timing_disposition = CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    observation.point.byte_count = 2u;
    observation.point.bytes[0] = 0x90u;
    model40_capture_observe(&capture, &observation);
    observation.protected_mode = TYPE_TRUE;
    observation.point.bytes[0] = 0x0fu;
    observation.point.bytes[1] = 0x20u;
    model40_capture_observe(&capture, &observation);
    observation.protected_mode = TYPE_FALSE;
    observation.point.bytes[0] = 0xeau;
    observation.point.byte_count = 1u;
    model40_capture_observe(&capture, &observation);
    observation.point.bytes[0] = 0x90u;
    model40_capture_observe(&capture, &observation);
    observation.point.bytes[0] = 0xffu;
    observation.point.bytes[1] = 0xe0u;
    observation.point.byte_count = 2u;
    model40_capture_observe(&capture, &observation);
    observation.point.bytes[0] = 0xe4u;
    observation.point.bytes[1] = 0x61u;
    model40_capture_observe(&capture, &observation);
    if (!capture.checkpoint_reached || !capture.c0a_collecting ||
        !capture.post_c0_io_seen || !capture.post_c0_io_read ||
        !capture.post_c0_io_port_known || capture.post_c0_io_port != 0x0061u ||
        capture.count != 3u || capture.classified != 3u || capture.unallocated != 0u ||
        capture.form_count != 3u) return 1;
    STD_PRINTF("M5:T391:S2:C0A-CAPTURE:OK\n");
    return 0;
}
static C_INT model40_capture_synthetic_fdc_read_data_smoke(C_VOID)
{
    model40_retirement_capture capture = { 0 };
    vm_session session = { 0 };

    capture.checkpoint_reached = TYPE_TRUE;
    capture.post_c0_io_seen = TYPE_TRUE;
    capture.post_c0_io_read = TYPE_TRUE;
    capture.post_c0_io_port_known = TYPE_TRUE;
    capture.post_c0_io_port = 0x0061u;
    capture.fdc_read_data_baseline_valid = TYPE_TRUE;
    capture.fdc_terminal_sequence_at_c0a = 7u;
    session.model40_fdc_terminal_observation_valid = TYPE_TRUE;
    session.model40_fdc_terminal_observation.sequence = 7u;
    session.model40_fdc_terminal_observation.command = 0xe6u;
    session.model40_fdc_terminal_observation.drive = 0u;
    session.model40_fdc_terminal_observation.successful = TYPE_TRUE;
    if (model40_capture_has_fdc_read_data(&capture, &session)) return 1;
    session.model40_fdc_terminal_observation.sequence = 8u;
    session.model40_fdc_terminal_observation.drive = 1u;
    if (model40_capture_has_fdc_read_data(&capture, &session)) return 1;
    session.model40_fdc_terminal_observation.drive = 0u;
    session.model40_fdc_terminal_observation.successful = TYPE_FALSE;
    if (model40_capture_has_fdc_read_data(&capture, &session)) return 1;
    session.model40_fdc_terminal_observation.successful = TYPE_TRUE;
    if (!model40_capture_has_fdc_read_data(&capture, &session)) return 1;
    session.model40_fdc_terminal_observation_valid = TYPE_FALSE;
    if (model40_capture_has_fdc_read_data(&capture, &session)) return 1;
    STD_PRINTF("M5:T393:S4:FDC-READ-DATA-CAPTURE:OK\n");
    return 0;
}

static C_INT model40_capture_synthetic_c1_transfer_smoke(C_VOID)
{
    model40_retirement_capture capture = { 0 };
    core_machine_retirement_observation observation = { 0 };

    capture.c1_transfer_diagnostic = TYPE_TRUE;
    observation.cpu_profile = CORE_MACHINE_CPU_PROFILE_80386;
    observation.timing_disposition = CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED;
    observation.point.byte_count = 2u;
    observation.point.bytes[0] = 0x90u;
    observation.protected_mode = TYPE_TRUE;
    model40_capture_observe(&capture, &observation);
    observation.protected_mode = TYPE_FALSE;
    model40_capture_observe(&capture, &observation);
    observation.point.bytes[0] = 0xe4u;
    observation.point.bytes[1] = 0x61u;
    model40_capture_observe(&capture, &observation);
    observation.point.bytes[0] = 0x90u;
    observation.point.byte_count = 1u;
    observation.point.linear_pc = 0x00001000u;
    model40_capture_observe(&capture, &observation);
    observation.point.linear_pc = MODEL40_BOOT_SECTOR_LINEAR_PC;
    model40_capture_observe(&capture, &observation);
    if (!capture.checkpoint_reached || !capture.post_c0_io_seen ||
        !capture.c1_collecting || !capture.c1_transfer_reached ||
        capture.count != 2u || capture.classified != 2u ||
        capture.unallocated != 0u || capture.form_count != 1u) return 1;
    STD_PRINTF("M5:T391:S5:C1-TRANSFER-CAPTURE:OK\n");
    return 0;
}
C_INT main(C_INT argc, C_CHAR **argv)
{
    if (argc == 2 && argv != STD_NULL &&
        !STD_STRCMP(argv[1], "--synthetic-c0-smoke")) {
        return model40_capture_synthetic_c0_smoke() ||
            model40_capture_synthetic_key_mapping_smoke() ||
            model40_capture_synthetic_c0a_smoke() ||
            model40_capture_synthetic_fdc_read_data_smoke() ||
            model40_capture_synthetic_c1_transfer_smoke();
    }
    if (argc == 2 && argv != STD_NULL &&
        !STD_STRCMP(argv[1], "--synthetic-c0a-smoke")) {
        return model40_capture_synthetic_c0a_smoke();
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
    C_INT post_c0_io_diagnostic = argc == 8 && argv != STD_NULL &&
        !STD_STRCMP(argv[7], "--post-c0-io-diagnostic");
    C_INT c0a_diagnostic = argc == 8 && argv != STD_NULL &&
        !STD_STRCMP(argv[7], "--c0a-diagnostic");
    C_INT c1_transfer_diagnostic = argc == 8 && argv != STD_NULL &&
        !STD_STRCMP(argv[7], "--c1-transfer-diagnostic");
    C_INT fdc_read_data_diagnostic = argc == 8 && argv != STD_NULL &&
        !STD_STRCMP(argv[7], "--fdc-read-data-diagnostic");

    if (!model40_capture_create_session(argc, argv, &session)) {
        STD_FPRINTF(STD_STDERR, "usage: capture even-image even-digest "
            "odd-image odd-digest provenance floppy-image [--terminal-bytes|--c1-diagnostic|--post-c0-io-diagnostic|--c0a-diagnostic|--c1-transfer-diagnostic]\n");
        return 2;
    }
    capture.c0a_diagnostic = c0a_diagnostic != 0;
    capture.c1_transfer_diagnostic = c1_transfer_diagnostic != 0;
    provider.callback = model40_capture_observe;
    provider.context = &capture;
    status = core_machine_set_retirement_observation_provider(
        session->core_machine, &provider);
    if (status == TYPE_STATUS_OK) status = core_machine_reset(session->core_machine);
    for (index = 0u; status == TYPE_STATUS_OK && index <
        MODEL40_CAPTURE_RETIREMENT_LIMIT &&
        (!capture.checkpoint_reached || (c1_diagnostic && !capture.c1_checkpoint_reached) ||
        (c1_transfer_diagnostic && !capture.c1_transfer_reached) ||
        (fdc_read_data_diagnostic && !capture.fdc_read_data_reached) ||
        ((post_c0_io_diagnostic || c0a_diagnostic) && !capture.post_c0_io_seen)) &&
        capture.unallocated == 0u && !capture.form_limit_reached; ++index) {
        elapsed_before_terminal = result.elapsed_ticks;
        status = core_machine_run(session->core_machine, budget, &result);
        if (status != TYPE_STATUS_OK || result.reason == CORE_MACHINE_STOP_FAULT) break;
        if (fdc_read_data_diagnostic && !capture.fdc_read_data_baseline_valid &&
            model40_capture_c0a_reached(&capture)) {
            capture.fdc_read_data_baseline_valid = TYPE_TRUE;
            if (session->model40_fdc_terminal_observation_valid) {
                capture.fdc_terminal_sequence_at_c0a =
                    session->model40_fdc_terminal_observation.sequence;
            }
        } else if (fdc_read_data_diagnostic &&
            model40_capture_has_fdc_read_data(&capture, session)) {
            capture.fdc_read_data_reached = TYPE_TRUE;
        }
    }
    if (fdc_read_data_diagnostic && capture.fdc_read_data_reached) terminal = "fdc-read-data";
    else if (c1_transfer_diagnostic && capture.c1_transfer_reached) terminal = "c1-boot-transfer";
    else if ((post_c0_io_diagnostic || c0a_diagnostic) && capture.post_c0_io_seen) terminal = "post-c0-io";
    else if (c1_diagnostic && capture.c1_checkpoint_reached) terminal = "c1-protected-entry";
    else if (c1_diagnostic && capture.unallocated != 0u) terminal =
        "c1-source-timing-unallocated";
    else if (capture.form_limit_reached) terminal = "form-capacity";
    else if (status != TYPE_STATUS_OK) terminal = "run-status";
    else if (result.reason == CORE_MACHINE_STOP_FAULT) terminal = "fault";
    else if (fdc_read_data_diagnostic) terminal = "fdc-read-data-retirement-budget-exhausted";
    else if (c1_transfer_diagnostic) terminal = "c1-transfer-retirement-budget-exhausted";
    else if (c1_diagnostic) terminal = "c1-retirement-budget-exhausted";
    else if (capture.checkpoint_reached) terminal = "protected-return-c0";
    else if (capture.unallocated != 0u) terminal = "source-timing-unallocated";
    model40_capture_emit(&capture);
    STD_PRINTF("M5:T394:S5:C0-KEY-MAPPING:forms=%u keys=%u key-limit=%u\n",
        (unsigned)capture.form_count, (unsigned)capture.key_count,
        (unsigned)capture.key_limit_reached);
    if (emit_terminal_bytes) model40_capture_emit_terminal_bytes(&capture);
    STD_PRINTF("M5:T390:S8:BYOB-BOOT-CAPTURE:terminal=%s count=%u classified=%u "
        "unallocated=%u forms=%u protected=%u checkpoint=%u c1=%u status=%u\n", terminal,
        (unsigned)capture.count, (unsigned)capture.classified,
        (unsigned)capture.unallocated, (unsigned)capture.form_count,
        (unsigned)capture.protected_mode_seen, (unsigned)capture.checkpoint_reached,
        (unsigned)capture.c1_checkpoint_reached, (unsigned)status);
    if (post_c0_io_diagnostic) {
        STD_PRINTF("M5:T390:S33:POST-C0-IO:terminal=%s port-known=%u port=%04X read=%u "
            "unallocated=%u status=%u\n", terminal, (unsigned)capture.post_c0_io_port_known,
            (unsigned)capture.post_c0_io_port, (unsigned)capture.post_c0_io_read,
            (unsigned)capture.unallocated, (unsigned)status);
    }
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
    if (post_c0_io_diagnostic) {
        return capture.checkpoint_reached && capture.post_c0_io_seen &&
            capture.unallocated == 0u && !capture.form_limit_reached &&
            status == TYPE_STATUS_OK ? 0 : 1;
    }
    if (fdc_read_data_diagnostic) {
        return model40_capture_c0a_reached(&capture) &&
            capture.fdc_read_data_baseline_valid && capture.fdc_read_data_reached &&
            capture.unallocated == 0u && !capture.form_limit_reached &&
            status == TYPE_STATUS_OK ? 0 : 1;
    }
    if (c1_transfer_diagnostic) {
        return capture.checkpoint_reached && capture.post_c0_io_seen &&
            capture.c1_transfer_reached && capture.unallocated == 0u &&
            !capture.form_limit_reached && status == TYPE_STATUS_OK ? 0 : 1;
    }
    if (c1_diagnostic) {
        return capture.checkpoint_reached && capture.c1_checkpoint_reached &&
            capture.unallocated == 0u && !capture.form_limit_reached &&
            status == TYPE_STATUS_OK ? 0 : 1;
    }
    return capture.checkpoint_reached && capture.unallocated == 0u &&
        !capture.form_limit_reached && status == TYPE_STATUS_OK ? 0 : 1;
}
