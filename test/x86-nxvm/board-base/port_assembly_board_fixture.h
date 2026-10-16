#ifndef TEST_PORT_ASSEMBLY_BOARD_FIXTURE_H
#define TEST_PORT_ASSEMBLY_BOARD_FIXTURE_H
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/board-base/dma_bus_interface.h"
#include "x86-nxvm/board-base/media_interface.h"

/* Transient copied assertion values; no private layout or persistent mirror. */
typedef struct test_port_assembly_board_observation {
    lib_bool fdc_configured, fdc_zero, fdc_topology_zero;
    lib_bool rtc_configured, rtc_present, rtc_config_zero;
    lib_bool hdc_configured, hdc_zero, hdc_topology_zero, hdc_dma_unbound;
    lib_bool parity_bound, profile_bound;
} test_port_assembly_board_observation;

test_port_assembly_board_observation test_port_assembly_board_capture(
    const x86_nxvm_board_state *board);
void test_port_assembly_reject_ports(lib_bool reject);
lib_bool test_port_assembly_ports_rejected(lib_size count);
lib_size test_port_assembly_submitted_port_count(void);
lib_status test_port_assembly_fdc_setup(x86_nxvm_board_state *board,
    x86_nxvm_board_dma_bus **out_dma, x86_nxvm_board_dma_request_binding *out_request);
lib_status test_port_assembly_fdc_try(x86_nxvm_board_state *board,
    const x86_nxvm_board_media_registry *media,
    const x86_nxvm_board_dma_request_binding *request, lib_bool fail_chip);
lib_status test_port_assembly_hdc_setup(x86_nxvm_board_state *board,
    const x86_nxvm_board_media_registry *media, x86_nxvm_board_hdc_protocol protocol,
    x86_nxvm_board_dma_bus **out_dma);
lib_status test_port_assembly_hdc_try(x86_nxvm_board_state *board,
    const x86_nxvm_board_media_registry *media, x86_nxvm_board_hdc_protocol protocol,
    lib_bool fail_chip);
lib_status test_port_assembly_hdc_address_read(x86_nxvm_board_state *board,
    lib_u32 *out_value);
lib_status test_port_assembly_fdc_address_read(x86_nxvm_board_state *board,
    lib_u32 *out_value);
lib_i32 test_port_assembly_refresh_count(x86_nxvm_board_state *board);
#endif
