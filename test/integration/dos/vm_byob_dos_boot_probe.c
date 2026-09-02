#include "type.h"

#include "core/machine/debug_interface.h"
#include <windows.h>
#ifdef exception_code
#undef exception_code
#endif

#include "core/machine/machine_interface.h"
#include "core/machine/machine.h"
#include "core/machine/memory.h"
#include "core/machine/retirement_observation_interface.h"
#include "core/machine/trace_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/waiting.h"

#define VM_BYOB_BOOT_WALL_LIMIT_MILLISECONDS 90000u
#define VM_BYOB_BOOT_NO_PROGRESS_LIMIT_MILLISECONDS 15000u
#define VM_BYOB_BOOT_DISPLAY_CADENCE_MILLISECONDS 16u
#define VM_BYOB_FDC_PORT_HISTORY 256u
#define VM_BYOB_HDC_PORT_HISTORY 64u
#define VM_BYOB_MODEL40_VIDEO_PORT_HISTORY 64u
#define VM_BYOB_MODEL40_VIDEO_SPECIAL_HISTORY 32u
#define VM_BYOB_NEAR_UD_HISTORY 16u
#define VM_BYOB_CMOS_BYTES 128u
#define VM_BYOB_MODEL40_POST_LATCH_PATHS 5u
#define VM_BYOB_MODEL40_POST_STATUS_WRITERS 12u
#define VM_BYOB_MODEL40_POST_STATUS_HELPER_HISTORY 4u
#define VM_BYOB_MODEL40_MEMORY_BASE_HISTORY 32u
#define VM_BYOB_MODEL40_POST_STATUS_58_HISTORY 16u
#define VM_BYOB_MODEL40_POST_PRIVATE_STATUS_PHYSICAL 0x0001c058u
#define VM_BYOB_MODEL40_POST_PRIVATE_STATUS_HISTORY 16u
#define VM_BYOB_MODEL40_HIGH_B_PAGE_BYTES 16u
#define VM_BYOB_MODEL40_HIGH_B_WRITE_HISTORY 32u
#define VM_BYOB_MODEL40_INT10_VECTOR_HISTORY 16u
#define VM_BYOB_MODEL40_ES_HISTORY 32u

typedef struct vm_byob_fdc_port_event {
    type_unsigned_32 linear_pc;
    type_unsigned_16 port;
    type_unsigned_8 value;
    type_bool write;
} vm_byob_fdc_port_event;

typedef struct vm_byob_boot_trace {
    core_machine *machine;
    type_unsigned_64 cpu_retires;
    type_unsigned_64 rom_retires;
    type_unsigned_64 low_memory_retires;
    type_unsigned_64 other_retires;
    type_unsigned_64 reset_events;
    type_unsigned_64 rom_memory_reads;
    type_unsigned_64 fdc_port_accesses;
    vm_byob_fdc_port_event fdc_port_history[VM_BYOB_FDC_PORT_HISTORY];
    type_unsigned_64 hdc_port_accesses;
    vm_byob_fdc_port_event hdc_port_history[VM_BYOB_HDC_PORT_HISTORY];
    type_unsigned_64 model40_video_port_accesses;
    vm_byob_fdc_port_event
        model40_video_port_history[VM_BYOB_MODEL40_VIDEO_PORT_HISTORY];
    type_unsigned_64 model40_video_special_accesses[5u];
    type_unsigned_8 model40_video_special_last_values[5u];
    type_unsigned_64 model40_video_error_writes;
    type_unsigned_8 model40_video_error_last_value;
    type_unsigned_32 model40_video_error_last_pc;
    vm_byob_fdc_port_event
        model40_video_special_history[VM_BYOB_MODEL40_VIDEO_SPECIAL_HISTORY];
    type_unsigned_64 model40_video_special_history_count;
    type_unsigned_8 cmos_index;
    type_unsigned_64 cmos_reads[VM_BYOB_CMOS_BYTES];
    type_unsigned_64 cmos_writes[VM_BYOB_CMOS_BYTES];
    type_unsigned_8 cmos_last_values[VM_BYOB_CMOS_BYTES];
    type_unsigned_32 cmos_last_write_pc[VM_BYOB_CMOS_BYTES];
    type_unsigned_64 xt_ppi_port_accesses;
    type_unsigned_64 pic_port_accesses;
    type_unsigned_64 pit_port_accesses;
    type_unsigned_64 cga_port_accesses;
    type_unsigned_64 cga_memory_writes;
    type_unsigned_64 pit_wait_first_retires;
    type_unsigned_64 pit_wait_second_retires;
    type_unsigned_64 irq0_retires;
    type_unsigned_32 last_linear_pc;
    type_unsigned_32 int15_linear;
    type_unsigned_64 int15_calls;
    type_unsigned_8 last_int15_ah;
    type_unsigned_32 int15_ah_seen;
    type_unsigned_64 sgdt_calls;
    type_unsigned_64 sidt_calls;
    type_unsigned_64 pushf_calls;
    type_unsigned_16 last_pushf_high;
    type_unsigned_8 int6_vector_write_bytes[4];
    type_unsigned_8 int6_vector_write_mask;
    type_bool int6_pre_fault_snapshot_valid;
    type_unsigned_16 int6_pre_fault_offset;
    type_unsigned_16 int6_pre_fault_segment;
    core_machine_cpu_execution_point near_ud_history[VM_BYOB_NEAR_UD_HISTORY];
    type_unsigned_32 near_ud_eflags[VM_BYOB_NEAR_UD_HISTORY];
    type_unsigned_32 near_ud_eax[VM_BYOB_NEAR_UD_HISTORY];
    type_unsigned_32 near_ud_ebx[VM_BYOB_NEAR_UD_HISTORY];
    type_unsigned_64 near_ud_count;
    type_bool real_286_high_flags_observed;
    core_machine_cpu_execution_point real_286_high_flags_point;
    type_unsigned_32 real_286_high_flags_value;
    type_bool boot_loader_jz_observed;
    core_machine_retirement_control_outcome boot_loader_jz_outcome;
    type_unsigned_32 boot_loader_previous_pc;
    core_machine_retirement_control_outcome boot_loader_previous_outcome;
    type_bool boot_loader_error_observed;
    type_unsigned_16 boot_loader_ds;
    type_unsigned_16 boot_loader_es;
    type_unsigned_16 boot_loader_si;
    type_unsigned_16 boot_loader_di;
    type_unsigned_16 boot_loader_cx;
    type_unsigned_16 boot_loader_flags;
    type_unsigned_8 boot_loader_left[11];
    type_unsigned_8 boot_loader_right[11];
    type_bool boot_loader_read_return_observed;
    type_unsigned_16 boot_loader_read_return_flags;
    type_unsigned_8 boot_loader_int13_state[5];
    type_unsigned_64 fdc_terminal_count;
    type_unsigned_64 fdc_failed_terminal_count;
    core_machine_fdc_terminal_observation last_fdc_terminal;
    type_bool model40_invalid_entry_observed;
    core_machine_cpu_execution_point model40_invalid_entry_source;
    type_unsigned_16 model40_invalid_entry_ss;
    type_unsigned_16 model40_invalid_entry_sp;
    type_bool model40_int10_vector_write_observed;
    type_unsigned_64 model40_int10_vector_write_count;
    type_unsigned_32 model40_int10_vector_write_pc;
    type_unsigned_16 model40_int10_vector_write_cs;
    type_unsigned_16 model40_int10_vector_offset;
    type_unsigned_16 model40_int10_vector_segment;
    type_unsigned_32 model40_int10_vector_history_pc[VM_BYOB_MODEL40_INT10_VECTOR_HISTORY];
    type_unsigned_16 model40_int10_vector_history_cs[VM_BYOB_MODEL40_INT10_VECTOR_HISTORY];
    type_unsigned_16 model40_int10_vector_history_offset[VM_BYOB_MODEL40_INT10_VECTOR_HISTORY];
    type_unsigned_16 model40_int10_vector_history_segment[VM_BYOB_MODEL40_INT10_VECTOR_HISTORY];
    type_unsigned_64 model40_int42_vector_write_count;
    type_unsigned_32 model40_int42_vector_write_pc[VM_BYOB_MODEL40_INT10_VECTOR_HISTORY];
    type_unsigned_16 model40_int42_vector_history_offset[VM_BYOB_MODEL40_INT10_VECTOR_HISTORY];
    type_unsigned_16 model40_int42_vector_history_segment[VM_BYOB_MODEL40_INT10_VECTOR_HISTORY];
    type_unsigned_64 model40_int10_entry_count;
    type_unsigned_32 model40_int10_entry_predecessor;
    type_unsigned_16 model40_int10_entry_ss;
    type_unsigned_16 model40_int10_entry_sp;
    type_bool model40_int10_iret_frame_observed;
    type_unsigned_64 model40_int10_iret_frame_count;
    type_unsigned_32 model40_int10_iret_frame_pc;
    type_unsigned_16 model40_int10_iret_ss;
    type_unsigned_16 model40_int10_iret_sp;
    type_unsigned_32 model40_int10_iret_esp;
    type_unsigned_32 model40_int10_iret_ss_base;
    type_unsigned_32 model40_int10_iret_ss_limit;
    type_bool model40_int10_iret_ss_big;
    type_bool model40_int10_iret_cs_default_32;
    type_unsigned_16 model40_int10_iret_words[4u];
    type_bool model40_bios_iret_frame_observed;
    type_unsigned_16 model40_bios_iret_ss;
    type_unsigned_16 model40_bios_iret_sp;
    type_unsigned_16 model40_bios_iret_words[4u];
    type_unsigned_64 model40_video_rom_entries;
    type_unsigned_32 model40_video_rom_first_pc;
    type_unsigned_64 kbc_write_count;
    type_unsigned_16 kbc_last_write_port;
    type_unsigned_8 kbc_last_write_value;
    type_unsigned_32 kbc_last_write_pc;
    type_unsigned_64 model40_resume_entries;
    type_unsigned_32 model40_resume_predecessor;
    type_unsigned_16 model40_resume_ax;
    type_unsigned_64 model40_reset_vector_target_entries;
    type_unsigned_32 model40_reset_vector_target_predecessor;
    type_bool model40_shutdown_diagnostic_valid;
    core_machine_cpu_diagnostic model40_shutdown_diagnostic;
    type_bool model40_protected_transition_observed;
    type_unsigned_8 model40_gdt[32];
    type_unsigned_8 model40_gdtr_pointer[6];
    type_unsigned_64 model40_post_setup_entries;
    type_unsigned_32 model40_post_setup_predecessor;
    type_unsigned_64 model40_video_clear_entries;
    type_unsigned_64 model40_video_delay_entries;
    type_unsigned_32 model40_video_delay_predecessor;
    type_unsigned_64 model40_post_latch_writes;
    type_unsigned_32 model40_post_latch_last_pc;
    type_unsigned_8 model40_post_latch_last_value;
    type_unsigned_64 model40_post_latch_path_entries[VM_BYOB_MODEL40_POST_LATCH_PATHS];
    type_unsigned_32 model40_post_latch_path_predecessors[VM_BYOB_MODEL40_POST_LATCH_PATHS];
    type_unsigned_64 model40_resume_prompt_entries;
    type_unsigned_32 model40_resume_prompt_predecessor;
    type_unsigned_64 model40_resume_wait_entries;
    type_unsigned_32 model40_resume_wait_predecessor;
    type_unsigned_64 model40_post_status_helper_entries;
    type_unsigned_32 model40_post_status_helper_predecessor;
    type_unsigned_32 model40_post_status_helper_predecessors[
        VM_BYOB_MODEL40_POST_STATUS_HELPER_HISTORY];
    type_unsigned_64 model40_post_status_writer_entries[VM_BYOB_MODEL40_POST_STATUS_WRITERS];
    type_unsigned_32 model40_post_status_writer_predecessors[VM_BYOB_MODEL40_POST_STATUS_WRITERS];
    type_unsigned_8 model40_post_status_value;
    type_unsigned_64 model40_memory_address_error_entries;
    type_unsigned_32 model40_memory_address_error_predecessor;
    type_unsigned_16 model40_memory_address_error_ds;
    type_unsigned_32 model40_memory_address_error_ds_base;
    type_unsigned_16 model40_memory_address_error_status;
    type_unsigned_64 model40_memory_address_test_entries;
    type_unsigned_32 model40_memory_address_test_eflags;
    type_unsigned_16 model40_memory_address_test_status;
    type_unsigned_64 model40_memory_compare_failures;
    type_unsigned_64 model40_memory_compare_branch_taken;
    type_unsigned_64 model40_memory_scas_entries;
    type_unsigned_16 model40_memory_scas_ax;
    type_unsigned_16 model40_memory_scas_di;
    type_unsigned_16 model40_memory_scas_es;
    type_unsigned_32 model40_memory_scas_es_base;
    type_unsigned_32 model40_memory_scas_eflags;
    type_unsigned_64 model40_memory_mismatch_entries;
    type_unsigned_16 model40_memory_mismatch_ax;
    type_unsigned_16 model40_memory_mismatch_di;
    type_unsigned_16 model40_memory_mismatch_es;
    type_unsigned_32 model40_memory_mismatch_es_base;
    type_unsigned_16 model40_memory_mismatch_value;
    type_unsigned_64 model40_memory_test_return_entries;
    type_unsigned_16 model40_memory_test_return_ax;
    type_unsigned_64 model40_memory_test_entries;
    type_unsigned_16 model40_memory_test_dx;
    type_unsigned_16 model40_memory_test_ax_entry;
    type_bool model40_memory_status_test_active;
    type_unsigned_16 model40_memory_status_test_ax_entry;
    type_unsigned_64 model40_memory_status_test_mismatches;
    type_unsigned_16 model40_memory_status_test_expected;
    type_unsigned_16 model40_memory_status_test_actual;
    type_unsigned_16 model40_memory_status_test_offset;
    type_unsigned_16 model40_memory_status_test_es;
    type_unsigned_32 model40_memory_status_test_es_base;
    type_unsigned_32 model40_memory_status_test_gdtr_base;
    type_unsigned_16 model40_memory_status_test_gdtr_limit;
    type_unsigned_8 model40_memory_status_test_descriptor[8u];
    type_unsigned_32 model40_memory_status_test_cr0;
    type_bool model40_memory_status_test_video_memory_disabled;
    type_unsigned_8 model40_memory_status_test_graphics_6;
    type_unsigned_8 model40_memory_status_test_sequencer_0;
    type_unsigned_8 model40_memory_status_test_high_b_page[
        VM_BYOB_MODEL40_HIGH_B_PAGE_BYTES];
    type_unsigned_64 model40_memory_high_b_page_writes;
    type_unsigned_32 model40_memory_high_b_page_first_pc;
    type_unsigned_32 model40_memory_high_b_page_last_pc;
    type_bool model40_memory_pattern_producer_active;
    type_unsigned_64 model40_memory_pattern_producer_entries;
    type_unsigned_64 model40_memory_pattern_producer_high_b_writes;
    type_unsigned_32 model40_memory_high_b_write_pcs[
        VM_BYOB_MODEL40_HIGH_B_WRITE_HISTORY];
    type_unsigned_8 model40_memory_high_b_write_producer[
        VM_BYOB_MODEL40_HIGH_B_WRITE_HISTORY];
    type_unsigned_8 model40_memory_high_b_write_pages[
        VM_BYOB_MODEL40_HIGH_B_WRITE_HISTORY][VM_BYOB_MODEL40_HIGH_B_PAGE_BYTES];
    type_unsigned_64 model40_post_status_58_writes;
    type_unsigned_32 model40_post_status_58_last_pc;
    type_unsigned_16 model40_post_status_58_last_value;
    type_unsigned_64 model40_post_status_58_observer_writes;
    type_unsigned_32 model40_post_status_58_observer_last_pc;
    type_unsigned_8 model40_post_status_58_observer_last_value;
    type_unsigned_32 model40_post_status_58_observer_pcs[
        VM_BYOB_MODEL40_POST_STATUS_58_HISTORY];
    type_unsigned_8 model40_post_status_58_observer_values[
        VM_BYOB_MODEL40_POST_STATUS_58_HISTORY];
    type_unsigned_64 model40_post_private_status_writes;
    type_unsigned_32 model40_post_private_status_last_pc;
    type_unsigned_16 model40_post_private_status_last_value;
    type_unsigned_32 model40_post_private_status_pcs[
        VM_BYOB_MODEL40_POST_PRIVATE_STATUS_HISTORY];
    type_unsigned_16 model40_post_private_status_values[
        VM_BYOB_MODEL40_POST_PRIVATE_STATUS_HISTORY];
    type_unsigned_64 model40_port61_reads;
    type_unsigned_32 model40_port61_last_pc;
    type_unsigned_8 model40_port61_last_value;
    type_unsigned_64 model40_memory_compare_error_branches;
    type_unsigned_64 model40_memory_parity_error_branches;
    type_unsigned_64 model40_memory_parity_test_reads;
    type_unsigned_8 model40_memory_parity_test_last_value;
    type_unsigned_64 model40_memory_error_exit_entries;
    type_unsigned_32 model40_memory_error_exit_predecessor;
    type_unsigned_16 model40_memory_error_es;
    type_unsigned_16 model40_memory_error_di;
    type_unsigned_16 model40_memory_error_ax;
    type_unsigned_32 model40_memory_compare_eax;
    type_unsigned_32 model40_memory_compare_edi;
    type_unsigned_32 model40_memory_compare_es_base;
    type_unsigned_16 model40_memory_compare_es_selector;
    type_unsigned_64 model40_memory_address_failures;
    type_unsigned_16 model40_memory_address_failure_ds;
    type_unsigned_16 model40_memory_address_failure_si;
    type_unsigned_32 model40_memory_address_failure_eax;
    type_unsigned_32 model40_memory_address_failure_ebx;
    type_unsigned_32 model40_memory_address_failure_ebp;
    type_unsigned_32 model40_memory_address_failure_eflags;
    type_unsigned_32 model40_memory_compare_cr0;
    type_unsigned_32 model40_memory_compare_gdtr_base;
    type_unsigned_32 model40_memory_compare_gdtr_limit;
    type_unsigned_8 model40_memory_compare_descriptor[8];
    type_unsigned_16 model40_memory_compare_value;
    type_unsigned_64 model40_memory_pattern_entries;
    type_unsigned_64 model40_ram_post_entries;
    type_unsigned_64 model40_ram_post_returns;
    type_unsigned_64 model40_ram_post_failures;
    type_unsigned_32 model40_memory_pattern_es_base;
    type_unsigned_16 model40_memory_pattern_es_selector;
    type_unsigned_32 model40_memory_pattern_after_es_base;
    type_unsigned_16 model40_memory_pattern_after_value;
    type_unsigned_32 model40_memory_pattern_after_eax;
    type_unsigned_32 model40_memory_pattern_after_edi;
    type_unsigned_32 model40_memory_pattern_bases[VM_BYOB_MODEL40_MEMORY_BASE_HISTORY];
    type_unsigned_32 model40_memory_compare_bases[VM_BYOB_MODEL40_MEMORY_BASE_HISTORY];
    type_unsigned_64 model40_memory_1e_writes;
    type_unsigned_32 model40_memory_1e_last_pc;
    type_unsigned_8 model40_memory_1e_last_value;
    type_unsigned_64 model40_memory_high_writes;
    type_unsigned_32 model40_memory_high_first_address;
    type_unsigned_32 model40_memory_high_last_address;
    type_unsigned_32 model40_memory_high_last_pc;
    type_unsigned_8 model40_memory_high_first_value;
    type_unsigned_8 model40_memory_high_last_value;
    type_unsigned_64 model40_memory_b_window_writes;
    type_unsigned_32 model40_memory_b_window_first_pc;
    type_unsigned_32 model40_memory_b_window_last_pc;
    type_unsigned_64 model40_memory_b_window_writes_at_mismatch;
    type_unsigned_32 model40_memory_b_window_last_pc_at_mismatch;
    type_unsigned_64 model40_memory_b_first_word_writes;
    type_unsigned_32 model40_memory_b_first_word_last_pc;
    type_unsigned_16 model40_memory_b_first_word_last_value;
    type_unsigned_8 model40_memory_b_first_word_graphics_6;
    type_unsigned_8 model40_memory_b_first_word_sequencer_0;
    type_bool model40_memory_b_first_word_video_memory_disabled;
    type_unsigned_64 model40_retirements;
    type_unsigned_16 model40_last_es_selector;
    type_unsigned_32 model40_last_es_base;
    type_unsigned_64 model40_es_change_count;
    type_unsigned_32 model40_es_change_pcs[VM_BYOB_MODEL40_ES_HISTORY];
    type_unsigned_16 model40_es_change_selectors[VM_BYOB_MODEL40_ES_HISTORY];
    type_unsigned_32 model40_es_change_bases[VM_BYOB_MODEL40_ES_HISTORY];
    type_unsigned_32 model40_es_change_cr0[VM_BYOB_MODEL40_ES_HISTORY];
    type_unsigned_64 model40_memory_b_first_word_retirements;
    type_unsigned_64 model40_memory_fb_first_word_retirements;
    type_unsigned_64 model40_memory_fb_page_writes;
    type_unsigned_32 model40_memory_fb_page_first_pc;
    type_unsigned_32 model40_memory_fb_page_last_pc;
    type_unsigned_64 model40_memory_fb_first_word_writes;
    type_unsigned_32 model40_memory_fb_first_word_last_pc;
    type_unsigned_16 model40_memory_fb_first_word_last_value;
    type_unsigned_64 model40_memory_pattern_write_count;
    type_unsigned_32 model40_memory_pattern_write_first_address;
    type_unsigned_32 model40_memory_pattern_write_last_address;
} vm_byob_boot_trace;

static C_INT vm_byob_text_memory_has(core_machine *machine, const C_CHAR *text);

static C_VOID vm_byob_fdc_port_record(vm_byob_boot_trace *trace,
    const core_machine_trace_event *event)
{
    vm_byob_fdc_port_event *record;
    STD_SIZE_T index;

    if (trace == STD_NULL || event == STD_NULL) return;
    index = (STD_SIZE_T)(trace->fdc_port_accesses % VM_BYOB_FDC_PORT_HISTORY);
    record = &trace->fdc_port_history[index];
    record->linear_pc = event->linear_pc;
    record->port = (type_unsigned_16)event->address;
    record->value = (type_unsigned_8)event->value;
    record->write = event->type == CORE_MACHINE_TRACE_PORT_WRITE;
}

static C_VOID vm_byob_hdc_port_record(vm_byob_boot_trace *trace,
    const core_machine_trace_event *event)
{
    vm_byob_fdc_port_event *record;
    STD_SIZE_T index;

    if (trace == STD_NULL || event == STD_NULL) return;
    index = (STD_SIZE_T)(trace->hdc_port_accesses % VM_BYOB_HDC_PORT_HISTORY);
    record = &trace->hdc_port_history[index];
    record->linear_pc = event->linear_pc;
    record->port = (type_unsigned_16)event->address;
    record->value = (type_unsigned_8)event->value;
    record->write = event->type == CORE_MACHINE_TRACE_PORT_WRITE;
}

