#include "type.h"

#include <windows.h>

#include "core/platform/presentation_mailbox_interface.h"
#include "core/machine/machine.h"
#include "core/machine/port.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_private.h"
#include "vm/platform/platform.h"
#include "vm/profile/device/floppy.h"
#include "test/integration/support/session_yaml.h"

#define BOOT_TIMEOUT 180000u
#define BOOT_POLL 10u
#define TEXT_CELLS (80u * 25u)
#define ASSET_UNAVAILABLE 77
#define BOOT_TRACE_KBC_TRANSACTIONS 32u
#define BOOT_TRACE_FDC_TRANSACTIONS 64u
#define BOOT_TRACE_FDC_TERMINALS 64u
#define BOOT_TRACE_POST_CODES 64u

typedef struct boot_trace_kbc_transaction {
    type_unsigned_16 port;
    type_unsigned_8 value;
    type_unsigned_8 kind;
} boot_trace_kbc_transaction;

typedef struct boot_trace_fdc_terminal_record {
    core_machine_fdc_terminal_observation result;
    type_unsigned_8 command[9];
    type_unsigned_8 ccr;
} boot_trace_fdc_terminal_record;

typedef struct boot_trace_probe {
    type_unsigned_64 cpu_retires;
    type_unsigned_64 external_cycle_commits;
    type_unsigned_64 port61_reads;
    type_unsigned_64 port61_refresh_low_reads;
    type_unsigned_64 port61_refresh_high_reads;
    type_unsigned_32 pit_writes;
    type_unsigned_32 kbc_writes;
    type_unsigned_32 last_pit_address;
    type_unsigned_32 last_pit_value;
    type_unsigned_32 last_kbc_address;
    type_unsigned_32 last_kbc_value;
    type_unsigned_32 kbc_transaction_count;
    boot_trace_kbc_transaction kbc_transactions[BOOT_TRACE_KBC_TRANSACTIONS];
    type_unsigned_32 fdc_transaction_count;
    boot_trace_kbc_transaction fdc_transactions[BOOT_TRACE_FDC_TRANSACTIONS];
    type_unsigned_32 fdc_read_data_commands;
    type_unsigned_32 fdc_read_id_commands;
    type_unsigned_32 fdc_terminal_count;
    const core_machine_fdc *fdc;
    boot_trace_fdc_terminal_record fdc_terminals[BOOT_TRACE_FDC_TERMINALS];
    type_unsigned_32 kbc_data_read_values[256];
    type_unsigned_32 interrupt_acknowledges;
    type_unsigned_32 interrupt_vectors[256];
    type_unsigned_32 post_interrupt_flag_writes;
    type_unsigned_8 last_post_interrupt_flag;
    type_unsigned_32 post_code_count;
    type_unsigned_8 post_codes[BOOT_TRACE_POST_CODES];
} boot_trace_probe;

static C_VOID boot_trace_fdc_terminal(C_VOID *opaque,
    const core_machine_fdc_terminal_observation *observation)
{
    boot_trace_probe *probe = (boot_trace_probe *)opaque;

    if (probe == STD_NULL || observation == STD_NULL) return;
    boot_trace_fdc_terminal_record *terminal = &probe->fdc_terminals[
        probe->fdc_terminal_count % BOOT_TRACE_FDC_TERMINALS];

    terminal->result = *observation;
    if (probe->fdc != STD_NULL) {
        STD_MEMCPY(terminal->command, probe->fdc->data.cmd, sizeof(terminal->command));
        terminal->ccr = probe->fdc->data.ccr;
    }
    ++probe->fdc_terminal_count;
}

