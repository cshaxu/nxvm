#include "support/cpu_v30_memory_fixture.h"
#include "lib/types/file.h"

_Static_assert(X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE==15 &&
    X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_EMULATION_L2==16,
    "Append the compatibility model without moving existing origins");

typedef enum emulation_operation {
    BAD, NOP, LXI, STAX, INX, INR, DCR, MVI, RLC, DAD, LDAX, DCX, RRC,
    RAL, RAR, SHLD, DAA, LHLD, CMA, STA, STC, LDA, CMC, MOV, HALT,
    ADD, ADC, SUB, SBB, ANA, XRA, ORA, CMP, RET, POP, JMP, CALL, PUSH,
    ALUI, RST, OUT, IN, XTHL, PCHL, XCHG, ESC, DI, SPHL, EI
} emulation_operation;

/* Independent transcription of the documented opcode table. BAD slots are
 * not undocumented 8080 aliases, 8085 RIM/SIM or native x86 instructions. */
static const emulation_operation operations[256] = {
    NOP,LXI,STAX,INX,INR,DCR,MVI,RLC,BAD,DAD,LDAX,DCX,INR,DCR,MVI,RRC,
    BAD,LXI,STAX,INX,INR,DCR,MVI,RAL,BAD,DAD,LDAX,DCX,INR,DCR,MVI,RAR,
    BAD,LXI,SHLD,INX,INR,DCR,MVI,DAA,BAD,DAD,LHLD,DCX,INR,DCR,MVI,CMA,
    BAD,LXI,STA,INX,INR,DCR,MVI,STC,BAD,DAD,LDA,DCX,INR,DCR,MVI,CMC,
    MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,
    MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,
    MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,
    MOV,MOV,MOV,MOV,MOV,MOV,HALT,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,MOV,
    ADD,ADD,ADD,ADD,ADD,ADD,ADD,ADD,ADC,ADC,ADC,ADC,ADC,ADC,ADC,ADC,
    SUB,SUB,SUB,SUB,SUB,SUB,SUB,SUB,SBB,SBB,SBB,SBB,SBB,SBB,SBB,SBB,
    ANA,ANA,ANA,ANA,ANA,ANA,ANA,ANA,XRA,XRA,XRA,XRA,XRA,XRA,XRA,XRA,
    ORA,ORA,ORA,ORA,ORA,ORA,ORA,ORA,CMP,CMP,CMP,CMP,CMP,CMP,CMP,CMP,
    RET,POP,JMP,JMP,CALL,PUSH,ALUI,RST,RET,RET,JMP,BAD,CALL,CALL,ALUI,RST,
    RET,POP,JMP,OUT,CALL,PUSH,ALUI,RST,RET,BAD,JMP,IN,CALL,BAD,ALUI,RST,
    RET,POP,JMP,XTHL,CALL,PUSH,ALUI,RST,RET,PCHL,JMP,XCHG,CALL,ESC,ALUI,RST,
    RET,POP,JMP,DI,CALL,PUSH,ALUI,RST,RET,SPHL,JMP,EI,CALL,BAD,ALUI,RST
};

typedef struct emulation_fixture {
    v30_memory_fixture ram;
    lib_u16 port;
    lib_u8 port_data, port_seen;
    lib_bool port_write, port_failed, port_active;
    lib_u32 port_calls, port_completions, port_phases;
} emulation_fixture;

static lib_status emulation_port(void *opaque, lib_u16 port, lib_u8 bytes,
    lib_bool write, lib_u32 *value)
{
    emulation_fixture *fixture = opaque;
    fixture->ram.invalid |= port != fixture->port || bytes != 1u ||
        write != fixture->port_write || fixture->port_active;
    ++fixture->port_calls;
    if (fixture->port_failed) return LIB_STATUS_IO_ERROR;
    fixture->port_active = LIB_TRUE;
    if (write) fixture->port_seen = (lib_u8)*value;
    else *value = fixture->port_data;
    return LIB_STATUS_OK;
}

static void emulation_complete(void *opaque, lib_u16 port, lib_u8 bytes, lib_bool write)
{
    emulation_fixture *fixture = opaque;
    fixture->ram.invalid |= port != fixture->port || bytes != 1u ||
        write != fixture->port_write || !fixture->port_active ||
        !fixture->ram.bus.instructions.data.flagIgnore ||
        (!write && fixture->ram.bus.cpu.data.al != fixture->port_data);
    fixture->port_active = LIB_FALSE;
    ++fixture->port_completions;
}

static lib_u64 emulation_cycle(void *opaque, x86_core_cpu_external_cycle_phase phase,
    x86_core_cpu_external_cycle_space space, lib_u32 address, lib_u8 bytes,
    lib_u8 write, x86_core_cpu_memory_access_provenance provenance)
{
    emulation_fixture *fixture = opaque;
    if (space == X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT) {
        fixture->ram.invalid |= address != fixture->port || bytes != 1u ||
            write != fixture->port_write || provenance != X86_CORE_CPU_MEMORY_ACCESS_DATA ||
            phase != (fixture->port_phases == 0u ? X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN :
                fixture->port_failed ? X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL :
                    X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT);
        ++fixture->port_phases;
        return 0u;
    }
    return v30_memory_cycle(opaque,phase,space,address,bytes,write,provenance);
}

static lib_u8 emulation_length(lib_u8 opcode)
{
    switch (operations[opcode]) {
    case BAD: return 0u;
    case LXI: case SHLD: case LHLD: case STA: case LDA: case JMP: case CALL: return 3u;
    case MVI: case ALUI: case OUT: case IN: case ESC: return 2u;
    default: return 1u;
    }
}

