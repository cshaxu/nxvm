#include "type.h"

#include "core/machine/debug_interface.h"
#include "core/machine/machine.h"

static type_status core_machine_debug_require_boundary(
    const core_machine *machine)
{
    core_machine_lifecycle lifecycle;
    type_status status;

    status = core_machine_get_lifecycle(machine, &lifecycle);
    if (status != TYPE_STATUS_OK) {
        return status;
    }
    return lifecycle == CORE_MACHINE_PAUSED || lifecycle == CORE_MACHINE_STOPPED ||
           lifecycle == CORE_MACHINE_FAULTED ? TYPE_STATUS_OK :
                                                TYPE_STATUS_INVALID_STATE;
}

type_status core_machine_debug_read_cpu(
    const core_machine *machine,
    core_machine_cpu_state *out_state)
{
    type_status status = core_machine_debug_require_boundary(machine);

    return status == TYPE_STATUS_OK ?
               core_machine_get_cpu_state(machine, out_state) : status;
}

type_status core_machine_debug_read_memory(
    const core_machine *machine,
    uint32_t physical,
    C_VOID *out_data,
    STD_SIZE_T size)
{
    type_status status = core_machine_debug_require_boundary(machine);

    return status == TYPE_STATUS_OK ?
               core_machine_memory_read(machine, physical, out_data, size) :
               status;
}

type_status core_machine_debug_step(
    core_machine *machine,
    core_machine_run_result *out_result)
{
    const core_machine_run_budget budget = { 1u, 0u };
    type_status status = core_machine_debug_require_boundary(machine);

    return status == TYPE_STATUS_OK ?
               core_machine_run(machine, budget, out_result) : status;
}

type_status core_machine_debug_continue(
    core_machine *machine,
    core_machine_run_budget budget,
    core_machine_run_result *out_result)
{
    type_status status = core_machine_debug_require_boundary(machine);

    return status == TYPE_STATUS_OK ?
               core_machine_run(machine, budget, out_result) : status;
}

type_status core_machine_debug_capture_instruction_observation(
    const core_machine *machine,
    core_machine_debug_instruction_observation *out_observation)
{
    type_status status = core_machine_debug_require_boundary(machine);

    if (status != TYPE_STATUS_OK || out_observation == STD_NULL) return
        status == TYPE_STATUS_OK ? TYPE_STATUS_INVALID_ARGUMENT : status;
    out_observation->cpu = machine->executor_cpu;
    out_observation->instructions = machine->executor_cpu_instructions;
    return TYPE_STATUS_OK;
}

type_status core_machine_debug_read_register(const core_machine *machine,
    core_machine_debug_register register_id, uint32_t *out_value)
{
    type_status status = core_machine_debug_require_boundary(machine);
    const t_cpu *cpu;

    if (status != TYPE_STATUS_OK || out_value == STD_NULL) return
        status == TYPE_STATUS_OK ? TYPE_STATUS_INVALID_ARGUMENT : status;
    cpu = &machine->executor_cpu;
    switch (register_id) {
    case CORE_MACHINE_DEBUG_EAX: *out_value = cpu->data.eax; break;
    case CORE_MACHINE_DEBUG_ECX: *out_value = cpu->data.ecx; break;
    case CORE_MACHINE_DEBUG_EDX: *out_value = cpu->data.edx; break;
    case CORE_MACHINE_DEBUG_EBX: *out_value = cpu->data.ebx; break;
    case CORE_MACHINE_DEBUG_ESP: *out_value = cpu->data.esp; break;
    case CORE_MACHINE_DEBUG_EBP: *out_value = cpu->data.ebp; break;
    case CORE_MACHINE_DEBUG_ESI: *out_value = cpu->data.esi; break;
    case CORE_MACHINE_DEBUG_EDI: *out_value = cpu->data.edi; break;
    case CORE_MACHINE_DEBUG_EIP: *out_value = cpu->data.eip; break;
    case CORE_MACHINE_DEBUG_EFLAGS: *out_value = cpu->data.eflags; break;
    case CORE_MACHINE_DEBUG_ES: *out_value = cpu->data.es.selector; break;
    case CORE_MACHINE_DEBUG_CS: *out_value = cpu->data.cs.selector; break;
    case CORE_MACHINE_DEBUG_SS: *out_value = cpu->data.ss.selector; break;
    case CORE_MACHINE_DEBUG_DS: *out_value = cpu->data.ds.selector; break;
    case CORE_MACHINE_DEBUG_FS: *out_value = cpu->data.fs.selector; break;
    case CORE_MACHINE_DEBUG_GS: *out_value = cpu->data.gs.selector; break;
    case CORE_MACHINE_DEBUG_CR0: *out_value = cpu->data.cr0; break;
    case CORE_MACHINE_DEBUG_CR1: *out_value = cpu->data.cr1; break;
    case CORE_MACHINE_DEBUG_CR2: *out_value = cpu->data.cr2; break;
    case CORE_MACHINE_DEBUG_CR3: *out_value = cpu->data.cr3; break;
    case CORE_MACHINE_DEBUG_CR4: *out_value = cpu->data.cr4; break;
    default: return TYPE_STATUS_INVALID_ARGUMENT;
    }
    return TYPE_STATUS_OK;
}

