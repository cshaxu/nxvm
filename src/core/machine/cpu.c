/* Copyright 2012-2014 Neko. */

/* VCPU defines the Central Processing Unit. */

#include "type.h"

#include "core/machine/cpu_instructions.h"

#include "core/machine/cpu.h"

#define cpu_state (*context->cpu)
#define instruction_state (*context->instructions)

void core_machine_cpu_execution_context_initialize(
    core_machine_cpu_execution_context *context, t_cpu *cpu,
    t_cpuins *instructions, t_ram *memory, t_port *port)
{
    if (context == NULL) return;
    context->cpu = cpu;
    context->instructions = instructions;
    context->memory = memory;
    context->port = port;
    context->pic_master = NULL;
    context->pic_slave = NULL;
    if (context->trace == NULL) {
        context->trace = (t_utils_trace *)MALLOC(sizeof(*context->trace));
    }
    if (context->trace != NULL) {
        MEMSET((void *)context->trace, Zero8, sizeof(*context->trace));
    }
    context->extension_context = NULL;
    context->stop_requested = False;
    context->reset_requested = False;
}

void core_machine_cpu_execution_context_bind_pic(
    core_machine_cpu_execution_context *context, t_pic *master,
    t_pic *slave)
{
    if (context == NULL) return;
    context->pic_master = master;
    context->pic_slave = slave;
}

void core_machine_cpu_execution_context_bind_extension(
    core_machine_cpu_execution_context *context, void *extension_context)
{
    if (context != NULL) context->extension_context = extension_context;
}

void *core_machine_cpu_execution_context_extension(
    const core_machine_cpu_execution_context *context)
{
    return context == NULL ? NULL : context->extension_context;
}

void core_machine_cpu_state_initialize(
    core_machine_cpu_execution_context *context) {
    if (context == NULL || context->cpu == NULL ||
        context->instructions == NULL) return;
    if (context != NULL) {
        context->stop_requested = False;
        context->reset_requested = False;
    }
    core_machine_cpu_execution_initialize(context);
}
void core_machine_cpu_state_reset(core_machine_cpu_execution_context *context) {
    if (context == NULL || context->cpu == NULL ||
        context->instructions == NULL) return;
    MEMSET((void *)context->cpu, Zero8, sizeof(t_cpu));
    if (context != NULL) {
        context->stop_requested = False;
        context->reset_requested = False;
    }

    cpu_state.data.eip = 0x0000fff0;
    cpu_state.data.eflags = 0x00000002;

    cpu_state.data.cs.base = 0xffff0000;
    cpu_state.data.cs.dpl = Zero4;
    cpu_state.data.cs.limit = Max32;
    cpu_state.data.cs.seg.accessed = True;
    cpu_state.data.cs.seg.executable = True;
    cpu_state.data.cs.seg.exec.conform = False;
    cpu_state.data.cs.seg.exec.defsize = False;
    cpu_state.data.cs.seg.exec.readable = True;
    cpu_state.data.cs.selector = 0xf000;
    cpu_state.data.cs.sregtype = SREG_CODE;
    cpu_state.data.cs.flagValid = True;

    cpu_state.data.ss.base = Zero32;
    cpu_state.data.ss.dpl = Zero4;
    cpu_state.data.ss.limit = Max16;
    cpu_state.data.ss.seg.accessed = True;
    cpu_state.data.ss.seg.executable = False;
    cpu_state.data.ss.seg.data.big = False;
    cpu_state.data.ss.seg.data.expdown = False;
    cpu_state.data.ss.seg.data.writable = True;
    cpu_state.data.ss.selector = Zero16;
    cpu_state.data.ss.sregtype = SREG_STACK;
    cpu_state.data.ss.flagValid = True;

    cpu_state.data.ds.base = Zero32;
    cpu_state.data.ds.dpl = Zero4;
    cpu_state.data.ds.limit = Max16;
    cpu_state.data.ds.seg.accessed = True;
    cpu_state.data.ss.seg.executable = False;
    cpu_state.data.ds.seg.data.big = False;
    cpu_state.data.ds.seg.data.expdown = False;
    cpu_state.data.ds.seg.data.writable = True;
    cpu_state.data.ds.selector = Zero16;
    cpu_state.data.ds.sregtype = SREG_DATA;
    cpu_state.data.ds.flagValid = True;
    cpu_state.data.gs = cpu_state.data.fs = cpu_state.data.es = cpu_state.data.ds;

    cpu_state.data.ldtr.base = Zero32;
    cpu_state.data.ldtr.dpl = Zero4;
    cpu_state.data.ldtr.limit = Max16;
    cpu_state.data.ldtr.selector = Zero16;
    cpu_state.data.ldtr.sregtype = SREG_LDTR;
    cpu_state.data.ldtr.sys.type = VCPU_DESC_SYS_TYPE_LDT;
    cpu_state.data.ldtr.flagValid = True;

    cpu_state.data.tr.base = Zero32;
    cpu_state.data.tr.dpl = Zero4;
    cpu_state.data.tr.limit = Max16;
    cpu_state.data.tr.selector = Zero16;
    cpu_state.data.tr.sregtype = SREG_TR;
    cpu_state.data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_16_AVL;
    cpu_state.data.tr.flagValid = True;

    cpu_state.data.idtr.base = Zero32;
    cpu_state.data.idtr.limit = 0x03ff;
    cpu_state.data.idtr.sregtype = SREG_IDTR;
    cpu_state.data.idtr.flagValid = True;

    cpu_state.data.gdtr.base = Zero32;
    cpu_state.data.gdtr.limit = Max16;
    cpu_state.data.gdtr.sregtype = SREG_GDTR;
    cpu_state.data.gdtr.flagValid = True;

    core_machine_cpu_execution_reset(context);

}

