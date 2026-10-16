#include "board_binding_fixture.h"
#include "core/board-base/machine_board_state.h"

static x86_core_machine_attachment *captured_binding;
static lib_size *constructor_attempts;
static lib_u32 rejected_device;
static lib_bool rejection_reached;
static lib_size *destroyed_machines;
static lib_bool rejected_ports;
static lib_bool port_rejection_reached;
static lib_size submitted_ports;
static x86_core_machine_executor_config *captured_executor;
static lib_size *captured_constructor_count;

void test_board_observe_constructor(x86_core_machine_executor_config *config,
    lib_size *count)
{
    captured_executor = config;
    captured_constructor_count = count;
}

void test_board_reject_ports(lib_bool reject)
{
    rejected_ports = reject;
    port_rejection_reached = LIB_FALSE;
    submitted_ports = 0u;
}

lib_bool test_board_ports_rejected(lib_size count)
{
    return port_rejection_reached && submitted_ports == count;
}

lib_size test_board_submitted_port_count(void)
{
    return submitted_ports;
}

static void board_test_destroy(x86_core_machine *machine)
{
    if (destroyed_machines != LIB_NULL) ++*destroyed_machines;
    x86_core_machine_destroy(machine);
}

static lib_status board_test_dma(core_machine_dma_bus **out_bus,
    x86_core_machine *machine, lib_u8 count)
{
    if (rejected_device == 1u) {
        rejection_reached = LIB_TRUE;
        *out_bus = LIB_NULL;
        return LIB_STATUS_NO_MEMORY;
    }
    return core_machine_dma_initialize(out_bus, machine, count);
}

static lib_status board_test_pic(core_machine_pic_bus **out_master,
    core_machine_pic_bus **out_slave, x86_core_machine *machine,
    core_machine_pic_topology topology)
{
    if (rejected_device == 2u) {
        rejection_reached = LIB_TRUE;
        *out_master = LIB_NULL;
        *out_slave = LIB_NULL;
        return LIB_STATUS_NO_MEMORY;
    }
    return core_machine_pic_initialize(out_master, out_slave, machine, topology);
}

static lib_status board_test_pit(x86_core_machine *machine,
    x86_pit *pit, lib_u16 base)
{
    if ((rejected_device == 3u && base == 0x40u) ||
        (rejected_device == 7u && base != 0x40u)) {
        rejection_reached = LIB_TRUE;
        return LIB_STATUS_NO_MEMORY;
    }
    return core_machine_pit_install_ports(machine, pit, base);
}

static lib_status board_test_video(x86_core_machine *machine, t_vadp **out_adapter)
{
    if (rejected_device == 4u) {
        rejection_reached = LIB_TRUE;
        *out_adapter = LIB_NULL;
        return LIB_STATUS_NO_MEMORY;
    }
    return core_machine_vadp_create(machine, out_adapter);
}

static lib_status board_test_kbc(x86_core_machine *machine, t_kbc **out_kbc)
{
    if (rejected_device == 5u) {
        rejection_reached = LIB_TRUE;
        *out_kbc = LIB_NULL;
        return LIB_STATUS_NO_MEMORY;
    }
    return core_machine_kbc_create(machine, out_kbc);
}

static lib_status board_test_xt_ppi(const core_machine_xt_ppi_keyboard_config *config,
    x86_core_machine *machine, core_machine_xt_ppi_keyboard **out_keyboard)
{
    if (rejected_device == 6u) {
        rejection_reached = LIB_TRUE;
        *out_keyboard = LIB_NULL;
        return LIB_STATUS_NO_MEMORY;
    }
    return core_machine_xt_ppi_keyboard_create(config, machine, out_keyboard);
}

static lib_status board_test_ports(x86_core_machine *machine,
    const x86_core_machine_port_route *routes, lib_size count)
{
    submitted_ports = count;
    if (rejected_ports) {
        port_rejection_reached = LIB_TRUE;
        return LIB_STATUS_NO_MEMORY;
    }
    if (rejected_device == 8u) {
        rejection_reached = LIB_TRUE;
        return LIB_STATUS_NO_MEMORY;
    }
    return x86_core_machine_install_port_routes(machine, routes, count);
}