static C_VOID vm_byob_kbc_port_record(vm_byob_boot_trace *trace,
    const core_machine_trace_event *event)
{
    if (trace == STD_NULL || event == STD_NULL ||
        event->type != CORE_MACHINE_TRACE_PORT_WRITE ||
        (event->address != 0x0060u && event->address != 0x0064u)) return;
    ++trace->kbc_write_count;
    trace->kbc_last_write_port = (type_unsigned_16)event->address;
    trace->kbc_last_write_value = (type_unsigned_8)event->value;
    trace->kbc_last_write_pc = event->linear_pc;
}

static C_VOID vm_byob_fdc_terminal_observe(C_VOID *context,
    const core_machine_fdc_terminal_observation *observation)
{
    vm_byob_boot_trace *trace = context;

    if (trace == STD_NULL || observation == STD_NULL) return;
    ++trace->fdc_terminal_count;
    if (!observation->successful) ++trace->fdc_failed_terminal_count;
    trace->last_fdc_terminal = *observation;
}

static C_VOID vm_byob_model40_vector_write_observe(C_VOID *context,
    type_unsigned_32 physical, type_native_unsigned bytes)
{
    vm_byob_boot_trace *trace = context;

    if (trace == STD_NULL || trace->machine == STD_NULL ||
        trace->model40_int10_vector_write_observed || physical > 0x0042u ||
        physical + bytes <= 0x0040u) return;
    trace->model40_int10_vector_write_observed = TYPE_TRUE;
    trace->model40_int10_vector_write_pc = trace->machine->executor_cpu.data.cs.base +
        trace->machine->executor_cpu.data.eip;
    trace->model40_int10_vector_write_cs = trace->machine->executor_cpu.data.cs.selector;
}

static C_VOID vm_byob_fdc_retirement_record(vm_byob_boot_trace *trace,
    const core_machine_retirement_observation *observation)
{
    vm_byob_fdc_port_event *record;
    STD_SIZE_T index;

    if (trace == STD_NULL || observation == STD_NULL ||
        observation->io_direction == CORE_MACHINE_RETIREMENT_IO_NONE ||
        observation->io_port < 0x03f0u || observation->io_port > 0x03f7u ||
        observation->io_port == 0x03f4u) return;
    index = (STD_SIZE_T)(trace->fdc_port_accesses % VM_BYOB_FDC_PORT_HISTORY);
    record = &trace->fdc_port_history[index];
    record->linear_pc = observation->point.linear_pc;
    record->port = observation->io_port;
    record->value = (type_unsigned_8)observation->io_value;
    record->write = observation->io_direction == CORE_MACHINE_RETIREMENT_IO_WRITE;
    ++trace->fdc_port_accesses;
}

static C_VOID vm_byob_model40_video_retirement_record(vm_byob_boot_trace *trace,
    const core_machine_retirement_observation *observation)
{
    vm_byob_fdc_port_event *record;
    STD_SIZE_T index;

    if (trace == STD_NULL || observation == STD_NULL ||
        observation->point.linear_pc < 0x000c0000u ||
        observation->point.linear_pc >= 0x000c4000u ||
        observation->io_direction == CORE_MACHINE_RETIREMENT_IO_NONE) return;
    if (observation->io_port == 0x0084u &&
        observation->io_direction == CORE_MACHINE_RETIREMENT_IO_WRITE) {
        ++trace->model40_video_error_writes;
        trace->model40_video_error_last_value = (type_unsigned_8)observation->io_value;
        trace->model40_video_error_last_pc = observation->point.linear_pc;
    }
    if (observation->io_port < 0x03b0u || observation->io_port > 0x0fc6u) return;
    index = (STD_SIZE_T)(trace->model40_video_port_accesses %
        VM_BYOB_MODEL40_VIDEO_PORT_HISTORY);
    record = &trace->model40_video_port_history[index];
    record->linear_pc = observation->point.linear_pc;
    record->port = observation->io_port;
    record->value = (type_unsigned_8)observation->io_value;
    record->write = observation->io_direction == CORE_MACHINE_RETIREMENT_IO_WRITE;
    ++trace->model40_video_port_accesses;
    {
        const type_unsigned_16 ports[5u] = { 0x03c2u, 0x03c6u, 0x07c6u,
            0x0bc6u, 0x0fc6u };
        STD_SIZE_T special;

        for (special = 0u; special < 5u; ++special) {
            if (observation->io_port == ports[special]) {
                const STD_SIZE_T special_index = (STD_SIZE_T)(
                    trace->model40_video_special_history_count %
                    VM_BYOB_MODEL40_VIDEO_SPECIAL_HISTORY);

                ++trace->model40_video_special_accesses[special];
                trace->model40_video_special_last_values[special] =
                    (type_unsigned_8)observation->io_value;
                trace->model40_video_special_history[special_index] = *record;
                ++trace->model40_video_special_history_count;
                break;
            }
        }
    }
}

static C_VOID vm_byob_cmos_retirement_record(vm_byob_boot_trace *trace,
    const core_machine_retirement_observation *observation)
{
    type_unsigned_8 index;

    if (trace == STD_NULL || observation == STD_NULL ||
        observation->io_direction == CORE_MACHINE_RETIREMENT_IO_NONE) return;
    if (observation->io_port == 0x0070u &&
        observation->io_direction == CORE_MACHINE_RETIREMENT_IO_WRITE) {
        trace->cmos_index = (type_unsigned_8)observation->io_value & 0x7fu;
        return;
    }
    if (observation->io_port != 0x0071u) return;
    index = trace->cmos_index;
    trace->cmos_last_values[index] = (type_unsigned_8)observation->io_value;
    if (observation->io_direction == CORE_MACHINE_RETIREMENT_IO_READ)
        ++trace->cmos_reads[index];
    else {
        ++trace->cmos_writes[index];
        trace->cmos_last_write_pc[index] = observation->point.linear_pc;
    }
}

static C_VOID vm_byob_retirement_observe(C_VOID *context,
    const core_machine_retirement_observation *observation)
{
    vm_byob_boot_trace *trace = context;

    if (trace == STD_NULL || observation == STD_NULL) return;
    if (trace->machine != STD_NULL) {
        const t_cpu_data_sreg *es = &trace->machine->executor_cpu.data.es;

        ++trace->model40_retirements;
        if (es->selector != trace->model40_last_es_selector ||
            es->base != trace->model40_last_es_base) {
            const STD_SIZE_T index = (STD_SIZE_T)(trace->model40_es_change_count %
                VM_BYOB_MODEL40_ES_HISTORY);

            trace->model40_es_change_pcs[index] = observation->point.linear_pc;
            trace->model40_es_change_selectors[index] = es->selector;
            trace->model40_es_change_bases[index] = es->base;
            trace->model40_es_change_cr0[index] = trace->machine->executor_cpu.data.cr0;
            ++trace->model40_es_change_count;
            trace->model40_last_es_selector = es->selector;
            trace->model40_last_es_base = es->base;
        }
    }
    if (observation->point.linear_pc == 0x000c0cd7u && trace->machine != STD_NULL) {
        ++trace->model40_int10_entry_count;
        trace->model40_int10_entry_predecessor = trace->last_linear_pc;
        trace->model40_int10_entry_ss = trace->machine->executor_cpu.data.ss.selector;
        trace->model40_int10_entry_sp = (type_unsigned_16)
            trace->machine->executor_cpu.data.esp;
    }
    if ((observation->point.linear_pc == 0x000c0d2du ||
         observation->point.linear_pc == 0x000c0d30u) && trace->machine != STD_NULL) {
        trace->model40_int10_iret_frame_observed = TYPE_TRUE;
        ++trace->model40_int10_iret_frame_count;
        trace->model40_int10_iret_frame_pc = observation->point.linear_pc;
        trace->model40_int10_iret_ss = trace->machine->executor_cpu.data.ss.selector;
        trace->model40_int10_iret_sp = (type_unsigned_16)
            trace->machine->executor_cpu.data.esp;
        trace->model40_int10_iret_esp = trace->machine->executor_cpu.data.esp;
        trace->model40_int10_iret_ss_base = trace->machine->executor_cpu.data.ss.base;
        trace->model40_int10_iret_ss_limit = trace->machine->executor_cpu.data.ss.limit;
        trace->model40_int10_iret_ss_big = trace->machine->executor_cpu.data.ss.seg.data.big;
        trace->model40_int10_iret_cs_default_32 =
            trace->machine->executor_cpu.data.cs.seg.exec.defsize;
        (C_VOID)core_machine_memory_read_physical(&trace->machine->executor_memory,
            trace->machine->executor_cpu.data.ss.base + trace->model40_int10_iret_sp,
            (type_virtual_address)trace->model40_int10_iret_words,
            sizeof(trace->model40_int10_iret_words));
    }
    if (observation->point.linear_pc == 0x000fd7a6u && trace->machine != STD_NULL) {
        trace->model40_bios_iret_frame_observed = TYPE_TRUE;
        trace->model40_bios_iret_ss = trace->machine->executor_cpu.data.ss.selector;
        trace->model40_bios_iret_sp = (type_unsigned_16)trace->machine->executor_cpu.data.esp;
        (C_VOID)core_machine_memory_read_physical(&trace->machine->executor_memory,
            trace->machine->executor_cpu.data.ss.base +
                (type_unsigned_16)(trace->model40_bios_iret_sp - 6u),
            (type_virtual_address)trace->model40_bios_iret_words,
            sizeof(trace->model40_bios_iret_words));
    }
    if (!trace->model40_invalid_entry_observed && trace->machine != STD_NULL &&
        trace->machine->executor_cpu.data.cs.base + trace->machine->executor_cpu.data.eip ==
            0x000e0cd7u) {
        trace->model40_invalid_entry_observed = TYPE_TRUE;
        trace->model40_invalid_entry_source = observation->point;
        trace->model40_invalid_entry_ss = trace->machine->executor_cpu.data.ss.selector;
        trace->model40_invalid_entry_sp = (type_unsigned_16)
            trace->machine->executor_cpu.data.esp;
    }
    if (observation->point.linear_pc >= 0x000c0000u &&
        observation->point.linear_pc < 0x000c4000u) {
        if (trace->model40_video_rom_entries == 0u)
            trace->model40_video_rom_first_pc = observation->point.linear_pc;
        ++trace->model40_video_rom_entries;
    }
    vm_byob_fdc_retirement_record(trace, observation);
    vm_byob_model40_video_retirement_record(trace, observation);
    vm_byob_cmos_retirement_record(trace, observation);
    if (observation->point.byte_count >= 3u &&
        observation->point.bytes[0u] == 0x0fu &&
        observation->point.bytes[1u] == 0x22u &&
        observation->point.bytes[2u] == 0x00u && trace->machine != STD_NULL) {
        trace->model40_protected_transition_observed =
            core_machine_memory_read(trace->machine, 0x0009f300u,
                trace->model40_gdt, sizeof(trace->model40_gdt)) == TYPE_STATUS_OK;
        (C_VOID)core_machine_memory_read(trace->machine, 0x000f0a13u,
            trace->model40_gdtr_pointer, sizeof(trace->model40_gdtr_pointer));
    }
    if (observation->io_direction == CORE_MACHINE_RETIREMENT_IO_WRITE &&
        (observation->io_port == 0x0060u || observation->io_port == 0x0064u)) {
        ++trace->kbc_write_count;
        trace->kbc_last_write_port = observation->io_port;
        trace->kbc_last_write_value = (type_unsigned_8)observation->io_value;
        trace->kbc_last_write_pc = observation->point.linear_pc;
    }
    if (observation->io_direction == CORE_MACHINE_RETIREMENT_IO_READ &&
        observation->io_port == 0x0061u) {
        ++trace->model40_port61_reads;
        trace->model40_port61_last_pc = observation->point.linear_pc;
        trace->model40_port61_last_value = (type_unsigned_8)observation->io_value;
        if (observation->point.linear_pc == 0x000fd1b1u) {
            ++trace->model40_memory_parity_test_reads;
            trace->model40_memory_parity_test_last_value =
                (type_unsigned_8)observation->io_value;
        }
    }
    if (observation->point.linear_pc == 0x000fd1abu &&
        observation->control_outcome == CORE_MACHINE_RETIREMENT_CONTROL_TAKEN) {
        ++trace->model40_memory_compare_error_branches;
    }
    if (observation->point.linear_pc == 0x000fd1b7u &&
        observation->control_outcome == CORE_MACHINE_RETIREMENT_CONTROL_TAKEN) {
        ++trace->model40_memory_parity_error_branches;
    }
    if (observation->point.linear_pc == 0x000fc2d0u && trace->machine != STD_NULL) {
        trace->model40_memory_error_es = trace->machine->executor_cpu.data.es.selector;
        trace->model40_memory_error_di = (type_unsigned_16)trace->machine->executor_cpu.data.edi;
        trace->model40_memory_error_ax = (type_unsigned_16)trace->machine->executor_cpu.data.eax;
    }
    if (observation->point.linear_pc == 0x000fc1a9u && trace->machine != STD_NULL) {
        ++trace->model40_memory_test_entries;
        trace->model40_memory_test_dx = (type_unsigned_16)trace->machine->executor_cpu.data.edx;
        trace->model40_memory_test_ax_entry = (type_unsigned_16)trace->machine->executor_cpu.data.eax;
    }
    if (observation->point.linear_pc == 0x000fc101u) {
        ++trace->model40_memory_pattern_producer_entries;
        trace->model40_memory_pattern_producer_active = TYPE_TRUE;
    }
    if (observation->point.linear_pc == 0x000fc1c8u) {
        trace->model40_memory_pattern_producer_active = TYPE_FALSE;
    }
    if (observation->point.linear_pc == 0x000fbe83u && trace->machine != STD_NULL) {
        trace->model40_memory_status_test_active = TYPE_TRUE;
        trace->model40_memory_status_test_ax_entry =
            (type_unsigned_16)trace->machine->executor_cpu.data.eax;
        trace->model40_memory_status_test_video_memory_disabled =
            trace->machine->shared_vadp.data.compaq_cpu_video_memory_disabled;
        trace->model40_memory_status_test_graphics_6 =
            trace->machine->shared_vadp.data.graphics[6u];
        trace->model40_memory_status_test_sequencer_0 =
            trace->machine->shared_vadp.data.sequencer[0u];
        trace->model40_memory_status_test_gdtr_base =
            trace->machine->executor_cpu.data.gdtr.base;
        trace->model40_memory_status_test_gdtr_limit =
            trace->machine->executor_cpu.data.gdtr.limit;
        (C_VOID)core_machine_memory_read(trace->machine,
            trace->model40_memory_status_test_gdtr_base + 0x48u,
            trace->model40_memory_status_test_descriptor,
            sizeof(trace->model40_memory_status_test_descriptor));
        (C_VOID)core_machine_memory_read_physical(&trace->machine->executor_memory,
            0x00fb0000u,
            (type_virtual_address)trace->model40_memory_status_test_high_b_page,
            sizeof(trace->model40_memory_status_test_high_b_page));
    }
    if (observation->point.linear_pc == 0x000fbe86u) {
        trace->model40_memory_status_test_active = TYPE_FALSE;
    }
    if (observation->point.linear_pc == 0x000fc2d2u) {
        ++trace->model40_memory_error_exit_entries;
        trace->model40_memory_error_exit_predecessor = trace->last_linear_pc;
    }
    if (observation->point.linear_pc == 0x000fd1eau && trace->machine != STD_NULL) {
        const t_cpu *cpu = &trace->machine->executor_cpu;

        ++trace->model40_memory_address_failures;
        trace->model40_memory_address_failure_ds = cpu->data.ds.selector;
        trace->model40_memory_address_failure_si = (type_unsigned_16)cpu->data.esi;
        trace->model40_memory_address_failure_eax = cpu->data.eax;
        trace->model40_memory_address_failure_ebx = cpu->data.ebx;
        trace->model40_memory_address_failure_ebp = cpu->data.ebp;
        trace->model40_memory_address_failure_eflags = cpu->data.eflags;
    }
    if (observation->point.linear_pc == 0x000fd14cu) {
        ++trace->model40_ram_post_entries;
    } else if (observation->point.linear_pc == 0x000fd1ddu) {
        ++trace->model40_ram_post_returns;
    } else if (observation->point.linear_pc == 0x000fd1eau) {
        ++trace->model40_ram_post_failures;
    }
    if (observation->point.linear_pc == 0x000fa660u) {
        ++trace->model40_resume_entries;
        trace->model40_resume_predecessor = trace->last_linear_pc;
        trace->model40_resume_ax = trace->machine == STD_NULL ? 0u :
            (type_unsigned_16)trace->machine->executor_cpu.data.eax;
    } else if (observation->point.linear_pc == 0x000fb8deu) {
        ++trace->model40_post_setup_entries;
        trace->model40_post_setup_predecessor = trace->last_linear_pc;
    } else if (observation->point.linear_pc == 0x000fb9b5u) {
        ++trace->model40_video_clear_entries;
    } else if (observation->point.linear_pc == 0x000fb9d5u) {
        ++trace->model40_video_delay_entries;
        trace->model40_video_delay_predecessor = trace->last_linear_pc;
    } else if (observation->point.linear_pc >= 0x000fd41du &&
        observation->point.linear_pc <= 0x000fd49eu) {
        STD_SIZE_T post_latch_index = VM_BYOB_MODEL40_POST_LATCH_PATHS;

        switch (observation->point.linear_pc) {
        case 0x000fd41du: post_latch_index = 0u; break;
        case 0x000fd434u: post_latch_index = 1u; break;
        case 0x000fd44cu: post_latch_index = 2u; break;
        case 0x000fd461u: post_latch_index = 3u; break;
        case 0x000fd49eu: post_latch_index = 4u; break;
        }
        if (post_latch_index < VM_BYOB_MODEL40_POST_LATCH_PATHS) {
            ++trace->model40_post_latch_path_entries[post_latch_index];
            trace->model40_post_latch_path_predecessors[post_latch_index] =
                trace->last_linear_pc;
        }
    } else if (observation->point.linear_pc == 0x000fd4b4u) {
        ++trace->model40_resume_prompt_entries;
        trace->model40_resume_prompt_predecessor = trace->last_linear_pc;
    } else if (observation->point.linear_pc == 0x000fd501u) {
        ++trace->model40_resume_wait_entries;
        trace->model40_resume_wait_predecessor = trace->last_linear_pc;
    }
    if (observation->point.linear_pc == 0x000fc7fbu) {
        const STD_SIZE_T helper_index = (STD_SIZE_T)(
            trace->model40_post_status_helper_entries %
            VM_BYOB_MODEL40_POST_STATUS_HELPER_HISTORY);

        trace->model40_post_status_helper_predecessors[helper_index] =
            trace->last_linear_pc;
        ++trace->model40_post_status_helper_entries;
        trace->model40_post_status_helper_predecessor = trace->last_linear_pc;
    } else if (observation->point.linear_pc == 0x000fc043u && trace->machine != STD_NULL) {
        ++trace->model40_memory_address_test_entries;
        trace->model40_memory_address_test_eflags = trace->machine->executor_cpu.data.eflags;
        (C_VOID)core_machine_memory_read_physical(&trace->machine->executor_memory,
            trace->machine->executor_cpu.data.ds.base + 0x58u,
            (type_virtual_address)&trace->model40_memory_address_test_status,
            sizeof(trace->model40_memory_address_test_status));
    } else if (observation->point.linear_pc == 0x000fc04bu && trace->machine != STD_NULL) {
        ++trace->model40_memory_address_error_entries;
        trace->model40_memory_address_error_predecessor = trace->last_linear_pc;
        trace->model40_memory_address_error_ds = trace->machine->executor_cpu.data.ds.selector;
        trace->model40_memory_address_error_ds_base = trace->machine->executor_cpu.data.ds.base;
        (C_VOID)core_machine_memory_read_physical(&trace->machine->executor_memory,
            trace->model40_memory_address_error_ds_base + 0x58u,
            (type_virtual_address)&trace->model40_memory_address_error_status,
            sizeof(trace->model40_memory_address_error_status));
    } else if (observation->point.linear_pc == 0x000f8720u && trace->machine != STD_NULL) {
        const STD_SIZE_T history_index = (STD_SIZE_T)(
            trace->model40_memory_pattern_entries % VM_BYOB_MODEL40_MEMORY_BASE_HISTORY);

        trace->model40_memory_pattern_bases[history_index] =
            trace->machine->executor_cpu.data.es.base;
        if (trace->model40_memory_pattern_entries == 0u) {
            trace->model40_memory_pattern_es_base = trace->machine->executor_cpu.data.es.base;
            trace->model40_memory_pattern_es_selector =
                trace->machine->executor_cpu.data.es.selector;
        }
        ++trace->model40_memory_pattern_entries;
    } else if (observation->point.linear_pc == 0x000f8738u && trace->machine != STD_NULL &&
        trace->model40_memory_pattern_after_edi == 0u) {
        trace->model40_memory_pattern_after_es_base =
            trace->machine->executor_cpu.data.es.base;
        trace->model40_memory_pattern_after_eax = trace->machine->executor_cpu.data.eax;
        trace->model40_memory_pattern_after_edi = trace->machine->executor_cpu.data.edi;
        {
            const type_unsigned_32 physical =
                trace->model40_memory_pattern_after_es_base +
                (type_unsigned_16)((trace->model40_memory_pattern_after_edi - 4u) & 0xffffu);

            if (physical >= 0x00fa0000u && physical <= 0x00fffffeu) {
                (C_VOID)core_machine_memory_read_physical(
                    &trace->machine->executor_memory, physical,
                    (type_virtual_address)&trace->model40_memory_pattern_after_value,
                    sizeof(trace->model40_memory_pattern_after_value));
            }
        }
    } else if (observation->point.linear_pc == 0x000fc1cbu && trace->machine != STD_NULL) {
        type_unsigned_32 physical = trace->machine->executor_cpu.data.es.base +
            (type_unsigned_16)((trace->machine->executor_cpu.data.edi - 2u) & 0xffffu);
        const STD_SIZE_T history_index = (STD_SIZE_T)(
            trace->model40_memory_compare_failures % VM_BYOB_MODEL40_MEMORY_BASE_HISTORY);

        trace->model40_memory_compare_bases[history_index] =
            trace->machine->executor_cpu.data.es.base;
        ++trace->model40_memory_compare_failures;
        trace->model40_memory_compare_eax = trace->machine->executor_cpu.data.eax;
        trace->model40_memory_compare_edi = trace->machine->executor_cpu.data.edi;
        trace->model40_memory_compare_es_base = trace->machine->executor_cpu.data.es.base;
        trace->model40_memory_compare_es_selector =
            trace->machine->executor_cpu.data.es.selector;
        trace->model40_memory_compare_cr0 = trace->machine->executor_cpu.data.cr0;
        trace->model40_memory_compare_gdtr_base = trace->machine->executor_cpu.data.gdtr.base;
        trace->model40_memory_compare_gdtr_limit = trace->machine->executor_cpu.data.gdtr.limit;
        (C_VOID)core_machine_memory_read(trace->machine,
            trace->model40_memory_compare_gdtr_base + 0x48u,
            trace->model40_memory_compare_descriptor,
            sizeof(trace->model40_memory_compare_descriptor));
        if (physical >= 0x00fa0000u && physical <= 0x00fffffeu) {
            (C_VOID)core_machine_memory_read_physical(
                &trace->machine->executor_memory, physical,
                (type_virtual_address)&trace->model40_memory_compare_value,
                sizeof(trace->model40_memory_compare_value));
        }
        ++trace->model40_memory_scas_entries;
        trace->model40_memory_scas_ax = (type_unsigned_16)trace->machine->executor_cpu.data.eax;
        trace->model40_memory_scas_di = (type_unsigned_16)trace->machine->executor_cpu.data.edi;
        trace->model40_memory_scas_es = trace->machine->executor_cpu.data.es.selector;
        trace->model40_memory_scas_es_base = trace->machine->executor_cpu.data.es.base;
        trace->model40_memory_scas_eflags = trace->machine->executor_cpu.data.eflags;
    } else if (observation->point.linear_pc == 0x000fc1ccu &&
        observation->control_outcome == CORE_MACHINE_RETIREMENT_CONTROL_TAKEN &&
        trace->machine != STD_NULL) {
        ++trace->model40_memory_compare_branch_taken;
        trace->model40_memory_compare_eax = trace->machine->executor_cpu.data.eax;
        trace->model40_memory_compare_edi = trace->machine->executor_cpu.data.edi;
        trace->model40_memory_compare_es_selector =
            trace->machine->executor_cpu.data.es.selector;
        trace->model40_memory_compare_es_base = trace->machine->executor_cpu.data.es.base;
        {
            const type_unsigned_32 physical = trace->model40_memory_compare_es_base +
                (type_unsigned_16)((trace->model40_memory_compare_edi - 2u) & 0xffffu);

            if (physical >= 0x00fa0000u && physical <= 0x00fffffeu) {
                (C_VOID)core_machine_memory_read_physical(
                    &trace->machine->executor_memory, physical,
                    (type_virtual_address)&trace->model40_memory_compare_value,
                    sizeof(trace->model40_memory_compare_value));
            }
        }
    } else if (observation->point.linear_pc == 0x000fc207u &&
        trace->machine != STD_NULL) {
        const type_unsigned_32 physical = trace->machine->executor_cpu.data.es.base +
            (type_unsigned_16)trace->machine->executor_cpu.data.edi;

        ++trace->model40_memory_mismatch_entries;
        trace->model40_memory_mismatch_ax = (type_unsigned_16)trace->machine->executor_cpu.data.eax;
        trace->model40_memory_mismatch_di = (type_unsigned_16)trace->machine->executor_cpu.data.edi;
        trace->model40_memory_mismatch_es = trace->machine->executor_cpu.data.es.selector;
        trace->model40_memory_mismatch_es_base = trace->machine->executor_cpu.data.es.base;
        trace->model40_memory_b_window_writes_at_mismatch =
            trace->model40_memory_b_window_writes;
        trace->model40_memory_b_window_last_pc_at_mismatch =
            trace->model40_memory_b_window_last_pc;
        (C_VOID)core_machine_memory_read_physical(&trace->machine->executor_memory, physical,
            (type_virtual_address)&trace->model40_memory_mismatch_value,
            sizeof(trace->model40_memory_mismatch_value));
        if (trace->model40_memory_status_test_active) {
            ++trace->model40_memory_status_test_mismatches;
            trace->model40_memory_status_test_expected =
                (type_unsigned_16)trace->machine->executor_cpu.data.eax;
            trace->model40_memory_status_test_offset =
                (type_unsigned_16)trace->machine->executor_cpu.data.edi;
            trace->model40_memory_status_test_es =
                trace->machine->executor_cpu.data.es.selector;
            trace->model40_memory_status_test_es_base =
                trace->machine->executor_cpu.data.es.base;
            trace->model40_memory_status_test_cr0 =
                trace->machine->executor_cpu.data.cr0;
            (C_VOID)core_machine_memory_read_physical(&trace->machine->executor_memory,
                trace->model40_memory_status_test_es_base +
                    trace->model40_memory_status_test_offset,
                (type_virtual_address)&trace->model40_memory_status_test_actual,
                sizeof(trace->model40_memory_status_test_actual));
        }
    } else if ((observation->point.linear_pc == 0x000fbe62u ||
        observation->point.linear_pc == 0x000fbe86u) &&
        trace->machine != STD_NULL) {
        ++trace->model40_memory_test_return_entries;
        trace->model40_memory_test_return_ax =
            (type_unsigned_16)trace->machine->executor_cpu.data.eax;
    } else if ((observation->point.linear_pc == 0x000f85b4u ||
        observation->point.linear_pc == 0x000fbc8au ||
        observation->point.linear_pc == 0x000fbcf3u ||
        observation->point.linear_pc == 0x000fbd51u ||
        observation->point.linear_pc == 0x000fbd9bu ||
        observation->point.linear_pc == 0x000fbdd4u ||
        observation->point.linear_pc == 0x000fbe09u ||
        observation->point.linear_pc == 0x000fbe38u ||
        observation->point.linear_pc == 0x000fbe67u ||
        observation->point.linear_pc == 0x000fbe8bu ||
        observation->point.linear_pc == 0x000fc084u ||
        observation->point.linear_pc == 0x000fc0acu) && trace->machine != STD_NULL) {
        ++trace->model40_post_status_58_writes;
        trace->model40_post_status_58_last_pc = observation->point.linear_pc;
        (C_VOID)core_machine_memory_read_physical(&trace->machine->executor_memory,
            0x00000458u, (type_virtual_address)&trace->model40_post_status_58_last_value,
            sizeof(trace->model40_post_status_58_last_value));
    } else {
        STD_SIZE_T post_status_index = VM_BYOB_MODEL40_POST_STATUS_WRITERS;

        switch (observation->point.linear_pc) {
        case 0x000f8324u: post_status_index = 0u; break;
        case 0x000fbbd1u: post_status_index = 1u; break;
        case 0x000fc801u: post_status_index = 2u; break;
        case 0x000fd30cu: post_status_index = 3u; break;
        case 0x000fd323u: post_status_index = 4u; break;
        case 0x000fd41du: post_status_index = 5u; break;
        case 0x000fd434u: post_status_index = 6u; break;
        case 0x000fd44cu: post_status_index = 7u; break;
        case 0x000fd461u: post_status_index = 8u; break;
        case 0x000fd49eu: post_status_index = 9u; break;
        case 0x000ff4e2u: post_status_index = 10u; break;
        case 0x000ff54du: post_status_index = 11u; break;
        }
        if (post_status_index < VM_BYOB_MODEL40_POST_STATUS_WRITERS) {
            ++trace->model40_post_status_writer_entries[post_status_index];
            trace->model40_post_status_writer_predecessors[post_status_index] =
                trace->last_linear_pc;
        }
        if (observation->point.linear_pc == 0x000fc801u) {
            (C_VOID)core_machine_memory_read(trace->machine, 0x00000412u,
                &trace->model40_post_status_value, 1u);
        }
    }
    if (!trace->real_286_high_flags_observed && trace->machine != STD_NULL &&
        trace->machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 &&
        (trace->machine->executor_cpu.data.cr0 & VCPU_CR0_PE) == 0u &&
        (trace->machine->executor_cpu.data.eflags & 0xf000u) != 0u) {
        trace->real_286_high_flags_observed = TYPE_TRUE;
        trace->real_286_high_flags_point = observation->point;
        trace->real_286_high_flags_value = trace->machine->executor_cpu.data.eflags;
    }
    if (trace->machine != STD_NULL &&
        (trace->machine->executor_cpu.data.cs.base +
            trace->machine->executor_cpu.data.eip == 0x0001450fu ||
         trace->machine->executor_cpu.data.cs.base +
            trace->machine->executor_cpu.data.eip == 0x000145efu)) {
        const type_unsigned_8 index = (type_unsigned_8)(
            trace->near_ud_count % VM_BYOB_NEAR_UD_HISTORY);

        trace->near_ud_history[index] = observation->point;
        trace->near_ud_eflags[index] = trace->machine == STD_NULL ? 0u :
            trace->machine->executor_cpu.data.eflags;
        trace->near_ud_eax[index] = trace->machine == STD_NULL ? 0u :
            trace->machine->executor_cpu.data.eax;
        trace->near_ud_ebx[index] = trace->machine == STD_NULL ? 0u :
            trace->machine->executor_cpu.data.ebx;
        ++trace->near_ud_count;
    }
    if (observation->point.linear_pc == trace->int15_linear &&
            trace->machine != STD_NULL) {
        ++trace->int15_calls;
        trace->last_int15_ah = (type_unsigned_8)(
            trace->machine->executor_cpu.data.eax >> 8u);
        if (trace->last_int15_ah == 0x24u) trace->int15_ah_seen |= 0x01u;
        if (trace->last_int15_ah == 0x87u) trace->int15_ah_seen |= 0x02u;
        if (trace->last_int15_ah == 0x88u) trace->int15_ah_seen |= 0x04u;
        if (trace->last_int15_ah == 0xc0u) trace->int15_ah_seen |= 0x08u;
    }
    if (observation->point.byte_count >= 3u &&
            observation->point.bytes[0u] == 0x0fu &&
            observation->point.bytes[1u] == 0x01u) {
        if ((observation->point.bytes[2u] & 0x38u) == 0u) ++trace->sgdt_calls;
        if ((observation->point.bytes[2u] & 0x38u) == 0x08u) ++trace->sidt_calls;
    }
    if (observation->point.byte_count >= 1u &&
            observation->point.bytes[0u] == 0x9cu && trace->machine != STD_NULL) {
        ++trace->pushf_calls;
        trace->last_pushf_high = (type_unsigned_16)(
            trace->machine->executor_cpu.data.eflags & 0xf000u);
    }
    if (observation->point.linear_pc == 0x00007cebu) {
        if (trace->machine != STD_NULL) {
            const type_unsigned_16 ds = trace->machine->executor_cpu.data.ds.selector;
            const type_unsigned_16 es = trace->machine->executor_cpu.data.es.selector;
            const type_unsigned_16 si = (type_unsigned_16)trace->machine->executor_cpu.data.esi;
            const type_unsigned_16 di = (type_unsigned_16)trace->machine->executor_cpu.data.edi;

            trace->boot_loader_ds = ds;
            trace->boot_loader_es = es;
            trace->boot_loader_si = si;
            trace->boot_loader_di = di;
            trace->boot_loader_cx = (type_unsigned_16)trace->machine->executor_cpu.data.ecx;
            trace->boot_loader_flags =
                (type_unsigned_16)trace->machine->executor_cpu.data.eflags;
            (C_VOID)core_machine_memory_read(trace->machine, (type_unsigned_32)ds * 16u + si,
                trace->boot_loader_left, sizeof(trace->boot_loader_left));
            (C_VOID)core_machine_memory_read(trace->machine, (type_unsigned_32)es * 16u + di,
                trace->boot_loader_right, sizeof(trace->boot_loader_right));
        }
        trace->boot_loader_jz_observed = TYPE_TRUE;
        trace->boot_loader_jz_outcome = observation->control_outcome;
    }
    if (observation->point.linear_pc == 0x00007cedu) {
        trace->boot_loader_error_observed = TYPE_TRUE;
        trace->boot_loader_previous_pc = trace->last_linear_pc;
        trace->boot_loader_previous_outcome = trace->boot_loader_jz_outcome;
    }
    if (observation->point.linear_pc == 0x00007d30u && trace->machine != STD_NULL) {
        trace->boot_loader_read_return_observed = TYPE_TRUE;
        trace->boot_loader_read_return_flags =
            (type_unsigned_16)trace->machine->executor_cpu.data.eflags;
        (C_VOID)core_machine_memory_read(trace->machine, 0x003eu,
            trace->boot_loader_int13_state, sizeof(trace->boot_loader_int13_state));
    }
    trace->last_linear_pc = observation->point.linear_pc;
}

