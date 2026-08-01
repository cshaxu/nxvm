#include <stdio.h>

#include "machine/core/machine.h"

typedef struct trace_fixture {
    nxvm_core_trace_event events[8];
    unsigned count;
    unsigned port_value;
} trace_fixture;

static void trace_callback(void *context, const nxvm_core_trace_event *event)
{
    trace_fixture *fixture = (trace_fixture *)context;

    if (fixture->count < 8u) {
        fixture->events[fixture->count++] = *event;
    }
}

static nxvm_core_status port_read(void *owner, uint16_t port, uint32_t *out_value)
{
    trace_fixture *fixture = (trace_fixture *)owner;

    (void)port;
    *out_value = fixture->port_value;
    return NXVM_CORE_STATUS_OK;
}

static nxvm_core_status port_write(void *owner, uint16_t port, uint32_t value)
{
    trace_fixture *fixture = (trace_fixture *)owner;

    (void)port;
    fixture->port_value = value;
    return NXVM_CORE_STATUS_OK;
}

static int expect_status(nxvm_core_status actual, nxvm_core_status expected)
{
    return actual == expected ? 0 : 1;
}

int main(void)
{
    nxvm_core_machine *machine = NULL;
    nxvm_core_machine_config config = {
        NXVM_CORE_ABI_VERSION, NXVM_CORE_PROFILE_TEST_MINIMAL, 0u
    };
    nxvm_core_trace_sink sink;
    nxvm_core_port_ops port_ops = { port_read, port_write };
    nxvm_core_run_budget budget = { 1u, 0u };
    nxvm_core_run_result result;
    trace_fixture fixture = { { { 0 } }, 0u, 0u };
    uint32_t value;
    int failed = 0;

    sink.callback = trace_callback;
    sink.context = &fixture;
    failed |= expect_status(nxvm_core_machine_create(&config, &machine),
                            NXVM_CORE_STATUS_OK);
    failed |= expect_status(nxvm_core_machine_set_trace_sink(machine, &sink),
                            NXVM_CORE_STATUS_OK);
    failed |= expect_status(nxvm_core_machine_reset(machine), NXVM_CORE_STATUS_OK);
    failed |= expect_status(nxvm_core_machine_install_port(
                                machine, 0x60u, 0x60u, &port_ops, &fixture),
                            NXVM_CORE_STATUS_OK);
    failed |= expect_status(nxvm_core_machine_port_write(machine, 0x60u, 0x55u),
                            NXVM_CORE_STATUS_OK);
    failed |= expect_status(nxvm_core_machine_port_read(machine, 0x60u, &value),
                            NXVM_CORE_STATUS_OK);
    failed |= value != 0x55u;
    failed |= expect_status(nxvm_core_machine_run(machine, budget, &result),
                            NXVM_CORE_STATUS_OK);
    failed |= result.reason != NXVM_CORE_STOP_BUDGET;
    failed |= expect_status(nxvm_core_machine_request_stop(machine),
                            NXVM_CORE_STATUS_OK);
    failed |= expect_status(nxvm_core_machine_run(machine, budget, &result),
                            NXVM_CORE_STATUS_OK);
    failed |= result.reason != NXVM_CORE_STOP_REQUESTED;
    failed |= fixture.count != 5u ||
              fixture.events[0].type != NXVM_CORE_TRACE_RESET ||
              fixture.events[1].type != NXVM_CORE_TRACE_PORT_WRITE ||
              fixture.events[1].address != 0x60u ||
              fixture.events[1].value != 0x55u ||
              fixture.events[2].type != NXVM_CORE_TRACE_PORT_READ ||
              fixture.events[3].type != NXVM_CORE_TRACE_RUN_BOUNDARY ||
              fixture.events[3].detail != NXVM_CORE_STOP_BUDGET ||
              fixture.events[4].type != NXVM_CORE_TRACE_STOP ||
              fixture.events[4].detail != NXVM_CORE_STOP_REQUESTED ||
              fixture.events[4].sequence != 4u;
    failed |= expect_status(nxvm_core_machine_reset(machine), NXVM_CORE_STATUS_OK);
    fixture.count = 0u;
    failed |= expect_status(nxvm_core_machine_report_fault(machine, 0x44u),
                            NXVM_CORE_STATUS_OK);
    failed |= fixture.count != 1u ||
              fixture.events[0].type != NXVM_CORE_TRACE_FAULT ||
              fixture.events[0].detail != 0x44u;
    failed |= expect_status(nxvm_core_machine_set_trace_sink(machine, NULL),
                            NXVM_CORE_STATUS_OK);
    failed |= expect_status(nxvm_core_machine_reset(machine), NXVM_CORE_STATUS_OK);
    failed |= fixture.count != 1u;

    nxvm_core_machine_destroy(machine);
    if (failed != 0) {
        return 1;
    }

    puts("M3:T2:S2:TRACE:OK");
    return 0;
}