static lib_status board_test_construct(
    const x86_core_machine_executor_config *config,
    x86_core_machine **out_machine)
{
    if (captured_executor != LIB_NULL) *captured_executor = *config;
    if (captured_constructor_count != LIB_NULL) ++*captured_constructor_count;
    if (constructor_attempts != LIB_NULL) {
        ++*constructor_attempts;
        *out_machine = LIB_NULL;
        return LIB_STATUS_NO_MEMORY;
    }
    return x86_core_machine_neutral_create(config, out_machine);
}

static lib_status board_test_bind(x86_core_machine *machine,
    const x86_core_machine_attachment *attachment)
{
    const lib_status status = x86_core_machine_bind_attachment(machine, attachment);

    if (status == LIB_STATUS_OK && captured_binding != LIB_NULL)
        *captured_binding = *attachment;
    return status;
}

/* Same-owner observation of the actual public bind call. Machine's own
 * attachment test proves its stored copy, including the context lifetime. */
#define x86_core_machine_bind_attachment board_test_bind
#define x86_core_machine_neutral_create board_test_construct
#define x86_core_machine_destroy board_test_destroy
#define core_machine_dma_initialize board_test_dma
#define core_machine_pic_initialize board_test_pic
#define core_machine_pit_install_ports board_test_pit
#define core_machine_vadp_create board_test_video
#define core_machine_kbc_create board_test_kbc
#define core_machine_xt_ppi_keyboard_create board_test_xt_ppi
#define x86_core_machine_install_port_routes board_test_ports
#include "core/board-base/machine_board.c"
#undef x86_core_machine_install_port_routes
#undef core_machine_xt_ppi_keyboard_create
#undef core_machine_kbc_create
#undef core_machine_vadp_create
#undef core_machine_pit_install_ports
#undef core_machine_pic_initialize
#undef core_machine_dma_initialize
#undef x86_core_machine_destroy
#undef x86_core_machine_neutral_create
#undef x86_core_machine_bind_attachment

lib_i32 test_board_device_failure(const core_machine_config *config,
    lib_u32 dependency)
{
    x86_core_machine *machine = (x86_core_machine *)(lib_uptr)1u;
    core_machine_board_state *board = (core_machine_board_state *)(lib_uptr)1u;
    lib_size destroyed = 0u;
    lib_status status;
    lib_i32 failed;

    rejected_device = dependency;
    rejection_reached = LIB_FALSE;
    destroyed_machines = &destroyed;
    status = core_machine_create(config, &machine, &board);
    destroyed_machines = LIB_NULL;
    rejected_device = 0u;
    failed = status != LIB_STATUS_NO_MEMORY || machine != LIB_NULL ||
        board != LIB_NULL || !rejection_reached || destroyed != 1u;
    if (status == LIB_STATUS_OK) x86_core_machine_destroy(machine);
    return failed;
}

lib_status test_board_constructor_failure(const core_machine_config *config,
    x86_core_machine **out_machine, core_machine_board_state **out_board,
    lib_size *out_attempts)
{
    lib_status status;

    *out_attempts = 0u;
    constructor_attempts = out_attempts;
    status = core_machine_create(config, out_machine, out_board);
    constructor_attempts = LIB_NULL;
    return status;
}

lib_status test_board_binding_create(const core_machine_config *config,
    x86_core_machine **out_machine, x86_core_machine_attachment *out_expected)
{
    core_machine_board_state *board = LIB_NULL;
    lib_status status;
    if (out_expected == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    lib_memory_set(out_expected, 0, sizeof(*out_expected));
    core_machine_board_refresh_nmi(LIB_NULL);
    captured_binding = out_expected;
    status = core_machine_create(config, out_machine, &board);
    captured_binding = LIB_NULL;
    if (status != LIB_STATUS_OK) return status;
    if (board->core != *out_machine || out_expected->context != board ||
        out_expected->reset_devices != core_machine_board_reset_devices ||
        out_expected->reset_clocks != core_machine_board_reset_clocks ||
        out_expected->refresh_nmi != core_machine_board_refresh_nmi ||
        out_expected->finalize_devices != core_machine_board_finalize_devices) {
        x86_core_machine_destroy(*out_machine);
        *out_machine = LIB_NULL;
        return LIB_STATUS_INTERNAL_ERROR;
    }
    return LIB_STATUS_OK;
}
