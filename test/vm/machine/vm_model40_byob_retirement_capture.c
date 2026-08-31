#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/machine.h"
#include "core/machine/retirement_observation_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/waiting.h"

#define MODEL40_CAPTURE_FORM_LIMIT 128u
/* DeskPro firmware performs a complete multi-pass RAM verification before its
 * first floppy request.  This remains finite, but the former ten-million
 * retirement budget cut through a normal 2 MiB verification pass. */
#define MODEL40_CAPTURE_RETIREMENT_LIMIT 100000000u
#define MODEL40_CAPTURE_RUN_INSTRUCTIONS 1024u
#define MODEL40_CAPTURE_RUN_LIMIT \
    ((MODEL40_CAPTURE_RETIREMENT_LIMIT + MODEL40_CAPTURE_RUN_INSTRUCTIONS - 1u) / \
        MODEL40_CAPTURE_RUN_INSTRUCTIONS)
#define MODEL40_BOOT_SECTOR_LINEAR_PC 0x00007c00u
#define MODEL40_CAPTURE_POST_C0_HISTORY 32u
#define MODEL40_CAPTURE_D4_TIMER_HISTORY 32u
#define MODEL40_CAPTURE_D4_MEMORY_HISTORY 8u
#define MODEL40_CAPTURE_RESET_HISTORY 4u

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

typedef struct model40_capture_execution_point {
    type_unsigned_32 linear_pc;
    type_bool protected_mode;
} model40_capture_execution_point;

typedef struct model40_capture_port_event {
    type_unsigned_64 sequence;
    type_unsigned_32 linear_pc;
    type_unsigned_16 port;
    type_unsigned_8 value;
    type_bool write;
} model40_capture_port_event;

typedef struct model40_retirement_capture {
    core_machine *machine;
    model40_retirement_capture_form forms[MODEL40_CAPTURE_FORM_LIMIT];
    model40_retirement_capture_key keys[MODEL40_CAPTURE_FORM_LIMIT];
    type_unsigned_32 count;
    type_unsigned_32 classified;
    type_unsigned_32 coprocessor_domain;
    type_unsigned_32 unallocated;
    type_unsigned_32 form_count;
    type_unsigned_32 key_count;
    type_bool c0a_diagnostic;
    type_bool c0a_collecting;
    type_bool c1_transfer_diagnostic;
    type_bool d4_timer_history_enabled;
    type_bool d4_failsafe_test_seen;
    type_bool d4_memory_pass_seen;
    type_unsigned_32 d4_memory_iteration_count;
    type_unsigned_32 d4_memory_ebp[MODEL40_CAPTURE_D4_MEMORY_HISTORY];
    type_unsigned_32 d4_memory_next_pc[MODEL40_CAPTURE_D4_MEMORY_HISTORY];
    type_unsigned_8 d4_failsafe_port_value;
    type_bool c1_collecting;
    type_bool c1_transfer_reached;
    type_bool fdc_read_data_reached;
    type_bool fdc_port_seen;
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
    model40_capture_port_event d4_timer_ports[MODEL40_CAPTURE_D4_TIMER_HISTORY];
    type_unsigned_32 d4_timer_port_count;
    type_bool observation_seen;
    type_bool previous_protected_mode;
    type_bool protected_mode_seen;
    type_bool form_catalog_truncated;
    type_bool key_limit_reached;
    type_bool terminal_bytes_available;
    type_bool iret_frame_seen;
    type_unsigned_32 iret_frame_count;
    type_bool iret_frame_read;
    type_bool iret_frame_stopped_read;
    type_unsigned_16 iret_ss;
    type_unsigned_16 iret_sp;
    type_unsigned_32 iret_ss_base;
    type_unsigned_16 iret_frame[3];
    type_unsigned_32 iret_frame_address;
    type_unsigned_16 iret_frame_stopped[3];
    type_bool iret_operand_size_32;
    type_bool iret_stack_size_32;
    type_bool nmi_entry_seen;
    type_unsigned_32 nmi_entry_count;
    type_bool nmi_entry_frame_read;
    type_unsigned_16 nmi_entry_ss;
    type_unsigned_16 nmi_entry_sp;
    type_unsigned_32 nmi_entry_ss_base;
    type_unsigned_16 nmi_entry_frame[3];
    type_unsigned_16 nmi_entry_source_cs;
    type_unsigned_16 nmi_entry_source_ip;
    type_unsigned_16 nmi_entry_source_flags;
    type_unsigned_32 nmi_entry_source_cr0;
    type_bool nmi_entry_vector2_read;
    type_unsigned_16 nmi_entry_vector2[2];
    type_unsigned_32 iret_cr0;
    type_bool reset_vector2_read;
    type_unsigned_16 reset_vector2[2];
    type_bool iret_route_seen;
    type_unsigned_32 iret_route_trace[32];
    type_unsigned_8 iret_route_opcode[32];
    type_unsigned_8 iret_route_trace_count;
    type_bool interrupt_handler_entry_seen;
    type_unsigned_32 interrupt_handler_entry_count;
    type_bool interrupt_handler_frame_read;
    type_unsigned_16 interrupt_handler_ss;
    type_unsigned_16 interrupt_handler_sp;
    type_unsigned_16 interrupt_handler_frame[3];
    type_unsigned_8 interrupt_handler_vector;
    type_bool interrupt_handler_vector_found;
    type_unsigned_16 interrupt_handler_source_cs;
    type_unsigned_16 interrupt_handler_source_ip;
    type_unsigned_16 interrupt_handler_source_sp;
    type_unsigned_8 interrupt_handler_ivt_vectors[16];
    type_unsigned_16 interrupt_handler_ivt_offsets[16];
    type_unsigned_8 interrupt_handler_ivt_count;
    type_unsigned_32 interrupt_handler_trace[32];
    type_unsigned_8 interrupt_handler_trace_opcode[32];
    type_unsigned_8 interrupt_handler_trace_count;
    type_bool interrupt_scan_entry_seen;
    type_unsigned_16 interrupt_scan_source_cs;
    type_unsigned_16 interrupt_scan_source_ip;
    type_unsigned_16 interrupt_scan_source_sp;
    type_unsigned_16 interrupt_scan_ss;
    type_unsigned_16 interrupt_scan_sp;
    type_bool interrupt_service_entry_seen;
    type_unsigned_32 interrupt_service_entry_count;
    type_bool interrupt_service_frame_read;
    type_unsigned_16 interrupt_service_ss;
    type_unsigned_16 interrupt_service_sp;
    type_unsigned_16 interrupt_service_source_cs;
    type_unsigned_16 interrupt_service_source_ip;
    type_unsigned_16 interrupt_service_source_sp;
    type_unsigned_16 interrupt_service_frame[3];
    type_unsigned_32 interrupt_service_trace[32];
    type_unsigned_8 interrupt_service_trace_opcode[32];
    type_unsigned_8 interrupt_service_trace_count;
    type_bool low_stack_transition_seen;
    type_unsigned_32 low_stack_transition_pc;
    type_unsigned_16 low_stack_transition_before;
    type_unsigned_16 low_stack_transition_after;
    type_unsigned_32 low_stack_transition_trace[32];
    type_unsigned_8 low_stack_transition_trace_opcode[32];
    type_unsigned_8 low_stack_transition_trace_count;
    type_bool stack_exhaustion_seen;
    type_unsigned_32 stack_exhaustion_pc;
    type_unsigned_16 stack_exhaustion_before;
    type_unsigned_16 stack_exhaustion_after;
    type_unsigned_32 stack_exhaustion_trace[32];
    type_unsigned_8 stack_exhaustion_trace_opcode[32];
    type_unsigned_8 stack_exhaustion_trace_count;
    type_bool minimum_stack_seen;
    type_unsigned_16 minimum_stack_value;
    type_unsigned_32 minimum_stack_pc;
    type_unsigned_32 minimum_stack_trace[32];
    type_unsigned_8 minimum_stack_trace_opcode[32];
    type_unsigned_8 minimum_stack_trace_count;
    type_bool last_io_valid;
    type_unsigned_32 last_io_pc;
    type_unsigned_16 last_io_port;
    type_unsigned_32 last_io_value;
    type_unsigned_8 last_io_direction;
    type_bool reset_vector_seen;
    type_unsigned_32 reset_vector_count;
    type_unsigned_8 reset_vector_shutdown_status[MODEL40_CAPTURE_RESET_HISTORY];
    type_unsigned_8 reset_vector_kbc_output_port[MODEL40_CAPTURE_RESET_HISTORY];
    type_bool reset_vector_io_valid;
    type_unsigned_32 reset_vector_io_pc;
    type_unsigned_16 reset_vector_io_port;
    type_unsigned_32 reset_vector_io_value;
    type_unsigned_8 reset_vector_io_direction;
    type_bool reset_instruction_seen;
    type_unsigned_8 reset_instruction_port;
    type_unsigned_8 reset_instruction_value;
    type_bool reset_instruction_state_seen;
    type_unsigned_8 reset_instruction_shutdown_status;
    type_unsigned_8 reset_instruction_kbc_output_port;
    type_bool last_software_interrupt_valid;
    type_unsigned_32 last_software_interrupt_pc;
    type_unsigned_8 last_software_interrupt_vector;
    type_unsigned_16 last_software_interrupt_ss;
    type_unsigned_16 last_software_interrupt_sp;
    type_unsigned_32 last_software_interrupt_target;
    type_unsigned_8 last_software_interrupt_target_bytes[4];
    type_bool last_software_interrupt_target_read;
    type_bool last_software_interrupt_target_stopped_read;
    type_unsigned_8 last_software_interrupt_target_stopped_bytes[4];
    type_unsigned_32 software_interrupt_trace[32];
    type_unsigned_8 software_interrupt_trace_count;
    type_unsigned_32 recent_linear_pc[32];
    type_unsigned_8 recent_opcode[32];
    type_unsigned_8 recent_count;
    type_unsigned_8 recent_next;
    type_bool zero_code_seen;
    type_unsigned_32 zero_code_trace[32];
    type_unsigned_8 zero_code_opcode[32];
    type_unsigned_8 zero_code_trace_count;
    type_unsigned_32 iret_io_pc[8];
    type_unsigned_16 iret_io_port[8];
    type_unsigned_32 iret_io_value[8];
    type_unsigned_8 iret_io_direction[8];
    type_unsigned_8 iret_io_count;
    model40_capture_execution_point post_c0_first[MODEL40_CAPTURE_POST_C0_HISTORY];
    type_unsigned_8 post_c0_first_count;
    model40_capture_execution_point post_c0_last[MODEL40_CAPTURE_POST_C0_HISTORY];
    type_unsigned_32 post_c0_last_count;
    model40_capture_port_event post_c0_ports[MODEL40_CAPTURE_POST_C0_HISTORY];
    type_unsigned_8 post_c0_port_count;
    type_bool post_c0_previous_valid;
    model40_capture_execution_point post_c0_previous;
    type_bool post_c0_non_rom_seen;
    model40_capture_execution_point post_c0_non_rom_predecessor;
    model40_capture_execution_point post_c0_non_rom;
} model40_retirement_capture;