static void emulation_code(emulation_fixture *fixture, lib_u16 pc,
    lib_u8 opcode, lib_u16 immediate)
{
    const lib_u8 code[] = {opcode,(lib_u8)immediate,(lib_u8)(immediate >> 8u),0u,0u,0u};
    for (lib_u8 byte = 0u; byte < sizeof(code); ++byte)
        fixture->ram.memory[fixture->ram.bus.cpu.data.cs.base+(lib_u16)(pc+byte)] = code[byte];
    fixture->ram.bus.cpu.data.eip = pc;
}

static void emulation_prepare(emulation_fixture *fixture, lib_u8 opcode,
    lib_u16 pc, lib_u16 immediate, lib_u16 stack, lib_u16 hl, lib_u32 flags, lib_u8 alias)
{
    static const x86_core_cpu_bus_provider provider = {
        .read_memory=v30_memory_read,.write_memory=v30_memory_write,
        .transfer_port=emulation_port,.complete_port=emulation_complete,
        .interrupt_pending=cpu_bus_pending,.acknowledge_interrupt=cpu_bus_acknowledge
    };
    v30_memory_restart(&fixture->ram);
    lib_memory_set(&fixture->port,0,sizeof(*fixture)-lib_offsetof(emulation_fixture,port));
    fixture->ram.bus.execution.bus = &provider;
    fixture->ram.bus.execution.bus_context = fixture;
    x86_core_cpu_execution_context_bind_external_cycle_provider(&fixture->ram.bus.execution,
        emulation_cycle,fixture);
    t_cpu *cpu = &fixture->ram.bus.cpu;
    cpu->data.eax = 0xa5679b83u;
    cpu->data.ebx = 0xb6780000u | hl;
    cpu->data.ecx = 0xc7892315u;
    cpu->data.edx = 0xd89a3426u;
    cpu->data.ebp = 0xe9ab0000u | stack;
    cpu->data.esi = 0xfabc5648u;
    cpu->data.edi = 0x1bcd6759u;
    cpu->data.esp = 0x2cde786au;
    cpu->data.eflags = flags;
    if (alias == 1u) { cpu->data.ds.base=cpu->data.cs.base; cpu->data.ds.selector=cpu->data.cs.selector; }
    if (alias == 2u) { cpu->data.ds.base=0u; cpu->data.ds.selector=0u; }
    const lib_u16 addresses[] = {hl,cpu->data.cx,cpu->data.dx,stack,immediate};
    for (lib_u8 item = 0u; item < sizeof(addresses)/sizeof(addresses[0]); ++item)
        for (lib_i8 delta = -2; delta <= 2; ++delta)
            fixture->ram.memory[cpu->data.ds.base+(lib_u16)(addresses[item]+delta)] =
                (lib_u8)(0x69u+17u*item+delta);
    emulation_code(fixture,pc,opcode,immediate);
    fixture->port = (lib_u8)immediate;
    fixture->port_write = opcode == 0xd3u;
    fixture->port_data = 0x96u;
}

static lib_u8 *emulation_register(t_cpu *cpu, lib_u8 reg)
{
    lib_u8 *const registers[] = {&cpu->data.ch,&cpu->data.cl,&cpu->data.dh,
        &cpu->data.dl,&cpu->data.bh,&cpu->data.bl,LIB_NULL,&cpu->data.al};
    return registers[reg];
}

static lib_u16 *emulation_pair(t_cpu *cpu, lib_u8 pair)
{
    lib_u16 *const pairs[] = {&cpu->data.cx,&cpu->data.dx,&cpu->data.bx,&cpu->data.bp};
    return pairs[pair];
}

static lib_u16 emulation_planned_word(const emulation_fixture *fixture,
    v30_memory_plan *plan, lib_u16 offset)
{
    lib_u16 word = 0u;
    for (lib_u8 byte = 0u; byte < 2u; ++byte) {
        const lib_u32 address = fixture->ram.bus.cpu.data.ds.base+(lib_u16)(offset+byte);
        lib_u8 value = fixture->ram.memory[address];
        for (lib_u8 previous = 0u; previous < plan->count; ++previous) {
            const v30_memory_expected_transfer *transfer = &plan->transfers[previous];
            if (transfer->write && transfer->address == address) value = (lib_u8)transfer->value;
        }
        v30_memory_plan_transfer(plan,&fixture->ram,address,1u,LIB_FALSE,value);
        word |= (lib_u16)value << (8u*byte);
    }
    return word;
}

static void emulation_plan_word(const emulation_fixture *fixture, v30_memory_plan *plan,
    lib_u16 offset, lib_u16 word, lib_bool push)
{
    for (lib_u8 byte = 0u; byte < 2u; ++byte) {
        const lib_u8 part = push ? (lib_u8)(1u-byte) : byte;
        v30_memory_plan_transfer(plan,&fixture->ram,
            fixture->ram.bus.cpu.data.ds.base+(lib_u16)(offset+part),1u,LIB_TRUE,
            (lib_u8)(word >> (8u*part)));
    }
}

static lib_u8 emulation_szp(lib_u8 value)
{
    lib_u8 ones = 0u;
    for (lib_u8 bit = 0u; bit < 8u; ++bit) ones += (value >> bit) & 1u;
    return (value & 0x80u) | (value == 0u ? 0x40u : 0u) | (!(ones & 1u) ? 4u : 0u);
}