void core_machine_cpu_execution_request_stop(
    core_machine_cpu_execution_context *context)
{
    if (context != NULL) context->stop_requested = True;
}
 t_bool core_machine_cpu_execution_consume_stop_request(
    core_machine_cpu_execution_context *context)
{
    t_bool requested = context != NULL && context->stop_requested;
    if (context != NULL) context->stop_requested = False;
    return requested;
}
void core_machine_cpu_execution_request_reset(
    core_machine_cpu_execution_context *context)
{
    if (context != NULL) context->reset_requested = True;
}
t_bool core_machine_cpu_execution_consume_reset_request(
    core_machine_cpu_execution_context *context)
{
    t_bool requested = context != NULL && context->reset_requested;
    if (context != NULL) context->reset_requested = False;
    return requested;
}

int core_machine_cpu_read_linear(core_machine_cpu_execution_context *context, uint32_t linear, void *out_data, uint8_t size)
{
    return core_machine_cpu_execution_read_linear(context, linear,
        (t_vaddrcc)out_data, size);
}

int core_machine_cpu_write_linear(core_machine_cpu_execution_context *context,
    uint32_t linear, const void *in_data, uint8_t size)
{
    return core_machine_cpu_execution_write_linear(context, linear,
        (t_vaddrcc)in_data, size);
}

int core_machine_cpu_load_segment(core_machine_cpu_execution_context *context,
    core_machine_cpu_segment segment, uint16_t selector)
{
    switch (segment) {
    case CORE_MACHINE_CPU_SEGMENT_ES:
        return core_machine_cpu_execution_load_segment(context,
            &cpu_state.data.es, selector);
    case CORE_MACHINE_CPU_SEGMENT_CS:
        return core_machine_cpu_execution_load_segment(context,
            &cpu_state.data.cs, selector);
    case CORE_MACHINE_CPU_SEGMENT_SS:
        return core_machine_cpu_execution_load_segment(context,
            &cpu_state.data.ss, selector);
    case CORE_MACHINE_CPU_SEGMENT_DS:
        return core_machine_cpu_execution_load_segment(context,
            &cpu_state.data.ds, selector);
    case CORE_MACHINE_CPU_SEGMENT_FS:
        return core_machine_cpu_execution_load_segment(context,
            &cpu_state.data.fs, selector);
    case CORE_MACHINE_CPU_SEGMENT_GS:
        return core_machine_cpu_execution_load_segment(context,
            &cpu_state.data.gs, selector);
    }
    return 1;
}

