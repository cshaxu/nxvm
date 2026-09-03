#include "type.h"

#include <windows.h>

#include "core/machine/cpu_instructions.h"
#include "core/machine/machine_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_private.h"
#include "vm/platform/win32/win32.h"
#include "test/integration/support/session_yaml.h"

#define TEXT_VIDEO_BASE 0x000b8000u
#define TEXT_VIDEO_CELLS (80u * 25u)
#define DOS_PROMPT_TIMEOUT_MILLISECONDS 20000u
#define MEM_FAULT_TIMEOUT_MILLISECONDS 2000u

static DWORD WINAPI vm_dos_mem_fault_run_machine(C_VOID *opaque)
{
    vm_session_control_start(&((vm_session *)opaque)->control);
    return 0u;
}

static C_INT vm_dos_mem_fault_has_prompt(const vm_session *session)
{
    core_platform_display_frame frame;
    STD_SIZE_T cell;

    (C_VOID)core_platform_presentation_mailbox_capture(
        session->presentation_mailbox, &frame);
    for (cell = 0u; cell + 3u < TEXT_VIDEO_CELLS; ++cell) {
        if (STD_ISALPHA(frame.characters[cell]) &&
            frame.characters[cell + 1u] == ':' &&
            frame.characters[cell + 2u] == '\\' &&
            frame.characters[cell + 3u] == '>') return 1;
    }
    return 0;
}

static C_VOID vm_dos_mem_fault_print(const core_machine_cpu_diagnostic *diagnostic)
{
    const core_machine_cpu_fault_snapshot *fault = &diagnostic->first_fault;
    STD_SIZE_T index;

    STD_PRINTF("M5:T152:S1:FAULT CS:IP=%04X:%08X L%08X EX=%08X CODE=%08X BYTES=",
        fault->point.cs, fault->point.eip, fault->point.linear_pc,
        fault->exception_mask, fault->exception_code);
    for (index = 0u; index < fault->point.byte_count; ++index) {
        STD_PRINTF("%02X", fault->point.bytes[index]);
    }
    STD_PRINTF(" EAX=%08X EBX=%08X ECX=%08X EDX=%08X ESP=%08X EBP=%08X ESI=%08X EDI=%08X FLAGS=%08X WINDOW=%u\n",
        fault->eax, fault->ebx, fault->ecx, fault->edx, fault->esp, fault->ebp,
        fault->esi, fault->edi, fault->eflags, (C_UINT)diagnostic->recent_count);
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    integration_yaml_session yaml_session;
    vm_session *session = STD_NULL;
    HANDLE thread = STD_NULL;
    DWORD elapsed;
    DWORD result;
    const C_UCHAR scan_codes[] = { 0x32u, 0x12u, 0x32u, 0x1cu };
    const C_UCHAR virtual_keys[] = { 'M', 'E', 'M', VK_RETURN };
    core_machine_cpu_diagnostic diagnostic;
    const C_CHAR *stage = "argument validation";
    STD_SIZE_T index;

    if (argc != 3) goto fail;
    stage = "session creation";
    if (integration_yaml_session_open(argv[1], argv[2], &yaml_session) != TYPE_STATUS_OK) {
        return 77;
    }
    session = yaml_session.session;
    stage = "machine thread creation";
    thread = CreateThread(STD_NULL, 0u, vm_dos_mem_fault_run_machine, session,
        0u, STD_NULL);
    if (thread == STD_NULL) goto fail;
    stage = "DOS prompt";
    for (elapsed = 0u; elapsed < DOS_PROMPT_TIMEOUT_MILLISECONDS; elapsed += 10u) {
        if (vm_dos_mem_fault_has_prompt(session)) break;
        Sleep(10u);
    }
    if (elapsed == DOS_PROMPT_TIMEOUT_MILLISECONDS) goto fail;
    stage = "MEM command completion";
    for (index = 0u; index < sizeof(scan_codes); ++index) {
        vm_platform_win32_keyboard_make_key_for(session->platform_run_context,
            session->platform_run_handle, scan_codes[index], virtual_keys[index], 1);
    }
    result = WaitForSingleObject(thread, MEM_FAULT_TIMEOUT_MILLISECONDS);
    if (result != WAIT_OBJECT_0 && result != WAIT_TIMEOUT) goto fail;
    stage = "FNINIT fault classification";
    if (result == WAIT_OBJECT_0) {
        if (core_machine_get_cpu_diagnostic(session->core_machine, &diagnostic) !=
            TYPE_STATUS_OK) goto fail;
        if (diagnostic.first_fault.valid) {
            vm_dos_mem_fault_print(&diagnostic);
            if (TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_UD) &&
                diagnostic.first_fault.point.byte_count >= 2u &&
                diagnostic.first_fault.point.bytes[0] == 0xdbu &&
                diagnostic.first_fault.point.bytes[1] == 0xe3u) goto fail;
        } else {
            STD_PRINTF("M5:T156:S1:DOS-MEM-NEXT:STOPPED\n");
        }
    } else {
        STD_PRINTF("M5:T156:S1:DOS-MEM-NEXT:RUNNING\n");
    }
    vm_session_stop(session);
    if (WaitForSingleObject(thread, 2000u) != WAIT_OBJECT_0) goto fail;
    CloseHandle(thread);
    integration_yaml_session_close(&yaml_session);
    STD_PRINTF("M5:T156:S1:DOS-MEM-FNINIT-PASSED:OK\n");
    return 0;

fail:
    STD_FPRINTF(STD_STDERR, "M5:T198:S1:DOS-MEM:FAIL stage=%s\n", stage);
    if (session != STD_NULL &&
        core_machine_get_cpu_diagnostic(session->core_machine, &diagnostic) ==
            TYPE_STATUS_OK && diagnostic.first_fault.valid) {
        vm_dos_mem_fault_print(&diagnostic);
    }
    if (session != STD_NULL) vm_session_stop(session);
    if (thread != STD_NULL) {
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
    }
    integration_yaml_session_close(&yaml_session);
    return 1;
}
