#include "core/bus.h"
#include "core/cpu.h"
#include "core/opcode_interface.h"

#define CORE_CPU_FLAG_C 0x01u
#define CORE_CPU_FLAG_Z 0x02u
#define CORE_CPU_FLAG_I 0x04u
#define CORE_CPU_FLAG_D 0x08u
#define CORE_CPU_FLAG_V 0x40u
#define CORE_CPU_FLAG_N 0x80u
#define CORE_CPU_FLAG_B 0x10u
#define CORE_CPU_FLAG_U 0x20u

static lib_status core_cpu_read(core_machine *machine, lib_u16 address,
    lib_u8 *out_value)
{
    lib_status status = core_bus_read(machine, address, out_value);
    if (status == LIB_STATUS_UNSUPPORTED) {
        machine->trap.trap_valid = LIB_TRUE;
        machine->trap.trap_reason = CORE_MACHINE_STOP_UNSUPPORTED_DEVICE;
        machine->trap.trap_address = address;
    }
    return status;
}

static lib_status core_cpu_write(core_machine *machine, lib_u16 address, lib_u8 value)
{
    lib_status status = core_bus_write(machine, address, value);
    if (status == LIB_STATUS_UNSUPPORTED) {
        machine->trap.trap_valid = LIB_TRUE;
        machine->trap.trap_reason = CORE_MACHINE_STOP_UNSUPPORTED_DEVICE;
        machine->trap.trap_address = address;
    }
    return status;
}

lib_bool core_cpu_flag_is_set(const core_machine *machine, lib_u8 flag)
{
    return machine != LIB_NULL && (machine->p & flag) != 0u;
}

void core_cpu_set_nz(core_machine *machine, lib_u8 value)
{
    machine->p &= (lib_u8)~(CORE_CPU_FLAG_N | CORE_CPU_FLAG_Z);
    if (value == 0u) machine->p |= CORE_CPU_FLAG_Z;
    if ((value & 0x80u) != 0u) machine->p |= CORE_CPU_FLAG_N;
}

static void core_cpu_compare(core_machine *machine, lib_u8 left, lib_u8 right)
{
    lib_u8 value = (lib_u8)(left - right);
    machine->p &= (lib_u8)~CORE_CPU_FLAG_C;
    if (left >= right) machine->p |= CORE_CPU_FLAG_C;
    core_cpu_set_nz(machine, value);
}

static void core_cpu_add(core_machine *machine, lib_u8 value)
{
    lib_u16 total = (lib_u16)machine->a + value +
        (core_cpu_flag_is_set(machine, CORE_CPU_FLAG_C) ? 1u : 0u);
    lib_u8 result = (lib_u8)total;
    machine->p &= (lib_u8)~(CORE_CPU_FLAG_C | CORE_CPU_FLAG_V);
    if (total > 0xffu) machine->p |= CORE_CPU_FLAG_C;
    if (((machine->a ^ result) & (value ^ result) & 0x80u) != 0u)
        machine->p |= CORE_CPU_FLAG_V;
    machine->a = result;
    core_cpu_set_nz(machine, result);
}

static lib_status core_cpu_fetch(core_machine *machine, lib_u8 *out_value)
{
    lib_status status = core_cpu_read(machine, machine->pc, out_value);
    if (status == LIB_STATUS_OK) ++machine->pc;
    return status;
}

static lib_u16 core_cpu_fetch_word(core_machine *machine, lib_status *out_status)
{
    lib_u8 low;
    lib_u8 high;

    *out_status = core_cpu_fetch(machine, &low);
    if (*out_status != LIB_STATUS_OK) return 0u;
    *out_status = core_cpu_fetch(machine, &high);
    return (lib_u16)low | ((lib_u16)high << 8u);
}

static lib_u16 core_cpu_indexed_address(core_machine *machine, lib_u8 index,
    lib_bool *out_crossed, lib_status *out_status)
{
    lib_u16 base = core_cpu_fetch_word(machine, out_status);
    lib_u16 address = (lib_u16)(base + index);
    lib_bool crossed = (base & 0xff00u) != (address & 0xff00u);
    lib_u8 discarded;
    /* Indexed writes and read-modify-writes always spend this cycle reading
     * the uncorrected-page address. Indexed reads do it only while correcting
     * a page crossing. Those callers pass no crossing result, keeping that
     * mode explicit at the call site without a second addressing helper. */
    if (*out_status == LIB_STATUS_OK && (crossed || out_crossed == LIB_NULL))
        *out_status = core_cpu_read(machine,
            (lib_u16)((base & 0xff00u) | (address & 0x00ffu)), &discarded);
    if (*out_status == LIB_STATUS_OK && out_crossed != LIB_NULL) *out_crossed = crossed;
    return address;
}

static lib_u8 core_cpu_zero_page_indexed_address(core_machine *machine, lib_u8 base,
    lib_u8 index, lib_status *out_status)
{
    lib_u8 discarded;

    /* Every zero-page indexed form reads its unindexed address on the cycle
     * between operand fetch and its effective access. */
    if (*out_status == LIB_STATUS_OK)
        *out_status = core_cpu_read(machine, base, &discarded);
    return (lib_u8)(base + index);
}

