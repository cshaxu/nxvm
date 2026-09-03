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
    return 0;
}

static C_VOID boot_timeout_report(const vm_session *session, const C_CHAR *name)
{
    core_platform_display_frame frame;
    core_machine_cpu_state cpu;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_observation observation;
    core_machine_time_observation time_observation;
    C_CHAR line[81];
    type_unsigned_8 equipment[2] = {0};
    type_unsigned_8 option_signature[2] = {0};
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
        STD_PRINTF("T515:YAML-BOOT:%s:FDC:phase=%u:cmd=%02X:index=%u:CHRN=%u/%u/%u:EOT=%u:CCR=%02X:result=%02X/%02X/%02X:remaining=%u:irq=%u\n",
            name, session->core_machine->fdc.data.phase,
            session->core_machine->fdc.data.cmd[0u],
            session->core_machine->fdc.data.command_index,
            session->core_machine->fdc.data.cylinder, session->core_machine->fdc.data.head,
            session->core_machine->fdc.data.sector, session->core_machine->fdc.data.eot,
            session->core_machine->fdc.data.ccr,
            session->core_machine->fdc.data.st0, session->core_machine->fdc.data.st1,
            session->core_machine->fdc.data.st2,
            session->core_machine->fdc.data.transfer_remaining,
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
        STD_PRINTF("T515:YAML-BOOT:%s:PIT1:output=%u:active=%u:reload=%u:remaining=%u:clock=%u/%u\n",
            name, session->core_machine->shared_pit.data.flagOutput[1u],
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
        if (core_machine_memory_read(session->core_machine, 0x000c0000u,
                option_signature, sizeof(option_signature)) == TYPE_STATUS_OK) {
            STD_PRINTF("T515:YAML-BOOT:%s:C0000=%02X%02X\n", name,
                option_signature[0u], option_signature[1u]);
        }
        STD_PRINTF("T515:YAML-BOOT:%s:KBC:output=%02X:pending=%u:input-full=%u\n",
            name, session->core_machine->shared_kbc.data.output_port,
            session->core_machine->shared_kbc.data.pending_write,
            session->core_machine->shared_kbc.data.input_buffer_full);
        STD_PRINTF("T515:YAML-BOOT:%s:REGS:EAX=%08X:EBX=%08X:ECX=%08X:EDX=%08X\n",
            name, session->core_machine->executor_cpu.data.eax,
            session->core_machine->executor_cpu.data.ebx,
            session->core_machine->executor_cpu.data.ecx,
            session->core_machine->executor_cpu.data.edx);
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
    C_INT result = 1;

    if ((argc != 3 && argc != 4) ||
        (argc == 4 && !boot_timeout_parse(argv[3], &timeout))) {
        return 1;
    }
    if (integration_yaml_session_open(argv[1], argv[2], &yaml_session) ==
        TYPE_STATUS_UNSUPPORTED) {
        STD_PRINTF("T515:YAML-BOOT:%s:UNAVAILABLE\n", argv[2]);
        return ASSET_UNAVAILABLE;
    }
    if (yaml_session.session == STD_NULL) return 1;
    session = yaml_session.session;
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
            boot_timeout_report(session, argv[2]);
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
