#include "app-mypc110/pc110_plan_interface.h"
#include "lib/types/file.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/debug_interface.h"
#include "x86-core/machine/retirement_observation_interface.h"
#include "x86-core/machine/trace_interface.h"
#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/waiting.h"

extern const x86_nxvm_machine_assets vm_app_firmware;

#define MYPC110_POST_DIAGNOSTIC_INSTRUCTIONS 8000000u
#define MYPC110_POST_DIAGNOSTIC_C000_INSTRUCTIONS 12000000u
/* A containment ceiling only: the success condition is the post-C000 wait
 * transition below, never this retirement count. */
#define MYPC110_POST_DIAGNOSTIC_C000_PORT61_FOLLOW_INSTRUCTIONS 13000000u
#define MYPC110_POST_DIAGNOSTIC_QUANTUM 256u
#define MYPC110_POST_DIAGNOSTIC_C000_QUANTUM 4096u
#define MYPC110_POST_DIAGNOSTIC_PORT_CAPACITY 128u
#define MYPC110_POST_DIAGNOSTIC_C000_PORT61_READ_CAPACITY 16u
#define MYPC110_POST_DIAGNOSTIC_RETIREMENT_CAPACITY 16u
#define MYPC110_POST_DIAGNOSTIC_C000_RETIREMENT_CAPACITY 16u
#define MYPC110_POST_DIAGNOSTIC_PORT61_FOLLOW_CAPACITY 32u
#define MYPC110_POST_DIAGNOSTIC_DATA_SELECTOR_BASE 0x0000d8a0u

typedef struct mypc110_post_diagnostic_port_write {
    lib_u32 linear_pc;
    lib_u32 type;
    lib_u16 port;
    lib_u8 value;
} mypc110_post_diagnostic_port_write;

typedef struct mypc110_post_diagnostic_port_read {
    lib_u64 elapsed_ticks;
    lib_u32 linear_pc;
    lib_u8 value;
} mypc110_post_diagnostic_port_read;

typedef struct mypc110_post_diagnostic_trace {
    x86_core_cpu_execution_point first_non_rom;
    x86_core_cpu_execution_point predecessor;
    x86_core_cpu_execution_point previous;
    lib_u32 pre_reset_c000_entries;
    lib_u32 post_reset_c000_entries;
    lib_u32 pre_reset_e000_entries;
    lib_u32 post_reset_e000_entries;
    x86_core_cpu_execution_point
        c000_retirements[MYPC110_POST_DIAGNOSTIC_C000_RETIREMENT_CAPACITY];
    lib_size c000_retirement_count;
    x86_core_cpu_execution_point
        pre_reset_retirements[MYPC110_POST_DIAGNOSTIC_RETIREMENT_CAPACITY];
    lib_size pre_reset_retirement_count;
    x86_core_cpu_debug_cpu_snapshot before_pop_ds;
    lib_bool before_pop_ds_valid;
    x86_core_cpu_debug_cpu_snapshot after_pop_ds;
    lib_bool after_pop_ds_valid;
    lib_bool post_reset_active;
    lib_bool entered_port61_wait;
    lib_bool left_port61_wait;
    x86_core_cpu_execution_point
        port61_follow_retirements[MYPC110_POST_DIAGNOSTIC_PORT61_FOLLOW_CAPACITY];
    lib_size port61_follow_retirement_count;
    lib_bool entered_lpt_checkpoint;
    lib_bool returned_to_port61_wait;
    lib_bool c000_only;
    lib_bool crossed_c000_port_boundary;
    lib_bool left_c000_after_boundary;
    x86_core_cpu_execution_point first_after_c000;
    lib_bool follow_c000_port61;
    lib_bool entered_post_c000_port61_wait;
    lib_bool first_post_c000_port61_counter_valid;
    lib_u32 first_post_c000_port61_ecx;
    lib_bool observed_post_c000_port61_return;
    lib_bool left_post_c000_port61_wait;
    x86_core_cpu_execution_point first_after_post_c000_port61_wait;
    mypc110_post_diagnostic_port_read
        post_c000_port61_reads[MYPC110_POST_DIAGNOSTIC_C000_PORT61_READ_CAPACITY];
    lib_size post_c000_port61_read_count;
    lib_bool left_rom;
    mypc110_post_diagnostic_port_write
        configuration_writes[MYPC110_POST_DIAGNOSTIC_PORT_CAPACITY];
    lib_size configuration_write_count;
} mypc110_post_diagnostic_trace;

