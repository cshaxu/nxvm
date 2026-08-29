#include "type.h"

#include <windows.h>

#include "core/machine/cpu.h"
#include "core/machine/debug_interface.h"
#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "core/platform/presentation_mailbox_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/fault.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/platform/win32/win32.h"
#include "../../core/support/core_machine_cpu_fixture.h"

#define VM_T287_TEXT_CELLS (80u * 25u)
#define VM_T287_BOOT_TIMEOUT_MILLISECONDS 60000u
#define VM_T287_SETUP_TIMEOUT_MILLISECONDS 20000u
#define VM_T287_FAULT_OBSERVATION_MILLISECONDS 60000u
#define VM_T288_POST_COPY_TIMEOUT_MILLISECONDS 720000u

static DWORD WINAPI vm_t287_run_machine(C_VOID *opaque)
{
    vm_session_control_start(&((vm_session *)opaque)->control);
    return 0u;
}

static C_INT vm_t287_has_text(const vm_session *session, const C_CHAR *text)
{
    core_platform_display_frame frame;
    STD_SIZE_T cell;
    STD_SIZE_T character;
    STD_SIZE_T length = STD_STRLEN(text);

    if (session == STD_NULL || text == STD_NULL || length == 0u ||
        core_platform_presentation_mailbox_capture(session->presentation_mailbox,
            &frame) != TYPE_STATUS_OK) return 0;
    for (cell = 0u; cell + length <= VM_T287_TEXT_CELLS; ++cell) {
        for (character = 0u; character < length; ++character) {
            if (frame.characters[cell + character] != (C_UCHAR)text[character]) break;
        }
        if (character == length) return 1;
    }
    return 0;
}

static C_INT vm_t287_has_prompt(const vm_session *session)
{
    core_platform_display_frame frame;
    STD_SIZE_T cell;

    if (session == STD_NULL || core_platform_presentation_mailbox_capture(
            session->presentation_mailbox, &frame) != TYPE_STATUS_OK) return 0;
    for (cell = 0u; cell + 1u < VM_T287_TEXT_CELLS; ++cell) {
        if (frame.characters[cell] == 'C' && frame.characters[cell + 1u] == '>') {
            return 1;
        }
        if (cell + 3u < VM_T287_TEXT_CELLS && frame.characters[cell] == 'C' &&
            frame.characters[cell + 1u] == ':' && frame.characters[cell + 2u] == '\\' &&
            frame.characters[cell + 3u] == '>') {
            return 1;
        }
    }
    return 0;
}

static C_INT vm_t287_wait_for(const vm_session *session, const C_CHAR *text,
    DWORD timeout)
{
    DWORD elapsed;

    for (elapsed = 0u; elapsed < timeout; elapsed += 10u) {
        if ((text == STD_NULL && vm_t287_has_prompt(session)) ||
            (text != STD_NULL && vm_t287_has_text(session, text))) return 1;
        if (elapsed >= 500u && !vm_session_control_is_running(&session->control)) {
            return 0;
        }
        Sleep(10u);
    }
    return 0;
}

static C_INT vm_t287_submit(const vm_session *session, const type_unsigned_8 *codes,
    STD_SIZE_T count)
{
    STD_SIZE_T index;

    if (session == STD_NULL || codes == STD_NULL) return 0;
    for (index = 0u; index < count; ++index) {
        if (core_machine_keyboard_receive_native_byte(session->core_machine,
                codes[index]) != TYPE_STATUS_OK) return 0;
        Sleep(25u);
    }
    return 1;
}

static C_INT vm_t287_type_setup(vm_session *session)
{
    static const type_unsigned_8 scan_codes[] = {
        0x2bu, 0x12u, 0x11u, 0x17u, 0x31u, 0x04u, 0x02u, 0x2bu, 0x1fu,
        0x12u, 0x14u, 0x16u, 0x19u, 0x34u, 0x12u, 0x2du, 0x12u, 0x1cu
    };
    static const type_unsigned_8 virtual_keys[] = {
        VK_OEM_5, 'E', 'W', 'I', 'N', '3', '1', VK_OEM_5, 'S', 'E', 'T',
        'U', 'P', VK_OEM_PERIOD, 'E', 'X', 'E', VK_RETURN
    };
    STD_SIZE_T index;

    if (session == STD_NULL) return 0;
    for (index = 0u; index < sizeof(scan_codes); ++index) {
        vm_platform_win32_keyboard_make_key_for(session->platform_run_context,
            session->platform_run_handle, scan_codes[index], virtual_keys[index], 1);
        Sleep(25u);
        vm_platform_win32_keyboard_make_key_for(session->platform_run_context,
            session->platform_run_handle, scan_codes[index], virtual_keys[index], 0);
        Sleep(25u);
    }
    return 1;
}

