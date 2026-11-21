#include "app-myob300/ob300_board_interface.h"
#include "app-myob300/ob300_chipset.h"
#include "app-myob300/ob300_display.h"
#include "app-myob300/ob300_mouse.h"
#include "x86-core/machine/attachment_interface.h"
#include "x86-core/machine/memory_interface.h"

#define MYOB300_CPU_CLOCK_HZ 20000000u
#define MYOB300_RAM_BYTES 0x200000u
#define MYOB300_RAM_PROBE_LIMIT 0x800000u
#define MYOB300_PIT_INPUT_HZ 1193182u
#define MYOB300_RTC_INPUT_HZ 32768u
#define MYOB300_KBC_COMMAND_RESPONSE_TICKS 256u
#define MYOB300_REFRESH_HZ 64000u
#define MYOB300_REFRESH_TICKS ((MYOB300_CPU_CLOCK_HZ + MYOB300_REFRESH_HZ / 2u) / MYOB300_REFRESH_HZ)
#define MYOB300_SYSTEM_ROM_BASE 0x000f8000u
#define MYOB300_SYSTEM_ROM_RESET_ALIAS 0x00ff8000u
#define MYOB300_MANAGEMENT_WINDOW_BASE 0x00060000u
#define MYOB300_MANAGEMENT_MEMORY_BYTES 0x10000u
#define MYOB300_MANAGEMENT_HALF_AB_BYTES 0x10000u
#define MYOB300_PANEL_DEFAULT_VSYNC_HZ 60u
#define MYOB300_PARALLEL_DATA_PORT 0x378u

struct myob300_board {
    myob300_chipset chipset;
    myob300_mouse mouse;
    x86_ct_vga *display;
    lib_u8 parallel_data;
    lib_u8 management_memory[MYOB300_MANAGEMENT_MEMORY_BYTES];
};

const x86_nxvm_board_config myob300_board_configuration = {
    .memory_bytes = MYOB300_RAM_BYTES,
    .cpu_profile = X86_CORE_CPU_PROFILE_AM386SXLV,
    .fpu_profile = X86_FPU_PROFILE_NONE,
    .ticks_per_instruction = 1u,
    .time_axis = {X86_CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL, MYOB300_CPU_CLOCK_HZ},
    .clock_plan = {
        .pit = {MYOB300_PIT_INPUT_HZ, MYOB300_CPU_CLOCK_HZ, 0u},
        .rtc = {MYOB300_RTC_INPUT_HZ, MYOB300_CPU_CLOCK_HZ, 0u}
    },
    .shared_pit_personality = X86_PIT_PERSONALITY_8254,
    .pic_topology = X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED,
    .dma_controller_count = X86_NXVM_BOARD_DMA_CONTROLLER_COUNT,
    .keyboard_topology = X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_8042,
    /* The internal keyboard fuse sense is high on input-port bit 2. */
    .kbc_input_port_configured = LIB_TRUE,
    .kbc_input_port = 0x84u,
    .kbc_retain_output_read = LIB_TRUE,
    /* L2: keep the self-test reply pending beyond POST's first data read. */
    .kbc_command_response_ticks = MYOB300_KBC_COMMAND_RESPONSE_TICKS,
    .display_topology = X86_NXVM_BOARD_DISPLAY_TOPOLOGY_NONE,
    .system_facade = X86_NXVM_BOARD_SYSTEM_FACADE_NONE,
    .system_control_port_present = LIB_TRUE
};

static void myob300_board_reset(void *owner, x86_nxvm_board_reset_phase phase)
{
    myob300_board *board = owner;

    if (phase == X86_NXVM_BOARD_RESET_BEFORE_DEVICES) {
        myob300_chipset_reset(&board->chipset);
        myob300_mouse_reset(&board->mouse);
        x86_ct_vga_reset(board->display);
        board->parallel_data = 0u;
        lib_memory_set(board->management_memory, 0, sizeof(board->management_memory));
    }
}

/* The onboard parallel connector exposes a readable SPP data latch. */
static lib_status myob300_board_parallel_read(void *owner, lib_u16 port,
    lib_u64 tick, lib_u32 *out_value)
{
    const myob300_board *board = owner;

    (void)port;
    (void)tick;
    *out_value = board->parallel_data;
    return LIB_STATUS_OK;
}

