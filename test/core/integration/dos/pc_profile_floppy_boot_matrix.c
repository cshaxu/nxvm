#include "core/machine/machine_interface.h"
#include "../../../core/machine/support/media.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "core/board-base/machine_board_interface.h"
#include <ctype.h>
#include <stdio.h>

#include <windows.h>
#undef exception_code

#include "test/core/machine/support/nxvm_presentation_capture.h"
#include "core/x86/machine_interface.h"
#include "core/x86/debug_interface.h"
#include "core/machine/lifecycle.h"
#include "core/machine/machine_private.h"
#include "core/board-base/floppy_interface.h"
#include "test/core/setup/session_ini.h"

#define BOOT_TIMEOUT 180000u
#define BOOT_POLL 10u
#define TEXT_CELLS (80u * 25u)
#define ASSET_UNAVAILABLE 77
#define BOOT_TRACE_KBC_TRANSACTIONS 32u
#define BOOT_TRACE_FDC_TRANSACTIONS 64u
#define BOOT_TRACE_POST_CODES 64u

typedef struct boot_trace_kbc_transaction {
    lib_u16 port;
    lib_u8 value;
    lib_u8 kind;
} boot_trace_kbc_transaction;

typedef struct boot_retirement_sample {
    lib_u64 tick;
    lib_u64 source;
    lib_u32 eip;
    lib_u32 eax;
    lib_u16 port;
    lib_u32 value;
} boot_retirement_sample;

typedef struct boot_trace_probe {
    lib_u64 cpu_retires;
    lib_u64 external_cycle_commits;
    lib_u64 port61_reads;
    lib_u64 port61_refresh_low_reads;
    lib_u64 port61_refresh_high_reads;
    lib_u32 retirement_sample_count;
    boot_retirement_sample retirement_samples[32];
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
    lib_u32 kbc_data_read_values[256];
    lib_u32 interrupt_acknowledges;
    lib_u32 interrupt_vectors[256];
    lib_u32 post_interrupt_flag_writes;
    lib_u8 last_post_interrupt_flag;
    lib_u32 post_code_count;
    lib_u8 post_codes[BOOT_TRACE_POST_CODES];
} boot_trace_probe;

static void boot_retirement_observe(void *opaque,
    const core_machine_retirement_observation *observation)
{
    boot_trace_probe *probe = opaque;

    if (observation->instruction_entry_cpu.cs.selector != 0xf000u ||
        observation->instruction_entry_cpu.eip < 0xd120u ||
        observation->instruction_entry_cpu.eip >= 0xd140u) return;
    probe->retirement_samples[probe->retirement_sample_count++ % 32u] =
        (boot_retirement_sample) {observation->elapsed_ticks, observation->source_ticks,
            observation->instruction_entry_cpu.eip, observation->current_cpu.eax,
            observation->io_port, observation->io_value};
}

static void boot_trace_observe(void *opaque, const core_machine_trace_event *event)
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

static lib_i32 boot_text_has(const core_machine_guest_display_frame *frame, const char *text)
{
    lib_size cell;
    const lib_size length = text == LIB_NULL ? 0u : lib_text_length(text);

    if (frame == LIB_NULL || length == 0u || length > TEXT_CELLS) return 0;
    for (cell = 0u; cell + length <= TEXT_CELLS; ++cell) {
        if (lib_memory_compare(&frame->characters[cell], text, length) == 0) return 1;
    }
    return 0;
}