static C_VOID vm_byob_trace(C_VOID *context, const core_machine_trace_event *event)
{
    vm_byob_boot_trace *trace = (vm_byob_boot_trace *)context;

    if (trace == STD_NULL || event == STD_NULL) return;
    if (event->type == CORE_MACHINE_TRACE_RESET) {
        ++trace->reset_events;
        return;
    }
    if (event->type == CORE_MACHINE_TRACE_STOP &&
        event->detail == CORE_MACHINE_STOP_RESET_REQUESTED && trace->machine != STD_NULL) {
        trace->model40_shutdown_diagnostic_valid =
            core_machine_get_cpu_diagnostic(trace->machine,
                &trace->model40_shutdown_diagnostic) == TYPE_STATUS_OK;
        trace->model40_protected_transition_observed =
            core_machine_memory_read(trace->machine, 0x0009f300u,
                trace->model40_gdt, sizeof(trace->model40_gdt)) == TYPE_STATUS_OK;
    }
    vm_byob_kbc_port_record(trace, event);
    if (event->type == CORE_MACHINE_TRACE_MEMORY_READ &&
        event->address >= 0x000f0000u && event->address < 0x00100000u) {
        ++trace->rom_memory_reads;
        return;
    }
    if (event->type == CORE_MACHINE_TRACE_MEMORY_WRITE &&
        event->address >= 0x000b8000u && event->address < 0x000bc000u) {
        ++trace->cga_memory_writes;
        return;
    }
    if (event->type == CORE_MACHINE_TRACE_MEMORY_WRITE &&
        event->address == 0x00000412u) {
        ++trace->model40_post_latch_writes;
        trace->model40_post_latch_last_pc = event->linear_pc;
        trace->model40_post_latch_last_value = (type_unsigned_8)event->value;
        return;
    }
    if (event->type == CORE_MACHINE_TRACE_MEMORY_WRITE &&
        event->address >= 0x001e0000u && event->address < 0x001e0010u) {
        ++trace->model40_memory_1e_writes;
        trace->model40_memory_1e_last_pc = event->linear_pc;
        trace->model40_memory_1e_last_value = (type_unsigned_8)event->value;
    }
    if (event->type == CORE_MACHINE_TRACE_MEMORY_WRITE &&
        event->address >= 0x0018u && event->address < 0x001cu) {
        const type_unsigned_8 index = (type_unsigned_8)(event->address - 0x0018u);

        trace->int6_vector_write_bytes[index] = (type_unsigned_8)event->value;
        trace->int6_vector_write_mask |= (type_unsigned_8)(1u << index);
        return;
    }
    if (event->type == CORE_MACHINE_TRACE_CPU_RETIRE) {
        if (event->linear_pc == 0x000fa5d3u) {
            ++trace->model40_reset_vector_target_entries;
            trace->model40_reset_vector_target_predecessor = trace->last_linear_pc;
        }
        ++trace->cpu_retires;
        trace->last_linear_pc = event->linear_pc;
        if (event->linear_pc >= 0x000fe360u && event->linear_pc < 0x000fe37cu)
            ++trace->pit_wait_first_retires;
        if (event->linear_pc >= 0x000fe37cu && event->linear_pc < 0x000fe398u)
            ++trace->pit_wait_second_retires;
        if (event->linear_pc >= 0x000fe390u && event->linear_pc < 0x000fe3a6u)
            ++trace->irq0_retires;
        return;
    }
    if (event->type == CORE_MACHINE_TRACE_TRANSACTION_BEGIN &&
        (event->detail & 0xffu) == CORE_MACHINE_TRANSACTION_OWNER_CPU &&
        ((event->detail >> 8u) & 0xffu) ==
            CORE_MACHINE_TRANSACTION_CPU_PORT_WRITE &&
        event->address >= 0x03f0u && event->address <= 0x03f7u) {
        core_machine_trace_event write = *event;

        write.type = CORE_MACHINE_TRACE_PORT_WRITE;
        vm_byob_fdc_port_record(trace, &write);
        ++trace->fdc_port_accesses;
        return;
    }
    if (event->type == CORE_MACHINE_TRACE_TRANSACTION_BEGIN &&
        (event->detail & 0xffu) == CORE_MACHINE_TRANSACTION_OWNER_CPU &&
        ((event->detail >> 8u) & 0xffu) ==
            CORE_MACHINE_TRANSACTION_CPU_PORT_WRITE &&
        (event->address == 0x0060u || event->address == 0x0064u)) {
        core_machine_trace_event write = *event;

        write.type = CORE_MACHINE_TRACE_PORT_WRITE;
        vm_byob_kbc_port_record(trace, &write);
        return;
    }
    if (event->type == CORE_MACHINE_TRACE_TRANSACTION_BEGIN &&
        (event->detail & 0xffu) == CORE_MACHINE_TRANSACTION_OWNER_CPU &&
        ((event->detail >> 8u) & 0xffu) ==
            CORE_MACHINE_TRANSACTION_CPU_PORT_WRITE &&
        event->address >= 0x01f0u && event->address <= 0x01f7u) {
        core_machine_trace_event write = *event;

        write.type = CORE_MACHINE_TRACE_PORT_WRITE;
        vm_byob_hdc_port_record(trace, &write);
        ++trace->hdc_port_accesses;
        return;
    }
    if (event->type == CORE_MACHINE_TRACE_TRANSACTION_COMMIT &&
        (event->detail & 0xffu) == CORE_MACHINE_TRANSACTION_OWNER_CPU &&
        ((event->detail >> 8u) & 0xffu) ==
            CORE_MACHINE_TRANSACTION_CPU_PORT_READ &&
        event->address >= 0x03f0u && event->address <= 0x03f7u) {
        core_machine_trace_event read = *event;

        read.type = CORE_MACHINE_TRACE_PORT_READ;
        vm_byob_fdc_port_record(trace, &read);
        ++trace->fdc_port_accesses;
        return;
    }
    if (event->type == CORE_MACHINE_TRACE_TRANSACTION_COMMIT &&
        (event->detail & 0xffu) == CORE_MACHINE_TRANSACTION_OWNER_CPU &&
        ((event->detail >> 8u) & 0xffu) ==
            CORE_MACHINE_TRANSACTION_CPU_PORT_READ &&
        event->address >= 0x01f0u && event->address <= 0x01f7u) {
        core_machine_trace_event read = *event;

        read.type = CORE_MACHINE_TRACE_PORT_READ;
        vm_byob_hdc_port_record(trace, &read);
        ++trace->hdc_port_accesses;
        return;
    }
    if (event->type != CORE_MACHINE_TRACE_PORT_READ &&
        event->type != CORE_MACHINE_TRACE_PORT_WRITE) return;
    if (event->type == CORE_MACHINE_TRACE_PORT_WRITE && event->address == 0x0084u) {
        ++trace->model40_video_error_writes;
        trace->model40_video_error_last_value = (type_unsigned_8)event->value;
        trace->model40_video_error_last_pc = event->linear_pc;
    }
    if (event->address >= 0x03f0u && event->address <= 0x03f7u) {
        vm_byob_fdc_port_record(trace, event);
        ++trace->fdc_port_accesses;
    } else if (event->address >= 0x01f0u && event->address <= 0x01f7u) {
        vm_byob_hdc_port_record(trace, event);
        ++trace->hdc_port_accesses;
    } else if (event->address >= 0x0060u && event->address <= 0x0063u) {
        ++trace->xt_ppi_port_accesses;
    } else if (event->address >= 0x0020u && event->address <= 0x0021u) {
        ++trace->pic_port_accesses;
    } else if (event->address >= 0x0040u && event->address <= 0x0043u) {
        ++trace->pit_port_accesses;
    } else if (event->address >= 0x03d0u && event->address <= 0x03dfu) {
        ++trace->cga_port_accesses;
    }
}