static lib_status myob300_board_parallel_write(void *owner, lib_u16 port,
    lib_u8 access_bytes, lib_u32 value)
{
    myob300_board *board = owner;

    (void)port;
    if (access_bytes != 1u) return LIB_STATUS_UNSUPPORTED;
    board->parallel_data = (lib_u8)value;
    return LIB_STATUS_OK;
}

static void myob300_board_management_active(void *owner, lib_bool active)
{
    myob300_board *board = owner;

    board->chipset.management_active = active;
}

static lib_status myob300_board_management_range(myob300_board *board, lib_u32 address,
    lib_u8 bytes, lib_u8 **mapping)
{
    const lib_u8 translation = MYOB300_CHIPSET_SMM_ENABLED | MYOB300_CHIPSET_SMM_TRANSLATED;
    const lib_u8 control = board->chipset.management_control;
    const lib_u8 selection = control & 7u;
    const lib_size capacity = selection == 7u ? sizeof(board->chipset.ab_shadow) :
        selection >= 4u ? 0x10000u : 0x8000u;
    const lib_u32 window_end = MYOB300_MANAGEMENT_WINDOW_BASE + (lib_u32)capacity;
    const lib_u64 access_end = (lib_u64)address + bytes;

    if (bytes == 0u) return LIB_STATUS_INVALID_ARGUMENT;
    if ((control & translation) != translation || address >= window_end ||
        access_end <= MYOB300_MANAGEMENT_WINDOW_BASE)
        return X86_CORE_MACHINE_SMM_ROUTE_ORDINARY;
    if (address < MYOB300_MANAGEMENT_WINDOW_BASE || access_end > window_end)
        return LIB_STATUS_UNSUPPORTED;
    /* SCAMP Table 40: SADS translates into the same onboard A/B DRAM
     * the firmware populates through the ordinary shadow controls. */
    switch (selection) {
    case 1u:
    case 4u:
    case 7u:
        *mapping = board->chipset.ab_shadow;
        break;
    case 2u:
    case 5u:
        *mapping = board->chipset.ab_shadow + MYOB300_MANAGEMENT_HALF_AB_BYTES;
        break;
    default:
        *mapping = board->management_memory;
        break;
    }
    *mapping += address - MYOB300_MANAGEMENT_WINDOW_BASE;
    return LIB_STATUS_OK;
}

static lib_status myob300_board_management_read(void *owner, lib_u32 address,
    void *destination, lib_u8 bytes, lib_bool observe_only)
{
    myob300_board *board = owner;
    lib_u8 *mapping;
    const lib_status status = myob300_board_management_range(board, address, bytes, &mapping);

    (void)observe_only;
    if (status == LIB_STATUS_OK) {
        lib_memory_copy(destination, mapping, bytes);
        return LIB_STATUS_OK;
    }
    if (status == X86_CORE_MACHINE_SMM_ROUTE_ORDINARY &&
        board->chipset.card.common.data != LIB_NULL) {
        const lib_u32 rom_offset = address >= MYOB300_SYSTEM_ROM_RESET_ALIAS ?
            address - MYOB300_SYSTEM_ROM_RESET_ALIAS :
            address - MYOB300_SYSTEM_ROM_BASE;
        if (rom_offset < MYOB300_CARD_RESET_BYTES &&
            bytes <= MYOB300_CARD_RESET_BYTES - rom_offset) {
            lib_memory_copy(destination,
                board->chipset.card.common.data + MYOB300_CARD_RESET_OFFSET + rom_offset, bytes);
            return LIB_STATUS_OK;
        }
    }
    return status;
}

static lib_status myob300_board_management_write(void *owner, lib_u32 address,
    const void *source, lib_u8 bytes)
{
    myob300_board *board = owner;
    lib_u8 *destination;
    const lib_status status = myob300_board_management_range(board, address, bytes, &destination);

    if (status != LIB_STATUS_OK) return status;
    lib_memory_copy(destination, source, bytes);
    return LIB_STATUS_OK;
}

static void myob300_board_no_action(void *owner)
{
    (void)owner;
}

