#ifndef TEST_CORE_PORT_ASSEMBLY_FIXTURE_H
#define TEST_CORE_PORT_ASSEMBLY_FIXTURE_H
#include "lib/types/types_interface.h"
#include "ibmpc/board-common/machine_board_interface.h"

typedef struct port_assembly_probe_state {
    lib_u32 value;
    lib_u64 read_tick;
    lib_u32 reads;
    lib_bool inconsistent_tick;
} port_assembly_probe_state;

lib_status port_assembly_read(void *owner, lib_u16 port, lib_u64 tick,
    lib_u32 *out_value);
lib_status port_assembly_write(void *owner, lib_u16 port, lib_u32 value);

lib_i32 port_assembly_fresh_default_create(void);
lib_i32 port_assembly_range_transaction(void);
lib_i32 port_assembly_batch_transaction(void);
lib_i32 port_assembly_read_time(void);
lib_i32 port_assembly_port_b_time(void);
lib_i32 port_assembly_dma_byte_lanes(void);
lib_i32 port_assembly_create_failure(void);
lib_i32 port_assembly_pit_transaction(void);
lib_i32 port_assembly_pic_transaction(void);
typedef lib_status (*test_port_assembly_profile_attach)(
    core_machine_board_state *board);
lib_i32 port_assembly_fdc_transaction(lib_size fail_at);
lib_i32 port_assembly_rtc_transaction(lib_size fail_at);
lib_i32 port_assembly_rtc_collision(void);
lib_i32 port_assembly_port_b_transaction(test_port_assembly_profile_attach attach,
    lib_size fail_at);
lib_i32 port_assembly_hdc_transaction(core_machine_hdc_protocol protocol,
    lib_size fail_at, lib_bool busy_dma, lib_bool busy_port);
#endif
