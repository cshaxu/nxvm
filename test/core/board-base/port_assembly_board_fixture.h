#ifndef TEST_PORT_ASSEMBLY_BOARD_FIXTURE_H
#define TEST_PORT_ASSEMBLY_BOARD_FIXTURE_H
#include "core/board-base/machine_board_interface.h"
#include "core/board-base/dma_bus_interface.h"
#include "core/board-base/media_interface.h"

/* Transient copied assertion values; no private layout or persistent mirror. */
typedef struct test_port_assembly_board_observation {
    lib_bool fdc_configured, fdc_zero, fdc_topology_zero;
    lib_bool rtc_configured, rtc_present, rtc_config_zero;
    lib_bool hdc_configured, hdc_zero, hdc_topology_zero, hdc_dma_unbound;
    lib_bool parity_bound, profile_bound;
} test_port_assembly_board_observation;

test_port_assembly_board_observation test_port_assembly_board_capture(
    const core_machine_board_state *board);
lib_status test_port_assembly_fdc_setup(core_machine_board_state *board,
    core_machine_dma_bus **out_dma, core_machine_dma_request_binding *out_request);
lib_status test_port_assembly_fdc_try(core_machine_board_state *board,
    const core_machine_media_registry *media,
    const core_machine_dma_request_binding *request, lib_bool fail_chip);
lib_status test_port_assembly_hdc_setup(core_machine_board_state *board,
    const core_machine_media_registry *media, core_machine_hdc_protocol protocol,
    core_machine_dma_bus **out_dma);
lib_status test_port_assembly_hdc_try(core_machine_board_state *board,
    const core_machine_media_registry *media, core_machine_hdc_protocol protocol,
    lib_bool fail_chip);
lib_status test_port_assembly_hdc_address_read(core_machine_board_state *board,
    lib_u32 *out_value);
lib_i32 test_port_assembly_refresh_count(core_machine_board_state *board);
#endif
