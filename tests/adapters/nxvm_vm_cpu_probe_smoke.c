#include <stdio.h>

#include "vm/product/cpu_probe.h"
#include "core/product/runtime/execution_context.h"
#include "core/machine/vcpuins.h"

static int expect_capture(
    const nxvm_cpu_probe_capture *capture,
    uint32_t eax,
    uint16_t ip,
    uint32_t exception_mask)
{
    return capture->after.eax == eax && capture->after.ip == ip &&
        capture->exception_mask == exception_mask;
}

int main(void)
{
    const uint8_t mov_ax[] = { 0xb8u, 0x34u, 0x12u };
    const uint8_t add_ax[] = { 0x05u, 0x34u, 0x12u };
    const uint8_t short_jump[] = { 0xebu, 0x02u };
    const uint8_t segment_prefix_nop[] = { 0x26u, 0x90u };
    const uint8_t prefixed_mov[] = { 0x66u, 0xb8u, 0x78u, 0x56u, 0x34u, 0x12u };
    const uint8_t invalid[] = { 0x0fu, 0x0bu };
    nxvm_cpu_probe_capture capture;
    int failed = 0;

    if (!nxvm_cpu_probe_begin()) {
        return 1;
    }
    failed |= !nxvm_cpu_probe_step(mov_ax, sizeof(mov_ax), &capture);
    failed |= !expect_capture(&capture, 0x00001234u, 3u, 0u);
    failed |= !nxvm_cpu_probe_step(add_ax, sizeof(add_ax), &capture);
    failed |= !expect_capture(&capture, 0x00001234u, 3u, 0u);
    failed |= !nxvm_cpu_probe_step(
        short_jump, sizeof(short_jump), &capture);
    failed |= !expect_capture(&capture, 0u, 4u, 0u);
    failed |= !nxvm_cpu_probe_step(
        segment_prefix_nop, sizeof(segment_prefix_nop), &capture);
    failed |= !expect_capture(&capture, 0u, 2u, 0u);
    failed |= !nxvm_cpu_probe_step(
        prefixed_mov, sizeof(prefixed_mov), &capture);
    failed |= !expect_capture(&capture, 0x12345678u, 6u, 0u);
    failed |= !nxvm_cpu_probe_step(invalid, sizeof(invalid), &capture);
    failed |= !expect_capture(&capture, 0u, 0u, VCPUINS_EXCEPT_UD);
    failed |= capture.byte_count != sizeof(invalid) ||
        capture.before.cs != 0u || capture.before.ip != 0u ||
        capture.before.linear_pc != 0u || capture.exception_code != 0u;
    nxvm_cpu_probe_end();

    failed |= nxvm_execution_context_current() != NULL;
    failed |= !nxvm_cpu_probe_begin();
    failed |= !nxvm_cpu_probe_step(mov_ax, sizeof(mov_ax), &capture);
    failed |= !expect_capture(&capture, 0x00001234u, 3u, 0u);
    nxvm_cpu_probe_end();
    failed |= nxvm_execution_context_current() != NULL;

    if (failed) {
        return 1;
    }
    puts("M5:T1:S1:CPU-PROBES:OK");
    return 0;
}
