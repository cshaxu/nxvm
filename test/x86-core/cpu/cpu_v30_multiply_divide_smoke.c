#include "support/cpu_v30_memory_fixture.h"
#include "lib/types/file.h"

typedef enum v30_multiply_operation {
    V30_MULTIPLY_UNSIGNED, V30_MULTIPLY_SIGNED,
    V30_DIVIDE_UNSIGNED, V30_DIVIDE_SIGNED,
    V30_MULTIPLY_IMMEDIATE_BYTE, V30_MULTIPLY_IMMEDIATE_WORD
} v30_multiply_operation;

typedef struct v30_multiply_case {
    v30_multiply_operation operation;
    lib_u32 dividend;
    lib_u16 operand, immediate;
    lib_u8 width, modrm, prefix;
    lib_bool flags;
} v30_multiply_case;

static lib_u32 *v30_multiply_word(t_cpu *cpu, lib_u8 reg)
{
    lib_u32 *words[] = {&cpu->data.eax,&cpu->data.ecx,&cpu->data.edx,&cpu->data.ebx,
        &cpu->data.esp,&cpu->data.ebp,&cpu->data.esi,&cpu->data.edi};
    return words[reg];
}

static lib_u16 v30_multiply_register(t_cpu *cpu, lib_u8 reg, lib_u8 width)
{
    const lib_u32 word = *v30_multiply_word(cpu,width == 2u ? reg : reg % 4u);
    return width == 2u ? (lib_u16)word : (lib_u8)(word >> (reg < 4u ? 0u : 8u));
}

static lib_u32 v30_multiply_address(t_cpu *cpu, const v30_multiply_case *item)
{
    const lib_u8 mod = item->modrm >> 6u, rm = item->modrm & 7u;
    const lib_u16 offsets[] = {cpu->data.bx+cpu->data.si,cpu->data.bx+cpu->data.di,
        cpu->data.bp+cpu->data.si,cpu->data.bp+cpu->data.di,cpu->data.si,
        cpu->data.di,cpu->data.bp,cpu->data.bx};
    const lib_bool stack = rm == 2u || rm == 3u || (rm == 6u && mod != 0u);
    const lib_u32 bases[] = {stack ? 0x20000u : 0x10000u,0x30000u,0x80000u,0x20000u,0x10000u};
    return bases[item->prefix] + (lib_u16)(mod == 0u && rm == 6u ? 0xffffu :
        offsets[rm] + (mod == 1u ? 0xff80u : mod == 2u ? 0xffffu : 0u));
}

