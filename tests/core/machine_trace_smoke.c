#include "type.h"



#include "core/machine/machine_interface.h"
#include "../support/core_machine_executor_fixture.h"

typedef struct trace_fixture {
    core_machine_trace_event events[8];
    C_UINT count;
    C_UINT port_value;
    type_status read_status;
} trace_fixture;

static C_VOID trace_callback(C_VOID *context, const core_machine_trace_event *event)
{
    trace_fixture *fixture = (trace_fixture *)context;

    if (fixture->count < 8u) {
        fixture->events[fixture->count++] = *event;
    }
}

static type_status port_read(C_VOID *owner, type_unsigned_16 port, type_unsigned_32 *out_value)
{
    trace_fixture *fixture = (trace_fixture *)owner;

    (C_VOID)port;
    if (fixture->read_status != TYPE_STATUS_OK) return fixture->read_status;
    *out_value = fixture->port_value;
    return TYPE_STATUS_OK;
}

static type_status port_write(C_VOID *owner, type_unsigned_16 port, type_unsigned_32 value)
{
    trace_fixture *fixture = (trace_fixture *)owner;

    (C_VOID)port;
    fixture->port_value = value;
    return TYPE_STATUS_OK;
}

static C_INT expect_status(type_status actual, type_status expected)
{
    return actual == expected ? 0 : 1;
}

C_INT main(C_VOID)
{
    core_machine *machine = STD_NULL;
    core_machine_trace_provider sink;
    core_machine_port_provider port_ops = { port_read, port_write };
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    trace_fixture fixture = { { { 0 } }, 0u, 0u, TYPE_STATUS_OK };
    type_unsigned_32 value;
    C_INT failed = 0;

    sink.callback = trace_callback;
    sink.context = &fixture;
    failed |= expect_status(test_core_machine_create_executor(0u, &machine),
                            TYPE_STATUS_OK);
    failed |= expect_status(core_machine_set_trace_provider(machine, &sink),
                            TYPE_STATUS_OK);
    failed |= expect_status(core_machine_install_port_provider(
                                machine, 0x60u, 0x60u, &port_ops, &fixture),
                            TYPE_STATUS_OK);
    failed |= expect_status(core_machine_freeze_execution_providers(machine),
                            TYPE_STATUS_OK);
    failed |= expect_status(core_machine_reset(machine), TYPE_STATUS_OK);
    failed |= expect_status(core_machine_bus_write(machine, 0x60u, 0x55u),
                            TYPE_STATUS_OK);
    failed |= expect_status(core_machine_bus_read(machine, 0x60u, &value),
                            TYPE_STATUS_OK);
    failed |= value != 0x55u;
    failed |= expect_status(core_machine_request_stop(machine),
                            TYPE_STATUS_OK);
    failed |= expect_status(core_machine_run(machine, budget, &result),
                            TYPE_STATUS_OK);
    failed |= result.reason != CORE_MACHINE_STOP_REQUESTED;
    failed |= fixture.count != 5u ||
              fixture.events[0].type != CORE_MACHINE_TRACE_RESET ||
              fixture.events[1].type != CORE_MACHINE_TRACE_PORT_WRITE ||
              fixture.events[1].address != 0x60u ||
              fixture.events[1].value != 0x55u ||
              fixture.events[2].type != CORE_MACHINE_TRACE_PORT_READ ||
              fixture.events[3].type != CORE_MACHINE_TRACE_RESET ||
              fixture.events[4].type != CORE_MACHINE_TRACE_STOP ||
              fixture.events[4].detail != CORE_MACHINE_STOP_REQUESTED ||
              fixture.events[4].sequence != 4u;
    failed |= expect_status(core_machine_reset(machine), TYPE_STATUS_OK);
    fixture.count = 0u;
    failed |= expect_status(core_machine_report_fault(machine, 0x44u),
                            TYPE_STATUS_OK);
    failed |= fixture.count != 1u ||
              fixture.events[0].type != CORE_MACHINE_TRACE_FAULT ||
              fixture.events[0].detail != 0x44u;
    failed |= expect_status(core_machine_reset(machine), TYPE_STATUS_OK);
    fixture.count = 0u;
    fixture.read_status = TYPE_STATUS_FAULT;
    value = 0xdeadbeefu;
    failed |= expect_status(core_machine_bus_read(machine, 0x60u, &value),
                            TYPE_STATUS_FAULT);
    failed |= value != 0xdeadbeefu || fixture.count != 1u ||
              fixture.events[0].type != CORE_MACHINE_TRACE_PORT_READ ||
              fixture.events[0].address != 0x60u ||
              fixture.events[0].value != 0u ||
              fixture.events[0].detail != TYPE_STATUS_FAULT;
    fixture.read_status = TYPE_STATUS_OK;
    failed |= expect_status(core_machine_bus_read(machine, 0x60u, &value),
                            TYPE_STATUS_OK) || fixture.count != 2u ||
              fixture.events[1].detail != TYPE_STATUS_OK;
    failed |= expect_status(core_machine_set_trace_provider(machine, STD_NULL),
                            TYPE_STATUS_OK);
    failed |= expect_status(core_machine_reset(machine), TYPE_STATUS_OK);
    failed |= fixture.count != 3u;

    core_machine_destroy(machine);
    if (failed != 0) return 1;

    puts("M3:T2:S2:TRACE:OK");
    return 0;
}
