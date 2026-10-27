/* External, embedded-BIOS qualification; earlier observation modes remain
 * available for manual investigation through this same execution harness. */
#include "app-mypc110/pc110_plan_interface.h"
#include "app-mypc110/pc110_firmware_interface.h"
#include "lib/types/file.h"
#include "lib/base/clock_interface.h"
#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/debug_interface.h"
#include "x86-core/machine/retirement_observation_interface.h"
#include "x86-core/machine/trace_interface.h"
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
    lib_u8 setup_input_deliveries;
    lib_u8 setup_input_bytes[MYPC110_SETUP_INPUT_BYTES];
    lib_size setup_input_byte_count;
    lib_size setup_cmos_write_count;
    mypc110_post_diagnostic_port_write
        configuration_writes[MYPC110_POST_DIAGNOSTIC_PORT_CAPACITY];
    lib_size configuration_write_count;
    lib_size boot_trace_count;
    lib_u64 boot_ticks;
    lib_u32 boot_irq0_entry;
    lib_u32 boot_irq8_entry;
    lib_u32 boot_irq0_count;
    lib_u32 boot_irq8_count;
    lib_u32 boot_smi_count;
} mypc110_post_diagnostic_trace;

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
         (port != 0x0070u && port != 0x0071u))) {
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

        if ((observation->io_direction == X86_CORE_MACHINE_RETIREMENT_IO_READ &&
                (port == 0x0060u || port == 0x0092u || port == 0x03e0u ||
                 port == 0x03e1u)) ||
            (observation->io_direction == X86_CORE_MACHINE_RETIREMENT_IO_WRITE &&
                (port == 0x0060u || port == 0x0064u || port == 0x0092u ||
                 port == 0x03e0u || port == 0x03e1u)) || setup_rtc_port) {
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
        ((port >= 0x1f0u && port <= 0x1f7u) || port == 0x3f6u ||
            port == 0x3e0u || port == 0x3e1u || port == 0x8bu)) {
        lib_c_printf("PC110:ATA:io pc=%08x port=%04x direction=%u bytes=%u value=%08x\n",
            observation->point.linear_pc, port, observation->io_direction,
            observation->io_bytes, observation->io_value);
        ++trace->boot_trace_count;
    }
}