type_status core_machine_debug_write_register(core_machine *machine,
    core_machine_debug_register register_id, uint32_t value)
{
    core_machine_debug_register_patch patch = {0};

    if (register_id >= CORE_MACHINE_DEBUG_REGISTER_COUNT)
        return TYPE_STATUS_INVALID_ARGUMENT;
    patch.mask = CORE_MACHINE_DEBUG_REGISTER_MASK(register_id);
    patch.values[register_id] = value;
    return core_machine_debug_patch_registers(machine, &patch);
}

static C_INT core_machine_debug_patch_segment(
    core_machine_cpu_execution_context *context, t_cpu *cpu,
    core_machine_debug_register register_id, uint32_t value)
{
    switch (register_id) {
    case CORE_MACHINE_DEBUG_ES:
        return core_machine_cpu_execution_load_segment(context, &cpu->data.es,
            (uint16_t)value);
    case CORE_MACHINE_DEBUG_CS:
        return core_machine_cpu_execution_load_segment(context, &cpu->data.cs,
            (uint16_t)value);
    case CORE_MACHINE_DEBUG_SS:
        return core_machine_cpu_execution_load_segment(context, &cpu->data.ss,
            (uint16_t)value);
    case CORE_MACHINE_DEBUG_DS:
        return core_machine_cpu_execution_load_segment(context, &cpu->data.ds,
            (uint16_t)value);
    case CORE_MACHINE_DEBUG_FS:
        return core_machine_cpu_execution_load_segment(context, &cpu->data.fs,
            (uint16_t)value);
    case CORE_MACHINE_DEBUG_GS:
        return core_machine_cpu_execution_load_segment(context, &cpu->data.gs,
            (uint16_t)value);
    default: return 0;
    }
}