static C_VOID model40_capture_record_post_c0_point(
    model40_retirement_capture *capture,
    const core_machine_retirement_observation *observation)
{
    type_unsigned_32 index;

    if (capture == STD_NULL || observation == STD_NULL || !capture->checkpoint_reached) {
        return;
    }
    if (!capture->post_c0_non_rom_seen && observation->point.linear_pc < 0x000f0000u) {
        capture->post_c0_non_rom_seen = TYPE_TRUE;
        capture->post_c0_non_rom = (model40_capture_execution_point) {
            observation->point.linear_pc, observation->protected_mode };
        if (capture->post_c0_previous_valid) {
            capture->post_c0_non_rom_predecessor = capture->post_c0_previous;
        }
    }
    if (capture->post_c0_first_count < MODEL40_CAPTURE_POST_C0_HISTORY) {
        capture->post_c0_first[capture->post_c0_first_count++] =
            (model40_capture_execution_point) { observation->point.linear_pc,
                observation->protected_mode };
    }
    index = capture->post_c0_last_count % MODEL40_CAPTURE_POST_C0_HISTORY;
    capture->post_c0_last[index] = (model40_capture_execution_point) {
        observation->point.linear_pc, observation->protected_mode };
    ++capture->post_c0_last_count;
    capture->post_c0_previous = (model40_capture_execution_point) {
        observation->point.linear_pc, observation->protected_mode };
    capture->post_c0_previous_valid = TYPE_TRUE;
}

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

static type_bool model40_capture_is_coprocessor_escape(
    const core_machine_retirement_observation *observation,
    type_unsigned_8 opcode_index)
{
    type_unsigned_8 opcode;

    if (observation == STD_NULL || opcode_index >= observation->point.byte_count) {
        return TYPE_FALSE;
    }
    opcode = observation->point.bytes[opcode_index];
    return opcode >= 0xd8u && opcode <= 0xdfu;
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
    if (capture == STD_NULL || observation == STD_NULL ||
        !capture->checkpoint_reached ||
        observation->io_direction == CORE_MACHINE_RETIREMENT_IO_NONE) return;
    if (!capture->post_c0_io_seen) {
        capture->post_c0_io_seen = TYPE_TRUE;
        capture->post_c0_io_port_known = TYPE_TRUE;
        capture->post_c0_io_port = observation->io_port;
        capture->post_c0_io_read =
            observation->io_direction == CORE_MACHINE_RETIREMENT_IO_READ;
    }
    if ((observation->io_port >= 0x03f2u && observation->io_port <= 0x03f5u) ||
        observation->io_port == 0x03f7u) capture->fdc_port_seen = TYPE_TRUE;
    if (capture->post_c0_port_count < MODEL40_CAPTURE_POST_C0_HISTORY) {
        capture->post_c0_ports[capture->post_c0_port_count++] =
            (model40_capture_port_event) { observation->sequence,
                observation->point.linear_pc, observation->io_port,
                (type_unsigned_8)observation->io_value,
                observation->io_direction == CORE_MACHINE_RETIREMENT_IO_WRITE };
    }
}

/* The D4 ROM's fail-safe POST loop is meaningful only relative to writes that
 * program the second 8254 and enable its latch.  Keep this as copied probe
 * evidence; it does not alter the guest or manufacture an interrupt. */
