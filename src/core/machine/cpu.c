/* Copyright 2012-2014 Neko. */

/* VCPU defines the Central Processing Unit. */

#include "type.h"

#include "core/machine/cpu_instructions.h"

#include "core/machine/cpu.h"

static t_cpu *coreMachineCpu;

void core_machine_cpu_execution_context_initialize(
    core_machine_cpu_execution_context *context, t_cpu *cpu,
    t_cpuins *instructions)
{
    if (context == NULL) return;
    context->cpu = cpu;
    context->instructions = instructions;
    context->stop_requested = False;
    context->reset_requested = False;
}

t_cpu *core_machine_cpu_current(void)
{
    return coreMachineCpu;
}

void core_machine_cpu_bind_live(t_cpu *cpu)
{
    coreMachineCpu = cpu;
}

void core_machine_cpu_unbind_live(void)
{
    coreMachineCpu = NULL;
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

#define cpu_state (*context->cpu)

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

#undef cpu_state
}

void vcpuInit() {
    core_machine_cpu_state_initialize(
        core_machine_cpu_execution_current_legacy());
}
void vcpuReset() {
    core_machine_cpu_state_reset(core_machine_cpu_execution_current_legacy());
}
void vcpuRefresh() {
    core_machine_cpu_execution_refresh(
        core_machine_cpu_execution_current_legacy());
}
void vcpuFinal() {
    core_machine_cpu_execution_finalize(
        core_machine_cpu_execution_current_legacy());
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

void vcpuRequestStop() {
    core_machine_cpu_execution_request_stop(
        core_machine_cpu_execution_current_legacy());
}
t_bool vcpuConsumeStopRequest() {
    return core_machine_cpu_execution_consume_stop_request(
        core_machine_cpu_execution_current_legacy());
}
void vcpuRequestReset() {
    core_machine_cpu_execution_request_reset(
        core_machine_cpu_execution_current_legacy());
}
t_bool vcpuConsumeResetRequest() {
    return core_machine_cpu_execution_consume_reset_request(
        core_machine_cpu_execution_current_legacy());
}

int core_machine_cpu_read_linear(uint32_t linear, void *out_data, uint8_t size)
{
    return vcpuinsReadLinear(linear, (t_vaddrcc)out_data, size);
}

int core_machine_cpu_write_linear(uint32_t linear, const void *in_data,
    uint8_t size)
{
    return vcpuinsWriteLinear(linear, (t_vaddrcc)in_data, size);
}

int core_machine_cpu_load_segment(core_machine_cpu_segment segment,
    uint16_t selector)
{
    switch (segment) {
    case CORE_MACHINE_CPU_SEGMENT_ES:
        return vcpuinsLoadSreg(&vcpu.data.es, selector);
    case CORE_MACHINE_CPU_SEGMENT_CS:
        return vcpuinsLoadSreg(&vcpu.data.cs, selector);
    case CORE_MACHINE_CPU_SEGMENT_SS:
        return vcpuinsLoadSreg(&vcpu.data.ss, selector);
    case CORE_MACHINE_CPU_SEGMENT_DS:
        return vcpuinsLoadSreg(&vcpu.data.ds, selector);
    case CORE_MACHINE_CPU_SEGMENT_FS:
        return vcpuinsLoadSreg(&vcpu.data.fs, selector);
    case CORE_MACHINE_CPU_SEGMENT_GS:
        return vcpuinsLoadSreg(&vcpu.data.gs, selector);
    }
    return 1;
}

int core_machine_cpu_get_code_default_size(void)
{
    return vcpu.data.cs.seg.exec.defsize;
}

uint32_t core_machine_cpu_get_code_base(void)
{
    return vcpu.data.cs.base;
}

void core_machine_cpu_set_watchpoint(core_machine_cpu_watchpoint kind,
    uint32_t linear)
{
    switch (kind) {
    case CORE_MACHINE_CPU_WATCH_READ:
        vcpuins.data.wrLinear = linear;
        vcpuins.data.flagWR = True;
        break;
    case CORE_MACHINE_CPU_WATCH_WRITE:
        vcpuins.data.wwLinear = linear;
        vcpuins.data.flagWW = True;
        break;
    case CORE_MACHINE_CPU_WATCH_EXECUTE:
        vcpuins.data.weLinear = linear;
        vcpuins.data.flagWE = True;
        break;
    }
}

void core_machine_cpu_clear_watchpoint(core_machine_cpu_watchpoint kind)
{
    switch (kind) {
    case CORE_MACHINE_CPU_WATCH_READ:
        vcpuins.data.flagWR = False;
        break;
    case CORE_MACHINE_CPU_WATCH_WRITE:
        vcpuins.data.flagWW = False;
        break;
    case CORE_MACHINE_CPU_WATCH_EXECUTE:
        vcpuins.data.flagWE = False;
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
void devicePrintCpuSreg() {
    print_sreg_seg(&vcpu.data.es, "ES");
    print_sreg_seg(&vcpu.data.cs, "CS");
    print_sreg_seg(&vcpu.data.ss, "SS");
    print_sreg_seg(&vcpu.data.ds, "DS");
    print_sreg_seg(&vcpu.data.fs, "FS");
    print_sreg_seg(&vcpu.data.gs, "GS");
    print_sreg_sys(&vcpu.data.tr, "TR  ");
    print_sreg_sys(&vcpu.data.ldtr, "LDTR");
    PRINTF("GDTR Base=%08X, Limit=%04X\n",
           vcpu.data.gdtr.base, vcpu.data.gdtr.limit);
    PRINTF("IDTR Base=%08X, Limit=%04X\n",
           vcpu.data.idtr.base, vcpu.data.idtr.limit);
}
/* Prints control registers */
void devicePrintCpuCreg() {
    PRINTF("CR0=%08X: %s %s %s %s %s %s\n", vcpu.data.cr0,
           _GetCR0_PG ? "PG" : "pg",
           _GetCR0_ET ? "ET" : "et",
           _GetCR0_TS ? "TS" : "ts",
           _GetCR0_EM ? "EM" : "em",
           _GetCR0_MP ? "MP" : "mp",
           _GetCR0_PE ? "PE" : "pe");
    PRINTF("CR2=PFLR=%08X\n", vcpu.data.cr2);
    PRINTF("CR3=PDBR=%08X\n", vcpu.data.cr3);
}
/* Prints regular registers */
void devicePrintCpuReg() {
    PRINTF( "EAX=%08X", vcpu.data.eax);
    PRINTF(" EBX=%08X", vcpu.data.ebx);
    PRINTF(" ECX=%08X", vcpu.data.ecx);
    PRINTF(" EDX=%08X", vcpu.data.edx);
    PRINTF("\nESP=%08X",vcpu.data.esp);
    PRINTF(" EBP=%08X", vcpu.data.ebp);
    PRINTF(" ESI=%08X", vcpu.data.esi);
    PRINTF(" EDI=%08X", vcpu.data.edi);
    PRINTF("\nEIP=%08X",vcpu.data.eip);
    PRINTF(" EFL=%08X", vcpu.data.eflags);
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
void devicePrintCpuMem() {
    t_nubitcc i;
    for (i = 0; i < vcpuins.data.msize; ++i) {
        PRINTF("%s: Lin=%08x, Data=%08x, Bytes=%1x\n",
               vcpuins.data.mem[i].flagWrite ? "Write" : "Read",
               vcpuins.data.mem[i].linear, vcpuins.data.mem[i].data, vcpuins.data.mem[i].byte);
    }
}
void devicePrintCpuWatch() {
    if (vcpuins.data.flagWR) {
        PRINTF("Watch-read point: Lin=%08x\n", vcpuins.data.wrLinear);
    }
    if (vcpuins.data.flagWW) {
        PRINTF("Watch-write point: Lin=%08x\n", vcpuins.data.wwLinear);
    }
    if (vcpuins.data.flagWE) {
        PRINTF("Watch-exec point: Lin=%08x\n", vcpuins.data.weLinear);
    }
}

void core_machine_cpu_print_registers(void)
{
    devicePrintCpuReg();
}

void core_machine_cpu_print_segment_registers(void)
{
    devicePrintCpuSreg();
}

void core_machine_cpu_print_control_registers(void)
{
    devicePrintCpuCreg();
}

void core_machine_cpu_print_memory_accesses(void)
{
    devicePrintCpuMem();
}

void core_machine_cpu_print_watchpoints(void)
{
    devicePrintCpuWatch();
}