static lib_i32 boot_terminal(const vm_machine *session, const char **out_name)
{
    core_machine_guest_display_frame frame;
    lib_size cell;

    if (session == LIB_NULL || out_name == LIB_NULL ||
        test_nxvm_machine_capture_presentation(session, &frame) !=
            LIB_STATUS_OK || frame.kind != CORE_MACHINE_GUEST_DISPLAY_KIND_TEXT) return 0;
    for (cell = 0u; cell + 3u < TEXT_CELLS; ++cell) {
        if (isalpha((lib_u8)frame.characters[cell]) && frame.characters[cell + 1u] == ':' &&
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

static lib_i32 boot_post_reports_keyboard_failure(const vm_machine *session)
{
    core_machine_guest_display_frame frame;

    return session != LIB_NULL &&
        test_nxvm_machine_capture_presentation(session, &frame) ==
            LIB_STATUS_OK && frame.kind == CORE_MACHINE_GUEST_DISPLAY_KIND_TEXT &&
        (boot_text_has(&frame, "301-Keyboard") || boot_text_has(&frame, "303-Keyboard"));
}

static void boot_timeout_report(const vm_machine *session, const char *name,
    const boot_trace_probe *trace_probe)
{
    core_machine_guest_display_frame frame;
    core_machine_cpu_state cpu;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_observation observation;
    core_machine_time_observation time_observation;
    char line[81];
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
    if (core_machine_get_cpu_state(session->core_machine, &cpu) == LIB_STATUS_OK) {
        printf("NXVM:INI-BOOT:%s:CPU:%04X:%08X:base=%08X:flags=%08X:halted=%u:FDD=%u:%ux%ux%u\n",
            name, cpu.cs, cpu.eip, cpu.cs_base, cpu.eflags, cpu.halted,
            vm_test_fdd_info(session->floppy[0u]).present,
            vm_test_fdd_info(session->floppy[0u]).geometry.cylinders, vm_test_fdd_info(session->floppy[0u]).geometry.heads, vm_test_fdd_info(session->floppy[0u]).geometry.sectors_per_track);
        if (core_machine_capture_observation(session->core_machine, &observation) ==
                LIB_STATUS_OK) {
            printf("NXVM:INI-BOOT:%s:STATE:elapsed=%llu:lifecycle=%u\n", name,
                (unsigned long long)observation.elapsed_ticks, observation.lifecycle);
        }
        if (core_machine_capture_time_observation(session->core_machine,
                &time_observation) == LIB_STATUS_OK) {
            printf("NXVM:INI-BOOT:%s:TIME:deadline=%llu:valid=%u:progress=%u\n",
                name, (unsigned long long)time_observation.next_deadline_tick,
                time_observation.next_deadline_valid,
                time_observation.progress_disposition);
        }
        if (core_machine_memory_read(session->core_machine, 0x00000410u,
                equipment, sizeof(equipment)) == LIB_STATUS_OK) {
            printf("NXVM:INI-BOOT:%s:BDA:equipment=%02X%02X\n", name,
                equipment[1u], equipment[0u]);
        }
        if (core_machine_memory_read(session->core_machine, 0x0000046au,
                &interrupt_flag, 1u) == LIB_STATUS_OK) {
            printf("NXVM:INI-BOOT:%s:POST-INTR-FLAG=%02X\n", name,
                interrupt_flag);
        }
        if (core_machine_memory_read(session->core_machine, 0x00000024u,
                keyboard_vector, sizeof(keyboard_vector)) == LIB_STATUS_OK) {
            printf("NXVM:INI-BOOT:%s:INT09=%02X%02X:%02X%02X\n", name,
                keyboard_vector[1u], keyboard_vector[0u], keyboard_vector[3u],
                keyboard_vector[2u]);
        }
        if (core_machine_memory_read(session->core_machine, 0x000c0000u,
                option_signature, sizeof(option_signature)) == LIB_STATUS_OK) {
            printf("NXVM:INI-BOOT:%s:C0000=%02X%02X\n", name,
                option_signature[0u], option_signature[1u]);
        }
        if (core_machine_memory_read(session->core_machine, 0x00007c00u,
                boot_bytes, sizeof(boot_bytes)) == LIB_STATUS_OK &&
            core_machine_memory_read(session->core_machine, 0x00007dfeu,
                boot_signature, sizeof(boot_signature)) == LIB_STATUS_OK) {
            printf("NXVM:INI-BOOT:%s:BOOT=%02X/%02X/%02X/%02X:sig=%02X%02X\n",
                name, boot_bytes[0u], boot_bytes[1u], boot_bytes[2u], boot_bytes[3u],
                boot_signature[1u], boot_signature[0u]);
        }
        {
            lib_bool a20;
            if (core_machine_observe_a20(session->core_machine, &a20) == LIB_STATUS_OK)
                printf("NXVM:INI-BOOT:%s:A20=%u\n", name, (unsigned int)a20);
        }
        if (trace_probe != LIB_NULL) {
            const lib_u32 retained_samples = trace_probe->retirement_sample_count < 32u ?
                trace_probe->retirement_sample_count : 32u;
            for (lib_u32 sample = trace_probe->retirement_sample_count - retained_samples;
                sample < trace_probe->retirement_sample_count; ++sample) {
                const boot_retirement_sample *value = &trace_probe->retirement_samples[sample % 32u];
                lib_c_printf("BOOT-DIAG:RETIRE:pc=%04X:tick=%llu:source=%llu:eax=%08X:port=%04X:value=%02X\n",
                    value->eip, value->tick, value->source, value->eax,
                    (lib_u32)value->port, value->value);
            }
            printf("NXVM:INI-BOOT:%s:TRACE:retired=%llu:external=%llu:port61=%llu:low=%llu:high=%llu:ports-pit=%u:last=%04X/%02X:kbc=%u:last=%04X/%02X\n",
                name, (unsigned long long)trace_probe->cpu_retires,
                (unsigned long long)trace_probe->external_cycle_commits,
                (unsigned long long)trace_probe->port61_reads,
                (unsigned long long)trace_probe->port61_refresh_low_reads,
                (unsigned long long)trace_probe->port61_refresh_high_reads,
                trace_probe->pit_writes, trace_probe->last_pit_address,
                trace_probe->last_pit_value, trace_probe->kbc_writes,
                trace_probe->last_kbc_address, trace_probe->last_kbc_value);
            printf("NXVM:INI-BOOT:%s:INTA=%u:IRQ1=%u:IRQ6=%u\n", name,
                trace_probe->interrupt_acknowledges, trace_probe->interrupt_vectors[0x09u],
                trace_probe->interrupt_vectors[0x0eu]);
            printf("NXVM:INI-BOOT:%s:POST-INTR-WRITES=%u:last=%02X\n", name,
                trace_probe->post_interrupt_flag_writes,
                trace_probe->last_post_interrupt_flag);
            printf("NXVM:INI-BOOT:%s:POST-CODES:", name);
            const lib_u32 retained_post = trace_probe->post_code_count <
                BOOT_TRACE_POST_CODES ? trace_probe->post_code_count : BOOT_TRACE_POST_CODES;
            const lib_u32 first_post = trace_probe->post_code_count - retained_post;
            for (index = 0u; index < retained_post; ++index) {
                printf("%02X ", trace_probe->post_codes[(first_post + index) %
                    BOOT_TRACE_POST_CODES]);
            }
            printf("\n");
            printf("NXVM:INI-BOOT:%s:KBC-CPU:", name);
            const lib_u32 retained = trace_probe->kbc_transaction_count <
                BOOT_TRACE_KBC_TRANSACTIONS ? trace_probe->kbc_transaction_count :
                BOOT_TRACE_KBC_TRANSACTIONS;
            const lib_u32 first = trace_probe->kbc_transaction_count - retained;
            for (index = 0u; index < retained; ++index) {
                const boot_trace_kbc_transaction *transaction =
                    &trace_probe->kbc_transactions[(first + index) %
                        BOOT_TRACE_KBC_TRANSACTIONS];
                printf("%04X/%02X/%u ", transaction->port,
                    transaction->value, transaction->kind);
            }
            printf("\n");
            printf("NXVM:INI-BOOT:%s:KBC-READS:00=%u:55=%u:65=%u:AA=%u:FA=%u:AB=%u:83=%u\n",
                name, trace_probe->kbc_data_read_values[0x00u],
                trace_probe->kbc_data_read_values[0x55u],
                trace_probe->kbc_data_read_values[0x65u],
                trace_probe->kbc_data_read_values[0xaau],
                trace_probe->kbc_data_read_values[0xfau],
                trace_probe->kbc_data_read_values[0xabu],
                trace_probe->kbc_data_read_values[0x83u]);
            printf("NXVM:INI-BOOT:%s:FDC-CPU:", name);
            const lib_u32 retained_fdc = trace_probe->fdc_transaction_count <
                BOOT_TRACE_FDC_TRANSACTIONS ? trace_probe->fdc_transaction_count :
                BOOT_TRACE_FDC_TRANSACTIONS;
            const lib_u32 first_fdc = trace_probe->fdc_transaction_count -
                retained_fdc;
            for (index = 0u; index < retained_fdc; ++index) {
                const boot_trace_kbc_transaction *transaction =
                    &trace_probe->fdc_transactions[(first_fdc + index) %
                        BOOT_TRACE_FDC_TRANSACTIONS];
                printf("%04X/%02X/%u ", transaction->port,
                    transaction->value, transaction->kind);
            }
            printf("\n");
        }
        {
            const core_machine_debug_register registers[] = {
                CORE_MACHINE_DEBUG_EAX, CORE_MACHINE_DEBUG_EBX,
                CORE_MACHINE_DEBUG_ECX, CORE_MACHINE_DEBUG_EDX,
                CORE_MACHINE_DEBUG_ESI, CORE_MACHINE_DEBUG_EDI,
                CORE_MACHINE_DEBUG_EBP
            };
            lib_u32 values[7];
            lib_size captured;

            for (captured = 0u; captured < sizeof(registers) / sizeof(registers[0]);
                    ++captured) {
                if (core_machine_debug_read_register(session->core_machine,
                        registers[captured], &values[captured]) != LIB_STATUS_OK) break;
            }
            if (captured == sizeof(registers) / sizeof(registers[0])) {
                printf("NXVM:INI-BOOT:%s:REGS:EAX=%08X:EBX=%08X:ECX=%08X:EDX=%08X:ESI=%08X:EDI=%08X:EBP=%08X\n",
                    name, values[0], values[1], values[2], values[3],
                    values[4], values[5], values[6]);
            }
        }
        if (core_machine_memory_read(session->core_machine, cpu.cs_base + cpu.eip, pc_bytes,
                sizeof(pc_bytes)) == LIB_STATUS_OK) {
            printf("NXVM:INI-BOOT:%s:PC-BYTES:%02X/%02X/%02X/%02X/%02X/%02X/%02X/%02X\n",
                name, pc_bytes[0u], pc_bytes[1u], pc_bytes[2u], pc_bytes[3u],
                pc_bytes[4u], pc_bytes[5u], pc_bytes[6u], pc_bytes[7u]);
        }
        if (core_machine_get_cpu_diagnostic(session->core_machine, &diagnostic) ==
                LIB_STATUS_OK && diagnostic.first_fault.valid) {
            lib_c_printf("BOOT-DIAG:%s:FIRST-FAULT:mask=%08X:code=%08X:pc=%04X:%08X\n",
                name, diagnostic.first_fault.exception_mask, diagnostic.first_fault.exception_code,
                diagnostic.first_fault.point.cs, diagnostic.first_fault.point.eip);
        }
        if (core_machine_get_cpu_diagnostic(session->core_machine, &diagnostic) ==
                LIB_STATUS_OK && diagnostic.recent_count != 0u) {
            const core_machine_cpu_execution_point *point =
                &diagnostic.recent[diagnostic.recent_count - 1u];
            printf("NXVM:INI-BOOT:%s:RECENT:CS=%04X:EIP=%08X:bytes=%02X/%02X/%02X\n",
                name, point->cs, point->eip, point->bytes[0u], point->bytes[1u],
                point->bytes[2u]);
        }
    }
    if (test_nxvm_machine_capture_presentation(session, &frame) != LIB_STATUS_OK ||
        frame.kind != CORE_MACHINE_GUEST_DISPLAY_KIND_TEXT) return;
    for (row = 0u; row < 25u; ++row) {
        lib_i32 nonblank = 0;
        for (index = 0u; index < 80u; ++index) {
            lib_u8 character = (lib_u8)frame.characters[row * 80u + index];
            line[index] = character >= 0x20u && character < 0x7fu ? (char)character : ' ';
            nonblank |= line[index] != ' ';
        }
        line[80u] = '\0';
        if (nonblank) printf("NXVM:INI-BOOT:%s:SCREEN:%u:%s\n", name,
            (unsigned int)row, line);
    }
}

static lib_i32 boot_timeout_parse(const char *text, DWORD *out_timeout)
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

static lib_i32 boot_cmos_seed_matches(const vm_machine *session)
{
    lib_u8 index;

    if (session == LIB_NULL || !session->construction.cmos_seed_present) return 1;
    for (index = 0x0eu; index < VM_MACHINE_CMOS_SEED_BYTES; ++index) {
        lib_u8 expected = session->construction.cmos_seed[index];
        lib_u32 actual;

        if (core_machine_bus_write(session->core_machine, 0x0070u, index) != LIB_STATUS_OK ||
            core_machine_bus_read(session->core_machine, 0x0071u, &actual) != LIB_STATUS_OK)
            return 0;
        if (actual != expected) {
            printf("NXVM:CMOS:index=%02X:expected=%02X:actual=%02X\n", index,
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
    const char *terminal = LIB_NULL;
    DWORD timeout = BOOT_TIMEOUT;
    ULONGLONG started;
    boot_trace_probe trace_probe = {0};
    lib_i32 trace_enabled = 0;
    lib_i32 standard_speed = 0;
    lib_i32 keyboard_post_failure_seen = 0;
    lib_i32 result = 1;

    if ((argc < 3 || argc > 6) ||
        (argc >= 4 && !boot_timeout_parse(argv[3], &timeout)) ||
        (argc >= 5 && lib_text_compare(argv[4], "trace") != 0 &&
            lib_text_compare(argv[4], "standard") != 0) ||
        (argc == 6 && (lib_text_compare(argv[4], "trace") != 0 ||
            lib_text_compare(argv[5], "standard") != 0))) {
        return 1;
    }
    trace_enabled = argc >= 5 && !lib_text_compare(argv[4], "trace");
    standard_speed = (argc == 5 && !lib_text_compare(argv[4], "standard")) || argc == 6;
    if (integration_ini_session_open(argv[1], argv[2], &ini_session) ==
        LIB_STATUS_UNSUPPORTED) {
        printf("NXVM:INI-BOOT:%s:UNAVAILABLE\n", argv[2]);
        return ASSET_UNAVAILABLE;
    }
    if (ini_session.session == LIB_NULL) {
        fprintf(stderr, "NXVM:INI-BOOT:%s:SESSION-OPEN-FAILED\n", argv[2]);
        return 1;
    }
    session = ini_session.session;
    if (trace_enabled) {
        if (core_machine_set_retirement_observation_provider(session->core_machine,
                &(core_machine_retirement_observation_provider) {boot_retirement_observe,
                    &trace_probe}) != LIB_STATUS_OK) goto done;
        if (core_machine_set_trace_provider(session->core_machine,
                &(core_machine_trace_provider) {boot_trace_observe, &trace_probe}) !=
                LIB_STATUS_OK) {
            fprintf(stderr, "NXVM:INI-BOOT:%s:TRACE-SETUP-FAILED\n", argv[2]);
            goto done;
        }
    }
    if (!boot_cmos_seed_matches(session)) {
        fprintf(stderr, "NXVM:INI-BOOT:%s:CMOS-SEED-MISMATCH\n", argv[2]);
        goto done;
    }
    if (vm_machine_set_speed(session, standard_speed ? VM_MACHINE_SPEED_STANDARD :
            VM_MACHINE_SPEED_TURBO) != LIB_STATUS_OK ||
        integration_ini_session_start(&ini_session) != LIB_STATUS_OK) goto done;
    started = GetTickCount64();
    while (GetTickCount64() - started < timeout) {
        if (boot_post_reports_keyboard_failure(session)) keyboard_post_failure_seen = 1;
        if (boot_terminal(session, &terminal)) break;
        Sleep(BOOT_POLL);
    }
    if (terminal == LIB_NULL || keyboard_post_failure_seen) {
        if (integration_ini_session_pause(&ini_session, 2000u) == LIB_STATUS_OK) {
            boot_timeout_report(session, argv[2], trace_enabled ? &trace_probe : LIB_NULL);
        }
    }
    if (terminal == LIB_NULL || keyboard_post_failure_seen) {
        if (keyboard_post_failure_seen) {
            printf("NXVM:INI-BOOT:%s:KEYBOARD-POST-FAILURE\n", argv[2]);
        }
        if (terminal == LIB_NULL) {
            printf("NXVM:INI-BOOT:%s:TERMINAL-TIMEOUT\n", argv[2]);
        }
        goto done;
    }
    printf("NXVM:INI-BOOT:%s:%s\n", argv[2], terminal);
    result = 0;
done:
    integration_ini_session_close(&ini_session);
    return result;
}