static lib_u32 emulation_flags(lib_u8 packed)
{
    static const lib_u8 masks[]={1u,4u,16u,64u,128u};
    lib_u32 flags=0xc02u;
    for (lib_u8 bit=0u; bit < sizeof(masks); ++bit)
        if (packed & (1u << bit)) flags |= masks[bit];
    return flags;
}

/* Independent integer/boolean oracle, not the production native ALU/flag
 * routines. Subtraction AC is the carry of A + ~operand + !borrow. */
static lib_u8 emulation_alu(lib_u8 group, lib_u8 a, lib_u8 b, lib_u32 *flags)
{
    const lib_u8 carry = (lib_u8)(*flags & 1u);
    lib_u16 wide;
    lib_bool auxiliary, cy;
    switch (group) {
    case 0u: case 1u:
        wide = (lib_u16)(a+b+(group == 1u ? carry : 0u));
        auxiliary = (a & 15u)+(b & 15u)+(group == 1u ? carry : 0u) > 15u;
        cy = wide > 255u;
        break;
    case 2u: case 3u: case 7u: {
        const lib_u16 subtrahend = (lib_u16)(b+(group == 3u ? carry : 0u));
        wide = (lib_u16)(a-subtrahend);
        auxiliary = (a & 15u)+(15u-(b & 15u))+(group == 3u ? 1u-carry : 1u) > 15u;
        cy = a < subtrahend;
        break;
    }
    case 4u: wide=a & b; auxiliary=!!((a | b) & 8u); cy=LIB_FALSE; break;
    case 5u: wide=a ^ b; auxiliary=LIB_FALSE; cy=LIB_FALSE; break;
    default: wide=a | b; auxiliary=LIB_FALSE; cy=LIB_FALSE; break;
    }
    *flags = (*flags & ~0xffu) | 2u | emulation_szp((lib_u8)wide) |
        (auxiliary ? 16u : 0u) | (cy ? 1u : 0u);
    return (lib_u8)wide;
}

static lib_bool emulation_condition(lib_u32 flags, lib_u8 condition)
{
    static const lib_u8 masks[] = {0x40u,0x40u,1u,1u,4u,4u,0x80u,0x80u};
    return (flags & masks[condition]) ? !!(condition & 1u) : !(condition & 1u);
}