int main(int argc, char **argv)
{
    const lib_bool setup_checkpoint = (argc == 3 || argc == 4) && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--setup") == 0;
    const lib_bool ata_checkpoint = argc == 3 && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--ata-boot") == 0;
    const x86_nxvm_machine_config config = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80486SX,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .bios_count = 1u,
        .fixed_disk_image = {ata_checkpoint ? argv[2] : LIB_NULL},
        .fixed_disk_mode = {LIB_STORAGE_MEDIUM_OVERLAY}};
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
    lib_bool fdc_checkpoint = argc == 2 && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--fdc-reset") == 0;
    lib_bool boot_checkpoint = setup_checkpoint || ata_checkpoint || (argc == 2 && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--resume-boot") == 0);
    lib_bool power_checkpoint = boot_checkpoint || (argc == 2 && argv != LIB_NULL &&
        lib_text_compare(argv[1], "--resume-button") == 0);
    lib_bool boot_handoff = LIB_FALSE;
    lib_bool ata_boot_sector = LIB_FALSE;
    lib_bool setup_entry = LIB_FALSE;
    lib_bool setup_bytes_match = LIB_FALSE;
    lib_bool setup_reset_completed = LIB_FALSE;
    lib_u8 setup_input_count = 0u;
    static lib_u8 setup_expected[MYPC110_SETUP_IMAGE_BYTES];
    static lib_u8 setup_actual[MYPC110_SETUP_IMAGE_BYTES];
    lib_u8 expected_sector[512u] = {0};
    lib_u64 boot_report = 0u;
    lib_bool button_sent = LIB_FALSE;
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
        !timer_checkpoint) return 2;
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
    if (ata_checkpoint) {
        lib_storage_medium *medium = LIB_NULL;
        status = lib_storage_medium_open(argv[2], LIB_STORAGE_MEDIUM_READONLY, &medium);
        if (status == LIB_STATUS_OK)
            status = lib_storage_medium_read_at(medium, 0u, expected_sector,
                sizeof(expected_sector));
        const lib_status closed = lib_storage_medium_destroy(&medium);
        if (status == LIB_STATUS_OK) status = closed;
    }
    if (setup_checkpoint) {
        lib_storage_medium *reference = LIB_NULL;
        status = lib_storage_medium_open(argv[2], LIB_STORAGE_MEDIUM_READONLY, &reference);
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
        status = mypc110_machine_construction_create(&config, &vm_app_firmware, &construction);
    if (status == LIB_STATUS_OK) power_firmware = construction.firmware_context;
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_machine_create(&config, &construction, &machine);
    if (status == LIB_STATUS_OK && power_checkpoint)
        status = x86_nxvm_machine_describe_emulator_driver(machine, &power_driver);
    if (status == LIB_STATUS_OK && !power_checkpoint) {
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
        !c000_follow_checkpoint && !c000_port61_follow_checkpoint) {
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
        if (status == LIB_STATUS_OK && result.reason == X86_CORE_MACHINE_STOP_BUDGET &&
            setup_entry && argc == 4 && setup_input_count < 2u && executed >= instruction_limit) {
            char before_path[512u];
            const lib_i32 length = lib_c_snprintf(before_path, sizeof(before_path),
                "%s.before-%u.ppm", argv[3], (unsigned int)setup_input_count);

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
        if (boot_checkpoint && !setup_checkpoint && post_reset_trace.power_restore_completed &&
            executed >= boot_report) {
            mypc110_post_diagnostic_boot_progress(machine->x86_core_machine, executed);
            boot_report = executed + 1000000u;
        }
        if (power_checkpoint && result.reason == X86_CORE_MACHINE_STOP_PAUSED && button_sent) {
            const lib_u32 expected = setup_checkpoint && post_reset_trace.power_restore_completed ?
                0x9009eu : boot_handoff ? 0x7c00u :
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
            if (setup_checkpoint && post_reset_trace.power_restore_completed) {
                setup_entry = retired.instruction_linear == 0x9009eu &&
                    retired.cs == 0x5000u && retired.eip == 0u;
                status = x86_core_machine_memory_read(machine->x86_core_machine,
                    0x50000u, setup_actual, sizeof(setup_actual));
                setup_bytes_match = status == LIB_STATUS_OK && lib_memory_compare(
                    setup_actual, setup_expected, sizeof(setup_actual)) == 0;
                lib_c_printf("PC110:SETUP:entry=%u target=%04x:%08x bytes=%u matched=%u\n",
                    setup_entry, retired.cs, retired.eip,
                    MYPC110_SETUP_IMAGE_BYTES, setup_bytes_match);
                if (argc == 4 && setup_entry && setup_bytes_match) {
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
                status = x86_core_machine_memory_read(machine->x86_core_machine,
                    0x7c00u, actual_sector, sizeof(actual_sector));
                ata_boot_sector = status == LIB_STATUS_OK &&
                    retired.instruction_linear == 0x7c00u &&
                    lib_memory_compare(actual_sector, expected_sector,
                        sizeof(actual_sector)) == 0;
                lib_c_printf("PC110:ATA:boot-sector=%u target=%04x:%08x\n",
                    ata_boot_sector, retired.cs, retired.eip);
                break;
            }
            if (post_reset_trace.power_restore_completed) {
                boot_handoff = retired.instruction_linear == 0xf52bdu &&
                    retired.instruction_byte_count >= 2u &&
                    retired.instruction_bytes[0u] == 0xcdu && retired.instruction_bytes[1u] == 0x19u;
                lib_c_printf("PC110:BOOT:int19=%u target=%04x:%08x\n",
                    boot_handoff, retired.cs, retired.eip);
                if (ata_checkpoint && boot_handoff) {
                    instruction_limit = 100000000u;
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
                /* Bound the new phase, not just the expensive cold prefix,
                 * so containment returns a usable final CPU observation. */
                instruction_limit = setup_checkpoint ? 120000000u : executed + 8000000u;
                if (!setup_checkpoint)
                    mypc110_post_diagnostic_boot_progress(machine->x86_core_machine, executed);
                boot_report = executed + 1000000u;
                status = x86_core_machine_debug_set_watchpoint(machine->x86_core_machine,
                    X86_CORE_CPU_DEBUG_WATCH_EXECUTE,
                    setup_checkpoint ? 0x9009eu : 0xf52bdu);
                continue;
            }
            break;
        }
        if (power_checkpoint && !boot_checkpoint && post_reset_trace.power_restore_completed) break;
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
            if (power_checkpoint && !button_sent &&
                x86_vl82c420_suspend_refresh_enabled(power_firmware->configuration_chip)) {
                const lib_u32 suspended_pc = result.linear_pc;
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
    if (setup_checkpoint && argc == 4 && setup_entry && setup_bytes_match &&
        status == LIB_STATUS_OK) {
        status = mypc110_post_diagnostic_write_frame(machine, &power_driver, argv[3]);
    }
    x86_nxvm_machine_destroy(machine);
    if (power_checkpoint) {
        lib_c_printf("PC110:POWER:button=%u restore=%u rsm=%u status=%d pc=%08x resets=%u executed=%llu\n",
            button_sent, post_reset_trace.power_restore_entered,
            post_reset_trace.power_restore_completed, status, result.linear_pc,
            processor_reset_count, (unsigned long long)executed);
        return status == LIB_STATUS_OK && button_sent &&
            post_reset_trace.power_restore_entered && post_reset_trace.power_restore_completed &&
            (!boot_checkpoint || (setup_checkpoint ? setup_entry : boot_handoff)) &&
            (!setup_checkpoint || (setup_bytes_match && (argc != 4 ||
                (setup_reset_completed && post_reset_trace.setup_input_byte_count ==
                    MYPC110_SETUP_INPUT_BYTES &&
                 post_reset_trace.setup_input_bytes[0u] == 0x1cu &&
                 post_reset_trace.setup_input_bytes[1u] == 0x9cu &&
                 post_reset_trace.setup_input_bytes[2u] == 0x1cu &&
                 post_reset_trace.setup_input_bytes[3u] == 0x9cu &&
                 post_reset_trace.setup_cmos_write_count != 0u)))) &&
            (!ata_checkpoint || ata_boot_sector) &&
            retained_cpu_image && observation.diagnostic.delivered_exception_count == reset_exception_count &&
            !observation.diagnostic.first_fault.valid ? 0 : 1;
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