static C_VOID boot_trace_observe(C_VOID *opaque, const core_machine_trace_event *event)
{
    boot_trace_probe *probe = (boot_trace_probe *)opaque;

    if (probe == STD_NULL || event == STD_NULL) return;
    if (event->type == CORE_MACHINE_TRACE_CPU_RETIRE) {
        ++probe->cpu_retires;
        return;
    }
    if (event->type == CORE_MACHINE_TRACE_CPU_EXTERNAL_CYCLE_COMMIT) {
        ++probe->external_cycle_commits;
        return;
    }
    if (event->type == CORE_MACHINE_TRACE_TRANSACTION_COMMIT &&
        (event->address == 0x0060u || event->address == 0x0064u)) {
        probe->kbc_transactions[probe->kbc_transaction_count %
            BOOT_TRACE_KBC_TRANSACTIONS] =
            (boot_trace_kbc_transaction) { (type_unsigned_16)event->address,
                (type_unsigned_8)event->value, (type_unsigned_8)(event->detail >> 8u) };
        ++probe->kbc_transaction_count;
        if (event->address == 0x0060u &&
            (event->detail >> 8u) == CORE_MACHINE_TRANSACTION_CPU_PORT_READ) {
            ++probe->kbc_data_read_values[(type_unsigned_8)event->value];
        }
    }
    if (event->type == CORE_MACHINE_TRACE_TRANSACTION_COMMIT &&
        (event->address == 0x03f5u || event->address == 0x03f7u)) {
        probe->fdc_transactions[probe->fdc_transaction_count %
            BOOT_TRACE_FDC_TRANSACTIONS] =
            (boot_trace_kbc_transaction) { (type_unsigned_16)event->address,
                (type_unsigned_8)event->value, (type_unsigned_8)(event->detail >> 8u) };
        ++probe->fdc_transaction_count;
    }
    if (event->type == CORE_MACHINE_TRACE_TRANSACTION_COMMIT &&
        (event->detail >> 8u) == CORE_MACHINE_TRANSACTION_CPU_INTERRUPT_ACKNOWLEDGE) {
        ++probe->interrupt_acknowledges;
        ++probe->interrupt_vectors[(type_unsigned_8)event->value];
    }
    if (event->type == CORE_MACHINE_TRACE_MEMORY_WRITE && event->address == 0x0000046au) {
        ++probe->post_interrupt_flag_writes;
        probe->last_post_interrupt_flag = (type_unsigned_8)event->value;
    }
    if (event->type == CORE_MACHINE_TRACE_TRANSACTION_COMMIT &&
        (event->detail >> 8u) == CORE_MACHINE_TRANSACTION_CPU_MEMORY_WRITE &&
        event->address == 0x0000046au) {
        ++probe->post_interrupt_flag_writes;
        probe->last_post_interrupt_flag = (type_unsigned_8)event->value;
    }
    if (event->type == CORE_MACHINE_TRACE_PORT_READ && event->address == 0x0061u) {
        ++probe->port61_reads;
        if ((event->value & 0x10u) != 0u) ++probe->port61_refresh_high_reads;
        else ++probe->port61_refresh_low_reads;
        return;
    }
    if (event->type == CORE_MACHINE_TRACE_PORT_WRITE && event->address == 0x0080u) {
        probe->post_codes[probe->post_code_count % BOOT_TRACE_POST_CODES] =
            (type_unsigned_8)event->value;
        ++probe->post_code_count;
    }
    if (event->type != CORE_MACHINE_TRACE_PORT_WRITE) return;
    if (event->address >= 0x0040u && event->address <= 0x0043u) {
        ++probe->pit_writes;
        probe->last_pit_address = event->address;
        probe->last_pit_value = event->value;
    }
    if (event->address == 0x0060u || event->address == 0x0064u) {
        ++probe->kbc_writes;
        probe->last_kbc_address = event->address;
        probe->last_kbc_value = event->value;
    }
}

static C_INT boot_text_has(const core_platform_display_frame *frame, const C_CHAR *text)
{
    STD_SIZE_T cell;
    const STD_SIZE_T length = text == STD_NULL ? 0u : STD_STRLEN(text);

    if (frame == STD_NULL || length == 0u || length > TEXT_CELLS) return 0;
    for (cell = 0u; cell + length <= TEXT_CELLS; ++cell) {
        if (STD_MEMCMP(&frame->characters[cell], text, length) == 0) return 1;
    }
    return 0;
}