static void emulation_expected(emulation_fixture *fixture, lib_u8 opcode,
    lib_u16 immediate, t_cpu *expected, v30_memory_plan *plan)
{
    const emulation_operation op = operations[opcode];
    const lib_u8 reg = (opcode/8u) % 8u, source = opcode % 8u, pair = (opcode/16u) % 4u;
    const lib_u16 address = expected->data.bx;
    lib_u8 value = 0u;
    lib_u16 word;
    expected->data.eip = (lib_u16)(expected->data.eip+emulation_length(opcode));
    if ((op >= ADD && op <= CMP) || op == MOV || op == INR || op == DCR) {
        const lib_u8 operand = op == INR || op == DCR ? reg : source;
        if (operand == 6u) {
            value = fixture->ram.memory[expected->data.ds.base+address];
            v30_memory_plan_transfer(plan,&fixture->ram,expected->data.ds.base+address,1u,LIB_FALSE,value);
        } else value = *emulation_register(expected,operand);
    }
    switch (op) {
    case BAD: case ESC: expected->data.eip=fixture->ram.bus.cpu.data.eip; break;
    case NOP: break;
    case LXI: *emulation_pair(expected,pair)=immediate; break;
    case STAX:
        v30_memory_plan_transfer(plan,&fixture->ram,expected->data.ds.base+
            *emulation_pair(expected,pair),1u,LIB_TRUE,expected->data.al);
        break;
    case LDAX: expected->data.al=fixture->ram.memory[expected->data.ds.base+*emulation_pair(expected,pair)];
        v30_memory_plan_transfer(plan,&fixture->ram,expected->data.ds.base+
            *emulation_pair(expected,pair),1u,LIB_FALSE,expected->data.al);
        break;
    case INX: ++*emulation_pair(expected,pair); break;
    case DCX: --*emulation_pair(expected,pair); break;
    case DAD: {
        const lib_u32 sum=(lib_u32)address+*emulation_pair(expected,pair);
        expected->data.bx=(lib_u16)sum;
        expected->data.eflags=(expected->data.eflags & ~1u) | (sum > 65535u);
        break;
    }
    case INR: case DCR: {
        const lib_u8 old_carry=(lib_u8)(expected->data.eflags & 1u);
        value=emulation_alu(op == INR ? 0u : 2u,value,1u,&expected->data.eflags);
        expected->data.eflags=(expected->data.eflags & ~1u) | old_carry;
        if (reg == 6u) v30_memory_plan_transfer(plan,&fixture->ram,expected->data.ds.base+address,1u,LIB_TRUE,value);
        else *emulation_register(expected,reg)=value;
        break;
    }
    case MVI: case MOV:
        if (op == MVI) value=(lib_u8)immediate;
        if (reg == 6u) v30_memory_plan_transfer(plan,&fixture->ram,expected->data.ds.base+address,1u,LIB_TRUE,value);
        else *emulation_register(expected,reg)=value;
        break;
    case HALT: expected->data.flagHalt=LIB_TRUE; break;
    case ADD: case ADC: case SUB: case SBB: case ANA: case XRA: case ORA: case CMP: case ALUI: {
        const lib_u8 group=op == ALUI ? reg : (lib_u8)(op-ADD);
        const lib_u8 result=emulation_alu(group,expected->data.al,
            op == ALUI ? (lib_u8)immediate : value,&expected->data.eflags);
        if (group != 7u) expected->data.al=result;
        break;
    }
    case RLC: case RRC: case RAL: case RAR: {
        const lib_u8 a=expected->data.al, carry=(lib_u8)(expected->data.eflags & 1u);
        const lib_bool left=op == RLC || op == RAL;
        const lib_u8 outgoing=left ? a >> 7u : a & 1u;
        expected->data.al=left ? (lib_u8)(a*2u+(op == RAL ? carry : outgoing)) :
            (lib_u8)(a/2u+128u*(op == RAR ? carry : outgoing));
        expected->data.eflags=(expected->data.eflags & ~1u) | outgoing;
        break;
    }
    case DAA: {
        lib_u16 adjusted=expected->data.al;
        const lib_bool low_adjust=(adjusted % 16u > 9u) || (expected->data.eflags & 16u);
        const lib_bool auxiliary=low_adjust && adjusted % 16u+6u > 15u;
        if (low_adjust) adjusted+=6u;
        const lib_bool high_adjust=adjusted/16u > 9u || (expected->data.eflags & 1u);
        if (high_adjust) adjusted+=0x60u;
        const lib_bool carry=adjusted > 255u || (expected->data.eflags & 1u);
        expected->data.al=(lib_u8)adjusted;
        expected->data.eflags=(expected->data.eflags & ~0xffu) | 2u |
            emulation_szp(expected->data.al) | (auxiliary ? 16u : 0u) | carry;
        break;
    }
    case CMA: expected->data.al=(lib_u8)~expected->data.al; break;
    case STC: expected->data.eflags |= 1u; break;
    case CMC: expected->data.eflags ^= 1u; break;
    case SHLD: emulation_plan_word(fixture,plan,immediate,address,LIB_FALSE); break;
    case LHLD: expected->data.bx=emulation_planned_word(fixture,plan,immediate); break;
    case STA: v30_memory_plan_transfer(plan,&fixture->ram,expected->data.ds.base+immediate,
        1u,LIB_TRUE,expected->data.al); break;
    case LDA: expected->data.al=fixture->ram.memory[expected->data.ds.base+immediate];
        v30_memory_plan_transfer(plan,&fixture->ram,expected->data.ds.base+immediate,
            1u,LIB_FALSE,expected->data.al); break;
    case JMP: case CALL:
        if (opcode == 0xc3u || opcode == 0xcdu || emulation_condition(expected->data.eflags,reg)) {
            if (op == CALL) {
                expected->data.bp-=2u;
                emulation_plan_word(fixture,plan,expected->data.bp,(lib_u16)expected->data.eip,LIB_TRUE);
            }
            expected->data.eip=immediate;
        }
        break;
    case RET:
        if (opcode == 0xc9u || emulation_condition(expected->data.eflags,reg)) {
            expected->data.eip=emulation_planned_word(fixture,plan,expected->data.bp);
            expected->data.bp+=2u;
        }
        break;
    case RST:
        expected->data.bp-=2u;
        emulation_plan_word(fixture,plan,expected->data.bp,(lib_u16)expected->data.eip,LIB_TRUE);
        expected->data.eip=opcode & 0x38u;
        break;
    case PUSH:
        word=pair == 3u ? (lib_u16)(256u*expected->data.al+(expected->data.flags & 0xd5u)+2u) :
            *emulation_pair(expected,pair);
        expected->data.bp-=2u;
        emulation_plan_word(fixture,plan,expected->data.bp,word,LIB_TRUE);
        break;
    case POP:
        word=emulation_planned_word(fixture,plan,expected->data.bp);
        expected->data.bp+=2u;
        if (pair == 3u) {
            expected->data.al=(lib_u8)(word >> 8u);
            expected->data.eflags=(expected->data.eflags & ~0xffu) | (word & 0xd5u) | 2u;
        } else *emulation_pair(expected,pair)=word;
        break;
    case XTHL:
        word=emulation_planned_word(fixture,plan,expected->data.bp);
        emulation_plan_word(fixture,plan,expected->data.bp,address,LIB_FALSE);
        expected->data.bx=word;
        break;
    case PCHL: expected->data.eip=address; break;
    case XCHG: expected->data.bx=expected->data.dx; expected->data.dx=address; break;
    case SPHL: expected->data.bp=address; break;
    case IN: expected->data.al=fixture->port_data; break;
    case OUT: break;
    case EI: expected->data.eflags |= 0x200u; break;
    case DI: expected->data.eflags &= ~0x200u; break;
    }
}

