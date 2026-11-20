#include "app-myob300/ob300_board_interface.h"
#include "app-myob300/ob300_chipset.h"
#include "app-myob300/ob300_display.h"
#include "x86-core/machine/memory_interface.h"

#define MYOB300_CPU_CLOCK_HZ 20000000u
#define MYOB300_RAM_BYTES 0x200000u
#define MYOB300_RAM_PROBE_LIMIT 0x800000u
#define MYOB300_PIT_INPUT_HZ 1193182u
#define MYOB300_RTC_INPUT_HZ 32768u
#define MYOB300_REFRESH_HZ 64000u
#define MYOB300_REFRESH_TICKS ((MYOB300_CPU_CLOCK_HZ + MYOB300_REFRESH_HZ / 2u) / MYOB300_REFRESH_HZ)
#define MYOB300_SYSTEM_ROM_BASE 0x000f8000u
#define MYOB300_SYSTEM_ROM_RESET_ALIAS 0x00ff8000u
#define MYOB300_MANAGEMENT_WINDOW_BASE 0x00060000u
#define MYOB300_MANAGEMENT_MEMORY_BYTES 0x20000u
#define MYOB300_PANEL_DEFAULT_VSYNC_HZ 60u

struct myob300_board {
    myob300_chipset chipset;
    x86_ct_vga *display;
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
    .display_topology = X86_NXVM_BOARD_DISPLAY_TOPOLOGY_NONE,
    .system_facade = X86_NXVM_BOARD_SYSTEM_FACADE_NONE,
    .system_control_port_present = LIB_TRUE
};

static void myob300_board_reset(void *owner, x86_nxvm_board_reset_phase phase)
{
    myob300_board *board = owner;

    if (phase == X86_NXVM_BOARD_RESET_BEFORE_DEVICES) {
        myob300_chipset_reset(&board->chipset);
        x86_ct_vga_reset(board->display);
        lib_memory_set(board->management_memory, 0, sizeof(board->management_memory));
    }
}

static void myob300_board_management_active(void *owner, lib_bool active)
{
    myob300_board *board = owner;

    board->chipset.management_active = active;
}

static lib_u8 *myob300_board_management_range(myob300_board *board, lib_u32 address,
    lib_u8 bytes)
{
    const lib_u8 control = board->chipset.management_control;
    const lib_u8 selection = control & 7u;
    const lib_size capacity = selection == 7u ? MYOB300_MANAGEMENT_MEMORY_BYTES :
        selection >= 4u ? 0x10000u : 0x8000u;
    const lib_u32 offset = address - MYOB300_MANAGEMENT_WINDOW_BASE;

    if ((control & 0xa0u) != 0xa0u || address < MYOB300_MANAGEMENT_WINDOW_BASE ||
        bytes == 0u || offset >= capacity || bytes > capacity - offset)
        return LIB_NULL;
    return board->management_memory + offset;
}

static lib_status myob300_board_management_read(void *owner, lib_u32 address,
    void *destination, lib_u8 bytes, lib_bool observe_only)
{
    myob300_board *board = owner;
    const lib_u8 *source = myob300_board_management_range(board, address, bytes);
    lib_u32 rom_offset;

    (void)observe_only;
    if (source == LIB_NULL && board->chipset.card.common.data != LIB_NULL && bytes != 0u) {
        rom_offset = address >= MYOB300_SYSTEM_ROM_RESET_ALIAS ?
            address - MYOB300_SYSTEM_ROM_RESET_ALIAS :
            address - MYOB300_SYSTEM_ROM_BASE;
        if (rom_offset < MYOB300_CARD_RESET_BYTES &&
            bytes <= MYOB300_CARD_RESET_BYTES - rom_offset)
            source = board->chipset.card.common.data + MYOB300_CARD_RESET_OFFSET + rom_offset;
    }
    if (source == LIB_NULL) return LIB_STATUS_UNSUPPORTED;
    lib_memory_copy(destination, source, bytes);
    return LIB_STATUS_OK;
}

static lib_status myob300_board_management_write(void *owner, lib_u32 address,
    const void *source, lib_u8 bytes)
{
    myob300_board *board = owner;
    lib_u8 *destination = myob300_board_management_range(board, address, bytes);

    if (destination == LIB_NULL) return LIB_STATUS_UNSUPPORTED;
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
    lib_status status;

    if (board == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    board->chipset.card.common = setup->common;
    board->chipset.card.attribute = setup->attribute;
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
    if (status == LIB_STATUS_OK) status = myob300_chipset_install(&board->chipset, services->core);
    if (status == LIB_STATUS_OK) {
        status = myob300_card_install(&board->chipset.card, services->core);
        if (status == LIB_STATUS_OK) {
            status = myob300_display_install(services->core, board->display);
            if (status != LIB_STATUS_OK)
                (void)x86_core_machine_remove_memory_device_routes(services->core, &board->chipset.card);
        }
        if (status != LIB_STATUS_OK) {
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

lib_status myob300_board_configure(x86_nxvm_board_plan *plan, myob300_board_setup *setup)
{
    const x86_nxvm_board_plan_topology topology = {
        .dma_present = LIB_TRUE,
        .dma = {X86_NXVM_BOARD_DMA_FDC_CHANNEL_UNBOUND,
            X86_NXVM_BOARD_DMA_CONTROLLER_COUNT, X86_NXVM_BOARD_DMA_CASCADE_CHANNEL},
        .absent_memory_count = 1u,
        .absent_memory = {
            {MYOB300_RAM_BYTES, MYOB300_RAM_PROBE_LIMIT - MYOB300_RAM_BYTES, 0xffu}
        },
        /* SCAMP II REFCTL resets to a 15.625 us interval. Port B observes
         * that cadence before the guest programs PIT counter 1. */
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
            .ticks_per_second = MYOB300_RTC_INPUT_HZ
        }
    };
    lib_status status;

    if (setup == LIB_NULL ||
        (setup->common.data == LIB_NULL ? setup->common.bytes != 0u :
            setup->common.bytes < MYOB300_CARD_BOOT_BYTES) ||
        ((setup->attribute.data == LIB_NULL) != (setup->attribute.bytes == 0u)))
        return LIB_STATUS_INVALID_ARGUMENT;
    setup->profile = LIB_NULL;
    status = x86_nxvm_board_plan_set_topology(plan, &topology);
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_board_plan_bind_profile_factory(plan,
            myob300_board_bind, setup);
    return status;
}