static lib_u16 core_cpu_indirect_x_address(core_machine *machine, lib_status *out_status)
{
    lib_u8 pointer;
    lib_u8 low;
    lib_u8 high;
    lib_u8 discarded;
    *out_status = core_cpu_fetch(machine, &pointer);
    if (*out_status != LIB_STATUS_OK) return 0u;
    *out_status = core_cpu_read(machine, pointer, &discarded);
    if (*out_status != LIB_STATUS_OK) return 0u;
    pointer = (lib_u8)(pointer + machine->x);
    *out_status = core_cpu_read(machine, pointer, &low);
    if (*out_status == LIB_STATUS_OK)
        *out_status = core_cpu_read(machine, (lib_u8)(pointer + 1u), &high);
    return (lib_u16)low | ((lib_u16)high << 8u);
}

static lib_u16 core_cpu_indirect_y_address(core_machine *machine, lib_bool *out_crossed,
    lib_status *out_status)
{
    lib_u8 pointer;
    lib_u8 low;
    lib_u8 high;
    lib_u16 base;
    *out_status = core_cpu_fetch(machine, &pointer);
    if (*out_status != LIB_STATUS_OK) return 0u;
    *out_status = core_cpu_read(machine, pointer, &low);
    if (*out_status == LIB_STATUS_OK)
        *out_status = core_cpu_read(machine, (lib_u8)(pointer + 1u), &high);
    base = (lib_u16)low | ((lib_u16)high << 8u);
    {
        lib_u16 address = (lib_u16)(base + machine->y);
        lib_bool crossed = (base & 0xff00u) != (address & 0xff00u);
        lib_u8 discarded;
        /* (zp),Y stores have the same compulsory uncorrected-page read as
         * absolute indexed stores; read forms need it only on a crossing. */
        if (*out_status == LIB_STATUS_OK && (crossed || out_crossed == LIB_NULL))
            *out_status = core_cpu_read(machine,
                (lib_u16)((base & 0xff00u) | (address & 0x00ffu)), &discarded);
        if (*out_status == LIB_STATUS_OK && out_crossed != LIB_NULL) *out_crossed = crossed;
        return address;
    }
}

static lib_status core_cpu_push(core_machine *machine, lib_u8 value)
{
    lib_status status = core_cpu_write(machine, (lib_u16)(0x0100u | machine->s), value);
    if (status == LIB_STATUS_OK) --machine->s;
    return status;
}

static lib_status core_cpu_pop(core_machine *machine, lib_u8 *out_value)
{
    ++machine->s;
    return core_cpu_read(machine, (lib_u16)(0x0100u | machine->s), out_value);
}