static lib_i32 emulation_nominal(const emulation_fixture *fixture, const t_cpu *before,
    lib_u8 opcode, const v30_memory_plan *plan)
{
    /* NEC8080AF printed194: independent operation table, not CPU dispatch. */
    static const lib_u8 base[]={
        [NOP]=4u,[LXI]=10u,[STAX]=7u,[INX]=5u,[INR]=5u,[DCR]=5u,[MVI]=7u,
        [RLC]=4u,[DAD]=10u,[LDAX]=7u,[DCX]=5u,[RRC]=4u,[RAL]=4u,[RAR]=4u,
        [SHLD]=16u,[DAA]=4u,[LHLD]=16u,[CMA]=4u,[STA]=13u,[STC]=4u,[LDA]=13u,
        [CMC]=4u,[MOV]=5u,[HALT]=7u,[ADD]=4u,[ADC]=4u,[SUB]=4u,[SBB]=4u,
        [ANA]=4u,[XRA]=4u,[ORA]=4u,[CMP]=4u,[RET]=10u,[POP]=10u,[JMP]=10u,
        [CALL]=17u,[PUSH]=11u,[ALUI]=7u,[RST]=11u,[OUT]=10u,[IN]=10u,
        [XTHL]=18u,[PCHL]=5u,[XCHG]=4u,[DI]=4u,[SPHL]=5u,[EI]=4u
    };
    const emulation_operation operation=operations[opcode];
    const lib_bool control=operation==RET || operation==JMP || operation==CALL ||
        operation==RST || operation==PCHL;
    const lib_bool taken=control && (operation==RST || operation==PCHL ||
        opcode==0xc9u || opcode==0xc3u || opcode==0xcdu ||
        emulation_condition(before->data.eflags,(opcode >> 3u) & 7u));
    lib_u64 clocks=base[operation];
    if (plan->count) {
        if (operation==MOV || operation==ADD || operation==ADC || operation==SUB ||
            operation==SBB || operation==ANA || operation==XRA || operation==ORA ||
            operation==CMP || operation==MVI) clocks=operation==MOV ? 7u :
                operation==MVI ? 10u : 7u;
        if (operation==INR || operation==DCR) clocks=10u;
    }
    if (operation==RET && opcode!=0xc9u) clocks=taken ? 11u : 5u;
    if (operation==CALL && !taken) clocks=11u;
    return v30_memory_check_nominal(&fixture->ram,clocks,0x01000000u | opcode,
        X86_CORE_CPU_TIMING_INPUT_MODE |
        (plan->count ? X86_CORE_CPU_TIMING_INPUT_MEMORY : 0u) |
        (control ? X86_CORE_CPU_TIMING_INPUT_CONTROL | X86_CORE_CPU_TIMING_INPUT_BRANCH : 0u) |
        (taken ? X86_CORE_CPU_TIMING_INPUT_BRANCH_TAKEN : 0u),
        X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_EMULATION_L2);
}

static lib_i32 emulation_check(emulation_fixture *fixture, lib_u8 opcode,
    lib_u16 immediate, lib_bool deep)
{
    const t_cpu before=fixture->ram.bus.cpu;
    t_cpu expected=before;
    v30_memory_plan plan={0};
    emulation_expected(fixture,opcode,immediate,&expected,&plan);
    const lib_bool unsupported=operations[opcode] == BAD || operations[opcode] == ESC;
    if (deep) {
        x86_core_cpu_instruction_lexeme lexeme;
        if (!!x86_core_cpu_execution_preview_lexeme(&fixture->ram.bus.execution,&lexeme) == unsupported ||
            (!unsupported && (lexeme.byte_count != emulation_length(opcode) ||
                lexeme.component_count != (emulation_length(opcode) == 3u ? 2u : emulation_length(opcode)))) ||
            lib_memory_compare(&fixture->ram.bus.cpu,&before,sizeof(before)) ||
            fixture->ram.reads || fixture->ram.writes || fixture->ram.fetches ||
            fixture->port_calls || fixture->ram.phase_count) return 1;
        fixture->ram.record_count=fixture->ram.observations=0u;
    }
    x86_core_cpu_execution_refresh(&fixture->ram.bus.execution);
    x86_core_cpu_instruction_observation observation;
    x86_core_cpu_execution_copy_observation(&fixture->ram.bus.execution,&observation);
    if (!unsupported && !observation.nec_emulation_mode) return 1;
    if (fixture->ram.bus.execution.stop_requested != unsupported ||
        fixture->ram.bus.faults != (lib_u32)unsupported || fixture->ram.bus.delivered_exceptions ||
        lib_memory_compare(&fixture->ram.bus.cpu,&expected,sizeof(expected)) ||
        v30_memory_check_plan(&fixture->ram,&plan,LIB_FALSE) || fixture->ram.invalid) {
        lib_c_printf("8080 opcode=%02x pc=%04x/%04x flags=%08x/%08x memory=%u stop=%u\n",
            opcode,(lib_u16)fixture->ram.bus.cpu.data.eip,(lib_u16)expected.data.eip,
            fixture->ram.bus.cpu.data.eflags,expected.data.eflags,plan.count,
            fixture->ram.bus.execution.stop_requested);
        return 1;
    }
    const lib_bool port=operations[opcode] == IN || operations[opcode] == OUT;
    if (fixture->port_calls != (lib_u32)port || fixture->port_completions != (lib_u32)port ||
        fixture->port_phases != 2u*(lib_u32)port || fixture->port_active ||
        (operations[opcode] == OUT && fixture->port_seen != before.data.al)) return 1;
    if (!unsupported && emulation_nominal(fixture,&before,opcode,&plan)) return 1;
    if (!deep || unsupported) return 0;
    const lib_u32 reads=fixture->ram.reads, writes=fixture->ram.writes,
        fetches=fixture->ram.fetches;
    for (lib_u8 kind=0u; kind < 4u; ++kind) {
        const lib_u32 count=kind == 0u ? reads : kind == 1u ? writes : kind == 2u ? fetches : port;
        for (lib_u32 ordinal=1u; ordinal <= count; ++ordinal) {
            emulation_fixture *retry=fixture;
            /* Recreate exactly the initial CPU/code/data, including aliases. */
            const lib_u8 alias=before.data.ds.base == before.data.cs.base ? 1u : before.data.ds.base == 0u ? 2u : 0u;
            emulation_prepare(retry,opcode,(lib_u16)before.data.eip,immediate,before.data.bp,
                before.data.bx,before.data.eflags,alias);
            retry->ram.bus.cpu=before;
            if (kind == 0u) retry->ram.fail_read=ordinal;
            else if (kind == 1u) retry->ram.fail_write=ordinal;
            else if (kind == 2u) retry->ram.fail_fetch=ordinal;
            else retry->port_failed=LIB_TRUE;
            x86_core_cpu_execution_refresh(&retry->ram.bus.execution);
            if (!retry->ram.bus.execution.stop_requested || retry->ram.bus.faults != 1u ||
                x86_core_cpu_capture_timing(&retry->ram.bus.execution).ticks ||
                retry->ram.bus.execution.completed_observation_valid ||
                retry->ram.bus.delivered_exceptions || retry->ram.invalid ||
                lib_memory_compare(&retry->ram.bus.cpu,&before,sizeof(before)) ||
                retry->port_active || retry->port_completions ||
                (kind != 2u && kind != 3u && v30_memory_check_plan(&retry->ram,&plan,LIB_TRUE))) {
                lib_c_printf("8080 failure opcode=%02x kind=%u ordinal=%u\n",opcode,kind,ordinal);
                return 1;
            }
        }
    }
    return 0;
}