static lib_bool myob300_board_no_refresh(void *owner, lib_u8 *address)
{
    (void)owner;
    (void)address;
    return LIB_FALSE;
}

static lib_bool myob300_board_no_deadline(void *owner, lib_u64 now, lib_u64 *deadline)
{
    (void)owner;
    (void)now;
    (void)deadline;
    return LIB_FALSE;
}

lib_status myob300_board_set_matrix_key(myob300_board *board, lib_u8 row,
    lib_u8 column, lib_bool pressed)
{
    lib_u16 mask;

    if (board == LIB_NULL || row >= MYOB300_KEYBOARD_ROWS ||
        column >= MYOB300_KEYBOARD_COLUMNS) return LIB_STATUS_INVALID_ARGUMENT;
    mask = (lib_u16)(1u << column);
    if (pressed) board->chipset.keyboard_columns[row] &= (lib_u16)~mask;
    else board->chipset.keyboard_columns[row] |= mask;
    return LIB_STATUS_OK;
}

void myob300_board_set_power_key(myob300_board *board, lib_bool pressed)
{
    myob300_chipset *chipset = &board->chipset;
    const lib_bool rising_edge = pressed && !chipset->power_key_pressed;

    chipset->power_key_pressed = pressed;
    /* L2: the awake switch pulses external SMI through SCAMP SMMEN. The
     * held level remains available to the firmware's wake poll in either mode. */
    if (rising_edge && !chipset->management_active &&
        (chipset->management_control & MYOB300_CHIPSET_SMM_ENABLED) != 0u &&
        x86_core_machine_set_smi(chipset->core, LIB_TRUE) == LIB_STATUS_OK)
        (void)x86_core_machine_set_smi(chipset->core, LIB_FALSE);
}

void myob300_board_receive_mouse(myob300_board *board, lib_i32 dx, lib_i32 dy, lib_u32 buttons)
{
    myob300_mouse_receive(&board->mouse, dx, dy, buttons);
}