static void mypc110_post_diagnostic_observe_trace(void *context,
    const x86_core_machine_trace_event *event)
{
    mypc110_post_diagnostic_trace *trace = context;
    lib_u16 port;

    if (trace == LIB_NULL || event == LIB_NULL ||
        (event->type != X86_CORE_MACHINE_TRACE_PORT_READ &&
         event->type != X86_CORE_MACHINE_TRACE_PORT_WRITE)) {
        return;
    }
    port = (lib_u16)event->address;
    if (port != 0x0022u && port != 0x0024u && port != 0x0025u &&
        port != 0x0074u && port != 0x0076u &&
        port != 0x00ecu && port != 0x00edu && port != 0x0060u &&
        port != 0x0061u && port != 0x0064u && port != 0x03c4u &&
        port != 0x03c5u) {
        return;
    }
    if (trace->configuration_write_count < MYPC110_POST_DIAGNOSTIC_PORT_CAPACITY) {
        trace->configuration_writes[trace->configuration_write_count++] =
            (mypc110_post_diagnostic_port_write) {
                event->linear_pc, event->type, port, (lib_u8)event->value};
    }
}

static void mypc110_post_diagnostic_observe_retirement(void *context,
    const x86_core_machine_retirement_observation *observation)
{
    mypc110_post_diagnostic_trace *trace = context;
    lib_size index;

    if (trace == LIB_NULL || observation == LIB_NULL) return;
    if (trace->c000_only) {
        if (trace->post_reset_active && observation->point.linear_pc >= 0x000c0000u &&
            observation->point.linear_pc < 0x000d0000u) {
            ++trace->post_reset_c000_entries;
            if (observation->point.linear_pc > 0x000c6000u)
                trace->crossed_c000_port_boundary = LIB_TRUE;
        } else if (trace->post_reset_active && trace->crossed_c000_port_boundary &&
            !trace->left_c000_after_boundary) {
            trace->left_c000_after_boundary = LIB_TRUE;
            trace->first_after_c000 = observation->point;
        }
        if (!trace->follow_c000_port61 || !trace->left_c000_after_boundary) return;
    }
    if (!trace->post_reset_active) {
        index = trace->pre_reset_retirement_count %
            MYPC110_POST_DIAGNOSTIC_RETIREMENT_CAPACITY;
        trace->pre_reset_retirements[index] = observation->point;
        ++trace->pre_reset_retirement_count;
    }
    if (observation->point.linear_pc >= 0x000c0000u &&
        observation->point.linear_pc < 0x000d0000u) {
        if (trace->post_reset_active)
            ++trace->post_reset_c000_entries;
        else
            ++trace->pre_reset_c000_entries;
        if (trace->post_reset_active) {
            index = trace->c000_retirement_count %
                MYPC110_POST_DIAGNOSTIC_C000_RETIREMENT_CAPACITY;
            trace->c000_retirements[index] = observation->point;
            ++trace->c000_retirement_count;
        }
    }
    if (observation->point.linear_pc >= 0x000e0000u &&
        observation->point.linear_pc < 0x000f0000u) {
        if (trace->post_reset_active)
            ++trace->post_reset_e000_entries;
        else
            ++trace->pre_reset_e000_entries;
    }
    if (observation->point.linear_pc == 0x000f6043u) {
        trace->before_pop_ds = observation->current_cpu;
        trace->before_pop_ds_valid = LIB_TRUE;
    }
    if (!trace->post_reset_active) return;
    if (observation->point.linear_pc >= 0x000fea86u &&
        observation->point.linear_pc <= 0x000fea90u) {
        trace->entered_port61_wait = LIB_TRUE;
    } else if (trace->entered_port61_wait) {
        trace->left_port61_wait = LIB_TRUE;
    }
    if (trace->left_port61_wait) {
        index = trace->port61_follow_retirement_count %
            MYPC110_POST_DIAGNOSTIC_PORT61_FOLLOW_CAPACITY;
        trace->port61_follow_retirements[index] = observation->point;
        ++trace->port61_follow_retirement_count;
    }
    if (trace->follow_c000_port61 && observation->point.linear_pc >= 0x000fea86u &&
        observation->point.linear_pc <= 0x000fea90u) {
        trace->entered_post_c000_port61_wait = LIB_TRUE;
        if (!trace->first_post_c000_port61_counter_valid &&
            observation->point.linear_pc == 0x000fea86u) {
            trace->first_post_c000_port61_counter_valid = LIB_TRUE;
            trace->first_post_c000_port61_ecx = observation->current_cpu.ecx;
        }
    }
    if (trace->follow_c000_port61 && trace->entered_post_c000_port61_wait &&
        observation->point.linear_pc == 0x000fea92u)
        trace->observed_post_c000_port61_return = LIB_TRUE;
    if (trace->follow_c000_port61 && trace->observed_post_c000_port61_return &&
        !trace->left_post_c000_port61_wait &&
        (observation->point.linear_pc < 0x000fea86u ||
         observation->point.linear_pc > 0x000fea92u)) {
        trace->left_post_c000_port61_wait = LIB_TRUE;
        trace->first_after_post_c000_port61_wait = observation->point;
    }
    if (trace->follow_c000_port61 && trace->left_c000_after_boundary &&
        observation->io_direction == X86_CORE_MACHINE_RETIREMENT_IO_READ &&
        observation->io_port == 0x0061u &&
        trace->post_c000_port61_read_count <
            MYPC110_POST_DIAGNOSTIC_C000_PORT61_READ_CAPACITY) {
        trace->post_c000_port61_reads[trace->post_c000_port61_read_count++] =
            (mypc110_post_diagnostic_port_read) {
                observation->elapsed_ticks, observation->point.linear_pc,
                (lib_u8)observation->io_value};
    }
    if (observation->point.linear_pc == 0x000f55b2u)
        trace->entered_lpt_checkpoint = LIB_TRUE;
    if (trace->entered_lpt_checkpoint && observation->point.linear_pc >= 0x000fea86u &&
        observation->point.linear_pc <= 0x000fea90u) {
        trace->returned_to_port61_wait = LIB_TRUE;
    }
    if (!trace->left_rom && observation->point.cs != 0xf000u) {
        trace->left_rom = LIB_TRUE;
        trace->predecessor = trace->previous;
        trace->first_non_rom = observation->point;
    }
    trace->previous = observation->point;
}