static void emulation_store_word(emulation_fixture *fixture, lib_u32 address, lib_u16 word)
{
    fixture->ram.memory[address]=(lib_u8)word;
    fixture->ram.memory[address+1u]=(lib_u8)(word >> 8u);
}

static lib_i32 emulation_enter(emulation_fixture *fixture)
{
    const lib_u16 pc=(lib_u16)fixture->ram.bus.cpu.data.eip;
    fixture->ram.memory[0x80500u]=0x0fu;
    fixture->ram.memory[0x80501u]=0xffu;
    fixture->ram.memory[0x80502u]=0x80u;
    emulation_store_word(fixture,0x200u,pc);
    emulation_store_word(fixture,0x202u,0x8000u);
    fixture->ram.bus.cpu.data.eip=0x500u;
    fixture->ram.bus.cpu.data.eflags |= X86_CPU_NEC_PSW_MD;
    x86_core_cpu_execution_refresh(&fixture->ram.bus.execution);
    x86_core_cpu_instruction_observation observation;
    x86_core_cpu_execution_copy_observation(&fixture->ram.bus.execution,&observation);
    return fixture->ram.bus.execution.stop_requested ||
        observation.nec_emulation_mode ||
        !fixture->ram.bus.execution.nec_md_write_enabled ||
        (fixture->ram.bus.cpu.data.flags & X86_CPU_NEC_PSW_MD) ||
        fixture->ram.bus.cpu.data.eip != pc;
}

static lib_i32 emulation_program(void)
{
    static emulation_fixture fixture;
    static const lib_u8 program[]={
        0x31u,0x01u,0x07u,       /* LXI SP,0701 */
        0x21u,0xffu,0xffu,       /* LXI H,ffff */
        0x36u,0x36u,0x34u,0x7eu, /* MVI M,36; INR M; MOV A,M */
        0xc6u,0x05u,0xf5u,0xafu,0xf1u, /* ADI 5; PUSH PSW; XRA A; POP PSW */
        0xcdu,0x40u,0x02u,       /* CALL 0240 */
        0xd3u,0x27u,0xdbu,0x27u, /* OUT 27; IN 27 */
        0xedu,0xedu,0x81u,       /* CALLN 81 */
        0xedu,0xfdu              /* RETEM */
    };
    emulation_prepare(&fixture,0x0fu,0x100u,0x80ffu,0x902u,0x1234u,0x8c03u,0u);
    const lib_u32 native_sp=fixture.ram.bus.cpu.data.esp;
    const lib_u32 native_si=fixture.ram.bus.cpu.data.esi;
    emulation_store_word(&fixture,0x200u,0x200u);
    emulation_store_word(&fixture,0x202u,0x9000u);
    emulation_store_word(&fixture,0x204u,0x300u);
    emulation_store_word(&fixture,0x206u,0x8000u);
    lib_memory_copy(fixture.ram.memory+0x90200u,program,sizeof(program));
    fixture.ram.memory[0x90240u]=0x3cu; /* INR A */
    fixture.ram.memory[0x90241u]=0xc9u;
    fixture.ram.memory[0x80300u]=0x46u; /* Native INC SI; RETI */
    fixture.ram.memory[0x80301u]=0xcfu;
    fixture.port=0x27u;
    fixture.port_write=LIB_TRUE;
    lib_u8 retired=0u;
    do {
        if (fixture.ram.bus.cpu.data.cs.base == 0x90000u &&
            fixture.ram.bus.cpu.data.eip == 0x214u) {
            fixture.port_write=LIB_FALSE;
            fixture.port_phases=0u;
        }
        const lib_bool emulation=(fixture.ram.bus.cpu.data.flags & X86_CPU_NEC_PSW_MD)==0u;
        x86_core_cpu_execution_refresh(&fixture.ram.bus.execution);
        x86_core_cpu_instruction_observation observation;
        x86_core_cpu_execution_copy_observation(&fixture.ram.bus.execution,&observation);
        if (observation.nec_emulation_mode!=emulation) return 1;
        if (fixture.ram.bus.execution.stop_requested || fixture.ram.invalid) return 1;
        if (++retired > 32u) return 1;
    } while (!(fixture.ram.bus.cpu.data.flags & X86_CPU_NEC_PSW_MD) ||
        fixture.ram.bus.cpu.data.cs.base != 0x80000u || fixture.ram.bus.cpu.data.eip != 0x103u);
    const t_cpu *cpu=&fixture.ram.bus.cpu;
    return retired != 19u || cpu->data.al != 0x96u || cpu->data.ah != 0x9bu ||
        cpu->data.flags != 0x8c03u || cpu->data.bx != 0xffffu || cpu->data.bp != 0x701u ||
        cpu->data.esp != native_sp || cpu->data.esi != native_si+1u ||
        fixture.ram.memory[0x1ffffu] != 0x37u || fixture.port_calls != 2u ||
        fixture.port_completions != 2u || fixture.port_seen != 0x3du ||
        fixture.port_active || fixture.ram.bus.execution.nec_md_write_enabled;
}