static C_VOID model40_capture_record_d4_timer_io(
    model40_retirement_capture *capture,
    const core_machine_retirement_observation *observation)
{
    type_bool selected;

    if (capture == STD_NULL || observation == STD_NULL ||
        !capture->d4_timer_history_enabled ||
        observation->io_direction == CORE_MACHINE_RETIREMENT_IO_NONE) return;
    selected = observation->io_port >= 0x0048u && observation->io_port <= 0x004bu;
    if (!selected && !(observation->io_port == 0x0061u &&
            observation->io_direction == CORE_MACHINE_RETIREMENT_IO_WRITE)) return;
    if (capture->d4_timer_port_count < MODEL40_CAPTURE_D4_TIMER_HISTORY) {
        capture->d4_timer_ports[capture->d4_timer_port_count++] =
            (model40_capture_port_event) { observation->sequence,
                observation->point.linear_pc, observation->io_port,
                (type_unsigned_8)observation->io_value,
                observation->io_direction == CORE_MACHINE_RETIREMENT_IO_WRITE };
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
    type_bool zero_code_entry;

    if (capture == STD_NULL || observation == STD_NULL) return;
    model40_capture_record_d4_timer_io(capture, observation);
    if (capture->machine != STD_NULL &&
        observation->point.linear_pc == 0x000fd1d8u) {
        type_unsigned_32 sample = capture->d4_memory_iteration_count++;

        if (sample < MODEL40_CAPTURE_D4_MEMORY_HISTORY) {
            capture->d4_memory_ebp[sample] = capture->machine->executor_cpu.data.ebp;
            capture->d4_memory_next_pc[sample] = capture->machine->executor_cpu.data.cs.base +
                capture->machine->executor_cpu.data.eip;
        }
    }
    if (capture->d4_timer_history_enabled &&
        observation->io_direction == CORE_MACHINE_RETIREMENT_IO_WRITE &&
        observation->io_port == 0x0084u && observation->io_value == 0x46u) {
        capture->d4_memory_pass_seen = TYPE_TRUE;
    }
    if (capture->d4_timer_history_enabled && !capture->d4_failsafe_test_seen &&
        observation->point.linear_pc == 0x000fd1b1u &&
        observation->io_direction == CORE_MACHINE_RETIREMENT_IO_READ &&
        observation->io_port == 0x0061u) {
        capture->d4_failsafe_test_seen = TYPE_TRUE;
        capture->d4_failsafe_port_value = (type_unsigned_8)observation->io_value;
    }
    zero_code_entry = !capture->zero_code_seen &&
        observation->point.linear_pc < 0x00001000u &&
        observation->point.byte_count != 0u && observation->point.bytes[0u] == 0u &&
        (capture->recent_count == 0u || observation->point.linear_pc !=
            capture->recent_linear_pc[(capture->recent_next +
                (sizeof(capture->recent_linear_pc) /
                sizeof(capture->recent_linear_pc[0u])) - 1u) %
                (sizeof(capture->recent_linear_pc) /
                sizeof(capture->recent_linear_pc[0u]))] + 2u);
    capture->recent_linear_pc[capture->recent_next] = observation->point.linear_pc;
    capture->recent_opcode[capture->recent_next] =
        observation->point.byte_count == 0u ? 0xffu : observation->point.bytes[0u];
    capture->recent_next = (capture->recent_next + 1u) %
        (sizeof(capture->recent_linear_pc) / sizeof(capture->recent_linear_pc[0u]));
    if (capture->recent_count < sizeof(capture->recent_linear_pc) /
            sizeof(capture->recent_linear_pc[0u])) ++capture->recent_count;
    if (!capture->iret_route_seen && observation->point.linear_pc == 0x000f0081u) {
        for (index = 0u; index < capture->recent_count; ++index) {
            type_unsigned_8 recent_index = (capture->recent_next + index) %
                (sizeof(capture->recent_linear_pc) /
                sizeof(capture->recent_linear_pc[0u]));
            capture->iret_route_trace[index] = capture->recent_linear_pc[recent_index];
            capture->iret_route_opcode[index] = capture->recent_opcode[recent_index];
        }
        capture->iret_route_trace_count = capture->recent_count;
        capture->iret_route_seen = TYPE_TRUE;
    }
    if (zero_code_entry) {
        for (index = 0u; index < capture->recent_count; ++index) {
            type_unsigned_8 recent_index = (capture->recent_next + index) %
                (sizeof(capture->recent_linear_pc) /
                sizeof(capture->recent_linear_pc[0u]));
            capture->zero_code_trace[index] = capture->recent_linear_pc[recent_index];
            capture->zero_code_opcode[index] = capture->recent_opcode[recent_index];
        }
        capture->zero_code_trace_count = capture->recent_count;
        capture->zero_code_seen = TYPE_TRUE;
    }
    if (capture->machine != STD_NULL &&
        observation->point.linear_pc == 0x000f57a6u) {
        capture->iret_frame_seen = TYPE_TRUE;
        ++capture->iret_frame_count;
        capture->iret_ss = capture->machine->executor_cpu_instructions.
            data.oldcpu.data.ss.selector;
        capture->iret_sp = (type_unsigned_16)capture->machine->
            executor_cpu_instructions.data.oldcpu.data.esp;
        capture->iret_ss_base = capture->machine->executor_cpu_instructions.
            data.oldcpu.data.ss.base;
        capture->iret_operand_size_32 = observation->operand_size_32;
        capture->iret_stack_size_32 = capture->machine->executor_cpu_instructions.
            data.oldcpu.data.ss.seg.data.big;
        capture->iret_cr0 = capture->machine->executor_cpu_instructions.
            data.oldcpu.data.cr0;
        capture->iret_frame_address = capture->machine->executor_cpu.data.ss.base +
            capture->iret_sp;
        if (core_machine_memory_read(capture->machine,
                capture->iret_frame_address,
                capture->iret_frame, sizeof(capture->iret_frame)) == TYPE_STATUS_OK) {
            capture->iret_frame_read = TYPE_TRUE;
        }
    }
    if (capture->machine != STD_NULL && !capture->interrupt_scan_entry_seen &&
        observation->point.linear_pc == 0x000f004du &&
        (capture->machine->executor_cpu_instructions.data.oldcpu.data.cs.base +
            capture->machine->executor_cpu_instructions.data.oldcpu.data.eip <
            0x000f004du ||
        capture->machine->executor_cpu_instructions.data.oldcpu.data.cs.base +
            capture->machine->executor_cpu_instructions.data.oldcpu.data.eip >
            0x000f0054u)) {
        capture->interrupt_scan_entry_seen = TYPE_TRUE;
        capture->interrupt_scan_source_cs = capture->machine->executor_cpu_instructions.
            data.oldcpu.data.cs.selector;
        capture->interrupt_scan_source_ip = (type_unsigned_16)capture->machine->
            executor_cpu_instructions.data.oldcpu.data.eip;
        capture->interrupt_scan_source_sp = (type_unsigned_16)capture->machine->
            executor_cpu_instructions.data.oldcpu.data.esp;
        capture->interrupt_scan_ss = capture->machine->executor_cpu.data.ss.selector;
        capture->interrupt_scan_sp = (type_unsigned_16)capture->machine->executor_cpu.data.esp;
    }
    if (capture->machine != STD_NULL &&
        observation->point.linear_pc == 0x000f001fu) {
        capture->interrupt_service_entry_seen = TYPE_TRUE;
        ++capture->interrupt_service_entry_count;
        capture->interrupt_service_ss = capture->machine->executor_cpu_instructions.
            data.oldcpu.data.ss.selector;
        capture->interrupt_service_sp = (type_unsigned_16)capture->machine->
            executor_cpu_instructions.data.oldcpu.data.esp;
        capture->interrupt_service_source_cs = capture->machine->
            executor_cpu_instructions.data.oldcpu.data.cs.selector;
        capture->interrupt_service_source_ip = (type_unsigned_16)capture->machine->
            executor_cpu_instructions.data.oldcpu.data.eip;
        capture->interrupt_service_source_sp = (type_unsigned_16)capture->machine->
            executor_cpu_instructions.data.oldcpu.data.esp;
        for (index = 0u; index < capture->recent_count; ++index) {
            type_unsigned_8 recent_index = (capture->recent_next + index) %
                (sizeof(capture->recent_linear_pc) /
                sizeof(capture->recent_linear_pc[0u]));

            capture->interrupt_service_trace[index] =
                capture->recent_linear_pc[recent_index];
            capture->interrupt_service_trace_opcode[index] =
                capture->recent_opcode[recent_index];
        }
        capture->interrupt_service_trace_count = capture->recent_count;
    }
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
    opcode_index = model40_capture_opcode_index(observation);
    if (observation->timing_disposition ==
        CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED) {
        ++capture->classified;
        model40_capture_record_key(capture, &observation->eligibility_key);
    } else if (model40_capture_is_coprocessor_escape(observation, opcode_index)) {
        /* ESC is a successful CPU-to-coprocessor boundary, not an unallocated
         * scalar CPU-time form.  The FPU owner supplies its own timing range. */
        ++capture->coprocessor_domain;
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
    model40_capture_record_post_c0_point(capture, observation);
    if (observation->io_direction != CORE_MACHINE_RETIREMENT_IO_NONE) {
        capture->last_io_valid = TYPE_TRUE;
        capture->last_io_pc = observation->point.linear_pc;
        capture->last_io_port = observation->io_port;
        capture->last_io_value = observation->io_value;
        capture->last_io_direction = (type_unsigned_8)observation->io_direction;
    }
    if (observation->point.linear_pc == 0xfffffff0u) {
        type_unsigned_32 reset_sample = capture->reset_vector_count++;

        if (!capture->reset_vector_seen) {
            capture->reset_vector_seen = TYPE_TRUE;
            capture->reset_vector_io_valid = capture->last_io_valid;
            capture->reset_vector_io_pc = capture->last_io_pc;
            capture->reset_vector_io_port = capture->last_io_port;
            capture->reset_vector_io_value = capture->last_io_value;
            capture->reset_vector_io_direction = capture->last_io_direction;
        }
        if (capture->machine != STD_NULL && reset_sample < MODEL40_CAPTURE_RESET_HISTORY) {
            capture->reset_vector_shutdown_status[reset_sample] =
                capture->machine->shared_rtc.registers[0x0fu];
            capture->reset_vector_kbc_output_port[reset_sample] =
                capture->machine->shared_kbc.data.output_port;
        }
    }
    if (capture->machine != STD_NULL && !capture->reset_instruction_seen &&
        observation->point.linear_pc == 0x000fc2f5u &&
        observation->point.byte_count >= 2u && observation->point.bytes[0u] == 0xe6u) {
        capture->reset_instruction_seen = TYPE_TRUE;
        capture->reset_instruction_port = observation->point.bytes[1u];
        capture->reset_instruction_value = (type_unsigned_8)capture->machine->
            executor_cpu_instructions.data.oldcpu.data.eax;
        capture->reset_instruction_state_seen = TYPE_TRUE;
        capture->reset_instruction_shutdown_status = capture->machine->shared_rtc.registers[0x0fu];
        capture->reset_instruction_kbc_output_port =
            capture->machine->shared_kbc.data.output_port;
    }
    if (capture->machine != STD_NULL && capture->low_stack_transition_seen &&
        (!capture->minimum_stack_seen ||
        (type_unsigned_16)capture->machine->
            executor_cpu.data.esp < capture->minimum_stack_value)) {
        capture->minimum_stack_seen = TYPE_TRUE;
        capture->minimum_stack_value = (type_unsigned_16)capture->machine->
            executor_cpu.data.esp;
        capture->minimum_stack_pc = observation->point.linear_pc;
        for (index = 0u; index < capture->recent_count; ++index) {
            type_unsigned_8 recent_index = (capture->recent_next + index) %
                (sizeof(capture->recent_linear_pc) /
                sizeof(capture->recent_linear_pc[0u]));

            capture->minimum_stack_trace[index] = capture->recent_linear_pc[recent_index];
            capture->minimum_stack_trace_opcode[index] = capture->recent_opcode[recent_index];
        }
        capture->minimum_stack_trace_count = capture->recent_count;
    }
    if (capture->machine != STD_NULL && !capture->low_stack_transition_seen &&
        capture->machine->executor_cpu_instructions.
            data.oldcpu.data.esp > 0x0100u &&
        capture->machine->executor_cpu.data.esp <= 0x0100u) {
        capture->low_stack_transition_seen = TYPE_TRUE;
        capture->low_stack_transition_pc = observation->point.linear_pc;
        capture->low_stack_transition_before = (type_unsigned_16)capture->machine->
            executor_cpu_instructions.data.oldcpu.data.esp;
        capture->low_stack_transition_after = (type_unsigned_16)capture->machine->
            executor_cpu.data.esp;
        for (index = 0u; index < capture->recent_count; ++index) {
            type_unsigned_8 recent_index = (capture->recent_next + index) %
                (sizeof(capture->recent_linear_pc) /
                sizeof(capture->recent_linear_pc[0u]));

            capture->low_stack_transition_trace[index] =
                capture->recent_linear_pc[recent_index];
            capture->low_stack_transition_trace_opcode[index] =
                capture->recent_opcode[recent_index];
        }
        capture->low_stack_transition_trace_count = capture->recent_count;
    }
    if (capture->machine != STD_NULL && !capture->stack_exhaustion_seen &&
        capture->machine->executor_cpu_instructions.
            data.oldcpu.data.esp > 0x0020u &&
        capture->machine->executor_cpu.data.esp <= 0x0020u) {
        capture->stack_exhaustion_seen = TYPE_TRUE;
        capture->stack_exhaustion_pc = observation->point.linear_pc;
        capture->stack_exhaustion_before = (type_unsigned_16)capture->machine->
            executor_cpu_instructions.data.oldcpu.data.esp;
        capture->stack_exhaustion_after = (type_unsigned_16)capture->machine->
            executor_cpu.data.esp;
        for (index = 0u; index < capture->recent_count; ++index) {
            type_unsigned_8 recent_index = (capture->recent_next + index) %
                (sizeof(capture->recent_linear_pc) /
                sizeof(capture->recent_linear_pc[0u]));

            capture->stack_exhaustion_trace[index] =
                capture->recent_linear_pc[recent_index];
            capture->stack_exhaustion_trace_opcode[index] =
                capture->recent_opcode[recent_index];
        }
        capture->stack_exhaustion_trace_count = capture->recent_count;
    }
    if (observation->point.linear_pc >= 0x000f5749u &&
        observation->point.linear_pc <= 0x000f57a6u &&
        observation->io_direction != CORE_MACHINE_RETIREMENT_IO_NONE &&
        capture->iret_io_count < sizeof(capture->iret_io_pc) /
            sizeof(capture->iret_io_pc[0u])) {
        index = capture->iret_io_count++;
        capture->iret_io_pc[index] = observation->point.linear_pc;
        capture->iret_io_port[index] = observation->io_port;
        capture->iret_io_value[index] = observation->io_value;
        capture->iret_io_direction[index] = (type_unsigned_8)observation->io_direction;
    }
    opcode = opcode_index < observation->point.byte_count ?
        observation->point.bytes[opcode_index] : 0xffu;
    if (!capture->last_software_interrupt_valid && opcode == 0xcdu &&
        opcode_index + 1u < observation->point.byte_count) {
        capture->last_software_interrupt_valid = TYPE_TRUE;
        capture->last_software_interrupt_pc = observation->point.linear_pc;
        capture->last_software_interrupt_vector = observation->point.bytes[opcode_index + 1u];
        if (capture->machine != STD_NULL) {
            capture->last_software_interrupt_ss =
                capture->machine->executor_cpu.data.ss.selector;
            capture->last_software_interrupt_sp =
                (type_unsigned_16)capture->machine->executor_cpu.data.esp;
            capture->last_software_interrupt_target =
                capture->machine->executor_cpu.data.cs.base +
                capture->machine->executor_cpu.data.eip;
            capture->last_software_interrupt_target_read =
                core_machine_memory_read_physical(&capture->machine->executor_memory,
                    capture->last_software_interrupt_target,
                    (type_virtual_address)capture->last_software_interrupt_target_bytes,
                    sizeof(capture->last_software_interrupt_target_bytes)) == TYPE_STATUS_OK;
        }
    }
    if (capture->last_software_interrupt_valid &&
        capture->software_interrupt_trace_count <
            sizeof(capture->software_interrupt_trace) /
            sizeof(capture->software_interrupt_trace[0u])) {
        capture->software_interrupt_trace[capture->software_interrupt_trace_count++] =
            observation->point.linear_pc;
    }
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
        capture->form_catalog_truncated = TYPE_TRUE;
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

static C_VOID model40_capture_emit_post_c0_history(
    const model40_retirement_capture *capture)
{
    type_unsigned_32 index;
    type_unsigned_32 start;
    type_unsigned_32 count;

    if (capture == STD_NULL) return;
    STD_PRINTF("M5:T498:S5:POST-C0-HISTORY:first=%u last=%u ports=%u\n",
        (unsigned)capture->post_c0_first_count,
        (unsigned)capture->post_c0_last_count,
        (unsigned)capture->post_c0_port_count);
    if (capture->post_c0_non_rom_seen) {
        STD_PRINTF("M5:T498:S5:POST-C0-ROM-EXIT:from=%08X-pm=%u-to=%08X-pm=%u\n",
            (unsigned)capture->post_c0_non_rom_predecessor.linear_pc,
            (unsigned)capture->post_c0_non_rom_predecessor.protected_mode,
            (unsigned)capture->post_c0_non_rom.linear_pc,
            (unsigned)capture->post_c0_non_rom.protected_mode);
    }
    for (index = 0u; index < capture->post_c0_first_count; ++index) {
        STD_PRINTF("M5:T498:S5:POST-C0-FIRST:pc=%08X-pm=%u\n",
            (unsigned)capture->post_c0_first[index].linear_pc,
            (unsigned)capture->post_c0_first[index].protected_mode);
    }
    count = capture->post_c0_last_count < MODEL40_CAPTURE_POST_C0_HISTORY ?
        capture->post_c0_last_count : MODEL40_CAPTURE_POST_C0_HISTORY;
    start = capture->post_c0_last_count > MODEL40_CAPTURE_POST_C0_HISTORY ?
        capture->post_c0_last_count % MODEL40_CAPTURE_POST_C0_HISTORY : 0u;
    for (index = 0u; index < count; ++index) {
        const model40_capture_execution_point *point = &capture->post_c0_last[
            (start + index) % MODEL40_CAPTURE_POST_C0_HISTORY];

        STD_PRINTF("M5:T498:S5:POST-C0-LAST:pc=%08X-pm=%u\n",
            (unsigned)point->linear_pc, (unsigned)point->protected_mode);
    }
    for (index = 0u; index < capture->post_c0_port_count; ++index) {
        const model40_capture_port_event *event = &capture->post_c0_ports[index];

        STD_PRINTF("M5:T498:S5:POST-C0-PORT:seq=%llu-pc=%08X-port=%04X-%s-value=%02X\n",
            (unsigned long long)event->sequence, (unsigned)event->linear_pc,
            (unsigned)event->port, event->write ? "write" : "read",
            (unsigned)event->value);
    }
}

static C_VOID model40_capture_emit_d4_timer_history(
    const model40_retirement_capture *capture)
{
    type_unsigned_32 index;

    if (capture == STD_NULL || !capture->d4_timer_history_enabled) return;
    STD_PRINTF("M5:T498:S5:D4-TIMER-HISTORY:ports=%u\n",
        (unsigned)capture->d4_timer_port_count);
    for (index = 0u; index < capture->d4_timer_port_count; ++index) {
        const model40_capture_port_event *event = &capture->d4_timer_ports[index];

        STD_PRINTF("M5:T498:S5:D4-TIMER-PORT:seq=%llu-pc=%08X-port=%04X-%s-value=%02X\n",
            (unsigned long long)event->sequence, (unsigned)event->linear_pc,
            (unsigned)event->port, event->write ? "write" : "read",
            (unsigned)event->value);
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
        STD_STRCMP(argv[7], "--fdc-read-data-diagnostic") &&
        STD_STRCMP(argv[7], "--port-sequence-diagnostic") &&
        STD_STRCMP(argv[7], "--d4-memory-diagnostic") &&
        STD_STRCMP(argv[7], "--warm-reset-diagnostic"))))) return 0;
    config.profile_kind = VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40;
    config.fdd_image = argv[6];
    config.model40_firmware = (vm_profile_model40_byob_manifest) {
        .even_path = argv[1], .even_sha256 = argv[2],
        .odd_path = argv[3], .odd_sha256 = argv[4], .provenance = argv[5] };
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
    observation.io_direction = CORE_MACHINE_RETIREMENT_IO_WRITE;
    observation.io_port = 0x0084u;
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
    observation.io_direction = CORE_MACHINE_RETIREMENT_IO_READ;
    observation.io_port = 0x0061u;
    observation.io_value = 0x35u;
    model40_capture_observe(&capture, &observation);
    if (!capture.checkpoint_reached || !capture.c0a_collecting ||
        !capture.post_c0_io_seen || !capture.post_c0_io_read ||
        !capture.post_c0_io_port_known || capture.post_c0_io_port != 0x0061u ||
        capture.post_c0_port_count != 1u || capture.post_c0_ports[0].value != 0x35u ||
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
    observation.io_direction = CORE_MACHINE_RETIREMENT_IO_READ;
    observation.io_port = 0x0061u;
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
    const core_machine_run_budget budget = { MODEL40_CAPTURE_RUN_INSTRUCTIONS, 0u };
    core_machine_retirement_observation_provider provider;
    core_machine_run_result result = { 0 };
    core_machine_cpu_diagnostic diagnostic = { 0 };
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
    C_INT port_sequence_diagnostic = argc == 8 && argv != STD_NULL &&
        !STD_STRCMP(argv[7], "--port-sequence-diagnostic");
    C_INT d4_memory_diagnostic = argc == 8 && argv != STD_NULL &&
        !STD_STRCMP(argv[7], "--d4-memory-diagnostic");
    C_INT warm_reset_diagnostic = argc == 8 && argv != STD_NULL &&
        !STD_STRCMP(argv[7], "--warm-reset-diagnostic");

    if (!model40_capture_create_session(argc, argv, &session)) {
        STD_FPRINTF(STD_STDERR, "usage: capture even-image even-digest "
            "odd-image odd-digest provenance floppy-image [--terminal-bytes|--c1-diagnostic|--post-c0-io-diagnostic|--c0a-diagnostic|--c1-transfer-diagnostic|--fdc-read-data-diagnostic|--port-sequence-diagnostic|--d4-memory-diagnostic|--warm-reset-diagnostic]\n");
        return 2;
    }
    capture.c0a_diagnostic = c0a_diagnostic != 0;
    capture.c1_transfer_diagnostic = c1_transfer_diagnostic != 0;
    capture.d4_timer_history_enabled = port_sequence_diagnostic != 0;
    capture.machine = session->core_machine;
    provider.callback = model40_capture_observe;
    provider.context = &capture;
    status = core_machine_set_retirement_observation_provider(
        session->core_machine, &provider);
    if (status == TYPE_STATUS_OK) status = core_machine_reset(session->core_machine);
    if (status == TYPE_STATUS_OK) {
        capture.reset_vector2_read = core_machine_memory_read(session->core_machine,
            8u, capture.reset_vector2, sizeof(capture.reset_vector2)) == TYPE_STATUS_OK;
        session->core_machine->executor_cpu_instructions.data.flagWW = TYPE_TRUE;
        session->core_machine->executor_cpu_instructions.data.wwLinear = 0x0000001au;
    }
    for (index = 0u; status == TYPE_STATUS_OK && index < MODEL40_CAPTURE_RUN_LIMIT &&
        (!capture.checkpoint_reached || (c1_diagnostic && !capture.c1_checkpoint_reached) ||
        (c1_transfer_diagnostic && !capture.c1_transfer_reached) ||
        (fdc_read_data_diagnostic && !capture.fdc_read_data_reached) ||
        (port_sequence_diagnostic && !capture.d4_memory_pass_seen) ||
        (d4_memory_diagnostic && capture.d4_memory_iteration_count < 3u) ||
        (warm_reset_diagnostic && capture.reset_vector_count < 2u) ||
        ((post_c0_io_diagnostic || c0a_diagnostic) && !capture.post_c0_io_seen)) &&
        !capture.zero_code_seen &&
        capture.unallocated == 0u; ++index) {
        elapsed_before_terminal = result.elapsed_ticks;
        status = core_machine_run(session->core_machine, budget, &result);
        if (status != TYPE_STATUS_OK || result.reason == CORE_MACHINE_STOP_FAULT) break;
        if (result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            C_INT advanced = 0;

            status = vm_session_waiting_advance(session, &result, &advanced);
            if (status != TYPE_STATUS_OK || !advanced) break;
        }
        if (capture.last_software_interrupt_valid &&
            !capture.last_software_interrupt_target_stopped_read) {
            capture.last_software_interrupt_target_stopped_read =
                core_machine_memory_read(session->core_machine,
                    capture.last_software_interrupt_target,
                    capture.last_software_interrupt_target_stopped_bytes,
                    sizeof(capture.last_software_interrupt_target_stopped_bytes)) ==
                    TYPE_STATUS_OK;
        }
        if (capture.iret_frame_seen && !capture.iret_frame_stopped_read) {
            capture.iret_frame_stopped_read = core_machine_memory_read(
                session->core_machine, capture.iret_frame_address,
                capture.iret_frame_stopped, sizeof(capture.iret_frame_stopped)) ==
                TYPE_STATUS_OK;
        }
        if (capture.interrupt_service_entry_seen &&
            !capture.interrupt_service_frame_read) {
            capture.interrupt_service_frame_read = core_machine_memory_read(
                session->core_machine, session->core_machine->executor_cpu_instructions.
                data.oldcpu.data.ss.base + capture.interrupt_service_sp,
                capture.interrupt_service_frame,
                sizeof(capture.interrupt_service_frame)) == TYPE_STATUS_OK;
        }
        {
            core_machine_cpu_state cpu = {0};

            if (core_machine_get_cpu_state(session->core_machine, &cpu) == TYPE_STATUS_OK &&
                cpu.cs_base + cpu.eip == 0x000f1bd0u) {
                capture.nmi_entry_seen = TYPE_TRUE;
                ++capture.nmi_entry_count;
                capture.nmi_entry_ss = session->core_machine->executor_cpu.data.ss.selector;
                capture.nmi_entry_sp = (type_unsigned_16)
                    session->core_machine->executor_cpu.data.esp;
                capture.nmi_entry_ss_base = session->core_machine->
                    executor_cpu.data.ss.base;
                capture.nmi_entry_source_cs = session->core_machine->
                    executor_cpu_instructions.data.oldcpu.data.cs.selector;
                capture.nmi_entry_source_ip = (type_unsigned_16)session->core_machine->
                    executor_cpu_instructions.data.oldcpu.data.eip;
                capture.nmi_entry_source_flags = (type_unsigned_16)session->core_machine->
                    executor_cpu_instructions.data.oldcpu.data.eflags;
                capture.nmi_entry_source_cr0 = session->core_machine->
                    executor_cpu_instructions.data.oldcpu.data.cr0;
                capture.nmi_entry_vector2_read = core_machine_memory_read(
                    session->core_machine, 8u, capture.nmi_entry_vector2,
                    sizeof(capture.nmi_entry_vector2)) == TYPE_STATUS_OK;
                capture.nmi_entry_frame_read = core_machine_memory_read(
                    session->core_machine,
                    session->core_machine->executor_cpu.data.ss.base +
                        capture.nmi_entry_sp,
                    capture.nmi_entry_frame, sizeof(capture.nmi_entry_frame)) ==
                    TYPE_STATUS_OK;
            }
            if (!capture.interrupt_handler_entry_seen &&
                core_machine_get_cpu_state(session->core_machine, &cpu) == TYPE_STATUS_OK &&
                cpu.cs_base + cpu.eip == 0x000f0060u &&
                (session->core_machine->executor_cpu_instructions.data.oldcpu.data.cs.base +
                    session->core_machine->executor_cpu_instructions.data.oldcpu.data.eip <
                    0x000f0060u ||
                session->core_machine->executor_cpu_instructions.data.oldcpu.data.cs.base +
                    session->core_machine->executor_cpu_instructions.data.oldcpu.data.eip >
                    0x000f0067u)) {
                type_unsigned_8 vector;

                capture.interrupt_handler_entry_seen = TYPE_TRUE;
                ++capture.interrupt_handler_entry_count;
                for (vector = 0u; vector < capture.recent_count; ++vector) {
                    type_unsigned_8 recent_index = (capture.recent_next + vector) %
                        (sizeof(capture.recent_linear_pc) /
                        sizeof(capture.recent_linear_pc[0u]));

                    capture.interrupt_handler_trace[vector] =
                        capture.recent_linear_pc[recent_index];
                    capture.interrupt_handler_trace_opcode[vector] =
                        capture.recent_opcode[recent_index];
                }
                capture.interrupt_handler_trace_count = capture.recent_count;
                capture.interrupt_handler_ss = session->core_machine->
                    executor_cpu.data.ss.selector;
                capture.interrupt_handler_sp = (type_unsigned_16)session->core_machine->
                    executor_cpu.data.esp;
                capture.interrupt_handler_source_cs = session->core_machine->
                    executor_cpu_instructions.data.oldcpu.data.cs.selector;
                capture.interrupt_handler_source_ip = (type_unsigned_16)session->core_machine->
                    executor_cpu_instructions.data.oldcpu.data.eip;
                capture.interrupt_handler_source_sp = (type_unsigned_16)session->core_machine->
                    executor_cpu_instructions.data.oldcpu.data.esp;
                capture.interrupt_handler_frame_read = core_machine_memory_read(
                    session->core_machine, session->core_machine->executor_cpu.data.ss.base +
                    capture.interrupt_handler_sp, capture.interrupt_handler_frame,
                    sizeof(capture.interrupt_handler_frame)) == TYPE_STATUS_OK;
                for (vector = 0u; vector != 0xffu; ++vector) {
                    type_unsigned_16 entry[2] = {0};

                    if (core_machine_memory_read(session->core_machine,
                            (type_unsigned_32)vector * 4u, entry, sizeof(entry)) ==
                            TYPE_STATUS_OK && entry[1u] == 0xf000u && entry[0u] <= 0x0060u) {
                        if (entry[0u] == 0x0060u) {
                            capture.interrupt_handler_vector = vector;
                            capture.interrupt_handler_vector_found = TYPE_TRUE;
                        }
                        if (capture.interrupt_handler_ivt_count <
                            sizeof(capture.interrupt_handler_ivt_vectors)) {
                            type_unsigned_8 slot = capture.interrupt_handler_ivt_count++;

                            capture.interrupt_handler_ivt_vectors[slot] = vector;
                            capture.interrupt_handler_ivt_offsets[slot] = entry[0u];
                        }
                    }
                }
            }
        }
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
    else if (capture.zero_code_seen) terminal = "zero-code-entry";
    else if (d4_memory_diagnostic && capture.d4_memory_iteration_count >= 3u) terminal =
        "d4-memory-iterations";
    else if (warm_reset_diagnostic && capture.reset_vector_count >= 2u) terminal =
        "warm-reset";
    else if (port_sequence_diagnostic && capture.d4_memory_pass_seen) terminal =
        "d4-memory-pass";
    else if (port_sequence_diagnostic && capture.post_c0_port_count ==
        MODEL40_CAPTURE_POST_C0_HISTORY) terminal = "post-c0-port-sequence";
    else if (c1_transfer_diagnostic && capture.c1_transfer_reached) terminal = "c1-boot-transfer";
    else if ((post_c0_io_diagnostic || c0a_diagnostic) && capture.post_c0_io_seen) terminal = "post-c0-io";
    else if (c1_diagnostic && capture.c1_checkpoint_reached) terminal = "c1-protected-entry";
    else if (c1_diagnostic && capture.unallocated != 0u) terminal =
        "c1-source-timing-unallocated";
    else if (status != TYPE_STATUS_OK) terminal = "run-status";
    else if (result.reason == CORE_MACHINE_STOP_FAULT) terminal = "fault";
    else if (fdc_read_data_diagnostic) terminal = "fdc-read-data-retirement-budget-exhausted";
    else if (c1_transfer_diagnostic) terminal = "c1-transfer-retirement-budget-exhausted";
    else if (c1_diagnostic) terminal = "c1-retirement-budget-exhausted";
    else if (capture.checkpoint_reached) terminal = "protected-return-c0";
    else if (capture.unallocated != 0u) terminal = "source-timing-unallocated";
    model40_capture_emit(&capture);
    model40_capture_emit_post_c0_history(&capture);
    model40_capture_emit_d4_timer_history(&capture);
    if (d4_memory_diagnostic) {
        type_unsigned_32 sample_count = capture.d4_memory_iteration_count;

        if (sample_count > MODEL40_CAPTURE_D4_MEMORY_HISTORY) {
            sample_count = MODEL40_CAPTURE_D4_MEMORY_HISTORY;
        }
        STD_PRINTF("M5:T498:S5:D4-MEMORY-ITERATIONS:count=%u", (unsigned)
            capture.d4_memory_iteration_count);
        for (index = 0u; index < sample_count; ++index) {
            STD_PRINTF("-ebp=%08X-next=%08X", (unsigned)capture.d4_memory_ebp[index],
                (unsigned)capture.d4_memory_next_pc[index]);
        }
        STD_PRINTF("\n");
    }
    if (capture.d4_failsafe_test_seen) {
        STD_PRINTF("M5:T498:S5:D4-FAILSAFE-TEST:value=%02X\n",
            (unsigned)capture.d4_failsafe_port_value);
    }
    STD_PRINTF("M5:T394:S5:C0-KEY-MAPPING:forms=%u keys=%u key-limit=%u\n",
        (unsigned)capture.form_count, (unsigned)capture.key_count,
        (unsigned)capture.key_limit_reached);
    if (emit_terminal_bytes || capture.unallocated != 0u) {
        model40_capture_emit_terminal_bytes(&capture);
    }
    STD_PRINTF("M5:T390:S8:BYOB-BOOT-CAPTURE:terminal=%s count=%u classified=%u "
        "coprocessor-domain=%u unallocated=%u forms=%u protected=%u checkpoint=%u c1=%u status=%u reason=%u detail=%08X pc=%08X\n", terminal,
        (unsigned)capture.count, (unsigned)capture.classified,
        (unsigned)capture.coprocessor_domain,
        (unsigned)capture.unallocated, (unsigned)capture.form_count,
        (unsigned)capture.protected_mode_seen, (unsigned)capture.checkpoint_reached,
        (unsigned)capture.c1_checkpoint_reached, (unsigned)status,
        (unsigned)result.reason, (unsigned)result.detail, (unsigned)result.linear_pc);
    if (fdc_read_data_diagnostic) {
        core_machine_cpu_state cpu = {0};
        type_unsigned_8 bytes[CORE_MACHINE_CPU_DIAGNOSTIC_BYTES] = {0};
        type_unsigned_32 linear_pc = 0u;

        STD_PRINTF("M5:T498:S5:FDC-PREDECESSOR:c0a=%u baseline=%u port=%u read-data=%u "
            "terminal-observation=%u post-c0-io=%u\n",
            (unsigned)model40_capture_c0a_reached(&capture),
            (unsigned)capture.fdc_read_data_baseline_valid,
            (unsigned)capture.fdc_port_seen,
            (unsigned)capture.fdc_read_data_reached,
            (unsigned)session->model40_fdc_terminal_observation_valid,
            (unsigned)capture.post_c0_io_seen);
        if (core_machine_get_cpu_state(session->core_machine, &cpu) == TYPE_STATUS_OK) {
            linear_pc = cpu.cs_base + cpu.eip;
            if (core_machine_memory_read(session->core_machine, linear_pc, bytes,
                    sizeof(bytes)) == TYPE_STATUS_OK) {
                STD_PRINTF("M5:T498:S5:FDC-PREDECESSOR-PC=%08X-bytes=%02X,%02X,%02X,%02X\n",
                    (unsigned)linear_pc, (unsigned)bytes[0u], (unsigned)bytes[1u],
                    (unsigned)bytes[2u], (unsigned)bytes[3u]);
            }
        }
    }
    if (core_machine_get_cpu_diagnostic(session->core_machine, &diagnostic) ==
        TYPE_STATUS_OK) {
        STD_PRINTF("M5:T498:S5:CPU-EXCEPTION-DIAGNOSTIC:fault-valid=%u-fault-mask=%08X-fault-code=%08X-fault-pc=%08X-fault-bytes=%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X-count=%u-first-valid=%u-first-mask=%08X-first-code=%08X-first-pc=%08X-last-valid=%u-last-mask=%08X-last-code=%08X-last-pc=%08X\n",
            (unsigned)diagnostic.first_fault.valid,
            (unsigned)diagnostic.first_fault.exception_mask,
            (unsigned)diagnostic.first_fault.exception_code,
            (unsigned)diagnostic.first_fault.point.linear_pc,
            (unsigned)diagnostic.first_fault.point.bytes[0u],
            (unsigned)diagnostic.first_fault.point.bytes[1u],
            (unsigned)diagnostic.first_fault.point.bytes[2u],
            (unsigned)diagnostic.first_fault.point.bytes[3u],
            (unsigned)diagnostic.first_fault.point.bytes[4u],
            (unsigned)diagnostic.first_fault.point.bytes[5u],
            (unsigned)diagnostic.first_fault.point.bytes[6u],
            (unsigned)diagnostic.first_fault.point.bytes[7u],
            (unsigned)diagnostic.delivered_exception_count,
            (unsigned)diagnostic.first_delivered_exception.valid,
            (unsigned)diagnostic.first_delivered_exception.exception_mask,
            (unsigned)diagnostic.first_delivered_exception.exception_code,
            (unsigned)diagnostic.first_delivered_exception.point.linear_pc,
            (unsigned)diagnostic.last_delivered_exception.valid,
            (unsigned)diagnostic.last_delivered_exception.exception_mask,
            (unsigned)diagnostic.last_delivered_exception.exception_code,
            (unsigned)diagnostic.last_delivered_exception.point.linear_pc);
    }
    {
        type_unsigned_8 bytes[CORE_MACHINE_CPU_DIAGNOSTIC_BYTES] = {0};

        if (core_machine_memory_read(session->core_machine, result.linear_pc,
                bytes, sizeof(bytes)) == TYPE_STATUS_OK) {
            STD_PRINTF("M5:T498:S5:FINAL-PC:pc=%08X-bytes=%02X,%02X,%02X,%02X,%02X,%02X\n",
                (unsigned)result.linear_pc, (unsigned)bytes[0u],
                (unsigned)bytes[1u], (unsigned)bytes[2u], (unsigned)bytes[3u],
                (unsigned)bytes[4u], (unsigned)bytes[5u]);
        }
    }
    {
        core_machine_cpu_state cpu = {0};

        if (core_machine_get_cpu_state(session->core_machine, &cpu) == TYPE_STATUS_OK) {
            STD_PRINTF("M5:T498:S5:FINAL-CPU:cs=%04X-base=%08X-eip=%08X-flags=%08X-halted=%u\n",
                (unsigned)cpu.cs, (unsigned)cpu.cs_base,
                (unsigned)cpu.eip, (unsigned)cpu.eflags,
                (unsigned)cpu.halted);
        }
    }
    STD_PRINTF("M5:T498:S5:FINAL-INTERNAL-CPU:es=%04X-base=%08X-ds=%04X-base=%08X-ss=%04X-base=%08X-ebx=%08X-eax=%08X-cr0=%08X\n",
        (unsigned)session->core_machine->executor_cpu.data.es.selector,
        (unsigned)session->core_machine->executor_cpu.data.es.base,
        (unsigned)session->core_machine->executor_cpu.data.ds.selector,
        (unsigned)session->core_machine->executor_cpu.data.ds.base,
        (unsigned)session->core_machine->executor_cpu.data.ss.selector,
        (unsigned)session->core_machine->executor_cpu.data.ss.base,
        (unsigned)session->core_machine->executor_cpu.data.ebx,
        (unsigned)session->core_machine->executor_cpu.data.eax,
        (unsigned)session->core_machine->executor_cpu.data.cr0);
    if (capture.iret_frame_seen) {
        STD_PRINTF("M5:T498:S5:IRET-FRAME:count=%u-read=%u-ss=%04X-base=%08X-sp=%04X-cr0=%08X-operand32=%u-stack32=%u-ip=%04X-cs=%04X-flags=%04X-stopped-read=%u-stopped-ip=%04X-stopped-cs=%04X-stopped-flags=%04X\n",
            (unsigned)capture.iret_frame_count, (unsigned)capture.iret_frame_read,
            (unsigned)capture.iret_ss, (unsigned)capture.iret_ss_base,
            (unsigned)capture.iret_sp,
            (unsigned)capture.iret_cr0,
            (unsigned)capture.iret_operand_size_32,
            (unsigned)capture.iret_stack_size_32,
            (unsigned)capture.iret_frame[0u], (unsigned)capture.iret_frame[1u],
            (unsigned)capture.iret_frame[2u],
            (unsigned)capture.iret_frame_stopped_read,
            (unsigned)capture.iret_frame_stopped[0u],
            (unsigned)capture.iret_frame_stopped[1u],
            (unsigned)capture.iret_frame_stopped[2u]);
    }
    if (capture.nmi_entry_seen) {
        STD_PRINTF("M5:T498:S5:NMI-ENTRY-FRAME:count=%u-read=%u-ss=%04X-base=%08X-sp=%04X-vector2-read=%u-vector2-ip=%04X-vector2-cs=%04X-source-cr0=%08X-source-ip=%04X-source-cs=%04X-source-flags=%04X-ip=%04X-cs=%04X-flags=%04X\n",
            (unsigned)capture.nmi_entry_count, (unsigned)capture.nmi_entry_frame_read,
            (unsigned)capture.nmi_entry_ss, (unsigned)capture.nmi_entry_ss_base,
            (unsigned)capture.nmi_entry_sp,
            (unsigned)capture.nmi_entry_vector2_read,
            (unsigned)capture.nmi_entry_vector2[0u],
            (unsigned)capture.nmi_entry_vector2[1u],
            (unsigned)capture.nmi_entry_source_cr0,
            (unsigned)capture.nmi_entry_source_ip,
            (unsigned)capture.nmi_entry_source_cs,
            (unsigned)capture.nmi_entry_source_flags,
            (unsigned)capture.nmi_entry_frame[0u],
            (unsigned)capture.nmi_entry_frame[1u],
            (unsigned)capture.nmi_entry_frame[2u]);
    }
    STD_PRINTF("M5:T498:S5:RESET-VECTOR2:read=%u-ip=%04X-cs=%04X\n",
        (unsigned)capture.reset_vector2_read, (unsigned)capture.reset_vector2[0u],
        (unsigned)capture.reset_vector2[1u]);
    if (capture.last_software_interrupt_valid) {
        STD_PRINTF("M5:T498:S5:LAST-SOFTWARE-INT:pc=%08X-vector=%02X-ss=%04X-sp=%04X-target=%08X-read=%u-bytes=%02X,%02X,%02X,%02X-stopped-read=%u-stopped-bytes=%02X,%02X,%02X,%02X\n",
            (unsigned)capture.last_software_interrupt_pc,
            (unsigned)capture.last_software_interrupt_vector,
            (unsigned)capture.last_software_interrupt_ss,
            (unsigned)capture.last_software_interrupt_sp,
            (unsigned)capture.last_software_interrupt_target,
            (unsigned)capture.last_software_interrupt_target_read,
            (unsigned)capture.last_software_interrupt_target_bytes[0u],
            (unsigned)capture.last_software_interrupt_target_bytes[1u],
            (unsigned)capture.last_software_interrupt_target_bytes[2u],
            (unsigned)capture.last_software_interrupt_target_bytes[3u],
            (unsigned)capture.last_software_interrupt_target_stopped_read,
            (unsigned)capture.last_software_interrupt_target_stopped_bytes[0u],
            (unsigned)capture.last_software_interrupt_target_stopped_bytes[1u],
            (unsigned)capture.last_software_interrupt_target_stopped_bytes[2u],
            (unsigned)capture.last_software_interrupt_target_stopped_bytes[3u]);
        STD_PRINTF("M5:T498:S5:SOFTWARE-INT-TRACE:count=%u",
            (unsigned)capture.software_interrupt_trace_count);
        for (index = 0u; index < capture.software_interrupt_trace_count; ++index) {
            STD_PRINTF("-%08X", (unsigned)capture.software_interrupt_trace[index]);
        }
        STD_PRINTF("\n");
    }
    if (capture.zero_code_seen) {
        STD_PRINTF("M5:T498:S5:ZERO-CODE-TRACE:count=%u",
            (unsigned)capture.zero_code_trace_count);
        for (index = 0u; index < capture.zero_code_trace_count; ++index) {
            STD_PRINTF("-%08X:%02X", (unsigned)capture.zero_code_trace[index],
                (unsigned)capture.zero_code_opcode[index]);
        }
        STD_PRINTF("\n");
    }
    if (capture.iret_route_seen) {
        STD_PRINTF("M5:T498:S5:IRET-ROUTE-TRACE:count=%u",
            (unsigned)capture.iret_route_trace_count);
        for (index = 0u; index < capture.iret_route_trace_count; ++index) {
            STD_PRINTF("-%08X:%02X", (unsigned)capture.iret_route_trace[index],
                (unsigned)capture.iret_route_opcode[index]);
        }
        STD_PRINTF("\n");
    }
    if (capture.interrupt_handler_entry_seen) {
        STD_PRINTF("M5:T498:S5:INTERRUPT-HANDLER-ENTRY:count=%u-frame-read=%u-ss=%04X-sp=%04X-source-ip=%04X-source-cs=%04X-source-sp=%04X-ip=%04X-cs=%04X-flags=%04X-vector-found=%u-vector=%02X\n",
            (unsigned)capture.interrupt_handler_entry_count,
            (unsigned)capture.interrupt_handler_frame_read,
            (unsigned)capture.interrupt_handler_ss,
            (unsigned)capture.interrupt_handler_sp,
            (unsigned)capture.interrupt_handler_source_ip,
            (unsigned)capture.interrupt_handler_source_cs,
            (unsigned)capture.interrupt_handler_source_sp,
            (unsigned)capture.interrupt_handler_frame[0u],
            (unsigned)capture.interrupt_handler_frame[1u],
            (unsigned)capture.interrupt_handler_frame[2u],
            (unsigned)capture.interrupt_handler_vector_found,
            (unsigned)capture.interrupt_handler_vector);
        STD_PRINTF("M5:T498:S5:INTERRUPT-HANDLER-IVT:count=%u",
            (unsigned)capture.interrupt_handler_ivt_count);
        for (index = 0u; index < capture.interrupt_handler_ivt_count; ++index) {
            STD_PRINTF("-%02X:%04X", (unsigned)capture.interrupt_handler_ivt_vectors[index],
                (unsigned)capture.interrupt_handler_ivt_offsets[index]);
        }
        STD_PRINTF("\n");
        STD_PRINTF("M5:T498:S5:INTERRUPT-HANDLER-TRACE:count=%u",
            (unsigned)capture.interrupt_handler_trace_count);
        for (index = 0u; index < capture.interrupt_handler_trace_count; ++index) {
            STD_PRINTF("-%08X:%02X", (unsigned)capture.interrupt_handler_trace[index],
                (unsigned)capture.interrupt_handler_trace_opcode[index]);
        }
        STD_PRINTF("\n");
    }
    if (capture.interrupt_scan_entry_seen) {
        STD_PRINTF("M5:T498:S5:INTERRUPT-SCAN-ENTRY:source-ip=%04X-source-cs=%04X-source-sp=%04X-ss=%04X-sp=%04X\n",
            (unsigned)capture.interrupt_scan_source_ip,
            (unsigned)capture.interrupt_scan_source_cs,
            (unsigned)capture.interrupt_scan_source_sp,
            (unsigned)capture.interrupt_scan_ss,
            (unsigned)capture.interrupt_scan_sp);
    }
    if (capture.interrupt_service_entry_seen) {
        STD_PRINTF("M5:T498:S5:INTERRUPT-SERVICE-ENTRY:count=%u-frame-read=%u-ss=%04X-sp=%04X-source-ip=%04X-source-cs=%04X-source-sp=%04X-ip=%04X-cs=%04X-flags=%04X\n",
            (unsigned)capture.interrupt_service_entry_count,
            (unsigned)capture.interrupt_service_frame_read,
            (unsigned)capture.interrupt_service_ss,
            (unsigned)capture.interrupt_service_sp,
            (unsigned)capture.interrupt_service_source_ip,
            (unsigned)capture.interrupt_service_source_cs,
            (unsigned)capture.interrupt_service_source_sp,
            (unsigned)capture.interrupt_service_frame[0u],
            (unsigned)capture.interrupt_service_frame[1u],
            (unsigned)capture.interrupt_service_frame[2u]);
        STD_PRINTF("M5:T498:S5:INTERRUPT-SERVICE-TRACE:count=%u",
            (unsigned)capture.interrupt_service_trace_count);
        for (index = 0u; index < capture.interrupt_service_trace_count; ++index) {
            STD_PRINTF("-%08X:%02X", (unsigned)capture.interrupt_service_trace[index],
                (unsigned)capture.interrupt_service_trace_opcode[index]);
        }
        STD_PRINTF("\n");
    }
    if (capture.low_stack_transition_seen) {
        STD_PRINTF("M5:T498:S5:LOW-STACK-TRANSITION:pc=%08X-before=%04X-after=%04X-trace-count=%u",
            (unsigned)capture.low_stack_transition_pc,
            (unsigned)capture.low_stack_transition_before,
            (unsigned)capture.low_stack_transition_after,
            (unsigned)capture.low_stack_transition_trace_count);
        for (index = 0u; index < capture.low_stack_transition_trace_count; ++index) {
            STD_PRINTF("-%08X:%02X", (unsigned)capture.low_stack_transition_trace[index],
                (unsigned)capture.low_stack_transition_trace_opcode[index]);
        }
        STD_PRINTF("\n");
    }
    if (capture.stack_exhaustion_seen) {
        STD_PRINTF("M5:T498:S5:STACK-EXHAUSTION:pc=%08X-before=%04X-after=%04X-trace-count=%u",
            (unsigned)capture.stack_exhaustion_pc,
            (unsigned)capture.stack_exhaustion_before,
            (unsigned)capture.stack_exhaustion_after,
            (unsigned)capture.stack_exhaustion_trace_count);
        for (index = 0u; index < capture.stack_exhaustion_trace_count; ++index) {
            STD_PRINTF("-%08X:%02X", (unsigned)capture.stack_exhaustion_trace[index],
                (unsigned)capture.stack_exhaustion_trace_opcode[index]);
        }
        STD_PRINTF("\n");
    }
    if (capture.minimum_stack_seen) {
        STD_PRINTF("M5:T498:S5:MINIMUM-STACK:pc=%08X-sp=%04X-trace-count=%u",
            (unsigned)capture.minimum_stack_pc,
            (unsigned)capture.minimum_stack_value,
            (unsigned)capture.minimum_stack_trace_count);
        for (index = 0u; index < capture.minimum_stack_trace_count; ++index) {
            STD_PRINTF("-%08X:%02X", (unsigned)capture.minimum_stack_trace[index],
                (unsigned)capture.minimum_stack_trace_opcode[index]);
        }
        STD_PRINTF("\n");
    }
    if (capture.last_io_valid) {
        STD_PRINTF("M5:T498:S5:LAST-IO:pc=%08X-direction=%u-port=%04X-value=%08X\n",
            (unsigned)capture.last_io_pc, (unsigned)capture.last_io_direction,
            (unsigned)capture.last_io_port, (unsigned)capture.last_io_value);
    }
    if (capture.reset_vector_seen) {
        STD_PRINTF("M5:T498:S5:RESET-VECTOR-PREDECESSOR-IO:valid=%u-pc=%08X-direction=%u-port=%04X-value=%08X\n",
            (unsigned)capture.reset_vector_io_valid,
            (unsigned)capture.reset_vector_io_pc,
            (unsigned)capture.reset_vector_io_direction,
            (unsigned)capture.reset_vector_io_port,
            (unsigned)capture.reset_vector_io_value);
    }
    if (warm_reset_diagnostic) {
        type_unsigned_32 reset_sample_count = capture.reset_vector_count;

        if (reset_sample_count > MODEL40_CAPTURE_RESET_HISTORY) {
            reset_sample_count = MODEL40_CAPTURE_RESET_HISTORY;
        }
        STD_PRINTF("M5:T498:S5:WARM-RESET:count=%u", (unsigned)capture.reset_vector_count);
        for (index = 0u; index < reset_sample_count; ++index) {
            STD_PRINTF("-shutdown=%02X-kbc-output=%02X",
                (unsigned)capture.reset_vector_shutdown_status[index],
                (unsigned)capture.reset_vector_kbc_output_port[index]);
        }
        if (capture.reset_instruction_state_seen) {
            STD_PRINTF("-before-pulse-shutdown=%02X-before-pulse-kbc-output=%02X",
                (unsigned)capture.reset_instruction_shutdown_status,
                (unsigned)capture.reset_instruction_kbc_output_port);
        }
        STD_PRINTF("\n");
    }
    if (capture.reset_instruction_seen) {
        STD_PRINTF("M5:T498:S5:RESET-INSTRUCTION:port=%02X-value=%02X\n",
            (unsigned)capture.reset_instruction_port,
            (unsigned)capture.reset_instruction_value);
    }
    if (capture.iret_io_count != 0u) {
        STD_PRINTF("M5:T498:S5:IRET-PREDECESSOR-IO:count=%u",
            (unsigned)capture.iret_io_count);
        for (index = 0u; index < capture.iret_io_count; ++index) {
            STD_PRINTF("-%08X:%u:%04X:%08X", (unsigned)capture.iret_io_pc[index],
                (unsigned)capture.iret_io_direction[index],
                (unsigned)capture.iret_io_port[index],
                (unsigned)capture.iret_io_value[index]);
        }
        STD_PRINTF("\n");
    }
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
    if (port_sequence_diagnostic) {
        return capture.checkpoint_reached && capture.post_c0_port_count ==
            MODEL40_CAPTURE_POST_C0_HISTORY && capture.unallocated == 0u &&
            status == TYPE_STATUS_OK ? 0 : 1;
    }
    if (post_c0_io_diagnostic) {
        return capture.checkpoint_reached && capture.post_c0_io_seen &&
            capture.unallocated == 0u &&
            status == TYPE_STATUS_OK ? 0 : 1;
    }
    if (fdc_read_data_diagnostic) {
        return model40_capture_c0a_reached(&capture) &&
            capture.fdc_read_data_baseline_valid && capture.fdc_read_data_reached &&
            capture.unallocated == 0u &&
            status == TYPE_STATUS_OK ? 0 : 1;
    }
    if (c1_transfer_diagnostic) {
        return capture.checkpoint_reached && capture.post_c0_io_seen &&
            capture.c1_transfer_reached && capture.unallocated == 0u &&
            status == TYPE_STATUS_OK ? 0 : 1;
    }
    if (c1_diagnostic) {
        return capture.checkpoint_reached && capture.c1_checkpoint_reached &&
        capture.unallocated == 0u &&
            status == TYPE_STATUS_OK ? 0 : 1;
    }
    return capture.checkpoint_reached && capture.unallocated == 0u &&
        status == TYPE_STATUS_OK ? 0 : 1;
}
