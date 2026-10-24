#include "app-mypc110/pc110_plan_interface.h"
#include "lib/types/file.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/retirement_observation_interface.h"
#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/waiting.h"

extern const x86_nxvm_machine_assets vm_app_firmware;

#define MYPC110_POST_DIAGNOSTIC_INSTRUCTIONS 8000000u
#define MYPC110_POST_DIAGNOSTIC_QUANTUM 256u

typedef struct mypc110_post_diagnostic_trace {
    x86_core_cpu_execution_point first_non_rom;
    x86_core_cpu_execution_point predecessor;
    x86_core_cpu_execution_point previous;
    lib_u32 pre_reset_c000_entries;
    lib_u32 post_reset_c000_entries;
    lib_u32 pre_reset_int10_entries;
    lib_u32 post_reset_int10_entries;
    lib_bool post_reset_active;
    lib_bool left_rom;
} mypc110_post_diagnostic_trace;

static void mypc110_post_diagnostic_observe_retirement(void *context,
    const x86_core_machine_retirement_observation *observation)
{
    mypc110_post_diagnostic_trace *trace = context;

    if (trace == LIB_NULL || observation == LIB_NULL) return;
    if (observation->point.linear_pc >= 0x000c0000u &&
        observation->point.linear_pc < 0x000f0000u) {
        if (trace->post_reset_active)
            ++trace->post_reset_c000_entries;
        else
            ++trace->pre_reset_c000_entries;
    }
    if (observation->point.linear_pc == 0x000f4ca9u) {
        if (trace->post_reset_active)
            ++trace->post_reset_int10_entries;
        else
            ++trace->pre_reset_int10_entries;
    }
    if (!trace->post_reset_active) return;
    if (!trace->left_rom && observation->point.cs != 0xf000u) {
        trace->left_rom = LIB_TRUE;
        trace->predecessor = trace->previous;
        trace->first_non_rom = observation->point;
    }
    trace->previous = observation->point;
}

