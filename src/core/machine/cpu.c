/* Copyright 2012-2014 Neko. */

/* VCPU defines the Central Processing Unit. */

#include "type.h"

#include "core/machine/cpu_instructions.h"

#include "core/machine/cpu.h"

#define cpu_state (*context->cpu)
#define instruction_state (*context->instructions)

C_VOID core_machine_cpu_execution_context_initialize(
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
        context->trace = (ntvdm64_type_trace *)STD_MALLOC(sizeof(*context->trace));
    }
    if (context->trace != NULL) {
        STD_MEMSET((C_VOID *)context->trace, NTVDM64_TYPE_ZERO_8, sizeof(*context->trace));
    }
    context->extension_context = NULL;
    context->stop_requested = NTVDM64_TYPE_FALSE;
    context->reset_requested = NTVDM64_TYPE_FALSE;
}

C_VOID core_machine_cpu_execution_context_bind_pic(
    core_machine_cpu_execution_context *context, t_pic *master,
    t_pic *slave)
{
    if (context == NULL) return;
    context->pic_master = master;
    context->pic_slave = slave;
}

C_VOID core_machine_cpu_execution_context_bind_extension(
    core_machine_cpu_execution_context *context, C_VOID *extension_context)
{
    if (context != NULL) context->extension_context = extension_context;
}

C_VOID *core_machine_cpu_execution_context_extension(
    const core_machine_cpu_execution_context *context)
{
    return context == NULL ? NULL : context->extension_context;
}