static lib_u8 v30_multiply_prepare(v30_memory_fixture *fixture, const v30_multiply_case *item)
{
    static const lib_u8 prefixes[] = {0u,0x26u,0x2eu,0x36u,0x3eu};
    lib_u8 code[8], length = 0u;
    const lib_u8 mod = item->modrm >> 6u, rm = item->modrm & 7u;
    v30_memory_restart(fixture);
    for (lib_u8 reg = 0u; reg < 8u; ++reg)
        *v30_multiply_word(&fixture->bus.cpu,reg) = 0x12340000u + reg * 0x1100u + 0x701u;
    fixture->bus.cpu.data.ax = (lib_u16)item->dividend;
    fixture->bus.cpu.data.dx = (lib_u16)(item->dividend >> 16u);
    fixture->bus.cpu.data.eflags |= VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
    if (item->flags) fixture->bus.cpu.data.eflags |= VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF;
    if (mod == 3u) {
        lib_u32 *word = v30_multiply_word(&fixture->bus.cpu,item->width == 2u ? rm : rm % 4u);
        const lib_u8 shift = item->width == 1u && rm >= 4u ? 8u : 0u;
        const lib_u32 mask = (item->width == 2u ? 0xffffu : 0xffu) << shift;
        *word = (*word & ~mask) | (((lib_u32)item->operand << shift) & mask);
    } else {
        const lib_u32 address = v30_multiply_address(&fixture->bus.cpu,item);
        fixture->memory[address] = (lib_u8)item->operand;
        fixture->memory[address+1u] = (lib_u8)(item->operand >> 8u);
    }
    if (item->prefix) code[length++] = prefixes[item->prefix];
    code[length++] = item->operation >= V30_MULTIPLY_IMMEDIATE_BYTE ?
        (item->operation == V30_MULTIPLY_IMMEDIATE_BYTE ? 0x6bu : 0x69u) : 0xf5u+item->width;
    code[length++] = item->modrm;
    if (mod < 3u && (mod != 0u || rm == 6u)) {
        code[length++] = mod == 1u ? 0x80u : 0xffu;
        if (mod != 1u) code[length++] = 0xffu;
    }
    if (item->operation >= V30_MULTIPLY_IMMEDIATE_BYTE) {
        code[length++] = (lib_u8)item->immediate;
        if (item->operation == V30_MULTIPLY_IMMEDIATE_WORD) code[length++] = (lib_u8)(item->immediate >> 8u);
    }
    lib_memory_copy(fixture->memory+0x80100u,code,length);
    lib_memory_copy(fixture->memory,(const lib_u8[]){0u,2u,0u,0x90u},4u);
    fixture->memory[0x90200u] = 0xcfu;
    const lib_u16 sp = fixture->bus.cpu.data.sp;
    for (lib_u8 word = 1u; word <= 4u; ++word) {
        const lib_u32 address = 0x20000u+(lib_u16)(sp-2u*word);
        fixture->memory[address] = fixture->memory[address+1u] = 0xa5u;
    }
    return length;
}

