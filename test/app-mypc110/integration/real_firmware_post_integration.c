/* External, embedded-BIOS qualification; earlier observation modes remain
 * available for manual investigation through this same execution harness. */
#include "app-mypc110/pc110_plan_interface.h"
#include "app-mypc110/pc110_cmos.h"
#include "app-mypc110/pc110_firmware_interface.h"
#include "lib/types/file.h"
#include "lib/base/clock_interface.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/debug_interface.h"
#include "x86-core/machine/retirement_observation_interface.h"
#include "x86-core/machine/trace_interface.h"
#include "x86-nxvm/chips/f65535/f65535_interface.h"
#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/display.h"
#include "x86-nxvm/machine/waiting.h"

extern const x86_nxvm_machine_assets vm_app_firmware;

#define MYPC110_POST_DIAGNOSTIC_INSTRUCTIONS 8000000u
#define MYPC110_POST_DIAGNOSTIC_C000_INSTRUCTIONS 12000000u
/* A containment ceiling only: the success condition is the post-C000 wait
 * transition below, never this retirement count. */
#define MYPC110_POST_DIAGNOSTIC_C000_PORT61_FOLLOW_INSTRUCTIONS 13000000u
#define MYPC110_POST_DIAGNOSTIC_QUANTUM 256u
#define MYPC110_POST_DIAGNOSTIC_C000_QUANTUM 4096u
#define MYPC110_POST_DIAGNOSTIC_POWER_QUANTUM 65536u
#define MYPC110_POST_DIAGNOSTIC_PORT_CAPACITY 128u
#define MYPC110_POST_DIAGNOSTIC_C000_PORT61_READ_CAPACITY 16u
#define MYPC110_POST_DIAGNOSTIC_RETIREMENT_CAPACITY 16u
#define MYPC110_POST_DIAGNOSTIC_C000_RETIREMENT_CAPACITY 16u
#define MYPC110_POST_DIAGNOSTIC_PORT61_FOLLOW_CAPACITY 32u
#define MYPC110_POST_DIAGNOSTIC_DATA_SELECTOR_BASE 0x0000d8a0u
#define MYPC110_SETUP_IMAGE_BYTES 232185u
#define MYPC110_SETUP_INPUT_BYTES 4u

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
    lib_bool power_restore_entered;
    lib_bool power_restore_completed;
    lib_bool fdc_checkpoint;
    lib_bool fdc_reset_asserted;
    lib_bool fdc_reset_released;
    lib_bool fdc_wait_returned;
    lib_bool fdc_reset_completed;
    lib_bool timer_checkpoint;
    lib_bool asmi_requested;
    lib_bool asmi_serviced;
    lib_bool smi_returned;
    lib_bool initialized;
    lib_bool copied_handler;
    lib_bool timer_passed;
    lib_bool timer_failed;
    lib_u16 timer_delta;
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
    lib_bool control_trace;
    lib_u8 cmos_trace_index;
    lib_bool cmos_trace_completed;
    lib_bool cmos_restore_check;
    lib_u16 cmos_expected_checksum;
    lib_u8 setup_input_deliveries;
    lib_u8 setup_input_bytes[MYPC110_SETUP_INPUT_BYTES];
    lib_size setup_input_byte_count;
    lib_size setup_cmos_write_count;
    lib_bool setup_postreset_tracking;
    lib_u8 setup_postreset_timer_phase;
    mypc110_post_diagnostic_port_write setup_postreset_last_io;
    mypc110_post_diagnostic_port_write
        configuration_writes[MYPC110_POST_DIAGNOSTIC_PORT_CAPACITY];
    lib_size configuration_write_count;
    lib_size boot_trace_count;
    lib_size inking_trace_count;
    lib_size boot_ata_trace_count;
    lib_u64 boot_ticks;
    lib_u32 boot_irq0_entry;
    lib_u32 boot_irq8_entry;
    lib_u32 boot_irq0_count;
    lib_u32 boot_irq8_count;
    lib_u32 boot_smi_count;
} mypc110_post_diagnostic_trace;

static void mypc110_post_diagnostic_observe_cmos(void *context,
    const x86_core_machine_retirement_observation *observation)
{
    mypc110_post_diagnostic_trace *trace = context;
    if (trace == LIB_NULL || observation == LIB_NULL) return;
    if (observation->point.linear_pc == 0x000f5e73u) {
        lib_c_printf("PC110:CMOS:checksum sum=%04x stored=%04x outcome=%u ticks=%llu\n",
            (lib_u16)observation->current_cpu.edx, (lib_u16)observation->current_cpu.ecx,
            observation->control_outcome, (unsigned long long)observation->timeline_ticks);
        lib_c_fflush(lib_c_stdout);
        if (trace->cmos_restore_check &&
            (lib_u16)observation->current_cpu.edx == trace->cmos_expected_checksum &&
            (lib_u16)observation->current_cpu.ecx == trace->cmos_expected_checksum)
            trace->cmos_trace_completed = LIB_TRUE;
    }
    if (observation->io_port == 0x70u &&
        observation->io_direction == X86_CORE_MACHINE_RETIREMENT_IO_WRITE) {
        trace->cmos_trace_index = (lib_u8)observation->io_value & 0x7fu;
    } else if (observation->io_port == 0x71u && trace->cmos_trace_index == 0x0eu &&
        observation->io_direction == X86_CORE_MACHINE_RETIREMENT_IO_WRITE) {
        lib_c_printf("PC110:CMOS:diag pc=%08x direction=%u value=%02x ticks=%llu\n",
            observation->point.linear_pc, observation->io_direction,
            (lib_u8)observation->io_value, (unsigned long long)observation->timeline_ticks);
        if (!trace->cmos_restore_check && (lib_u8)observation->io_value == 0x40u)
            trace->cmos_trace_completed = LIB_TRUE;
        lib_c_fflush(lib_c_stdout);
    }
}

static void mypc110_post_diagnostic_print_ports(mypc110_post_diagnostic_trace *trace)
{
    lib_c_printf("PC110:POST:port-records=%llu capacity=%u\n",
        (unsigned long long)trace->configuration_write_count,
        MYPC110_POST_DIAGNOSTIC_PORT_CAPACITY);
    for (lib_size index = 0u; index < trace->configuration_write_count; ++index) {
        const mypc110_post_diagnostic_port_write *event = &trace->configuration_writes[index];

        lib_c_printf("PC110:POST:port-%s pc=%08x port=%04x value=%02x\n",
            event->type == X86_CORE_MACHINE_TRACE_PORT_READ ? "read" : "write",
            event->linear_pc, event->port, event->value);
    }
    trace->configuration_write_count = 0u;
    lib_c_fflush(lib_c_stdout);
}

