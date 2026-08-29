#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct shift_machine { core_machine *machine; } shift_machine;

static C_VOID shift_reset(C_VOID *opaque)
{
    shift_machine *state=(shift_machine *)opaque;
    if(state!=STD_NULL)(C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider shift_provider={shift_reset,STD_NULL};

static C_INT shift_prepare(core_machine_cpu_profile profile,shift_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    if(state==STD_NULL)return 0;
    STD_MEMSET(state,0,sizeof(*state));
    if(core_machine_create(&config,&state->machine)!=TYPE_STATUS_OK||
        !test_core_machine_fixture_bind_freeze_reset(state->machine,
            &shift_provider,state)) {
        core_machine_destroy(state->machine);state->machine=STD_NULL;return 0;
    }
    return 1;
}

static C_INT shift_run_real(shift_machine *state,const type_unsigned_8 *code,STD_SIZE_T bytes,
    C_INT fault,t_cpu *out,core_machine_cpu_diagnostic *diagnostic)
{
    core_machine_run_result result;
    type_status status;
    if(state==STD_NULL||state->machine==STD_NULL||
        !test_core_machine_fixture_prepare_real_mode_execution(state->machine,0u)||
        core_machine_memory_write(state->machine,0u,code,bytes)!=TYPE_STATUS_OK)return 0;
    if(fault&&!test_core_machine_fixture_preflight_real_ud_terminal(state->machine))return 0;
    status=core_machine_run(state->machine,(core_machine_run_budget){1u,0u},&result);
    if(status!=(fault?TYPE_STATUS_FAULT:TYPE_STATUS_OK)||result.reason!=(fault?
        CORE_MACHINE_STOP_FAULT:CORE_MACHINE_STOP_BUDGET)||
        core_machine_get_cpu_diagnostic(state->machine,diagnostic)!=TYPE_STATUS_OK)return 0;
    *out=test_core_machine_fixture_capture_cpu_after_run(state->machine);return 1;
}

static type_unsigned_32 shift_parity(type_unsigned_32 value)
{
    type_unsigned_8 bits=0u;
    value&=0xffu;
    while(value){bits^=(type_unsigned_8)(value&1u);value>>=1u;}
    return bits?0u:VCPU_EFLAGS_PF;
}

static type_unsigned_32 shift_flags(type_unsigned_32 result,type_unsigned_32 destination,type_unsigned_8 count,
    type_unsigned_8 width,C_INT right)
{
    const type_unsigned_32 sign=width==16u?0x8000u:0x80000000u;
    const type_unsigned_32 mask=width==16u?0xffffu:0xffffffffu;
    type_unsigned_32 flags=shift_parity(result);
    type_unsigned_32 cf=right?(destination>>(count-1u)):
        (destination>>(width-count));
    if(cf&1u)flags|=VCPU_EFLAGS_CF;
    if((result&mask)==0u)flags|=VCPU_EFLAGS_ZF;
    if(result&sign)flags|=VCPU_EFLAGS_SF;
    if(count==1u&&((destination^result)&sign))flags|=VCPU_EFLAGS_OF;
    return flags;
}

static type_unsigned_32 shift_result(type_unsigned_32 destination,type_unsigned_32 source,type_unsigned_8 count,
    type_unsigned_8 width,C_INT right)
{
    const type_unsigned_32 mask=width==16u?0xffffu:0xffffffffu;
    destination&=mask;source&=mask;
    return right?((destination>>count)|(source<<(width-count)))&mask:
        ((destination<<count)|(source>>(width-count)))&mask;
}

static C_INT shift_test_forms(C_VOID)
{
    static const type_unsigned_8 immediate_opcodes[]={0xa4u,0xacu};
    static const type_unsigned_8 cl_opcodes[]={0xa5u,0xadu};
    const type_unsigned_32 initial_flags=VCPU_EFLAGS_AF|VCPU_EFLAGS_ZF;
    type_unsigned_8 direction,width,memory,count_index,cl;

    for(direction=0u;direction<2u;++direction)for(width=0u;width<2u;++width)
    for(memory=0u;memory<2u;++memory)for(cl=0u;cl<2u;++cl)
    for(count_index=1u;count_index<=(width?31u:16u);++count_index) {
        const type_unsigned_8 count=count_index;
        const type_unsigned_8 operand32=width;
        const type_unsigned_8 address32=memory&&width;
        const type_unsigned_8 opcode=cl?cl_opcodes[direction]:immediate_opcodes[direction];
        type_unsigned_8 code[8]={0};
        STD_SIZE_T bytes=0u;
        const type_unsigned_32 destination=width?0x81234567u:0xaabb8123u;
        const type_unsigned_32 source=cl?((width?0x76543200u:0xccdd7600u)|count):
            (width?0x76543210u:0xccdd7654u);
        const type_unsigned_32 expected=shift_result(destination,source,count,width?32u:16u,direction);
        const type_unsigned_32 expected_flags=shift_flags(expected,destination,count,width?32u:16u,direction);
        const type_unsigned_32 flag_mask=VCPU_EFLAGS_CF|VCPU_EFLAGS_PF|VCPU_EFLAGS_ZF|
            VCPU_EFLAGS_SF|(count==1u?VCPU_EFLAGS_OF:0u);
        shift_machine state;t_cpu after={0};core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 observed=0u;
        C_INT failed=!shift_prepare(CORE_MACHINE_CPU_PROFILE_80386,&state);

        if(address32)code[bytes++]=0x67u;
        if(operand32)code[bytes++]=0x66u;
        code[bytes++]=0x0fu;code[bytes++]=opcode;
        if(memory) {
            code[bytes++]=0x0eu;
            if(address32){}
            else {code[bytes++]=0x00u;code[bytes++]=0x40u;}
        } else code[bytes++]=0xc8u;
        if(!cl)code[bytes++]=count;
        if(!failed) {
            state.machine->executor_cpu.data.eax=destination;
            state.machine->executor_cpu.data.ecx=source;
            state.machine->executor_cpu.data.esi=0x4000u;
            state.machine->executor_cpu.data.eflags=initial_flags;
            failed|=memory&&core_machine_memory_write(state.machine,0x4000u,&destination,width?4u:2u)!=TYPE_STATUS_OK;
            failed|=!shift_run_real(&state,code,bytes,0,&after,&diagnostic)||diagnostic.first_fault.valid;
            if(memory)failed|=core_machine_memory_read(state.machine,0x4000u,&observed,width?4u:2u)!=TYPE_STATUS_OK;
            else observed=after.data.eax;
            failed|=(width?observed:(observed&0xffffu))!=expected||
                (after.data.eflags&flag_mask)!=(expected_flags&flag_mask)||after.data.eip!=bytes;
        }
        core_machine_destroy(state.machine);if(failed)return 0;
    }
    return 1;
}

static C_INT shift_test_count_zero(C_VOID)
{
    static const type_unsigned_8 immediate[]={0x66u,0x0fu,0xa4u,0xc8u,0u};
    static const type_unsigned_8 cl[]={0x0fu,0xadu,0xc8u};
    const type_unsigned_32 flags=VCPU_EFLAGS_CF|VCPU_EFLAGS_AF|VCPU_EFLAGS_ZF|VCPU_EFLAGS_OF;
    type_unsigned_8 index;
    for(index=0u;index<2u;++index) {
        shift_machine state;t_cpu after;core_machine_cpu_diagnostic diagnostic;
        C_INT failed=!shift_prepare(CORE_MACHINE_CPU_PROFILE_80386,&state);
        if(!failed) {
            state.machine->executor_cpu.data.eax=0xaabbccdd;
            state.machine->executor_cpu.data.ecx=0u;
            state.machine->executor_cpu.data.eflags=flags;
            failed|=!shift_run_real(&state,index?cl:immediate,index?sizeof(cl):sizeof(immediate),0,&after,&diagnostic)||
                after.data.eax!=0xaabbccddu||after.data.eflags!=flags;
        }
        core_machine_destroy(state.machine);if(failed)return 0;
    }
    return 1;
}

static C_INT shift_test_profile(C_VOID)
{
    static const type_unsigned_8 code[]={0x0fu,0xa4u,0xc8u,1u};
    core_machine_cpu_profile profiles[]={CORE_MACHINE_CPU_PROFILE_80186,CORE_MACHINE_CPU_PROFILE_80286};
    type_unsigned_8 index;
    for(index=0u;index<2u;++index) {
        shift_machine state;t_cpu after;core_machine_cpu_diagnostic diagnostic;
        C_INT failed=!shift_prepare(profiles[index],&state);
        if(!failed) {
            state.machine->executor_cpu.data.eax=0xaabbccdd;
            state.machine->executor_cpu.data.ecx=0x11223344;
            state.machine->executor_cpu.data.eflags=VCPU_EFLAGS_ZF;
            failed|=!shift_run_real(&state,code,sizeof(code),1,&after,&diagnostic)||
                !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,VCPUINS_EXCEPT_UD)||
                after.data.eax!=0xaabbccdd||after.data.eflags!=VCPU_EFLAGS_ZF||after.data.eip!=0u;
        }
        core_machine_destroy(state.machine);if(failed)return 0;
    }
    return 1;
}

static C_INT shift_prepare_protected(C_INT writable,C_INT out_of_limit,
    shift_machine *state)
{
    static const type_unsigned_8 pointer[]={0x1fu,0,0,0x03u,0,0};
    type_unsigned_8 gdt[]={
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const type_unsigned_8 bootstrap[]={
        0x0fu,0x01u,0x16u,0x00u,0x01u,0xb8u,0x01u,0x00u,
        0x0fu,0x01u,0xf0u,0xb8u,0x10u,0x00u,0x8eu,0xd8u,
        0x8eu,0xc0u,0xb8u,0x18u,0x00u,0x8eu,0xd0u,0xbcu,
        0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const type_unsigned_8 halt[]={0xf4u};
    core_machine_run_result result;

    gdt[16u]=out_of_limit?0x0fu:0xffu;gdt[17u]=out_of_limit?0u:0xffu;
    gdt[21u]=writable?0x92u:0x90u;
    return shift_prepare(CORE_MACHINE_CPU_PROFILE_80386,state)&&
        core_machine_memory_write(state->machine,0x0100u,pointer,sizeof(pointer))==TYPE_STATUS_OK&&
        core_machine_memory_write(state->machine,0x0300u,gdt,sizeof(gdt))==TYPE_STATUS_OK&&
        core_machine_memory_write(state->machine,0u,bootstrap,sizeof(bootstrap))==TYPE_STATUS_OK&&
        core_machine_memory_write(state->machine,0x2000u,halt,sizeof(halt))==TYPE_STATUS_OK&&
        core_machine_run(state->machine,(core_machine_run_budget){96u,0u},&result)==TYPE_STATUS_OK&&
        result.reason==CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT shift_test_access_failure(C_VOID)
{
    static const type_unsigned_8 shld[]={0x0fu,0xa4u,0x0eu,0x10u,0u,1u};
    static const type_unsigned_8 shrd[]={0x0fu,0xacu,0x0eu,0x10u,0u,1u};
    const type_unsigned_32 flags=VCPU_EFLAGS_CF|VCPU_EFLAGS_ZF;
    type_unsigned_8 pass;
    for(pass=0u;pass<2u;++pass) {
        shift_machine state;t_cpu after;core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;type_unsigned_16 before=0x8123u,observed=0u;
        C_INT failed=!shift_prepare_protected(pass==0u,pass==0u,&state);
        if(!failed) {
            state.machine->executor_cpu.data.ecx=0x7654u;
            state.machine->executor_cpu.data.eflags=flags;
            failed|=core_machine_memory_write(state.machine,0x3010u,&before,2u)!=TYPE_STATUS_OK||
                core_machine_memory_write(state.machine,0x2000u,pass?shrd:shld,sizeof(shld))!=TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine,0u);
            failed|=core_machine_run(state.machine,(core_machine_run_budget){1u,0u},&result)!=TYPE_STATUS_FAULT||
                result.reason!=CORE_MACHINE_STOP_FAULT||core_machine_get_cpu_diagnostic(state.machine,&diagnostic)!=TYPE_STATUS_OK;
            after=test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed|=!diagnostic.first_fault.valid||!TYPE_GET_BIT(diagnostic.first_fault.exception_mask,VCPUINS_EXCEPT_DF)||
                core_machine_memory_read_physical(&state.machine->executor_memory,0x3010u,
                    TYPE_REFERENCE_OF(observed),sizeof(observed))!=TYPE_STATUS_OK||observed!=before||
                after.data.eflags!=flags||after.data.eip!=0u;
        }
        core_machine_destroy(state.machine);if(failed)return 0;
    }
    return 1;
}

C_INT main(C_VOID)
{
    if(!shift_test_forms()||!shift_test_count_zero()||!shift_test_profile()||
        !shift_test_access_failure())return 1;
    STD_PRINTF("M5:T310:S6:DOUBLE-SHIFT:OK\n");
    STD_PRINTF("M5:T401:S62:DOUBLE-SHIFT-PROFILES:OK\n");return 0;
}