int main(void)
{
    const x86_nxvm_machine_config config = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80486SX,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .bios_count = 1u};
    x86_nxvm_machine_construction construction = {0};
    x86_nxvm_machine *machine = LIB_NULL;
    x86_core_machine_run_result result = {0};
    x86_core_machine_observation observation = {0};
    x86_core_machine_observation reset_observation = {0};
    x86_core_machine_observation post_reset_observation = {0};
    x86_core_cpu_fault_snapshot post_reset_exception = {0};
    mypc110_post_diagnostic_trace post_reset_trace = {0};
    lib_u8 warm_signature[2u] = {0};
    lib_u8 c000_header[4u] = {0};
    lib_u64 executed = 0u;
    lib_u32 processor_reset_count = 0u;
    lib_u32 reset_exception_count = 0u;
    lib_bool advanced = LIB_FALSE;
    lib_u32 wait_count = 0u;
    lib_status status;

    status = mypc110_machine_construction_create(&config, &vm_app_firmware, &construction);
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_machine_create(&config, &construction, &machine);
    if (status == LIB_STATUS_OK) {
        status = x86_core_machine_set_retirement_observation_provider(
            machine->x86_core_machine,
            &(x86_core_machine_retirement_observation_provider){
                .callback = mypc110_post_diagnostic_observe_retirement,
                .context = &post_reset_trace});
    }
    if (status == LIB_STATUS_OK) {
        x86_nxvm_machine_executor_state_start(&machine->control.state);
    }
    while (status == LIB_STATUS_OK && executed < MYPC110_POST_DIAGNOSTIC_INSTRUCTIONS) {
        status = x86_core_machine_run(machine->x86_core_machine,
            (x86_core_machine_run_budget) {.instructions =
                processor_reset_count != 0u && !post_reset_exception.valid ? 1u :
                MYPC110_POST_DIAGNOSTIC_QUANTUM,
                .ticks = 0u}, &result);
        executed += result.executed;
        if (processor_reset_count != 0u && !post_reset_exception.valid &&
            x86_core_machine_capture_observation(machine->x86_core_machine,
                &observation) == LIB_STATUS_OK &&
            observation.diagnostic.delivered_exception_count > reset_exception_count) {
            post_reset_exception = observation.diagnostic.last_delivered_exception;
            post_reset_observation = observation;
        }
        if (status != LIB_STATUS_OK || result.reason == X86_CORE_MACHINE_STOP_FAULT) break;
        if (result.reason == X86_CORE_MACHINE_STOP_RESET_REQUESTED) {
            ++processor_reset_count;
            post_reset_trace.post_reset_active = LIB_TRUE;
            if (x86_core_machine_capture_observation(machine->x86_core_machine,
                    &reset_observation) == LIB_STATUS_OK) {
                reset_exception_count =
                    reset_observation.diagnostic.delivered_exception_count;
            }
            (void)x86_core_machine_memory_read(machine->x86_core_machine,
                0x00000472u, warm_signature, sizeof(warm_signature));
            (void)x86_core_machine_memory_read(machine->x86_core_machine,
                0x000c0000u, c000_header, sizeof(c000_header));
            continue;
        }
        if (result.reason == X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            ++wait_count;
            status = x86_nxvm_machine_waiting_advance(machine, &result, &advanced);
            if (status != LIB_STATUS_OK || !advanced) break;
        } else if (result.reason != X86_CORE_MACHINE_STOP_BUDGET) {
            break;
        }
    }
    if (machine != LIB_NULL) x86_nxvm_machine_control_end_run(&machine->control);
    if (machine != LIB_NULL)
        (void)x86_core_machine_capture_observation(machine->x86_core_machine, &observation);
    lib_c_printf("PC110:POST:status=%d reason=%u pc=%08x detail=%08x executed=%llu resets=%u waits=%u advanced=%u warm=%02x%02x c000-header=%02x%02x%02x%02x exceptions=%u first=%08x/%08x post-reset=%08x/%04x:%08x bytes=%02x %02x %02x %02x c000=%u/%u int10=%u/%u\n",
        (int)status, (lib_u32)result.reason, result.linear_pc, result.detail,
        (unsigned long long)executed, processor_reset_count, wait_count, advanced,
        warm_signature[1u], warm_signature[0u],
        c000_header[0u], c000_header[1u], c000_header[2u], c000_header[3u],
        observation.diagnostic.delivered_exception_count,
        observation.diagnostic.first_delivered_exception.exception_mask,
        observation.diagnostic.first_delivered_exception.point.linear_pc,
        post_reset_exception.exception_mask,
        post_reset_exception.point.cs, post_reset_exception.point.eip,
        post_reset_exception.point.bytes[0u], post_reset_exception.point.bytes[1u],
        post_reset_exception.point.bytes[2u], post_reset_exception.point.bytes[3u],
        post_reset_trace.pre_reset_c000_entries,
        post_reset_trace.post_reset_c000_entries,
        post_reset_trace.pre_reset_int10_entries,
        post_reset_trace.post_reset_int10_entries);
    if (post_reset_exception.valid) {
        lib_c_printf("PC110:POST:state=%04x:%08x base=%08x eflags=%08x halted=%u\n",
            post_reset_observation.cpu.cs, post_reset_observation.cpu.eip,
            post_reset_observation.cpu.cs_base, post_reset_observation.cpu.eflags,
            post_reset_observation.cpu.halted);
        if (post_reset_trace.left_rom) {
            lib_c_printf("PC110:POST:leave-rom=%04x:%08x %02x %02x %02x %02x -> %04x:%08x %02x %02x %02x %02x\n",
                post_reset_trace.predecessor.cs, post_reset_trace.predecessor.eip,
                post_reset_trace.predecessor.bytes[0u],
                post_reset_trace.predecessor.bytes[1u],
                post_reset_trace.predecessor.bytes[2u],
                post_reset_trace.predecessor.bytes[3u],
                post_reset_trace.first_non_rom.cs, post_reset_trace.first_non_rom.eip,
                post_reset_trace.first_non_rom.bytes[0u],
                post_reset_trace.first_non_rom.bytes[1u],
                post_reset_trace.first_non_rom.bytes[2u],
                post_reset_trace.first_non_rom.bytes[3u]);
        }
    }
    x86_nxvm_machine_destroy(machine);
    return status == LIB_STATUS_OK &&
        executed >= MYPC110_POST_DIAGNOSTIC_INSTRUCTIONS &&
        processor_reset_count != 0u ? 0 : 1;
}