type_status core_machine_debug_patch_registers(core_machine *machine,
    const core_machine_debug_register_patch *patch)
{
    const uint32_t valid_mask =
        (1u << CORE_MACHINE_DEBUG_REGISTER_COUNT) - 1u;
    core_machine_cpu_execution_context candidate_context;
    t_cpu candidate_cpu;
    type_status status = core_machine_debug_require_boundary(machine);
    core_machine_debug_register register_id;

    if (status != TYPE_STATUS_OK || patch == STD_NULL) return
        status == TYPE_STATUS_OK ? TYPE_STATUS_INVALID_ARGUMENT : status;
    if (patch->mask == 0u || (patch->mask & ~valid_mask) != 0u)
        return TYPE_STATUS_INVALID_ARGUMENT;
    candidate_cpu = machine->executor_cpu;
    candidate_context = machine->executor_cpu_execution;
    candidate_context.cpu = &candidate_cpu;
    for (register_id = CORE_MACHINE_DEBUG_EAX;
         register_id < CORE_MACHINE_DEBUG_REGISTER_COUNT; ++register_id) {
        if ((patch->mask & CORE_MACHINE_DEBUG_REGISTER_MASK(register_id)) == 0u)
            continue;
        switch (register_id) {
        case CORE_MACHINE_DEBUG_EAX: candidate_cpu.data.eax = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_ECX: candidate_cpu.data.ecx = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_EDX: candidate_cpu.data.edx = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_EBX: candidate_cpu.data.ebx = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_ESP: candidate_cpu.data.esp = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_EBP: candidate_cpu.data.ebp = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_ESI: candidate_cpu.data.esi = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_EDI: candidate_cpu.data.edi = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_EIP: candidate_cpu.data.eip = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_EFLAGS: candidate_cpu.data.eflags = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_CR0: candidate_cpu.data.cr0 = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_CR1: candidate_cpu.data.cr1 = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_CR2: candidate_cpu.data.cr2 = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_CR3: candidate_cpu.data.cr3 = patch->values[register_id]; break;
        case CORE_MACHINE_DEBUG_CR4: candidate_cpu.data.cr4 = patch->values[register_id]; break;
        default:
            if (core_machine_debug_patch_segment(&candidate_context,
                    &candidate_cpu, register_id, patch->values[register_id]))
                return TYPE_STATUS_INVALID_STATE;
            break;
        }
    }
    machine->executor_cpu = candidate_cpu;
    return TYPE_STATUS_OK;
}

type_status core_machine_debug_get_code_default_size(const core_machine *machine,
    C_INT *out_default_size)
{
    type_status status = core_machine_debug_require_boundary(machine);
    if (status != TYPE_STATUS_OK || out_default_size == STD_NULL) return
        status == TYPE_STATUS_OK ? TYPE_STATUS_INVALID_ARGUMENT : status;
    *out_default_size = core_machine_cpu_get_code_default_size(
        &machine->executor_cpu_execution);
    return TYPE_STATUS_OK;
}

type_status core_machine_debug_get_code_base(const core_machine *machine,
    uint32_t *out_base)
{
    type_status status = core_machine_debug_require_boundary(machine);
    if (status != TYPE_STATUS_OK || out_base == STD_NULL) return
        status == TYPE_STATUS_OK ? TYPE_STATUS_INVALID_ARGUMENT : status;
    *out_base = core_machine_cpu_get_code_base(&machine->executor_cpu_execution);
    return TYPE_STATUS_OK;
}

type_status core_machine_debug_read_linear(core_machine *machine, uint32_t address,
    C_VOID *out_data, uint8_t size)
{
    type_status status = core_machine_debug_require_boundary(machine);
    if (status != TYPE_STATUS_OK) return status;
    return core_machine_cpu_read_linear(&machine->executor_cpu_execution, address,
        out_data, size) == 0 ? TYPE_STATUS_OK : TYPE_STATUS_INVALID_STATE;
}

type_status core_machine_debug_write_linear(core_machine *machine, uint32_t address,
    const C_VOID *data, uint8_t size)
{
    type_status status = core_machine_debug_require_boundary(machine);
    if (status != TYPE_STATUS_OK) return status;
    return core_machine_cpu_write_linear(&machine->executor_cpu_execution, address,
        data, size) == 0 ? TYPE_STATUS_OK : TYPE_STATUS_INVALID_STATE;
}

type_status core_machine_debug_read_real(core_machine *machine, uint16_t segment,
    uint16_t offset, C_VOID *out_data, STD_SIZE_T size)
{
    type_status status = core_machine_debug_require_boundary(machine);
    if (status != TYPE_STATUS_OK) return status;
    return core_machine_memory_read_real_from(&machine->executor_memory, segment,
        offset, out_data, size);
}

