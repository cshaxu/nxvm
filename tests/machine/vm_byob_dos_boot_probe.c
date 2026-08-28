#include "type.h"

#include <windows.h>

#include "core/machine/machine_interface.h"
#include "core/machine/machine.h"
#include "core/machine/retirement_observation_interface.h"
#include "core/machine/trace_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/waiting.h"

#define VM_BYOB_BOOT_WALL_LIMIT_MILLISECONDS 60000u
#define VM_BYOB_BOOT_NO_PROGRESS_LIMIT_MILLISECONDS 15000u
#define VM_BYOB_BOOT_DISPLAY_CADENCE_MILLISECONDS 16u
#define VM_BYOB_FDC_PORT_HISTORY 24u

typedef struct vm_byob_fdc_port_event {
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
    type_unsigned_64 xt_ppi_port_accesses;
    type_unsigned_64 pic_port_accesses;
    type_unsigned_64 pit_port_accesses;
    type_unsigned_64 cga_port_accesses;
    type_unsigned_64 cga_memory_writes;
    type_unsigned_64 pit_wait_first_retires;
    type_unsigned_64 pit_wait_second_retires;
    type_unsigned_64 irq0_retires;
    type_unsigned_32 last_linear_pc;
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
} vm_byob_boot_trace;

static C_VOID vm_byob_fdc_port_record(vm_byob_boot_trace *trace,
    const core_machine_trace_event *event)
{
    vm_byob_fdc_port_event *record;
    STD_SIZE_T index;

    if (trace == STD_NULL || event == STD_NULL) return;
    index = (STD_SIZE_T)(trace->fdc_port_accesses % VM_BYOB_FDC_PORT_HISTORY);
    record = &trace->fdc_port_history[index];
    record->port = (type_unsigned_16)event->address;
    record->value = (type_unsigned_8)event->value;
    record->write = event->type == CORE_MACHINE_TRACE_PORT_WRITE;
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

static C_VOID vm_byob_retirement_observe(C_VOID *context,
    const core_machine_retirement_observation *observation)
{
    vm_byob_boot_trace *trace = context;

    if (trace == STD_NULL || observation == STD_NULL) return;
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
    if (event->type == CORE_MACHINE_TRACE_CPU_RETIRE) {
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
    if (event->type != CORE_MACHINE_TRACE_PORT_READ &&
        event->type != CORE_MACHINE_TRACE_PORT_WRITE) return;
    if (event->address >= 0x03f0u && event->address <= 0x03f7u) {
        vm_byob_fdc_port_record(trace, event);
        ++trace->fdc_port_accesses;
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

static C_INT vm_byob_configure(C_INT argc, C_CHAR **argv, vm_session_config *config)
{
    if (argc < 3 || argv == STD_NULL || config == STD_NULL) return 0;
    STD_MEMSET(config, 0, sizeof(*config));
    config->fdd_image = argv[2];
    if (!STD_STRCMP(argv[1], "ibm-5160-model-268")) {
        if (argc != 5) return 0;
        config->profile_kind = VM_SESSION_PROFILE_IBM_5160_MODEL_268;
        config->xt_firmware = (vm_profile_xt_5160_268_byob_manifest) {
            argv[3], argv[4], STD_NULL, STD_NULL, "owner-authorized external BYOB probe"};
        return 1;
    }
    if (!STD_STRCMP(argv[1], "ibm-5170-model-339")) {
        config->profile_kind = VM_SESSION_PROFILE_IBM_5170_MODEL_339;
        return argc == 3 || (argc == 4 &&
            vm_byob_parse_floppy_format(argv[3], &config->floppy_format));
    }
    if (!STD_STRCMP(argv[1], "compaq-deskpro-386-model-40")) {
        if (argc != 7) return 0;
        config->profile_kind = VM_SESSION_PROFILE_COMPAQ_DESKPRO_386_MODEL_40;
        config->model40_firmware = (vm_profile_model40_byob_manifest) {
            argv[3], argv[4], argv[5], argv[6], "owner-authorized external BYOB probe"};
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
    const core_machine_run_budget budget = {256u, 2048u};
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
    C_INT short_budget = argc > 1 && !STD_STRCMP(argv[argc - 1], "--short");
    C_INT trace_enabled;
    C_INT turbo;
    C_INT exit_code = 1;

    if (short_budget) {
        --argc;
        wall_limit = 25000u;
        no_progress_limit = 7500u;
    }
    trace_enabled = argc > 1 && !STD_STRCMP(argv[argc - 1], "--trace");
    if (trace_enabled) --argc;
    turbo = argc > 1 && !STD_STRCMP(argv[argc - 1], "--turbo");
    if (turbo) --argc;
    if (!vm_byob_configure(argc, argv, &config)) {
        STD_PRINTF("BOOT-PROBE=invalid-arguments\n");
        goto done;
    }
    status = vm_session_create(&config, &session);
    if (status != TYPE_STATUS_OK || session == STD_NULL) {
        STD_PRINTF("BOOT-PROBE=session-create-failed-status=%u\n",
            (type_unsigned_32)status);
        goto done;
    }
    trace.machine = session->core_machine;
    if (turbo && vm_session_set_speed(session, VM_SESSION_SPEED_TURBO) != TYPE_STATUS_OK) {
        STD_PRINTF("BOOT-PROBE=setup-failed\n");
        goto done;
    }
    if (core_machine_set_retirement_observation_provider(session->core_machine,
            &(core_machine_retirement_observation_provider) {
                vm_byob_retirement_observe, &trace }) != TYPE_STATUS_OK) {
        STD_PRINTF("BOOT-PROBE=setup-failed\n");
        goto done;
    }
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
    while (GetTickCount64() - started < wall_limit) {
        type_unsigned_32 current;
        ULONGLONG now;

        if (core_machine_run(session->core_machine, budget, &result) != TYPE_STATUS_OK) {
            type_unsigned_8 fault_bytes[4] = {0u};

            (C_VOID)core_machine_memory_read(session->core_machine, result.linear_pc,
                fault_bytes, sizeof(fault_bytes));
            STD_PRINTF("BOOT-PROBE=run-failed-reason=%u-detail=%08X-pc=%05X\n",
                (unsigned int)result.reason, (unsigned int)result.detail,
                (unsigned int)result.linear_pc);
            STD_PRINTF("BOOT-PROBE=fault-bytes=%02X,%02X,%02X,%02X\n",
                (unsigned int)fault_bytes[0u], (unsigned int)fault_bytes[1u],
                (unsigned int)fault_bytes[2u], (unsigned int)fault_bytes[3u]);
            goto done;
        }
        ++run_count;
        last_reason = result.reason;
        last_detail = result.detail;
        executed_total += result.executed;
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
        if (now >= next_display_capture) {
            type_status display_status = core_machine_capture_display_snapshot(
                session->core_machine, &snapshot);

            next_display_capture = now + VM_BYOB_BOOT_DISPLAY_CADENCE_MILLISECONDS;
            if (display_status != TYPE_STATUS_OK && display_status != TYPE_STATUS_UNSUPPORTED) {
                STD_PRINTF("BOOT-PROBE=display-failed-status=%u\n",
                    (unsigned int)display_status);
                goto done;
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
        if (!have_linear_pc || result.linear_pc != linear_pc) {
            linear_pc = result.linear_pc;
            have_linear_pc = 1;
            progress = now;
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
    if (diagnostic.last_delivered_exception.valid) {
        STD_PRINTF("BOOT-PROBE=delivered-exceptions=%u-last-mask=%08X-last-code=%08X\n",
            (unsigned int)diagnostic.delivered_exception_count,
            (unsigned int)diagnostic.last_delivered_exception.exception_mask,
            (unsigned int)diagnostic.last_delivered_exception.exception_code);
    }
    if (trace.reset_events != 0u) STD_PRINTF("BOOT-PROBE=trace-reset\n");
    if (trace.rom_memory_reads != 0u) STD_PRINTF("BOOT-PROBE=trace-rom-read\n");
    if (trace.xt_ppi_port_accesses != 0u) STD_PRINTF("BOOT-PROBE=trace-xt-ppi\n");
    if (trace.pic_port_accesses != 0u) STD_PRINTF("BOOT-PROBE=trace-pic\n");
    if (trace.pit_port_accesses != 0u) STD_PRINTF("BOOT-PROBE=trace-pit\n");
    if (trace.cga_port_accesses != 0u) STD_PRINTF("BOOT-PROBE=trace-cga-port\n");
    if (trace.cga_memory_writes != 0u) STD_PRINTF("BOOT-PROBE=trace-cga-memory\n");
    if (trace.fdc_port_accesses != 0u) STD_PRINTF("BOOT-PROBE=trace-fdc\n");
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
        type_unsigned_16 stack_words[8] = {0};
        type_unsigned_8 bda_post_status = 0u;
        type_unsigned_8 bda_diskette_status = 0u;
        type_unsigned_8 bda_motor_status = 0u;
        type_unsigned_16 int13_offset = 0u;
        type_unsigned_16 int13_segment = 0u;
        type_unsigned_8 int13_state[5] = {0u};
        type_unsigned_8 first_failure_bytes[4] = {0u};
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
        STD_PRINTF("BOOT-PROBE=final-cx=%04X-di=%04X-ds=%04X-es=%04X-ax=%04X-bp=%04X\n",
            (unsigned int)(session->core_machine->executor_cpu.data.ecx & 0xffffu),
            (unsigned int)(session->core_machine->executor_cpu.data.edi & 0xffffu),
            (unsigned int)session->core_machine->executor_cpu.data.ds.selector,
            (unsigned int)session->core_machine->executor_cpu.data.es.selector,
            (unsigned int)(session->core_machine->executor_cpu.data.eax & 0xffffu),
            (unsigned int)(session->core_machine->executor_cpu.data.ebp & 0xffffu));
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
        if (trace.fdc_port_accesses != 0u) {
            const type_unsigned_64 history = trace.fdc_port_accesses <
                VM_BYOB_FDC_PORT_HISTORY ? trace.fdc_port_accesses :
                VM_BYOB_FDC_PORT_HISTORY;
            type_unsigned_64 entry;

            for (entry = 0u; entry < history; ++entry) {
                const STD_SIZE_T index = (STD_SIZE_T)((trace.fdc_port_accesses - history +
                    entry) % VM_BYOB_FDC_PORT_HISTORY);
                const vm_byob_fdc_port_event *record = &trace.fdc_port_history[index];

                STD_PRINTF("BOOT-PROBE=fdc-port-%c-%04X-%02X\n",
                    record->write ? 'w' : 'r', (unsigned int)record->port,
                    (unsigned int)record->value);
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
        (C_VOID)core_machine_memory_read(session->core_machine, 0x043fu,
            &bda_motor_status, sizeof(bda_motor_status));
        STD_PRINTF("BOOT-PROBE=bda-diskette-status=%02X-motor-status=%02X\n",
            (unsigned int)bda_diskette_status, (unsigned int)bda_motor_status);
        if (core_machine_memory_read(session->core_machine, 0x004cu, &int13_offset,
                sizeof(int13_offset)) == TYPE_STATUS_OK &&
            core_machine_memory_read(session->core_machine, 0x004eu, &int13_segment,
                sizeof(int13_segment)) == TYPE_STATUS_OK) {
            STD_PRINTF("BOOT-PROBE=int13-vector=%04X:%04X\n",
                (unsigned int)int13_segment, (unsigned int)int13_offset);
        }
        if (core_machine_memory_read(session->core_machine, 0x003eu, int13_state,
                sizeof(int13_state)) == TYPE_STATUS_OK) {
            STD_PRINTF("BOOT-PROBE=int13-state=%02X,%02X,%02X,%02X,%02X\n",
                (unsigned int)int13_state[0u], (unsigned int)int13_state[1u],
                (unsigned int)int13_state[2u], (unsigned int)int13_state[3u],
                (unsigned int)int13_state[4u]);
        }
        if (core_machine_memory_read(session->core_machine, 0x0001458fu,
                first_failure_bytes, sizeof(first_failure_bytes)) == TYPE_STATUS_OK) {
            STD_PRINTF("BOOT-PROBE=model339-first-ud-bytes=%02X,%02X,%02X,%02X\n",
                (unsigned int)first_failure_bytes[0u],
                (unsigned int)first_failure_bytes[1u],
                (unsigned int)first_failure_bytes[2u],
                (unsigned int)first_failure_bytes[3u]);
        }
        }
        if (core_machine_memory_read(session->core_machine,
                session->core_machine->executor_cpu.data.ss.base +
                    (session->core_machine->executor_cpu.data.esp & 0xffffu),
                stack_words, sizeof(stack_words)) == TYPE_STATUS_OK) {
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
        }
    }
    if (session != STD_NULL && session->core_machine != STD_NULL) {
        (C_VOID)core_machine_set_trace_provider(session->core_machine, STD_NULL);
    }
    vm_session_destroy(session);
    return exit_code;
}
