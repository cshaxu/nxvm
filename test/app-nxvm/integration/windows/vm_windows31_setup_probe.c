#include "lib/types/types_interface.h"
#include <stdio.h>

#include <windows.h>

#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/debug_interface.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_interface.h"
#include "test/app-nxvm/unit/core/machine/support/vm_presentation_capture.h"
#include "test/app-nxvm/integration/support/session_ini.h"
#include "app-nxvm/machine/control.h"
#include "app-nxvm/machine/fault.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/machine/machine_private.h"
#include "../../unit/core/devices/support/core_machine_cpu_fixture.h"

#define VM_T287_TEXT_CELLS (80u * 25u)
#define VM_T287_BOOT_TIMEOUT_MILLISECONDS 60000u
#define VM_T287_SETUP_TIMEOUT_MILLISECONDS 20000u
#define VM_T287_FAULT_OBSERVATION_MILLISECONDS 60000u
#define VM_T288_POST_COPY_TIMEOUT_MILLISECONDS 720000u

static lib_i32 vm_t287_submit_input(vm_machine *session,
    lib_u16 scan_code, lib_u16 virtual_key, lib_i32 pressed)
{
    core_machine_guest_input_event event = { 0 };

    if (session == LIB_NULL) return 0;
    event.kind = CORE_MACHINE_GUEST_INPUT_KEY;
    event.data.key.scan_code = scan_code;
    event.data.key.virtual_key = virtual_key;
    event.data.key.pressed = pressed != 0;
    return vm_machine_submit_host_input(session, &event) == LIB_STATUS_OK;
}

static lib_i32 vm_t287_has_text(const vm_machine *session, const char *text)
{
    core_machine_guest_display_frame frame;
    lib_size cell;
    lib_size character;
    lib_size length = lib_text_length(text);

    if (session == LIB_NULL || text == LIB_NULL || length == 0u ||
        test_vm_machine_capture_presentation(session, &frame) != LIB_STATUS_OK) return 0;
    for (cell = 0u; cell + length <= VM_T287_TEXT_CELLS; ++cell) {
        for (character = 0u; character < length; ++character) {
            if (frame.characters[cell + character] != (lib_u8)text[character]) break;
        }
        if (character == length) return 1;
    }
    return 0;
}

