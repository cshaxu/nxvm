#include "type.h"

#include <windows.h>







#include "core/machine/debug_interface.h"
#include "core/machine/machine_interface.h"

#include "vm/composition/session/lifecycle.h"

#include "vm/composition/session/control.h"

#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"

#include "vm/machine/fdd.h"

#define TEXT_VIDEO_BASE 0x000b8000u
#define TEXT_VIDEO_CELLS (80u * 25u)
#define DOS_PROMPT_TIMEOUT_MILLISECONDS 5000u

static C_INT has_dos_prompt(const vm_session *session);

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
    vm_session *session = STD_NULL;
    C_INT owns_session = 0;
    C_INT turbo = 0;

    if (argc != 2 && argc != 3) return 1;
    if (argc == 3 && !STD_STRCMP(argv[2], "8086")) {
        const vm_session_config config = {
            .fdd_image = argv[1],
            .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
            .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
        };

        if (vm_session_create(&config, &session) != TYPE_STATUS_OK) return 1;
        owns_session = 1;
    } else if ((argc == 2 || !STD_STRCMP(argv[2], "turbo")) &&
        vm_session_create(STD_NULL, &session) == TYPE_STATUS_OK) {
        turbo = argc == 3;
        owns_session = 1;
        if (vm_machine_fdd_insert_for(&session->fdd, argv[1]) != 0 ||
            (turbo && vm_session_set_speed(session, VM_SESSION_SPEED_TURBO) !=
                TYPE_STATUS_OK)) goto fail;
    } else {
        return 1;
    }
    thread = CreateThread(STD_NULL, 0u, run_full_pc, session, 0u, STD_NULL);
    if (thread == STD_NULL) goto fail;

    for (elapsed = 0u; elapsed < DOS_PROMPT_TIMEOUT_MILLISECONDS; elapsed += 10u) {
        if (has_dos_prompt(session)) {
            prompt_seen = 1;
            break;
        }
        Sleep(10u);
    }
    vm_session_control_request_pause(&session->control, VM_SESSION_PAUSE_EXPLICIT);
    if (!vm_session_control_wait_for_pause(&session->control, 2000u)) goto fail;
    if (!prompt_seen) prompt_seen = has_dos_prompt(session);
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
    puts(argc == 3 && !turbo ? "M5:T209:S3:DOS-PROMPT-8086:OK" :
        turbo ? "M5:T459:S1:DOS-PROMPT-TURBO:OK" : "M5:T70:S2:DOS-PROMPT:OK");
    return 0;

fail:
    if (session != STD_NULL) dump_first_fault(session->core_machine);
    vm_session_stop(session);
    if (owns_session) vm_session_destroy(session);
    return 1;
}
static C_INT has_dos_prompt(const vm_session *session)
{
    core_platform_display_frame frame;
    STD_SIZE_T cell;

    if (session == STD_NULL) return 0;
    (C_VOID)core_platform_presentation_mailbox_capture(
        session->presentation_mailbox, &frame);
    for (cell = 0u; cell + 3u < TEXT_VIDEO_CELLS; ++cell) {
        const C_UCHAR drive = frame.characters[cell];
        if (STD_ISALPHA((C_UCHAR)drive) && frame.characters[cell + 1u] == ':' &&
            frame.characters[cell + 2u] == '\\' && frame.characters[cell + 3u] == '>') {
            return 1;
        }
    }
    return 0;
}
