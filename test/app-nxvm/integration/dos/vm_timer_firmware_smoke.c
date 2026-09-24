#include "lib/types/types_interface.h"
#include <stdio.h>

#include <windows.h>

#include "x86/debug/protocol_interface.h"
#include "test/app-nxvm/integration/support/session_ini.h"

#define VM_TIMER_BDA_TICKS 0x046cu
#define VM_TIMER_BDA_ROLLOVER 0x0470u
#define VM_TIMER_DAILY_LIMIT 0x001800b0u

static lib_i32 vm_timer_debug_execute(integration_ini_session *session,
    const x86_debug_request *request, x86_debug_response *out_response)
{
    common_machine_debug_lease lease;
    lib_size response_size = 0u;

    if (session == LIB_NULL || session->common_machine == LIB_NULL ||
        common_machine_debug_acquire(session->common_machine, &lease) != LIB_STATUS_OK ||
        common_machine_debug_execute_with_lease(session->common_machine, &lease,
            request, sizeof(*request), out_response, sizeof(*out_response),
            &response_size) != LIB_STATUS_OK || response_size != sizeof(*out_response))
        return 0;
    return 1;
}

static lib_i32 vm_timer_debug_break_real(integration_ini_session *session,
    lib_u32 linear)
{
    x86_debug_request request = {
        .operation = X86_DEBUG_SET_EXECUTION_PLAN,
        .execution_kind = X86_DEBUG_EXECUTION_BREAK_REAL,
        .address = linear
    };
    x86_debug_response response;

    if (!vm_timer_debug_execute(session, &request, &response) ||
        integration_ini_session_resume(session, 2000u) != LIB_STATUS_OK ||
        !integration_ini_session_wait_for_state(session, COMMON_MACHINE_PAUSED,
            2000u)) return 0;
    request = (x86_debug_request) { .operation = X86_DEBUG_GET_EXECUTION_RESULT };
    return vm_timer_debug_execute(session, &request, &response) && response.enabled;
}

static lib_i32 vm_timer_debug_write_real(integration_ini_session *session,
    lib_u16 segment, lib_u16 offset, const lib_u8 *data,
    lib_u8 bytes)
{
    x86_debug_request request = {
        .operation = X86_DEBUG_WRITE_REAL, .segment = segment, .offset = offset,
        .bytes = bytes
    };
    x86_debug_response response;

    if (bytes > X86_DEBUG_BYTES) return 0;
    lib_memory_copy(request.data, data, bytes);
    return vm_timer_debug_execute(session, &request, &response);
}

static lib_i32 vm_timer_debug_write_register(integration_ini_session *session,
    x86_debug_register register_id, lib_u32 value)
{
    x86_debug_request request = {
        .operation = X86_DEBUG_WRITE_REGISTER, .register_id = register_id,
        .address = value
    };
    x86_debug_response response;

    return vm_timer_debug_execute(session, &request, &response);
}

static lib_i32 vm_timer_debug_read_register(integration_ini_session *session,
    x86_debug_register register_id, lib_u32 *out_value)
{
    x86_debug_request request = {
        .operation = X86_DEBUG_READ_REGISTER, .register_id = register_id
    };
    x86_debug_response response;

    if (out_value == LIB_NULL || !vm_timer_debug_execute(session, &request, &response))
        return 0;
    *out_value = response.value;
    return 1;
}

