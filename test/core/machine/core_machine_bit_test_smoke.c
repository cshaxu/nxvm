#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define BIT_SOURCE_MEMORY 0x5000u

typedef struct bit_memory_provider {
    type_unsigned_32 reads;
    type_unsigned_32 writes;
    type_unsigned_8 value[4];
    type_status read_status;
    type_status write_status;
} bit_memory_provider;

typedef struct bit_machine { core_machine *machine; } bit_machine;

static type_status bit_memory_read(C_VOID *owner, type_unsigned_32 physical,
    type_virtual_address destination, type_native_unsigned bytes)
{
    bit_memory_provider *provider=(bit_memory_provider *)owner;
    if(provider==STD_NULL||physical!=BIT_SOURCE_MEMORY||(bytes!=2u&&bytes!=4u))
        return TYPE_STATUS_INVALID_ARGUMENT;
    ++provider->reads;
    if(provider->read_status!=TYPE_STATUS_OK)return provider->read_status;
    STD_MEMCPY((C_VOID *)destination,provider->value,bytes);
    return TYPE_STATUS_OK;
}

static type_status bit_memory_write(C_VOID *owner, type_unsigned_32 physical,
    type_virtual_address source, type_native_unsigned bytes)
{
    bit_memory_provider *provider=(bit_memory_provider *)owner;
    if(provider==STD_NULL||physical!=BIT_SOURCE_MEMORY||(bytes!=2u&&bytes!=4u))
        return TYPE_STATUS_INVALID_ARGUMENT;
    ++provider->writes;
    if(provider->write_status!=TYPE_STATUS_OK)return provider->write_status;
    STD_MEMCPY(provider->value,(const C_VOID *)source,bytes);
    return TYPE_STATUS_OK;
}

static type_status bit_memory_query(C_VOID *owner, type_unsigned_32 physical,
    type_native_unsigned bytes, core_machine_memory_access access)
{
    (C_VOID)owner;
    return physical==BIT_SOURCE_MEMORY&&(bytes==2u||bytes==4u)&&
        (access==CORE_MACHINE_MEMORY_ACCESS_READ||access==CORE_MACHINE_MEMORY_ACCESS_WRITE)?
        TYPE_STATUS_OK:TYPE_STATUS_UNSUPPORTED;
}