static C_VOID vm_byob_model40_memory_write_observe(C_VOID *context,
    type_unsigned_32 physical, type_native_unsigned bytes)
{
    vm_byob_boot_trace *trace = (vm_byob_boot_trace *)context;
    type_unsigned_32 pc;

    if (trace == STD_NULL) return;
    pc = trace->machine == STD_NULL ? 0u : trace->machine->executor_cpu.data.cs.base +
        trace->machine->executor_cpu.data.eip;
    if (physical <= 0x00000042u &&
        (type_unsigned_64)physical + bytes > 0x00000040u) {
        const STD_SIZE_T index = (STD_SIZE_T)(trace->model40_int10_vector_write_count %
            VM_BYOB_MODEL40_INT10_VECTOR_HISTORY);

        trace->model40_int10_vector_write_observed = TYPE_TRUE;
        ++trace->model40_int10_vector_write_count;
        trace->model40_int10_vector_write_pc = pc;
        trace->model40_int10_vector_write_cs =
            trace->machine->executor_cpu.data.cs.selector;
        (C_VOID)core_machine_memory_read_physical(&trace->machine->executor_memory,
            0x00000040u,
            (type_virtual_address)&trace->model40_int10_vector_offset,
            sizeof(trace->model40_int10_vector_offset));
        (C_VOID)core_machine_memory_read_physical(&trace->machine->executor_memory,
            0x00000042u,
            (type_virtual_address)&trace->model40_int10_vector_segment,
            sizeof(trace->model40_int10_vector_segment));
        trace->model40_int10_vector_history_pc[index] = pc;
        trace->model40_int10_vector_history_cs[index] =
            trace->machine->executor_cpu.data.cs.selector;
        trace->model40_int10_vector_history_offset[index] =
            trace->model40_int10_vector_offset;
        trace->model40_int10_vector_history_segment[index] =
            trace->model40_int10_vector_segment;
    }
    if (physical <= 0x0000010au &&
        (type_unsigned_64)physical + bytes > 0x00000108u) {
        const STD_SIZE_T index = (STD_SIZE_T)(trace->model40_int42_vector_write_count %
            VM_BYOB_MODEL40_INT10_VECTOR_HISTORY);

        ++trace->model40_int42_vector_write_count;
        trace->model40_int42_vector_write_pc[index] = pc;
        (C_VOID)core_machine_memory_read_physical(&trace->machine->executor_memory,
            0x00000108u, (type_virtual_address)&trace->model40_int42_vector_history_offset[index],
            sizeof(trace->model40_int42_vector_history_offset[index]));
        (C_VOID)core_machine_memory_read_physical(&trace->machine->executor_memory,
            0x0000010au, (type_virtual_address)&trace->model40_int42_vector_history_segment[index],
            sizeof(trace->model40_int42_vector_history_segment[index]));
    }
    if (physical <= 0x00000458u && (type_unsigned_64)physical + bytes > 0x00000458u) {
        const STD_SIZE_T index = (STD_SIZE_T)(
            trace->model40_post_status_58_observer_writes %
            VM_BYOB_MODEL40_POST_STATUS_58_HISTORY);

        ++trace->model40_post_status_58_observer_writes;
        trace->model40_post_status_58_observer_last_pc = pc;
        (C_VOID)core_machine_memory_read_physical(&trace->machine->executor_memory,
            0x00000458u,
            (type_virtual_address)&trace->model40_post_status_58_observer_last_value,
            sizeof(trace->model40_post_status_58_observer_last_value));
        trace->model40_post_status_58_observer_pcs[index] = pc;
        trace->model40_post_status_58_observer_values[index] =
            trace->model40_post_status_58_observer_last_value;
    }
    if (physical <= VM_BYOB_MODEL40_POST_PRIVATE_STATUS_PHYSICAL &&
        (type_unsigned_64)physical + bytes > VM_BYOB_MODEL40_POST_PRIVATE_STATUS_PHYSICAL) {
        const STD_SIZE_T index = (STD_SIZE_T)(
            trace->model40_post_private_status_writes %
            VM_BYOB_MODEL40_POST_PRIVATE_STATUS_HISTORY);

        ++trace->model40_post_private_status_writes;
        trace->model40_post_private_status_last_pc = pc;
        (C_VOID)core_machine_memory_read_physical(&trace->machine->executor_memory,
            VM_BYOB_MODEL40_POST_PRIVATE_STATUS_PHYSICAL,
            (type_virtual_address)&trace->model40_post_private_status_last_value,
            sizeof(trace->model40_post_private_status_last_value));
        trace->model40_post_private_status_pcs[index] = pc;
        trace->model40_post_private_status_values[index] =
            trace->model40_post_private_status_last_value;
    }
    if (physical >= 0x000b0000u && physical < 0x000c0000u) {
        if (trace->model40_memory_b_window_writes == 0u)
            trace->model40_memory_b_window_first_pc = pc;
        ++trace->model40_memory_b_window_writes;
        trace->model40_memory_b_window_last_pc = pc;
    }
    if (physical < 0x000b0002u && (type_unsigned_64)physical + bytes > 0x000b0000u) {
        ++trace->model40_memory_b_first_word_writes;
        trace->model40_memory_b_first_word_retirements = trace->model40_retirements;
        trace->model40_memory_b_first_word_last_pc = pc;
        (C_VOID)core_machine_memory_read_physical(&trace->machine->executor_memory,
            0x000b0000u, (type_virtual_address)&trace->model40_memory_b_first_word_last_value,
            sizeof(trace->model40_memory_b_first_word_last_value));
        trace->model40_memory_b_first_word_graphics_6 =
            trace->machine->shared_vadp.data.graphics[6u];
        trace->model40_memory_b_first_word_sequencer_0 =
            trace->machine->shared_vadp.data.sequencer[0u];
        trace->model40_memory_b_first_word_video_memory_disabled =
            trace->machine->shared_vadp.data.compaq_cpu_video_memory_disabled;
    }
    if (physical >= 0x00fb0000u && physical < 0x00fc0000u) {
        if (trace->model40_memory_fb_page_writes == 0u)
            trace->model40_memory_fb_page_first_pc = pc;
        ++trace->model40_memory_fb_page_writes;
        trace->model40_memory_fb_page_last_pc = pc;
    }
    if (physical < 0x00fb0010u && (type_unsigned_64)physical + bytes > 0x00fb0000u) {
        const STD_SIZE_T history = (STD_SIZE_T)(trace->model40_memory_high_b_page_writes %
            VM_BYOB_MODEL40_HIGH_B_WRITE_HISTORY);

        if (trace->model40_memory_high_b_page_writes == 0u)
            trace->model40_memory_high_b_page_first_pc = pc;
        ++trace->model40_memory_high_b_page_writes;
        trace->model40_memory_high_b_page_last_pc = pc;
        trace->model40_memory_high_b_write_pcs[history] = pc;
        trace->model40_memory_high_b_write_producer[history] =
            trace->model40_memory_pattern_producer_active;
        if (trace->model40_memory_pattern_producer_active)
            ++trace->model40_memory_pattern_producer_high_b_writes;
        (C_VOID)core_machine_memory_read_physical(&trace->machine->executor_memory,
            0x00fb0000u,
            (type_virtual_address)trace->model40_memory_high_b_write_pages[history],
            sizeof(trace->model40_memory_high_b_write_pages[history]));
    }
    if (physical < 0x00fb0002u && (type_unsigned_64)physical + bytes > 0x00fb0000u) {
        ++trace->model40_memory_fb_first_word_writes;
        trace->model40_memory_fb_first_word_retirements = trace->model40_retirements;
        trace->model40_memory_fb_first_word_last_pc = pc;
        (C_VOID)core_machine_memory_read_physical(&trace->machine->executor_memory,
            0x00fb0000u, (type_virtual_address)&trace->model40_memory_fb_first_word_last_value,
            sizeof(trace->model40_memory_fb_first_word_last_value));
    }
    if (physical < 0x00fa0000u || physical >= 0x01000000u) return;
    if (trace->model40_memory_high_writes == 0u) {
        trace->model40_memory_high_first_address = physical;
        trace->model40_memory_high_first_value = (type_unsigned_8)bytes;
    }
    ++trace->model40_memory_high_writes;
    trace->model40_memory_high_last_address = physical;
    trace->model40_memory_high_last_pc = pc;
    trace->model40_memory_high_last_value = (type_unsigned_8)bytes;
    if (trace->model40_memory_high_last_pc == 0x000f8738u) {
        if (trace->model40_memory_pattern_write_count == 0u)
            trace->model40_memory_pattern_write_first_address = physical;
        ++trace->model40_memory_pattern_write_count;
        trace->model40_memory_pattern_write_last_address = physical;
    }
}

static C_INT vm_byob_snapshot_has(const core_machine_display_snapshot *snapshot,
    const C_CHAR *text)
{
    STD_SIZE_T cell;
    STD_SIZE_T length;

    if (snapshot == STD_NULL || text == STD_NULL ||
        snapshot->kind != CORE_MACHINE_DISPLAY_KIND_TEXT) return 0;
    length = STD_STRLEN(text);
    for (cell = 0u; cell + length <= (STD_SIZE_T)snapshot->columns * snapshot->rows;
        ++cell) {
        if (!STD_MEMCMP(snapshot->characters + cell, text, length)) return 1;
    }
    return 0;
}

static C_INT vm_byob_snapshot_has_prompt(const core_machine_display_snapshot *snapshot)
{
    STD_SIZE_T cell;
    STD_SIZE_T cells;

    if (snapshot == STD_NULL || snapshot->kind != CORE_MACHINE_DISPLAY_KIND_TEXT) return 0;
    cells = (STD_SIZE_T)snapshot->columns * snapshot->rows;
    for (cell = 0u; cell + 3u < cells; ++cell) {
        if (STD_ISALPHA(snapshot->characters[cell]) &&
            snapshot->characters[cell + 1u] == ':' &&
            snapshot->characters[cell + 2u] == '\\' &&
            snapshot->characters[cell + 3u] == '>') return 1;
    }
    return 0;
}

static C_INT vm_byob_send_f1(vm_session *session, C_INT pressed,
    type_unsigned_8 *out_scan_set)
{
    core_platform_input_event event = {0};
    type_unsigned_8 scan_set = 0u;

    if (session == STD_NULL || session->core_machine == STD_NULL ||
        out_scan_set == STD_NULL || core_machine_keyboard_get_native_scan_set(
            session->core_machine, &scan_set) !=
            TYPE_STATUS_OK) return 0;
    *out_scan_set = scan_set;
    event.kind = CORE_PLATFORM_INPUT_KEY;
    event.data.key.scan_code = 0x3bu;
    event.data.key.virtual_key = 0x70u;
    event.data.key.pressed = pressed != 0;
    return vm_session_submit_host_input(session, &event) == TYPE_STATUS_OK;
}

static C_INT vm_byob_text_memory_has(core_machine *machine, const C_CHAR *text)
{
    type_unsigned_8 bytes[0x4000];
    STD_SIZE_T offset;
    STD_SIZE_T length;

    if (machine == STD_NULL || text == STD_NULL ||
        core_machine_memory_read(machine, 0x000b8000u, bytes, sizeof(bytes)) !=
            TYPE_STATUS_OK) return 0;
    length = STD_STRLEN(text);
    for (offset = 0u; offset + length * 2u <= sizeof(bytes); offset += 2u) {
        STD_SIZE_T index;

        for (index = 0u; index < length && bytes[offset + index * 2u] == text[index];
            ++index) {}
        if (index == length) return 1;
    }
    return 0;
}

static C_VOID vm_byob_print_text_rows(core_machine *machine)
{
    type_unsigned_8 bytes[80u * 25u * 2u];
    type_unsigned_32 row;

    if (machine == STD_NULL || core_machine_memory_read(machine, 0x000b8000u,
            bytes, sizeof(bytes)) != TYPE_STATUS_OK) return;
    for (row = 0u; row < 25u; ++row) {
        C_CHAR line[81];
        type_unsigned_32 column;
        C_INT nonblank = 0;

        for (column = 0u; column < 80u; ++column) {
            type_unsigned_8 character = bytes[(row * 80u + column) * 2u];

            line[column] = character >= 0x20u && character < 0x7fu ?
                (C_CHAR)character : ' ';
            if (line[column] != ' ') nonblank = 1;
        }
        line[80u] = '\0';
        if (nonblank) STD_PRINTF("BOOT-PROBE=text-row-%u=%s\n",
            (unsigned int)row, line);
    }
}

static C_INT vm_byob_memory_has(core_machine *machine, type_unsigned_32 address,
    STD_SIZE_T byte_count, const C_CHAR *text)
{
    type_unsigned_8 bytes[1024];
    STD_SIZE_T offset;
    STD_SIZE_T length;

    if (machine == STD_NULL || text == STD_NULL || byte_count > sizeof(bytes) ||
        core_machine_memory_read(machine, address, bytes, byte_count) != TYPE_STATUS_OK) {
        return 0;
    }
    length = STD_STRLEN(text);
    for (offset = 0u; offset + length <= byte_count; ++offset) {
        if (!STD_MEMCMP(bytes + offset, text, length)) return 1;
    }
    return 0;
}

static C_INT vm_byob_memory_equal(core_machine *machine, type_unsigned_32 left,
    type_unsigned_32 right, STD_SIZE_T byte_count)
{
    type_unsigned_8 left_bytes[32];
    type_unsigned_8 right_bytes[32];

    return machine != STD_NULL && byte_count <= sizeof(left_bytes) &&
        core_machine_memory_read(machine, left, left_bytes, byte_count) == TYPE_STATUS_OK &&
        core_machine_memory_read(machine, right, right_bytes, byte_count) == TYPE_STATUS_OK &&
        !STD_MEMCMP(left_bytes, right_bytes, byte_count);
}

static type_unsigned_32 vm_byob_snapshot_checksum(
    const core_machine_display_snapshot *snapshot)
{
    type_unsigned_32 checksum = 0u;
    STD_SIZE_T cell;
    STD_SIZE_T cells;

    if (snapshot == STD_NULL) return 0u;
    cells = (STD_SIZE_T)snapshot->columns * snapshot->rows;
    for (cell = 0u; cell < cells; ++cell) {
        checksum = checksum * 33u + snapshot->characters[cell];
    }
    return checksum;
}

static C_INT vm_byob_parse_cpu(const C_CHAR *text,
    core_machine_cpu_profile *out_profile)
{
    if (text == STD_NULL || out_profile == STD_NULL) return 0;
    if (!STD_STRCMP(text, "8086")) *out_profile = CORE_MACHINE_CPU_PROFILE_8086;
    else if (!STD_STRCMP(text, "8088")) *out_profile = CORE_MACHINE_CPU_PROFILE_8088;
    else if (!STD_STRCMP(text, "80186")) *out_profile = CORE_MACHINE_CPU_PROFILE_80186;
    else if (!STD_STRCMP(text, "80286")) *out_profile = CORE_MACHINE_CPU_PROFILE_80286;
    else if (!STD_STRCMP(text, "80386")) *out_profile = CORE_MACHINE_CPU_PROFILE_80386;
    else return 0;
    return 1;
}

static C_INT vm_byob_parse_floppy_format(const C_CHAR *text,
    vm_session_floppy_format *out_format)
{
    if (text == STD_NULL || out_format == STD_NULL) return 0;
    if (!STD_STRCMP(text, "360k")) *out_format = VM_SESSION_FLOPPY_FORMAT_360K;
    else if (!STD_STRCMP(text, "720k")) *out_format = VM_SESSION_FLOPPY_FORMAT_720K;
    else if (!STD_STRCMP(text, "1200k")) *out_format = VM_SESSION_FLOPPY_FORMAT_1200K;
    else if (!STD_STRCMP(text, "1440k")) *out_format = VM_SESSION_FLOPPY_FORMAT_1440K;
    else return 0;
    return 1;
}

static C_INT vm_byob_parse_ibm_5170_memory(const C_CHAR *text,
    STD_SIZE_T *out_memory_bytes)
{
    if (text == STD_NULL || out_memory_bytes == STD_NULL) return 0;
    if (!STD_STRCMP(text, "512k")) *out_memory_bytes = 512u * 1024u;
    else if (!STD_STRCMP(text, "640k")) *out_memory_bytes = 640u * 1024u;
    else if (!STD_STRCMP(text, "1536k")) *out_memory_bytes = 1536u * 1024u;
    else if (!STD_STRCMP(text, "2048k")) *out_memory_bytes = 2048u * 1024u;
    else if (!STD_STRCMP(text, "2560k")) *out_memory_bytes = 2560u * 1024u;
    else if (!STD_STRCMP(text, "3072k")) *out_memory_bytes = 3072u * 1024u;
    else return 0;
    return 1;
}

static C_INT vm_byob_configure(C_INT argc, C_CHAR **argv, vm_session_config *config)
{
    if (argc < 3 || argv == STD_NULL || config == STD_NULL) return 0;
    STD_MEMSET(config, 0, sizeof(*config));
    config->floppy_image[0u] = argv[2];
    if (!STD_STRCMP(argv[1], "ibm-5160-model-268")) {
        if (argc != 5) return 0;
        config->profile_kind = VM_SESSION_PROFILE_IBM_5160_MODEL_268;
        config->bios_path[0u] = argv[3];
        config->bios_count = 1u;
        return 1;
    }
    if (!STD_STRCMP(argv[1], "ibm-5170-model-339")) {
        config->profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339;
        if (argc == 3) return 1;
        if (argc == 4) return vm_byob_parse_floppy_format(argv[3], &config->floppy_format) ||
            vm_byob_parse_ibm_5170_memory(argv[3], &config->memory_bytes);
        return argc == 5 && vm_byob_parse_floppy_format(argv[3], &config->floppy_format) &&
            vm_byob_parse_ibm_5170_memory(argv[4], &config->memory_bytes);
    }
    if (!STD_STRCMP(argv[1], "compaq-deskpro-386-model-40")) {
        if (argc != 7 && argc != 8 && argc != 9 && argc != 10) return 0;
        config->profile_kind = VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40;
        config->bios_path[0u] = argv[3];
        config->bios_path[1u] = argv[5];
        config->bios_count = 2u;
        config->video_path = argc >= 9 ? argv[7] : STD_NULL;
        if (argc == 8) return vm_byob_parse_floppy_format(argv[7],
            &config->floppy_format);
        if (argc == 10) return vm_byob_parse_floppy_format(argv[9],
            &config->floppy_format);
        return 1;
    }
    if (!STD_STRCMP(argv[1], "default-pc-at")) {
        config->profile_kind = VM_SESSION_PROFILE_DEFAULT_PC_AT;
        if (argc != 4 && argc != 5) return 0;
        if (!vm_byob_parse_cpu(argv[3], &config->cpu_profile)) return 0;
        return argc == 4 || vm_byob_parse_floppy_format(argv[4], &config->floppy_format);
    }
    return 0;
}