static C_INT vm_t288_type_windows(vm_session *session)
{
    static const type_unsigned_8 scan_codes[] = {
        0x11u, 0x17u, 0x31u, 0x20u, 0x18u, 0x11u, 0x1fu, 0x1cu
    };
    static const type_unsigned_8 virtual_keys[] = {
        'W', 'I', 'N', 'D', 'O', 'W', 'S', VK_RETURN
    };
    STD_SIZE_T index;

    if (session == STD_NULL) return 0;
    for (index = 0u; index < sizeof(scan_codes); ++index) {
        vm_platform_win32_keyboard_make_key_for(session->platform_run_context,
            session->platform_run_handle, scan_codes[index], virtual_keys[index], 1);
        Sleep(25u);
        vm_platform_win32_keyboard_make_key_for(session->platform_run_context,
            session->platform_run_handle, scan_codes[index], virtual_keys[index], 0);
        Sleep(25u);
    }
    return 1;
}

static C_VOID vm_t287_print_frame(const vm_session *session)
{
    core_platform_display_frame frame;
    STD_SIZE_T row;
    STD_SIZE_T column;

    if (session == STD_NULL || core_platform_presentation_mailbox_capture(
            session->presentation_mailbox, &frame) != TYPE_STATUS_OK) return;
    for (row = 0u; row < 25u; ++row) {
        for (column = 0u; column < 80u; ++column) {
            C_UCHAR character = frame.characters[row * 80u + column];

            STD_PRINTF("%c", character == 0u ? ' ' : character);
        }
        STD_PRINTF("\n");
    }
}

