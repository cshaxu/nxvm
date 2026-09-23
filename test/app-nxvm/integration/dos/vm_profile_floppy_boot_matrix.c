#include "lib/types/types_interface.h"
#include "type.h"

#include <windows.h>

#include "test/app-nxvm/unit/core/machine/support/vm_presentation_capture.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/port.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/profiles/device/floppy.h"
#include "test/app-nxvm/integration/support/session_ini.h"

#define BOOT_TIMEOUT 180000u
#define BOOT_POLL 10u
#define TEXT_CELLS (80u * 25u)
#define ASSET_UNAVAILABLE 77
#define BOOT_TRACE_KBC_TRANSACTIONS 32u
#define BOOT_TRACE_FDC_TRANSACTIONS 64u
#define BOOT_TRACE_FDC_TERMINALS 64u
#define BOOT_TRACE_POST_CODES 64u

typedef struct boot_trace_kbc_transaction {
    lib_u16 port;
    lib_u8 value;
    lib_u8 kind;
} boot_trace_kbc_transaction;

typedef struct boot_trace_fdc_terminal_record {
    core_machine_fdc_terminal_observation result;
    lib_u8 command[9];
    lib_u8 ccr;
} boot_trace_fdc_terminal_record;

typedef struct boot_trace_probe {
    lib_u64 cpu_retires;
    lib_u64 external_cycle_commits;
    lib_u64 port61_reads;
    lib_u64 port61_refresh_low_reads;
    lib_u64 port61_refresh_high_reads;
    lib_u32 pit_writes;
    lib_u32 kbc_writes;
    lib_u32 last_pit_address;
    lib_u32 last_pit_value;
    lib_u32 last_kbc_address;
    lib_u32 last_kbc_value;
    lib_u32 kbc_transaction_count;
    boot_trace_kbc_transaction kbc_transactions[BOOT_TRACE_KBC_TRANSACTIONS];
    lib_u32 fdc_transaction_count;
    boot_trace_kbc_transaction fdc_transactions[BOOT_TRACE_FDC_TRANSACTIONS];
    lib_u32 fdc_read_data_commands;
    lib_u32 fdc_read_id_commands;
    lib_u32 fdc_terminal_count;
    const core_machine_fdc *fdc;
    boot_trace_fdc_terminal_record fdc_terminals[BOOT_TRACE_FDC_TERMINALS];
    lib_u32 kbc_data_read_values[256];
    lib_u32 interrupt_acknowledges;
    lib_u32 interrupt_vectors[256];
    lib_u32 post_interrupt_flag_writes;
    lib_u8 last_post_interrupt_flag;
    lib_u32 post_code_count;
    lib_u8 post_codes[BOOT_TRACE_POST_CODES];
} boot_trace_probe;

static C_VOID boot_trace_fdc_terminal(C_VOID *opaque,
    const core_machine_fdc_terminal_observation *observation)
{
    boot_trace_probe *probe = (boot_trace_probe *)opaque;

    if (probe == LIB_NULL || observation == LIB_NULL) return;
    boot_trace_fdc_terminal_record *terminal = &probe->fdc_terminals[
        probe->fdc_terminal_count % BOOT_TRACE_FDC_TERMINALS];

    terminal->result = *observation;
    if (probe->fdc != LIB_NULL) {
        lib_memory_copy(terminal->command, probe->fdc->data.cmd, sizeof(terminal->command));
        terminal->ccr = probe->fdc->data.ccr;
    }
    ++probe->fdc_terminal_count;
}