static C_INT boot_terminal(const vm_session *session, const C_CHAR **out_name)
{
    core_platform_display_frame frame;
    STD_SIZE_T cell;

    if (session == STD_NULL || out_name == STD_NULL ||
        core_platform_presentation_mailbox_capture(session->presentation_mailbox, &frame) !=
            TYPE_STATUS_OK || frame.kind != CORE_PLATFORM_DISPLAY_KIND_TEXT) return 0;
    for (cell = 0u; cell + 3u < TEXT_CELLS; ++cell) {
        if (STD_ISALPHA((C_UCHAR)frame.characters[cell]) && frame.characters[cell + 1u] == ':' &&
            frame.characters[cell + 2u] == '\\' && frame.characters[cell + 3u] == '>') {
            *out_name = "dos-prompt";
            return 1;
        }
    }
    if (boot_text_has(&frame, "Enter new date")) { *out_name = "date-input"; return 1; }
    if (boot_text_has(&frame, "ENTER=Continue")) { *out_name = "installer-ready"; return 1; }
    if (boot_text_has(&frame, "Setup is determining your system configuration")) {
        *out_name = "installer-running";
        return 1;
    }
    return 0;
}

static C_VOID boot_timeout_report(const vm_session *session, const C_CHAR *name,
    const boot_trace_probe *trace_probe)
{
    core_platform_display_frame frame;
    core_machine_cpu_state cpu;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_observation observation;
    core_machine_time_observation time_observation;
    C_CHAR line[81];
    type_unsigned_8 equipment[2] = {0};
    type_unsigned_8 interrupt_flag = 0u;
    type_unsigned_8 option_signature[2] = {0};
    type_unsigned_8 keyboard_vector[4] = {0};
    type_unsigned_8 pc_bytes[8] = {0};
    type_unsigned_8 boot_bytes[4] = {0};
    type_unsigned_8 boot_signature[2] = {0};
    STD_SIZE_T index;
    STD_SIZE_T row;

    if (session == STD_NULL || name == STD_NULL) return;
    if (core_machine_get_cpu_state(session->core_machine, &cpu) == TYPE_STATUS_OK) {
        (C_VOID)core_machine_capture_observation(session->core_machine, &observation);
        STD_PRINTF("T515:YAML-BOOT:%s:CPU:%04X:%08X:flags=%08X:halted=%u:elapsed=%llu:lifecycle=%u:FDD=%u:%ux%ux%u:CMOS10=%02X\n",
            name, cpu.cs, cpu.eip, cpu.eflags, cpu.halted,
            (unsigned long long)observation.elapsed_ticks, observation.lifecycle,
            session->fdd.connect.flagDiskExist,
            session->fdd.data.ncyl, session->fdd.data.nhead, session->fdd.data.nsector,
            session->core_machine->shared_rtc.registers[CORE_MACHINE_RTC_TYPE_DISK_FLOPPY]);
        STD_PRINTF("T515:YAML-BOOT:%s:FDC:phase=%u:cmd=%02X:index=%u:CHRN=%u/%u/%u:EOT=%u:CCR=%02X:result=%02X/%02X/%02X:remaining=%u:gate=%u:due=%llu:irq=%u\n",
            name, session->core_machine->fdc.data.phase,
            session->core_machine->fdc.data.cmd[0u],
            session->core_machine->fdc.data.command_index,
            session->core_machine->fdc.data.cylinder, session->core_machine->fdc.data.head,
            session->core_machine->fdc.data.sector, session->core_machine->fdc.data.eot,
            session->core_machine->fdc.data.ccr,
            session->core_machine->fdc.data.st0, session->core_machine->fdc.data.st1,
            session->core_machine->fdc.data.st2,
            session->core_machine->fdc.data.transfer_remaining,
            session->core_machine->fdc.data.dma_byte_gate_pending,
            (unsigned long long)session->core_machine->fdc.data.next_dma_byte_tick,
            session->core_machine->fdc.connect.irq_source.asserted);
        if (core_machine_capture_time_observation(session->core_machine,
                &time_observation) == TYPE_STATUS_OK) {
            STD_PRINTF("T515:YAML-BOOT:%s:TIME:deadline=%llu:valid=%u:progress=%u\n",
                name, (unsigned long long)time_observation.next_deadline_tick,
                time_observation.next_deadline_valid,
                time_observation.progress_disposition);
        }
        STD_PRINTF("T515:YAML-BOOT:%s:PIC:IRR=%02X:IMR=%02X:ISR=%02X:IRQ0=%u\n",
            name, session->core_machine->shared_pic_master.data.irr,
            session->core_machine->shared_pic_master.data.imr,
            session->core_machine->shared_pic_master.data.isr,
            session->core_machine->shared_pit_irq0_source.asserted);
        STD_PRINTF("T515:YAML-BOOT:%s:PIT:out0=%u/active=%u/reload=%u/rem=%u:out1=%u/active=%u/reload=%u/rem=%u:clock=%u/%u\n",
            name, session->core_machine->shared_pit.data.flagOutput[0u],
            session->core_machine->shared_pit.data.flagActive[0u],
            session->core_machine->shared_pit.data.reload[0u],
            session->core_machine->shared_pit.data.remaining[0u],
            session->core_machine->shared_pit.data.flagOutput[1u],
            session->core_machine->shared_pit.data.flagActive[1u],
            session->core_machine->shared_pit.data.reload[1u],
            session->core_machine->shared_pit.data.remaining[1u],
            session->core_machine->pit_clock.numerator,
            session->core_machine->pit_clock.denominator);
        STD_PRINTF("T515:YAML-BOOT:%s:CMOS:diag=%02X:floppy=%02X:fixed=%02X:equip=%02X:base=%02X%02X:extended=%02X%02X\n",
            name, session->core_machine->shared_rtc.registers[0x0eu],
            session->core_machine->shared_rtc.registers[0x10u],
            session->core_machine->shared_rtc.registers[0x12u],
            session->core_machine->shared_rtc.registers[0x14u],
            session->core_machine->shared_rtc.registers[0x16u],
            session->core_machine->shared_rtc.registers[0x15u],
            session->core_machine->shared_rtc.registers[0x18u],
            session->core_machine->shared_rtc.registers[0x17u]);
        if (core_machine_memory_read(session->core_machine, 0x00000410u,
                equipment, sizeof(equipment)) == TYPE_STATUS_OK) {
            STD_PRINTF("T515:YAML-BOOT:%s:BDA:equipment=%02X%02X\n", name,
                equipment[1u], equipment[0u]);
        }
        if (core_machine_memory_read(session->core_machine, 0x0000046au,
                &interrupt_flag, 1u) == TYPE_STATUS_OK) {
            STD_PRINTF("T516:YAML-BOOT:%s:POST-INTR-FLAG=%02X\n", name,
                interrupt_flag);
        }
        if (core_machine_memory_read(session->core_machine, 0x00000024u,
                keyboard_vector, sizeof(keyboard_vector)) == TYPE_STATUS_OK) {
            STD_PRINTF("T516:YAML-BOOT:%s:INT09=%02X%02X:%02X%02X\n", name,
                keyboard_vector[1u], keyboard_vector[0u], keyboard_vector[3u],
                keyboard_vector[2u]);
        }
        if (core_machine_memory_read(session->core_machine, 0x000c0000u,
                option_signature, sizeof(option_signature)) == TYPE_STATUS_OK) {
            STD_PRINTF("T515:YAML-BOOT:%s:C0000=%02X%02X\n", name,
                option_signature[0u], option_signature[1u]);
        }
        if (core_machine_memory_read(session->core_machine, 0x00007c00u,
                boot_bytes, sizeof(boot_bytes)) == TYPE_STATUS_OK &&
            core_machine_memory_read(session->core_machine, 0x00007dfeu,
                boot_signature, sizeof(boot_signature)) == TYPE_STATUS_OK) {
            STD_PRINTF("T516:YAML-BOOT:%s:BOOT=%02X/%02X/%02X/%02X:sig=%02X%02X\n",
                name, boot_bytes[0u], boot_bytes[1u], boot_bytes[2u], boot_bytes[3u],
                boot_signature[1u], boot_signature[0u]);
        }
        STD_PRINTF("T515:YAML-BOOT:%s:KBC:output=%02X:command=%02X:fifo=%u:serial=%u:enabled=%u:scan=%u:typematic=%u/%02X/%llu:bat=%u:pending=%u:input-full=%u\n",
            name, session->core_machine->shared_kbc.data.output_port,
            session->core_machine->shared_kbc.data.command_byte,
            session->core_machine->shared_kbc.data.fifo_count,
            session->core_machine->shared_kbc.data.keyboard_serial_count,
            session->core_machine->shared_kbc.data.keyboard_enabled,
            session->core_machine->shared_kbc.data.scanning_enabled,
            session->core_machine->shared_kbc.data.typematic_active,
            session->core_machine->shared_kbc.data.typematic_scan_code,
            (unsigned long long)session->core_machine->shared_kbc.data.typematic_remaining_ticks,
            session->core_machine->shared_kbc.data.keyboard_bat_pending,
            session->core_machine->shared_kbc.data.pending_write,
            session->core_machine->shared_kbc.data.input_buffer_full);
        STD_PRINTF("T516:YAML-BOOT:%s:KBC-QUEUE:delayed=%u/%u:polls=%u:head=%u:", name,
            session->core_machine->shared_kbc.data.delayed_response_index,
            session->core_machine->shared_kbc.data.delayed_response_count,
            session->core_machine->shared_kbc.data.response_status_polls_remaining,
            session->core_machine->shared_kbc.data.fifo_head);
        for (index = 0u; index < session->core_machine->shared_kbc.data.fifo_count;
            ++index) {
            const type_unsigned_8 queue_index = (type_unsigned_8)((
                session->core_machine->shared_kbc.data.fifo_head + index) %
                CORE_MACHINE_KBC_FIFO_CAPACITY);
            STD_PRINTF("%02X/%u ", session->core_machine->shared_kbc.data.fifo[queue_index],
                session->core_machine->shared_kbc.data.fifo_origin[queue_index]);
        }
        STD_PRINTF("\n");
        if (trace_probe != STD_NULL) {
            STD_PRINTF("T516:YAML-BOOT:%s:TRACE:retired=%llu:external=%llu:port61=%llu:low=%llu:high=%llu:ports-pit=%u:last=%04X/%02X:kbc=%u:last=%04X/%02X\n",
                name, (unsigned long long)trace_probe->cpu_retires,
                (unsigned long long)trace_probe->external_cycle_commits,
                (unsigned long long)trace_probe->port61_reads,
                (unsigned long long)trace_probe->port61_refresh_low_reads,
                (unsigned long long)trace_probe->port61_refresh_high_reads,
                trace_probe->pit_writes, trace_probe->last_pit_address,
                trace_probe->last_pit_value, trace_probe->kbc_writes,
                trace_probe->last_kbc_address, trace_probe->last_kbc_value);
            STD_PRINTF("T516:YAML-BOOT:%s:INTA=%u:IRQ1=%u:IRQ6=%u\n", name,
                trace_probe->interrupt_acknowledges, trace_probe->interrupt_vectors[0x09u],
                trace_probe->interrupt_vectors[0x0eu]);
            STD_PRINTF("T516:YAML-BOOT:%s:POST-INTR-WRITES=%u:last=%02X\n", name,
                trace_probe->post_interrupt_flag_writes,
                trace_probe->last_post_interrupt_flag);
            STD_PRINTF("T516:YAML-BOOT:%s:POST-CODES:", name);
            const type_unsigned_32 retained_post = trace_probe->post_code_count <
                BOOT_TRACE_POST_CODES ? trace_probe->post_code_count : BOOT_TRACE_POST_CODES;
            const type_unsigned_32 first_post = trace_probe->post_code_count - retained_post;
            for (index = 0u; index < retained_post; ++index) {
                STD_PRINTF("%02X ", trace_probe->post_codes[(first_post + index) %
                    BOOT_TRACE_POST_CODES]);
            }
            STD_PRINTF("\n");
            STD_PRINTF("T516:YAML-BOOT:%s:KBC-CPU:", name);
            const type_unsigned_32 retained = trace_probe->kbc_transaction_count <
                BOOT_TRACE_KBC_TRANSACTIONS ? trace_probe->kbc_transaction_count :
                BOOT_TRACE_KBC_TRANSACTIONS;
            const type_unsigned_32 first = trace_probe->kbc_transaction_count - retained;
            for (index = 0u; index < retained; ++index) {
                const boot_trace_kbc_transaction *transaction =
                    &trace_probe->kbc_transactions[(first + index) %
                        BOOT_TRACE_KBC_TRANSACTIONS];
                STD_PRINTF("%04X/%02X/%u ", transaction->port,
                    transaction->value, transaction->kind);
            }
            STD_PRINTF("\n");
            STD_PRINTF("T516:YAML-BOOT:%s:KBC-READS:00=%u:55=%u:65=%u:AA=%u:FA=%u:AB=%u:83=%u\n",
                name, trace_probe->kbc_data_read_values[0x00u],
                trace_probe->kbc_data_read_values[0x55u],
                trace_probe->kbc_data_read_values[0x65u],
                trace_probe->kbc_data_read_values[0xaau],
                trace_probe->kbc_data_read_values[0xfau],
                trace_probe->kbc_data_read_values[0xabu],
                trace_probe->kbc_data_read_values[0x83u]);
            STD_PRINTF("T516:YAML-BOOT:%s:FDC-CPU:read=%u:id=%u:", name,
                trace_probe->fdc_read_data_commands, trace_probe->fdc_read_id_commands);
            const type_unsigned_32 retained_fdc = trace_probe->fdc_transaction_count <
                BOOT_TRACE_FDC_TRANSACTIONS ? trace_probe->fdc_transaction_count :
                BOOT_TRACE_FDC_TRANSACTIONS;
            const type_unsigned_32 first_fdc = trace_probe->fdc_transaction_count -
                retained_fdc;
            for (index = 0u; index < retained_fdc; ++index) {
                const boot_trace_kbc_transaction *transaction =
                    &trace_probe->fdc_transactions[(first_fdc + index) %
                        BOOT_TRACE_FDC_TRANSACTIONS];
                STD_PRINTF("%04X/%02X/%u ", transaction->port,
                    transaction->value, transaction->kind);
            }
            STD_PRINTF("\n");
            STD_PRINTF("T516:YAML-BOOT:%s:FDC-TERMINALS:", name);
            const type_unsigned_32 retained_terminal = trace_probe->fdc_terminal_count <
                BOOT_TRACE_FDC_TERMINALS ? trace_probe->fdc_terminal_count :
                BOOT_TRACE_FDC_TERMINALS;
            const type_unsigned_32 first_terminal = trace_probe->fdc_terminal_count -
                retained_terminal;
            for (index = 0u; index < retained_terminal; ++index) {
                const boot_trace_fdc_terminal_record *terminal =
                    &trace_probe->fdc_terminals[(first_terminal + index) %
                        BOOT_TRACE_FDC_TERMINALS];
                STD_PRINTF("%02X:%u:%02X/%02X/%02X/%02X/%02X/%02X/%02X:%u:cmd=%02X/%02X/%02X/%02X/%02X/%02X/%02X/%02X/%02X:ccr=%02X ",
                    terminal->result.command, terminal->result.drive,
                    terminal->result.result[0u], terminal->result.result[1u],
                    terminal->result.result[2u], terminal->result.result[3u],
                    terminal->result.result[4u], terminal->result.result[5u],
                    terminal->result.result[6u], terminal->result.successful,
                    terminal->command[0u], terminal->command[1u], terminal->command[2u],
                    terminal->command[3u], terminal->command[4u], terminal->command[5u],
                    terminal->command[6u], terminal->command[7u], terminal->command[8u],
                    terminal->ccr);
            }
            STD_PRINTF("\n");
        }
        STD_PRINTF("T515:YAML-BOOT:%s:REGS:EAX=%08X:EBX=%08X:ECX=%08X:EDX=%08X\n",
            name, session->core_machine->executor_cpu.data.eax,
            session->core_machine->executor_cpu.data.ebx,
            session->core_machine->executor_cpu.data.ecx,
            session->core_machine->executor_cpu.data.edx);
        if (core_machine_memory_read(session->core_machine,
                ((type_unsigned_32)cpu.cs << 4u) + cpu.eip, pc_bytes,
                sizeof(pc_bytes)) == TYPE_STATUS_OK) {
            STD_PRINTF("T516:YAML-BOOT:%s:PC-BYTES:%02X/%02X/%02X/%02X/%02X/%02X/%02X/%02X\n",
                name, pc_bytes[0u], pc_bytes[1u], pc_bytes[2u], pc_bytes[3u],
                pc_bytes[4u], pc_bytes[5u], pc_bytes[6u], pc_bytes[7u]);
        }
        if (core_machine_get_cpu_diagnostic(session->core_machine, &diagnostic) ==
                TYPE_STATUS_OK && diagnostic.recent_count != 0u) {
            const core_machine_cpu_execution_point *point =
                &diagnostic.recent[diagnostic.recent_count - 1u];
            STD_PRINTF("T515:YAML-BOOT:%s:RECENT:CS=%04X:EIP=%08X:bytes=%02X/%02X/%02X\n",
                name, point->cs, point->eip, point->bytes[0u], point->bytes[1u],
                point->bytes[2u]);
        }
    }
    if (core_platform_presentation_mailbox_capture(session->presentation_mailbox,
            &frame) != TYPE_STATUS_OK || frame.kind != CORE_PLATFORM_DISPLAY_KIND_TEXT) return;
    for (row = 0u; row < 25u; ++row) {
        C_INT nonblank = 0;
        for (index = 0u; index < 80u; ++index) {
            C_UCHAR character = (C_UCHAR)frame.characters[row * 80u + index];
            line[index] = character >= 0x20u && character < 0x7fu ? (C_CHAR)character : ' ';
            nonblank |= line[index] != ' ';
        }
        line[80u] = '\0';
        if (nonblank) STD_PRINTF("T515:YAML-BOOT:%s:SCREEN:%u:%s\n", name,
            (unsigned int)row, line);
    }
}