static lib_status mypc110_post_diagnostic_write_frame(x86_nxvm_machine *machine,
    const emulator_machine_driver *driver, const char *path)
{
    kvm_window_frame_storage frame = {0};
    kvm_console_text_frame console = {0};
    lib_bool produced = LIB_FALSE;
    lib_status status = x86_nxvm_machine_publish_display(machine, LIB_TRUE);

    if (status == LIB_STATUS_OK)
        status = driver->copy_frame(driver->context, &frame, &console, &produced);
    lib_c_printf("PC110:SETUP:frame status=%d produced=%u graphics=%u\n",
        status, produced, frame.frame.graphics != LIB_NULL);
    if (status == LIB_STATUS_OK && produced && frame.frame.graphics != LIB_NULL) {
        const kvm_window_graphics_frame *image = frame.frame.graphics;
        lib_c_file *output = lib_c_fopen(path, "wb");

        lib_c_printf("PC110:SETUP:image width=%u height=%u stride=%u\n",
            image->width, image->height, image->stride);
        if (output == LIB_NULL) status = LIB_STATUS_IO_ERROR;
        if (output != LIB_NULL) {
            if (lib_c_fprintf(output, "P6\n%u %u\n255\n", image->width, image->height) < 0)
                status = LIB_STATUS_IO_ERROR;
            for (lib_u32 row = 0u; status == LIB_STATUS_OK && row < image->height; ++row)
                for (lib_u32 column = 0u; column < image->width; ++column) {
                    const lib_u32 pixel = image->pixels[(lib_size)row * image->stride + column];
                    const lib_u8 rgb[3u] = {(lib_u8)(pixel >> 16u),
                        (lib_u8)(pixel >> 8u), (lib_u8)pixel};
                    if (lib_c_fwrite(rgb, 1u, sizeof(rgb), output) != sizeof(rgb)) {
                        status = LIB_STATUS_IO_ERROR;
                        break;
                    }
                }
            if (lib_c_fclose(output) != 0) status = LIB_STATUS_IO_ERROR;
        }
    } else if (status == LIB_STATUS_OK) {
        status = LIB_STATUS_UNSUPPORTED;
    }
    kvm_window_frame_release(&frame);
    kvm_console_frame_release(&console);
    return status;
}

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
        port != 0x00ecu && port != 0x00edu && port != 0x00f9u &&
        port != 0x00fbu && port != 0x0060u &&
        port != 0x0061u && port != 0x0064u && port != 0x03c4u &&
        port != 0x03c5u && port != 0x03e0u && port != 0x03e1u &&
        (trace->setup_input_deliveries == 0u ||
         (port != 0x0070u && port != 0x0071u)) &&
        (!trace->setup_postreset_tracking ||
         (port != 0x0020u && port != 0x0021u &&
          (port < 0x0040u || port > 0x0043u)))) {
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
    if (trace->control_trace) {
        const lib_u16 port = observation->io_port;
        const lib_bool setup_rtc_port = trace->setup_input_deliveries != 0u &&
            (port == 0x0070u || port == 0x0071u);

        if (observation->io_direction != X86_CORE_MACHINE_RETIREMENT_IO_NONE &&
            ((port >= 0x1f1u && port <= 0x1f7u) || port == 0x3f6u) &&
            trace->boot_ata_trace_count < MYPC110_POST_DIAGNOSTIC_PORT_CAPACITY) {
            lib_c_printf("PC110:ATA:post-io pc=%08x port=%04x direction=%u bytes=%u value=%08x\n",
                observation->point.linear_pc, port, observation->io_direction,
                observation->io_bytes, observation->io_value);
            ++trace->boot_ata_trace_count;
        }

        if (setup_rtc_port && port == 0x0070u &&
            observation->io_direction == X86_CORE_MACHINE_RETIREMENT_IO_WRITE)
            trace->cmos_trace_index = (lib_u8)observation->io_value & 0x7fu;
        if (setup_rtc_port && port == 0x0071u &&
            observation->io_direction == X86_CORE_MACHINE_RETIREMENT_IO_WRITE &&
            (trace->cmos_trace_index == 0x2eu || trace->cmos_trace_index == 0x2fu)) {
            lib_c_printf("PC110:SETUP:checksum-write index=%02x value=%02x pc=%08x\n",
                trace->cmos_trace_index, (lib_u8)observation->io_value,
                observation->point.linear_pc);
            lib_c_fflush(lib_c_stdout);
        }

        if (trace->setup_postreset_tracking) {
            if (observation->point.linear_pc == 0x000f68c2u) {
                trace->configuration_write_count = 0u;
            }
            if (observation->point.linear_pc == 0x000f68f4u ||
                observation->point.linear_pc == 0x000f68ffu ||
                observation->point.linear_pc == 0x000feb67u)
                lib_c_printf("PC110:SETUP:postreset-pit-check pc=%08x ax=%04x flags=%08x outcome=%u\n",
                    observation->point.linear_pc, (lib_u16)observation->current_cpu.eax,
                    observation->current_cpu.eflags, observation->control_outcome);
            if (observation->point.linear_pc == 0x000f691eu)
                trace->setup_postreset_timer_phase = 1u;
            if (observation->point.linear_pc == 0x000f692eu)
                trace->setup_postreset_timer_phase = 2u;
            if (observation->point.linear_pc == 0x000f6930u) {
                lib_c_printf("PC110:SETUP:postreset-pit-failure phase=%u ax=%04x bx=%04x cx=%04x\n",
                    trace->setup_postreset_timer_phase,
                    (lib_u16)observation->current_cpu.eax,
                    (lib_u16)observation->current_cpu.ebx,
                    (lib_u16)observation->current_cpu.ecx);
                mypc110_post_diagnostic_print_ports(trace);
            }
            if (observation->point.linear_pc == 0x000f6981u) {
                lib_c_printf("PC110:SETUP:postreset-timer ax=%04x bx=%04x\n",
                    (lib_u16)observation->current_cpu.eax,
                    (lib_u16)observation->current_cpu.ebx);
            }
            if (observation->point.linear_pc == 0x000f698fu)
                trace->timer_passed = LIB_TRUE;
            index = trace->pre_reset_retirement_count %
                MYPC110_POST_DIAGNOSTIC_RETIREMENT_CAPACITY;
            trace->pre_reset_retirements[index] = observation->point;
            ++trace->pre_reset_retirement_count;
            if (observation->io_direction != X86_CORE_MACHINE_RETIREMENT_IO_NONE)
                trace->setup_postreset_last_io = (mypc110_post_diagnostic_port_write){
                    observation->point.linear_pc,
                    observation->io_direction == X86_CORE_MACHINE_RETIREMENT_IO_READ ?
                        X86_CORE_MACHINE_TRACE_PORT_READ : X86_CORE_MACHINE_TRACE_PORT_WRITE,
                    port, (lib_u8)observation->io_value};
        }

        if ((observation->io_direction == X86_CORE_MACHINE_RETIREMENT_IO_READ &&
                (port == 0x0060u || port == 0x0092u || port == 0x03e0u ||
                 port == 0x03e1u)) ||
            (observation->io_direction == X86_CORE_MACHINE_RETIREMENT_IO_WRITE &&
                (port == 0x0060u || port == 0x0064u || port == 0x0092u ||
                 port == 0x03e0u || port == 0x03e1u)) || setup_rtc_port ||
            (trace->setup_postreset_tracking &&
             (observation->io_direction == X86_CORE_MACHINE_RETIREMENT_IO_READ ||
              observation->io_direction == X86_CORE_MACHINE_RETIREMENT_IO_WRITE) &&
             (port == 0x0020u || port == 0x0021u ||
              (port >= 0x0040u && port <= 0x0043u)))) {
            if (trace->configuration_write_count < MYPC110_POST_DIAGNOSTIC_PORT_CAPACITY)
                trace->configuration_writes[trace->configuration_write_count++] =
                    (mypc110_post_diagnostic_port_write){observation->point.linear_pc,
                        observation->io_direction == X86_CORE_MACHINE_RETIREMENT_IO_READ ?
                            X86_CORE_MACHINE_TRACE_PORT_READ : X86_CORE_MACHINE_TRACE_PORT_WRITE,
                        port, (lib_u8)observation->io_value};
        }
        if (trace->setup_input_deliveries != 0u &&
            observation->io_direction == X86_CORE_MACHINE_RETIREMENT_IO_READ &&
            port == 0x0060u && trace->setup_input_byte_count < MYPC110_SETUP_INPUT_BYTES) {
            trace->setup_input_bytes[trace->setup_input_byte_count++] =
                (lib_u8)observation->io_value;
        }
        if (trace->setup_input_deliveries != 0u &&
            observation->io_direction == X86_CORE_MACHINE_RETIREMENT_IO_WRITE &&
            (port == 0x0070u || port == 0x0071u)) {
            ++trace->setup_cmos_write_count;
        }
        return;
    }
    if (trace->timer_checkpoint) {
        if (trace->fdc_checkpoint && trace->timer_passed) {
            if (observation->io_direction == X86_CORE_MACHINE_RETIREMENT_IO_WRITE &&
                observation->io_port == 0x03f2u) {
                if (observation->io_value == 0x08u) trace->fdc_reset_asserted = LIB_TRUE;
                if (trace->fdc_reset_asserted && observation->io_value == 0x0cu)
                    trace->fdc_reset_released = LIB_TRUE;
            }
            if (trace->fdc_reset_released && observation->point.linear_pc == 0x000fc971u)
                trace->fdc_wait_returned = LIB_TRUE;
            if (trace->fdc_wait_returned && observation->point.linear_pc == 0x000fbd07u)
                trace->fdc_reset_completed = LIB_TRUE;
        }
        if (trace->initialized && observation->io_port == 0x15eeu &&
            observation->io_direction == X86_CORE_MACHINE_RETIREMENT_IO_WRITE)
            trace->asmi_requested = LIB_TRUE;
        if (trace->asmi_requested && observation->point.linear_pc == 0x0003a774u)
            trace->asmi_serviced = LIB_TRUE;
        if (trace->asmi_serviced && observation->point.linear_pc == 0x0003dbc8u)
            trace->smi_returned = LIB_TRUE;
        if (observation->point.linear_pc == 0x000e912fu &&
            observation->current_cpu.ecx == 0u) trace->copied_handler = LIB_TRUE;
        if (observation->point.linear_pc == 0x000f3457u)
            trace->initialized = LIB_TRUE;
        if (observation->point.linear_pc == 0x000f6981u)
            trace->timer_delta = (lib_u16)observation->current_cpu.ebx;
        if (observation->point.linear_pc == 0x000f698fu)
            trace->timer_passed = LIB_TRUE;
        if (observation->point.linear_pc == 0x000f6999u)
            trace->timer_failed = LIB_TRUE;
        return;
    }
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

static void mypc110_post_diagnostic_boot_progress(
    x86_core_machine *machine, lib_u64 executed)
{
    x86_core_machine_observation observation = {0};
    lib_u64 milliseconds = 0u;
    const lib_status clock = base_clock_milliseconds(&milliseconds);
    const lib_status capture = x86_core_machine_capture_observation(machine,
        &observation);

    lib_c_printf("PC110:BOOT:progress clock=%d host-ms=%llu capture=%d executed=%llu ticks=%llu pc=%04x:%08x exceptions=%u\n",
        clock, (unsigned long long)milliseconds, capture,
        (unsigned long long)executed, (unsigned long long)observation.elapsed_ticks,
        observation.cpu.cs, observation.cpu.eip,
        observation.diagnostic.delivered_exception_count);
    lib_c_fflush(lib_c_stdout);
}

static void mypc110_post_diagnostic_observe_boot(void *context,
    const x86_core_machine_retirement_observation *observation)
{
    mypc110_post_diagnostic_trace *trace = context;
    const lib_u16 port = observation->io_port;

    trace->boot_ticks = observation->timeline_ticks;
    if (observation->point.linear_pc == trace->boot_irq0_entry) ++trace->boot_irq0_count;
    if (observation->point.linear_pc == trace->boot_irq8_entry) ++trace->boot_irq8_count;
    if (observation->point.linear_pc == 0x38000u) ++trace->boot_smi_count;
    if (trace->inking_trace_count < 64u &&
        ((observation->io_direction != X86_CORE_MACHINE_RETIREMENT_IO_NONE &&
          (port == 0x05e0u || port == 0x05e1u || port == 0x05e2u ||
           port == 0x15e0u || port == 0x15e1u || port == 0x15e2u ||
           port == 0x25e0u || port == 0x25e1u || port == 0x25e2u ||
           port == 0x35e0u || port == 0x35e1u || port == 0x35e2u)) ||
         (observation->point.bytes[0u] == 0xcdu && observation->point.bytes[1u] == 0x15u &&
          (lib_u16)observation->instruction_entry_cpu.eax == 0x5380u))) {
        lib_c_printf("PC110:INK:pc=%08x port=%04x dir=%u value=%08x ax=%04x bx=%04x cx=%04x\n",
            observation->point.linear_pc, port, observation->io_direction,
            observation->io_value, (lib_u16)observation->instruction_entry_cpu.eax,
            (lib_u16)observation->instruction_entry_cpu.ebx,
            (lib_u16)observation->instruction_entry_cpu.ecx);
        ++trace->inking_trace_count;
    }
    if (observation->io_direction != X86_CORE_MACHINE_RETIREMENT_IO_NONE &&
        ((port >= 0x1f0u && port <= 0x1f7u) || port == 0x3f6u)) {
        if (trace->boot_ata_trace_count < MYPC110_POST_DIAGNOSTIC_PORT_CAPACITY) {
            lib_c_printf("PC110:ATA:io pc=%08x port=%04x direction=%u bytes=%u value=%08x\n",
                observation->point.linear_pc, port, observation->io_direction,
                observation->io_bytes, observation->io_value);
            ++trace->boot_ata_trace_count;
        }
        return;
    }
    if (trace->boot_trace_count >= MYPC110_POST_DIAGNOSTIC_PORT_CAPACITY)
        return;
    if (observation->point.bytes[0u] == 0xcdu &&
        observation->point.bytes[1u] == 0x15u) {
        lib_c_printf("PC110:ATA:int15 pc=%08x ax=%04x bx=%04x cx=%04x dx=%04x\n",
            observation->point.linear_pc,
            (lib_u16)observation->instruction_entry_cpu.eax,
            (lib_u16)observation->instruction_entry_cpu.ebx,
            (lib_u16)observation->instruction_entry_cpu.ecx,
            (lib_u16)observation->instruction_entry_cpu.edx);
        ++trace->boot_trace_count;
    } else if (observation->io_direction != X86_CORE_MACHINE_RETIREMENT_IO_NONE &&
        (port == 0x3e0u || port == 0x3e1u || port == 0x8bu)) {
        lib_c_printf("PC110:ATA:io pc=%08x port=%04x direction=%u bytes=%u value=%08x\n",
            observation->point.linear_pc, port, observation->io_direction,
            observation->io_bytes, observation->io_value);
        ++trace->boot_trace_count;
    }
}

int main(int argc, char **argv)
{
    const lib_bool cmos_restore_checkpoint = argc == 3 && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--restore-cmos") == 0;
    const lib_bool cmos_origin_checkpoint = cmos_restore_checkpoint ||
        (argc == 3 && argv != LIB_NULL &&
         lib_text_compare(argv[1], "--cmos-checksum") == 0);
    const lib_bool os_ps2_checkpoint = argc == 6 && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--setup-pair-os-ps2") == 0;
    const lib_bool os_inking_checkpoint = os_ps2_checkpoint ||
        (argc == 6 && argv != LIB_NULL &&
         lib_text_compare(argv[1], "--setup-pair-os-inking") == 0);
    const lib_bool os_late_key_checkpoint = os_inking_checkpoint ||
        (argc == 6 && argv != LIB_NULL &&
         lib_text_compare(argv[1], "--setup-pair-os-late-key") == 0);
    const lib_bool os_key_checkpoint = os_late_key_checkpoint ||
        (argc == 6 && argv != LIB_NULL &&
         lib_text_compare(argv[1], "--setup-pair-os-key") == 0);
    const lib_bool os_storm_checkpoint = os_key_checkpoint ||
        (argc == 6 && argv != LIB_NULL &&
         lib_text_compare(argv[1], "--setup-pair-os-storm") == 0);
    const lib_bool os_exception_checkpoint = os_storm_checkpoint ||
        (argc == 6 && argv != LIB_NULL &&
         lib_text_compare(argv[1], "--setup-pair-os-exception") == 0);
    const lib_bool os_checkpoint = os_exception_checkpoint ||
        (argc == 6 && argv != LIB_NULL &&
         lib_text_compare(argv[1], "--setup-pair-os-progress") == 0);
    const lib_bool loader_checkpoint = os_checkpoint ||
        (argc == 6 && argv != LIB_NULL &&
         lib_text_compare(argv[1], "--setup-pair-system-loader") == 0);
    const lib_bool system_files_checkpoint = loader_checkpoint ||
        (argc == 6 && argv != LIB_NULL &&
         lib_text_compare(argv[1], "--setup-pair-system-files") == 0);
    const lib_bool partition_checkpoint = system_files_checkpoint ||
        (argc == 6 && argv != LIB_NULL &&
         lib_text_compare(argv[1], "--setup-pair-partition-boot") == 0);
    const lib_bool setup_pair_master_checkpoint = partition_checkpoint ||
        (argc == 6 && argv != LIB_NULL &&
         lib_text_compare(argv[1], "--setup-pair-master-boot") == 0);
    const lib_bool setup_pair_checkpoint = setup_pair_master_checkpoint ||
        (argc == 6 && argv != LIB_NULL &&
         lib_text_compare(argv[1], "--setup-pair-boot") == 0);
    const lib_bool setup_master_checkpoint = argc == 5 && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--setup-master-boot") == 0;
    const lib_bool setup_slave_checkpoint = setup_pair_checkpoint ||
        (argc == 5 && argv != LIB_NULL &&
         lib_text_compare(argv[1], "--setup-slave-boot") == 0);
    const lib_bool setup_media_checkpoint = setup_slave_checkpoint || setup_master_checkpoint;
    const lib_bool setup_resume_checkpoint = setup_media_checkpoint ||
        ((argc == 4 || argc == 5) && argv != LIB_NULL &&
         lib_text_compare(argv[1], "--setup-postreset") == 0);
    const lib_bool setup_checkpoint = (cmos_origin_checkpoint && !cmos_restore_checkpoint) || setup_resume_checkpoint ||
        ((argc == 3 || argc == 4) && argv != LIB_NULL &&
         lib_text_compare(argv[1], "--setup") == 0);
    const lib_bool ata_slave_checkpoint = setup_slave_checkpoint ||
        (argc == 3 && argv != LIB_NULL &&
         lib_text_compare(argv[1], "--ata-slave-boot") == 0);
    const lib_bool ata_checkpoint = ata_slave_checkpoint || setup_master_checkpoint || (argc == 3 && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--ata-boot") == 0);
    const x86_nxvm_machine_config config = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80486SX,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .bios_count = 1u,
        .fixed_disk_image = {setup_pair_checkpoint ? argv[5] :
            ata_checkpoint && !ata_slave_checkpoint ? argv[2] : LIB_NULL,
            ata_slave_checkpoint ? argv[2] : LIB_NULL},
        .fixed_disk_mode = {LIB_STORAGE_MEDIUM_OVERLAY, LIB_STORAGE_MEDIUM_OVERLAY}};
    const char *setup_reference_path = setup_media_checkpoint ? argv[3] :
        setup_checkpoint ? argv[2] : LIB_NULL;
    const char *setup_frame_path = setup_media_checkpoint ? argv[4] :
        setup_checkpoint && argc >= 4 ? argv[3] : LIB_NULL;
    x86_nxvm_machine_construction construction = {0};
    x86_nxvm_machine_assets selected_firmware = vm_app_firmware;
    lib_u8 restored_cmos[X86_NXVM_MACHINE_EXTENDED_CMOS_SEED_BYTES];
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
    lib_bool fdc_checkpoint = argc == 2 && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--fdc-reset") == 0;
    lib_bool boot_checkpoint = setup_checkpoint || ata_checkpoint || (argc == 2 && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--resume-boot") == 0);
    lib_bool power_checkpoint = boot_checkpoint || (argc == 2 && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--resume-button") == 0);
    lib_bool boot_handoff = LIB_FALSE;
    lib_bool ata_boot_sector = LIB_FALSE;
    lib_bool partition_boot_sector = LIB_FALSE;
    lib_bool system_files_reached = LIB_FALSE;
    lib_bool loader_reached = LIB_FALSE;
    lib_bool os_exception_logged = LIB_FALSE;
    lib_bool os_key_sent = LIB_FALSE;
    lib_u8 os_inking_events = 0u;
    lib_u64 os_inking_next = 0u;
    lib_bool os_final_frame_written = LIB_FALSE;
    lib_u8 os_pointer_phase = 0u;
    lib_bool setup_entry = LIB_FALSE;
    lib_bool setup_bytes_match = LIB_FALSE;
    lib_bool setup_reset_completed = LIB_FALSE;
    lib_bool setup_postreset_handoff = LIB_FALSE;
    lib_u8 setup_input_count = 0u;
    static lib_u8 setup_expected[MYPC110_SETUP_IMAGE_BYTES];
    static lib_u8 setup_actual[MYPC110_SETUP_IMAGE_BYTES];
    lib_u8 expected_sector[512u] = {0};
    lib_u8 expected_partition[512u] = {0};
    lib_u8 expected_loader[1536u] = {0};
    lib_u64 boot_report = 0u;
    lib_bool button_sent = LIB_FALSE;
    lib_bool setup_wake_sent = LIB_FALSE;
    lib_bool setup_wake_pending = LIB_FALSE;
    lib_u8 suspended_cpu_image[512u] = {0};
    lib_u8 restored_cpu_image[512u] = {0};
    lib_bool retained_cpu_image = LIB_FALSE;
    mypc110_firmware *power_firmware = LIB_NULL;
    emulator_machine_driver power_driver = {0};
    lib_bool timer_checkpoint = power_checkpoint || fdc_checkpoint || (argc == 2 && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--timer") == 0);
    lib_u64 instruction_limit;
    lib_u32 wait_count = 0u;
    lib_status status;

    if (argc != 1 && !reset_checkpoint && !port61_progress_checkpoint &&
        !port61_follow_checkpoint && !lpt_cycle_checkpoint &&
        !c000_checkpoint && !c000_follow_checkpoint && !c000_port61_follow_checkpoint &&
        !timer_checkpoint && !cmos_restore_checkpoint) return 2;
    instruction_limit = c000_port61_follow_checkpoint ?
        MYPC110_POST_DIAGNOSTIC_C000_PORT61_FOLLOW_INSTRUCTIONS :
        (c000_checkpoint || c000_follow_checkpoint ?
            MYPC110_POST_DIAGNOSTIC_C000_INSTRUCTIONS :
            MYPC110_POST_DIAGNOSTIC_INSTRUCTIONS);
    if (timer_checkpoint) instruction_limit = 200000000u;
    if (fdc_checkpoint) instruction_limit = 32000000u;
    if (power_checkpoint) instruction_limit = 60000000u;
    if (boot_checkpoint) instruction_limit = 100000000u;
    if (setup_checkpoint) instruction_limit = 120000000u;
    post_reset_trace.fdc_checkpoint = fdc_checkpoint;
    post_reset_trace.control_trace = setup_checkpoint;
    post_reset_trace.timer_checkpoint = timer_checkpoint;
    post_reset_trace.c000_only = c000_checkpoint || c000_follow_checkpoint ||
        c000_port61_follow_checkpoint;
    post_reset_trace.follow_c000_port61 = c000_port61_follow_checkpoint;

    status = LIB_STATUS_OK;
    if (cmos_restore_checkpoint) {
        lib_bool present = LIB_FALSE;
        status = mypc110_cmos_load(argv[2], restored_cmos, &present);
        if (status == LIB_STATUS_OK && !present) status = LIB_STATUS_INVALID_STATE;
        if (status == LIB_STATUS_OK) {
            selected_firmware.cmos_seed = (x86_nxvm_machine_asset_bytes){
                restored_cmos, sizeof(restored_cmos)};
            post_reset_trace.cmos_restore_check = LIB_TRUE;
            post_reset_trace.cmos_expected_checksum = (lib_u16)(
                ((lib_u16)restored_cmos[0x2eu] << 8u) | restored_cmos[0x2fu]);
        }
    }
    if (ata_checkpoint) {
        lib_storage_medium *medium = LIB_NULL;
        status = lib_storage_medium_open(setup_pair_master_checkpoint ? argv[5] : argv[2],
            LIB_STORAGE_MEDIUM_READONLY, &medium);
        if (status == LIB_STATUS_OK)
            status = lib_storage_medium_read_at(medium, 0u, expected_sector,
                sizeof(expected_sector));
        if (status == LIB_STATUS_OK && partition_checkpoint) {
            lib_u32 active_count = 0u;
            lib_u32 partition_lba = 0u;
            for (lib_u32 i = 0u; i < 4u; ++i) {
                const lib_u8 *entry = &expected_sector[446u + i * 16u];
                if (entry[0u] == 0x80u) {
                    ++active_count;
                    partition_lba = (lib_u32)entry[8u] | ((lib_u32)entry[9u] << 8u) |
                        ((lib_u32)entry[10u] << 16u) | ((lib_u32)entry[11u] << 24u);
                }
            }
            if (expected_sector[510u] != 0x55u || expected_sector[511u] != 0xaau ||
                active_count != 1u || partition_lba == 0u ||
                (lib_u64)partition_lba * 512u + 512u > lib_storage_medium_byte_count(medium))
                status = LIB_STATUS_INVALID_ARGUMENT;
            else
                status = lib_storage_medium_read_at(medium, (lib_u64)partition_lba * 512u,
                    expected_partition, sizeof(expected_partition));
            if (status == LIB_STATUS_OK &&
                (expected_partition[510u] != 0x55u || expected_partition[511u] != 0xaau))
                status = LIB_STATUS_INVALID_ARGUMENT;
            if (status == LIB_STATUS_OK && loader_checkpoint) {
                const lib_u32 bytes_per_sector = (lib_u32)expected_partition[11u] |
                    ((lib_u32)expected_partition[12u] << 8u);
                const lib_u32 sectors_per_cluster = expected_partition[13u];
                const lib_u32 reserved = (lib_u32)expected_partition[14u] |
                    ((lib_u32)expected_partition[15u] << 8u);
                const lib_u32 root_entries = (lib_u32)expected_partition[17u] |
                    ((lib_u32)expected_partition[18u] << 8u);
                const lib_u32 fat_sectors = (lib_u32)expected_partition[22u] |
                    ((lib_u32)expected_partition[23u] << 8u);
                const lib_u64 root_lba = (lib_u64)partition_lba + reserved +
                    expected_partition[16u] * fat_sectors;
                lib_u8 root_entry[32u] = {0};
                if (bytes_per_sector != 512u || sectors_per_cluster == 0u ||
                    root_entries == 0u || (lib_u64)root_lba * 512u + sizeof(root_entry) >
                        lib_storage_medium_byte_count(medium))
                    status = LIB_STATUS_INVALID_ARGUMENT;
                else
                    status = lib_storage_medium_read_at(medium, (lib_u64)root_lba * 512u,
                        root_entry, sizeof(root_entry));
                const lib_u32 cluster = (lib_u32)root_entry[26u] |
                    ((lib_u32)root_entry[27u] << 8u);
                const lib_u64 loader_lba = (lib_u64)root_lba +
                    ((lib_u64)root_entries * 32u + 511u) / 512u +
                    (cluster >= 2u ? (lib_u64)(cluster - 2u) * sectors_per_cluster : 0u);
                if (status == LIB_STATUS_OK && (cluster < 2u ||
                    lib_memory_compare(root_entry, "IBMBIO  COM", 11u) != 0 ||
                    loader_lba * 512u + sizeof(expected_loader) > lib_storage_medium_byte_count(medium)))
                    status = LIB_STATUS_INVALID_ARGUMENT;
                if (status == LIB_STATUS_OK)
                    status = lib_storage_medium_read_at(medium, loader_lba * 512u,
                        expected_loader, sizeof(expected_loader));
            }
        }
        const lib_status closed = lib_storage_medium_destroy(&medium);
        if (status == LIB_STATUS_OK) status = closed;
    }
    if (setup_checkpoint && status == LIB_STATUS_OK) {
        lib_storage_medium *reference = LIB_NULL;
        status = lib_storage_medium_open(setup_reference_path, LIB_STORAGE_MEDIUM_READONLY, &reference);
        if (status == LIB_STATUS_OK &&
            lib_storage_medium_byte_count(reference) != sizeof(setup_expected))
            status = LIB_STATUS_INVALID_ARGUMENT;
        if (status == LIB_STATUS_OK)
            status = lib_storage_medium_read_at(reference, 0u, setup_expected,
                sizeof(setup_expected));
        const lib_status closed = lib_storage_medium_destroy(&reference);
        if (status == LIB_STATUS_OK) status = closed;
    }
    if (status == LIB_STATUS_OK)
        status = mypc110_machine_construction_create(&config, &selected_firmware, &construction);
    if (status == LIB_STATUS_OK) power_firmware = construction.firmware_context;
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_machine_create(&config, &construction, &machine);
    if (status == LIB_STATUS_OK && cmos_origin_checkpoint)
        status = x86_core_machine_set_retirement_observation_provider(
            machine->x86_core_machine,
            &(x86_core_machine_retirement_observation_provider){
                .callback = mypc110_post_diagnostic_observe_cmos,
                .context = &post_reset_trace});
    if (status == LIB_STATUS_OK && power_checkpoint)
        status = x86_nxvm_machine_describe_emulator_driver(machine, &power_driver);
    if (status == LIB_STATUS_OK && (!power_checkpoint || setup_media_checkpoint) && !cmos_origin_checkpoint) {
        status = x86_core_machine_set_retirement_observation_provider(
            machine->x86_core_machine,
            &(x86_core_machine_retirement_observation_provider){
                .callback = mypc110_post_diagnostic_observe_retirement,
                .context = &post_reset_trace});
    }
    if (status == LIB_STATUS_OK && !timer_checkpoint && !setup_checkpoint) {
        status = x86_core_machine_set_trace_provider(machine->x86_core_machine,
            &(x86_core_machine_trace_provider) {
                .callback = mypc110_post_diagnostic_observe_trace,
                .context = &post_reset_trace});
    }
    if (status == LIB_STATUS_OK && !timer_checkpoint && !reset_checkpoint && !port61_progress_checkpoint &&
        !port61_follow_checkpoint && !lpt_cycle_checkpoint && !c000_checkpoint &&
        !c000_follow_checkpoint && !c000_port61_follow_checkpoint && !cmos_restore_checkpoint) {
        status = x86_core_machine_debug_set_watchpoint(machine->x86_core_machine,
            X86_CORE_CPU_DEBUG_WATCH_EXECUTE,
            0x000f6060u);
    }
    if (status == LIB_STATUS_OK) {
        x86_nxvm_machine_executor_state_start(&machine->control.state);
    }
    if (boot_checkpoint && !setup_checkpoint && status == LIB_STATUS_OK)
        mypc110_post_diagnostic_boot_progress(machine->x86_core_machine, executed);
    while (status == LIB_STATUS_OK && executed < instruction_limit) {
        status = x86_core_machine_run(machine->x86_core_machine,
            (x86_core_machine_run_budget) {.instructions =
                power_checkpoint ? MYPC110_POST_DIAGNOSTIC_POWER_QUANTUM :
                timer_checkpoint ? MYPC110_POST_DIAGNOSTIC_C000_QUANTUM :
                processor_reset_count != 0u && !post_reset_exception.valid &&
                    !port61_progress_checkpoint && !port61_follow_checkpoint &&
                    !lpt_cycle_checkpoint && !c000_checkpoint &&
                    !c000_follow_checkpoint && !c000_port61_follow_checkpoint ? 1u :
                c000_checkpoint || c000_follow_checkpoint || c000_port61_follow_checkpoint ?
                    MYPC110_POST_DIAGNOSTIC_C000_QUANTUM :
                MYPC110_POST_DIAGNOSTIC_QUANTUM,
                .ticks = 0u}, &result);
        executed += result.executed;
        if (cmos_origin_checkpoint && post_reset_trace.cmos_trace_completed) {
            lib_c_printf("PC110:CMOS:origin-observed executed=%llu\n",
                (unsigned long long)executed);
            break;
        }
        if (status == LIB_STATUS_OK && result.reason == X86_CORE_MACHINE_STOP_BUDGET &&
            setup_entry && argc >= 4 && setup_input_count < 2u && executed >= instruction_limit) {
            char before_path[512u];
            const lib_i32 length = lib_c_snprintf(before_path, sizeof(before_path),
                "%s.before-%u.ppm", setup_frame_path, (unsigned int)setup_input_count);

            if (length < 0 || (lib_size)length >= sizeof(before_path)) {
                status = LIB_STATUS_LIMIT_EXCEEDED;
                break;
            }
            status = mypc110_post_diagnostic_write_frame(machine, &power_driver, before_path);
            mypc110_post_diagnostic_print_ports(&post_reset_trace);
            if (status != LIB_STATUS_OK) break;
            kvm_input_event key = {.type = KVM_EVENT_KEY,
                .data.key = {.key = BASE_KEY_ENTER, .pressed = LIB_TRUE}};
            post_reset_trace.setup_input_deliveries = setup_input_count + 1u;
            power_driver.deliver_input(power_driver.context, &key);
            key.data.key.pressed = LIB_FALSE;
            power_driver.deliver_input(power_driver.context, &key);
            ++setup_input_count;
            instruction_limit = executed + 10000000u;
            lib_c_printf("PC110:SETUP:enter-delivered count=%u executed=%llu\n",
                (unsigned int)setup_input_count, (unsigned long long)executed);
        }
        if (boot_checkpoint && executed >= boot_report) {
            mypc110_post_diagnostic_boot_progress(machine->x86_core_machine, executed);
            boot_report = executed + 1000000u;
        }
        if (power_checkpoint && result.reason == X86_CORE_MACHINE_STOP_PAUSED && button_sent) {
            const lib_u32 expected = setup_wake_pending ?
                (post_reset_trace.power_restore_entered ? 0x3dbc5u : 0x39336u) :
                boot_handoff && ata_checkpoint ?
                (loader_checkpoint && system_files_reached ? 0x700u :
                 system_files_checkpoint && partition_boot_sector ? 0x7d05u : 0x7c00u) :
                setup_resume_checkpoint && setup_reset_completed ?
                0xf52bdu : setup_checkpoint && post_reset_trace.power_restore_completed ?
                0x9009eu :
                post_reset_trace.power_restore_completed ? 0xf52bdu :
                post_reset_trace.power_restore_entered ? 0x3dbc5u : 0x39336u;
            x86_core_cpu_debug_instruction_observation retired = {0};
            status = x86_core_machine_debug_capture_instruction_observation(
                machine->x86_core_machine, &retired);
            if (status != LIB_STATUS_OK || !retired.watch_hit || retired.watch_address != expected) break;
            status = x86_core_machine_debug_clear_watchpoint(machine->x86_core_machine,
                X86_CORE_CPU_DEBUG_WATCH_EXECUTE);
            if (status == LIB_STATUS_INVALID_STATE) {
                /* The watch hit may precede its physical retirement tail.
                 * One normal step must finish that tail, not execute new code. */
                x86_core_cpu_debug_cpu_snapshot before_tail = {0};
                x86_core_cpu_debug_cpu_snapshot after_tail = {0};
                status = x86_core_machine_debug_capture_cpu_snapshot(machine->x86_core_machine,
                    X86_CORE_CPU_SNAPSHOT_CURRENT, &before_tail);
                if (status == LIB_STATUS_OK)
                    status = x86_core_machine_debug_step(machine->x86_core_machine, &result);
                if (status == LIB_STATUS_OK)
                    status = x86_core_machine_debug_capture_cpu_snapshot(machine->x86_core_machine,
                        X86_CORE_CPU_SNAPSHOT_CURRENT, &after_tail);
                if (status != LIB_STATUS_OK || result.executed != 1u ||
                    before_tail.cs.base != after_tail.cs.base || before_tail.eip != after_tail.eip) break;
                status = x86_core_machine_debug_clear_watchpoint(machine->x86_core_machine,
                    X86_CORE_CPU_DEBUG_WATCH_EXECUTE);
            }
            if (status != LIB_STATUS_OK) break;
            lib_c_printf("PC110:POWER:retired-watch=%08x pc=%08x\n", expected, result.linear_pc);
            if (setup_resume_checkpoint && setup_reset_completed && !setup_wake_pending &&
                !boot_handoff) {
                setup_postreset_handoff = retired.instruction_linear == 0xf52bdu;
                lib_c_printf("PC110:SETUP:postreset-int19=%u target=%04x:%08x\n",
                    setup_postreset_handoff, retired.cs, retired.eip);
                if (!setup_media_checkpoint || !setup_postreset_handoff) break;
            }
            if (setup_checkpoint && post_reset_trace.power_restore_completed &&
                !setup_postreset_handoff) {
                setup_entry = retired.instruction_linear == 0x9009eu &&
                    retired.cs == 0x5000u && retired.eip == 0u;
                status = x86_core_machine_memory_read(machine->x86_core_machine,
                    0x50000u, setup_actual, sizeof(setup_actual));
                setup_bytes_match = status == LIB_STATUS_OK && lib_memory_compare(
                    setup_actual, setup_expected, sizeof(setup_actual)) == 0;
                lib_c_printf("PC110:SETUP:entry=%u target=%04x:%08x bytes=%u matched=%u\n",
                    setup_entry, retired.cs, retired.eip,
                    MYPC110_SETUP_IMAGE_BYTES, setup_bytes_match);
                if (argc >= 4 && setup_entry && setup_bytes_match) {
                    if (ata_checkpoint) post_reset_trace.configuration_write_count = 0u;
                    status = x86_core_machine_set_retirement_observation_provider(
                        machine->x86_core_machine,
                        &(x86_core_machine_retirement_observation_provider){
                            .callback = mypc110_post_diagnostic_observe_retirement,
                            .context = &post_reset_trace});
                    if (status == LIB_STATUS_OK)
                        status = x86_core_machine_set_trace_provider(machine->x86_core_machine,
                            &(x86_core_machine_trace_provider) {
                                .callback = mypc110_post_diagnostic_observe_trace,
                                .context = &post_reset_trace});
                    if (status != LIB_STATUS_OK) break;
                    instruction_limit = executed + 20000000u;
                    continue;
                }
                break;
            }
            if (boot_handoff) {
                lib_u8 actual_sector[512u] = {0};
                if (loader_checkpoint && system_files_reached) {
                    lib_u8 actual_loader[sizeof(expected_loader)] = {0};
                    status = x86_core_machine_memory_read(machine->x86_core_machine,
                        0x700u, actual_loader, sizeof(actual_loader));
                    loader_reached = status == LIB_STATUS_OK && retired.instruction_linear == 0x700u &&
                        lib_memory_compare(actual_loader, expected_loader, sizeof(actual_loader)) == 0;
                    lib_c_printf("PC110:ATA:system-loader=%u target=%04x:%08x\n",
                        loader_reached, retired.cs, retired.eip);
                    if (os_checkpoint && loader_reached) {
                        if (os_inking_checkpoint) {
                            /* The loader is verified. The Core fault provider
                             * still records delivered exceptions while the
                             * per-retirement observer is absent. Restore it
                             * before driver input to observe actual port I/O. */
                            status = x86_core_machine_set_retirement_observation_provider(
                                machine->x86_core_machine, LIB_NULL);
                            if (status != LIB_STATUS_OK) break;
                            lib_c_printf("PC110:INK:retirement-observation=off\n");
                        }
                        instruction_limit = executed + 400000000u;
                        lib_c_printf("PC110:OS:continue=%llu limit=%llu\n",
                            (unsigned long long)executed,
                            (unsigned long long)instruction_limit);
                        continue;
                    }
                    break;
                }
                if (system_files_checkpoint && partition_boot_sector) {
                    system_files_reached = retired.instruction_linear == 0x7d05u;
                    lib_c_printf("PC110:ATA:system-files=%u target=%04x:%08x\n",
                        system_files_reached, retired.cs, retired.eip);
                    if (loader_checkpoint && system_files_reached) {
                        status = x86_core_machine_debug_set_watchpoint(machine->x86_core_machine,
                            X86_CORE_CPU_DEBUG_WATCH_EXECUTE, 0x700u);
                        instruction_limit = executed + 30000000u;
                        continue;
                    }
                    break;
                }
                status = x86_core_machine_memory_read(machine->x86_core_machine,
                    0x7c00u, actual_sector, sizeof(actual_sector));
                if (partition_checkpoint && ata_boot_sector) {
                    partition_boot_sector = status == LIB_STATUS_OK &&
                        retired.instruction_linear == 0x7c00u &&
                        lib_memory_compare(actual_sector, expected_partition,
                            sizeof(actual_sector)) == 0;
                    lib_c_printf("PC110:ATA:partition-sector=%u target=%04x:%08x\n",
                        partition_boot_sector, retired.cs, retired.eip);
                    if (system_files_checkpoint && partition_boot_sector) {
                        status = x86_core_machine_debug_set_watchpoint(machine->x86_core_machine,
                            X86_CORE_CPU_DEBUG_WATCH_EXECUTE, 0x7d05u);
                        instruction_limit = executed + 30000000u;
                        continue;
                    }
                    break;
                }
                ata_boot_sector = status == LIB_STATUS_OK &&
                    retired.instruction_linear == 0x7c00u &&
                    lib_memory_compare(actual_sector, expected_sector,
                        sizeof(actual_sector)) == 0;
                lib_c_printf("PC110:ATA:boot-sector=%u target=%04x:%08x\n",
                    ata_boot_sector, retired.cs, retired.eip);
                if (partition_checkpoint && ata_boot_sector) {
                    status = x86_core_machine_debug_set_watchpoint(machine->x86_core_machine,
                        X86_CORE_CPU_DEBUG_WATCH_EXECUTE, 0x7c00u);
                    instruction_limit = executed + 30000000u;
                    continue;
                }
                break;
            }
            if (post_reset_trace.power_restore_completed) {
                boot_handoff = retired.instruction_linear == 0xf52bdu &&
                    retired.instruction_byte_count >= 2u &&
                    retired.instruction_bytes[0u] == 0xcdu && retired.instruction_bytes[1u] == 0x19u;
                lib_c_printf("PC110:BOOT:int19=%u target=%04x:%08x\n",
                    boot_handoff, retired.cs, retired.eip);
                if (ata_checkpoint && boot_handoff) {
                    /* Bound the observed slave bootstrap phase itself. Its
                     * finite floppy retry sequence outlives the old total
                     * cold-start ceiling before the BIOS reaches drive 80h. */
                    instruction_limit = setup_media_checkpoint || ata_slave_checkpoint ? executed + 100000000u :
                        100000000u;
                    post_reset_trace.boot_ata_trace_count = 0u;
                    mypc110_post_diagnostic_boot_progress(machine->x86_core_machine, executed);
                    lib_u16 vector[2u] = {0};
                    status = x86_core_machine_memory_read(machine->x86_core_machine,
                        8u * 4u, vector, sizeof(vector));
                    post_reset_trace.boot_irq0_entry = ((lib_u32)vector[1u] << 4u) + vector[0u];
                    if (status == LIB_STATUS_OK)
                        status = x86_core_machine_memory_read(machine->x86_core_machine,
                            0x70u * 4u, vector, sizeof(vector));
                    post_reset_trace.boot_irq8_entry = ((lib_u32)vector[1u] << 4u) + vector[0u];
                    lib_c_printf("PC110:ATA:vectors irq0=%08x irq8=%08x status=%d\n",
                        post_reset_trace.boot_irq0_entry,
                        post_reset_trace.boot_irq8_entry, status);
                    if (status == LIB_STATUS_OK)
                        status = x86_core_machine_set_retirement_observation_provider(
                        machine->x86_core_machine,
                        &(x86_core_machine_retirement_observation_provider){
                            .callback = mypc110_post_diagnostic_observe_boot,
                            .context = &post_reset_trace});
                    if (status == LIB_STATUS_OK)
                        status = x86_core_machine_debug_set_watchpoint(machine->x86_core_machine,
                            X86_CORE_CPU_DEBUG_WATCH_EXECUTE, 0x7c00u);
                    continue;
                }
                break;
            }
            if (!post_reset_trace.power_restore_entered) {
                post_reset_trace.power_restore_entered = LIB_TRUE;
                status = x86_core_machine_debug_set_watchpoint(machine->x86_core_machine,
                    X86_CORE_CPU_DEBUG_WATCH_EXECUTE, 0x3dbc5u);
                continue;
            }
            x86_core_cpu_debug_cpu_snapshot resumed_cpu = {0};
            lib_u32 step = 0u;
            /* Execution watchpoints report after the CALL. Follow its real
             * EOSMI helper to the RSM boundary without changing guest state. */
            for (; status == LIB_STATUS_OK && step < 128u; ++step) {
                status = x86_core_machine_debug_capture_cpu_snapshot(machine->x86_core_machine,
                    X86_CORE_CPU_SNAPSHOT_CURRENT, &resumed_cpu);
                if (status != LIB_STATUS_OK || resumed_cpu.cs.base + resumed_cpu.eip == 0x3dbc8u) break;
                status = x86_core_machine_debug_step(machine->x86_core_machine, &result);
            }
            if (status != LIB_STATUS_OK || step == 128u) break;
            status = x86_core_machine_memory_read(machine->x86_core_machine,
                0x3fe00u, restored_cpu_image, sizeof(restored_cpu_image));
            retained_cpu_image = status == LIB_STATUS_OK && lib_memory_compare(
                suspended_cpu_image, restored_cpu_image, sizeof(restored_cpu_image)) == 0;
            if (!retained_cpu_image) break;
            status = x86_core_machine_debug_step(machine->x86_core_machine, &result);
            if (status == LIB_STATUS_OK)
                status = x86_core_machine_debug_capture_cpu_snapshot(machine->x86_core_machine,
                    X86_CORE_CPU_SNAPSHOT_CURRENT, &resumed_cpu);
            post_reset_trace.power_restore_completed = status == LIB_STATUS_OK && result.executed == 1u &&
                result.reason == X86_CORE_MACHINE_STOP_BUDGET;
            lib_c_printf("PC110:POWER:rsm status=%d executed=%llu return=%04x:%08x flags=%08x retained=%u\n",
                status, (unsigned long long)result.executed, resumed_cpu.cs.selector,
                resumed_cpu.eip, resumed_cpu.eflags, retained_cpu_image);
            lib_c_fflush(lib_c_stdout);
            if (boot_checkpoint && post_reset_trace.power_restore_completed) {
                if (setup_wake_pending) {
                    setup_wake_pending = LIB_FALSE;
                    lib_c_printf("PC110:SETUP:postreset-rsm=1\n");
                }
                /* Bound the new phase, not just the expensive cold prefix,
                 * so containment returns a usable final CPU observation. */
                instruction_limit = setup_resume_checkpoint && setup_reset_completed ?
                    executed + 20000000u : setup_checkpoint ? 120000000u : executed + 8000000u;
                if (!setup_checkpoint)
                    mypc110_post_diagnostic_boot_progress(machine->x86_core_machine, executed);
                boot_report = executed + 1000000u;
                status = x86_core_machine_debug_set_watchpoint(machine->x86_core_machine,
                    X86_CORE_CPU_DEBUG_WATCH_EXECUTE,
                    setup_resume_checkpoint && setup_reset_completed ? 0xf52bdu :
                        setup_checkpoint ? 0x9009eu : 0xf52bdu);
                continue;
            }
            break;
        }
        if (power_checkpoint && !boot_checkpoint && post_reset_trace.power_restore_completed) break;
        if (os_exception_checkpoint && loader_reached && !os_exception_logged &&
            status == LIB_STATUS_OK) {
            status = x86_core_machine_capture_observation(machine->x86_core_machine, &observation);
            if (status != LIB_STATUS_OK) break;
            if (observation.diagnostic.delivered_exception_count > reset_exception_count) {
                const x86_core_cpu_fault_snapshot *exception =
                    &observation.diagnostic.last_delivered_exception;
                lib_c_printf("PC110:OS:exception count=%u mask=%08x code=%08x point=%04x:%08x bytes=%02x %02x %02x %02x flags=%08x\n",
                    observation.diagnostic.delivered_exception_count,
                    exception->exception_mask, exception->exception_code,
                    exception->point.cs, exception->point.eip,
                    exception->point.bytes[0u], exception->point.bytes[1u],
                    exception->point.bytes[2u], exception->point.bytes[3u],
                    exception->eflags);
                x86_core_cpu_debug_cpu_snapshot snapshot = {0};
                lib_u8 gate[8u] = {0};
                const lib_status snapshot_status = x86_core_machine_debug_capture_cpu_snapshot(
                    machine->x86_core_machine, X86_CORE_CPU_SNAPSHOT_CURRENT, &snapshot);
                const lib_status gate_status = snapshot_status == LIB_STATUS_OK ?
                    x86_core_machine_debug_read_linear(machine->x86_core_machine,
                        snapshot.idtr.base + 0x1cu * 8u, gate, sizeof(gate)) : snapshot_status;
                lib_c_printf("PC110:OS:idt snapshot=%d base=%08x limit=%08x gate=%d bytes=%02x %02x %02x %02x %02x %02x %02x %02x\n",
                    snapshot_status, snapshot.idtr.base, snapshot.idtr.limit, gate_status,
                    gate[0u], gate[1u], gate[2u], gate[3u], gate[4u], gate[5u], gate[6u], gate[7u]);
                os_exception_logged = LIB_TRUE;
            }
        }
        if (os_storm_checkpoint && loader_reached && os_exception_logged && !os_key_sent &&
            status == LIB_STATUS_OK) {
            status = x86_core_machine_capture_observation(machine->x86_core_machine, &observation);
            if (status != LIB_STATUS_OK) break;
            const x86_core_cpu_fault_snapshot *exception =
                &observation.diagnostic.last_delivered_exception;
            if ((os_key_checkpoint && exception->valid &&
                    exception->point.bytes[0u] == 0xcdu && exception->point.bytes[1u] == 0x16u &&
                    ((exception->eax >> 8u) & 0xffu) == 1u &&
                    (!os_late_key_checkpoint || exception->point.bytes[2u] == 0x74u)) ||
                (!os_key_checkpoint && observation.diagnostic.delivered_exception_count >= 1000u)) {
                lib_c_printf("PC110:OS:%s count=%u mask=%08x code=%08x point=%04x:%08x bytes=%02x %02x %02x %02x flags=%08x eax=%08x edx=%08x ss=%04x esp=%08x\n",
                    os_key_checkpoint ? "input-wait" : "storm",
                    observation.diagnostic.delivered_exception_count,
                    exception->exception_mask, exception->exception_code,
                    exception->point.cs, exception->point.eip,
                    exception->point.bytes[0u], exception->point.bytes[1u],
                    exception->point.bytes[2u], exception->point.bytes[3u],
                    exception->eflags, exception->eax, exception->edx,
                    exception->ss, exception->esp);
                if (os_key_checkpoint) {
                    if (os_inking_checkpoint) {
                        status = x86_core_machine_set_retirement_observation_provider(
                            machine->x86_core_machine,
                            &(x86_core_machine_retirement_observation_provider){
                                .callback = mypc110_post_diagnostic_observe_boot,
                                .context = &post_reset_trace});
                        if (status != LIB_STATUS_OK) break;
                        lib_c_printf("PC110:INK:retirement-observation=on\n");
                    }
                    kvm_input_event key = {.type = KVM_EVENT_KEY,
                        .data.key = {.key = BASE_KEY_ENTER, .pressed = LIB_TRUE}};
                    power_driver.deliver_input(power_driver.context, &key);
                    key.data.key.pressed = LIB_FALSE;
                    power_driver.deliver_input(power_driver.context, &key);
                    os_key_sent = LIB_TRUE;
                    instruction_limit = executed + (os_inking_checkpoint ? 100000000u : 50000000u);
                    if (os_inking_checkpoint) os_inking_next = executed + 10000000u;
                    lib_c_printf("PC110:OS:key-enter=1 executed=%llu\n",
                        (unsigned long long)executed);
                    continue;
                }
                break;
            }
        }
        if (fdc_checkpoint && post_reset_trace.fdc_reset_completed) break;
        if (timer_checkpoint && !fdc_checkpoint && !power_checkpoint && (post_reset_trace.timer_passed ||
                post_reset_trace.timer_failed ||
                result.reason == X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT)) break;
        if (processor_reset_count != 0u && !post_reset_exception.valid &&
            x86_core_machine_capture_observation(machine->x86_core_machine,
                &observation) == LIB_STATUS_OK &&
            observation.diagnostic.delivered_exception_count > reset_exception_count) {
            post_reset_exception = observation.diagnostic.last_delivered_exception;
            post_reset_observation = observation;
        }
        if (status != LIB_STATUS_OK || result.reason == X86_CORE_MACHINE_STOP_FAULT) break;
        if (os_inking_checkpoint && os_key_sent && os_inking_events < 8u &&
            executed >= os_inking_next) {
            const lib_i32 corners[4][2] = {{5, 5}, {250, 5}, {5, 250}, {250, 250}};
            lib_u32 control = 0u;
            status = x86_core_machine_bus_read(machine->x86_core_machine, 0x15e2u, &control);
            if (status != LIB_STATUS_OK) break;
            if ((control & 0xfeu) == 0x38u) {
                const lib_u8 corner = os_inking_events / 2u;
                if ((os_inking_events & 1u) == 0u) {
                    char frame_path[512u];
                    const lib_i32 length = lib_c_snprintf(frame_path, sizeof(frame_path),
                        "%s.corner-%u.ppm", setup_frame_path, (unsigned int)corner);
                    if (length < 0 || (lib_size)length >= sizeof(frame_path)) {
                        status = LIB_STATUS_LIMIT_EXCEEDED;
                        break;
                    }
                    status = mypc110_post_diagnostic_write_frame(machine, &power_driver, frame_path);
                    if (status != LIB_STATUS_OK) break;
                }
                kvm_input_event point = {.type = KVM_EVENT_MOUSE,
                    .data.mouse = {.relative = LIB_FALSE,
                        .absolute_x = corners[corner][0], .absolute_y = corners[corner][1],
                        .buttons = (os_inking_events & 1u) == 0u ? BASE_MOUSE_BUTTON_LEFT : 0u}};
                power_driver.deliver_input(power_driver.context, &point);
                ++os_inking_events;
                os_inking_next = executed + ((os_inking_events & 1u) != 0u ?
                    1000000u : 10000000u);
                lib_c_printf("PC110:INK:host-event=%u corner=%u executed=%llu\n",
                    os_inking_events, corner, (unsigned long long)executed);
                lib_c_fflush(lib_c_stdout);
            }
        }
        if (os_inking_checkpoint && os_inking_events == 8u && os_pointer_phase < 2u &&
            executed >= os_inking_next) {
            char frame_path[512u];
            const lib_i32 length = lib_c_snprintf(frame_path, sizeof(frame_path),
                "%s.pointer-%u.ppm", setup_frame_path, (unsigned int)os_pointer_phase);
            if (length < 0 || (lib_size)length >= sizeof(frame_path)) {
                status = LIB_STATUS_LIMIT_EXCEEDED;
                break;
            }
            status = mypc110_post_diagnostic_write_frame(machine, &power_driver, frame_path);
            if (status != LIB_STATUS_OK) break;
            if (os_pointer_phase == 0u) {
                if (os_ps2_checkpoint) {
                    status = x86_nxvm_board_mouse_receive_relative(machine->board, 40, -20, 0u);
                    if (status != LIB_STATUS_OK) break;
                } else {
                    const kvm_input_event motion = {.type = KVM_EVENT_MOUSE,
                        .data.mouse = {.relative = LIB_TRUE, .delta_x = 40, .delta_y = 20}};
                    power_driver.deliver_input(power_driver.context, &motion);
                }
                os_inking_next = executed + 5000000u;
            }
            ++os_pointer_phase;
            lib_c_printf("PC110:POINTER:phase=%u executed=%llu\n", os_pointer_phase,
                (unsigned long long)executed);
            lib_c_fflush(lib_c_stdout);
        }
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
        if (result.reason == X86_CORE_MACHINE_STOP_RESET_REQUESTED) {
            ++processor_reset_count;
            if (setup_resume_checkpoint && setup_reset_completed) {
                status = x86_core_machine_capture_observation(machine->x86_core_machine,
                    &reset_observation);
                if (status != LIB_STATUS_OK) break;
                reset_exception_count = reset_observation.diagnostic.delivered_exception_count;
                lib_c_printf("PC110:SETUP:postreset-reset pc=%08x resets=%u\n",
                    result.linear_pc, processor_reset_count);
                lib_c_printf("PC110:SETUP:postreset-last-io=%08x/%u/%04x/%02x\n",
                    post_reset_trace.setup_postreset_last_io.linear_pc,
                    post_reset_trace.setup_postreset_last_io.type,
                    post_reset_trace.setup_postreset_last_io.port,
                    post_reset_trace.setup_postreset_last_io.value);
                if (result.linear_pc == 0xfffffff0u && processor_reset_count < 16u) {
                    status = x86_core_machine_debug_set_watchpoint(machine->x86_core_machine,
                        X86_CORE_CPU_DEBUG_WATCH_EXECUTE,
                        setup_wake_pending ? 0x39336u : boot_handoff ? 0x7c00u : 0xf52bdu);
                    if (status == LIB_STATUS_OK) continue;
                }
                break;
            }
            if (setup_checkpoint && setup_entry && setup_input_count == 2u) {
                setup_reset_completed = result.linear_pc == 0xfffffff0u;
                lib_c_printf("PC110:SETUP:processor-reset=%u pc=%08x resets=%u\n",
                    setup_reset_completed, result.linear_pc, processor_reset_count);
                lib_c_printf("PC110:SETUP:input-bytes=%llu values=%02x %02x %02x %02x cmos-writes=%llu\n",
                    (unsigned long long)post_reset_trace.setup_input_byte_count,
                    post_reset_trace.setup_input_bytes[0u], post_reset_trace.setup_input_bytes[1u],
                    post_reset_trace.setup_input_bytes[2u], post_reset_trace.setup_input_bytes[3u],
                    (unsigned long long)post_reset_trace.setup_cmos_write_count);
                mypc110_post_diagnostic_print_ports(&post_reset_trace);
                if (setup_resume_checkpoint && setup_reset_completed) {
                    post_reset_trace.setup_postreset_tracking = LIB_TRUE;
                    instruction_limit = executed + 120000000u;
                    status = x86_core_machine_debug_set_watchpoint(machine->x86_core_machine,
                        X86_CORE_CPU_DEBUG_WATCH_EXECUTE, 0xf52bdu);
                    if (status == LIB_STATUS_OK) continue;
                }
                break;
            }
            if (power_checkpoint && button_sent) {
                /* Processor reset clears CPU-owned debugger controls. */
                status = x86_core_machine_debug_set_watchpoint(machine->x86_core_machine,
                    X86_CORE_CPU_DEBUG_WATCH_EXECUTE,
                    setup_checkpoint && post_reset_trace.power_restore_completed ? 0x9009eu :
                        boot_handoff ? 0x7c00u :
                        post_reset_trace.power_restore_completed ? 0xf52bdu : 0x39336u);
            }
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
            x86_core_cpu_debug_cpu_snapshot cpu = {0};

            ++wait_count;
            if (power_checkpoint && (!button_sent ||
                (setup_resume_checkpoint && setup_reset_completed && !setup_wake_sent)) &&
                x86_vl82c420_suspend_refresh_enabled(power_firmware->configuration_chip)) {
                const lib_u32 suspended_pc = result.linear_pc;
                const lib_bool post_setup_wake = button_sent;
                status = x86_core_machine_run(machine->x86_core_machine,
                    (x86_core_machine_run_budget){4096u, 0u}, &result);
                if (status != LIB_STATUS_OK || result.executed != 0u ||
                    result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) break;
                lib_c_printf("PC110:POWER:no-event pc=%08x repeat=%08x executed=%llu\n",
                    suspended_pc, result.linear_pc, (unsigned long long)result.executed);
                if (setup_checkpoint) mypc110_post_diagnostic_print_ports(&post_reset_trace);
                lib_c_fflush(lib_c_stdout);
                status = x86_core_machine_memory_read(machine->x86_core_machine,
                    0x3fe00u, suspended_cpu_image, sizeof(suspended_cpu_image));
                if (status == LIB_STATUS_OK) {
                    if (post_setup_wake) {
                        lib_c_printf("PC110:SETUP:postreset-suspend pc=%08x\n", suspended_pc);
                        post_reset_trace.power_restore_entered = LIB_FALSE;
                        post_reset_trace.power_restore_completed = LIB_FALSE;
                        retained_cpu_image = LIB_FALSE;
                        setup_wake_sent = LIB_TRUE;
                        setup_wake_pending = LIB_TRUE;
                    }
                    kvm_input_event key = {.type = KVM_EVENT_KEY,
                        .data.key = {.key = BASE_KEY_F12, .pressed = LIB_TRUE}};
                    power_driver.deliver_input(power_driver.context, &key);
                    key.data.key.pressed = LIB_FALSE;
                    power_driver.deliver_input(power_driver.context, &key);
                    if (x86_vl82c420_suspend_refresh_enabled(power_firmware->configuration_chip))
                        status = LIB_STATUS_INVALID_STATE;
                }
                button_sent = status == LIB_STATUS_OK;
                if (boot_checkpoint && !setup_checkpoint)
                    mypc110_post_diagnostic_boot_progress(machine->x86_core_machine, executed);
                continue;
            }
            status = x86_core_machine_debug_capture_cpu_snapshot(machine->x86_core_machine,
                X86_CORE_CPU_SNAPSHOT_CURRENT, &cpu);
            /* A POST error's CLI/HLT is terminal for this diagnostic. */
            if (status != LIB_STATUS_OK ||
                (cpu.eflags & X86_CORE_CPU_DEBUG_EFLAGS_IF) == 0u) break;
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
    if (boot_checkpoint && machine != LIB_NULL) {
        x86_core_cpu_debug_cpu_snapshot cpu = {0};
        lib_u8 bytes[16u] = {0};
        lib_status capture = x86_core_machine_debug_capture_cpu_snapshot(
            machine->x86_core_machine, X86_CORE_CPU_SNAPSHOT_CURRENT, &cpu);
        lib_c_printf("PC110:BOOT:state=%d %04x:%08x base=%08x cr0=%08x flags=%08x ax=%08x bx=%08x cx=%08x dx=%08x ds=%04x/%08x ss=%04x/%08x sp=%08x\n",
            capture, cpu.cs.selector, cpu.eip, cpu.cs.base, cpu.cr0, cpu.eflags,
            cpu.eax, cpu.ebx, cpu.ecx, cpu.edx, cpu.ds.selector, cpu.ds.base,
            cpu.ss.selector, cpu.ss.base, cpu.esp);
        if (capture == LIB_STATUS_OK)
            capture = x86_core_machine_memory_read(machine->x86_core_machine,
                cpu.cs.base + cpu.eip, bytes, sizeof(bytes));
        lib_c_printf("PC110:BOOT:code=%d", capture);
        if (capture == LIB_STATUS_OK)
            for (lib_size index = 0u; index < sizeof(bytes); ++index)
                lib_c_printf(" %02x", bytes[index]);
        lib_c_printf("\n");
        if (setup_resume_checkpoint && !setup_postreset_handoff && capture == LIB_STATUS_OK) {
            lib_u16 stack[16u] = {0};
            capture = x86_core_machine_memory_read(machine->x86_core_machine,
                cpu.ss.base + cpu.esp, stack, sizeof(stack));
            lib_c_printf("PC110:SETUP:postreset-stack=%d", capture);
            if (capture == LIB_STATUS_OK)
                for (lib_size index = 0u; index < sizeof(stack) / sizeof(stack[0]); ++index)
                    lib_c_printf(" %04x", stack[index]);
            lib_c_printf("\n");
        }
        if (ata_checkpoint && capture == LIB_STATUS_OK) {
            lib_u16 stack[16u] = {0};
            capture = x86_core_machine_memory_read(machine->x86_core_machine,
                cpu.ss.base + cpu.esp, stack, sizeof(stack));
            lib_c_printf("PC110:ATA:stack=%d", capture);
            if (capture == LIB_STATUS_OK)
                for (lib_size index = 0u; index < sizeof(stack) / sizeof(stack[0]); ++index)
                    lib_c_printf(" %04x", stack[index]);
            lib_c_printf("\n");
            lib_u8 bda[0x200u] = {0};
            capture = x86_core_machine_memory_read(machine->x86_core_machine,
                0x400u, bda, sizeof(bda));
            lib_c_printf("PC110:ATA:timers capture=%d ticks=%llu irq0=%u irq8=%u smi=%u bda-timer=%02x%02x%02x%02x wait=%02x disk=%02x motor=%02x\n",
                capture, (unsigned long long)post_reset_trace.boot_ticks,
                post_reset_trace.boot_irq0_count, post_reset_trace.boot_irq8_count,
                post_reset_trace.boot_smi_count, bda[0x6fu], bda[0x6eu],
                bda[0x6du], bda[0x6cu], bda[0x132u], bda[0x75u], bda[0x40u]);
        }
    }
    if (timer_checkpoint) {
        lib_c_printf("PC110:TIMER:passed=%u failed=%u delta=%04x executed=%llu pc=%08x status=%d copied=%u initialized=%u asmi=%u/%u rsm=%u\n",
            post_reset_trace.timer_passed, post_reset_trace.timer_failed,
            post_reset_trace.timer_delta, (unsigned long long)executed,
            result.linear_pc, (int)status, post_reset_trace.copied_handler,
            post_reset_trace.initialized, post_reset_trace.asmi_requested,
            post_reset_trace.asmi_serviced, post_reset_trace.smi_returned);
        lib_c_fflush(lib_c_stdout);
    }
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
    if (c000_checkpoint || c000_follow_checkpoint || c000_port61_follow_checkpoint) {
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
            lib_c_printf("PC110:POST:c000=%04x:%08x bytes=%02x %02x %02x %02x\n",
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
    mypc110_post_diagnostic_print_ports(&post_reset_trace);
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
    if (setup_checkpoint && !setup_resume_checkpoint && argc == 4 && setup_entry && setup_bytes_match &&
        status == LIB_STATUS_OK) {
        status = mypc110_post_diagnostic_write_frame(machine, &power_driver, setup_frame_path);
    }
    if (setup_resume_checkpoint && machine != LIB_NULL) {
        if (os_key_checkpoint && construction.display_snapshot_context != LIB_NULL) {
            x86_f65535 *display = (x86_f65535 *)construction.display_snapshot_context;
            const lib_u16 index_ports[] = {0x03d4u, 0x03ceu, 0x03c4u};
            const lib_u16 data_ports[] = {0x03d5u, 0x03cfu, 0x03c5u};
            const lib_u8 counts[] = {25u, 9u, 5u};
            for (lib_u32 bank = 0u; bank < 3u; ++bank) {
                lib_c_printf("PC110:OS:video-bank=%u", bank);
                for (lib_u32 i = 0u; i < counts[bank]; ++i) {
                    lib_u8 value = 0u;
                    lib_status read_status = x86_f65535_vga_write(display, index_ports[bank], (lib_u8)i);
                    if (read_status == LIB_STATUS_OK)
                        read_status = x86_f65535_vga_read(display, data_ports[bank], &value);
                    lib_c_printf(" %02x/%d", value, read_status);
                }
                lib_c_printf("\n");
            }
        }
        if (os_exception_checkpoint) {
            x86_core_cpu_debug_cpu_snapshot snapshot = {0};
            lib_u8 code[32u] = {0};
            const lib_status snapshot_status = x86_core_machine_debug_capture_cpu_snapshot(
                machine->x86_core_machine, X86_CORE_CPU_SNAPSHOT_CURRENT, &snapshot);
            const lib_u32 address = snapshot.cs.base + snapshot.eip;
            const lib_status code_status = snapshot_status == LIB_STATUS_OK && address >= 16u ?
                x86_core_machine_debug_read_linear(machine->x86_core_machine,
                    address - 16u, code, sizeof(code)) : LIB_STATUS_INVALID_STATE;
            lib_c_printf("PC110:OS:code-context status=%d start=%08x", code_status, address - 16u);
            for (lib_u32 i = 0u; i < sizeof(code); ++i) lib_c_printf(" %02x", code[i]);
            lib_c_printf("\n");
        }
        kvm_window_frame_storage frame = {0};
        kvm_console_text_frame console = {0};
        lib_bool produced = LIB_FALSE;
        lib_status frame_status = x86_nxvm_machine_publish_display(machine, LIB_TRUE);
        if (frame_status == LIB_STATUS_OK)
            frame_status = power_driver.copy_frame(power_driver.context, &frame, &console, &produced);
        lib_c_printf("PC110:SETUP:postreset-text status=%d produced=%u columns=%u rows=%u\n",
            frame_status, produced, console.base.text_columns, console.base.text_rows);
        if (frame_status == LIB_STATUS_OK && produced && console.base.cells != LIB_NULL)
            for (lib_u32 row = 0u; row < console.base.text_rows; ++row) {
                lib_c_printf("PC110:SETUP:row ");
                for (lib_u32 col = 0u; col < console.base.text_columns; ++col) {
                    const lib_u8 glyph = console.base.cells[row * console.base.stride + col].glyph_index;
                    lib_c_printf("%c", glyph >= 32u && glyph < 127u ? glyph : ' ');
                }
                lib_c_printf("\n");
            }
        kvm_window_frame_release(&frame);
        kvm_console_frame_release(&console);
        if (setup_media_checkpoint) {
            char final_path[512u];
            const lib_i32 length = lib_c_snprintf(final_path, sizeof(final_path),
                "%s.final.ppm", setup_frame_path);
            const lib_status final_status = length < 0 || (lib_size)length >= sizeof(final_path) ?
                LIB_STATUS_LIMIT_EXCEEDED :
                mypc110_post_diagnostic_write_frame(machine, &power_driver, final_path);
            os_final_frame_written = final_status == LIB_STATUS_OK;
            lib_c_printf("PC110:SETUP:final-frame=%d\n", final_status);
        }
    }
    if (setup_resume_checkpoint && !setup_media_checkpoint && argc == 5 && status == LIB_STATUS_OK &&
        setup_postreset_handoff) {
        status = mypc110_cmos_save(argv[4], machine);
        lib_c_printf("PC110:SETUP:cmos-save=%d\n", status);
    }
    x86_nxvm_machine_destroy(machine);
    if (cmos_origin_checkpoint)
        return status == LIB_STATUS_OK && post_reset_trace.cmos_trace_completed ? 0 : 1;
    if (power_checkpoint) {
        if (os_checkpoint)
            lib_c_printf("PC110:OS:stop=%d pc=%08x executed=%llu limit=%llu status=%d\n",
                result.reason, result.linear_pc, (unsigned long long)executed,
                (unsigned long long)instruction_limit, status);
        const lib_bool passed = status == LIB_STATUS_OK && button_sent &&
            post_reset_trace.power_restore_entered && post_reset_trace.power_restore_completed &&
            (!boot_checkpoint || (setup_checkpoint ? setup_entry : boot_handoff)) &&
            (!setup_checkpoint || (setup_bytes_match && (argc < 4 ||
                (setup_reset_completed && post_reset_trace.setup_input_byte_count ==
                    MYPC110_SETUP_INPUT_BYTES &&
                 post_reset_trace.setup_input_bytes[0u] == 0x1cu &&
                 post_reset_trace.setup_input_bytes[1u] == 0x9cu &&
                 post_reset_trace.setup_input_bytes[2u] == 0x1cu &&
                 post_reset_trace.setup_input_bytes[3u] == 0x9cu &&
                 post_reset_trace.setup_cmos_write_count != 0u &&
                 (!setup_resume_checkpoint || (!setup_wake_pending &&
                    setup_postreset_handoff)))))) &&
            (!ata_checkpoint || ata_boot_sector) &&
            (!partition_checkpoint || partition_boot_sector) &&
            (!system_files_checkpoint || system_files_reached) &&
            (!loader_checkpoint || loader_reached) &&
            retained_cpu_image &&
            (os_inking_checkpoint || observation.diagnostic.delivered_exception_count == reset_exception_count) &&
            (!os_inking_checkpoint || (os_key_sent && os_inking_events == 8u &&
                os_pointer_phase == 2u && os_final_frame_written)) &&
            !observation.diagnostic.first_fault.valid;
        lib_c_printf("PC110:POWER:button=%u restore=%u rsm=%u status=%d pc=%08x resets=%u executed=%llu\n",
            button_sent, post_reset_trace.power_restore_entered,
            post_reset_trace.power_restore_completed, status, result.linear_pc,
            processor_reset_count, (unsigned long long)executed);
        lib_c_printf("PC110:POWER:result=%u exceptions=%u baseline=%u fault=%u\n",
            passed, observation.diagnostic.delivered_exception_count,
            reset_exception_count, observation.diagnostic.first_fault.valid);
        return passed ? 0 : 1;
    }
    if (fdc_checkpoint) {
        lib_c_printf("PC110:FDC-RESET:asserted=%u released=%u irq-return=%u senses-complete=%u\n",
            post_reset_trace.fdc_reset_asserted, post_reset_trace.fdc_reset_released,
            post_reset_trace.fdc_wait_returned, post_reset_trace.fdc_reset_completed);
        if (!post_reset_trace.fdc_reset_completed) return 1;
    }
    if (timer_checkpoint) {
        return status == LIB_STATUS_OK && post_reset_trace.timer_passed &&
            post_reset_trace.copied_handler && post_reset_trace.initialized &&
            post_reset_trace.asmi_requested && post_reset_trace.asmi_serviced &&
            post_reset_trace.smi_returned &&
            !post_reset_trace.timer_failed &&
            post_reset_trace.timer_delta >= 0x1614u &&
            post_reset_trace.timer_delta <= 0x1868u ? 0 : 1;
    }
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
    return status == LIB_STATUS_OK &&
        result.reason == X86_CORE_MACHINE_STOP_PAUSED &&
        post_reset_trace.before_pop_ds_valid && post_reset_trace.after_pop_ds_valid &&
        post_reset_trace.after_pop_ds.ds.selector == 0x0008u &&
        post_reset_trace.after_pop_ds.ds.base ==
            MYPC110_POST_DIAGNOSTIC_DATA_SELECTOR_BASE ? 0 : 1;
}