int core_machine_cpu_get_code_default_size(const core_machine_cpu_execution_context *context)
{
    return cpu_state.data.cs.seg.exec.defsize;
}

uint32_t core_machine_cpu_get_code_base(const core_machine_cpu_execution_context *context)
{
    return cpu_state.data.cs.base;
}

void core_machine_cpu_set_watchpoint(core_machine_cpu_execution_context *context,
    core_machine_cpu_watchpoint kind, uint32_t linear)
{
    switch (kind) {
    case CORE_MACHINE_CPU_WATCH_READ:
        instruction_state.data.wrLinear = linear;
        instruction_state.data.flagWR = True;
        break;
    case CORE_MACHINE_CPU_WATCH_WRITE:
        instruction_state.data.wwLinear = linear;
        instruction_state.data.flagWW = True;
        break;
    case CORE_MACHINE_CPU_WATCH_EXECUTE:
        instruction_state.data.weLinear = linear;
        instruction_state.data.flagWE = True;
        break;
    }
}

void core_machine_cpu_clear_watchpoint(core_machine_cpu_execution_context *context,
    core_machine_cpu_watchpoint kind)
{
    switch (kind) {
    case CORE_MACHINE_CPU_WATCH_READ:
        instruction_state.data.flagWR = False;
        break;
    case CORE_MACHINE_CPU_WATCH_WRITE:
        instruction_state.data.flagWW = False;
        break;
    case CORE_MACHINE_CPU_WATCH_EXECUTE:
        instruction_state.data.flagWE = False;
        break;
    }
}