/* Wide arithmetic computes results before any destination alias is changed. */
static lib_bool v30_multiply_expected(t_cpu *cpu, const v30_multiply_case *item,
    lib_u16 operand, lib_u32 *defined)
{
    const lib_u8 bits = item->width * 8u;
    const lib_u32 mask = item->width == 1u ? 255u : 65535u;
    const lib_i64 sign = (mask+1u)/2u;
    const lib_bool divide = item->operation == V30_DIVIDE_UNSIGNED || item->operation == V30_DIVIDE_SIGNED;
    const lib_bool signed_operation = item->operation != V30_MULTIPLY_UNSIGNED &&
        item->operation != V30_DIVIDE_UNSIGNED;
    lib_i64 left = divide ? (item->width == 1u ? cpu->data.ax :
        (lib_u32)cpu->data.ax + 65536u*cpu->data.dx) : cpu->data.ax & mask;
    lib_i64 right = operand & mask;
    if (signed_operation) {
        const lib_i64 dividend_range = divide ? (mask+1ull)*(mask+1ull) : mask+1ull;
        if (left >= dividend_range/2) left -= dividend_range;
        if (right >= sign) right -= mask+1u;
    }
    if (item->operation >= V30_MULTIPLY_IMMEDIATE_BYTE) {
        left = right;
        const lib_u32 range = item->operation == V30_MULTIPLY_IMMEDIATE_BYTE ? 256u : 65536u;
        right = item->immediate % range;
        if (right >= range/2u) right -= range;
    }
    if (divide) {
        *defined = ~(VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF |
            VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_CF);
        if (!right) return LIB_TRUE;
        const lib_i64 quotient = left/right, remainder = left-quotient*right;
        if (signed_operation ? quotient < 1-sign || quotient >= sign : quotient > mask) return LIB_TRUE;
        if (item->width == 1u) cpu->data.ax = (lib_u8)quotient + 256u*(lib_u8)remainder;
        else { cpu->data.ax = (lib_u16)quotient; cpu->data.dx = (lib_u16)remainder; }
        return LIB_FALSE;
    }
    const lib_i64 product = left*right;
    const lib_bool overflow = signed_operation ? product < -sign || product >= sign : product > mask;
    *defined = ~(VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
    cpu->data.eflags = (cpu->data.eflags & ~(VCPU_EFLAGS_CF | VCPU_EFLAGS_OF)) |
        (overflow ? VCPU_EFLAGS_CF | VCPU_EFLAGS_OF : 0u);
    if (item->operation >= V30_MULTIPLY_IMMEDIATE_BYTE)
        *v30_multiply_word(cpu,(item->modrm >> 3u) & 7u) =
            (*v30_multiply_word(cpu,(item->modrm >> 3u) & 7u) & 0xffff0000u) | (lib_u16)product;
    else if (item->width == 1u) cpu->data.ax = (lib_u16)product;
    else { cpu->data.ax = (lib_u16)product; cpu->data.dx = (lib_u16)((lib_u32)product >> bits); }
    return LIB_FALSE;
}

static lib_i32 v30_multiply_check(const v30_multiply_case *item, lib_bool deep)
{
    static v30_memory_fixture fixture;
    const lib_u8 length = v30_multiply_prepare(&fixture,item);
    const t_cpu before = fixture.bus.cpu;
    t_cpu expected = before;
    const lib_bool memory = (item->modrm >> 6u) != 3u;
    const lib_u32 address = memory ? v30_multiply_address(&expected,item) : 0u;
    const lib_u16 operand = memory ? fixture.memory[address] +
        (item->width == 2u ? 256u*fixture.memory[address+1u] : 0u) :
        v30_multiply_register(&expected,item->modrm & 7u,item->width);
    lib_u32 defined;
    const lib_bool trap = v30_multiply_expected(&expected,item,operand,&defined);
    v30_memory_plan plan = {0};
    if (memory) v30_memory_plan_transfer(&plan,&fixture,address,item->width,LIB_FALSE,operand);
    const lib_u16 next_ip = 0x100u+length;
    if (trap) {
        const lib_u16 frame[] = {(lib_u16)(before.data.eflags | 0x7000u),0x8000u,next_ip};
        for (lib_u8 word = 0u; word < 3u; ++word)
            v30_memory_plan_transfer(&plan,&fixture,0x20000u+(lib_u16)(before.data.sp-2u*(word+1u)),
                2u,LIB_TRUE,frame[word]);
        v30_memory_plan_transfer(&plan,&fixture,0u,2u,LIB_FALSE,0x200u);
        v30_memory_plan_transfer(&plan,&fixture,2u,2u,LIB_FALSE,0x9000u);
        expected.data.cs.selector = 0x9000u;
        expected.data.cs.base = 0x90000u;
        expected.data.eip = 0x200u;
        expected.data.sp -= 6u;
        expected.data.eflags &= ~(VCPU_EFLAGS_IF | VCPU_EFLAGS_TF);
    } else expected.data.eip = next_ip;
    if (deep) {
        x86_core_cpu_instruction_lexeme lexeme;
        if (!x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
            lexeme.byte_count != length || fixture.reads || fixture.writes || fixture.fetches ||
            fixture.phase_count || lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))) {
            lib_c_printf("V30:MULTIPLY-DIVIDE:PREVIEW op=%u modrm=%02x prefix=%u\n",
                item->operation,item->modrm,item->prefix);
            return 1;
        }
        for (lib_u32 index = 0u; index < fixture.record_count; ++index)
            if (!fixture.records[index].observe ||
                fixture.records[index].provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA) return 1;
        fixture.record_count = fixture.observations = 0u;
    }
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    t_cpu actual = fixture.bus.cpu;
    actual.data.eflags &= defined;
    expected.data.eflags &= defined;
    if (fixture.bus.execution.stop_requested || fixture.bus.faults ||
        fixture.bus.delivered_exceptions != (lib_u32)trap ||
        (trap && fixture.bus.delivered_exception.exception_mask != VCPUINS_EXCEPT_DE) ||
        fixture.bus.instruction_count != 1u || v30_memory_check_completion(&fixture,LIB_TRUE,LIB_FALSE) ||
        lib_memory_compare(&actual,&expected,sizeof(actual)) ||
        v30_memory_check_plan(&fixture,&plan,LIB_FALSE)) {
        lib_c_printf("V30:MULTIPLY-DIVIDE:FAIL op=%u width=%u modrm=%02x prefix=%u input=%08x operand=%04x trap=%u ax=%04x/%04x dx=%04x/%04x ip=%04x/%04x sp=%04x/%04x\n",
            item->operation,item->width,item->modrm,item->prefix,item->dividend,item->operand,trap,
            actual.data.ax,expected.data.ax,actual.data.dx,expected.data.dx,
            actual.data.eip,expected.data.eip,actual.data.sp,expected.data.sp);
        if (trap) {
            const lib_u32 stack = 0x20000u+actual.data.sp;
            lib_c_printf("V30:MULTIPLY-DIVIDE:FRAME saved=%04x expected=%04x records=%u\n",
                fixture.memory[stack]+256u*fixture.memory[stack+1u],next_ip,
                v30_memory_check_plan(&fixture,&plan,LIB_FALSE));
        }
        return 1;
    }
    const lib_u32 reads = fixture.reads, writes = fixture.writes;
    if (deep && trap) {
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        t_cpu resumed = before;
        resumed.data.eip = next_ip;
        if (fixture.bus.execution.stop_requested ||
            lib_memory_compare(&fixture.bus.cpu,&resumed,sizeof(resumed))) {
            lib_c_printf("V30:MULTIPLY-DIVIDE:IRET op=%u modrm=%02x cs=%04x ip=%04x/%04x sp=%04x/%04x flags=%04x/%04x\n",
                item->operation,item->modrm,fixture.bus.cpu.data.cs.selector,fixture.bus.cpu.data.eip,
                resumed.data.eip,fixture.bus.cpu.data.sp,resumed.data.sp,
                fixture.bus.cpu.data.eflags,resumed.data.eflags);
            return 1;
        }
    }
    if (deep) for (lib_u8 write = 0u; write < 2u; ++write)
        for (lib_u32 part = 1u; part <= (write ? writes : reads); ++part) {
            v30_multiply_prepare(&fixture,item);
            if (write) fixture.fail_write = part;
            else fixture.fail_read = part;
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (!fixture.bus.execution.stop_requested || !fixture.bus.faults || fixture.bus.delivered_exceptions ||
                v30_memory_check_completion(&fixture,LIB_FALSE,LIB_FALSE) ||
                fixture.bus.execution.interrupt_shadow != CPU_INTERRUPT_SHADOW_NONE ||
                lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before)) ||
                v30_memory_check_plan(&fixture,&plan,LIB_TRUE)) {
                lib_c_printf("V30:MULTIPLY-DIVIDE:REFUSAL op=%u modrm=%02x width=%u prefix=%u write=%u part=%u stop=%u faults=%u delivered=%u records=%u\n",
                    item->operation,item->modrm,item->width,item->prefix,write,part,
                    fixture.bus.execution.stop_requested,fixture.bus.faults,fixture.bus.delivered_exceptions,
                    v30_memory_check_plan(&fixture,&plan,LIB_TRUE));
                return 1;
            }
        }
    return 0;
}

