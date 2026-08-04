#include "type.h"

#include <windows.h>







#include "core/machine/debug_interface.h"
#include "core/machine/machine_interface.h"

#include "vm/composition/session/lifecycle.h"

#include "vm/composition/session/control.h"

#include "vm/composition/session/session.h"

#include "vm/machine/fdd.h"

#define TEXT_VIDEO_BASE 0x000b8000u
#define TEXT_VIDEO_CELLS (80u * 25u)
#define DOS_PROMPT_TIMEOUT_MILLISECONDS 3000u

static C_INT has_dos_prompt(const core_machine *machine);

static DWORD WINAPI run_full_pc(C_VOID *opaque)
{
    vm_session_start((vm_session *)opaque);
    return 0u;
}

static C_VOID dump_first_fault(core_machine *machine)
{
    core_machine_cpu_diagnostic diagnostic;
    const core_machine_cpu_fault_snapshot *fault;
    STD_SIZE_T index;

    if (core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
        TYPE_STATUS_OK || !diagnostic.first_fault.valid) return;
    fault = &diagnostic.first_fault;
    STD_FPRINTF(STD_STDERR,
        "M5:T155:S1:BOOT-FAULT CS:IP=%04X:%08X BYTES=",
        fault->point.cs, fault->point.eip);
    for (index = 0u; index < fault->point.byte_count; ++index) {
        STD_FPRINTF(STD_STDERR, "%02X", fault->point.bytes[index]);
    }
    STD_FPUTC('\n', STD_STDERR);
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    HANDLE thread;
    DWORD result;
    DWORD elapsed;
    C_INT prompt_seen = 0;
    vm_session *session;
    C_INT owns_session = 0;

    if (argc != 2 && argc != 3) return 1;
    if (argc == 3) {
        const vm_session_config config = {
            .fdd_image = argv[1],
            .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
            .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
        };

        if (STD_STRCMP(argv[2], "8086") ||
            vm_session_create(&config, &session) != TYPE_STATUS_OK) return 1;
        owns_session = 1;
    } else {
        session = (vm_session *)STD_CALLOC(1u, sizeof(*session));
        if (session == STD_NULL) return 1;
        vm_session_initialize(session);
        if (vm_machine_fdd_insert_for(&session->fdd, argv[1]) != 0) goto fail;
    }
    thread = CreateThread(STD_NULL, 0u, run_full_pc, session, 0u, STD_NULL);
    if (thread == STD_NULL) goto fail;

    for (elapsed = 0u; elapsed < DOS_PROMPT_TIMEOUT_MILLISECONDS; elapsed += 10u) {
        Sleep(10u);
    }
    vm_session_control_request_pause(&session->control, VM_SESSION_PAUSE_EXPLICIT);
    if (!vm_session_control_wait_for_pause(&session->control, 2000u)) goto fail;
    prompt_seen = has_dos_prompt(session->core_machine);
    if (!prompt_seen) {
        dump_first_fault(session->core_machine);
        STD_FPUTS("M5:T70:S2:DOS-PROMPT:TIMEOUT\n", STD_STDERR);
        goto fail;
    }
    vm_session_stop(session);
    result = WaitForSingleObject(thread, 2000u);
    CloseHandle(thread);
    if (result != WAIT_OBJECT_0) {
        STD_FPUTS("M5:T70:S2:DOS-PROMPT:TIMEOUT\n", STD_STDERR);
        goto fail;
    }
    if (owns_session) vm_session_destroy(session);
    else {
        vm_session_finalize(session);
        STD_FREE(session);
    }
    puts(argc == 3 ? "M5:T209:S3:DOS-PROMPT-8086:OK" :
        "M5:T70:S2:DOS-PROMPT:OK");
    return 0;

fail:
    if (session != STD_NULL) dump_first_fault(session->core_machine);
    vm_session_stop(session);
    if (owns_session) vm_session_destroy(session);
    else {
        vm_session_finalize(session);
        STD_FREE(session);
    }
    return 1;
}
static C_INT has_dos_prompt(const core_machine *machine)
{
    C_UCHAR text[TEXT_VIDEO_CELLS * 2u];
    STD_SIZE_T cell;

    if (core_machine_debug_read_memory(machine, TEXT_VIDEO_BASE, text,
            sizeof(text)) != TYPE_STATUS_OK) return 0;
    for (cell = 0u; cell + 3u < TEXT_VIDEO_CELLS; ++cell) {
        const C_UCHAR drive = text[cell * 2u];
        if (STD_ISALPHA((C_UCHAR)drive) && text[(cell + 1u) * 2u] == ':' &&
            text[(cell + 2u) * 2u] == '\\' && text[(cell + 3u) * 2u] == '>') {
            return 1;
        }
    }
    return 0;
}