static lib_i32 emulation_interrupts(void)
{
    static emulation_fixture fixture;
    /* EI cannot acknowledge INT until the following emulation instruction.
     * Native RETI then resumes that instruction's successor in 8080 mode. */
    emulation_prepare(&fixture,0xfbu,0x100u,0u,0x702u,0x1234u,2u,0u);
    if (emulation_enter(&fixture)) return 1;
    fixture.ram.memory[0x80101u]=0u;
    fixture.ram.memory[0x90200u]=0xcfu;
    emulation_store_word(&fixture,0xc0u,0x200u);
    emulation_store_word(&fixture,0xc2u,0x9000u);
    fixture.ram.bus.interrupt=LIB_TRUE;
    x86_core_cpu_execution_refresh(&fixture.ram.bus.execution);
    if (fixture.ram.bus.cpu.data.eip != 0x101u || fixture.ram.bus.acknowledgements ||
        !(fixture.ram.bus.cpu.data.flags & 0x200u) ||
        fixture.ram.bus.execution.interrupt_shadow != CPU_INTERRUPT_SHADOW_INTR) return 1;
    x86_core_cpu_execution_refresh(&fixture.ram.bus.execution);
    if (fixture.ram.bus.cpu.data.cs.base != 0x90000u || fixture.ram.bus.cpu.data.eip != 0x200u ||
        fixture.ram.bus.acknowledgements != 1u ||
        !(fixture.ram.bus.cpu.data.flags & X86_CPU_NEC_PSW_MD)) return 1;
    x86_core_cpu_instruction_observation observation;
    x86_core_cpu_execution_copy_observation(&fixture.ram.bus.execution,&observation);
    if (!observation.nec_emulation_mode) return 1;
    x86_core_cpu_execution_refresh(&fixture.ram.bus.execution);
    if (fixture.ram.bus.cpu.data.cs.base != 0x80000u || fixture.ram.bus.cpu.data.eip != 0x102u ||
        (fixture.ram.bus.cpu.data.flags & X86_CPU_NEC_PSW_MD) ||
        fixture.ram.bus.execution.stop_requested) return 1;
    /* DI suppresses acknowledgement, but a held INT still wakes V30 HALT,
     * without a vector or mode change. Intel's existing IF-qualified wake stays. */
    for (lib_u8 native=0u; native < 2u; ++native)
        for (lib_u8 enabled=0u; enabled < 2u; ++enabled)
            for (lib_u8 nmi=0u; nmi < 2u; ++nmi) {
                emulation_prepare(&fixture,native ? 0xf4u : 0x76u,0x100u,0u,0x702u,0x1234u,
                    2u | (native ? X86_CPU_NEC_PSW_MD : 0u) | (enabled ? 0x200u : 0u),0u);
                if (!native && emulation_enter(&fixture)) return 1;
                const t_cpu before=fixture.ram.bus.cpu;
                const lib_u8 vector=nmi ? 2u : 0x30u;
                emulation_store_word(&fixture,4u*vector,0x200u);
                emulation_store_word(&fixture,4u*vector+2u,0x9000u);
                fixture.ram.memory[0x90200u]=0xcfu;
                x86_core_cpu_execution_refresh(&fixture.ram.bus.execution);
                if (!fixture.ram.bus.cpu.data.flagHalt || fixture.ram.bus.cpu.data.eip != 0x101u) return 1;
                x86_core_cpu_execution_refresh(&fixture.ram.bus.execution);
                if (!fixture.ram.bus.cpu.data.flagHalt || fixture.ram.bus.cpu.data.eip != 0x101u) return 1;
                if (nmi) {
                    if (!x86_core_cpu_request_nmi(&fixture.ram.bus.execution)) return 1;
                } else fixture.ram.bus.interrupt=LIB_TRUE;
                x86_core_cpu_execution_refresh(&fixture.ram.bus.execution);
                if (fixture.ram.bus.cpu.data.flagHalt || fixture.ram.bus.execution.stop_requested ||
                    fixture.ram.bus.acknowledgements != (lib_u32)(enabled && !nmi)) return 1;
                if (nmi || enabled) {
                    if (fixture.ram.bus.cpu.data.cs.base != 0x90000u ||
                        fixture.ram.bus.cpu.data.eip != 0x200u ||
                        !(fixture.ram.bus.cpu.data.flags & X86_CPU_NEC_PSW_MD)) return 1;
                    x86_core_cpu_execution_refresh(&fixture.ram.bus.execution);
                }
                t_cpu expected=before;
                expected.data.eip=0x101u;
                if (lib_memory_compare(&fixture.ram.bus.cpu,&expected,sizeof(expected))) return 1;
            }
    emulation_prepare(&fixture,0xf3u,0x100u,0u,0x702u,0x1234u,0x202u,0u);
    fixture.ram.bus.interrupt=LIB_TRUE;
    x86_core_cpu_execution_refresh(&fixture.ram.bus.execution);
    return fixture.ram.bus.execution.stop_requested || fixture.ram.bus.acknowledgements ||
        fixture.ram.bus.cpu.data.eip != 0x101u || (fixture.ram.bus.cpu.data.flags & 0x200u);
}