static C_VOID boot_trace_observe(C_VOID *opaque, const core_machine_trace_event *event)
{
    boot_trace_probe *probe = (boot_trace_probe *)opaque;

    if (probe == LIB_NULL || event == LIB_NULL) return;
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
            (boot_trace_kbc_transaction) { (lib_u16)event->address,
                (lib_u8)event->value, (lib_u8)(event->detail >> 8u) };
        ++probe->kbc_transaction_count;
        if (event->address == 0x0060u &&
            (event->detail >> 8u) == CORE_MACHINE_TRANSACTION_CPU_PORT_READ) {
            ++probe->kbc_data_read_values[(lib_u8)event->value];
        }
    }
    if (event->type == CORE_MACHINE_TRACE_TRANSACTION_COMMIT &&
        (event->address == 0x03f5u || event->address == 0x03f7u)) {
        probe->fdc_transactions[probe->fdc_transaction_count %
            BOOT_TRACE_FDC_TRANSACTIONS] =
            (boot_trace_kbc_transaction) { (lib_u16)event->address,
                (lib_u8)event->value, (lib_u8)(event->detail >> 8u) };
        ++probe->fdc_transaction_count;
    }
    if (event->type == CORE_MACHINE_TRACE_TRANSACTION_COMMIT &&
        (event->detail >> 8u) == CORE_MACHINE_TRANSACTION_CPU_INTERRUPT_ACKNOWLEDGE) {
        ++probe->interrupt_acknowledges;
        ++probe->interrupt_vectors[(lib_u8)event->value];
    }
    if (event->type == CORE_MACHINE_TRACE_MEMORY_WRITE && event->address == 0x0000046au) {
        ++probe->post_interrupt_flag_writes;
        probe->last_post_interrupt_flag = (lib_u8)event->value;
    }
    if (event->type == CORE_MACHINE_TRACE_TRANSACTION_COMMIT &&
        (event->detail >> 8u) == CORE_MACHINE_TRANSACTION_CPU_MEMORY_WRITE &&
        event->address == 0x0000046au) {
        ++probe->post_interrupt_flag_writes;
        probe->last_post_interrupt_flag = (lib_u8)event->value;
    }
    if (event->type == CORE_MACHINE_TRACE_PORT_READ && event->address == 0x0061u) {
        ++probe->port61_reads;
        if ((event->value & 0x10u) != 0u) ++probe->port61_refresh_high_reads;
        else ++probe->port61_refresh_low_reads;
        return;
    }
    if (event->type == CORE_MACHINE_TRACE_PORT_WRITE && event->address == 0x0080u) {
        probe->post_codes[probe->post_code_count % BOOT_TRACE_POST_CODES] =
            (lib_u8)event->value;
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

static C_INT boot_text_has(const core_machine_guest_display_frame *frame, const C_CHAR *text)
{
    lib_size cell;
    const lib_size length = text == LIB_NULL ? 0u : lib_text_length(text);

    if (frame == LIB_NULL || length == 0u || length > TEXT_CELLS) return 0;
    for (cell = 0u; cell + length <= TEXT_CELLS; ++cell) {
        if (lib_memory_compare(&frame->characters[cell], text, length) == 0) return 1;
    }
    return 0;
}

static C_INT boot_terminal(const vm_machine *session, const C_CHAR **out_name)
{
    core_machine_guest_display_frame frame;
    lib_size cell;

    if (session == LIB_NULL || out_name == LIB_NULL ||
        test_vm_machine_capture_presentation(session, &frame) !=
            TYPE_STATUS_OK || frame.kind != CORE_MACHINE_GUEST_DISPLAY_KIND_TEXT) return 0;
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

static C_INT boot_post_reports_keyboard_failure(const vm_machine *session)
{
    core_machine_guest_display_frame frame;

    return session != LIB_NULL &&
        test_vm_machine_capture_presentation(session, &frame) ==
            TYPE_STATUS_OK && frame.kind == CORE_MACHINE_GUEST_DISPLAY_KIND_TEXT &&
        (boot_text_has(&frame, "301-Keyboard") || boot_text_has(&frame, "303-Keyboard"));
}

static C_VOID boot_timeout_report(const vm_machine *session, const C_CHAR *name,
    const boot_trace_probe *trace_probe)
{
    core_machine_guest_display_frame frame;
    core_machine_cpu_state cpu;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_observation observation;
    core_machine_time_observation time_observation;
    C_CHAR line[81];
    lib_u8 equipment[2] = {0};
    lib_u8 interrupt_flag = 0u;
    lib_u8 option_signature[2] = {0};
    lib_u8 keyboard_vector[4] = {0};
    lib_u8 pc_bytes[8] = {0};
    lib_u8 boot_bytes[4] = {0};
    lib_u8 boot_signature[2] = {0};
    lib_size index;
    lib_size row;

    if (session == LIB_NULL || name == LIB_NULL) return;
    if (core_machine_get_cpu_state(session->core_machine, &cpu) == TYPE_STATUS_OK) {
        (C_VOID)core_machine_capture_observation(session->core_machine, &observation);
        STD_PRINTF("T515:INI-BOOT:%s:CPU:%04X:%08X:base=%08X:flags=%08X:halted=%u:elapsed=%llu:lifecycle=%u:FDD=%u:%ux%ux%u:CMOS10=%02X\n",
            name, cpu.cs, cpu.eip, cpu.cs_base, cpu.eflags, cpu.halted,
            (unsigned long long)observation.elapsed_ticks, observation.lifecycle,
            session->fdd.connect.flagDiskExist,
            session->fdd.data.ncyl, session->fdd.data.nhead, session->fdd.data.nsector,
            session->core_machine->shared_rtc.registers[CORE_MACHINE_RTC_TYPE_DISK_FLOPPY]);
        STD_PRINTF("T515:INI-BOOT:%s:FDC:phase=%u:cmd=%02X:index=%u:CHRN=%u/%u/%u:EOT=%u:CCR=%02X:result=%02X/%02X/%02X:remaining=%u:gate=%u:due=%llu:irq=%u\n",
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
            STD_PRINTF("T515:INI-BOOT:%s:TIME:deadline=%llu:valid=%u:progress=%u\n",
                name, (unsigned long long)time_observation.next_deadline_tick,
                time_observation.next_deadline_valid,
                time_observation.progress_disposition);
        }
        STD_PRINTF("T515:INI-BOOT:%s:PIC:IRR=%02X:IMR=%02X:ISR=%02X:IRQ0=%u\n",
            name, session->core_machine->shared_pic_master.data.irr,
            session->core_machine->shared_pic_master.data.imr,
            session->core_machine->shared_pic_master.data.isr,
            session->core_machine->shared_pit_irq0_source.asserted);
        STD_PRINTF("T515:INI-BOOT:%s:PIT:out0=%u/active=%u/reload=%u/rem=%u:out1=%u/active=%u/reload=%u/rem=%u:clock=%u/%u\n",
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
        STD_PRINTF("T526:S13:INI-BOOT:%s:PIT0:control=%02X:count=%04X:latch=%04X:latched=%u:read=%u\n",
            name, session->core_machine->shared_pit.data.cw[0u],
            session->core_machine->shared_pit.data.count[0u],
            session->core_machine->shared_pit.data.latch[0u],
            session->core_machine->shared_pit.data.flagLatch[0u],
            session->core_machine->shared_pit.data.flagRead[0u]);
        STD_PRINTF("T515:INI-BOOT:%s:CMOS:diag=%02X:floppy=%02X:fixed=%02X:equip=%02X:base=%02X%02X:extended=%02X%02X\n",
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
            STD_PRINTF("T515:INI-BOOT:%s:BDA:equipment=%02X%02X\n", name,
                equipment[1u], equipment[0u]);
        }
        if (core_machine_memory_read(session->core_machine, 0x0000046au,
                &interrupt_flag, 1u) == TYPE_STATUS_OK) {
            STD_PRINTF("T516:INI-BOOT:%s:POST-INTR-FLAG=%02X\n", name,
                interrupt_flag);
        }
        if (core_machine_memory_read(session->core_machine, 0x00000024u,
                keyboard_vector, sizeof(keyboard_vector)) == TYPE_STATUS_OK) {
            STD_PRINTF("T516:INI-BOOT:%s:INT09=%02X%02X:%02X%02X\n", name,
                keyboard_vector[1u], keyboard_vector[0u], keyboard_vector[3u],
                keyboard_vector[2u]);
        }
        if (core_machine_memory_read(session->core_machine, 0x000c0000u,
                option_signature, sizeof(option_signature)) == TYPE_STATUS_OK) {
            STD_PRINTF("T515:INI-BOOT:%s:C0000=%02X%02X\n", name,
                option_signature[0u], option_signature[1u]);
        }
        if (core_machine_memory_read(session->core_machine, 0x00007c00u,
                boot_bytes, sizeof(boot_bytes)) == TYPE_STATUS_OK &&
            core_machine_memory_read(session->core_machine, 0x00007dfeu,
                boot_signature, sizeof(boot_signature)) == TYPE_STATUS_OK) {
            STD_PRINTF("T516:INI-BOOT:%s:BOOT=%02X/%02X/%02X/%02X:sig=%02X%02X\n",
                name, boot_bytes[0u], boot_bytes[1u], boot_bytes[2u], boot_bytes[3u],
                boot_signature[1u], boot_signature[0u]);
        }
        STD_PRINTF("T515:INI-BOOT:%s:KBC:output=%02X:command=%02X:fifo=%u:serial=%u:enabled=%u:scan=%u:typematic=%u/%02X/%llu:bat=%u:pending=%u:input-full=%u\n",
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
        STD_PRINTF("T516:INI-BOOT:%s:KBC-QUEUE:delayed=%u/%u:polls=%u:head=%u:", name,
            session->core_machine->shared_kbc.data.delayed_response_index,
            session->core_machine->shared_kbc.data.delayed_response_count,
            session->core_machine->shared_kbc.data.response_status_polls_remaining,
            session->core_machine->shared_kbc.data.fifo_head);
        for (index = 0u; index < session->core_machine->shared_kbc.data.fifo_count;
            ++index) {
            const lib_u8 queue_index = (lib_u8)((
                session->core_machine->shared_kbc.data.fifo_head + index) %
                CORE_MACHINE_KBC_FIFO_CAPACITY);
            STD_PRINTF("%02X/%u ", session->core_machine->shared_kbc.data.fifo[queue_index],
                session->core_machine->shared_kbc.data.fifo_origin[queue_index]);
        }
        STD_PRINTF("\n");
        if (trace_probe != LIB_NULL) {
            STD_PRINTF("T516:INI-BOOT:%s:TRACE:retired=%llu:external=%llu:port61=%llu:low=%llu:high=%llu:ports-pit=%u:last=%04X/%02X:kbc=%u:last=%04X/%02X\n",
                name, (unsigned long long)trace_probe->cpu_retires,
                (unsigned long long)trace_probe->external_cycle_commits,
                (unsigned long long)trace_probe->port61_reads,
                (unsigned long long)trace_probe->port61_refresh_low_reads,
                (unsigned long long)trace_probe->port61_refresh_high_reads,
                trace_probe->pit_writes, trace_probe->last_pit_address,
                trace_probe->last_pit_value, trace_probe->kbc_writes,
                trace_probe->last_kbc_address, trace_probe->last_kbc_value);
            STD_PRINTF("T516:INI-BOOT:%s:INTA=%u:IRQ1=%u:IRQ6=%u\n", name,
                trace_probe->interrupt_acknowledges, trace_probe->interrupt_vectors[0x09u],
                trace_probe->interrupt_vectors[0x0eu]);
            STD_PRINTF("T516:INI-BOOT:%s:POST-INTR-WRITES=%u:last=%02X\n", name,
                trace_probe->post_interrupt_flag_writes,
                trace_probe->last_post_interrupt_flag);
            STD_PRINTF("T516:INI-BOOT:%s:POST-CODES:", name);
            const lib_u32 retained_post = trace_probe->post_code_count <
                BOOT_TRACE_POST_CODES ? trace_probe->post_code_count : BOOT_TRACE_POST_CODES;
            const lib_u32 first_post = trace_probe->post_code_count - retained_post;
            for (index = 0u; index < retained_post; ++index) {
                STD_PRINTF("%02X ", trace_probe->post_codes[(first_post + index) %
                    BOOT_TRACE_POST_CODES]);
            }
            STD_PRINTF("\n");
            STD_PRINTF("T516:INI-BOOT:%s:KBC-CPU:", name);
            const lib_u32 retained = trace_probe->kbc_transaction_count <
                BOOT_TRACE_KBC_TRANSACTIONS ? trace_probe->kbc_transaction_count :
                BOOT_TRACE_KBC_TRANSACTIONS;
            const lib_u32 first = trace_probe->kbc_transaction_count - retained;
            for (index = 0u; index < retained; ++index) {
                const boot_trace_kbc_transaction *transaction =
                    &trace_probe->kbc_transactions[(first + index) %
                        BOOT_TRACE_KBC_TRANSACTIONS];
                STD_PRINTF("%04X/%02X/%u ", transaction->port,
                    transaction->value, transaction->kind);
            }
            STD_PRINTF("\n");
            STD_PRINTF("T516:INI-BOOT:%s:KBC-READS:00=%u:55=%u:65=%u:AA=%u:FA=%u:AB=%u:83=%u\n",
                name, trace_probe->kbc_data_read_values[0x00u],
                trace_probe->kbc_data_read_values[0x55u],
                trace_probe->kbc_data_read_values[0x65u],
                trace_probe->kbc_data_read_values[0xaau],
                trace_probe->kbc_data_read_values[0xfau],
                trace_probe->kbc_data_read_values[0xabu],
                trace_probe->kbc_data_read_values[0x83u]);
            STD_PRINTF("T516:INI-BOOT:%s:FDC-CPU:read=%u:id=%u:", name,
                trace_probe->fdc_read_data_commands, trace_probe->fdc_read_id_commands);
            const lib_u32 retained_fdc = trace_probe->fdc_transaction_count <
                BOOT_TRACE_FDC_TRANSACTIONS ? trace_probe->fdc_transaction_count :
                BOOT_TRACE_FDC_TRANSACTIONS;
            const lib_u32 first_fdc = trace_probe->fdc_transaction_count -
                retained_fdc;
            for (index = 0u; index < retained_fdc; ++index) {
                const boot_trace_kbc_transaction *transaction =
                    &trace_probe->fdc_transactions[(first_fdc + index) %
                        BOOT_TRACE_FDC_TRANSACTIONS];
                STD_PRINTF("%04X/%02X/%u ", transaction->port,
                    transaction->value, transaction->kind);
            }
            STD_PRINTF("\n");
            STD_PRINTF("T516:INI-BOOT:%s:FDC-TERMINALS:", name);
            const lib_u32 retained_terminal = trace_probe->fdc_terminal_count <
                BOOT_TRACE_FDC_TERMINALS ? trace_probe->fdc_terminal_count :
                BOOT_TRACE_FDC_TERMINALS;
            const lib_u32 first_terminal = trace_probe->fdc_terminal_count -
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
        STD_PRINTF("T515:INI-BOOT:%s:REGS:EAX=%08X:EBX=%08X:ECX=%08X:EDX=%08X:ESI=%08X:EDI=%08X:EBP=%08X\n",
            name, session->core_machine->executor_cpu.data.eax,
            session->core_machine->executor_cpu.data.ebx,
            session->core_machine->executor_cpu.data.ecx,
            session->core_machine->executor_cpu.data.edx,
            session->core_machine->executor_cpu.data.esi,
            session->core_machine->executor_cpu.data.edi,
            session->core_machine->executor_cpu.data.ebp);
        if (core_machine_memory_read(session->core_machine, cpu.cs_base + cpu.eip, pc_bytes,
                sizeof(pc_bytes)) == TYPE_STATUS_OK) {
            STD_PRINTF("T516:INI-BOOT:%s:PC-BYTES:%02X/%02X/%02X/%02X/%02X/%02X/%02X/%02X\n",
                name, pc_bytes[0u], pc_bytes[1u], pc_bytes[2u], pc_bytes[3u],
                pc_bytes[4u], pc_bytes[5u], pc_bytes[6u], pc_bytes[7u]);
        }
        if (core_machine_get_cpu_diagnostic(session->core_machine, &diagnostic) ==
                TYPE_STATUS_OK && diagnostic.recent_count != 0u) {
            const core_machine_cpu_execution_point *point =
                &diagnostic.recent[diagnostic.recent_count - 1u];
            STD_PRINTF("T515:INI-BOOT:%s:RECENT:CS=%04X:EIP=%08X:bytes=%02X/%02X/%02X\n",
                name, point->cs, point->eip, point->bytes[0u], point->bytes[1u],
                point->bytes[2u]);
        }
    }
    if (test_vm_machine_capture_presentation(session, &frame) != TYPE_STATUS_OK ||
        frame.kind != CORE_MACHINE_GUEST_DISPLAY_KIND_TEXT) return;
    for (row = 0u; row < 25u; ++row) {
        C_INT nonblank = 0;
        for (index = 0u; index < 80u; ++index) {
            C_UCHAR character = (C_UCHAR)frame.characters[row * 80u + index];
            line[index] = character >= 0x20u && character < 0x7fu ? (C_CHAR)character : ' ';
            nonblank |= line[index] != ' ';
        }
        line[80u] = '\0';
        if (nonblank) STD_PRINTF("T515:INI-BOOT:%s:SCREEN:%u:%s\n", name,
            (unsigned int)row, line);
    }
}

static C_INT boot_timeout_parse(const C_CHAR *text, DWORD *out_timeout)
{
    lib_u64 value = 0u;

    if (text == LIB_NULL || out_timeout == LIB_NULL || *text == '\0') return 0;
    while (*text != '\0') {
        if (*text < '0' || *text > '9' || value > 429496729u) return 0;
        value = value * 10u + (lib_u64)(*text - '0');
        if (value > 4294967295u) return 0;
        ++text;
    }
    *out_timeout = (DWORD)value;
    return value != 0u;
}

static C_INT boot_cmos_seed_matches(const vm_machine *session)
{
    t_port *port;
    lib_u8 index;

    if (session == LIB_NULL || !session->cmos_seed_present) return 1;
    port = &session->core_machine->executor_port;
    for (index = 0x0eu; index < VM_MACHINE_CMOS_SEED_BYTES; ++index) {
        lib_u8 expected = session->cmos_seed[index];
        lib_u8 actual;

        core_machine_port_write(port, 0x0070u, index);
        actual = (lib_u8)core_machine_port_read(port, 0x0071u);
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
    integration_ini_session ini_session;
    vm_machine *session;
    const C_CHAR *terminal = LIB_NULL;
    DWORD timeout = BOOT_TIMEOUT;
    ULONGLONG started;
    boot_trace_probe trace_probe = {0};
    C_INT trace_enabled = 0;
    C_INT standard_speed = 0;
    C_INT keyboard_post_failure_seen = 0;
    C_INT result = 1;

    if ((argc < 3 || argc > 6) ||
        (argc >= 4 && !boot_timeout_parse(argv[3], &timeout)) ||
        (argc >= 5 && lib_c_strcmp(argv[4], "trace") != 0 &&
            lib_c_strcmp(argv[4], "standard") != 0) ||
        (argc == 6 && (lib_c_strcmp(argv[4], "trace") != 0 ||
            lib_c_strcmp(argv[5], "standard") != 0))) {
        return 1;
    }
    trace_enabled = argc >= 5 && !lib_c_strcmp(argv[4], "trace");
    standard_speed = (argc == 5 && !lib_c_strcmp(argv[4], "standard")) || argc == 6;
    if (integration_ini_session_open(argv[1], argv[2], &ini_session) ==
        TYPE_STATUS_UNSUPPORTED) {
        STD_PRINTF("T515:INI-BOOT:%s:UNAVAILABLE\n", argv[2]);
        return ASSET_UNAVAILABLE;
    }
    if (ini_session.session == LIB_NULL) {
        STD_FPRINTF(STD_STDERR, "T515:INI-BOOT:%s:SESSION-OPEN-FAILED\n", argv[2]);
        return 1;
    }
    session = ini_session.session;
    if (trace_enabled) {
        trace_probe.fdc = &session->core_machine->fdc;
        (C_VOID)core_machine_set_trace_provider(session->core_machine,
            &(core_machine_trace_provider) {boot_trace_observe, &trace_probe});
        session->core_machine->fdc.connect.observation_provider =
            (core_machine_fdc_terminal_observation_provider) {boot_trace_fdc_terminal,
                &trace_probe};
    }
    if (!boot_cmos_seed_matches(session)) {
        STD_FPRINTF(STD_STDERR, "T515:INI-BOOT:%s:CMOS-SEED-MISMATCH\n", argv[2]);
        goto done;
    }
    if (vm_machine_set_speed(session, standard_speed ? VM_MACHINE_SPEED_STANDARD :
            VM_MACHINE_SPEED_TURBO) != TYPE_STATUS_OK ||
        integration_ini_session_start(&ini_session) != TYPE_STATUS_OK) goto done;
    started = GetTickCount64();
    while (GetTickCount64() - started < timeout) {
        if (boot_post_reports_keyboard_failure(session)) keyboard_post_failure_seen = 1;
        if (boot_terminal(session, &terminal)) break;
        Sleep(BOOT_POLL);
    }
    if (terminal == LIB_NULL || keyboard_post_failure_seen) {
        if (integration_ini_session_pause(&ini_session, 2000u) == TYPE_STATUS_OK) {
            boot_timeout_report(session, argv[2], trace_enabled ? &trace_probe : LIB_NULL);
        }
    }
    if (terminal == LIB_NULL || keyboard_post_failure_seen) {
        if (keyboard_post_failure_seen) {
            STD_PRINTF("T515:INI-BOOT:%s:KEYBOARD-POST-FAILURE\n", argv[2]);
        }
        if (terminal == LIB_NULL) {
            STD_PRINTF("T515:INI-BOOT:%s:TERMINAL-TIMEOUT\n", argv[2]);
        }
        goto done;
    }
    STD_PRINTF("T515:INI-BOOT:%s:%s\n", argv[2], terminal);
    result = 0;
done:
    integration_ini_session_close(&ini_session);
    return result;
}