lib_i32 main(lib_i32 argc, char **argv)
{
    integration_ini_session ini_session;
    DWORD elapsed;
    lib_u32 bda_ticks = 0u;
    lib_u32 int1a_ticks;
    lib_u32 paused_eip;
    lib_u32 observed_paused_eip;
    lib_u32 rollover_seed = VM_TIMER_DAILY_LIMIT - 1u;
    lib_u8 rollover_byte = 0u;
    lib_u32 register_value;
    lib_i32 stage = 0;
    static const lib_u8 int1a_program[] = { 0xb4u, 0x00u, 0xcdu, 0x1au, 0xf4u };
    static const lib_u8 rollover_program[] = {
        0xcdu, 0x08u, 0xb4u, 0x00u, 0xcdu, 0x1au, 0xf4u
    };

    stage = 1;
    if (argc != 3) goto fail;
    if (integration_ini_session_open(argv[1], argv[2], &ini_session) != LIB_STATUS_OK) {
        return 77;
    }
    stage = 2;
    if (integration_ini_session_start(&ini_session) != LIB_STATUS_OK) goto fail;
    stage = 3;
    /* Host time is only a bounded startup watchdog; guest time remains core-owned. */
    for (elapsed = 0u; elapsed < 1000u; elapsed += 10u) Sleep(10u);
    stage = 4;
    if (integration_ini_session_pause(&ini_session, 2000u) != LIB_STATUS_OK) goto fail;
    {
        x86_debug_request request = {
            .operation = X86_DEBUG_READ_REAL, .offset = VM_TIMER_BDA_TICKS,
            .bytes = sizeof(bda_ticks)
        };
        x86_debug_response response;

        if (!vm_timer_debug_execute(&ini_session, &request, &response)) goto fail;
        lib_memory_copy(&bda_ticks, response.data, sizeof(bda_ticks));
        if (bda_ticks == 0u) goto fail;
    }
    stage = 5;
    if (!vm_timer_debug_read_register(&ini_session, X86_DEBUG_EIP,
            &paused_eip)) goto fail;
    /* This is a host-side watchdog observation, never a guest clock source. */
    Sleep(25u);
    if (!vm_timer_debug_read_register(&ini_session, X86_DEBUG_EIP,
            &observed_paused_eip) || observed_paused_eip != paused_eip) goto fail;
    stage = 6;
    if (!vm_timer_debug_write_real(&ini_session, 0u, 0x1000u, int1a_program,
            sizeof(int1a_program)) ||
        !vm_timer_debug_write_register(&ini_session, X86_DEBUG_CS, 0u) ||
        !vm_timer_debug_write_register(&ini_session, X86_DEBUG_EIP, 0x1000u) ||
        !vm_timer_debug_write_register(&ini_session, X86_DEBUG_EAX, 0u)) goto fail;
    stage = 7;
    if (!vm_timer_debug_break_real(&ini_session, 0x1004u)) goto fail;
    stage = 8;
    if (!vm_timer_debug_read_register(&ini_session, X86_DEBUG_ECX, &int1a_ticks) ||
        !vm_timer_debug_read_register(&ini_session, X86_DEBUG_EDX, &register_value)) goto fail;
    int1a_ticks = (int1a_ticks << 16) | (register_value & 0xffffu);
    if (!vm_timer_debug_read_register(&ini_session, X86_DEBUG_EAX, &register_value) ||
        int1a_ticks != bda_ticks || (register_value & 0xffu) != 0u) goto fail;
    stage = 9;
    if (!vm_timer_debug_write_real(&ini_session, 0u, VM_TIMER_BDA_TICKS,
            (const lib_u8 *)&rollover_seed, sizeof(rollover_seed)) ||
        !vm_timer_debug_write_real(&ini_session, 0u, VM_TIMER_BDA_ROLLOVER,
            &rollover_byte, sizeof(rollover_byte)) ||
        !vm_timer_debug_write_real(&ini_session, 0u, 0x1100u, rollover_program,
            sizeof(rollover_program)) ||
        !vm_timer_debug_write_register(&ini_session, X86_DEBUG_CS, 0u) ||
        !vm_timer_debug_write_register(&ini_session, X86_DEBUG_EIP, 0x1100u) ||
        !vm_timer_debug_write_register(&ini_session, X86_DEBUG_EAX, 0u) ||
        !vm_timer_debug_break_real(&ini_session, 0x1106u)) goto fail;
    stage = 10;
    if (!vm_timer_debug_read_register(&ini_session, X86_DEBUG_ECX, &int1a_ticks) ||
        !vm_timer_debug_read_register(&ini_session, X86_DEBUG_EDX, &register_value)) goto fail;
    int1a_ticks = (int1a_ticks << 16) | (register_value & 0xffffu);
    if (!vm_timer_debug_read_register(&ini_session, X86_DEBUG_EAX, &register_value) ||
        int1a_ticks != 0u || (register_value & 0xffu) != 1u) goto fail;
    integration_ini_session_close(&ini_session);
    printf("M5:T225:S4:IRQ0-BDA-INT1A-ROLLOVER:DOS:OK\n");
    return 0;

fail:
    fprintf(stderr, "M5:T225:S3:TIMER:FAIL:%d:%u\n", stage, bda_ticks);
    integration_ini_session_close(&ini_session);
    return 1;
}