static C_INT boot_timeout_parse(const C_CHAR *text, DWORD *out_timeout)
{
    type_unsigned_64 value = 0u;

    if (text == STD_NULL || out_timeout == STD_NULL || *text == '\0') return 0;
    while (*text != '\0') {
        if (*text < '0' || *text > '9' || value > 429496729u) return 0;
        value = value * 10u + (type_unsigned_64)(*text - '0');
        if (value > 4294967295u) return 0;
        ++text;
    }
    *out_timeout = (DWORD)value;
    return value != 0u;
}

static DWORD WINAPI boot_start(C_VOID *opaque)
{
    type_status status = vm_session_start((vm_session *)opaque);

    if (status != TYPE_STATUS_OK) {
        STD_PRINTF("T515:YAML-BOOT:START-FAILED:%d\n", (C_INT)status);
    }
    return 0u;
}

static C_INT boot_cmos_seed_matches(const vm_session *session,
    const vm_product_session_request *request)
{
    type_unsigned_8 seed[VM_SESSION_CMOS_SEED_BYTES];
    t_port *port;
    STD_FILE *file;
    type_unsigned_8 index;
    C_INT loaded;

    if (session == STD_NULL || request == STD_NULL || request->cmos[0] == '\0') return 1;
    file = STD_FOPEN(request->cmos, "rb");
    if (file == STD_NULL) return 0;
    loaded = STD_FREAD(seed, 1u, sizeof(seed), file) == sizeof(seed) &&
        STD_FCLOSE(file) == 0;
    if (!loaded) return 0;
    port = &session->core_machine->executor_port;
    for (index = 0x0eu; index < VM_SESSION_CMOS_SEED_BYTES; ++index) {
        type_unsigned_8 expected = seed[index];
        type_unsigned_8 actual;

        core_machine_port_write(port, 0x0070u, index);
        actual = (type_unsigned_8)core_machine_port_read(port, 0x0071u);
        if (actual != expected) {
            STD_PRINTF("T515:CMOS:index=%02X:expected=%02X:actual=%02X\n", index,
                expected, actual);
            return 0;
        }
    }
    return 1;
}