lib_status core_machine_service_interrupt(core_machine *machine,
    lib_bool *out_serviced, lib_u32 *out_cycles)
{
    lib_bool nmi;
    lib_u8 low;
    lib_u8 high;
    lib_status status;

    if (machine == LIB_NULL || out_serviced == LIB_NULL || out_cycles == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_serviced = LIB_FALSE;
    *out_cycles = 0u;
    core_bus_trace_reset(machine);
    if (machine->nmi_pending && machine->nmi_defer_once) {
        machine->nmi_defer_once = LIB_FALSE;
        return LIB_STATUS_OK;
    }
    nmi = machine->nmi_pending;
    if (!nmi && (!machine->irq_asserted || machine->irq_poll_i))
        return LIB_STATUS_OK;
    if (nmi) machine->nmi_pending = LIB_FALSE;
    machine->interrupt_phase = 1u;
    /* The interrupt sequence has two discarded program reads before its
     * three stack writes and vector fetch. */
    status = core_cpu_read(machine, machine->pc, &low);
    if (status == LIB_STATUS_OK) status = core_cpu_read(machine, machine->pc, &low);
    if (status == LIB_STATUS_OK) status = core_cpu_push(machine, (lib_u8)(machine->pc >> 8u));
    if (status == LIB_STATUS_OK) status = core_cpu_push(machine, (lib_u8)machine->pc);
    if (status == LIB_STATUS_OK) status = core_cpu_push(machine,
        (lib_u8)((machine->p | CORE_CPU_FLAG_U) & (lib_u8)~CORE_CPU_FLAG_B));
    if (status == LIB_STATUS_OK && !nmi && machine->nmi_pending) {
        nmi = LIB_TRUE;
        machine->nmi_pending = LIB_FALSE;
    }
    machine->interrupt_phase = 2u;
    if (status == LIB_STATUS_OK) status = core_cpu_read(machine, nmi ? 0xfffau : 0xfffeu, &low);
    if (status == LIB_STATUS_OK) status = core_cpu_read(machine, nmi ? 0xfffbu : 0xffffu, &high);
    machine->interrupt_phase = 0u;
    if (status != LIB_STATUS_OK) return status;
    machine->pc = (lib_u16)low | ((lib_u16)high << 8u);
    machine->p |= CORE_CPU_FLAG_I;
    machine->irq_poll_i = LIB_TRUE;
    machine->cycles += 7u;
    *out_serviced = LIB_TRUE;
    *out_cycles = 7u;
    return LIB_STATUS_OK;
}

lib_status core_machine_step(core_machine *machine, lib_u32 *out_cycles)
{
    lib_status status;
    lib_u8 opcode;
    lib_u8 operand;
    lib_u16 address;
    lib_u16 opcode_pc;
    lib_u32 cycles = 0u;
    lib_bool crossed = LIB_FALSE;
    lib_bool poll_i;
    core_opcode_metadata opcode_metadata;

    if (machine == LIB_NULL || out_cycles == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_cycles = 0u;
    core_bus_trace_reset(machine);
    if (machine->trap.trap_valid) return LIB_STATUS_INVALID_STATE;
    opcode_pc = machine->pc;
    poll_i = core_cpu_flag_is_set(machine, CORE_CPU_FLAG_I);
    status = core_cpu_fetch(machine, &opcode);
    if (status != LIB_STATUS_OK) return status;
    if (!core_opcode_describe(opcode, &opcode_metadata)) {
        machine->trap.trap_valid = LIB_TRUE;
        machine->trap.trap_reason = CORE_MACHINE_STOP_UNSUPPORTED_OPCODE;
        machine->trap.trap_pc = opcode_pc;
        machine->trap.trap_opcode = opcode;
        cycles = 1u;
        goto completed;
    }
    switch (opcode) {
    case 0x78u:
        machine->p |= CORE_CPU_FLAG_I;
        cycles = 2u;
        break;
    case 0x18u: machine->p &= (lib_u8)~CORE_CPU_FLAG_C; cycles = 2u; break;
    case 0x38u: machine->p |= CORE_CPU_FLAG_C; cycles = 2u; break;
    case 0x58u: machine->p &= (lib_u8)~CORE_CPU_FLAG_I; cycles = 2u; break;
    case 0xb8u: machine->p &= (lib_u8)~CORE_CPU_FLAG_V; cycles = 2u; break;
    case 0xd8u: machine->p &= (lib_u8)~CORE_CPU_FLAG_D; cycles = 2u; break;
    case 0xf8u: machine->p |= CORE_CPU_FLAG_D; cycles = 2u; break;
    case 0xeau:
        cycles = 2u;
        break;
    case 0x0au:
        machine->p &= (lib_u8)~CORE_CPU_FLAG_C;
        if ((machine->a & 0x80u) != 0u) machine->p |= CORE_CPU_FLAG_C;
        machine->a <<= 1u; core_cpu_set_nz(machine, machine->a); cycles = 2u; break;
    case 0x4au:
        machine->p &= (lib_u8)~CORE_CPU_FLAG_C;
        if ((machine->a & 0x01u) != 0u) machine->p |= CORE_CPU_FLAG_C;
        machine->a >>= 1u; core_cpu_set_nz(machine, machine->a); cycles = 2u; break;
    case 0x2au:
    {
        lib_bool carry = core_cpu_flag_is_set(machine, CORE_CPU_FLAG_C);
        lib_bool next = (machine->a & 0x80u) != 0u;
        machine->a = (lib_u8)((machine->a << 1u) | (carry ? 1u : 0u));
        machine->p &= (lib_u8)~CORE_CPU_FLAG_C;
        if (next) machine->p |= CORE_CPU_FLAG_C;
        core_cpu_set_nz(machine, machine->a); cycles = 2u; break;
    }
    case 0x6au:
    {
        lib_bool carry = core_cpu_flag_is_set(machine, CORE_CPU_FLAG_C);
        lib_bool next = (machine->a & 0x01u) != 0u;
        machine->a = (lib_u8)((machine->a >> 1u) | (carry ? 0x80u : 0u));
        machine->p &= (lib_u8)~CORE_CPU_FLAG_C;
        if (next) machine->p |= CORE_CPU_FLAG_C;
        core_cpu_set_nz(machine, machine->a); cycles = 2u; break;
    }
    case 0xa9u:
        status = core_cpu_fetch(machine, &machine->a);
        core_cpu_set_nz(machine, machine->a);
        cycles = 2u;
        break;
    case 0x09u:
        status = core_cpu_fetch(machine, &operand);
        if (status == LIB_STATUS_OK) { machine->a |= operand; core_cpu_set_nz(machine, machine->a); }
        cycles = 2u; break;
    case 0x29u:
        status = core_cpu_fetch(machine, &operand);
        if (status == LIB_STATUS_OK) { machine->a &= operand; core_cpu_set_nz(machine, machine->a); }
        cycles = 2u; break;
    case 0x49u:
        status = core_cpu_fetch(machine, &operand);
        if (status == LIB_STATUS_OK) { machine->a ^= operand; core_cpu_set_nz(machine, machine->a); }
        cycles = 2u; break;
    case 0xc9u:
        status = core_cpu_fetch(machine, &operand);
        if (status == LIB_STATUS_OK) core_cpu_compare(machine, machine->a, operand);
        cycles = 2u; break;
    case 0xc0u:
        status = core_cpu_fetch(machine, &operand);
        if (status == LIB_STATUS_OK) core_cpu_compare(machine, machine->y, operand);
        cycles = 2u; break;
    case 0x24u: case 0x2cu:
    {
        lib_u8 value;
        if (opcode == 0x24u) {
            status = core_cpu_fetch(machine, &operand);
            address = operand;
            cycles = 3u;
        } else {
            address = core_cpu_fetch_word(machine, &status);
            cycles = 4u;
        }
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &value);
        if (status == LIB_STATUS_OK) {
            machine->p &= (lib_u8)~(CORE_CPU_FLAG_N | CORE_CPU_FLAG_V | CORE_CPU_FLAG_Z);
            machine->p |= value & (CORE_CPU_FLAG_N | CORE_CPU_FLAG_V);
            if ((machine->a & value) == 0u) machine->p |= CORE_CPU_FLAG_Z;
        }
        break;
    }
    case 0x69u:
        status = core_cpu_fetch(machine, &operand);
        if (status == LIB_STATUS_OK) core_cpu_add(machine, operand);
        cycles = 2u; break;
    case 0xe9u:
        status = core_cpu_fetch(machine, &operand);
        if (status == LIB_STATUS_OK) core_cpu_add(machine, (lib_u8)~operand);
        cycles = 2u; break;
    case 0x65u: case 0xe5u: case 0x6du: case 0xedu:
    {
        lib_u8 value;
        lib_bool subtract = opcode == 0xe5u || opcode == 0xedu;
        if (opcode == 0x65u || opcode == 0xe5u) {
            status = core_cpu_fetch(machine, &operand);
            address = operand;
            cycles = 3u;
        } else {
            address = core_cpu_fetch_word(machine, &status);
            cycles = 4u;
        }
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &value);
        if (status == LIB_STATUS_OK) core_cpu_add(machine, subtract ? (lib_u8)~value : value);
        break;
    }
    case 0x75u: case 0xf5u: case 0x7du: case 0xfdu: case 0x79u: case 0xf9u:
    case 0x61u: case 0xe1u: case 0x71u: case 0xf1u:
    {
        lib_u8 value;
        lib_bool subtract = opcode == 0xf5u || opcode == 0xfdu || opcode == 0xf9u || opcode == 0xe1u || opcode == 0xf1u;
        if (opcode == 0x75u || opcode == 0xf5u) {
            status = core_cpu_fetch(machine, &operand);
            address = core_cpu_zero_page_indexed_address(machine, operand, machine->x, &status); cycles = 4u;
        } else if (opcode == 0x7du || opcode == 0xfdu) {
            address = core_cpu_indexed_address(machine, machine->x, &crossed, &status); cycles = 4u + (crossed ? 1u : 0u);
        } else if (opcode == 0x79u || opcode == 0xf9u) {
            address = core_cpu_indexed_address(machine, machine->y, &crossed, &status); cycles = 4u + (crossed ? 1u : 0u);
        } else if (opcode == 0x61u || opcode == 0xe1u) {
            address = core_cpu_indirect_x_address(machine, &status); cycles = 6u;
        } else {
            address = core_cpu_indirect_y_address(machine, &crossed, &status); cycles = 5u + (crossed ? 1u : 0u);
        }
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &value);
        if (status == LIB_STATUS_OK) core_cpu_add(machine, subtract ? (lib_u8)~value : value);
        break;
    }
    case 0x05u: case 0x25u: case 0x45u: case 0x0du: case 0x2du: case 0x4du:
    {
        lib_u8 value;
        if (opcode == 0x05u || opcode == 0x25u || opcode == 0x45u) {
            status = core_cpu_fetch(machine, &operand);
            address = operand;
            cycles = 3u;
        } else {
            address = core_cpu_fetch_word(machine, &status);
            cycles = 4u;
        }
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &value);
        if (status == LIB_STATUS_OK) {
            if (opcode == 0x05u || opcode == 0x0du) machine->a |= value;
            else if (opcode == 0x25u || opcode == 0x2du) machine->a &= value;
            else machine->a ^= value;
            core_cpu_set_nz(machine, machine->a);
        }
        break;
    }
    case 0x15u: case 0x35u: case 0x55u: case 0x1du: case 0x3du: case 0x5du:
    case 0x19u: case 0x39u: case 0x59u:
    {
        lib_u8 value;
        if (opcode == 0x15u || opcode == 0x35u || opcode == 0x55u) {
            status = core_cpu_fetch(machine, &operand);
            address = core_cpu_zero_page_indexed_address(machine, operand, machine->x, &status); cycles = 4u;
        } else if (opcode == 0x1du || opcode == 0x3du || opcode == 0x5du) {
            address = core_cpu_indexed_address(machine, machine->x, &crossed, &status);
            cycles = 4u + (crossed ? 1u : 0u);
        } else {
            address = core_cpu_indexed_address(machine, machine->y, &crossed, &status);
            cycles = 4u + (crossed ? 1u : 0u);
        }
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &value);
        if (status == LIB_STATUS_OK) {
            if (opcode == 0x15u || opcode == 0x1du || opcode == 0x19u) machine->a |= value;
            else if (opcode == 0x35u || opcode == 0x3du || opcode == 0x39u) machine->a &= value;
            else machine->a ^= value;
            core_cpu_set_nz(machine, machine->a);
        }
        break;
    }
    case 0x01u: case 0x21u: case 0x41u: case 0x11u: case 0x31u: case 0x51u:
    {
        lib_u8 value;
        if (opcode == 0x01u || opcode == 0x21u || opcode == 0x41u) {
            address = core_cpu_indirect_x_address(machine, &status); cycles = 6u;
        } else {
            address = core_cpu_indirect_y_address(machine, &crossed, &status);
            cycles = 5u + (crossed ? 1u : 0u);
        }
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &value);
        if (status == LIB_STATUS_OK) {
            if (opcode == 0x01u || opcode == 0x11u) machine->a |= value;
            else if (opcode == 0x21u || opcode == 0x31u) machine->a &= value;
            else machine->a ^= value;
            core_cpu_set_nz(machine, machine->a);
        }
        break;
    }
    case 0xc5u: case 0xcdu: case 0xc4u: case 0xccu: case 0xe4u: case 0xecu:
    {
        lib_u8 value;
        lib_u8 left = (opcode == 0xc5u || opcode == 0xcdu) ? machine->a :
            ((opcode == 0xc4u || opcode == 0xccu) ? machine->y : machine->x);
        if (opcode == 0xc5u || opcode == 0xc4u || opcode == 0xe4u) {
            status = core_cpu_fetch(machine, &operand);
            address = operand;
            cycles = 3u;
        } else {
            address = core_cpu_fetch_word(machine, &status);
            cycles = 4u;
        }
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &value);
        if (status == LIB_STATUS_OK) core_cpu_compare(machine, left, value);
        break;
    }
    case 0xd5u: case 0xddu: case 0xd9u: case 0xc1u: case 0xd1u:
    {
        lib_u8 value;
        if (opcode == 0xd5u) {
            status = core_cpu_fetch(machine, &operand);
            address = core_cpu_zero_page_indexed_address(machine, operand, machine->x, &status); cycles = 4u;
        } else if (opcode == 0xddu) {
            address = core_cpu_indexed_address(machine, machine->x, &crossed, &status); cycles = 4u + (crossed ? 1u : 0u);
        } else if (opcode == 0xd9u) {
            address = core_cpu_indexed_address(machine, machine->y, &crossed, &status); cycles = 4u + (crossed ? 1u : 0u);
        } else if (opcode == 0xc1u) {
            address = core_cpu_indirect_x_address(machine, &status); cycles = 6u;
        } else {
            address = core_cpu_indirect_y_address(machine, &crossed, &status); cycles = 5u + (crossed ? 1u : 0u);
        }
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &value);
        if (status == LIB_STATUS_OK) core_cpu_compare(machine, machine->a, value);
        break;
    }
    case 0x06u: case 0x46u: case 0x26u: case 0x66u:
    case 0x16u: case 0x56u: case 0x36u: case 0x76u:
    case 0x0eu: case 0x4eu: case 0x2eu: case 0x6eu:
    case 0x1eu: case 0x5eu: case 0x3eu: case 0x7eu:
    {
        lib_u8 value;
        lib_u8 original;
        lib_bool rotate = opcode == 0x26u || opcode == 0x66u || opcode == 0x36u || opcode == 0x76u || opcode == 0x2eu || opcode == 0x6eu || opcode == 0x3eu || opcode == 0x7eu;
        lib_bool right = opcode == 0x46u || opcode == 0x66u || opcode == 0x56u || opcode == 0x76u || opcode == 0x4eu || opcode == 0x6eu || opcode == 0x5eu || opcode == 0x7eu;
        lib_bool carry;
        if (opcode == 0x06u || opcode == 0x46u || opcode == 0x26u || opcode == 0x66u || opcode == 0x16u || opcode == 0x56u || opcode == 0x36u || opcode == 0x76u) {
            status = core_cpu_fetch(machine, &operand); address = operand; cycles = 5u;
            if (opcode == 0x16u || opcode == 0x56u || opcode == 0x36u || opcode == 0x76u) {
                address = core_cpu_zero_page_indexed_address(machine, address, machine->x, &status); cycles = 6u;
            }
        } else {
            if (opcode == 0x1eu || opcode == 0x5eu || opcode == 0x3eu || opcode == 0x7eu) {
                address = core_cpu_indexed_address(machine, machine->x, LIB_NULL, &status);
                cycles = 7u;
            } else {
                address = core_cpu_fetch_word(machine, &status); cycles = 6u;
            }
        }
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &value);
        if (status == LIB_STATUS_OK) {
            original = value;
            carry = core_cpu_flag_is_set(machine, CORE_CPU_FLAG_C);
            machine->p &= (lib_u8)~CORE_CPU_FLAG_C;
            if (right) {
                if ((value & 1u) != 0u) machine->p |= CORE_CPU_FLAG_C;
                value = (lib_u8)((value >> 1u) | (rotate && carry ? 0x80u : 0u));
            } else {
                if ((value & 0x80u) != 0u) machine->p |= CORE_CPU_FLAG_C;
                value = (lib_u8)((value << 1u) | (rotate && carry ? 1u : 0u));
            }
            status = core_cpu_write(machine, address, original);
            if (status == LIB_STATUS_OK) status = core_cpu_write(machine, address, value);
            if (status == LIB_STATUS_OK) core_cpu_set_nz(machine, value);
        }
        break;
    }
    case 0xa5u:
        status = core_cpu_fetch(machine, &operand);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, operand, &machine->a);
        core_cpu_set_nz(machine, machine->a);
        cycles = 3u;
        break;
    case 0xb5u:
        status = core_cpu_fetch(machine, &operand);
        address = core_cpu_zero_page_indexed_address(machine, operand, machine->x, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &machine->a);
        core_cpu_set_nz(machine, machine->a); cycles = 4u; break;
    case 0xadu:
        address = core_cpu_fetch_word(machine, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &machine->a);
        core_cpu_set_nz(machine, machine->a); cycles = 4u; break;
    case 0xbdu:
        address = core_cpu_indexed_address(machine, machine->x, &crossed, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &machine->a);
        core_cpu_set_nz(machine, machine->a); cycles = 4u + (crossed ? 1u : 0u); break;
    case 0xb9u:
        address = core_cpu_indexed_address(machine, machine->y, &crossed, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &machine->a);
        core_cpu_set_nz(machine, machine->a); cycles = 4u + (crossed ? 1u : 0u); break;
    case 0xa1u:
        address = core_cpu_indirect_x_address(machine, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &machine->a);
        core_cpu_set_nz(machine, machine->a); cycles = 6u; break;
    case 0xb1u:
        address = core_cpu_indirect_y_address(machine, &crossed, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &machine->a);
        core_cpu_set_nz(machine, machine->a); cycles = 5u + (crossed ? 1u : 0u); break;
    case 0x85u:
        status = core_cpu_fetch(machine, &operand);
        if (status == LIB_STATUS_OK) status = core_cpu_write(machine, operand, machine->a);
        cycles = 3u;
        break;
    case 0x8du:
        address = core_cpu_fetch_word(machine, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_write(machine, address, machine->a);
        cycles = 4u;
        break;
    case 0x95u:
        status = core_cpu_fetch(machine, &operand);
        address = core_cpu_zero_page_indexed_address(machine, operand, machine->x, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_write(machine, address, machine->a);
        cycles = 4u; break;
    case 0x9du:
        address = core_cpu_indexed_address(machine, machine->x, LIB_NULL, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_write(machine, address, machine->a);
        cycles = 5u; break;
    case 0x99u:
        address = core_cpu_indexed_address(machine, machine->y, LIB_NULL, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_write(machine, address, machine->a);
        cycles = 5u; break;
    case 0x81u:
        address = core_cpu_indirect_x_address(machine, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_write(machine, address, machine->a);
        cycles = 6u; break;
    case 0x91u:
        address = core_cpu_indirect_y_address(machine, LIB_NULL, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_write(machine, address, machine->a);
        cycles = 6u; break;
    case 0x84u:
        status = core_cpu_fetch(machine, &operand);
        if (status == LIB_STATUS_OK) status = core_cpu_write(machine, operand, machine->y);
        cycles = 3u; break;
    case 0x94u:
        status = core_cpu_fetch(machine, &operand);
        address = core_cpu_zero_page_indexed_address(machine, operand, machine->x, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_write(machine, address, machine->y);
        cycles = 4u; break;
    case 0x8cu:
        address = core_cpu_fetch_word(machine, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_write(machine, address, machine->y);
        cycles = 4u; break;
    case 0x86u:
        status = core_cpu_fetch(machine, &operand);
        if (status == LIB_STATUS_OK) status = core_cpu_write(machine, operand, machine->x);
        cycles = 3u; break;
    case 0x96u:
        status = core_cpu_fetch(machine, &operand);
        address = core_cpu_zero_page_indexed_address(machine, operand, machine->y, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_write(machine, address, machine->x);
        cycles = 4u; break;
    case 0x8eu:
        address = core_cpu_fetch_word(machine, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_write(machine, address, machine->x);
        cycles = 4u; break;
    case 0xe6u: case 0xc6u: case 0xf6u: case 0xd6u:
    {
        lib_bool decrement = opcode == 0xc6u || opcode == 0xd6u;
        lib_u8 value;
        lib_u8 original;
        status = core_cpu_fetch(machine, &operand);
        if (opcode == 0xf6u || opcode == 0xd6u)
            operand = core_cpu_zero_page_indexed_address(machine, operand, machine->x, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, operand, &value);
        if (status == LIB_STATUS_OK) {
            original = value;
            value = decrement ? (lib_u8)(value - 1u) : (lib_u8)(value + 1u);
            status = core_cpu_write(machine, operand, original);
            if (status == LIB_STATUS_OK) status = core_cpu_write(machine, operand, value);
            if (status == LIB_STATUS_OK) core_cpu_set_nz(machine, value);
        }
        cycles = (opcode == 0xe6u || opcode == 0xc6u) ? 5u : 6u; break;
    }
    case 0xeeu: case 0xceu: case 0xfeu: case 0xdeu:
    {
        lib_bool decrement = opcode == 0xceu || opcode == 0xdeu;
        lib_u8 value;
        lib_u8 original;
        if (opcode == 0xfeu || opcode == 0xdeu)
            address = core_cpu_indexed_address(machine, machine->x, LIB_NULL, &status);
        else
            address = core_cpu_fetch_word(machine, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &value);
        if (status == LIB_STATUS_OK) {
            original = value;
            value = decrement ? (lib_u8)(value - 1u) : (lib_u8)(value + 1u);
            status = core_cpu_write(machine, address, original);
            if (status == LIB_STATUS_OK) status = core_cpu_write(machine, address, value);
            if (status == LIB_STATUS_OK) core_cpu_set_nz(machine, value);
        }
        cycles = (opcode == 0xeeu || opcode == 0xceu) ? 6u : 7u; break;
    }
    case 0xa2u:
        status = core_cpu_fetch(machine, &machine->x);
        core_cpu_set_nz(machine, machine->x);
        cycles = 2u;
        break;
    case 0xa0u:
        status = core_cpu_fetch(machine, &machine->y);
        core_cpu_set_nz(machine, machine->y);
        cycles = 2u;
        break;
    case 0xa4u:
        status = core_cpu_fetch(machine, &operand);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, operand, &machine->y);
        core_cpu_set_nz(machine, machine->y); cycles = 3u; break;
    case 0xb4u:
        status = core_cpu_fetch(machine, &operand);
        address = core_cpu_zero_page_indexed_address(machine, operand, machine->x, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &machine->y);
        core_cpu_set_nz(machine, machine->y); cycles = 4u; break;
    case 0xacu:
        address = core_cpu_fetch_word(machine, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &machine->y);
        core_cpu_set_nz(machine, machine->y); cycles = 4u; break;
    case 0xbcu:
        address = core_cpu_indexed_address(machine, machine->x, &crossed, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &machine->y);
        core_cpu_set_nz(machine, machine->y); cycles = 4u + (crossed ? 1u : 0u); break;
    case 0xa6u:
        status = core_cpu_fetch(machine, &operand);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, operand, &machine->x);
        core_cpu_set_nz(machine, machine->x); cycles = 3u; break;
    case 0xb6u:
        status = core_cpu_fetch(machine, &operand);
        address = core_cpu_zero_page_indexed_address(machine, operand, machine->y, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &machine->x);
        core_cpu_set_nz(machine, machine->x); cycles = 4u; break;
    case 0xaeu:
        address = core_cpu_fetch_word(machine, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &machine->x);
        core_cpu_set_nz(machine, machine->x); cycles = 4u; break;
    case 0xbeu:
        address = core_cpu_indexed_address(machine, machine->y, &crossed, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &machine->x);
        core_cpu_set_nz(machine, machine->x); cycles = 4u + (crossed ? 1u : 0u); break;
    case 0xe8u:
        ++machine->x;
        core_cpu_set_nz(machine, machine->x);
        cycles = 2u;
        break;
    case 0xc8u: ++machine->y; core_cpu_set_nz(machine, machine->y); cycles = 2u; break;
    case 0x88u: --machine->y; core_cpu_set_nz(machine, machine->y); cycles = 2u; break;
    case 0xaau: machine->x = machine->a; core_cpu_set_nz(machine, machine->x); cycles = 2u; break;
    case 0xa8u: machine->y = machine->a; core_cpu_set_nz(machine, machine->y); cycles = 2u; break;
    case 0x8au: machine->a = machine->x; core_cpu_set_nz(machine, machine->a); cycles = 2u; break;
    case 0x98u: machine->a = machine->y; core_cpu_set_nz(machine, machine->a); cycles = 2u; break;
    case 0xbau: machine->x = machine->s; core_cpu_set_nz(machine, machine->x); cycles = 2u; break;
    case 0x9au: machine->s = machine->x; cycles = 2u; break;
    case 0xcau:
        --machine->x;
        core_cpu_set_nz(machine, machine->x);
        cycles = 2u;
        break;
    case 0xe0u:
        status = core_cpu_fetch(machine, &operand);
        if (status == LIB_STATUS_OK) core_cpu_compare(machine, machine->x, operand);
        cycles = 2u;
        break;
    case 0x10u: case 0x30u: case 0x50u: case 0x70u:
    case 0x90u: case 0xb0u: case 0xd0u: case 0xf0u:
    {
        lib_bool take;
        status = core_cpu_fetch(machine, &operand);
        cycles = 2u;
        take = (opcode == 0x10u && !core_cpu_flag_is_set(machine, CORE_CPU_FLAG_N)) ||
            (opcode == 0x30u && core_cpu_flag_is_set(machine, CORE_CPU_FLAG_N)) ||
            (opcode == 0x50u && !core_cpu_flag_is_set(machine, CORE_CPU_FLAG_V)) ||
            (opcode == 0x70u && core_cpu_flag_is_set(machine, CORE_CPU_FLAG_V)) ||
            (opcode == 0x90u && !core_cpu_flag_is_set(machine, CORE_CPU_FLAG_C)) ||
            (opcode == 0xb0u && core_cpu_flag_is_set(machine, CORE_CPU_FLAG_C)) ||
            (opcode == 0xd0u && !core_cpu_flag_is_set(machine, CORE_CPU_FLAG_Z)) ||
            (opcode == 0xf0u && core_cpu_flag_is_set(machine, CORE_CPU_FLAG_Z));
        if (status == LIB_STATUS_OK && take) {
            lib_u16 target = (lib_u16)(machine->pc + (lib_i32)(lib_i8)operand);
            lib_u16 uncorrected = (lib_u16)((machine->pc & 0xff00u) |
                (target & 0x00ffu));
            lib_u8 discarded;
            ++cycles;
            status = core_cpu_read(machine, uncorrected, &discarded);
            if (status == LIB_STATUS_OK &&
                (machine->pc & 0xff00u) != (target & 0xff00u)) {
                ++cycles;
                status = core_cpu_read(machine, target, &discarded);
            }
            if (status == LIB_STATUS_OK) machine->pc = target;
        }
        break;
    }
    case 0x4cu:
        address = core_cpu_fetch_word(machine, &status);
        if (status == LIB_STATUS_OK) machine->pc = address;
        cycles = 3u;
        break;
    case 0x6cu:
    {
        lib_u8 low;
        lib_u8 high;
        address = core_cpu_fetch_word(machine, &status);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &low);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine,
            (lib_u16)((address & 0xff00u) | ((address + 1u) & 0x00ffu)), &high);
        if (status == LIB_STATUS_OK) machine->pc = (lib_u16)low | ((lib_u16)high << 8u);
        cycles = 5u;
        break;
    }
    case 0x20u:
    {
        lib_u8 high;
        status = core_cpu_fetch(machine, &operand);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine,
            (lib_u16)(0x0100u | machine->s), &high);
        if (status == LIB_STATUS_OK) status = core_cpu_push(machine,
            (lib_u8)(machine->pc >> 8u));
        if (status == LIB_STATUS_OK) status = core_cpu_push(machine,
            (lib_u8)machine->pc);
        if (status == LIB_STATUS_OK) status = core_cpu_fetch(machine, &high);
        if (status == LIB_STATUS_OK) machine->pc = (lib_u16)operand | ((lib_u16)high << 8u);
        cycles = 6u; break;
    }
    case 0x60u:
    {
        lib_u8 low;
        lib_u8 high;
        status = core_cpu_read(machine, machine->pc, &operand);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine,
            (lib_u16)(0x0100u | machine->s), &operand);
        if (status == LIB_STATUS_OK) status = core_cpu_pop(machine, &low);
        if (status == LIB_STATUS_OK) status = core_cpu_pop(machine, &high);
        address = (lib_u16)low | ((lib_u16)high << 8u);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, address, &operand);
        if (status == LIB_STATUS_OK) machine->pc = (lib_u16)(address + 1u);
        cycles = 6u; break;
    }
    case 0x48u:
        status = core_cpu_read(machine, machine->pc, &operand);
        if (status == LIB_STATUS_OK) status = core_cpu_push(machine, machine->a);
        cycles = 3u; break;
    case 0x68u:
        status = core_cpu_read(machine, machine->pc, &operand);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine,
            (lib_u16)(0x0100u | machine->s), &operand);
        if (status == LIB_STATUS_OK) status = core_cpu_pop(machine, &machine->a);
        if (status == LIB_STATUS_OK) core_cpu_set_nz(machine, machine->a);
        cycles = 4u; break;
    case 0x08u:
        status = core_cpu_read(machine, machine->pc, &operand);
        if (status == LIB_STATUS_OK) status = core_cpu_push(machine,
            (lib_u8)(machine->p | CORE_CPU_FLAG_B | CORE_CPU_FLAG_U));
        cycles = 3u; break;
    case 0x28u:
        status = core_cpu_read(machine, machine->pc, &operand);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine,
            (lib_u16)(0x0100u | machine->s), &operand);
        if (status == LIB_STATUS_OK) status = core_cpu_pop(machine, &machine->p);
        if (status == LIB_STATUS_OK) {
            machine->p |= CORE_CPU_FLAG_U;
            machine->p &= (lib_u8)~CORE_CPU_FLAG_B;
        }
        cycles = 4u; break;
    case 0x00u:
    {
        lib_u8 low;
        lib_u8 high;
        status = core_cpu_read(machine, machine->pc, &operand);
        if (status == LIB_STATUS_OK) ++machine->pc;
        if (status == LIB_STATUS_OK) status = core_cpu_push(machine, (lib_u8)(machine->pc >> 8u));
        if (status == LIB_STATUS_OK) status = core_cpu_push(machine, (lib_u8)machine->pc);
        if (status == LIB_STATUS_OK) status = core_cpu_push(machine,
            (lib_u8)(machine->p | CORE_CPU_FLAG_B | CORE_CPU_FLAG_U));
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, 0xfffeu, &low);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine, 0xffffu, &high);
        if (status == LIB_STATUS_OK) machine->pc = (lib_u16)low | ((lib_u16)high << 8u);
        machine->p |= CORE_CPU_FLAG_I;
        cycles = 7u; break;
    }
    case 0x40u:
    {
        lib_u8 low;
        lib_u8 high;
        status = core_cpu_read(machine, machine->pc, &operand);
        if (status == LIB_STATUS_OK) status = core_cpu_read(machine,
            (lib_u16)(0x0100u | machine->s), &operand);
        if (status == LIB_STATUS_OK) status = core_cpu_pop(machine, &machine->p);
        if (status == LIB_STATUS_OK) { machine->p |= CORE_CPU_FLAG_U; machine->p &= (lib_u8)~CORE_CPU_FLAG_B; }
        if (status == LIB_STATUS_OK) status = core_cpu_pop(machine, &low);
        if (status == LIB_STATUS_OK) status = core_cpu_pop(machine, &high);
        if (status == LIB_STATUS_OK) machine->pc = (lib_u16)low | ((lib_u16)high << 8u);
        cycles = 6u; break;
    }
    default:
        machine->trap.trap_valid = LIB_TRUE;
        machine->trap.trap_reason = CORE_MACHINE_STOP_UNSUPPORTED_OPCODE;
        machine->trap.trap_pc = opcode_pc;
        machine->trap.trap_opcode = opcode;
        cycles = 1u;
        break;
    }