lib_i32 main(void)
{
    static emulation_fixture fixture;
    lib_u32 cases=0u, admitted=0u;
    for (lib_u16 opcode=0u; opcode < 256u; ++opcode) {
        if (operations[opcode] != BAD) ++admitted;
        for (lib_u8 flags=0u; flags < 32u; ++flags) {
            const lib_u32 psw=0x40000000u | emulation_flags(flags);
            emulation_prepare(&fixture,(lib_u8)opcode,0x100u,0x4567u,0x702u,0x1234u,psw,0u);
            if (emulation_check(&fixture,(lib_u8)opcode,0x4567u,flags == 0u)) return 1;
            ++cases;
        }
    }
    if (admitted != 245u) return 1;
    static const lib_u16 edges[]={0u,1u,0xfffeu,0xffffu};
    for (lib_u16 opcode=0u; opcode < 256u; ++opcode) {
        if (operations[opcode] == ESC) continue;
        for (lib_u8 edge=0u; edge < sizeof(edges)/sizeof(edges[0]); ++edge)
            for (lib_u8 alias=0u; alias < 3u; ++alias) {
                const lib_u16 pc=edges[edge], address=edges[3u-edge];
                emulation_prepare(&fixture,(lib_u8)opcode,pc,address,address,address,0xc97u,alias);
                if (emulation_check(&fixture,(lib_u8)opcode,address,LIB_TRUE)) return 1;
                ++cases;
            }
    }
    /* All 8 ALUs, every byte pair/carry through register, M and immediate.
     * Other register mappings are covered by the complete slot matrix. */
    for (lib_u8 group=0u; group < 8u; ++group)
        for (lib_u16 a=0u; a < 256u; ++a)
            for (lib_u16 b=0u; b < 256u; ++b)
                for (lib_u8 carry=0u; carry < 2u; ++carry)
                    for (lib_u8 operand=0u; operand < 3u; ++operand) {
                        const lib_u8 bases[]={0x80u,0x86u,0xc6u};
                        const lib_u8 opcode=(lib_u8)(bases[operand]+8u*group);
                        emulation_prepare(&fixture,opcode,0x100u,b,0x702u,0x1234u,0xc02u | carry,0u);
                        fixture.ram.bus.cpu.data.al=(lib_u8)a;
                        fixture.ram.bus.cpu.data.ch=(lib_u8)b;
                        fixture.ram.memory[0x11234u]=(lib_u8)b;
                        if (emulation_check(&fixture,opcode,b,LIB_FALSE)) return 1;
                        ++cases;
                    }
    for (lib_u16 a=0u; a < 256u; ++a)
        for (lib_u8 flags=0u; flags < 32u; ++flags) {
            static const lib_u8 unary[]={0x27u,0x07u,0x0fu,0x17u,0x1fu,0x3cu,0x3du};
            for (lib_u8 item=0u; item < sizeof(unary); ++item) {
                emulation_prepare(&fixture,unary[item],0x100u,0u,0x702u,0x1234u,emulation_flags(flags),0u);
                fixture.ram.bus.cpu.data.al=(lib_u8)a;
                if (emulation_check(&fixture,unary[item],0u,LIB_FALSE)) return 1;
                ++cases;
            }
        }
    for (lib_u8 pair=0u; pair < 4u; ++pair)
        for (lib_u32 value=0u; value < 65536u; ++value) {
            const lib_u8 opcodes[]={(lib_u8)(0x01u+16u*pair),(lib_u8)(0x03u+16u*pair),
                (lib_u8)(0x0bu+16u*pair),(lib_u8)(0x09u+16u*pair)};
            for (lib_u8 item=0u; item < sizeof(opcodes); ++item) {
                const lib_u8 opcode=opcodes[item];
                emulation_prepare(&fixture,opcode,0x100u,(lib_u16)value,0x702u,0x8001u,0xc97u,0u);
                *emulation_pair(&fixture.ram.bus.cpu,pair)=(lib_u16)value;
                if (emulation_check(&fixture,opcode,(lib_u16)value,LIB_FALSE)) return 1;
                ++cases;
            }
        }
    for (lib_u32 word=0u; word < 65536u; ++word) {
        emulation_prepare(&fixture,0xf1u,0x100u,0u,0x702u,0x1234u,0xc02u,0u);
        emulation_store_word(&fixture,0x10702u,(lib_u16)word);
        if (emulation_check(&fixture,0xf1u,0u,LIB_FALSE)) return 1;
        ++cases;
    }
    for (lib_u16 port=0u; port < 256u; ++port)
        for (lib_u8 write=0u; write < 2u; ++write) {
            const lib_u8 opcode=write ? 0xd3u : 0xdbu;
            emulation_prepare(&fixture,opcode,0xffffu,port,0x702u,0x1234u,0xc97u,0u);
            if (emulation_check(&fixture,opcode,port,LIB_TRUE)) return 1;
            ++cases;
        }
    if (emulation_program()) { lib_c_printf("V30:EMULATION:PROGRAM:FAIL\n"); return 1; }
    if (emulation_interrupts()) { lib_c_printf("V30:EMULATION:INTERRUPT:FAIL\n"); return 1; }
    lib_c_printf("V30:EMULATION:OK slots=%u cases=%u\n",admitted,cases);
    lib_c_printf("V30:EMULATION-NOMINAL-L2:OK\n");
    return 0;
}
