#include "app-myob300/ob300_board_interface.h"
#include "app-myob300/ob300_chipset.h"
#include "x86-core/machine/memory_interface.h"

#define MYOB300_CPU_CLOCK_HZ 20000000u
#define MYOB300_PIT_INPUT_HZ 1193182u
#define MYOB300_RTC_INPUT_HZ 32768u
#define MYOB300_SYSTEM_ROM_BASE 0x000f8000u
#define MYOB300_SYSTEM_ROM_RESET_ALIAS 0x00ff8000u
#define MYOB300_MANAGEMENT_WINDOW_BASE 0x00060000u
#define MYOB300_MANAGEMENT_MEMORY_BYTES 0x20000u

typedef struct myob300_board {
    myob300_chipset chipset;
    lib_u8 management_memory[MYOB300_MANAGEMENT_MEMORY_BYTES];
    lib_u8 reset_image[MYOB300_SYSTEM_ROM_BYTES];
    lib_bool reset_image_present;
} myob300_board;

const x86_nxvm_board_config myob300_board_configuration = {
    .memory_bytes = 2u * 1024u * 1024u,
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
    if (source == LIB_NULL && board->reset_image_present && bytes != 0u) {
        rom_offset = address >= MYOB300_SYSTEM_ROM_RESET_ALIAS ?
            address - MYOB300_SYSTEM_ROM_RESET_ALIAS :
            address - MYOB300_SYSTEM_ROM_BASE;
        if (rom_offset < MYOB300_SYSTEM_ROM_BYTES &&
            bytes <= MYOB300_SYSTEM_ROM_BYTES - rom_offset)
            source = board->reset_image + rom_offset;
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

static lib_status myob300_board_bind(
    const x86_nxvm_board_profile_services *services, void *owner,
    x86_nxvm_board_profile_binding *out_binding)
{
    myob300_board *board = lib_allocate_zero(1u, sizeof(*board));
    lib_status status;

    if (board == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    status = LIB_STATUS_OK;
    if (owner != LIB_NULL) {
        lib_memory_copy(board->reset_image, owner, sizeof(board->reset_image));
        board->reset_image_present = LIB_TRUE;
        status = x86_core_machine_register_immutable_rom_mapping(services->core,
            MYOB300_SYSTEM_ROM_BASE, board->reset_image, sizeof(board->reset_image));
        if (status == LIB_STATUS_OK)
            status = x86_core_machine_register_immutable_rom_mapping_reset_alias(services->core,
                MYOB300_SYSTEM_ROM_BASE, MYOB300_SYSTEM_ROM_RESET_ALIAS,
                sizeof(board->reset_image));
    }
    if (status == LIB_STATUS_OK) status = myob300_chipset_install(&board->chipset, services->core);
    if (status != LIB_STATUS_OK) {
        lib_release(board);
        return status;
    }
    *out_binding = (x86_nxvm_board_profile_binding){
        .context = board,
        .reset = myob300_board_reset,
        .refresh_nmi = myob300_board_no_action,
        .refresh_request = myob300_board_no_refresh,
        .refresh_complete = myob300_board_no_action,
        .next_deadline = myob300_board_no_deadline,
        .finalize = lib_release,
        .system_management_active = myob300_board_management_active,
        .read_smm_memory = myob300_board_management_read,
        .write_smm_memory = myob300_board_management_write
    };
    return LIB_STATUS_OK;
}

lib_status myob300_board_configure(x86_nxvm_board_plan *plan,
    const lib_u8 *reset_image, lib_size bytes)
{
    const x86_nxvm_board_plan_topology topology = {
        .dma_present = LIB_TRUE,
        .dma = {X86_NXVM_BOARD_DMA_FDC_CHANNEL_UNBOUND,
            X86_NXVM_BOARD_DMA_CONTROLLER_COUNT, X86_NXVM_BOARD_DMA_CASCADE_CHANNEL},
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

    if (reset_image == LIB_NULL ? bytes != 0u : bytes != MYOB300_SYSTEM_ROM_BYTES)
        return LIB_STATUS_INVALID_ARGUMENT;
    status = x86_nxvm_board_plan_set_topology(plan, &topology);
    if (status == LIB_STATUS_OK)
        status = x86_nxvm_board_plan_bind_profile_factory(plan,
            myob300_board_bind, (void *)reset_image);
    return status;
}