int main(int argc, char **argv)
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
    x86_core_cpu_fault_snapshot reset_exception = {0};
    x86_core_cpu_debug_cpu_snapshot reset_cpu = {0};
    lib_u64 before_pop_ds_descriptor = 0u;
    lib_status before_pop_ds_memory_status = LIB_STATUS_INVALID_STATE;
    lib_bool advanced = LIB_FALSE;
    lib_bool reset_checkpoint = argc == 2 && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--after-reset") == 0;
    lib_bool port61_progress_checkpoint = argc == 2 && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--port61-progress") == 0;
    lib_bool port61_follow_checkpoint = argc == 2 && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--port61-follow") == 0;
    lib_bool lpt_cycle_checkpoint = argc == 2 && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--lpt-cycle") == 0;
    lib_bool c000_checkpoint = argc == 2 && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--c000") == 0;
    lib_bool c000_follow_checkpoint = argc == 2 && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--c000-follow") == 0;
    lib_bool c000_port61_follow_checkpoint = argc == 2 && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--c000-port61-follow") == 0;
    lib_bool c000_port61_return_checkpoint = argc == 2 && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--c000-port61-return") == 0;
    lib_u64 instruction_limit;
    lib_u32 wait_count = 0u;
    lib_status status;

    if (argc != 1 && !reset_checkpoint && !port61_progress_checkpoint &&
        !port61_follow_checkpoint && !lpt_cycle_checkpoint &&
        !c000_checkpoint && !c000_follow_checkpoint && !c000_port61_follow_checkpoint &&
        !c000_port61_return_checkpoint) return 2;
    instruction_limit = c000_port61_follow_checkpoint || c000_port61_return_checkpoint ?
        MYPC110_POST_DIAGNOSTIC_C000_PORT61_FOLLOW_INSTRUCTIONS :
        (c000_checkpoint || c000_follow_checkpoint ?
            MYPC110_POST_DIAGNOSTIC_C000_INSTRUCTIONS :
            MYPC110_POST_DIAGNOSTIC_INSTRUCTIONS);
    post_reset_trace.c000_only = c000_checkpoint || c000_follow_checkpoint ||
        c000_port61_follow_checkpoint || c000_port61_return_checkpoint;
    post_reset_trace.follow_c000_port61 = c000_port61_follow_checkpoint ||
        c000_port61_return_checkpoint;

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
        status = x86_core_machine_set_trace_provider(machine->x86_core_machine,
            &(x86_core_machine_trace_provider) {
                .callback = mypc110_post_diagnostic_observe_trace,
                .context = &post_reset_trace});
    }
    if (status == LIB_STATUS_OK && !reset_checkpoint && !port61_progress_checkpoint &&
        !port61_follow_checkpoint && !lpt_cycle_checkpoint && !c000_checkpoint &&
        !c000_follow_checkpoint && !c000_port61_follow_checkpoint &&
        !c000_port61_return_checkpoint) {
        status = x86_core_machine_debug_set_watchpoint(machine->x86_core_machine,
            X86_CORE_CPU_DEBUG_WATCH_EXECUTE,
            0x000f6060u);
    }
    if (status == LIB_STATUS_OK) {
        x86_nxvm_machine_executor_state_start(&machine->control.state);
    }
    while (status == LIB_STATUS_OK && executed < instruction_limit) {
        status = x86_core_machine_run(machine->x86_core_machine,
            (x86_core_machine_run_budget) {.instructions =
                processor_reset_count != 0u && !post_reset_exception.valid &&
                    !port61_progress_checkpoint && !port61_follow_checkpoint &&
                    !lpt_cycle_checkpoint && !c000_checkpoint &&
                    !c000_follow_checkpoint && !c000_port61_follow_checkpoint &&
                    !c000_port61_return_checkpoint ? 1u :
                c000_checkpoint || c000_follow_checkpoint || c000_port61_follow_checkpoint ||
                    c000_port61_return_checkpoint ?
                    MYPC110_POST_DIAGNOSTIC_C000_QUANTUM :
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
        if (port61_progress_checkpoint && post_reset_trace.left_port61_wait) break;
        if (port61_follow_checkpoint &&
            post_reset_trace.port61_follow_retirement_count >=
                MYPC110_POST_DIAGNOSTIC_PORT61_FOLLOW_CAPACITY) break;
        if (lpt_cycle_checkpoint && post_reset_trace.returned_to_port61_wait) break;
        if (c000_checkpoint && post_reset_trace.crossed_c000_port_boundary) break;
        if (c000_follow_checkpoint && post_reset_trace.left_c000_after_boundary) break;
        if (c000_port61_follow_checkpoint &&
            post_reset_trace.post_c000_port61_read_count >=
                MYPC110_POST_DIAGNOSTIC_C000_PORT61_READ_CAPACITY) break;
        if (c000_port61_return_checkpoint &&
            post_reset_trace.left_post_c000_port61_wait) break;
        if (result.reason == X86_CORE_MACHINE_STOP_RESET_REQUESTED) {
            ++processor_reset_count;
            post_reset_trace.post_reset_active = LIB_TRUE;
            if (x86_core_machine_capture_observation(machine->x86_core_machine,
                    &reset_observation) == LIB_STATUS_OK) {
                reset_exception_count =
                    reset_observation.diagnostic.delivered_exception_count;
                reset_exception = reset_observation.diagnostic.last_delivered_exception;
            }
            (void)x86_core_machine_memory_read(machine->x86_core_machine,
                0x00000472u, warm_signature, sizeof(warm_signature));
            (void)x86_core_machine_memory_read(machine->x86_core_machine,
                0x000c0000u, c000_header, sizeof(c000_header));
            (void)x86_core_machine_debug_capture_cpu_snapshot(machine->x86_core_machine,
                X86_CORE_CPU_SNAPSHOT_INSTRUCTION_ENTRY, &reset_cpu);
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
    if (status == LIB_STATUS_OK && result.reason == X86_CORE_MACHINE_STOP_PAUSED &&
        machine != LIB_NULL) {
        status = x86_core_machine_debug_capture_cpu_snapshot(
            machine->x86_core_machine, X86_CORE_CPU_SNAPSHOT_CURRENT,
            &post_reset_trace.after_pop_ds);
        post_reset_trace.after_pop_ds_valid = status == LIB_STATUS_OK;
    }
    if (post_reset_trace.before_pop_ds_valid && machine != LIB_NULL) {
        const x86_core_cpu_debug_cpu_snapshot *cpu =
            &post_reset_trace.before_pop_ds;

        before_pop_ds_memory_status = x86_core_machine_memory_read(
            machine->x86_core_machine,
            cpu->gdtr.base + 0x48u, &before_pop_ds_descriptor,
            sizeof(before_pop_ds_descriptor));
    }
    if (machine != LIB_NULL) x86_nxvm_machine_control_end_run(&machine->control);
    if (machine != LIB_NULL)
        (void)x86_core_machine_capture_observation(machine->x86_core_machine, &observation);
    lib_c_printf("PC110:POST:status=%d reason=%u pc=%08x detail=%08x fault=%08x/%08x executed=%llu resets=%u waits=%u advanced=%u warm=%02x%02x c000-header=%02x%02x%02x%02x exceptions=%u first=%08x/%08x post-reset=%08x/%04x:%08x bytes=%02x %02x %02x %02x c000=%u/%u e000=%u/%u port61-wait=%u/%u\n",
        (int)status, (lib_u32)result.reason, result.linear_pc, result.detail,
        observation.diagnostic.first_fault.exception_mask,
        observation.diagnostic.first_fault.exception_code,
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
        post_reset_trace.pre_reset_e000_entries,
        post_reset_trace.post_reset_e000_entries,
        post_reset_trace.entered_port61_wait, post_reset_trace.left_port61_wait);
    if (reset_exception.valid) {
        lib_c_printf("PC110:POST:reset-exception=%08x code=%08x point=%04x:%08x bytes=%02x %02x %02x %02x stack=%04x:%08x flags=%08x\n",
            reset_exception.exception_mask, reset_exception.exception_code,
            reset_exception.point.cs, reset_exception.point.eip,
            reset_exception.point.bytes[0u], reset_exception.point.bytes[1u],
            reset_exception.point.bytes[2u], reset_exception.point.bytes[3u],
            reset_exception.ss, reset_exception.esp, reset_exception.eflags);
    }
    if (observation.diagnostic.first_fault.valid) {
        const x86_core_cpu_fault_snapshot *fault =
            &observation.diagnostic.first_fault;

        lib_c_printf("PC110:POST:fault=%08x code=%08x point=%04x:%08x bytes=%02x %02x %02x %02x eax=%08x edx=%08x flags=%08x\n",
            fault->exception_mask, fault->exception_code,
            fault->point.cs, fault->point.eip,
            fault->point.bytes[0u], fault->point.bytes[1u],
            fault->point.bytes[2u], fault->point.bytes[3u],
            fault->eax, fault->edx, fault->eflags);
    }
    if (post_reset_trace.before_pop_ds_valid) {
        const x86_core_cpu_debug_cpu_snapshot *cpu =
            &post_reset_trace.before_pop_ds;

        lib_c_printf("PC110:POST:before-pop-ds cr0=%08x cs=%04x/%08x es=%04x/%08x ds=%04x/%08x ss=%04x/%08x esp=%08x eip=%08x gdtr=%04x/%08x/%08x descriptor48-status=%d value=%016llx\n",
            cpu->cr0, cpu->cs.selector, cpu->cs.base,
            cpu->es.selector, cpu->es.base, cpu->ds.selector, cpu->ds.base,
            cpu->ss.selector, cpu->ss.base, cpu->esp, cpu->eip,
            cpu->gdtr.selector, cpu->gdtr.base, cpu->gdtr.limit,
            (int)before_pop_ds_memory_status,
            (unsigned long long)before_pop_ds_descriptor);
    }
    if (c000_checkpoint || c000_follow_checkpoint || c000_port61_follow_checkpoint ||
        c000_port61_return_checkpoint) {
        const lib_size retained = post_reset_trace.c000_retirement_count <
            MYPC110_POST_DIAGNOSTIC_C000_RETIREMENT_CAPACITY ?
            post_reset_trace.c000_retirement_count :
            MYPC110_POST_DIAGNOSTIC_C000_RETIREMENT_CAPACITY;
        const lib_size first = post_reset_trace.c000_retirement_count >
            MYPC110_POST_DIAGNOSTIC_C000_RETIREMENT_CAPACITY ?
            post_reset_trace.c000_retirement_count %
            MYPC110_POST_DIAGNOSTIC_C000_RETIREMENT_CAPACITY : 0u;

        for (lib_size index = 0u; index < retained; ++index) {
            const x86_core_cpu_execution_point *point =
                &post_reset_trace.c000_retirements[(first + index) %
                    MYPC110_POST_DIAGNOSTIC_C000_RETIREMENT_CAPACITY];
            printf("PC110:POST:c000=%04x:%08x bytes=%02x %02x %02x %02x\n",
                point->cs, point->eip, point->bytes[0u], point->bytes[1u],
                point->bytes[2u], point->bytes[3u]);
        }
    }
    if ((c000_follow_checkpoint || c000_port61_follow_checkpoint) &&
        post_reset_trace.left_c000_after_boundary) {
        const x86_core_cpu_execution_point *point = &post_reset_trace.first_after_c000;

        lib_c_printf("PC110:POST:after-c000=%04x:%08x bytes=%02x %02x %02x %02x\n",
            point->cs, point->eip, point->bytes[0u], point->bytes[1u],
            point->bytes[2u], point->bytes[3u]);
    }
    if (c000_port61_follow_checkpoint) {
        const lib_size retained = post_reset_trace.post_c000_port61_read_count;

        for (lib_size index = 0u; index < retained; ++index) {
            const mypc110_post_diagnostic_port_read *read =
                &post_reset_trace.post_c000_port61_reads[index];

            lib_c_printf("PC110:POST:c000-port61-read tick=%llu pc=%08x value=%02x\n",
                (unsigned long long)read->elapsed_ticks, read->linear_pc, read->value);
        }
        lib_c_printf("PC110:POST:c000-port61-first-ecx=%08x\n",
            post_reset_trace.first_post_c000_port61_ecx);
    }
    if (c000_port61_return_checkpoint) {
        lib_c_printf("PC110:POST:c000-port61-first-ecx=%08x after-ret=%04x:%08x\n",
            post_reset_trace.first_post_c000_port61_ecx,
            post_reset_trace.first_after_post_c000_port61_wait.cs,
            post_reset_trace.first_after_post_c000_port61_wait.eip);
    }
    if (post_reset_trace.after_pop_ds_valid) {
        const x86_core_cpu_debug_cpu_snapshot *cpu = &post_reset_trace.after_pop_ds;

        lib_c_printf("PC110:POST:after-second-pop-ds ds=%04x/%08x es=%04x/%08x eip=%08x\n",
            cpu->ds.selector, cpu->ds.base, cpu->es.selector, cpu->es.base, cpu->eip);
    }
    if (reset_exception.valid) {
        lib_c_printf("PC110:POST:reset-cpu cr0=%08x cs=%04x/%08x ss=%04x/%08x ds=%04x/%08x esp=%08x\n",
            reset_cpu.cr0, reset_cpu.cs.selector, reset_cpu.cs.base,
            reset_cpu.ss.selector, reset_cpu.ss.base,
            reset_cpu.ds.selector, reset_cpu.ds.base, reset_cpu.esp);
    }
    if (reset_observation.diagnostic.first_fault.valid) {
        const x86_core_cpu_fault_snapshot *fault =
            &reset_observation.diagnostic.first_fault;

        lib_c_printf("PC110:POST:first-fault=%08x code=%08x point=%04x:%08x stack=%04x:%08x\n",
            fault->exception_mask, fault->exception_code, fault->point.cs,
            fault->point.eip, fault->ss, fault->esp);
    }
    for (lib_size index = 0u; index < reset_observation.diagnostic.recent_count;
        ++index) {
        const x86_core_cpu_execution_point *point =
            &reset_observation.diagnostic.recent[index];

        lib_c_printf("PC110:POST:recent=%04x:%08x bytes=%02x %02x %02x %02x\n",
            point->cs, point->eip, point->bytes[0u], point->bytes[1u],
            point->bytes[2u], point->bytes[3u]);
    }
    if (processor_reset_count != 0u) {
        const lib_size retained = post_reset_trace.pre_reset_retirement_count <
                MYPC110_POST_DIAGNOSTIC_RETIREMENT_CAPACITY ?
            post_reset_trace.pre_reset_retirement_count :
            MYPC110_POST_DIAGNOSTIC_RETIREMENT_CAPACITY;
        const lib_size first = post_reset_trace.pre_reset_retirement_count >
                MYPC110_POST_DIAGNOSTIC_RETIREMENT_CAPACITY ?
            post_reset_trace.pre_reset_retirement_count %
                MYPC110_POST_DIAGNOSTIC_RETIREMENT_CAPACITY : 0u;

        for (lib_size index = 0u; index < retained; ++index) {
            const x86_core_cpu_execution_point *point =
                &post_reset_trace.pre_reset_retirements[(first + index) %
                    MYPC110_POST_DIAGNOSTIC_RETIREMENT_CAPACITY];

            lib_c_printf("PC110:POST:before-reset=%04x:%08x bytes=%02x %02x %02x %02x\n",
                point->cs, point->eip, point->bytes[0u], point->bytes[1u],
                point->bytes[2u], point->bytes[3u]);
        }
    }
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
    for (lib_size index = 0u; index < post_reset_trace.configuration_write_count;
        ++index) {
        const mypc110_post_diagnostic_port_write *write =
            &post_reset_trace.configuration_writes[index];

        lib_c_printf("PC110:POST:port-%s pc=%08x port=%04x value=%02x\n",
            write->type == X86_CORE_MACHINE_TRACE_PORT_READ ? "read" : "write",
            write->linear_pc, write->port, write->value);
    }
    if (port61_follow_checkpoint) {
        const lib_size retained = post_reset_trace.port61_follow_retirement_count <
            MYPC110_POST_DIAGNOSTIC_PORT61_FOLLOW_CAPACITY ?
            post_reset_trace.port61_follow_retirement_count :
            MYPC110_POST_DIAGNOSTIC_PORT61_FOLLOW_CAPACITY;

        for (lib_size index = 0u; index < retained; ++index) {
            const x86_core_cpu_execution_point *point =
                &post_reset_trace.port61_follow_retirements[index];

            lib_c_printf("PC110:POST:port61-follow=%04x:%08x bytes=%02x %02x %02x %02x\n",
                point->cs, point->eip, point->bytes[0u], point->bytes[1u],
                point->bytes[2u], point->bytes[3u]);
        }
    }
    x86_nxvm_machine_destroy(machine);
    if (reset_checkpoint) {
        return status == LIB_STATUS_OK && result.reason == X86_CORE_MACHINE_STOP_BUDGET &&
            processor_reset_count == 1u && c000_header[0u] == 0x55u &&
            c000_header[1u] == 0xaau && !observation.diagnostic.first_fault.valid ? 0 : 1;
    }
    if (port61_progress_checkpoint) {
        return status == LIB_STATUS_OK && processor_reset_count == 1u &&
            post_reset_trace.entered_port61_wait && post_reset_trace.left_port61_wait ? 0 : 1;
    }
    if (port61_follow_checkpoint) {
        return status == LIB_STATUS_OK && processor_reset_count == 1u &&
            post_reset_trace.port61_follow_retirement_count >=
                MYPC110_POST_DIAGNOSTIC_PORT61_FOLLOW_CAPACITY ? 0 : 1;
    }
    if (lpt_cycle_checkpoint) {
        return status == LIB_STATUS_OK && processor_reset_count == 1u &&
            post_reset_trace.entered_lpt_checkpoint &&
            post_reset_trace.returned_to_port61_wait ? 0 : 1;
    }
    if (c000_checkpoint) {
        return status == LIB_STATUS_OK && processor_reset_count == 1u &&
            post_reset_trace.post_reset_c000_entries != 0u &&
            post_reset_trace.crossed_c000_port_boundary &&
            !observation.diagnostic.first_fault.valid ? 0 : 1;
    }
    if (c000_follow_checkpoint) {
        return status == LIB_STATUS_OK && processor_reset_count == 1u &&
            post_reset_trace.crossed_c000_port_boundary &&
            post_reset_trace.left_c000_after_boundary &&
            !observation.diagnostic.first_fault.valid ? 0 : 1;
    }
    if (c000_port61_follow_checkpoint) {
        return status == LIB_STATUS_OK && processor_reset_count == 1u &&
            post_reset_trace.crossed_c000_port_boundary &&
            post_reset_trace.left_c000_after_boundary &&
            post_reset_trace.entered_post_c000_port61_wait &&
            post_reset_trace.post_c000_port61_read_count >=
                MYPC110_POST_DIAGNOSTIC_C000_PORT61_READ_CAPACITY &&
            !observation.diagnostic.first_fault.valid ? 0 : 1;
    }
    if (c000_port61_return_checkpoint) {
        return post_reset_trace.first_post_c000_port61_counter_valid &&
            post_reset_trace.observed_post_c000_port61_return &&
            post_reset_trace.left_post_c000_port61_wait &&
            !observation.diagnostic.first_fault.valid ? 0 : 1;
    }
    return status == LIB_STATUS_OK &&
        result.reason == X86_CORE_MACHINE_STOP_PAUSED &&
        post_reset_trace.before_pop_ds_valid && post_reset_trace.after_pop_ds_valid &&
        post_reset_trace.after_pop_ds.ds.selector == 0x0008u &&
        post_reset_trace.after_pop_ds.ds.base ==
            MYPC110_POST_DIAGNOSTIC_DATA_SELECTOR_BASE ? 0 : 1;
}