int main(C_INT argc, C_CHAR **argv)
{
    /* Keep the host-side diagnostic wall-clock budget observable even when a
       guest instruction is stalled behind an unbounded Core wait path. */
    core_machine_run_budget budget = {256u, 256u};
    vm_session_config config;
    vm_session *session = STD_NULL;
    core_machine_run_result result;
    core_machine_display_snapshot snapshot;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_trace_provider trace_provider;
    vm_byob_boot_trace trace = {0};
    ULONGLONG started;
    ULONGLONG progress;
    ULONGLONG next_display_capture;
    type_unsigned_32 checksum = 0u;
    type_unsigned_32 linear_pc = 0u;
    type_unsigned_32 waiting_linear_pc = 0u;
    type_status status;
    type_unsigned_16 post_caller_offset = 0u;
    type_unsigned_16 int6_offset = 0u;
    type_unsigned_16 int6_segment = 0u;
    type_unsigned_8 post_interrupt_flag = 0u;
    type_unsigned_8 post_manufacturing_error_flag = 0u;
    type_unsigned_8 last_fdc_command = 0u;
    type_unsigned_8 last_fdc_bytes[9] = {0};
    type_unsigned_8 last_fdc_result[3] = {0u, 0u, 0u};
    type_unsigned_8 last_fdc_phase = 0u;
    type_unsigned_32 last_fdc_remaining = 0u;
    type_unsigned_16 last_dma_address = 0u;
    type_unsigned_16 last_dma_count = 0u;
    type_unsigned_8 last_dma_mode = 0u;
    type_unsigned_64 executed_total = 0u;
    type_unsigned_64 run_count = 0u;
    type_unsigned_32 last_reason = CORE_MACHINE_STOP_NONE;
    type_unsigned_32 last_detail = 0u;
    C_INT last_wait_advanced = 0;
    type_unsigned_32 wall_limit = VM_BYOB_BOOT_WALL_LIMIT_MILLISECONDS;
    type_unsigned_32 no_progress_limit = VM_BYOB_BOOT_NO_PROGRESS_LIMIT_MILLISECONDS;
    ULONGLONG resume_f1_prompt_at = 0u;
    ULONGLONG resume_f1_make_at = 0u;
    ULONGLONG next_text_memory_scan;
    C_INT have_checksum = 0;
    C_INT have_linear_pc = 0;
    C_INT waiting_for_interrupt = 0;
    C_INT waiting_with_deadline = 0;
    C_INT waiting_interrupts_enabled = 0;
    C_INT waiting_in_rom = 0;
    C_INT waiting_in_low_memory = 0;
    C_INT post_memory_failure = 0;
    C_INT post_keyboard_failure = 0;
    C_INT post_floppy_failure = 0;
    C_INT post_resume_required = 0;
    C_INT resume_f1_sent = 0;
    C_INT short_budget;
    C_INT stop_at_first_exception;
    C_INT trace_enabled;
    C_INT turbo;
    C_INT no_retirement_observation;
    C_INT press_resume_f1;
    const C_CHAR *hdd_image = STD_NULL;
    C_INT exit_code = 1;

    stop_at_first_exception = 0;
    short_budget = 0;
    trace_enabled = 0;
    no_retirement_observation = 0;
    press_resume_f1 = 0;
    turbo = 0;
    while (argc > 1) {
        const C_CHAR *option = argv[argc - 1];

        if (argc > 2 && !STD_STRCMP(argv[argc - 2], "--hdd")) {
            hdd_image = option;
            argc -= 2;
            continue;
        }
        else if (!STD_STRCMP(option, "--first-exception")) stop_at_first_exception = 1;
        else if (!STD_STRCMP(option, "--short")) short_budget = 1;
        else if (!STD_STRCMP(option, "--trace")) trace_enabled = 1;
        else if (!STD_STRCMP(option, "--no-retirement-observation"))
            no_retirement_observation = 1;
        else if (!STD_STRCMP(option, "--resume-f1")) press_resume_f1 = 1;
        else if (!STD_STRCMP(option, "--turbo")) turbo = 1;
        else break;
        --argc;
    }
    if (short_budget) {
        wall_limit = 25000u;
        no_progress_limit = 7500u;
    }
    if (!vm_byob_configure(argc, argv, &config)) {
        STD_PRINTF("BOOT-PROBE=invalid-arguments\n");
        goto done;
    }
    config.fixed_disk_image[0u] = hdd_image;
    status = vm_session_create(&config, &session);
    if (status != TYPE_STATUS_OK || session == STD_NULL) {
        STD_PRINTF("BOOT-PROBE=session-create-failed-status=%u\n",
            (type_unsigned_32)status);
        goto done;
    }
    {
        core_machine_memory_route boundary_route = CORE_MACHINE_MEMORY_ROUTE_PROVIDER;
        const type_status boundary_status = core_machine_memory_query_physical(
            &session->core_machine->executor_memory, 0x00100000u, 2u,
            CORE_MACHINE_MEMORY_ACCESS_WRITE, &boundary_route);

        STD_PRINTF("BOOT-PROBE=memory-installed=%llu-a20=%u-boundary-write-status=%u-route=%u\n",
            (unsigned long long)session->core_machine->executor_memory.connect.installed_bytes,
            (unsigned int)session->core_machine->executor_memory.data.flagA20,
            (unsigned int)boundary_status, (unsigned int)boundary_route);
    }
    if (session->core_machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ||
        session->core_machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80386) {
        type_unsigned_8 reset_bytes[4] = {0u};
        const type_unsigned_32 reset_physical =
            session->core_machine->cpu_profile == CORE_MACHINE_CPU_PROFILE_80286 ?
                0x00fffff0u : 0xfffffff0u;
        STD_SIZE_T index;

        (C_VOID)core_machine_memory_read_reset_physical(
            &session->core_machine->executor_memory, reset_physical,
            (type_virtual_address)reset_bytes, sizeof(reset_bytes));
        STD_PRINTF("BOOT-PROBE=reset-read=%02X,%02X,%02X,%02X-rom-mappings=%u\n",
            (unsigned int)reset_bytes[0u], (unsigned int)reset_bytes[1u],
            (unsigned int)reset_bytes[2u], (unsigned int)reset_bytes[3u],
            (unsigned int)session->core_machine->immutable_rom_mapping_count);
        for (index = 0u; index < session->core_machine->immutable_rom_mapping_count;
                ++index) {
            const core_machine_immutable_rom_mapping *mapping =
                &session->core_machine->immutable_rom_mappings[index];

            STD_PRINTF("BOOT-PROBE=rom-map-%u=%08X/%u\n", (unsigned int)index,
                (unsigned int)mapping->physical_start, (unsigned int)mapping->bytes);
        }
    }
    trace.machine = session->core_machine;
    if (config.profile_kind == VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40) {
        t_ram *memory = &session->core_machine->executor_memory;

        if (memory->connect.write_observer_count <
            CORE_MACHINE_MEMORY_WRITE_OBSERVER_CAPACITY) {
            memory->connect.write_observers[memory->connect.write_observer_count++] =
                (core_machine_memory_write_observer_slot) {
                    vm_byob_model40_memory_write_observe, &trace };
        } else {
            STD_PRINTF("BOOT-PROBE=setup-failed\n");
            goto done;
        }
        (C_VOID)core_machine_debug_set_watchpoint(session->core_machine,
            CORE_MACHINE_DEBUG_WATCH_WRITE, 0x00fe0000u);
    }
    {
        type_unsigned_16 offset = 0u;
        type_unsigned_16 segment = 0u;

        if (core_machine_memory_read(session->core_machine, 0x0054u, &offset,
                sizeof(offset)) == TYPE_STATUS_OK &&
            core_machine_memory_read(session->core_machine, 0x0056u, &segment,
                sizeof(segment)) == TYPE_STATUS_OK) {
            trace.int15_linear = (type_unsigned_32)segment * 16u + offset;
        }
    }
    if (turbo && vm_session_set_speed(session, VM_SESSION_SPEED_TURBO) != TYPE_STATUS_OK) {
        STD_PRINTF("BOOT-PROBE=setup-failed\n");
        goto done;
    }
    if (turbo) {
        /* The probe has no interactive command/display loop.  A larger,
         * instruction-only quantum matches the production Turbo runner:
         * Core still performs every instruction and deadline, but an
         * unrelated tick ceiling cannot turn this diagnostic into Standard. */
        budget.instructions = 4096u;
        budget.ticks = 0u;
    }
    if (!no_retirement_observation &&
        core_machine_set_retirement_observation_provider(session->core_machine,
            &(core_machine_retirement_observation_provider) {
                vm_byob_retirement_observe, &trace }) != TYPE_STATUS_OK) {
        STD_PRINTF("BOOT-PROBE=setup-failed\n");
        goto done;
    }
    (C_VOID)core_machine_register_memory_write_observer(session->core_machine,
        vm_byob_model40_vector_write_observe, &trace);
    /* This is probe-only observability after construction; the FDC retains the
       sole terminal event path and no guest-visible state is changed. */
    session->core_machine->fdc.connect.observation_provider =
        (core_machine_fdc_terminal_observation_provider) {
            vm_byob_fdc_terminal_observe, &trace };
    if (trace_enabled) {
        trace_provider.callback = vm_byob_trace;
        trace_provider.context = &trace;
        if (core_machine_set_trace_provider(session->core_machine, &trace_provider) !=
            TYPE_STATUS_OK) {
            STD_PRINTF("BOOT-PROBE=setup-failed\n");
            goto done;
        }
    }
    STD_ATOMIC_STORE(&session->control.flagRun, TYPE_TRUE);
    started = GetTickCount64();
    progress = started;
    next_display_capture = started;
    next_text_memory_scan = started;
    while (GetTickCount64() - started < wall_limit) {
        core_machine_run_budget run_budget = budget;
        type_unsigned_32 current;
        ULONGLONG now;

        if (stop_at_first_exception && have_linear_pc &&
            (linear_pc == 0x0001450fu || linear_pc == 0x000145efu) &&
            core_machine_memory_read(session->core_machine, 0x0018u,
                &trace.int6_pre_fault_offset,
                sizeof(trace.int6_pre_fault_offset)) == TYPE_STATUS_OK &&
            core_machine_memory_read(session->core_machine, 0x001au,
                &trace.int6_pre_fault_segment,
                sizeof(trace.int6_pre_fault_segment)) == TYPE_STATUS_OK) {
            trace.int6_pre_fault_snapshot_valid = TYPE_TRUE;
            run_budget.instructions = 1u;
        }
        /* This probe drives Core directly, so it explicitly executes the
         * production runner's command boundary before each Core quantum.
         * Host input stays ordered and Core remains the only state mutator. */
        vm_session_execution_context_run_command_boundary(
            &session->control.execution_context);
        if (core_machine_run(session->core_machine, run_budget, &result) != TYPE_STATUS_OK) {
            type_unsigned_8 fault_bytes[4] = {0u};
            type_unsigned_8 far_pointer[6] = {0u};
            type_unsigned_8 descriptor[8] = {0u};
            const t_cpu *fault_cpu = &session->core_machine->executor_cpu;

            (C_VOID)core_machine_memory_read_physical(&session->core_machine->executor_memory,
                result.linear_pc, (type_virtual_address)fault_bytes, sizeof(fault_bytes));
            (C_VOID)core_machine_memory_read_physical(&session->core_machine->executor_memory,
                0x000f8a1fu, (type_virtual_address)far_pointer, sizeof(far_pointer));
            (C_VOID)core_machine_memory_read_physical(&session->core_machine->executor_memory,
                0x000f0a0bu, (type_virtual_address)descriptor, sizeof(descriptor));
            STD_PRINTF("BOOT-PROBE=run-failed-reason=%u-detail=%08X-pc=%05X\n",
                (unsigned int)result.reason, (unsigned int)result.detail,
                (unsigned int)result.linear_pc);
            STD_PRINTF("BOOT-PROBE=fault-bytes=%02X,%02X,%02X,%02X\n",
                (unsigned int)fault_bytes[0u], (unsigned int)fault_bytes[1u],
                (unsigned int)fault_bytes[2u], (unsigned int)fault_bytes[3u]);
            STD_PRINTF("BOOT-PROBE=cpu-cr0=%08X-cs=%04X-base=%08X-eip=%08X-gdt=%08X/%08X\n",
                (unsigned int)fault_cpu->data.cr0, (unsigned int)fault_cpu->data.cs.selector,
                (unsigned int)fault_cpu->data.cs.base, (unsigned int)fault_cpu->data.eip,
                (unsigned int)fault_cpu->data.gdtr.base, (unsigned int)fault_cpu->data.gdtr.limit);
            STD_PRINTF("BOOT-PROBE=d4-control=%02X-ram-setup=%04X\n",
                (unsigned int)session->core_machine->d4_memory.control,
                (unsigned int)session->core_machine->d4_memory.ram_setup);
            STD_PRINTF("BOOT-PROBE=transaction-owner=%u-hold-owner=%u-hold-ack=%u-refresh-pending=%u\n",
                (unsigned int)session->core_machine->transaction.owner,
                (unsigned int)session->core_machine->transaction.hold_owner,
                (unsigned int)session->core_machine->transaction.hold_acknowledged,
                (unsigned int)session->core_machine->d4_refresh_hold_pending);
            STD_PRINTF("BOOT-PROBE=far-pointer=%02X,%02X,%02X,%02X,%02X,%02X-gdt-entry=%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X\n",
                (unsigned int)far_pointer[0u], (unsigned int)far_pointer[1u],
                (unsigned int)far_pointer[2u], (unsigned int)far_pointer[3u],
                (unsigned int)far_pointer[4u], (unsigned int)far_pointer[5u],
                (unsigned int)descriptor[0u], (unsigned int)descriptor[1u],
                (unsigned int)descriptor[2u], (unsigned int)descriptor[3u],
                (unsigned int)descriptor[4u], (unsigned int)descriptor[5u],
                (unsigned int)descriptor[6u], (unsigned int)descriptor[7u]);
            goto done;
        }
        ++run_count;
        last_reason = result.reason;
        last_detail = result.detail;
        executed_total += result.executed;
        if (stop_at_first_exception && core_machine_get_cpu_diagnostic(
                session->core_machine, &diagnostic) == TYPE_STATUS_OK &&
            diagnostic.first_delivered_exception.valid) {
            STD_SIZE_T index;

            STD_PRINTF("BOOT-PROBE=first-exception-pc=%05X\n", (unsigned int)
                diagnostic.first_delivered_exception.point.linear_pc);
            STD_PRINTF("BOOT-PROBE=first-exception-flags=%08X-eax=%08X-ebx=%08X-ecx=%08X-edx=%08X\n",
                (unsigned int)diagnostic.first_delivered_exception.eflags,
                (unsigned int)diagnostic.first_delivered_exception.eax,
                (unsigned int)diagnostic.first_delivered_exception.ebx,
                (unsigned int)diagnostic.first_delivered_exception.ecx,
                (unsigned int)diagnostic.first_delivered_exception.edx);
            for (index = 0u; index < diagnostic.recent_count; ++index) {
                const core_machine_cpu_execution_point *point = &diagnostic.recent[index];

                STD_PRINTF("BOOT-PROBE=pre-exception-%u=%05X-%02X,%02X,%02X,%02X\n",
                    (unsigned int)index, (unsigned int)point->linear_pc,
                    (unsigned int)point->bytes[0u], (unsigned int)point->bytes[1u],
                    (unsigned int)point->bytes[2u], (unsigned int)point->bytes[3u]);
            }
            goto done;
        }
        if (session->core_machine->fdc.data.command_index != 0u ||
            session->core_machine->fdc.data.phase != core_machine_fdc_PHASE_COMMAND) {
            last_fdc_command = session->core_machine->fdc.data.cmd[0u];
            STD_MEMCPY(last_fdc_bytes, session->core_machine->fdc.data.cmd,
                sizeof(last_fdc_bytes));
            last_fdc_result[0u] = session->core_machine->fdc.data.st0;
            last_fdc_result[1u] = session->core_machine->fdc.data.st1;
            last_fdc_result[2u] = session->core_machine->fdc.data.st2;
            last_fdc_phase = (type_unsigned_8)session->core_machine->fdc.data.phase;
            last_fdc_remaining = session->core_machine->fdc.data.transfer_remaining;
            last_dma_address = session->core_machine->shared_dma_primary.data.currAddr[2u];
            last_dma_count = session->core_machine->shared_dma_primary.data.currCount[2u];
            last_dma_mode = session->core_machine->shared_dma_primary.data.mode[2u];
        }
        if (result.reason == CORE_MACHINE_STOP_FAULT) {
            STD_PRINTF("BOOT-PROBE=guest-fault\n");
            goto done;
        }
        if (result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            core_machine_cpu_state cpu;
            core_machine_time_observation observation;
            C_INT advanced = 0;

            if (core_machine_capture_time_observation(session->core_machine,
                    &observation) != TYPE_STATUS_OK ||
                core_machine_get_cpu_state(session->core_machine, &cpu) !=
                    TYPE_STATUS_OK) {
                STD_PRINTF("BOOT-PROBE=run-failed\n");
                goto done;
            }
            waiting_for_interrupt = 1;
            waiting_with_deadline = observation.next_deadline_valid;
            waiting_interrupts_enabled = (cpu.eflags & 0x00000200u) != 0u;
            waiting_in_rom = cpu.cs_base + cpu.eip >= 0x000f0000u &&
                cpu.cs_base + cpu.eip < 0x00100000u;
            waiting_in_low_memory = cpu.cs_base + cpu.eip < 0x00040000u;
            waiting_linear_pc = cpu.cs_base + cpu.eip;
            if (vm_session_waiting_advance(session, &result, &advanced) !=
                TYPE_STATUS_OK) {
                STD_PRINTF("BOOT-PROBE=run-failed\n");
                goto done;
            }
            last_wait_advanced = advanced;
        }
        else waiting_for_interrupt = 0;
        now = GetTickCount64();
        /* REP string instructions may correctly retire thousands of units at
         * one linear PC during firmware memory tests.  A changing PC is useful
         * diagnostic context, but it is not the definition of guest progress. */
        if (result.executed != 0u || result.ticks != 0u) progress = now;
        if (now >= next_display_capture) {
            type_status display_status = core_machine_capture_display_snapshot(
                session->core_machine, &snapshot);

            next_display_capture = now + VM_BYOB_BOOT_DISPLAY_CADENCE_MILLISECONDS;
            if (display_status != TYPE_STATUS_OK && display_status != TYPE_STATUS_UNSUPPORTED) {
                STD_PRINTF("BOOT-PROBE=display-failed-status=%u\n",
                    (unsigned int)display_status);
                goto done;
            }
            if (now >= next_text_memory_scan) {
                const C_INT resume_visible = vm_byob_text_memory_has(
                    session->core_machine, "RESUME");
                const C_INT installer_visible = resume_f1_sent &&
                    vm_byob_text_memory_has(session->core_machine, "ENTER=Continue");

                next_text_memory_scan = now + 250u;
                if (press_resume_f1 && !resume_f1_sent &&
                    resume_f1_prompt_at == 0u && resume_visible) {
                    resume_f1_prompt_at = now;
                }
                post_resume_required |= resume_visible;
                if (installer_visible) {
                    STD_PRINTF("BOOT-PROBE=installer-ready\n");
                    exit_code = 0;
                    goto done;
                }
            }
            if (display_status == TYPE_STATUS_OK) {
                if (vm_byob_snapshot_has_prompt(&snapshot)) {
                    STD_PRINTF("BOOT-PROBE=dos-prompt\n");
                    exit_code = 0;
                    goto done;
                }
                if (vm_byob_snapshot_has(&snapshot, "Current date")) {
                    STD_PRINTF("BOOT-PROBE=date-input\n");
                    exit_code = 0;
                    goto done;
                }
                if (vm_byob_snapshot_has(&snapshot, "ENTER=Continue")) {
                    STD_PRINTF("BOOT-PROBE=installer-ready\n");
                    exit_code = 0;
                    goto done;
                }
                current = vm_byob_snapshot_checksum(&snapshot);
                post_memory_failure |= vm_byob_snapshot_has(&snapshot, "201");
                post_keyboard_failure |= vm_byob_snapshot_has(&snapshot, "301");
                post_floppy_failure |= vm_byob_snapshot_has(&snapshot, "601");
                post_resume_required |= vm_byob_snapshot_has(&snapshot, "RESUME");
                if (!have_checksum || current != checksum) {
                    checksum = current;
                    have_checksum = 1;
                    progress = now;
                }
            }
        }
        if (press_resume_f1 && !resume_f1_sent && resume_f1_prompt_at != 0u &&
            now - resume_f1_prompt_at >= 100u) {
            type_unsigned_8 scan_set = 0u;

            if (!vm_byob_send_f1(session, 1, &scan_set)) {
                STD_PRINTF("BOOT-PROBE=resume-f1-input-failed\n");
                goto done;
            }
            resume_f1_sent = 1;
            resume_f1_make_at = now;
            started = GetTickCount64();
            progress = started;
            STD_PRINTF("BOOT-PROBE=resume-f1-sent-scan-set=%u\n",
                (unsigned int)scan_set);
        }
        if (resume_f1_sent && resume_f1_make_at != 0u &&
            now - resume_f1_make_at >= 25u) {
            type_unsigned_8 scan_set = 0u;

            if (!vm_byob_send_f1(session, 0, &scan_set)) {
                STD_PRINTF("BOOT-PROBE=resume-f1-release-failed\n");
                goto done;
            }
            resume_f1_make_at = 0u;
        }
        if (!have_linear_pc || result.linear_pc != linear_pc) {
            linear_pc = result.linear_pc;
            have_linear_pc = 1;
        }
        if (GetTickCount64() - progress >= no_progress_limit) {
            if (waiting_for_interrupt) {
                if (!waiting_with_deadline) STD_PRINTF("BOOT-PROBE=waiting-no-deadline\n");
                else if (waiting_interrupts_enabled)
                    STD_PRINTF("BOOT-PROBE=waiting-deadline-if-enabled\n");
                else if (waiting_in_rom) STD_PRINTF("BOOT-PROBE=waiting-rom-if-disabled\n");
                else if (waiting_in_low_memory)
                    STD_PRINTF("BOOT-PROBE=waiting-low-memory-if-disabled\n");
                else STD_PRINTF("BOOT-PROBE=waiting-other-if-disabled\n");
                STD_PRINTF("BOOT-PROBE=waiting-linear-pc=%05X\n",
                    (unsigned int)waiting_linear_pc);
                STD_PRINTF("BOOT-PROBE=post-message-offset=%04X\n",
                    (unsigned int)session->core_machine->executor_cpu.data.bp);
                if (core_machine_memory_read(session->core_machine,
                        session->core_machine->executor_cpu.data.ss.base +
                        session->core_machine->executor_cpu.data.sp + 2u,
                        &post_caller_offset, sizeof(post_caller_offset)) == TYPE_STATUS_OK) {
                    STD_PRINTF("BOOT-PROBE=post-caller-offset=%04X\n",
                        (unsigned int)post_caller_offset);
                }
                if (core_machine_memory_read(session->core_machine, 0x046bu,
                        &post_interrupt_flag, sizeof(post_interrupt_flag)) ==
                    TYPE_STATUS_OK) {
                    STD_PRINTF(post_interrupt_flag == 0u ?
                        "BOOT-PROBE=post-interrupt-flag-cleared\n" :
                        "BOOT-PROBE=post-interrupt-flag-set\n");
                }
                if (core_machine_memory_read(session->core_machine, 0x0415u,
                        &post_manufacturing_error_flag,
                        sizeof(post_manufacturing_error_flag)) == TYPE_STATUS_OK) {
                    STD_PRINTF("BOOT-PROBE=post-manufacturing-error-flag=%02X\n",
                        (unsigned int)post_manufacturing_error_flag);
                }
                STD_PRINTF("BOOT-PROBE=post-bl=%02X-post-flags=%04X\n",
                    (unsigned int)(session->core_machine->executor_cpu.data.ebx & 0xffu),
                    (unsigned int)(session->core_machine->executor_cpu.data.eflags & 0xffffu));
                STD_PRINTF("BOOT-PROBE=post-dma-status=%02X-request=%02X-mask=%02X\n",
                    (unsigned int)session->core_machine->shared_dma_primary.data.status,
                    (unsigned int)session->core_machine->shared_dma_primary.data.request,
                    (unsigned int)session->core_machine->shared_dma_primary.data.mask);
                STD_PRINTF("BOOT-PROBE=post-dma-command=%02X-mode0=%02X-count0=%04X-address0=%04X\n",
                    (unsigned int)session->core_machine->shared_dma_primary.data.command,
                    (unsigned int)session->core_machine->shared_dma_primary.data.mode[0u],
                    (unsigned int)session->core_machine->shared_dma_primary.data.currCount[0u],
                    (unsigned int)session->core_machine->shared_dma_primary.data.currAddr[0u]);
                STD_PRINTF("BOOT-PROBE=post-elapsed-ticks=%llu\n",
                    (unsigned long long)session->core_machine->elapsed_ticks);
                STD_PRINTF("BOOT-PROBE=pit-waits-first=%llu-second=%llu-irq0=%llu\n",
                    (unsigned long long)trace.pit_wait_first_retires,
                    (unsigned long long)trace.pit_wait_second_retires,
                    (unsigned long long)trace.irq0_retires);
                STD_PRINTF("BOOT-PROBE=trace-cpu-retires=%llu\n",
                    (unsigned long long)trace.cpu_retires);
                STD_PRINTF("BOOT-PROBE=pit0-cw=%02X-reload=%u-remaining=%u-out=%u-active=%u\n",
                    (unsigned int)session->core_machine->shared_pit.data.cw[0u],
                    (unsigned int)session->core_machine->shared_pit.data.reload[0u],
                    (unsigned int)session->core_machine->shared_pit.data.remaining[0u],
                    (unsigned int)session->core_machine->shared_pit.data.flagOutput[0u],
                    (unsigned int)session->core_machine->shared_pit.data.flagActive[0u]);
                if (session->core_machine->xt_ppi_keyboard.mode_control == 0x9bu)
                    STD_PRINTF("BOOT-PROBE=xt-ppi-unconfigured\n");
                else STD_PRINTF("BOOT-PROBE=xt-ppi-configured\n");
                if (session->core_machine->xt_keyboard.clock_held)
                    STD_PRINTF("BOOT-PROBE=xt-keyboard-clock-held\n");
                if (session->core_machine->xt_keyboard.bat_active)
                    STD_PRINTF("BOOT-PROBE=xt-keyboard-bat-active\n");
                if (session->core_machine->xt_keyboard.bat_result_pending)
                    STD_PRINTF("BOOT-PROBE=xt-keyboard-bat-pending\n");
                if (session->core_machine->xt_ppi_keyboard.byte_ready)
                    STD_PRINTF("BOOT-PROBE=xt-keyboard-byte-ready\n");
                STD_PRINTF("BOOT-PROBE=pic-imr=%02X\n",
                    (unsigned int)session->core_machine->shared_pic_master.data.imr);
                if (session->core_machine->shared_pic_master.data.imr == 0xffu)
                    STD_PRINTF("BOOT-PROBE=pic-imr-ff\n");
                else STD_PRINTF("BOOT-PROBE=pic-imr-not-ff\n");
                if (session->core_machine->shared_pic_master.data.status == OCW1)
                    STD_PRINTF("BOOT-PROBE=pic-ocw1-ready\n");
                else STD_PRINTF("BOOT-PROBE=pic-not-ocw1-ready\n");
                if (post_memory_failure) STD_PRINTF("BOOT-PROBE=post-memory-failure\n");
                else if (post_keyboard_failure)
                    STD_PRINTF("BOOT-PROBE=post-keyboard-failure\n");
                else if (post_floppy_failure)
                    STD_PRINTF("BOOT-PROBE=post-floppy-failure\n");
                else STD_PRINTF("BOOT-PROBE=post-other-failure\n");
            } else {
                STD_PRINTF("BOOT-PROBE=running-no-display-progress\n");
            }
            if (executed_total < 16u) STD_PRINTF("BOOT-PROBE=retirement-under-16\n");
            else if (executed_total < 256u) STD_PRINTF("BOOT-PROBE=retirement-under-256\n");
            else STD_PRINTF("BOOT-PROBE=retirement-at-least-256\n");
            if (trace_enabled && trace.int15_calls != 0u) {
                STD_PRINTF("BOOT-PROBE=trace-int15-calls=%llu-last-ah=%02X\n",
                    (unsigned long long)trace.int15_calls,
                    (unsigned int)trace.last_int15_ah);
                STD_PRINTF("BOOT-PROBE=trace-int15-known-services=%02X\n",
                    (unsigned int)trace.int15_ah_seen);
            }
            if (trace_enabled && (trace.sgdt_calls != 0u || trace.sidt_calls != 0u)) {
                STD_PRINTF("BOOT-PROBE=trace-sgdt=%llu-sidt=%llu\n",
                    (unsigned long long)trace.sgdt_calls,
                    (unsigned long long)trace.sidt_calls);
            }
            if (trace_enabled && trace.pushf_calls != 0u) {
                STD_PRINTF("BOOT-PROBE=trace-pushf=%llu-last-high=%04X\n",
                    (unsigned long long)trace.pushf_calls,
                    (unsigned int)trace.last_pushf_high);
            }
            goto done;
        }
    }
    if (trace_enabled) {
    STD_PRINTF("BOOT-PROBE=run-count=%llu-last-reason=%u-wait-advanced=%u\n",
        (unsigned long long)run_count, (unsigned int)last_reason,
        (unsigned int)last_wait_advanced);
    STD_PRINTF("BOOT-PROBE=last-detail=%08X-cs=%08X-ip=%08X\n",
        (unsigned int)last_detail,
        (unsigned int)session->core_machine->executor_cpu.data.cs.base,
        (unsigned int)session->core_machine->executor_cpu.data.eip);
    STD_PRINTF("BOOT-PROBE=last-retired-pc=%05X\n",
        (unsigned int)trace.last_linear_pc);
    if (diagnostic.last_delivered_exception.valid) {
        STD_PRINTF("BOOT-PROBE=delivered-exceptions=%u-last-mask=%08X-last-code=%08X\n",
            (unsigned int)diagnostic.delivered_exception_count,
            (unsigned int)diagnostic.last_delivered_exception.exception_mask,
            (unsigned int)diagnostic.last_delivered_exception.exception_code);
        STD_PRINTF("BOOT-PROBE=first-exception-pc=%05X-last-exception-pc=%05X\n",
            (unsigned int)diagnostic.first_delivered_exception.point.linear_pc,
            (unsigned int)diagnostic.last_delivered_exception.point.linear_pc);
    }
    if (trace.reset_events != 0u) STD_PRINTF("BOOT-PROBE=trace-reset\n");
    if (trace.rom_memory_reads != 0u) STD_PRINTF("BOOT-PROBE=trace-rom-read\n");
    if (trace.xt_ppi_port_accesses != 0u) STD_PRINTF("BOOT-PROBE=trace-xt-ppi\n");
    if (trace.pic_port_accesses != 0u) STD_PRINTF("BOOT-PROBE=trace-pic\n");
    if (trace.pit_port_accesses != 0u) STD_PRINTF("BOOT-PROBE=trace-pit\n");
    if (trace.cga_port_accesses != 0u) STD_PRINTF("BOOT-PROBE=trace-cga-port\n");
    if (trace.cga_memory_writes != 0u) STD_PRINTF("BOOT-PROBE=trace-cga-memory\n");
    if (trace.fdc_port_accesses != 0u) STD_PRINTF("BOOT-PROBE=trace-fdc\n");
    if (trace.int15_calls != 0u) {
        STD_PRINTF("BOOT-PROBE=trace-int15-calls=%llu-last-ah=%02X\n",
            (unsigned long long)trace.int15_calls,
            (unsigned int)trace.last_int15_ah);
    }
    if (trace.fdc_port_accesses == 0u && trace.xt_ppi_port_accesses == 0u) {
        STD_PRINTF("BOOT-PROBE=wall-timeout-pre-fdc-no-xt-ppi\n");
    }
    else if (trace.fdc_port_accesses == 0u) {
        STD_PRINTF("BOOT-PROBE=wall-timeout-pre-fdc-after-xt-ppi\n");
    }
    else if (trace.xt_ppi_port_accesses == 0u) {
        STD_PRINTF("BOOT-PROBE=wall-timeout-fdc-without-xt-ppi\n");
    } else STD_PRINTF("BOOT-PROBE=wall-timeout-fdc-and-xt-ppi\n");
    }
done:
    if (session != STD_NULL && session->core_machine != STD_NULL) {
        type_unsigned_16 bda_equipment = 0u;
        type_unsigned_16 bda_keyboard_head = 0u;
        type_unsigned_16 bda_keyboard_tail = 0u;
        type_unsigned_8 bda_wait_state[16] = {0};
        type_unsigned_16 bda_memory_kib = 0u;
        type_unsigned_16 stack_words[8] = {0};
        type_unsigned_8 bda_post_status = 0u;
        type_unsigned_8 bda_diskette_status = 0u;
        type_unsigned_8 bda_motor_wait = 0u;
        type_unsigned_8 bda_motor_status = 0u;
        type_unsigned_16 int13_offset = 0u;
        type_unsigned_16 int13_segment = 0u;
        type_unsigned_16 int1e_offset = 0u;
        type_unsigned_16 int1e_segment = 0u;
        type_unsigned_16 irq6_offset = 0u;
        type_unsigned_16 irq6_segment = 0u;
        type_unsigned_16 int15_offset = 0u;
        type_unsigned_16 int15_segment = 0u;
        type_unsigned_8 int13_state[5] = {0u};
        type_unsigned_8 next_instruction_bytes[8] = {0u};

        STD_MEMSET(&diagnostic, 0, sizeof(diagnostic));
        core_machine_cpu_diagnostic_capture(session->core_machine, &diagnostic);
        if (diagnostic.first_fault.valid) {
            const core_machine_cpu_execution_point *point =
                &diagnostic.first_fault.point;

            STD_PRINTF("BOOT-PROBE=first-fault-mask=%08X-code=%08X-pc=%05X-bytes=%02X,%02X,%02X,%02X\n",
                (unsigned int)diagnostic.first_fault.exception_mask,
                (unsigned int)diagnostic.first_fault.exception_code,
                (unsigned int)point->linear_pc, (unsigned int)point->bytes[0u],
                (unsigned int)point->bytes[1u], (unsigned int)point->bytes[2u],
                (unsigned int)point->bytes[3u]);
        }
        if (diagnostic.last_delivered_exception.valid) {
            const core_machine_cpu_execution_point *point =
                &diagnostic.last_delivered_exception.point;

            STD_PRINTF("BOOT-PROBE=last-delivered-pc=%05X-bytes=%02X,%02X,%02X,%02X\n",
                (unsigned int)point->linear_pc, (unsigned int)point->bytes[0u],
                (unsigned int)point->bytes[1u], (unsigned int)point->bytes[2u],
                (unsigned int)point->bytes[3u]);
        }
        if (trace.model40_invalid_entry_observed) {
            const core_machine_cpu_execution_point *point =
                &trace.model40_invalid_entry_source;

            STD_PRINTF("BOOT-PROBE=model40-invalid-entry-source=%05X-bytes=%02X,%02X,%02X,%02X-ss=%04X-sp=%04X\n",
                (unsigned int)point->linear_pc, (unsigned int)point->bytes[0u],
                (unsigned int)point->bytes[1u], (unsigned int)point->bytes[2u],
                (unsigned int)point->bytes[3u], (unsigned int)trace.model40_invalid_entry_ss,
                (unsigned int)trace.model40_invalid_entry_sp);
        }
        if (trace.model40_int10_vector_write_observed) {
            type_unsigned_64 entry;
            const type_unsigned_64 count = trace.model40_int10_vector_write_count <
                VM_BYOB_MODEL40_INT10_VECTOR_HISTORY ?
                trace.model40_int10_vector_write_count :
                VM_BYOB_MODEL40_INT10_VECTOR_HISTORY;

            STD_PRINTF("BOOT-PROBE=model40-int10-vector-writer=%llu-%05X-cs=%04X-vector=%04X:%04X\n",
                (unsigned long long)trace.model40_int10_vector_write_count,
                (unsigned int)trace.model40_int10_vector_write_pc,
                (unsigned int)trace.model40_int10_vector_write_cs,
                (unsigned int)trace.model40_int10_vector_segment,
                (unsigned int)trace.model40_int10_vector_offset);
            for (entry = 0u; entry < count; ++entry) {
                const STD_SIZE_T index = (STD_SIZE_T)(
                    (trace.model40_int10_vector_write_count - count + entry) %
                    VM_BYOB_MODEL40_INT10_VECTOR_HISTORY);

                STD_PRINTF("BOOT-PROBE=model40-int10-vector-write-%05X-cs=%04X=%04X:%04X\n",
                    (unsigned int)trace.model40_int10_vector_history_pc[index],
                    (unsigned int)trace.model40_int10_vector_history_cs[index],
                    (unsigned int)trace.model40_int10_vector_history_segment[index],
                    (unsigned int)trace.model40_int10_vector_history_offset[index]);
            }
        }
        {
            type_unsigned_16 int42_offset = 0u;
            type_unsigned_16 int42_segment = 0u;
            type_unsigned_64 entry;

            (C_VOID)core_machine_memory_read(session->core_machine, 0x0108u,
                &int42_offset, sizeof(int42_offset));
            (C_VOID)core_machine_memory_read(session->core_machine, 0x010au,
                &int42_segment, sizeof(int42_segment));
            STD_PRINTF("BOOT-PROBE=model40-int42-vector=%04X:%04X\n",
                (unsigned int)int42_segment, (unsigned int)int42_offset);
            for (entry = 0u; entry < trace.model40_int42_vector_write_count &&
                    entry < VM_BYOB_MODEL40_INT10_VECTOR_HISTORY; ++entry) {
                STD_PRINTF("BOOT-PROBE=model40-int42-vector-write-%05X=%04X:%04X\n",
                    (unsigned int)trace.model40_int42_vector_write_pc[entry],
                    (unsigned int)trace.model40_int42_vector_history_segment[entry],
                    (unsigned int)trace.model40_int42_vector_history_offset[entry]);
            }
        }
        if (trace.model40_int10_entry_count != 0u) {
            STD_PRINTF("BOOT-PROBE=model40-int10-entry=%llu-predecessor=%05X-ss=%04X-sp=%04X\n",
                (unsigned long long)trace.model40_int10_entry_count,
                (unsigned int)trace.model40_int10_entry_predecessor,
                (unsigned int)trace.model40_int10_entry_ss,
                (unsigned int)trace.model40_int10_entry_sp);
        }
        if (trace.model40_int10_iret_frame_observed) {
            STD_PRINTF("BOOT-PROBE=model40-int10-iret-frame=%llu-pc:%05X-cs-default-32:%u-ss:%04X-base:%08X-limit:%08X-big:%u-esp:%08X-words:%04X,%04X,%04X,%04X\n",
                (unsigned long long)trace.model40_int10_iret_frame_count,
                (unsigned int)trace.model40_int10_iret_frame_pc,
                (unsigned int)trace.model40_int10_iret_cs_default_32,
                (unsigned int)trace.model40_int10_iret_ss,
                (unsigned int)trace.model40_int10_iret_ss_base,
                (unsigned int)trace.model40_int10_iret_ss_limit,
                (unsigned int)trace.model40_int10_iret_ss_big,
                (unsigned int)trace.model40_int10_iret_esp,
                (unsigned int)trace.model40_int10_iret_words[0u],
                (unsigned int)trace.model40_int10_iret_words[1u],
                (unsigned int)trace.model40_int10_iret_words[2u],
                (unsigned int)trace.model40_int10_iret_words[3u]);
        }
        if (trace.model40_bios_iret_frame_observed) {
            STD_PRINTF("BOOT-PROBE=model40-bios-iret-frame=ss:%04X-sp:%04X-words:%04X,%04X,%04X,%04X\n",
                (unsigned int)trace.model40_bios_iret_ss,
                (unsigned int)trace.model40_bios_iret_sp,
                (unsigned int)trace.model40_bios_iret_words[0u],
                (unsigned int)trace.model40_bios_iret_words[1u],
                (unsigned int)trace.model40_bios_iret_words[2u],
                (unsigned int)trace.model40_bios_iret_words[3u]);
        }
        STD_PRINTF("BOOT-PROBE=model40-video-rom-entries=%llu-first=%05X\n",
            (unsigned long long)trace.model40_video_rom_entries,
            (unsigned int)trace.model40_video_rom_first_pc);
        STD_PRINTF("BOOT-PROBE=model40-video-special=3C2:%llu/%02X-3C6:%llu/%02X-7C6:%llu/%02X-BC6:%llu/%02X-FC6:%llu/%02X\n",
            (unsigned long long)trace.model40_video_special_accesses[0u],
            (unsigned int)trace.model40_video_special_last_values[0u],
            (unsigned long long)trace.model40_video_special_accesses[1u],
            (unsigned int)trace.model40_video_special_last_values[1u],
            (unsigned long long)trace.model40_video_special_accesses[2u],
            (unsigned int)trace.model40_video_special_last_values[2u],
            (unsigned long long)trace.model40_video_special_accesses[3u],
            (unsigned int)trace.model40_video_special_last_values[3u],
            (unsigned long long)trace.model40_video_special_accesses[4u],
            (unsigned int)trace.model40_video_special_last_values[4u]);
        STD_PRINTF("BOOT-PROBE=model40-video-error-port-84=%llu/%02X-at-%05X\n",
            (unsigned long long)trace.model40_video_error_writes,
            (unsigned int)trace.model40_video_error_last_value,
            (unsigned int)trace.model40_video_error_last_pc);
        {
            type_unsigned_64 entry;
            const type_unsigned_64 count = trace.model40_video_special_history_count <
                VM_BYOB_MODEL40_VIDEO_SPECIAL_HISTORY ?
                trace.model40_video_special_history_count :
                VM_BYOB_MODEL40_VIDEO_SPECIAL_HISTORY;

            for (entry = 0u; entry < count; ++entry) {
                const STD_SIZE_T index = (STD_SIZE_T)(
                    (trace.model40_video_special_history_count - count + entry) %
                    VM_BYOB_MODEL40_VIDEO_SPECIAL_HISTORY);
                const vm_byob_fdc_port_event *record =
                    &trace.model40_video_special_history[index];

                STD_PRINTF("BOOT-PROBE=model40-video-special-port-%05X-%c-%04X-%02X\n",
                    (unsigned int)record->linear_pc, record->write ? 'w' : 'r',
                    (unsigned int)record->port, (unsigned int)record->value);
            }
        }
        if (trace.model40_video_port_accesses != 0u) {
            type_unsigned_64 entry;
            const type_unsigned_64 count = trace.model40_video_port_accesses <
                VM_BYOB_MODEL40_VIDEO_PORT_HISTORY ? trace.model40_video_port_accesses :
                VM_BYOB_MODEL40_VIDEO_PORT_HISTORY;

            for (entry = 0u; entry < count; ++entry) {
                const STD_SIZE_T index = (STD_SIZE_T)((trace.model40_video_port_accesses -
                    count + entry) % VM_BYOB_MODEL40_VIDEO_PORT_HISTORY);
                const vm_byob_fdc_port_event *record =
                    &trace.model40_video_port_history[index];

                STD_PRINTF("BOOT-PROBE=model40-video-port-%05X-%c-%04X-%02X\n",
                    (unsigned int)record->linear_pc, record->write ? 'w' : 'r',
                    (unsigned int)record->port, (unsigned int)record->value);
            }
        }
        if (diagnostic.first_delivered_exception.valid) {
            const core_machine_cpu_execution_point *point =
                &diagnostic.first_delivered_exception.point;

            STD_PRINTF("BOOT-PROBE=first-delivered-pc=%05X-bytes=%02X,%02X,%02X,%02X\n",
                (unsigned int)point->linear_pc, (unsigned int)point->bytes[0u],
                (unsigned int)point->bytes[1u], (unsigned int)point->bytes[2u],
                (unsigned int)point->bytes[3u]);
        }
        if (trace.int6_vector_write_mask != 0u) {
            STD_PRINTF("BOOT-PROBE=int6-vector-write-mask=%02X-bytes=%02X,%02X,%02X,%02X\n",
                (unsigned int)trace.int6_vector_write_mask,
                (unsigned int)trace.int6_vector_write_bytes[0u],
                (unsigned int)trace.int6_vector_write_bytes[1u],
                (unsigned int)trace.int6_vector_write_bytes[2u],
                (unsigned int)trace.int6_vector_write_bytes[3u]);
        }
        if (trace.int6_pre_fault_snapshot_valid) {
            STD_PRINTF("BOOT-PROBE=int6-vector-before-66-prefix=%04X:%04X\n",
                (unsigned int)trace.int6_pre_fault_segment,
                (unsigned int)trace.int6_pre_fault_offset);
        }
        if (trace.near_ud_count != 0u) {
            type_unsigned_64 count = trace.near_ud_count;
            type_unsigned_64 index;

            if (count > VM_BYOB_NEAR_UD_HISTORY) count = VM_BYOB_NEAR_UD_HISTORY;
            for (index = 0u; index < count; ++index) {
                const type_unsigned_8 history_index = (type_unsigned_8)(
                    (trace.near_ud_count - count + index) % VM_BYOB_NEAR_UD_HISTORY);
                const core_machine_cpu_execution_point *point =
                    &trace.near_ud_history[history_index];

                STD_PRINTF("BOOT-PROBE=pre-ud-%u-pc=%05X-bytes=%02X,%02X,%02X,%02X-flags=%08X-eax=%08X-ebx=%08X\n",
                    (unsigned int)index, (unsigned int)point->linear_pc,
                    (unsigned int)point->bytes[0u], (unsigned int)point->bytes[1u],
                    (unsigned int)point->bytes[2u], (unsigned int)point->bytes[3u],
                    (unsigned int)trace.near_ud_eflags[history_index],
                    (unsigned int)trace.near_ud_eax[history_index],
                    (unsigned int)trace.near_ud_ebx[history_index]);
            }
        }
        if (trace.real_286_high_flags_observed) {
            STD_PRINTF("BOOT-PROBE=real-286-high-flags-pc=%05X-flags=%08X\n",
                (unsigned int)trace.real_286_high_flags_point.linear_pc,
                (unsigned int)trace.real_286_high_flags_value);
        }
        if (trace.int15_calls != 0u) {
            STD_PRINTF("BOOT-PROBE=int15-calls=%llu-services=%02X-last-ah=%02X\n",
                (unsigned long long)trace.int15_calls,
                (unsigned int)trace.int15_ah_seen,
                (unsigned int)trace.last_int15_ah);
        }
        STD_PRINTF("BOOT-PROBE=final-executed=%llu-trace-retires=%llu-runs=%llu-last-reason=%u\n",
            (unsigned long long)executed_total,
            (unsigned long long)trace.cpu_retires,
            (unsigned long long)run_count, (unsigned int)last_reason);
        STD_PRINTF("BOOT-PROBE=final-pc=%05X-elapsed=%llu\n",
            (unsigned int)linear_pc,
            (unsigned long long)session->core_machine->elapsed_ticks);
        if (core_machine_memory_read(session->core_machine, linear_pc,
                next_instruction_bytes, sizeof(next_instruction_bytes)) == TYPE_STATUS_OK) {
            STD_PRINTF("BOOT-PROBE=next-bytes=%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X\n",
                (unsigned int)next_instruction_bytes[0u],
                (unsigned int)next_instruction_bytes[1u],
                (unsigned int)next_instruction_bytes[2u],
                (unsigned int)next_instruction_bytes[3u],
                (unsigned int)next_instruction_bytes[4u],
                (unsigned int)next_instruction_bytes[5u],
                (unsigned int)next_instruction_bytes[6u],
                (unsigned int)next_instruction_bytes[7u]);
        }
        STD_PRINTF("BOOT-PROBE=final-cx=%04X-dx=%04X-di=%04X-ds=%04X-es=%04X-ax=%04X-bp=%04X\n",
            (unsigned int)(session->core_machine->executor_cpu.data.ecx & 0xffffu),
            (unsigned int)(session->core_machine->executor_cpu.data.edx & 0xffffu),
            (unsigned int)(session->core_machine->executor_cpu.data.edi & 0xffffu),
            (unsigned int)session->core_machine->executor_cpu.data.ds.selector,
            (unsigned int)session->core_machine->executor_cpu.data.es.selector,
            (unsigned int)(session->core_machine->executor_cpu.data.eax & 0xffffu),
            (unsigned int)(session->core_machine->executor_cpu.data.ebp & 0xffffu));
        /* Temporary Model 40 firmware diagnosis: the BIOS's F90CCh delay
           latches PIT0 twice.  Report the owner-local state so the probe can
           distinguish a changing counter from a stuck latch without changing
           guest-visible behavior. */
        STD_PRINTF("BOOT-PROBE=pit0-count=%04X-latch=%04X-remaining=%u-latched=%u-active=%u-read=%u\n",
            (unsigned int)session->core_machine->shared_pit.data.count[0u],
            (unsigned int)session->core_machine->shared_pit.data.latch[0u],
            (unsigned int)session->core_machine->shared_pit.data.remaining[0u],
            (unsigned int)session->core_machine->shared_pit.data.flagLatch[0u],
            (unsigned int)session->core_machine->shared_pit.data.flagActive[0u],
            (unsigned int)session->core_machine->shared_pit.data.flagRead[0u]);
        STD_PRINTF("BOOT-PROBE=fdc-phase=%u-dor=%02X-msr=%02X-st=%02X/%02X/%02X-reset=%u/%u-seek=%u-cylinder=%u\n",
            (unsigned int)session->core_machine->fdc.data.phase,
            (unsigned int)session->core_machine->fdc.data.dor,
            (unsigned int)session->core_machine->fdc.data.msr,
            (unsigned int)session->core_machine->fdc.data.st0,
            (unsigned int)session->core_machine->fdc.data.st1,
            (unsigned int)session->core_machine->fdc.data.st2,
            (unsigned int)session->core_machine->fdc.data.reset_pending,
            (unsigned int)session->core_machine->fdc.data.reset_sense_mask,
            (unsigned int)session->core_machine->fdc.data.seek_result_count,
            (unsigned int)session->core_machine->fdc.data.cylinder);
        STD_PRINTF("BOOT-PROBE=fdc-result=%02X-length=%u-index=%u-st3=%02X\n",
            (unsigned int)session->core_machine->fdc.data.ret[0u],
            (unsigned int)session->core_machine->fdc.data.result_length,
            (unsigned int)session->core_machine->fdc.data.result_index,
            (unsigned int)session->core_machine->fdc.data.st3);
        STD_PRINTF("BOOT-PROBE=last-fdc-command=%02X-phase=%u-st=%02X/%02X/%02X\n",
            (unsigned int)last_fdc_command, (unsigned int)last_fdc_phase,
            (unsigned int)last_fdc_result[0u], (unsigned int)last_fdc_result[1u],
            (unsigned int)last_fdc_result[2u]);
        STD_PRINTF("BOOT-PROBE=last-fdc-bytes=%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X\n",
            (unsigned int)last_fdc_bytes[0u], (unsigned int)last_fdc_bytes[1u],
            (unsigned int)last_fdc_bytes[2u], (unsigned int)last_fdc_bytes[3u],
            (unsigned int)last_fdc_bytes[4u], (unsigned int)last_fdc_bytes[5u],
            (unsigned int)last_fdc_bytes[6u], (unsigned int)last_fdc_bytes[7u],
            (unsigned int)last_fdc_bytes[8u]);
        STD_PRINTF("BOOT-PROBE=last-fdc-remaining=%u-dma2-address=%04X-count=%04X-mode=%02X\n",
            (unsigned int)last_fdc_remaining, (unsigned int)last_dma_address,
            (unsigned int)last_dma_count, (unsigned int)last_dma_mode);
        STD_PRINTF("BOOT-PROBE=fdc-terminals=%llu-failed=%llu-last=%02X/%u/%02X,%02X,%02X\n",
            (unsigned long long)trace.fdc_terminal_count,
            (unsigned long long)trace.fdc_failed_terminal_count,
            (unsigned int)trace.last_fdc_terminal.command,
            (unsigned int)trace.last_fdc_terminal.drive,
            (unsigned int)trace.last_fdc_terminal.result[0u],
            (unsigned int)trace.last_fdc_terminal.result[1u],
            (unsigned int)trace.last_fdc_terminal.result[2u]);
        STD_PRINTF("BOOT-PROBE=model40-resume=%llu-resume-predecessor=%05X-resume-ax=%04X-post-setup=%llu-post-predecessor=%05X-video-clear=%llu-delay=%llu-delay-predecessor=%05X\n",
            (unsigned long long)trace.model40_resume_entries,
            (unsigned int)trace.model40_resume_predecessor,
            (unsigned int)trace.model40_resume_ax,
            (unsigned long long)trace.model40_post_setup_entries,
            (unsigned int)trace.model40_post_setup_predecessor,
            (unsigned long long)trace.model40_video_clear_entries,
            (unsigned long long)trace.model40_video_delay_entries,
            (unsigned int)trace.model40_video_delay_predecessor);
        STD_PRINTF("BOOT-PROBE=model40-d4-control=%02X-ram-setup=%04X\n",
            (unsigned int)session->core_machine->d4_memory.control,
            (unsigned int)session->core_machine->d4_memory.ram_setup);
        STD_PRINTF("BOOT-PROBE=model40-reset-vector-target=%llu-predecessor=%05X\n",
            (unsigned long long)trace.model40_reset_vector_target_entries,
            (unsigned int)trace.model40_reset_vector_target_predecessor);
        STD_PRINTF("BOOT-PROBE=model40-post-latch-writes=%llu-last=%05X-%02X\n",
            (unsigned long long)trace.model40_post_latch_writes,
            (unsigned int)trace.model40_post_latch_last_pc,
            (unsigned int)trace.model40_post_latch_last_value);
        STD_PRINTF("BOOT-PROBE=model40-post-latch-paths=FD41D:%llu/%05X-FD434:%llu/%05X-FD44C:%llu/%05X-FD461:%llu/%05X-FD49E:%llu/%05X\n",
            (unsigned long long)trace.model40_post_latch_path_entries[0u],
            (unsigned int)trace.model40_post_latch_path_predecessors[0u],
            (unsigned long long)trace.model40_post_latch_path_entries[1u],
            (unsigned int)trace.model40_post_latch_path_predecessors[1u],
            (unsigned long long)trace.model40_post_latch_path_entries[2u],
            (unsigned int)trace.model40_post_latch_path_predecessors[2u],
            (unsigned long long)trace.model40_post_latch_path_entries[3u],
            (unsigned int)trace.model40_post_latch_path_predecessors[3u],
            (unsigned long long)trace.model40_post_latch_path_entries[4u],
            (unsigned int)trace.model40_post_latch_path_predecessors[4u]);
        STD_PRINTF("BOOT-PROBE=model40-resume-prompt=%llu/%05X-wait=%llu/%05X\n",
            (unsigned long long)trace.model40_resume_prompt_entries,
            (unsigned int)trace.model40_resume_prompt_predecessor,
            (unsigned long long)trace.model40_resume_wait_entries,
            (unsigned int)trace.model40_resume_wait_predecessor);
        STD_PRINTF("BOOT-PROBE=model40-post-status-helper=%llu/%05X-value=%02X\n",
            (unsigned long long)trace.model40_post_status_helper_entries,
            (unsigned int)trace.model40_post_status_helper_predecessor,
            (unsigned int)trace.model40_post_status_value);
        STD_PRINTF("BOOT-PROBE=model40-memory-address-error=%llu/%05X-ds=%04X/%08X-status=%04X\n",
            (unsigned long long)trace.model40_memory_address_error_entries,
            (unsigned int)trace.model40_memory_address_error_predecessor,
            (unsigned int)trace.model40_memory_address_error_ds,
            (unsigned int)trace.model40_memory_address_error_ds_base,
            (unsigned int)trace.model40_memory_address_error_status);
        STD_PRINTF("BOOT-PROBE=model40-memory-address-test=%llu-flags=%08X-status=%04X\n",
            (unsigned long long)trace.model40_memory_address_test_entries,
            (unsigned int)trace.model40_memory_address_test_eflags,
            (unsigned int)trace.model40_memory_address_test_status);
        STD_PRINTF("BOOT-PROBE=model40-memory-pattern=%llu-es=%04X/%08X-after=%08X-eax=%08X-edi=%08X-value=%04X\n",
            (unsigned long long)trace.model40_memory_pattern_entries,
            (unsigned int)trace.model40_memory_pattern_es_selector,
            (unsigned int)trace.model40_memory_pattern_es_base,
            (unsigned int)trace.model40_memory_pattern_after_es_base,
            (unsigned int)trace.model40_memory_pattern_after_eax,
            (unsigned int)trace.model40_memory_pattern_after_edi,
            (unsigned int)trace.model40_memory_pattern_after_value);
        STD_PRINTF("BOOT-PROBE=model40-ram-post=entry:%llu-return:%llu-failure:%llu\n",
            (unsigned long long)trace.model40_ram_post_entries,
            (unsigned long long)trace.model40_ram_post_returns,
            (unsigned long long)trace.model40_ram_post_failures);
        if (trace.model40_memory_pattern_entries != 0u ||
            trace.model40_memory_compare_failures != 0u) {
            STD_SIZE_T index;

            STD_PRINTF("BOOT-PROBE=model40-memory-bases=");
            for (index = 0u; index < VM_BYOB_MODEL40_MEMORY_BASE_HISTORY; ++index)
                STD_PRINTF("%08X/%08X%s",
                    (unsigned int)trace.model40_memory_pattern_bases[index],
                    (unsigned int)trace.model40_memory_compare_bases[index],
                    index + 1u == VM_BYOB_MODEL40_MEMORY_BASE_HISTORY ? "\n" : ",");
        }
        STD_PRINTF("BOOT-PROBE=model40-memory-1e-writes=%llu-pc=%05X-value=%02X\n",
            (unsigned long long)trace.model40_memory_1e_writes,
            (unsigned int)trace.model40_memory_1e_last_pc,
            (unsigned int)trace.model40_memory_1e_last_value);
    STD_PRINTF("BOOT-PROBE=model40-memory-high-write-cycles=%llu-first=%05X/%02X-last=%05X/%05X/%02X\n",
            (unsigned long long)trace.model40_memory_high_writes,
            (unsigned int)trace.model40_memory_high_first_address,
            (unsigned int)trace.model40_memory_high_first_value,
            (unsigned int)trace.model40_memory_high_last_address,
            (unsigned int)trace.model40_memory_high_last_pc,
            (unsigned int)trace.model40_memory_high_last_value);
        STD_PRINTF("BOOT-PROBE=model40-memory-b-window-writes=%llu-first=%05X-last=%05X-at-mismatch=%llu/%05X\n",
            (unsigned long long)trace.model40_memory_b_window_writes,
            (unsigned int)trace.model40_memory_b_window_first_pc,
            (unsigned int)trace.model40_memory_b_window_last_pc,
            (unsigned long long)trace.model40_memory_b_window_writes_at_mismatch,
            (unsigned int)trace.model40_memory_b_window_last_pc_at_mismatch);
        STD_PRINTF("BOOT-PROBE=model40-memory-b-first-word-writes=%llu-last=%05X/%04X-retirements=%llu-video-disabled=%u-gdc6=%02X-seq0=%02X\n",
            (unsigned long long)trace.model40_memory_b_first_word_writes,
            (unsigned int)trace.model40_memory_b_first_word_last_pc,
            (unsigned int)trace.model40_memory_b_first_word_last_value,
            (unsigned long long)trace.model40_memory_b_first_word_retirements,
            (unsigned int)trace.model40_memory_b_first_word_video_memory_disabled,
            (unsigned int)trace.model40_memory_b_first_word_graphics_6,
            (unsigned int)trace.model40_memory_b_first_word_sequencer_0);
        STD_PRINTF("BOOT-PROBE=model40-memory-fb-page-writes=%llu-first=%05X-last=%05X\n",
            (unsigned long long)trace.model40_memory_fb_page_writes,
            (unsigned int)trace.model40_memory_fb_page_first_pc,
            (unsigned int)trace.model40_memory_fb_page_last_pc);
        STD_PRINTF("BOOT-PROBE=model40-memory-fb-first-word-writes=%llu-last=%05X/%04X-retirements=%llu\n",
            (unsigned long long)trace.model40_memory_fb_first_word_writes,
            (unsigned int)trace.model40_memory_fb_first_word_last_pc,
            (unsigned int)trace.model40_memory_fb_first_word_last_value,
            (unsigned long long)trace.model40_memory_fb_first_word_retirements);
        {
            const STD_SIZE_T count = trace.model40_es_change_count <
                VM_BYOB_MODEL40_ES_HISTORY ?
                (STD_SIZE_T)trace.model40_es_change_count : VM_BYOB_MODEL40_ES_HISTORY;
            const STD_SIZE_T first = trace.model40_es_change_count > count ?
                (STD_SIZE_T)(trace.model40_es_change_count % VM_BYOB_MODEL40_ES_HISTORY) : 0u;
            STD_SIZE_T index;

            for (index = 0u; index < count; ++index) {
                const STD_SIZE_T slot = (first + index) % VM_BYOB_MODEL40_ES_HISTORY;

                STD_PRINTF("BOOT-PROBE=model40-es-change-%u=%05X-%04X/%08X-cr0=%08X\n",
                    (unsigned int)index,
                    (unsigned int)trace.model40_es_change_pcs[slot],
                    (unsigned int)trace.model40_es_change_selectors[slot],
                    (unsigned int)trace.model40_es_change_bases[slot],
                    (unsigned int)trace.model40_es_change_cr0[slot]);
            }
        }
        STD_PRINTF("BOOT-PROBE=model40-memory-pattern-writes=%llu-first=%05X-last=%05X\n",
            (unsigned long long)trace.model40_memory_pattern_write_count,
            (unsigned int)trace.model40_memory_pattern_write_first_address,
            (unsigned int)trace.model40_memory_pattern_write_last_address);
        STD_PRINTF("BOOT-PROBE=model40-memory-compare-failures=%llu-branches=%llu-eax=%08X-edi=%08X-es=%04X/%08X-cr0=%08X-gdtr=%08X/%08X-desc=%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X-value=%04X\n",
            (unsigned long long)trace.model40_memory_compare_failures,
            (unsigned long long)trace.model40_memory_compare_branch_taken,
            (unsigned int)trace.model40_memory_compare_eax,
            (unsigned int)trace.model40_memory_compare_edi,
            (unsigned int)trace.model40_memory_compare_es_selector,
            (unsigned int)trace.model40_memory_compare_es_base,
            (unsigned int)trace.model40_memory_compare_cr0,
            (unsigned int)trace.model40_memory_compare_gdtr_base,
            (unsigned int)trace.model40_memory_compare_gdtr_limit,
            (unsigned int)trace.model40_memory_compare_descriptor[0u],
            (unsigned int)trace.model40_memory_compare_descriptor[1u],
            (unsigned int)trace.model40_memory_compare_descriptor[2u],
            (unsigned int)trace.model40_memory_compare_descriptor[3u],
            (unsigned int)trace.model40_memory_compare_descriptor[4u],
            (unsigned int)trace.model40_memory_compare_descriptor[5u],
            (unsigned int)trace.model40_memory_compare_descriptor[6u],
            (unsigned int)trace.model40_memory_compare_descriptor[7u],
            (unsigned int)trace.model40_memory_compare_value);
        STD_PRINTF("BOOT-PROBE=model40-memory-address-failures=%llu-ds=%04X-si=%04X-eax=%08X-ebx=%08X-ebp=%08X-flags=%08X\n",
            (unsigned long long)trace.model40_memory_address_failures,
            (unsigned int)trace.model40_memory_address_failure_ds,
            (unsigned int)trace.model40_memory_address_failure_si,
            (unsigned int)trace.model40_memory_address_failure_eax,
            (unsigned int)trace.model40_memory_address_failure_ebx,
            (unsigned int)trace.model40_memory_address_failure_ebp,
            (unsigned int)trace.model40_memory_address_failure_eflags);
        STD_PRINTF("BOOT-PROBE=model40-memory-scas=%llu-ax=%04X-di=%04X-es=%04X/%08X-flags=%08X\n",
            (unsigned long long)trace.model40_memory_scas_entries,
            (unsigned int)trace.model40_memory_scas_ax,
            (unsigned int)trace.model40_memory_scas_di,
            (unsigned int)trace.model40_memory_scas_es,
            (unsigned int)trace.model40_memory_scas_es_base,
            (unsigned int)trace.model40_memory_scas_eflags);
        STD_PRINTF("BOOT-PROBE=model40-memory-mismatch=%llu-ax=%04X-di=%04X-es=%04X/%08X-value=%04X\n",
            (unsigned long long)trace.model40_memory_mismatch_entries,
            (unsigned int)trace.model40_memory_mismatch_ax,
            (unsigned int)trace.model40_memory_mismatch_di,
            (unsigned int)trace.model40_memory_mismatch_es,
            (unsigned int)trace.model40_memory_mismatch_es_base,
            (unsigned int)trace.model40_memory_mismatch_value);
        STD_PRINTF("BOOT-PROBE=model40-memory-test-return=%llu-ax=%04X\n",
            (unsigned long long)trace.model40_memory_test_return_entries,
            (unsigned int)trace.model40_memory_test_return_ax);
        STD_PRINTF("BOOT-PROBE=model40-memory-test-entry=%llu-dx=%04X-ax=%04X\n",
            (unsigned long long)trace.model40_memory_test_entries,
            (unsigned int)trace.model40_memory_test_dx,
            (unsigned int)trace.model40_memory_test_ax_entry);
        STD_PRINTF("BOOT-PROBE=model40-memory-status-test=ax=%04X-mismatches=%llu-expected=%04X-actual=%04X-offset=%04X-es=%04X/%08X-cr0=%08X-gdtr=%08X/%04X-desc=%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X-video-disabled=%u-gdc6=%02X-seq0=%02X\n",
            (unsigned int)trace.model40_memory_status_test_ax_entry,
            (unsigned long long)trace.model40_memory_status_test_mismatches,
            (unsigned int)trace.model40_memory_status_test_expected,
            (unsigned int)trace.model40_memory_status_test_actual,
            (unsigned int)trace.model40_memory_status_test_offset,
            (unsigned int)trace.model40_memory_status_test_es,
            (unsigned int)trace.model40_memory_status_test_es_base,
            (unsigned int)trace.model40_memory_status_test_cr0,
            (unsigned int)trace.model40_memory_status_test_gdtr_base,
            (unsigned int)trace.model40_memory_status_test_gdtr_limit,
            (unsigned int)trace.model40_memory_status_test_descriptor[0u],
            (unsigned int)trace.model40_memory_status_test_descriptor[1u],
            (unsigned int)trace.model40_memory_status_test_descriptor[2u],
            (unsigned int)trace.model40_memory_status_test_descriptor[3u],
            (unsigned int)trace.model40_memory_status_test_descriptor[4u],
            (unsigned int)trace.model40_memory_status_test_descriptor[5u],
            (unsigned int)trace.model40_memory_status_test_descriptor[6u],
            (unsigned int)trace.model40_memory_status_test_descriptor[7u],
            (unsigned int)trace.model40_memory_status_test_video_memory_disabled,
            (unsigned int)trace.model40_memory_status_test_graphics_6,
            (unsigned int)trace.model40_memory_status_test_sequencer_0);
        STD_PRINTF("BOOT-PROBE=model40-memory-status-high-b=%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X-writes=%llu/%05X/%05X\n",
            (unsigned int)trace.model40_memory_status_test_high_b_page[0u],
            (unsigned int)trace.model40_memory_status_test_high_b_page[1u],
            (unsigned int)trace.model40_memory_status_test_high_b_page[2u],
            (unsigned int)trace.model40_memory_status_test_high_b_page[3u],
            (unsigned int)trace.model40_memory_status_test_high_b_page[4u],
            (unsigned int)trace.model40_memory_status_test_high_b_page[5u],
            (unsigned int)trace.model40_memory_status_test_high_b_page[6u],
            (unsigned int)trace.model40_memory_status_test_high_b_page[7u],
            (unsigned int)trace.model40_memory_status_test_high_b_page[8u],
            (unsigned int)trace.model40_memory_status_test_high_b_page[9u],
            (unsigned int)trace.model40_memory_status_test_high_b_page[10u],
            (unsigned int)trace.model40_memory_status_test_high_b_page[11u],
            (unsigned int)trace.model40_memory_status_test_high_b_page[12u],
            (unsigned int)trace.model40_memory_status_test_high_b_page[13u],
            (unsigned int)trace.model40_memory_status_test_high_b_page[14u],
            (unsigned int)trace.model40_memory_status_test_high_b_page[15u],
            (unsigned long long)trace.model40_memory_high_b_page_writes,
            (unsigned int)trace.model40_memory_high_b_page_first_pc,
            (unsigned int)trace.model40_memory_high_b_page_last_pc);
        STD_PRINTF("BOOT-PROBE=model40-memory-pattern-producer=entries:%llu-high-b-writes:%llu\n",
            (unsigned long long)trace.model40_memory_pattern_producer_entries,
            (unsigned long long)trace.model40_memory_pattern_producer_high_b_writes);
        {
            const STD_SIZE_T count = trace.model40_memory_high_b_page_writes <
                VM_BYOB_MODEL40_HIGH_B_WRITE_HISTORY ?
                (STD_SIZE_T)trace.model40_memory_high_b_page_writes :
                VM_BYOB_MODEL40_HIGH_B_WRITE_HISTORY;
            STD_SIZE_T index;

            for (index = 0u; index < count; ++index) {
                STD_PRINTF("BOOT-PROBE=model40-high-b-write-%u=%05X-producer=%u-page=%02X,%02X,%02X,%02X\n",
                    (unsigned int)index,
                    (unsigned int)trace.model40_memory_high_b_write_pcs[index],
                    (unsigned int)trace.model40_memory_high_b_write_producer[index],
                    (unsigned int)trace.model40_memory_high_b_write_pages[index][0u],
                    (unsigned int)trace.model40_memory_high_b_write_pages[index][1u],
                    (unsigned int)trace.model40_memory_high_b_write_pages[index][2u],
                    (unsigned int)trace.model40_memory_high_b_write_pages[index][3u]);
            }
        }
        STD_PRINTF("BOOT-PROBE=model40-post-status-58-writes=%llu-last=%05X-value=%04X\n",
            (unsigned long long)trace.model40_post_status_58_writes,
            (unsigned int)trace.model40_post_status_58_last_pc,
            (unsigned int)trace.model40_post_status_58_last_value);
        STD_PRINTF("BOOT-PROBE=model40-post-status-58-observer=%llu-last=%05X-value=%02X\n",
            (unsigned long long)trace.model40_post_status_58_observer_writes,
            (unsigned int)trace.model40_post_status_58_observer_last_pc,
            (unsigned int)trace.model40_post_status_58_observer_last_value);
        STD_PRINTF("BOOT-PROBE=model40-post-private-status=%llu-last=%05X-value=%04X\n",
            (unsigned long long)trace.model40_post_private_status_writes,
            (unsigned int)trace.model40_post_private_status_last_pc,
            (unsigned int)trace.model40_post_private_status_last_value);
        {
            const STD_SIZE_T count = trace.model40_post_private_status_writes <
                VM_BYOB_MODEL40_POST_PRIVATE_STATUS_HISTORY ?
                (STD_SIZE_T)trace.model40_post_private_status_writes :
                VM_BYOB_MODEL40_POST_PRIVATE_STATUS_HISTORY;
            STD_SIZE_T index;

            for (index = 0u; index < count; ++index) {
                STD_PRINTF("BOOT-PROBE=model40-post-private-status-write-%u=%05X/%04X\n",
                    (unsigned int)index,
                    (unsigned int)trace.model40_post_private_status_pcs[index],
                    (unsigned int)trace.model40_post_private_status_values[index]);
            }
        }
        {
            const STD_SIZE_T count = trace.model40_post_status_58_observer_writes <
                VM_BYOB_MODEL40_POST_STATUS_58_HISTORY ?
                (STD_SIZE_T)trace.model40_post_status_58_observer_writes :
                VM_BYOB_MODEL40_POST_STATUS_58_HISTORY;
            STD_SIZE_T index;

            for (index = 0u; index < count; ++index) {
                STD_PRINTF("BOOT-PROBE=model40-post-status-58-write-%u=%05X/%02X\n",
                    (unsigned int)index,
                    (unsigned int)trace.model40_post_status_58_observer_pcs[index],
                    (unsigned int)trace.model40_post_status_58_observer_values[index]);
            }
        }
        STD_PRINTF("BOOT-PROBE=model40-port61-reads=%llu-last=%05X/%02X\n",
            (unsigned long long)trace.model40_port61_reads,
            (unsigned int)trace.model40_port61_last_pc,
            (unsigned int)trace.model40_port61_last_value);
        STD_PRINTF("BOOT-PROBE=model40-memory-test-error-branches=compare:%llu-parity:%llu-port61:%llu/%02X\n",
            (unsigned long long)trace.model40_memory_compare_error_branches,
            (unsigned long long)trace.model40_memory_parity_error_branches,
            (unsigned long long)trace.model40_memory_parity_test_reads,
            (unsigned int)trace.model40_memory_parity_test_last_value);
        STD_PRINTF("BOOT-PROBE=model40-memory-error-exit=%llu-predecessor=%05X-es=%04X-di=%04X-ax=%04X\n",
            (unsigned long long)trace.model40_memory_error_exit_entries,
            (unsigned int)trace.model40_memory_error_exit_predecessor,
            (unsigned int)trace.model40_memory_error_es,
            (unsigned int)trace.model40_memory_error_di,
            (unsigned int)trace.model40_memory_error_ax);
        {
            STD_SIZE_T helper_index;
            const STD_SIZE_T helper_count = trace.model40_post_status_helper_entries <
                VM_BYOB_MODEL40_POST_STATUS_HELPER_HISTORY ?
                (STD_SIZE_T)trace.model40_post_status_helper_entries :
                VM_BYOB_MODEL40_POST_STATUS_HELPER_HISTORY;

            for (helper_index = 0u; helper_index < helper_count; ++helper_index) {
                STD_PRINTF("BOOT-PROBE=model40-post-status-helper-caller-%u=%05X\n",
                    (unsigned int)helper_index,
                    (unsigned int)trace.model40_post_status_helper_predecessors[helper_index]);
            }
        }
        {
            static const type_unsigned_32 writer_pcs[VM_BYOB_MODEL40_POST_STATUS_WRITERS] = {
                0x000f8324u, 0x000fbbd1u, 0x000fc801u, 0x000fd30cu,
                0x000fd323u, 0x000fd41du, 0x000fd434u, 0x000fd44cu,
                0x000fd461u, 0x000fd49eu, 0x000ff4e2u, 0x000ff54du
            };
            STD_SIZE_T writer_index;

            for (writer_index = 0u;
                    writer_index < VM_BYOB_MODEL40_POST_STATUS_WRITERS;
                    ++writer_index) {
                if (trace.model40_post_status_writer_entries[writer_index] == 0u) continue;
                STD_PRINTF("BOOT-PROBE=model40-post-status-writer=%05X:%llu/%05X\n",
                    (unsigned int)writer_pcs[writer_index],
                    (unsigned long long)trace.model40_post_status_writer_entries[writer_index],
                    (unsigned int)trace.model40_post_status_writer_predecessors[writer_index]);
            }
        }
        if (trace.model40_shutdown_diagnostic_valid) {
            STD_SIZE_T shutdown_index;

            STD_PRINTF("BOOT-PROBE=model40-shutdown-exceptions=%llu-first=%u/%u-last=%u/%u\n",
                (unsigned long long)trace.model40_shutdown_diagnostic.delivered_exception_count,
                (unsigned int)trace.model40_shutdown_diagnostic.first_delivered_exception.exception_mask,
                (unsigned int)trace.model40_shutdown_diagnostic.first_delivered_exception.exception_code,
                (unsigned int)trace.model40_shutdown_diagnostic.last_delivered_exception.exception_mask,
                (unsigned int)trace.model40_shutdown_diagnostic.last_delivered_exception.exception_code);
            for (shutdown_index = 0u;
                    shutdown_index < trace.model40_shutdown_diagnostic.recent_count;
                    ++shutdown_index) {
                const core_machine_cpu_execution_point *point =
                    &trace.model40_shutdown_diagnostic.recent[shutdown_index];

                STD_PRINTF("BOOT-PROBE=model40-shutdown-recent-%u=%05X-%02X,%02X,%02X,%02X\n",
                    (unsigned int)shutdown_index, (unsigned int)point->linear_pc,
                    (unsigned int)point->bytes[0u], (unsigned int)point->bytes[1u],
                    (unsigned int)point->bytes[2u], (unsigned int)point->bytes[3u]);
            }
        }
        if (trace.model40_protected_transition_observed) {
            STD_PRINTF("BOOT-PROBE=model40-gdtr-pointer=%02X,%02X,%02X,%02X,%02X,%02X\n",
                (unsigned int)trace.model40_gdtr_pointer[0u],
                (unsigned int)trace.model40_gdtr_pointer[1u],
                (unsigned int)trace.model40_gdtr_pointer[2u],
                (unsigned int)trace.model40_gdtr_pointer[3u],
                (unsigned int)trace.model40_gdtr_pointer[4u],
                (unsigned int)trace.model40_gdtr_pointer[5u]);
            STD_PRINTF("BOOT-PROBE=model40-gdt=%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X\n",
                (unsigned int)trace.model40_gdt[0u], (unsigned int)trace.model40_gdt[1u],
                (unsigned int)trace.model40_gdt[2u], (unsigned int)trace.model40_gdt[3u],
                (unsigned int)trace.model40_gdt[4u], (unsigned int)trace.model40_gdt[5u],
                (unsigned int)trace.model40_gdt[6u], (unsigned int)trace.model40_gdt[7u],
                (unsigned int)trace.model40_gdt[8u], (unsigned int)trace.model40_gdt[9u],
                (unsigned int)trace.model40_gdt[10u], (unsigned int)trace.model40_gdt[11u],
                (unsigned int)trace.model40_gdt[12u], (unsigned int)trace.model40_gdt[13u],
                (unsigned int)trace.model40_gdt[14u], (unsigned int)trace.model40_gdt[15u],
                (unsigned int)trace.model40_gdt[16u], (unsigned int)trace.model40_gdt[17u],
                (unsigned int)trace.model40_gdt[18u], (unsigned int)trace.model40_gdt[19u],
                (unsigned int)trace.model40_gdt[20u], (unsigned int)trace.model40_gdt[21u],
                (unsigned int)trace.model40_gdt[22u], (unsigned int)trace.model40_gdt[23u],
                (unsigned int)trace.model40_gdt[24u], (unsigned int)trace.model40_gdt[25u],
                (unsigned int)trace.model40_gdt[26u], (unsigned int)trace.model40_gdt[27u],
                (unsigned int)trace.model40_gdt[28u], (unsigned int)trace.model40_gdt[29u],
                (unsigned int)trace.model40_gdt[30u], (unsigned int)trace.model40_gdt[31u]);
        }
        STD_PRINTF("BOOT-PROBE=kbc-writes=%llu-last=%05X-%04X-%02X\n",
            (unsigned long long)trace.kbc_write_count,
            (unsigned int)trace.kbc_last_write_pc,
            (unsigned int)trace.kbc_last_write_port,
            (unsigned int)trace.kbc_last_write_value);
        STD_PRINTF("BOOT-PROBE=hdc-phase=%u-status=%02X-error=%02X-command=%02X-count=%u-sector=%u-cylinder=%02X%02X-drive-head=%02X\n",
            (unsigned int)session->core_machine->hdc.data.phase,
            (unsigned int)session->core_machine->hdc.data.status,
            (unsigned int)session->core_machine->hdc.data.error,
            (unsigned int)session->core_machine->hdc.data.last_command,
            (unsigned int)session->core_machine->hdc.data.command_count,
            (unsigned int)session->core_machine->hdc.data.sector_number,
            (unsigned int)session->core_machine->hdc.data.cylinder_high,
            (unsigned int)session->core_machine->hdc.data.cylinder_low,
            (unsigned int)session->core_machine->hdc.data.drive_head);
        if (trace.fdc_port_accesses != 0u) {
            const type_unsigned_64 history = trace.fdc_port_accesses <
                VM_BYOB_FDC_PORT_HISTORY ? trace.fdc_port_accesses :
                VM_BYOB_FDC_PORT_HISTORY;
            type_unsigned_64 entry;

            for (entry = 0u; entry < history; ++entry) {
                const STD_SIZE_T index = (STD_SIZE_T)((trace.fdc_port_accesses - history +
                    entry) % VM_BYOB_FDC_PORT_HISTORY);
                const vm_byob_fdc_port_event *record = &trace.fdc_port_history[index];

                STD_PRINTF("BOOT-PROBE=fdc-port-%05X-%c-%04X-%02X\n",
                    (unsigned int)record->linear_pc, record->write ? 'w' : 'r',
                    (unsigned int)record->port,
                    (unsigned int)record->value);
            }
        }
        if (trace.hdc_port_accesses != 0u) {
            const type_unsigned_64 history = trace.hdc_port_accesses <
                VM_BYOB_HDC_PORT_HISTORY ? trace.hdc_port_accesses :
                VM_BYOB_HDC_PORT_HISTORY;
            type_unsigned_64 entry;

            for (entry = 0u; entry < history; ++entry) {
                const STD_SIZE_T index = (STD_SIZE_T)((trace.hdc_port_accesses - history +
                    entry) % VM_BYOB_HDC_PORT_HISTORY);
                const vm_byob_fdc_port_event *record = &trace.hdc_port_history[index];

                STD_PRINTF("BOOT-PROBE=hdc-port-%05X-%c-%04X-%02X\n",
                    (unsigned int)record->linear_pc, record->write ? 'w' : 'r',
                    (unsigned int)record->port, (unsigned int)record->value);
            }
        }
        {
            type_unsigned_8 index;

            for (index = 0u; index < VM_BYOB_CMOS_BYTES; ++index) {
                if (trace.cmos_reads[index] != 0u || trace.cmos_writes[index] != 0u) {
                    STD_PRINTF("BOOT-PROBE=cmos-%02X-r=%llu-w=%llu-last=%02X-write-pc=%05X\n",
                        (unsigned int)index,
                        (unsigned long long)trace.cmos_reads[index],
                        (unsigned long long)trace.cmos_writes[index],
                        (unsigned int)trace.cmos_last_values[index],
                        (unsigned int)trace.cmos_last_write_pc[index]);
                }
            }
        }
        STD_PRINTF("BOOT-PROBE=pic-imr=%02X-irr=%02X-isr=%02X-fdc-irq=%u\n",
            (unsigned int)session->core_machine->shared_pic_master.data.imr,
            (unsigned int)session->core_machine->shared_pic_master.data.irr,
            (unsigned int)session->core_machine->shared_pic_master.data.isr,
            (unsigned int)session->core_machine->fdc.connect.irq_source.asserted);
        if (core_machine_memory_read(session->core_machine, 0x0410u,
                &bda_equipment, sizeof(bda_equipment)) == TYPE_STATUS_OK &&
            core_machine_memory_read(session->core_machine, 0x0415u,
                &bda_post_status, sizeof(bda_post_status)) == TYPE_STATUS_OK &&
            core_machine_memory_read(session->core_machine, 0x041au,
                &bda_keyboard_head, sizeof(bda_keyboard_head)) == TYPE_STATUS_OK &&
            core_machine_memory_read(session->core_machine, 0x041cu,
                &bda_keyboard_tail, sizeof(bda_keyboard_tail)) == TYPE_STATUS_OK) {
        STD_PRINTF("BOOT-PROBE=bda-equipment=%04X-post-status=%02X-kbd-head=%04X-kbd-tail=%04X\n",
            (unsigned int)bda_equipment, (unsigned int)bda_post_status,
            (unsigned int)bda_keyboard_head, (unsigned int)bda_keyboard_tail);
        (C_VOID)core_machine_memory_read(session->core_machine, 0x0441u,
            &bda_diskette_status, sizeof(bda_diskette_status));
        (C_VOID)core_machine_memory_read(session->core_machine, 0x043eu,
            &bda_motor_wait, sizeof(bda_motor_wait));
        (C_VOID)core_machine_memory_read(session->core_machine, 0x043fu,
            &bda_motor_status, sizeof(bda_motor_status));
        STD_PRINTF("BOOT-PROBE=bda-diskette-status=%02X-motor-wait=%02X-motor-status=%02X\n",
            (unsigned int)bda_diskette_status, (unsigned int)bda_motor_wait,
            (unsigned int)bda_motor_status);
        if (core_machine_memory_read(session->core_machine, 0x0480u,
                bda_wait_state, sizeof(bda_wait_state)) == TYPE_STATUS_OK) {
            STD_PRINTF("BOOT-PROBE=bda-80=%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,"
                "%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X\n",
                (unsigned int)bda_wait_state[0u], (unsigned int)bda_wait_state[1u],
                (unsigned int)bda_wait_state[2u], (unsigned int)bda_wait_state[3u],
                (unsigned int)bda_wait_state[4u], (unsigned int)bda_wait_state[5u],
                (unsigned int)bda_wait_state[6u], (unsigned int)bda_wait_state[7u],
                (unsigned int)bda_wait_state[8u], (unsigned int)bda_wait_state[9u],
                (unsigned int)bda_wait_state[10u], (unsigned int)bda_wait_state[11u],
                (unsigned int)bda_wait_state[12u], (unsigned int)bda_wait_state[13u],
                (unsigned int)bda_wait_state[14u], (unsigned int)bda_wait_state[15u]);
        }
        if (core_machine_memory_read(session->core_machine, 0x0490u,
                bda_wait_state, 4u) == TYPE_STATUS_OK) {
            STD_PRINTF("BOOT-PROBE=bda-fdc-post=%02X,%02X,%02X,%02X\n",
                (unsigned int)bda_wait_state[0u], (unsigned int)bda_wait_state[1u],
                (unsigned int)bda_wait_state[2u], (unsigned int)bda_wait_state[3u]);
        }
        if (core_machine_memory_read(session->core_machine, 0x004cu, &int13_offset,
                sizeof(int13_offset)) == TYPE_STATUS_OK &&
            core_machine_memory_read(session->core_machine, 0x004eu, &int13_segment,
                sizeof(int13_segment)) == TYPE_STATUS_OK) {
            STD_PRINTF("BOOT-PROBE=int13-vector=%04X:%04X\n",
                (unsigned int)int13_segment, (unsigned int)int13_offset);
        }
        if (core_machine_memory_read(session->core_machine, 0x0413u, &bda_memory_kib,
                sizeof(bda_memory_kib)) == TYPE_STATUS_OK) {
            STD_PRINTF("BOOT-PROBE=bda-memory-kib=%u\n", (unsigned int)bda_memory_kib);
        }
        if (core_machine_memory_read(session->core_machine, 0x0078u, &int1e_offset,
                sizeof(int1e_offset)) == TYPE_STATUS_OK &&
            core_machine_memory_read(session->core_machine, 0x007au, &int1e_segment,
                sizeof(int1e_segment)) == TYPE_STATUS_OK) {
            type_unsigned_8 int1e_bytes[4] = {0u};

            (C_VOID)core_machine_memory_read(session->core_machine,
                (type_unsigned_32)int1e_segment * 16u + int1e_offset,
                int1e_bytes, sizeof(int1e_bytes));
            STD_PRINTF("BOOT-PROBE=int1e-vector=%04X:%04X-bytes=%02X,%02X,%02X,%02X\n",
                (unsigned int)int1e_segment, (unsigned int)int1e_offset,
                (unsigned int)int1e_bytes[0u], (unsigned int)int1e_bytes[1u],
                (unsigned int)int1e_bytes[2u], (unsigned int)int1e_bytes[3u]);
        }
        if (core_machine_memory_read(session->core_machine, 0x0038u, &irq6_offset,
                sizeof(irq6_offset)) == TYPE_STATUS_OK &&
            core_machine_memory_read(session->core_machine, 0x003au, &irq6_segment,
                sizeof(irq6_segment)) == TYPE_STATUS_OK) {
            STD_PRINTF("BOOT-PROBE=irq6-vector=%04X:%04X\n",
                (unsigned int)irq6_segment, (unsigned int)irq6_offset);
        }
        if (core_machine_memory_read(session->core_machine, 0x0054u, &int15_offset,
                sizeof(int15_offset)) == TYPE_STATUS_OK &&
            core_machine_memory_read(session->core_machine, 0x0056u, &int15_segment,
                sizeof(int15_segment)) == TYPE_STATUS_OK) {
            STD_PRINTF("BOOT-PROBE=int15-vector=%04X:%04X\n",
                (unsigned int)int15_segment, (unsigned int)int15_offset);
        }
        if (core_machine_memory_read(session->core_machine, 0x0018u, &int6_offset,
                sizeof(int6_offset)) == TYPE_STATUS_OK &&
            core_machine_memory_read(session->core_machine, 0x001au, &int6_segment,
                sizeof(int6_segment)) == TYPE_STATUS_OK) {
            STD_PRINTF("BOOT-PROBE=int6-vector=%04X:%04X\n",
                (unsigned int)int6_segment, (unsigned int)int6_offset);
        }
        if (core_machine_memory_read(session->core_machine, 0x003eu, int13_state,
                sizeof(int13_state)) == TYPE_STATUS_OK) {
            STD_PRINTF("BOOT-PROBE=int13-state=%02X,%02X,%02X,%02X,%02X\n",
                (unsigned int)int13_state[0u], (unsigned int)int13_state[1u],
                (unsigned int)int13_state[2u], (unsigned int)int13_state[3u],
                (unsigned int)int13_state[4u]);
        }
        }
        if (core_machine_memory_read_physical(&session->core_machine->executor_memory,
                session->core_machine->executor_cpu.data.ss.base +
                    (session->core_machine->executor_cpu.data.esp & 0xffffu),
                (type_virtual_address)stack_words, sizeof(stack_words)) == TYPE_STATUS_OK) {
            STD_PRINTF("BOOT-PROBE=stack-ss=%04X-sp=%04X-words=%04X,%04X,%04X,%04X,%04X,%04X,%04X,%04X\n",
                (unsigned int)session->core_machine->executor_cpu.data.ss.selector,
                (unsigned int)(session->core_machine->executor_cpu.data.esp & 0xffffu),
                (unsigned int)stack_words[0u], (unsigned int)stack_words[1u],
                (unsigned int)stack_words[2u], (unsigned int)stack_words[3u],
                (unsigned int)stack_words[4u], (unsigned int)stack_words[5u],
                (unsigned int)stack_words[6u], (unsigned int)stack_words[7u]);
        }
        if (waiting_for_interrupt) {
            STD_PRINTF("BOOT-PROBE=waiting-deadline=%u-if=%u-advanced=%u-pc=%05X\n",
                (unsigned int)waiting_with_deadline,
                (unsigned int)waiting_interrupts_enabled,
                (unsigned int)last_wait_advanced,
                (unsigned int)waiting_linear_pc);
            STD_PRINTF("BOOT-PROBE=waiting-fdc-phase=%u-hdc-phase=%u-dma-pending=%u\n",
                (unsigned int)session->core_machine->fdc.data.phase,
                (unsigned int)session->core_machine->hdc.data.phase,
                (unsigned int)core_machine_dma_has_pending_request(
                    &session->core_machine->shared_dma_primary,
                    &session->core_machine->shared_dma_secondary));
            STD_PRINTF("BOOT-PROBE=waiting-kbc=%llu/%llu/%llu-pit-rule=%u\n",
                (unsigned long long)session->core_machine->shared_kbc.data.typematic_remaining_ticks,
                (unsigned long long)session->core_machine->shared_kbc.data.response_remaining_ticks,
                (unsigned long long)session->core_machine->shared_kbc.data.serial_delivery_remaining_ticks,
                (unsigned int)session->core_machine->timing_plan.controller_timing.pit_clock);
        }
        if (post_resume_required) STD_PRINTF("BOOT-PROBE=post-resume-required\n");
        if (post_memory_failure) STD_PRINTF("BOOT-PROBE=post-memory-failure\n");
        if (post_keyboard_failure) STD_PRINTF("BOOT-PROBE=post-keyboard-failure\n");
        if (post_floppy_failure) STD_PRINTF("BOOT-PROBE=post-floppy-failure\n");
        if (vm_byob_text_memory_has(session->core_machine, "101"))
            STD_PRINTF("BOOT-PROBE=text-system-board-error\n");
        if (vm_byob_text_memory_has(session->core_machine, "201"))
            STD_PRINTF("BOOT-PROBE=text-memory-error\n");
        if (vm_byob_text_memory_has(session->core_machine, "301"))
            STD_PRINTF("BOOT-PROBE=text-keyboard-error\n");
        if (vm_byob_text_memory_has(session->core_machine, "601"))
            STD_PRINTF("BOOT-PROBE=text-diskette-error\n");
        vm_byob_print_text_rows(session->core_machine);
        if (vm_byob_text_memory_has(session->core_machine, "RESUME"))
            STD_PRINTF("BOOT-PROBE=text-resume-required\n");
        if (vm_byob_text_memory_has(session->core_machine, "MS-DOS"))
            STD_PRINTF("BOOT-PROBE=text-msdos\n");
        if (vm_byob_text_memory_has(session->core_machine, "Starting MS-DOS"))
            STD_PRINTF("BOOT-PROBE=text-starting-msdos\n");
        if (vm_byob_text_memory_has(session->core_machine, "Non-System disk"))
            STD_PRINTF("BOOT-PROBE=text-non-system-disk\n");
        if (vm_byob_text_memory_has(session->core_machine, "Replace and press"))
            STD_PRINTF("BOOT-PROBE=text-replace-media\n");
        if (vm_byob_text_memory_has(session->core_machine, "Current date"))
            STD_PRINTF("BOOT-PROBE=text-date-input\n");
        if (vm_byob_text_memory_has(session->core_machine, "A:\\>"))
            STD_PRINTF("BOOT-PROBE=text-dos-prompt\n");
        if (vm_byob_memory_has(session->core_machine, 0x0500u, 11u, "IO      SYS"))
            STD_PRINTF("BOOT-PROBE=root-first-is-io-sys\n");
        if (vm_byob_memory_has(session->core_machine, 0x0520u, 11u, "MSDOS   SYS"))
            STD_PRINTF("BOOT-PROBE=root-second-is-msdos-sys\n");
        if (vm_byob_memory_equal(session->core_machine, 0x0500u, 0x7de6u, 11u))
            STD_PRINTF("BOOT-PROBE=root-first-name-equals-loader\n");
        if (vm_byob_memory_equal(session->core_machine, 0x0520u, 0x7df1u, 11u))
            STD_PRINTF("BOOT-PROBE=root-second-name-equals-loader\n");
        if (trace.boot_loader_jz_observed) {
            STD_PRINTF("BOOT-PROBE=boot-loader-jz-outcome=%u\n",
                (unsigned int)trace.boot_loader_jz_outcome);
        }
        if (trace.boot_loader_read_return_observed) {
            STD_PRINTF("BOOT-PROBE=boot-loader-read-return-flags=%04X-int13-state=%02X,%02X,%02X,%02X,%02X\n",
                (unsigned int)trace.boot_loader_read_return_flags,
                (unsigned int)trace.boot_loader_int13_state[0u],
                (unsigned int)trace.boot_loader_int13_state[1u],
                (unsigned int)trace.boot_loader_int13_state[2u],
                (unsigned int)trace.boot_loader_int13_state[3u],
                (unsigned int)trace.boot_loader_int13_state[4u]);
        }
        if (trace.boot_loader_jz_observed) {
            STD_PRINTF("BOOT-PROBE=boot-loader-compare-ds=%04X-es=%04X-si=%04X-di=%04X-cx=%04X-flags=%04X-left=%02X,%02X,%02X,%02X-right=%02X,%02X,%02X,%02X\n",
                (unsigned int)trace.boot_loader_ds, (unsigned int)trace.boot_loader_es,
                (unsigned int)trace.boot_loader_si, (unsigned int)trace.boot_loader_di,
                (unsigned int)trace.boot_loader_cx, (unsigned int)trace.boot_loader_flags,
                (unsigned int)trace.boot_loader_left[0u],
                (unsigned int)trace.boot_loader_left[1u],
                (unsigned int)trace.boot_loader_left[2u],
                (unsigned int)trace.boot_loader_left[3u],
                (unsigned int)trace.boot_loader_right[0u],
                (unsigned int)trace.boot_loader_right[1u],
                (unsigned int)trace.boot_loader_right[2u],
                (unsigned int)trace.boot_loader_right[3u]);
        }
        if (trace.boot_loader_error_observed) {
            STD_PRINTF("BOOT-PROBE=boot-loader-error-predecessor=%05X\n",
                (unsigned int)trace.boot_loader_previous_pc);
        }
        if (diagnostic.last_delivered_exception.valid) {
            STD_PRINTF("BOOT-PROBE=delivered-exceptions=%u-last-mask=%08X-last-code=%08X\n",
                (unsigned int)diagnostic.delivered_exception_count,
                (unsigned int)diagnostic.last_delivered_exception.exception_mask,
                (unsigned int)diagnostic.last_delivered_exception.exception_code);
            STD_PRINTF("BOOT-PROBE=first-exception-pc=%05X-last-exception-pc=%05X\n",
                (unsigned int)diagnostic.first_delivered_exception.point.linear_pc,
                (unsigned int)diagnostic.last_delivered_exception.point.linear_pc);
            STD_PRINTF("BOOT-PROBE=first-exception-ss=%04X/%08X-esp=%08X-eax=%08X-eflags=%08X\n",
                (unsigned int)diagnostic.first_delivered_exception.ss,
                (unsigned int)diagnostic.first_delivered_exception.ss_base,
                (unsigned int)diagnostic.first_delivered_exception.esp,
                (unsigned int)diagnostic.first_delivered_exception.eax,
                (unsigned int)diagnostic.first_delivered_exception.eflags);
        }
        STD_PRINTF("BOOT-PROBE=last-retired-pc=%05X\n",
            (unsigned int)trace.last_linear_pc);
    }
    if (session != STD_NULL && session->core_machine != STD_NULL) {
        (C_VOID)core_machine_set_trace_provider(session->core_machine, STD_NULL);
    }
    vm_session_destroy(session);
    return exit_code;
}