C_VOID core_machine_cpu_state_initialize(
    core_machine_cpu_execution_context *context) {
    if (context == NULL || context->cpu == NULL ||
        context->instructions == NULL) return;
    if (context != NULL) {
        context->stop_requested = NTVDM64_TYPE_FALSE;
        context->reset_requested = NTVDM64_TYPE_FALSE;
    }
    core_machine_cpu_execution_initialize(context);
}
C_VOID core_machine_cpu_state_reset(core_machine_cpu_execution_context *context) {
    if (context == NULL || context->cpu == NULL ||
        context->instructions == NULL) return;
    STD_MEMSET((C_VOID *)context->cpu, NTVDM64_TYPE_ZERO_8, sizeof(t_cpu));
    if (context != NULL) {
        context->stop_requested = NTVDM64_TYPE_FALSE;
        context->reset_requested = NTVDM64_TYPE_FALSE;
    }

    cpu_state.data.eip = 0x0000fff0;
    cpu_state.data.eflags = 0x00000002;

    cpu_state.data.cs.base = 0xffff0000;
    cpu_state.data.cs.dpl = NTVDM64_TYPE_ZERO_4;
    cpu_state.data.cs.limit = NTVDM64_TYPE_MAX_UNSIGNED_32;
    cpu_state.data.cs.seg.accessed = NTVDM64_TYPE_TRUE;
    cpu_state.data.cs.seg.executable = NTVDM64_TYPE_TRUE;
    cpu_state.data.cs.seg.exec.conform = NTVDM64_TYPE_FALSE;
    cpu_state.data.cs.seg.exec.defsize = NTVDM64_TYPE_FALSE;
    cpu_state.data.cs.seg.exec.readable = NTVDM64_TYPE_TRUE;
    cpu_state.data.cs.selector = 0xf000;
    cpu_state.data.cs.sregtype = SREG_CODE;
    cpu_state.data.cs.flagValid = NTVDM64_TYPE_TRUE;

    cpu_state.data.ss.base = NTVDM64_TYPE_ZERO_32;
    cpu_state.data.ss.dpl = NTVDM64_TYPE_ZERO_4;
    cpu_state.data.ss.limit = NTVDM64_TYPE_MAX_UNSIGNED_16;
    cpu_state.data.ss.seg.accessed = NTVDM64_TYPE_TRUE;
    cpu_state.data.ss.seg.executable = NTVDM64_TYPE_FALSE;
    cpu_state.data.ss.seg.data.big = NTVDM64_TYPE_FALSE;
    cpu_state.data.ss.seg.data.expdown = NTVDM64_TYPE_FALSE;
    cpu_state.data.ss.seg.data.writable = NTVDM64_TYPE_TRUE;
    cpu_state.data.ss.selector = NTVDM64_TYPE_ZERO_16;
    cpu_state.data.ss.sregtype = SREG_STACK;
    cpu_state.data.ss.flagValid = NTVDM64_TYPE_TRUE;

    cpu_state.data.ds.base = NTVDM64_TYPE_ZERO_32;
    cpu_state.data.ds.dpl = NTVDM64_TYPE_ZERO_4;
    cpu_state.data.ds.limit = NTVDM64_TYPE_MAX_UNSIGNED_16;
    cpu_state.data.ds.seg.accessed = NTVDM64_TYPE_TRUE;
    cpu_state.data.ss.seg.executable = NTVDM64_TYPE_FALSE;
    cpu_state.data.ds.seg.data.big = NTVDM64_TYPE_FALSE;
    cpu_state.data.ds.seg.data.expdown = NTVDM64_TYPE_FALSE;
    cpu_state.data.ds.seg.data.writable = NTVDM64_TYPE_TRUE;
    cpu_state.data.ds.selector = NTVDM64_TYPE_ZERO_16;
    cpu_state.data.ds.sregtype = SREG_DATA;
    cpu_state.data.ds.flagValid = NTVDM64_TYPE_TRUE;
    cpu_state.data.gs = cpu_state.data.fs = cpu_state.data.es = cpu_state.data.ds;

    cpu_state.data.ldtr.base = NTVDM64_TYPE_ZERO_32;
    cpu_state.data.ldtr.dpl = NTVDM64_TYPE_ZERO_4;
    cpu_state.data.ldtr.limit = NTVDM64_TYPE_MAX_UNSIGNED_16;
    cpu_state.data.ldtr.selector = NTVDM64_TYPE_ZERO_16;
    cpu_state.data.ldtr.sregtype = SREG_LDTR;
    cpu_state.data.ldtr.sys.type = VCPU_DESC_SYS_TYPE_LDT;
    cpu_state.data.ldtr.flagValid = NTVDM64_TYPE_TRUE;

    cpu_state.data.tr.base = NTVDM64_TYPE_ZERO_32;
    cpu_state.data.tr.dpl = NTVDM64_TYPE_ZERO_4;
    cpu_state.data.tr.limit = NTVDM64_TYPE_MAX_UNSIGNED_16;
    cpu_state.data.tr.selector = NTVDM64_TYPE_ZERO_16;
    cpu_state.data.tr.sregtype = SREG_TR;
    cpu_state.data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_16_AVL;
    cpu_state.data.tr.flagValid = NTVDM64_TYPE_TRUE;

    cpu_state.data.idtr.base = NTVDM64_TYPE_ZERO_32;
    cpu_state.data.idtr.limit = 0x03ff;
    cpu_state.data.idtr.sregtype = SREG_IDTR;
    cpu_state.data.idtr.flagValid = NTVDM64_TYPE_TRUE;

    cpu_state.data.gdtr.base = NTVDM64_TYPE_ZERO_32;
    cpu_state.data.gdtr.limit = NTVDM64_TYPE_MAX_UNSIGNED_16;
    cpu_state.data.gdtr.sregtype = SREG_GDTR;
    cpu_state.data.gdtr.flagValid = NTVDM64_TYPE_TRUE;

    core_machine_cpu_execution_reset(context);

}

C_VOID core_machine_cpu_execution_request_stop(
    core_machine_cpu_execution_context *context)
{
    if (context != NULL) context->stop_requested = NTVDM64_TYPE_TRUE;
}
 ntvdm64_type_bool core_machine_cpu_execution_consume_stop_request(
    core_machine_cpu_execution_context *context)
{
    ntvdm64_type_bool requested = context != NULL && context->stop_requested;
    if (context != NULL) context->stop_requested = NTVDM64_TYPE_FALSE;
    return requested;
}
C_VOID core_machine_cpu_execution_request_reset(
    core_machine_cpu_execution_context *context)
{
    if (context != NULL) context->reset_requested = NTVDM64_TYPE_TRUE;
}
ntvdm64_type_bool core_machine_cpu_execution_consume_reset_request(
    core_machine_cpu_execution_context *context)
{
    ntvdm64_type_bool requested = context != NULL && context->reset_requested;
    if (context != NULL) context->reset_requested = NTVDM64_TYPE_FALSE;
    return requested;
}

C_INT core_machine_cpu_read_linear(core_machine_cpu_execution_context *context, uint32_t linear, C_VOID *out_data, uint8_t size)
{
    return core_machine_cpu_execution_read_linear(context, linear,
        (ntvdm64_type_virtual_address)out_data, size);
}