lib_status myob300_board_capture_display(void *context, x86_video_snapshot *out_snapshot)
{
    myob300_board *board = context;
    lib_u64 ticks;
    lib_u64 vsync;
    lib_status status;

    if (board == LIB_NULL || out_snapshot == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_core_machine_get_elapsed_ticks(board->chipset.core, &ticks);
    if (status != LIB_STATUS_OK) return status;
    /* L2 nominal panel rate on the existing guest time axis. */
    vsync = (ticks / MYOB300_CPU_CLOCK_HZ) * MYOB300_PANEL_DEFAULT_VSYNC_HZ +
        (ticks % MYOB300_CPU_CLOCK_HZ) * MYOB300_PANEL_DEFAULT_VSYNC_HZ / MYOB300_CPU_CLOCK_HZ;
    return x86_ct_vga_capture_snapshot(board->display, vsync, out_snapshot);
}

static void myob300_board_finalize(void *owner)
{
    myob300_board *board = owner;
    x86_ct_vga_destroy(board->display);
    lib_release(board);
}

static lib_status myob300_board_bind(
    const x86_nxvm_board_profile_services *services, void *owner,
    x86_nxvm_board_profile_binding *out_binding)
{
    myob300_board_setup *setup = owner;
    myob300_board *board = lib_allocate_zero(1u, sizeof(*board));
    const x86_core_machine_port_route parallel = {
        MYOB300_PARALLEL_DATA_PORT, myob300_board_parallel_read,
        myob300_board_parallel_write, board, LIB_FALSE, 0u
    };
    lib_status status;

    if (board == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    board->chipset.card.common = setup->common;
    board->chipset.card.attribute = setup->attribute;
    board->chipset.fixed_disk_card_installed = setup->fixed_disk_card_installed;
    status = LIB_STATUS_OK;
    if (setup->common.data != LIB_NULL) {
        status = x86_core_machine_register_immutable_rom_mapping(services->core,
            MYOB300_SYSTEM_ROM_BASE, setup->common.data + MYOB300_CARD_RESET_OFFSET,
            MYOB300_CARD_RESET_BYTES);
        if (status == LIB_STATUS_OK)
            status = x86_core_machine_register_immutable_rom_mapping_reset_alias(services->core,
                MYOB300_SYSTEM_ROM_BASE, MYOB300_SYSTEM_ROM_RESET_ALIAS,
                MYOB300_CARD_RESET_BYTES);
    }
    if (status == LIB_STATUS_OK) status = x86_ct_vga_create(X86_CT_VGA_65510, &board->display);
    if (status == LIB_STATUS_OK) status = myob300_chipset_install(&board->chipset, services);
    if (status == LIB_STATUS_OK) {
        status = myob300_mouse_install(&board->mouse, services);
        if (status == LIB_STATUS_OK)
            status = x86_core_machine_install_port_routes(services->core, &parallel, 1u);
        if (status == LIB_STATUS_OK)
            status = myob300_card_install(&board->chipset.card, services->core);
        if (status == LIB_STATUS_OK) {
            status = myob300_display_install(services->core, board->display);
            if (status != LIB_STATUS_OK)
                (void)x86_core_machine_remove_memory_device_routes(services->core, &board->chipset.card);
        }
        if (status != LIB_STATUS_OK) {
            (void)x86_core_machine_remove_port_routes(services->core, &board->mouse);
            (void)x86_core_machine_remove_port_routes(services->core, board);
            (void)x86_core_machine_remove_memory_device_routes(services->core, &board->chipset);
            (void)x86_core_machine_remove_port_routes(services->core, &board->chipset);
        }
    }
    if (status != LIB_STATUS_OK) {
        myob300_board_finalize(board);
        return status;
    }
    *out_binding = (x86_nxvm_board_profile_binding){
        .context = board,
        .reset = myob300_board_reset,
        .refresh_nmi = myob300_board_no_action,
        .refresh_request = myob300_board_no_refresh,
        .refresh_complete = myob300_board_no_action,
        .next_deadline = myob300_board_no_deadline,
        .finalize = myob300_board_finalize,
        .system_management_active = myob300_board_management_active,
        .read_smm_memory = myob300_board_management_read,
        .write_smm_memory = myob300_board_management_write
    };
    setup->profile = board;
    return LIB_STATUS_OK;
}

const x86_nxvm_board_plan_topology myob300_board_topology = {
    .dma_present = LIB_TRUE,
    .dma = {X86_NXVM_BOARD_DMA_FDC_CHANNEL_UNBOUND,
        X86_NXVM_BOARD_DMA_CONTROLLER_COUNT, X86_NXVM_BOARD_DMA_CASCADE_CHANNEL},
    .absent_memory_count = 1u,
    .absent_memory = {
        {MYOB300_RAM_BYTES, MYOB300_RAM_PROBE_LIMIT - MYOB300_RAM_BYTES, 0xffu}
    },
    /* SCAMP II REFCTL resets to a 15.625 us interval, before PIT programming. */
    .planar_parity_present = LIB_TRUE,
    .planar_parity = {
        .port = 0x61u,
        .refresh_status_source =
            X86_NXVM_BOARD_PLANAR_PARITY_REFRESH_STATUS_ELAPSED_TICK_TOGGLE,
        .refresh_status_toggle_ticks = MYOB300_REFRESH_TICKS
    },
    .rtc_cmos_present = LIB_TRUE,
    .rtc_cmos = {
        .index_port = 0x70u,
        .data_port = 0x71u,
        .irq = 8u,
        .nmi_mask_bit = 0x80u,
        .extended_registers = LIB_TRUE,
        .ticks_per_second = MYOB300_RTC_INPUT_HZ
    }
};

lib_status myob300_board_configure(void *owner, x86_nxvm_board_plan *plan)
{
    myob300_board_setup *setup = owner;

    if (setup == LIB_NULL ||
        (setup->common.data == LIB_NULL ? setup->common.bytes != 0u :
            setup->common.bytes < MYOB300_CARD_BOOT_BYTES) ||
        ((setup->attribute.data == LIB_NULL) != (setup->attribute.bytes == 0u)))
        return LIB_STATUS_INVALID_ARGUMENT;
    setup->profile = LIB_NULL;
    return x86_nxvm_board_plan_bind_profile_factory(plan, myob300_board_bind, setup);
}