static C_VOID vm_t287_report_fault(vm_session *session, const C_CHAR *stage)
{
    core_machine_cpu_diagnostic diagnostic = {0};
    t_cpu cpu;
    STD_SIZE_T index;

    if (session == STD_NULL) return;
    (C_VOID)core_machine_get_cpu_diagnostic(session->core_machine, &diagnostic);
    STD_PRINTF("M5:T287:S23:WINDOWS31:SETUP:CHECKPOINT stage=%s running=%d "
        "ata_commands=%u last_command=%02X\n", stage,
        vm_session_control_is_running(&session->control),
        session->core_machine->hdc.data.command_count,
        session->core_machine->hdc.data.last_command);
    if (diagnostic.first_fault.valid) {
        const core_machine_cpu_fault_snapshot *fault = &diagnostic.first_fault;

        STD_PRINTF("M5:T288:S1:WINDOWS31:FAULT mask=%08X code=%08X "
            "cs=%04X ip=%08X linear=%08X opcode=", fault->exception_mask,
            fault->exception_code, fault->point.cs, fault->point.eip,
            fault->point.linear_pc);
        for (index = 0u; index < fault->point.byte_count; ++index) {
            STD_PRINTF("%02X", fault->point.bytes[index]);
        }
        STD_PRINTF(" eax=%08X ebx=%08X ecx=%08X edx=%08X esp=%08X ebp=%08X "
            "esi=%08X edi=%08X flags=%08X\n",
            fault->eax, fault->ebx, fault->ecx, fault->edx, fault->esp,
            fault->ebp, fault->esi, fault->edi, fault->eflags);
        for (index = 0u; index < diagnostic.recent_count; ++index) {
            const core_machine_cpu_execution_point *point =
                &diagnostic.recent[index];

            STD_PRINTF("M5:T288:S1:WINDOWS31:RECENT cs=%04X ip=%08X "
                "linear=%08X opcode=%02X%02X%02X\n", point->cs, point->eip,
                point->linear_pc, point->bytes[0], point->bytes[1],
                point->bytes[2]);
        }
    }
    cpu = test_core_machine_fixture_capture_cpu_after_run(session->core_machine);
    if (session->core_machine != STD_NULL) {
        STD_PRINTF("M5:T287:S23:WINDOWS31:CPU cr0=%08X cr2=%08X cr3=%08X "
            "gdtr=%08X/%04X idtr=%08X/%04X cs=%04X:%08X/%08X ds=%04X:%08X/%08X "
            "ss=%04X:%08X/%08X\n", cpu.data.cr0, cpu.data.cr2, cpu.data.cr3,
            cpu.data.gdtr.base, cpu.data.gdtr.limit, cpu.data.idtr.base,
            cpu.data.idtr.limit, cpu.data.cs.selector, cpu.data.cs.base,
            cpu.data.cs.limit, cpu.data.ds.selector, cpu.data.ds.base,
            cpu.data.ds.limit, cpu.data.ss.selector, cpu.data.ss.base,
            cpu.data.ss.limit);
    }
    vm_t287_print_frame(session);
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    const vm_session_config config = {
        .hdd_image = argc >= 2 ? argv[1] : STD_NULL,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    const type_unsigned_8 enter[] = {0x5au};
    HANDLE thread = STD_NULL;
    vm_session *session = STD_NULL;
    const C_CHAR *stage = "create";
    C_INT observed_setup_inf = 0;
    C_INT passed = 0;
    C_INT advance_steps = argc == 3 ? argv[2][0] - '0' : 0;
    C_INT date_prompt = 0;
    DWORD elapsed;

    if ((argc != 2 && argc != 3) || vm_session_create(&config, &session) != TYPE_STATUS_OK ||
        session == STD_NULL) goto fail;
    thread = CreateThread(STD_NULL, 0u, vm_t287_run_machine, session, 0u, STD_NULL);
    if (thread == STD_NULL) goto fail;
    stage = "boot";
    for (elapsed = 0u; elapsed < VM_T287_BOOT_TIMEOUT_MILLISECONDS; elapsed += 10u) {
        date_prompt = vm_t287_has_text(session, "Enter new date");
        if (date_prompt || vm_t287_has_prompt(session)) break;
        if (elapsed >= 500u && !vm_session_control_is_running(&session->control)) {
            goto fail;
        }
        Sleep(10u);
    }
    if (elapsed == VM_T287_BOOT_TIMEOUT_MILLISECONDS) goto fail;
    if (date_prompt) {
        stage = "date";
        if (!vm_t287_submit(session, enter, sizeof(enter))) goto fail;
        stage = "time";
        if (!vm_t287_wait_for(session, "Enter new time", VM_T287_BOOT_TIMEOUT_MILLISECONDS) ||
            !vm_t287_submit(session, enter, sizeof(enter))) goto fail;
    }
    stage = "prompt";
    if (!vm_t287_wait_for(session, STD_NULL, VM_T287_BOOT_TIMEOUT_MILLISECONDS)) goto fail;
    stage = "setup-command";
    if (!vm_t287_type_setup(session)) goto fail;
    stage = "setup-result";
    observed_setup_inf = vm_t287_wait_for(session, "Reading SETUP.INF...",
        VM_T287_SETUP_TIMEOUT_MILLISECONDS);
    if (observed_setup_inf) {
        Sleep(VM_T287_FAULT_OBSERVATION_MILLISECONDS);
        if (!vm_session_control_is_running(&session->control)) {
            vm_t287_report_fault(session, "setup-inf-fault");
            goto done;
        }
        if (vm_t287_has_text(session, "Welcome to Setup.")) {
            stage = "welcome";
            if (advance_steps != 0) {
                if (!vm_t287_submit(session, enter, sizeof(enter))) goto fail;
                Sleep(3000u);
                if (advance_steps >= 2) {
                    if (!vm_t287_submit(session, enter, sizeof(enter))) goto fail;
                    Sleep(3000u);
                }
                if (advance_steps >= 3) {
                    if (!vm_t288_type_windows(session)) goto fail;
                    if (advance_steps == 3) Sleep(30000u);
                    if (advance_steps >= 4) {
                        stage = "post-copy";
                        for (elapsed = 0u;
                                elapsed < VM_T288_POST_COPY_TIMEOUT_MILLISECONDS;
                                elapsed += 100u) {
                            if (!vm_session_control_is_running(&session->control)) break;
                            Sleep(100u);
                        }
                    }
                }
                vm_t287_report_fault(session, "after-welcome-enter");
                goto done;
            }
            passed = 1;
            STD_PRINTF("M5:T287:S24:WINDOWS31:SETUP:OK result=welcome\n");
            vm_t287_print_frame(session);
            goto done;
        }
        vm_t287_report_fault(session, "setup-inf-running");
    } else {
        vm_t287_report_fault(session, stage);
    }

done:
    if (session != STD_NULL) vm_session_stop(session);
    if (thread != STD_NULL) {
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
    }
    vm_session_destroy(session);
    return passed ? 0 : 1;

fail:
    vm_t287_report_fault(session, stage);
    goto done;
}
