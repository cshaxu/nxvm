#include "type.h"

#include <windows.h>

#include "core/machine/cpu_instructions.h"
#include "core/machine/machine_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session.h"
#include "vm/machine/fdd.h"
#include "vm/platform/win32/win32.h"

#define TEXT_VIDEO_BASE 0x000b8000u
#define TEXT_VIDEO_CELLS (80u * 25u)
#define DOS_PROMPT_TIMEOUT_MILLISECONDS 3000u
#define MEM_FAULT_TIMEOUT_MILLISECONDS 2000u

static DWORD WINAPI vm_dos_mem_fault_run_machine(C_VOID *opaque)
{
    vm_session_control_start(((vm_session *)opaque)->control);
    return 0u;
}

static C_INT vm_dos_mem_fault_has_prompt(const vm_session *session)
{
    const C_UCHAR *screen = (const C_UCHAR *)
        core_machine_executor_memory_borrow(session->core_machine)->connect.pBase +
        TEXT_VIDEO_BASE;
    STD_SIZE_T cell;

    for (cell = 0u; cell + 3u < TEXT_VIDEO_CELLS; ++cell) {
        if (STD_ISALPHA(screen[cell * 2u]) &&
            screen[(cell + 1u) * 2u] == ':' &&
            screen[(cell + 2u) * 2u] == '\\' &&
            screen[(cell + 3u) * 2u] == '>') return 1;
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
    vm_session *session = STD_NULL;
    HANDLE thread = STD_NULL;
    DWORD elapsed;
    DWORD result;
    const C_UCHAR scan_codes[] = { 0x32u, 0x12u, 0x32u, 0x1cu };
    const C_UCHAR virtual_keys[] = { 'M', 'E', 'M', VK_RETURN };
    core_machine_cpu_diagnostic diagnostic;
    STD_SIZE_T index;

    if (argc != 2 || vm_session_create(STD_NULL, &session) != NTVDM64_STATUS_OK ||
        vm_machine_fdd_insert_for(session->fdd, argv[1]) != 0) goto fail;
    thread = CreateThread(STD_NULL, 0u, vm_dos_mem_fault_run_machine, session,
        0u, STD_NULL);
    if (thread == STD_NULL) goto fail;
    for (elapsed = 0u; elapsed < DOS_PROMPT_TIMEOUT_MILLISECONDS; elapsed += 10u) {
        if (vm_dos_mem_fault_has_prompt(session)) break;
        Sleep(10u);
    }
    if (elapsed == DOS_PROMPT_TIMEOUT_MILLISECONDS) goto fail;
    for (index = 0u; index < sizeof(scan_codes); ++index) {
        vm_platform_win32_keyboard_make_key_for(session->platform_run_context,
            scan_codes[index], virtual_keys[index]);
    }
    result = WaitForSingleObject(thread, MEM_FAULT_TIMEOUT_MILLISECONDS);
    if (result != WAIT_OBJECT_0 ||
        core_machine_get_cpu_diagnostic(session->core_machine, &diagnostic) !=
            NTVDM64_STATUS_OK ||
        !diagnostic.first_fault.valid ||
        !NTVDM64_TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
            VCPUINS_EXCEPT_UD)) goto fail;
    vm_dos_mem_fault_print(&diagnostic);
    CloseHandle(thread);
    vm_session_destroy(session);
    STD_PRINTF("M5:T152:S1:DOS-MEM-UD:OK\n");
    return 0;

fail:
    if (session != STD_NULL) vm_session_stop(session);
    if (thread != STD_NULL) {
        WaitForSingleObject(thread, 2000u);
        CloseHandle(thread);
    }
    vm_session_destroy(session);
    return 1;
}