/* Prints user segment registers (ES, CS, SS, DS, FS, GS) */
static void print_sreg_seg(t_cpu_data_sreg *rsreg, const t_strptr label) {
    PRINTF("%s=%04X, Base=%08X, Limit=%08X, DPL=%01X, %s, ", label,
           rsreg->selector, rsreg->base, rsreg->limit,
           rsreg->dpl, rsreg->seg.accessed ? "A" : "a");
    if (rsreg->seg.executable) {
        PRINTF("Code, %s, %s, %s\n",
               rsreg->seg.exec.conform ? "C" : "c",
               rsreg->seg.exec.readable ? "Rw" : "rw",
               rsreg->seg.exec.defsize ? "32" : "16");
    } else {
        PRINTF("Data, %s, %s, %s\n",
               rsreg->seg.data.expdown ? "E" : "e",
               rsreg->seg.data.writable ? "RW" : "Rw",
               rsreg->seg.data.big ? "BIG" : "big");
    }
}
/* Prints system segment registers (TR, LDTR) */
static void print_sreg_sys(t_cpu_data_sreg *rsreg, const t_strptr label) {
    PRINTF("%s=%04X, Base=%08X, Limit=%08X, DPL=%01X, Type=%04X\n", label,
           rsreg->selector, rsreg->base, rsreg->limit,
           rsreg->dpl, rsreg->sys.type);
}
/* Prints segment registers */
void core_machine_cpu_print_segment_registers(const core_machine_cpu_execution_context *context) {
    print_sreg_seg(&cpu_state.data.es, "ES");
    print_sreg_seg(&cpu_state.data.cs, "CS");
    print_sreg_seg(&cpu_state.data.ss, "SS");
    print_sreg_seg(&cpu_state.data.ds, "DS");
    print_sreg_seg(&cpu_state.data.fs, "FS");
    print_sreg_seg(&cpu_state.data.gs, "GS");
    print_sreg_sys(&cpu_state.data.tr, "TR  ");
    print_sreg_sys(&cpu_state.data.ldtr, "LDTR");
    PRINTF("GDTR Base=%08X, Limit=%04X\n",
           cpu_state.data.gdtr.base, cpu_state.data.gdtr.limit);
    PRINTF("IDTR Base=%08X, Limit=%04X\n",
           cpu_state.data.idtr.base, cpu_state.data.idtr.limit);
}
/* Prints control registers */
void core_machine_cpu_print_control_registers(const core_machine_cpu_execution_context *context) {
    PRINTF("CR0=%08X: %s %s %s %s %s %s\n", cpu_state.data.cr0,
           _GetCR0_PG ? "PG" : "pg",
           _GetCR0_ET ? "ET" : "et",
           _GetCR0_TS ? "TS" : "ts",
           _GetCR0_EM ? "EM" : "em",
           _GetCR0_MP ? "MP" : "mp",
           _GetCR0_PE ? "PE" : "pe");
    PRINTF("CR2=PFLR=%08X\n", cpu_state.data.cr2);
    PRINTF("CR3=PDBR=%08X\n", cpu_state.data.cr3);
}
/* Prints regular registers */
void core_machine_cpu_print_registers(const core_machine_cpu_execution_context *context) {
    PRINTF( "EAX=%08X", cpu_state.data.eax);
    PRINTF(" EBX=%08X", cpu_state.data.ebx);
    PRINTF(" ECX=%08X", cpu_state.data.ecx);
    PRINTF(" EDX=%08X", cpu_state.data.edx);
    PRINTF("\nESP=%08X",cpu_state.data.esp);
    PRINTF(" EBP=%08X", cpu_state.data.ebp);
    PRINTF(" ESI=%08X", cpu_state.data.esi);
    PRINTF(" EDI=%08X", cpu_state.data.edi);
    PRINTF("\nEIP=%08X",cpu_state.data.eip);
    PRINTF(" EFL=%08X", cpu_state.data.eflags);
    PRINTF(": ");
    PRINTF("%s ", _GetEFLAGS_VM ? "VM" : "vm");
    PRINTF("%s ", _GetEFLAGS_RF ? "RF" : "rf");
    PRINTF("%s ", _GetEFLAGS_NT ? "NT" : "nt");
    PRINTF("IOPL=%01X ", _GetEFLAGS_IOPL);
    PRINTF("%s ", _GetEFLAGS_OF ? "OF" : "of");
    PRINTF("%s ", _GetEFLAGS_DF ? "DF" : "df");
    PRINTF("%s ", _GetEFLAGS_IF ? "IF" : "if");
    PRINTF("%s ", _GetEFLAGS_TF ? "TF" : "tf");
    PRINTF("%s ", _GetEFLAGS_SF ? "SF" : "sf");
    PRINTF("%s ", _GetEFLAGS_ZF ? "ZF" : "zf");
    PRINTF("%s ", _GetEFLAGS_AF ? "AF" : "af");
    PRINTF("%s ", _GetEFLAGS_PF ? "PF" : "pf");
    PRINTF("%s ", _GetEFLAGS_CF ? "CF" : "cf");
    PRINTF("\n");
}
/* Prints active memory info */
void core_machine_cpu_print_memory_accesses(const core_machine_cpu_execution_context *context) {
    t_nubitcc i;
    for (i = 0; i < instruction_state.data.msize; ++i) {
        PRINTF("%s: Lin=%08x, Data=%08x, Bytes=%1x\n",
               instruction_state.data.mem[i].flagWrite ? "Write" : "Read",
               instruction_state.data.mem[i].linear, instruction_state.data.mem[i].data, instruction_state.data.mem[i].byte);
    }
}
void core_machine_cpu_print_watchpoints(const core_machine_cpu_execution_context *context) {
    if (instruction_state.data.flagWR) {
        PRINTF("Watch-read point: Lin=%08x\n", instruction_state.data.wrLinear);
    }
    if (instruction_state.data.flagWW) {
        PRINTF("Watch-write point: Lin=%08x\n", instruction_state.data.wwLinear);
    }
    if (instruction_state.data.flagWE) {
        PRINTF("Watch-exec point: Lin=%08x\n", instruction_state.data.weLinear);
    }
}

