#include "type.h"

#include "core/machine/debug_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/port.h"

#define CHECKPOINTS 64u

static C_INT timing_checkpoint_run(core_machine *machine,
    const uint8_t *program, uint8_t *statuses)
{
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    t_port *port;
    uint32_t index;

    if (core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0xfffffff0u, program, CHECKPOINTS) !=
            TYPE_STATUS_OK) {
        STD_FPRINTF(STD_STDERR, "T221 setup failed\n");
        return 1;
    }
    for (index = 0u; index < CHECKPOINTS; ++index) {
        if (core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            result.ticks != 1u || result.elapsed_ticks != index + 1u) {
            STD_FPRINTF(STD_STDERR,
                "T221 run failed index=%u reason=%d executed=%llu ticks=%llu elapsed=%llu\n",
                index, (C_INT)result.reason, (unsigned long long)result.executed,
                (unsigned long long)result.ticks,
                (unsigned long long)result.elapsed_ticks);
            return 1;
        }
        port = core_machine_debug_port_borrow(machine);
        if (port == STD_NULL) {
            STD_FPRINTF(STD_STDERR, "T221 port failed index=%u\n", index);
            return 1;
        }
        statuses[index] = core_machine_port_read(port, 0x03dau);
    }
    return 0;
}

C_INT main(C_VOID)
{
    core_machine *machine = STD_NULL;
    core_machine_config config = { 0 };
    uint8_t program[CHECKPOINTS];
    uint8_t first[CHECKPOINTS];
    uint8_t second[CHECKPOINTS];
    C_INT failed = 0;

    STD_MEMSET(program, 0x90, sizeof(program));
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= core_machine_memory_register_mapping(
        core_machine_configuration_memory_borrow(machine), 0xfffffff0u,
        0x000ffff0u, CHECKPOINTS) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= timing_checkpoint_run(machine, program, first);
    failed |= timing_checkpoint_run(machine, program, second);
    failed |= STD_MEMCMP(first, second, sizeof(first)) != 0;

    failed |= first[0u] != 0x01u || first[46u] != 0x01u ||
        first[47u] != 0u || first[54u] != 0u || first[55u] != 0x08u ||
        first[62u] != 0x08u || first[63u] != 0x01u;

    if (failed) {
        STD_FPRINTF(STD_STDERR,
            "M5:T221:S2:TIMING-CHECKPOINT:FAIL first=%u,%u,%u,%u,%u,%u,%u "
            "second=%u,%u,%u,%u,%u,%u,%u\n",
            first[0u], first[46u], first[47u], first[54u], first[55u],
            first[62u], first[63u], second[0u], second[46u], second[47u],
            second[54u], second[55u], second[62u], second[63u]);
        core_machine_destroy(machine);
        return 1;
    }
    core_machine_destroy(machine);
    STD_PRINTF("M5:T221:S2:TIMING-CHECKPOINT:OK\n");
    return 0;
}