type_status core_machine_debug_write_real(core_machine *machine, uint16_t segment,
    uint16_t offset, const C_VOID *data, STD_SIZE_T size)
{
    type_status status = core_machine_debug_require_boundary(machine);
    if (status != TYPE_STATUS_OK) return status;
    return core_machine_memory_write_real_to(&machine->executor_memory, segment,
        offset, data, size);
}

type_status core_machine_debug_read_port(core_machine *machine, uint16_t port,
    uint32_t *out_value)
{
    type_status status = core_machine_debug_require_boundary(machine);
    if (status != TYPE_STATUS_OK || out_value == STD_NULL) return
        status == TYPE_STATUS_OK ? TYPE_STATUS_INVALID_ARGUMENT : status;
    status = core_machine_port_execute_read(&machine->executor_port, port);
    if (status != TYPE_STATUS_OK) return status;
    *out_value = machine->executor_port.data.ioDWord;
    return TYPE_STATUS_OK;
}

type_status core_machine_debug_write_port(core_machine *machine, uint16_t port,
    uint32_t value)
{
    type_status status = core_machine_debug_require_boundary(machine);
    if (status != TYPE_STATUS_OK) return status;
    {
        uint32_t prior_value = machine->executor_port.data.ioDWord;

        machine->executor_port.data.ioDWord = value;
        status = core_machine_port_execute_write(&machine->executor_port, port);
        if (status != TYPE_STATUS_OK) machine->executor_port.data.ioDWord =
            prior_value;
    }
    return status;
}

type_status core_machine_debug_set_watchpoint(core_machine *machine,
    core_machine_debug_watch_kind kind, uint32_t address)
{
    type_status status = core_machine_debug_require_boundary(machine);
    if (status != TYPE_STATUS_OK || kind > CORE_MACHINE_DEBUG_WATCH_EXECUTE)
        return status != TYPE_STATUS_OK ? status : TYPE_STATUS_INVALID_ARGUMENT;
    core_machine_cpu_set_watchpoint(&machine->executor_cpu_execution,
        (core_machine_cpu_watchpoint)kind, address);
    return TYPE_STATUS_OK;
}

type_status core_machine_debug_clear_watchpoint(core_machine *machine,
    core_machine_debug_watch_kind kind)
{
    type_status status = core_machine_debug_require_boundary(machine);
    if (status != TYPE_STATUS_OK || kind > CORE_MACHINE_DEBUG_WATCH_EXECUTE)
        return status != TYPE_STATUS_OK ? status : TYPE_STATUS_INVALID_ARGUMENT;
    core_machine_cpu_clear_watchpoint(&machine->executor_cpu_execution,
        (core_machine_cpu_watchpoint)kind);
    return TYPE_STATUS_OK;
}

#define CORE_MACHINE_DEBUG_PRINT(name, function) \
type_status name(core_machine *machine) { \
    type_status status = core_machine_debug_require_boundary(machine); \
    if (status != TYPE_STATUS_OK) return status; \
    function(&machine->executor_cpu_execution); \
    return TYPE_STATUS_OK; \
}

CORE_MACHINE_DEBUG_PRINT(core_machine_debug_print_registers,
    core_machine_cpu_print_registers)
CORE_MACHINE_DEBUG_PRINT(core_machine_debug_print_segment_registers,
    core_machine_cpu_print_segment_registers)
CORE_MACHINE_DEBUG_PRINT(core_machine_debug_print_control_registers,
    core_machine_cpu_print_control_registers)
CORE_MACHINE_DEBUG_PRINT(core_machine_debug_print_memory_accesses,
    core_machine_cpu_print_memory_accesses)
CORE_MACHINE_DEBUG_PRINT(core_machine_debug_print_watchpoints,
    core_machine_cpu_print_watchpoints)

#undef CORE_MACHINE_DEBUG_PRINT