completed:
    /* MOS 6500 Appendix A places the next-PC bus address on the second cycle
     * of implied and accumulator forms. The value is discarded but it is a
     * real read and updates the CPU data latch. Immediate forms already have
     * their operand fetch, so the one-record shape identifies only this class. */
    if (status == LIB_STATUS_OK && cycles == 2u && machine->trace_count == 1u)
        status = core_cpu_read(machine, machine->pc, &operand);
    machine->cycles += cycles;
    if (status != LIB_STATUS_OK) {
        if (!machine->trap.trap_valid) {
            machine->trap.trap_valid = LIB_TRUE;
            machine->trap.trap_reason = CORE_MACHINE_STOP_BUS_FAILURE;
        }
        machine->trap.trap_pc = opcode_pc;
        machine->trap.trap_opcode = opcode;
    }
    if (!machine->trap.trap_valid) {
        /* NMOS polling uses I before CLI/SEI/PLP modify it, while RTI has
         * restored P before its later polling point. All other forms leave I
         * unchanged, so their post-state is the applicable sampled value. */
        if (opcode == 0x58u || opcode == 0x78u || opcode == 0x28u)
            machine->irq_poll_i = poll_i;
        else
            machine->irq_poll_i = core_cpu_flag_is_set(machine, CORE_CPU_FLAG_I);
        ++machine->instructions;
    }
    *out_cycles = cycles;
    return LIB_STATUS_OK;
}