static lib_i32 vm_t287_has_prompt(const vm_machine *session)
{
    core_machine_guest_display_frame frame;
    lib_size cell;

    if (session == LIB_NULL || test_vm_machine_capture_presentation(session,
            &frame) != LIB_STATUS_OK) return 0;
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

static lib_i32 vm_t287_wait_for(const vm_machine *session, const char *text,
    DWORD timeout)
{
    DWORD elapsed;

    for (elapsed = 0u; elapsed < timeout; elapsed += 10u) {
        if ((text == LIB_NULL && vm_t287_has_prompt(session)) ||
            (text != LIB_NULL && vm_t287_has_text(session, text))) return 1;
        if (elapsed >= 500u && !vm_machine_control_is_running(&session->control)) {
            return 0;
        }
        Sleep(10u);
    }
    return 0;
}

static lib_i32 vm_t287_submit_return(vm_machine *session)
{
    if (!vm_t287_submit_input(session, 0x1cu, VK_RETURN, 1)) return 0;
    Sleep(25u);
    return vm_t287_submit_input(session, 0x1cu, VK_RETURN, 0);
}

static lib_i32 vm_t287_type_setup(vm_machine *session)
{
    static const lib_u8 scan_codes[] = {
        0x2bu, 0x12u, 0x11u, 0x17u, 0x31u, 0x04u, 0x02u, 0x2bu, 0x1fu,
        0x12u, 0x14u, 0x16u, 0x19u, 0x34u, 0x12u, 0x2du, 0x12u, 0x1cu
    };
    static const lib_u8 virtual_keys[] = {
        VK_OEM_5, 'E', 'W', 'I', 'N', '3', '1', VK_OEM_5, 'S', 'E', 'T',
        'U', 'P', VK_OEM_PERIOD, 'E', 'X', 'E', VK_RETURN
    };
    lib_size index;

    if (session == LIB_NULL) return 0;
    for (index = 0u; index < sizeof(scan_codes); ++index) {
        if (!vm_t287_submit_input(session, scan_codes[index], virtual_keys[index], 1)) {
            return 0;
        }
        Sleep(25u);
        if (!vm_t287_submit_input(session, scan_codes[index], virtual_keys[index], 0)) {
            return 0;
        }
        Sleep(25u);
    }
    return 1;
}

static lib_i32 vm_t288_type_windows(vm_machine *session)
{
    static const lib_u8 scan_codes[] = {
        0x11u, 0x17u, 0x31u, 0x20u, 0x18u, 0x11u, 0x1fu, 0x1cu
    };
    static const lib_u8 virtual_keys[] = {
        'W', 'I', 'N', 'D', 'O', 'W', 'S', VK_RETURN
    };
    lib_size index;

    if (session == LIB_NULL) return 0;
    for (index = 0u; index < sizeof(scan_codes); ++index) {
        if (!vm_t287_submit_input(session, scan_codes[index], virtual_keys[index], 1)) {
            return 0;
        }
        Sleep(25u);
        if (!vm_t287_submit_input(session, scan_codes[index], virtual_keys[index], 0)) {
            return 0;
        }
        Sleep(25u);
    }
    return 1;
}

static void vm_t287_print_frame(const vm_machine *session)
{
    core_machine_guest_display_frame frame;
    lib_size row;
    lib_size column;

    if (session == LIB_NULL || test_vm_machine_capture_presentation(session,
            &frame) != LIB_STATUS_OK) return;
    for (row = 0u; row < 25u; ++row) {
        for (column = 0u; column < 80u; ++column) {
            lib_u8 character = frame.characters[row * 80u + column];

            printf("%c", character == 0u ? ' ' : character);
        }
        printf("\n");
    }
}

static void vm_t287_report_fault(vm_machine *session, const char *stage)
{
    core_machine_cpu_diagnostic diagnostic = {0};
    t_cpu cpu;
    lib_size index;

    if (session == LIB_NULL) return;
    (void)core_machine_get_cpu_diagnostic(session->core_machine, &diagnostic);
    printf("M5:T287:S23:WINDOWS31:SETUP:CHECKPOINT stage=%s running=%d "
        "ata_commands=%u last_command=%02X\n", stage,
        vm_machine_control_is_running(&session->control),
        session->core_machine->hdc.data.command_count,
        session->core_machine->hdc.data.last_command);
    if (diagnostic.first_fault.valid) {
        const core_machine_cpu_fault_snapshot *fault = &diagnostic.first_fault;

        printf("M5:T288:S1:WINDOWS31:FAULT mask=%08X code=%08X "
            "cs=%04X ip=%08X linear=%08X opcode=", fault->exception_mask,
            fault->exception_code, fault->point.cs, fault->point.eip,
            fault->point.linear_pc);
        for (index = 0u; index < fault->point.byte_count; ++index) {
            printf("%02X", fault->point.bytes[index]);
        }
        printf(" eax=%08X ebx=%08X ecx=%08X edx=%08X esp=%08X ebp=%08X "
            "esi=%08X edi=%08X flags=%08X\n",
            fault->eax, fault->ebx, fault->ecx, fault->edx, fault->esp,
            fault->ebp, fault->esi, fault->edi, fault->eflags);
        for (index = 0u; index < diagnostic.recent_count; ++index) {
            const core_machine_cpu_execution_point *point =
                &diagnostic.recent[index];

            printf("M5:T288:S1:WINDOWS31:RECENT cs=%04X ip=%08X "
                "linear=%08X opcode=%02X%02X%02X\n", point->cs, point->eip,
                point->linear_pc, point->bytes[0], point->bytes[1],
                point->bytes[2]);
        }
    }
    cpu = test_core_machine_fixture_capture_cpu_after_run(session->core_machine);
    if (session->core_machine != LIB_NULL) {
        printf("M5:T287:S23:WINDOWS31:CPU cr0=%08X cr2=%08X cr3=%08X "
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

lib_i32 main(lib_i32 argc, char **argv)
{
    integration_ini_session ini_session = {0};
    vm_machine *session = LIB_NULL;
    const char *stage = "create";
    lib_i32 observed_setup_inf = 0;
    lib_i32 passed = 0;
    lib_i32 advance_steps = argc == 4 ? argv[3][0] - '0' : 0;
    lib_i32 date_prompt = 0;
    DWORD elapsed;

    if ((argc != 3 && argc != 4) || integration_ini_session_open(argv[1], argv[2],
            &ini_session) != LIB_STATUS_OK) return 77;
    session = ini_session.session;
    if (session == LIB_NULL) goto fail;
    if (integration_ini_session_start(&ini_session) != LIB_STATUS_OK) goto fail;
    stage = "boot";
    for (elapsed = 0u; elapsed < VM_T287_BOOT_TIMEOUT_MILLISECONDS; elapsed += 10u) {
        date_prompt = vm_t287_has_text(session, "Enter new date");
        if (date_prompt || vm_t287_has_prompt(session)) break;
        if (elapsed >= 500u && !vm_machine_control_is_running(&session->control)) {
            goto fail;
        }
        Sleep(10u);
    }
    if (elapsed == VM_T287_BOOT_TIMEOUT_MILLISECONDS) goto fail;
    if (date_prompt) {
        stage = "date";
        if (!vm_t287_submit_return(session)) goto fail;
        stage = "time";
        if (!vm_t287_wait_for(session, "Enter new time", VM_T287_BOOT_TIMEOUT_MILLISECONDS) ||
            !vm_t287_submit_return(session)) goto fail;
    }
    stage = "prompt";
    if (!vm_t287_wait_for(session, LIB_NULL, VM_T287_BOOT_TIMEOUT_MILLISECONDS)) goto fail;
    stage = "setup-command";
    if (!vm_t287_type_setup(session)) goto fail;
    stage = "setup-result";
    observed_setup_inf = vm_t287_wait_for(session, "Reading SETUP.INF...",
        VM_T287_SETUP_TIMEOUT_MILLISECONDS);
    if (observed_setup_inf) {
        Sleep(VM_T287_FAULT_OBSERVATION_MILLISECONDS);
        if (!vm_machine_control_is_running(&session->control)) {
            vm_t287_report_fault(session, "setup-inf-fault");
            goto done;
        }
        if (vm_t287_has_text(session, "Welcome to Setup.")) {
            stage = "welcome";
            if (advance_steps != 0) {
                if (!vm_t287_submit_return(session)) goto fail;
                Sleep(3000u);
                if (advance_steps >= 2) {
                    if (!vm_t287_submit_return(session)) goto fail;
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
                            if (!vm_machine_control_is_running(&session->control)) break;
                            Sleep(100u);
                        }
                    }
                }
                vm_t287_report_fault(session, "after-welcome-enter");
                goto done;
            }
            passed = 1;
            printf("M5:T287:S24:WINDOWS31:SETUP:OK result=welcome\n");
            vm_t287_print_frame(session);
            goto done;
        }
        vm_t287_report_fault(session, "setup-inf-running");
    } else {
        vm_t287_report_fault(session, stage);
    }

done:
    if (session != LIB_NULL) vm_machine_stop(session);
    integration_ini_session_close(&ini_session);
    return passed ? 0 : 1;

fail:
    vm_t287_report_fault(session, stage);
    goto done;
}
