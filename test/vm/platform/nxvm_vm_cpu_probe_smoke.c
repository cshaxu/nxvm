#include "type.h"



#include "adapters/support/vm_cpu_probe.h"

#include "vm/composition/session/execution.h"

#include "core/machine/cpu_instructions.h"

static C_INT expect_capture(
    const vm_session_cpu_probe_capture *capture,
    type_unsigned_32 eax,
    type_unsigned_16 ip,
    type_unsigned_32 exception_mask)
{
    return capture->after.eax == eax && capture->after.ip == ip &&
        capture->exception_mask == exception_mask;
}

C_INT main(C_VOID)
{
    const type_unsigned_8 mov_ax[] = { 0xb8u, 0x34u, 0x12u };
    const type_unsigned_8 add_ax[] = { 0x05u, 0x34u, 0x12u };
    const type_unsigned_8 short_jump[] = { 0xebu, 0x02u };
    const type_unsigned_8 segment_prefix_nop[] = { 0x26u, 0x90u };
    const type_unsigned_8 prefixed_mov[] = { 0x66u, 0xb8u, 0x78u, 0x56u, 0x34u, 0x12u };
    const type_unsigned_8 invalid[] = { 0x0fu, 0x0bu };
    vm_session_cpu_probe_capture capture;
    test_vm_cpu_probe *probe = STD_NULL;
    C_INT failed = 0;

    if (!vm_session_cpu_probe_create(&probe)) {
        return 1;
    }
    failed |= !vm_session_cpu_probe_step(probe, mov_ax, sizeof(mov_ax), &capture);
    failed |= !expect_capture(&capture, 0x00001234u, 3u, 0u);
    failed |= !vm_session_cpu_probe_step(probe, add_ax, sizeof(add_ax), &capture);
    failed |= !expect_capture(&capture, 0x00001234u, 3u, 0u);
    failed |= !vm_session_cpu_probe_step(probe,
        short_jump, sizeof(short_jump), &capture);
    failed |= !expect_capture(&capture, 0u, 4u, 0u);
    failed |= !vm_session_cpu_probe_step(probe,
        segment_prefix_nop, sizeof(segment_prefix_nop), &capture);
    failed |= !expect_capture(&capture, 0u, 2u, 0u);
    failed |= !vm_session_cpu_probe_step(probe,
        prefixed_mov, sizeof(prefixed_mov), &capture);
    failed |= !expect_capture(&capture, 0x12345678u, 6u, 0u);
    failed |= !vm_session_cpu_probe_step(probe, invalid, sizeof(invalid), &capture);
    failed |= !expect_capture(&capture, 0u, 0u, VCPUINS_EXCEPT_UD);
    failed |= capture.byte_count != sizeof(invalid) ||
        capture.before.cs != 0u || capture.before.ip != 0u ||
        capture.before.linear_pc != 0u || capture.exception_code != 0u;
    vm_session_cpu_probe_destroy(probe);
    probe = STD_NULL;

    failed |= !vm_session_cpu_probe_create(&probe);
    failed |= !vm_session_cpu_probe_step(probe, mov_ax, sizeof(mov_ax), &capture);
    failed |= !expect_capture(&capture, 0x00001234u, 3u, 0u);
    vm_session_cpu_probe_destroy(probe);

    if (failed) {
        return 1;
    }
    puts("M5:T1:S1:CPU-PROBES:OK");
    return 0;
}