lib_i32 main(void)
{
    static const lib_u16 values[] = {0u,1u,2u,127u,128u,255u,256u,0x7fffu,0x8000u,0xfffeu,0xffffu};
    static const lib_u8 divisors[] = {0u,1u,2u,127u,128u,129u,254u,255u};
    lib_u32 cases = 0u;
    for (lib_u8 width = 1u; width <= 2u; ++width) {
        const v30_multiply_case item = {.operation=V30_DIVIDE_SIGNED,.width=width,
            .modrm=0xf9u,.dividend=width == 1u ? 0xff80u : 0xffff8000u,.operand=1u};
        if (v30_multiply_check(&item,LIB_TRUE)) return 1;
        ++cases;
    }
    for (lib_u8 operation = 0u; operation < 6u; ++operation)
        for (lib_u8 width = 1u; width <= 2u; ++width) {
            if (operation >= V30_MULTIPLY_IMMEDIATE_BYTE && width == 1u) continue;
            for (lib_u16 modrm = 0u; modrm <= 255u; ++modrm) {
                if (operation < V30_MULTIPLY_IMMEDIATE_BYTE && ((modrm >> 3u) & 7u) != operation+4u) continue;
                for (lib_u8 prefix = 0u; prefix < 5u; ++prefix)
                    for (lib_u8 value = 0u; value < sizeof(values)/sizeof(values[0]); ++value)
                        for (lib_u8 flags = 0u; flags < 2u; ++flags) {
                            const v30_multiply_case item = {.operation=(v30_multiply_operation)operation,
                                .width=width,.modrm=(lib_u8)modrm,.prefix=prefix,.operand=values[value],
                                .immediate=values[(value+3u) % (sizeof(values)/sizeof(values[0]))],
                                .dividend=value & 1u ? 0xffff8000u : 0x00010080u,.flags=flags != 0u};
                            if (v30_multiply_check(&item,LIB_TRUE)) return 1;
                            ++cases;
                        }
            }
        }
    for (lib_u8 operation = 0u; operation < 4u; ++operation)
        for (lib_u32 left = 0u; left < (operation < V30_DIVIDE_UNSIGNED ? 256u : 65536u); ++left)
            for (lib_u16 right = 0u; right < (operation < V30_DIVIDE_UNSIGNED ? 256u : sizeof(divisors)); ++right) {
                const v30_multiply_case item = {.operation=(v30_multiply_operation)operation,.width=1u,
                    .modrm=(lib_u8)(0xc1u+8u*(operation+4u)),.dividend=left,
                    .operand=operation < V30_DIVIDE_UNSIGNED ? right : divisors[right],.flags=(left & 1u) != 0u};
                if (v30_multiply_check(&item,LIB_FALSE)) return 1;
                ++cases;
            }
    for (lib_u8 operation = 0u; operation < 6u; ++operation)
        for (lib_u8 high = 0u; high < sizeof(values)/sizeof(values[0]); ++high)
            for (lib_u8 low = 0u; low < sizeof(values)/sizeof(values[0]); ++low)
                for (lib_u8 operand = 0u; operand < sizeof(values)/sizeof(values[0]); ++operand) {
                    const v30_multiply_case item = {.operation=(v30_multiply_operation)operation,.width=2u,
                        .modrm=operation < V30_MULTIPLY_IMMEDIATE_BYTE ? (lib_u8)(0xc1u+8u*(operation+4u)) : 0xc1u,
                        .dividend=65536u*values[high]+values[low],.operand=values[operand],.immediate=values[high]};
                    if (v30_multiply_check(&item,LIB_FALSE)) return 1;
                    ++cases;
                }
    for (lib_u8 operation = V30_MULTIPLY_IMMEDIATE_BYTE; operation <= V30_MULTIPLY_IMMEDIATE_WORD; ++operation)
        for (lib_u32 value = 0u; value <= 65535u; ++value)
            for (lib_u8 boundary = 0u; boundary < 4u; ++boundary) {
                const v30_multiply_case item = {.operation=(v30_multiply_operation)operation,.width=2u,
                    .modrm=0xc1u,.operand=operation == V30_MULTIPLY_IMMEDIATE_BYTE ? (lib_u16)value : values[boundary+7u],
                    .immediate=operation == V30_MULTIPLY_IMMEDIATE_BYTE ? divisors[boundary+4u] : (lib_u16)value};
                if (v30_multiply_check(&item,LIB_FALSE)) return 1;
                ++cases;
            }
    lib_c_printf("V30:MULTIPLY-DIVIDE:OK cases=%u\n",cases);
    return 0;
}