static C_VOID bit_reset(C_VOID *opaque)
{
    bit_machine *state = (bit_machine *)opaque;
    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider bit_provider = {bit_reset,STD_NULL};

static C_INT bit_prepare(core_machine_cpu_profile profile, bit_memory_provider *provider,
    bit_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    if (state == STD_NULL) return 0;
    STD_MEMSET(state,0,sizeof(*state));
    if (core_machine_create(&config,&state->machine) != TYPE_STATUS_OK ||
        (provider!=STD_NULL&&test_core_machine_fixture_register_memory_device_provider(
            state->machine,BIT_SOURCE_MEMORY,4u,bit_memory_read,bit_memory_write,
            bit_memory_query,provider)!=TYPE_STATUS_OK)||
        !test_core_machine_fixture_bind_freeze_reset(state->machine,
            &bit_provider,state)) {
        core_machine_destroy(state->machine); state->machine=STD_NULL; return 0;
    }
    return 1;
}

static C_INT bit_run(bit_machine *state,const type_unsigned_8 *code,STD_SIZE_T bytes,
    C_INT fault,t_cpu *out,core_machine_cpu_diagnostic *diagnostic)
{
    const core_machine_run_budget budget={1u,0u}; core_machine_run_result result;
    type_status status;
    if (state==STD_NULL || state->machine==STD_NULL || !test_core_machine_fixture_prepare_real_mode_execution(state->machine,0u) ||
        core_machine_memory_write(state->machine,0u,code,bytes)!=TYPE_STATUS_OK) return 0;
    if(fault&&!test_core_machine_fixture_preflight_real_ud_terminal(state->machine))return 0;
    status=core_machine_run(state->machine,budget,&result);
    if (status!=(fault?TYPE_STATUS_FAULT:TYPE_STATUS_OK) || result.reason!=(fault?CORE_MACHINE_STOP_FAULT:CORE_MACHINE_STOP_BUDGET) ||
        core_machine_get_cpu_diagnostic(state->machine,diagnostic)!=TYPE_STATUS_OK) return 0;
    *out=test_core_machine_fixture_capture_cpu_after_run(state->machine); return 1;
}

static C_INT bit_test_register_forms(C_VOID)
{
    static const type_unsigned_8 opcodes[]={0xa3u,0xabu,0xb3u,0xbbu};
    static const type_unsigned_32 expected[]={0x00000002u,0x00000002u,0x00000000u,0x00000000u};
    type_unsigned_8 index;
    for(index=0u;index<4u;++index) {
        type_unsigned_8 operand32;
        for(operand32=0u;operand32<2u;++operand32) {
            type_unsigned_8 code[]={0x66u,0x0fu,opcodes[index],0xc8u};
            bit_machine state; t_cpu after; core_machine_cpu_diagnostic diagnostic;
            C_INT failed=!bit_prepare(CORE_MACHINE_CPU_PROFILE_80386,STD_NULL,&state);
            if (!operand32) { code[0]=0x0fu; code[1]=opcodes[index]; code[2]=0xc8u; }
            if (!failed) {
                state.machine->executor_cpu.data.eax=operand32?2u:0xaabb0002u;
                state.machine->executor_cpu.data.ecx=1u;
                state.machine->executor_cpu.data.eflags=VCPU_EFLAGS_ZF|VCPU_EFLAGS_OF;
                failed|=!bit_run(&state,code,operand32?4u:3u,0,&after,&diagnostic)||diagnostic.first_fault.valid||
                    after.data.eax!=(operand32?expected[index]:(0xaabb0000u|(expected[index]&0xffffu)))||
                    !!TYPE_GET_BIT(after.data.eflags,VCPU_EFLAGS_CF)!=1;
            }
            core_machine_destroy(state.machine);if(failed)return 0;
        }
    }
    return 1;
}

static C_INT bit_test_immediate_and_memory(C_VOID)
{
    static const type_unsigned_8 groups[]={4u,5u,6u,7u};
    type_unsigned_8 group;
    for(group=0u;group<4u;++group) {
        type_unsigned_8 code[]={0x0fu,0xbau,(type_unsigned_8)(0xe1u+(groups[group]-4u)*8u),1u};
        bit_machine state; t_cpu after; core_machine_cpu_diagnostic diagnostic;
        C_INT failed=!bit_prepare(CORE_MACHINE_CPU_PROFILE_80386,STD_NULL,&state);
        if(!failed) {
            state.machine->executor_cpu.data.ecx=2u;
            state.machine->executor_cpu.data.eflags=VCPU_EFLAGS_ZF;
            failed|=!bit_run(&state,code,sizeof(code),0,&after,&diagnostic)||diagnostic.first_fault.valid||
                after.data.ecx!=((group>=2u)?0u:2u)||!TYPE_GET_BIT(after.data.eflags,VCPU_EFLAGS_CF);
        }
        core_machine_destroy(state.machine);if(failed)return 0;
    }
    {
        static const type_unsigned_8 signed_code[]={0x0fu,0xabu,0x0eu,0x02u,0x40u};
        static const type_unsigned_8 immediate_code[]={0x0fu,0xbau,0x2eu,0x00u,0x40u,0x10u};
        static const type_unsigned_8 immediate32_code[]={0x66u,0x0fu,0xbau,0x2eu,0x08u,0x40u,0x21u};
        type_unsigned_16 first=0u,second=0u,read=0u;
        type_unsigned_32 third=0u,fourth=0u;
        bit_machine state;t_cpu after;core_machine_cpu_diagnostic diagnostic;
        C_INT failed=!bit_prepare(CORE_MACHINE_CPU_PROFILE_80386,STD_NULL,&state);
        if(!failed) {
            state.machine->executor_cpu.data.ecx=0xffffu;
            state.machine->executor_cpu.data.eflags=VCPU_EFLAGS_ZF;
            failed|=core_machine_memory_write(state.machine,0x4000u,&first,2u)!=TYPE_STATUS_OK||
                core_machine_memory_write(state.machine,0x4002u,&second,2u)!=TYPE_STATUS_OK||
                !bit_run(&state,signed_code,sizeof(signed_code),0,&after,&diagnostic)||
                core_machine_memory_read(state.machine,0x4000u,&read,2u)!=TYPE_STATUS_OK||read!=0x8000u||
                TYPE_GET_BIT(after.data.eflags,VCPU_EFLAGS_CF);
        }
        if(!failed) {
            state.machine->executor_cpu.data.eflags=VCPU_EFLAGS_ZF;
            failed|=!bit_run(&state,immediate_code,sizeof(immediate_code),0,&after,&diagnostic)||
                core_machine_memory_read(state.machine,0x4002u,&read,2u)!=TYPE_STATUS_OK||read!=1u||
                TYPE_GET_BIT(after.data.eflags,VCPU_EFLAGS_CF);
        }
        if(!failed) {
            state.machine->executor_cpu.data.eflags=VCPU_EFLAGS_ZF;
            failed|=core_machine_memory_write(state.machine,0x4008u,&third,4u)!=TYPE_STATUS_OK||
                core_machine_memory_write(state.machine,0x400cu,&fourth,4u)!=TYPE_STATUS_OK||
                !bit_run(&state,immediate32_code,sizeof(immediate32_code),0,&after,&diagnostic)||
                core_machine_memory_read(state.machine,0x400cu,&fourth,4u)!=TYPE_STATUS_OK||fourth!=2u||
                TYPE_GET_BIT(after.data.eflags,VCPU_EFLAGS_CF);
        }
        core_machine_destroy(state.machine);if(failed)return 0;
    }
    {
        static const type_unsigned_8 memory_bt[]={0x0fu,0xa3u,0x0eu,0x00u,0x40u};
        static const type_unsigned_8 address32_bts[]={0x67u,0x66u,0x0fu,0xabu,0x0eu};
        type_unsigned_16 word=2u,read16=0u;
        type_unsigned_32 dword=0u,read32=0u;
        bit_machine state;t_cpu after;core_machine_cpu_diagnostic diagnostic;
        C_INT failed=!bit_prepare(CORE_MACHINE_CPU_PROFILE_80386,STD_NULL,&state);
        if(!failed) {
            state.machine->executor_cpu.data.ecx=1u;
            failed|=core_machine_memory_write(state.machine,0x4000u,&word,2u)!=TYPE_STATUS_OK||
                !bit_run(&state,memory_bt,sizeof(memory_bt),0,&after,&diagnostic)||
                core_machine_memory_read(state.machine,0x4000u,&read16,2u)!=TYPE_STATUS_OK||read16!=word||
                !TYPE_GET_BIT(after.data.eflags,VCPU_EFLAGS_CF);
        }
        if(!failed) {
            state.machine->executor_cpu.data.esi=0x4004u;
            state.machine->executor_cpu.data.ecx=1u;
            failed|=core_machine_memory_write(state.machine,0x4004u,&dword,4u)!=TYPE_STATUS_OK||
                !bit_run(&state,address32_bts,sizeof(address32_bts),0,&after,&diagnostic)||
                core_machine_memory_read(state.machine,0x4004u,&read32,4u)!=TYPE_STATUS_OK||read32!=2u||
                TYPE_GET_BIT(after.data.eflags,VCPU_EFLAGS_CF);
        }
        core_machine_destroy(state.machine);if(failed)return 0;
    }
    return 1;
}

static C_INT bit_test_memory_destination_forms(C_VOID)
{
    static const type_unsigned_8 opcodes[]={0xa3u,0xabu,0xb3u,0xbbu};
    type_unsigned_8 form;

    for(form=0u;form<4u;++form) {
        type_unsigned_8 indexed[]={0x0fu,opcodes[form],0x0eu,0x00u,0x40u};
        type_unsigned_8 immediate[]={0x0fu,0xbau,(type_unsigned_8)(0x26u+form*8u),0x00u,0x40u,1u};
        type_unsigned_16 value=2u,read=0u;
        bit_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed=!bit_prepare(CORE_MACHINE_CPU_PROFILE_80386,STD_NULL,&state);

        if(!failed) {
            state.machine->executor_cpu.data.ecx=1u;
            failed|=core_machine_memory_write(state.machine,0x4000u,&value,2u)!=TYPE_STATUS_OK||
                !bit_run(&state,indexed,sizeof(indexed),0,&after,&diagnostic)||
                core_machine_memory_read(state.machine,0x4000u,&read,2u)!=TYPE_STATUS_OK||
                read!=((form>=2u)?0u:2u)||!TYPE_GET_BIT(after.data.eflags,VCPU_EFLAGS_CF);
        }
        if(!failed) {
            value=2u;
            failed|=core_machine_memory_write(state.machine,0x4000u,&value,2u)!=TYPE_STATUS_OK||
                !bit_run(&state,immediate,sizeof(immediate),0,&after,&diagnostic)||
                core_machine_memory_read(state.machine,0x4000u,&read,2u)!=TYPE_STATUS_OK||
                read!=((form>=2u)?0u:2u)||!TYPE_GET_BIT(after.data.eflags,VCPU_EFLAGS_CF);
        }
        core_machine_destroy(state.machine);
        if(failed)return 0;
    }
    return 1;
}

static C_INT bit_test_rejection(C_VOID)
{
    static const type_unsigned_8 invalid_ba[]={0x0fu,0xbau,0x06u,0x00u,0x50u,0u};
    static const type_unsigned_8 bt_memory[]={0x0fu,0xa3u,0x0eu,0x00u,0x50u};
    core_machine_cpu_profile profiles[]={CORE_MACHINE_CPU_PROFILE_80186,CORE_MACHINE_CPU_PROFILE_80286};
    type_unsigned_8 i;
    for(i=0u;i<2u;++i) {
        bit_machine state;t_cpu after;core_machine_cpu_diagnostic diagnostic;
        bit_memory_provider provider={0u,0u,{0u,0u,0u,0u},TYPE_STATUS_OK,TYPE_STATUS_OK};
        C_INT failed=!bit_prepare(profiles[i],&provider,&state);
        if(!failed) { state.machine->executor_cpu.data.eflags=VCPU_EFLAGS_ZF;state.machine->executor_cpu.data.ecx=0xaabbccddu;
            failed|=!bit_run(&state,bt_memory,sizeof(bt_memory),1,&after,&diagnostic)||!TYPE_GET_BIT(diagnostic.first_fault.exception_mask,VCPUINS_EXCEPT_UD)||provider.reads!=0u||provider.writes!=0u||after.data.ecx!=0xaabbccddu||after.data.eflags!=VCPU_EFLAGS_ZF||after.data.eip!=0u; }
        core_machine_destroy(state.machine);if(failed)return 0;
    }
    { bit_memory_provider provider={0u,0u,{0u,0u,0u,0u},TYPE_STATUS_OK,TYPE_STATUS_OK};bit_machine state;t_cpu after;core_machine_cpu_diagnostic diagnostic;C_INT failed=!bit_prepare(CORE_MACHINE_CPU_PROFILE_80386,&provider,&state);
      if(!failed){state.machine->executor_cpu.data.eflags=VCPU_EFLAGS_ZF;failed|=!bit_run(&state,invalid_ba,sizeof(invalid_ba),1,&after,&diagnostic)||!TYPE_GET_BIT(diagnostic.first_fault.exception_mask,VCPUINS_EXCEPT_UD)||provider.reads!=0u||provider.writes!=0u||after.data.eflags!=VCPU_EFLAGS_ZF||after.data.eip!=0u;}
      core_machine_destroy(state.machine);if(failed)return 0; }
    return 1;
}

static C_INT bit_prepare_protected(C_INT writable, C_INT out_of_limit,
    bit_machine *state)
{
    static const type_unsigned_8 gdt_pointer[]={0x1fu,0,0,0x03u,0,0};
    type_unsigned_8 gdt[]={
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const type_unsigned_8 bootstrap[]={
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,
        0xb8u,0x18u,0x00u,0x8eu,0xd0u,
        0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const type_unsigned_8 halt[]={0xf4u};
    const core_machine_run_budget budget={96u,0u};
    core_machine_run_result result;

    gdt[16u]=out_of_limit?0x0fu:0xffu;
    gdt[17u]=out_of_limit?0u:0xffu;
    gdt[21u]=writable?0x92u:0x90u;
    return bit_prepare(CORE_MACHINE_CPU_PROFILE_80386,STD_NULL,state)&&
        core_machine_memory_write(state->machine,0x0100u,gdt_pointer,sizeof(gdt_pointer))==TYPE_STATUS_OK&&
        core_machine_memory_write(state->machine,0x0300u,gdt,sizeof(gdt))==TYPE_STATUS_OK&&
        core_machine_memory_write(state->machine,0u,bootstrap,sizeof(bootstrap))==TYPE_STATUS_OK&&
        core_machine_memory_write(state->machine,0x2000u,halt,sizeof(halt))==TYPE_STATUS_OK&&
        core_machine_run(state->machine,budget,&result)==TYPE_STATUS_OK&&
        result.reason==CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT bit_test_access_failure(C_VOID)
{
    static const type_unsigned_8 read_code[]={0x0fu,0xa3u,0x0eu,0x10u,0u};
    static const type_unsigned_8 write_code[]={0x0fu,0xabu,0x0eu,0x10u,0u};
    const type_unsigned_32 flags=VCPU_EFLAGS_ZF;
    type_unsigned_8 pass;

    for(pass=0u;pass<2u;++pass) {
        bit_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        type_unsigned_16 before=2u,after_memory=0u;
        C_INT failed;

        failed=!bit_prepare_protected(pass==0u,pass==0u,&state);
        if(!failed) {
            state.machine->executor_cpu.data.ecx=1u;
            state.machine->executor_cpu.data.eflags=flags;
            failed|=core_machine_memory_write(state.machine,0x3010u,&before,sizeof(before))!=TYPE_STATUS_OK||
                core_machine_memory_write(state.machine,0x2000u,pass==0u?read_code:write_code,
                    pass==0u?sizeof(read_code):sizeof(write_code))!=TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine,0u);
            failed|=core_machine_run(state.machine,(core_machine_run_budget){1u,0u},&result)!=TYPE_STATUS_FAULT||
                result.reason!=CORE_MACHINE_STOP_FAULT||core_machine_get_cpu_diagnostic(state.machine,&diagnostic)!=TYPE_STATUS_OK;
            after=test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed|=!diagnostic.first_fault.valid||!TYPE_GET_BIT(diagnostic.first_fault.exception_mask,VCPUINS_EXCEPT_DF)||
                core_machine_memory_read_physical(&state.machine->executor_memory,0x3010u,
                    TYPE_REFERENCE_OF(after_memory),sizeof(after_memory))!=TYPE_STATUS_OK||after_memory!=before||
                after.data.ecx!=1u||after.data.eflags!=flags||after.data.eip!=0u;
        }
        core_machine_destroy(state.machine);
        if(failed)return 0;
    }
    return 1;
}

C_INT main(C_VOID)
{
    if(!bit_test_register_forms()||!bit_test_immediate_and_memory()||
        !bit_test_memory_destination_forms()||!bit_test_rejection()||
        !bit_test_access_failure())return 1;
    STD_PRINTF("M5:T310:S5:BIT:OK\n");
    STD_PRINTF("M5:T401:S61:BIT-TEST-PROFILES:OK\n");return 0;
}
