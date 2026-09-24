#include "lib/types/types_interface.h"
#include <ctype.h>
#include <stdio.h>

#include <windows.h>







#include "app-nxvm/devices/debug_interface.h"
#include "test/app-nxvm/unit/core/machine/support/vm_presentation_capture.h"
#include "app-nxvm/devices/machine_interface.h"

#include "app-nxvm/machine/lifecycle.h"

#include "app-nxvm/machine/machine_private.h"
#include "test/app-nxvm/integration/support/session_ini.h"

#define TEXT_VIDEO_BASE 0x000b8000u
#define TEXT_VIDEO_CELLS (80u * 25u)
#define DOS_PROMPT_TIMEOUT_MILLISECONDS 5000u

static lib_i32 has_dos_prompt(const vm_machine *session);

static void dump_first_fault(core_machine *machine)
{
    core_machine_cpu_diagnostic diagnostic;
    const core_machine_cpu_fault_snapshot *fault;
    lib_size index;

    if (core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
        LIB_STATUS_OK || !diagnostic.first_fault.valid) return;
    fault = &diagnostic.first_fault;
    fprintf(stderr,
        "M5:T155:S1:BOOT-FAULT CS:IP=%04X:%08X BYTES=",
        fault->point.cs, fault->point.eip);
    for (index = 0u; index < fault->point.byte_count; ++index) {
        fprintf(stderr, "%02X", fault->point.bytes[index]);
    }
    fprintf(stderr, "%s", "\n");
}

lib_i32 main(lib_i32 argc, char **argv)
{
    DWORD elapsed;
    lib_i32 prompt_seen = 0;
    integration_ini_session ini_session;
    vm_machine *session;
    lib_i32 turbo = 0;

    if ((argc != 3 && argc != 4) || integration_ini_session_open(argv[1], argv[2],
            &ini_session) != LIB_STATUS_OK) return 77;
    session = ini_session.session;
    turbo = argc == 4;
    if ((turbo && lib_text_compare(argv[3], "turbo")) ||
        (turbo && vm_machine_set_speed(session, VM_MACHINE_SPEED_TURBO) != LIB_STATUS_OK)) {
        goto fail;
    }
    if (integration_ini_session_start(&ini_session) != LIB_STATUS_OK) goto fail;

    for (elapsed = 0u; elapsed < DOS_PROMPT_TIMEOUT_MILLISECONDS; elapsed += 10u) {
        if (has_dos_prompt(session)) {
            prompt_seen = 1;
            break;
        }
        Sleep(10u);
    }
    if (integration_ini_session_pause(&ini_session, 2000u) != LIB_STATUS_OK)
        goto fail;
    if (!prompt_seen) prompt_seen = has_dos_prompt(session);
    if (!prompt_seen) {
        dump_first_fault(session->core_machine);
        fprintf(stderr, "%s", "M5:T70:S2:DOS-PROMPT:TIMEOUT\n");
        goto fail;
    }
    integration_ini_session_close(&ini_session);
    puts(turbo ? "M5:T459:S1:DOS-PROMPT-TURBO:OK" : "M5:T70:S2:DOS-PROMPT:OK");
    return 0;

fail:
    if (session != LIB_NULL) dump_first_fault(session->core_machine);
    integration_ini_session_close(&ini_session);
    return 1;
}
static lib_i32 has_dos_prompt(const vm_machine *session)
{
    core_machine_guest_display_frame frame;
    lib_size cell;

    if (session == LIB_NULL) return 0;
    (void)test_vm_machine_capture_presentation(session, &frame);
    for (cell = 0u; cell + 3u < TEXT_VIDEO_CELLS; ++cell) {
        const lib_u8 drive = frame.characters[cell];
        if (isalpha((lib_u8)drive) && frame.characters[cell + 1u] == ':' &&
            frame.characters[cell + 2u] == '\\' && frame.characters[cell + 3u] == '>') {
            return 1;
        }
    }
    return 0;
}