C_INT core_machine_cpu_write_linear(core_machine_cpu_execution_context *context,
    uint32_t linear, const C_VOID *in_data, uint8_t size)
{
    return core_machine_cpu_execution_write_linear(context, linear,
        (ntvdm64_type_virtual_address)in_data, size);
}

C_INT core_machine_cpu_load_segment(core_machine_cpu_execution_context *context,
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

C_INT core_machine_cpu_get_code_default_size(const core_machine_cpu_execution_context *context)
{
    return cpu_state.data.cs.seg.exec.defsize;
}

uint32_t core_machine_cpu_get_code_base(const core_machine_cpu_execution_context *context)
{
    return cpu_state.data.cs.base;
}

C_VOID core_machine_cpu_set_watchpoint(core_machine_cpu_execution_context *context,
    core_machine_cpu_watchpoint kind, uint32_t linear)
{
    switch (kind) {
    case CORE_MACHINE_CPU_WATCH_READ:
        instruction_state.data.wrLinear = linear;
        instruction_state.data.flagWR = NTVDM64_TYPE_TRUE;
        break;
    case CORE_MACHINE_CPU_WATCH_WRITE:
        instruction_state.data.wwLinear = linear;
        instruction_state.data.flagWW = NTVDM64_TYPE_TRUE;
        break;
    case CORE_MACHINE_CPU_WATCH_EXECUTE:
        instruction_state.data.weLinear = linear;
        instruction_state.data.flagWE = NTVDM64_TYPE_TRUE;
        break;
    }
}

C_VOID core_machine_cpu_clear_watchpoint(core_machine_cpu_execution_context *context,
    core_machine_cpu_watchpoint kind)
{
    switch (kind) {
    case CORE_MACHINE_CPU_WATCH_READ:
        instruction_state.data.flagWR = NTVDM64_TYPE_FALSE;
        break;
    case CORE_MACHINE_CPU_WATCH_WRITE:
        instruction_state.data.flagWW = NTVDM64_TYPE_FALSE;
        break;
    case CORE_MACHINE_CPU_WATCH_EXECUTE:
        instruction_state.data.flagWE = NTVDM64_TYPE_FALSE;
        break;
    }
}

/* Prints user segment registers (ES, CS, SS, DS, FS, GS) */
static C_VOID print_sreg_seg(t_cpu_data_sreg *rsreg, const ntvdm64_type_string_pointer label) {
    STD_PRINTF("%s=%04X, Base=%08X, Limit=%08X, DPL=%01X, %s, ", label,
           rsreg->selector, rsreg->base, rsreg->limit,
           rsreg->dpl, rsreg->seg.accessed ? "A" : "a");
    if (rsreg->seg.executable) {
        STD_PRINTF("Code, %s, %s, %s\n",
               rsreg->seg.exec.conform ? "C" : "c",
               rsreg->seg.exec.readable ? "Rw" : "rw",
               rsreg->seg.exec.defsize ? "32" : "16");
    } else {
        STD_PRINTF("Data, %s, %s, %s\n",
               rsreg->seg.data.expdown ? "E" : "e",
               rsreg->seg.data.writable ? "RW" : "Rw",
               rsreg->seg.data.big ? "BIG" : "big");
    }
}
/* Prints system segment registers (TR, LDTR) */
static C_VOID print_sreg_sys(t_cpu_data_sreg *rsreg, const ntvdm64_type_string_pointer label) {
    STD_PRINTF("%s=%04X, Base=%08X, Limit=%08X, DPL=%01X, Type=%04X\n", label,
           rsreg->selector, rsreg->base, rsreg->limit,
           rsreg->dpl, rsreg->sys.type);
}
/* Prints segment registers */
C_VOID core_machine_cpu_print_segment_registers(const core_machine_cpu_execution_context *context) {
    print_sreg_seg(&cpu_state.data.es, "ES");
    print_sreg_seg(&cpu_state.data.cs, "CS");
    print_sreg_seg(&cpu_state.data.ss, "SS");
    print_sreg_seg(&cpu_state.data.ds, "DS");
    print_sreg_seg(&cpu_state.data.fs, "FS");
    print_sreg_seg(&cpu_state.data.gs, "GS");
    print_sreg_sys(&cpu_state.data.tr, "TR  ");
    print_sreg_sys(&cpu_state.data.ldtr, "LDTR");
    STD_PRINTF("GDTR Base=%08X, Limit=%04X\n",
           cpu_state.data.gdtr.base, cpu_state.data.gdtr.limit);
    STD_PRINTF("IDTR Base=%08X, Limit=%04X\n",
           cpu_state.data.idtr.base, cpu_state.data.idtr.limit);
}
/* Prints control registers */
C_VOID core_machine_cpu_print_control_registers(const core_machine_cpu_execution_context *context) {
    STD_PRINTF("CR0=%08X: %s %s %s %s %s %s\n", cpu_state.data.cr0,
           _GetCR0_PG ? "PG" : "pg",
           _GetCR0_ET ? "ET" : "et",
           _GetCR0_TS ? "TS" : "ts",
           _GetCR0_EM ? "EM" : "em",
           _GetCR0_MP ? "MP" : "mp",
           _GetCR0_PE ? "PE" : "pe");
    STD_PRINTF("CR2=PFLR=%08X\n", cpu_state.data.cr2);
    STD_PRINTF("CR3=PDBR=%08X\n", cpu_state.data.cr3);
}
/* Prints regular registers */
C_VOID core_machine_cpu_print_registers(const core_machine_cpu_execution_context *context) {
    STD_PRINTF( "EAX=%08X", cpu_state.data.eax);
    STD_PRINTF(" EBX=%08X", cpu_state.data.ebx);
    STD_PRINTF(" ECX=%08X", cpu_state.data.ecx);
    STD_PRINTF(" EDX=%08X", cpu_state.data.edx);
    STD_PRINTF("\nESP=%08X",cpu_state.data.esp);
    STD_PRINTF(" EBP=%08X", cpu_state.data.ebp);
    STD_PRINTF(" ESI=%08X", cpu_state.data.esi);
    STD_PRINTF(" EDI=%08X", cpu_state.data.edi);
    STD_PRINTF("\nEIP=%08X",cpu_state.data.eip);
    STD_PRINTF(" EFL=%08X", cpu_state.data.eflags);
    STD_PRINTF(": ");
    STD_PRINTF("%s ", _GetEFLAGS_VM ? "VM" : "vm");
    STD_PRINTF("%s ", _GetEFLAGS_RF ? "RF" : "rf");
    STD_PRINTF("%s ", _GetEFLAGS_NT ? "NT" : "nt");
    STD_PRINTF("IOPL=%01X ", _GetEFLAGS_IOPL);
    STD_PRINTF("%s ", _GetEFLAGS_OF ? "OF" : "of");
    STD_PRINTF("%s ", _GetEFLAGS_DF ? "DF" : "df");
    STD_PRINTF("%s ", _GetEFLAGS_IF ? "IF" : "if");
    STD_PRINTF("%s ", _GetEFLAGS_TF ? "TF" : "tf");
    STD_PRINTF("%s ", _GetEFLAGS_SF ? "SF" : "sf");
    STD_PRINTF("%s ", _GetEFLAGS_ZF ? "ZF" : "zf");
    STD_PRINTF("%s ", _GetEFLAGS_AF ? "AF" : "af");
    STD_PRINTF("%s ", _GetEFLAGS_PF ? "PF" : "pf");
    STD_PRINTF("%s ", _GetEFLAGS_CF ? "CF" : "cf");
    STD_PRINTF("\n");
}
/* Prints active memory info */
C_VOID core_machine_cpu_print_memory_accesses(const core_machine_cpu_execution_context *context) {
    ntvdm64_type_native_unsigned i;
    for (i = 0; i < instruction_state.data.msize; ++i) {
        STD_PRINTF("%s: Lin=%08x, Data=%08x, Bytes=%1x\n",
               instruction_state.data.mem[i].flagWrite ? "Write" : "Read",
               instruction_state.data.mem[i].linear, instruction_state.data.mem[i].data, instruction_state.data.mem[i].byte);
    }
}
C_VOID core_machine_cpu_print_watchpoints(const core_machine_cpu_execution_context *context) {
    if (instruction_state.data.flagWR) {
        STD_PRINTF("Watch-read point: Lin=%08x\n", instruction_state.data.wrLinear);
    }
    if (instruction_state.data.flagWW) {
        STD_PRINTF("Watch-write point: Lin=%08x\n", instruction_state.data.wwLinear);
    }
    if (instruction_state.data.flagWE) {
        STD_PRINTF("Watch-exec point: Lin=%08x\n", instruction_state.data.weLinear);
    }
}