int main(int argc, char **argv)
{
    integration_yaml_session yaml_session;
    vm_session *session;
    HANDLE thread = STD_NULL;
    const C_CHAR *terminal = STD_NULL;
    DWORD timeout = BOOT_TIMEOUT;
    ULONGLONG started;
    boot_trace_probe trace_probe = {0};
    C_INT trace_enabled = 0;
    C_INT result = 1;

    if ((argc != 3 && argc != 4 && argc != 5) ||
        (argc >= 4 && !boot_timeout_parse(argv[3], &timeout)) ||
        (argc == 5 && STD_STRCMP(argv[4], "trace") != 0)) {
        return 1;
    }
    trace_enabled = argc == 5;
    if (integration_yaml_session_open(argv[1], argv[2], &yaml_session) ==
        TYPE_STATUS_UNSUPPORTED) {
        STD_PRINTF("T515:YAML-BOOT:%s:UNAVAILABLE\n", argv[2]);
        return ASSET_UNAVAILABLE;
    }
    if (yaml_session.session == STD_NULL) return 1;
    session = yaml_session.session;
    if (trace_enabled) {
        trace_probe.fdc = &session->core_machine->fdc;
        (C_VOID)core_machine_set_trace_provider(session->core_machine,
            &(core_machine_trace_provider) {boot_trace_observe, &trace_probe});
        session->core_machine->fdc.connect.observation_provider =
            (core_machine_fdc_terminal_observation_provider) {boot_trace_fdc_terminal,
                &trace_probe};
    }
    if (!boot_cmos_seed_matches(session, &yaml_session.request)) {
        STD_PRINTF("T515:YAML-BOOT:%s:CMOS-SEED-MISMATCH\n", argv[2]);
        goto done;
    }
    vm_platform_run_context_set_display_mode(session->platform_run_context,
        !STD_STRCMP(yaml_session.request.display, "window") ? VM_PLATFORM_DISPLAY_WINDOW :
        !STD_STRCMP(yaml_session.request.display, "auto") ? VM_PLATFORM_DISPLAY_AUTO :
        VM_PLATFORM_DISPLAY_CONSOLE);
    if (vm_session_set_speed(session, VM_SESSION_SPEED_TURBO) != TYPE_STATUS_OK ||
        (thread = CreateThread(STD_NULL, 0u, boot_start, session, 0u, STD_NULL)) == STD_NULL) goto done;
    started = GetTickCount64();
    while (GetTickCount64() - started < timeout) {
        if (boot_terminal(session, &terminal)) break;
        Sleep(BOOT_POLL);
    }
    if (terminal == STD_NULL) {
        vm_session_control_request_pause(&session->control, VM_SESSION_PAUSE_EXPLICIT);
        if (vm_session_control_wait_for_pause(&session->control, 2000u)) {
            boot_timeout_report(session, argv[2], trace_enabled ? &trace_probe : STD_NULL);
        }
    }
    vm_session_stop(session);
    if (WaitForSingleObject(thread, 2000u) != WAIT_OBJECT_0) goto done;
    CloseHandle(thread); thread = STD_NULL;
    if (terminal == STD_NULL) {
        STD_PRINTF("T515:YAML-BOOT:%s:TERMINAL-TIMEOUT\n", argv[2]);
        goto done;
    }
    STD_PRINTF("T515:YAML-BOOT:%s:%s\n", argv[2], terminal);
    result = 0;
done:
    if (thread != STD_NULL) CloseHandle(thread);
    integration_yaml_session_close(&yaml_session);
    return result;
}
