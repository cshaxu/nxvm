#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define SCAN_PROVIDER_ADDRESS 0x5000u
typedef struct scan_provider { type_unsigned_32 reads; } scan_provider;
typedef struct scan_machine { core_machine *machine; } scan_machine;

static type_status scan_read(C_VOID *owner,type_unsigned_32 physical,
    type_virtual_address destination,type_native_unsigned bytes)
{
    scan_provider *provider=(scan_provider *)owner;
    (C_VOID)destination;
    if(provider==STD_NULL||physical!=SCAN_PROVIDER_ADDRESS||(bytes!=2u&&bytes!=4u))
        return TYPE_STATUS_INVALID_ARGUMENT;
    ++provider->reads;return TYPE_STATUS_OK;
}
static type_status scan_write(C_VOID *owner,type_unsigned_32 physical,
    type_virtual_address source,type_native_unsigned bytes)
{ (C_VOID)owner;(C_VOID)physical;(C_VOID)source;(C_VOID)bytes;return TYPE_STATUS_UNSUPPORTED; }
static type_status scan_query(C_VOID *owner,type_unsigned_32 physical,
    type_native_unsigned bytes,core_machine_memory_access access)
{ (C_VOID)owner;return physical==SCAN_PROVIDER_ADDRESS&&(bytes==2u||bytes==4u)&&
    access==CORE_MACHINE_MEMORY_ACCESS_READ?TYPE_STATUS_OK:TYPE_STATUS_UNSUPPORTED; }
static C_VOID scan_reset(C_VOID *opaque)
{ scan_machine *state=(scan_machine *)opaque;if(state!=STD_NULL)(C_VOID)test_core_machine_fixture_reset_real_mode(state->machine); }
static const core_machine_execution_provider scan_execution={scan_reset,STD_NULL};

static C_INT scan_prepare(core_machine_cpu_profile profile,scan_provider *provider,
    scan_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if(core_machine_create(&config,&state->machine)!=TYPE_STATUS_OK||
        (provider!=STD_NULL&&test_core_machine_fixture_register_memory_device_provider(state->machine,
            SCAN_PROVIDER_ADDRESS,4u,scan_read,scan_write,scan_query,provider)!=TYPE_STATUS_OK)||
        !test_core_machine_fixture_bind_freeze_reset(state->machine,
            &scan_execution,state)) {
        core_machine_destroy(state->machine);state->machine=STD_NULL;return 0;
    }
    return 1;
}
static C_INT scan_run_real(scan_machine *state,const type_unsigned_8 *code,STD_SIZE_T bytes,
    C_INT fault,t_cpu *out,core_machine_cpu_diagnostic *diagnostic)
{
    core_machine_run_result result;type_status status;
    if(state==STD_NULL||state->machine==STD_NULL||!test_core_machine_fixture_prepare_real_mode_execution(state->machine,0u)||
        core_machine_memory_write(state->machine,0u,code,bytes)!=TYPE_STATUS_OK)return 0;
    if(fault&&!test_core_machine_fixture_preflight_real_ud_terminal(state->machine))return 0;
    status=core_machine_run(state->machine,(core_machine_run_budget){1u,0u},&result);
    if(status!=(fault?TYPE_STATUS_FAULT:TYPE_STATUS_OK)||result.reason!=(fault?CORE_MACHINE_STOP_FAULT:CORE_MACHINE_STOP_BUDGET)||
        core_machine_get_cpu_diagnostic(state->machine,diagnostic)!=TYPE_STATUS_OK)return 0;
    *out=test_core_machine_fixture_capture_cpu_after_run(state->machine);return 1;
}

static C_INT scan_test_forms(C_VOID)
{
    static const type_unsigned_8 opcodes[]={0xbcu,0xbdu};
    const type_unsigned_32 flags=VCPU_EFLAGS_CF|VCPU_EFLAGS_OF;
    type_unsigned_8 opcode,width,memory,zero;
    for(opcode=0u;opcode<2u;++opcode)for(width=0u;width<2u;++width)
    for(memory=0u;memory<2u;++memory)for(zero=0u;zero<2u;++zero) {
        type_unsigned_8 code[6]={0};STD_SIZE_T bytes=0u;
        const type_unsigned_32 source=zero?0u:(width?0x80000120u:0x00008120u);
        const type_unsigned_32 expected=opcode? (width?31u:15u):5u;
        scan_machine state;t_cpu after={0};core_machine_cpu_diagnostic diagnostic;type_unsigned_32 read=0u;
        C_INT failed=!scan_prepare(CORE_MACHINE_CPU_PROFILE_80386,STD_NULL,&state);
        if (memory && width) code[bytes++] = 0x67u;
        if (width) code[bytes++] = 0x66u;
        code[bytes++]=0x0fu;code[bytes++]=opcodes[opcode];
        if(memory){code[bytes++]=0x0eu;if(width){}else{code[bytes++]=0x00u;code[bytes++]=0x40u;}}
        else code[bytes++]=0xc8u;
        if(!failed) {
            state.machine->executor_cpu.data.eax=source;
            state.machine->executor_cpu.data.ecx=0xaabbccddu;
            state.machine->executor_cpu.data.esi=0x4000u;
            state.machine->executor_cpu.data.eflags=flags;
            failed |= (memory && core_machine_memory_write(state.machine, 0x4000u,
                &source, width ? 4u : 2u) != TYPE_STATUS_OK) ||
                !scan_run_real(&state,code,bytes,0,&after,&diagnostic)||diagnostic.first_fault.valid;
            if(!zero)failed|=(width?after.data.ecx:(after.data.ecx&0xffffu))!=expected||
                TYPE_GET_BIT(after.data.eflags,VCPU_EFLAGS_ZF);
            else failed|=!TYPE_GET_BIT(after.data.eflags,VCPU_EFLAGS_ZF);
            if(memory)failed|=core_machine_memory_read(state.machine,0x4000u,&read,width?4u:2u)!=TYPE_STATUS_OK||read!=source;
        }
        core_machine_destroy(state.machine);if(failed)return 0;
    }
    return 1;
}

static C_INT scan_test_profile(C_VOID)
{
    static const type_unsigned_8 code[]={0x0fu,0xbcu,0x0eu,0x00u,0x50u};
    core_machine_cpu_profile profiles[]={CORE_MACHINE_CPU_PROFILE_80186,CORE_MACHINE_CPU_PROFILE_80286};type_unsigned_8 index;
    for(index=0u;index<2u;++index) {
        scan_provider provider={0u};scan_machine state;t_cpu after;core_machine_cpu_diagnostic diagnostic;
        C_INT failed=!scan_prepare(profiles[index],&provider,&state);
        if(!failed) {state.machine->executor_cpu.data.eax=0xaabbccddu;state.machine->executor_cpu.data.eflags=VCPU_EFLAGS_CF;
            failed|=!scan_run_real(&state,code,sizeof(code),1,&after,&diagnostic)||
                !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,VCPUINS_EXCEPT_UD)||provider.reads!=0u||
                after.data.eax!=0xaabbccddu||after.data.eflags!=VCPU_EFLAGS_CF||after.data.eip!=0u;}
        core_machine_destroy(state.machine);if(failed)return 0;
    }
    return 1;
}

static C_INT scan_prepare_limit(scan_machine *state)
{
    static const type_unsigned_8 pointer[]={0x1fu,0,0,0x03u,0,0};
    static const type_unsigned_8 gdt[]={
        0,0,0,0,0,0,0,0,0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0x0fu,0,0,0x30u,0,0x92u,0,0,0xffu,0xffu,0,0x40u,0,0x92u,0,0
    };
    static const type_unsigned_8 bootstrap[]={
        0x0fu,0x01u,0x16u,0x00u,0x01u,0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,0xb8u,0x18u,0x00u,
        0x8eu,0xd0u,0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const type_unsigned_8 halt[]={0xf4u};core_machine_run_result result;
    return scan_prepare(CORE_MACHINE_CPU_PROFILE_80386,STD_NULL,state)&&
        core_machine_memory_write(state->machine,0x0100u,pointer,sizeof(pointer))==TYPE_STATUS_OK&&
        core_machine_memory_write(state->machine,0x0300u,gdt,sizeof(gdt))==TYPE_STATUS_OK&&
        core_machine_memory_write(state->machine,0u,bootstrap,sizeof(bootstrap))==TYPE_STATUS_OK&&
        core_machine_memory_write(state->machine,0x2000u,halt,sizeof(halt))==TYPE_STATUS_OK&&
        core_machine_run(state->machine,(core_machine_run_budget){96u,0u},&result)==TYPE_STATUS_OK&&
        result.reason==CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT scan_test_read_failure(C_VOID)
{
    static const type_unsigned_8 codes[][5]={{0x0fu,0xbcu,0x0eu,0x10u,0u},{0x0fu,0xbdu,0x0eu,0x10u,0u}};
    const type_unsigned_32 flags=VCPU_EFLAGS_CF|VCPU_EFLAGS_OF;type_unsigned_8 index;
    for(index=0u;index<2u;++index) {
        scan_machine state;t_cpu after;core_machine_cpu_diagnostic diagnostic;core_machine_run_result result;
        C_INT failed=!scan_prepare_limit(&state);
        if(!failed) {
            state.machine->executor_cpu.data.ecx=0xaabbccddu;state.machine->executor_cpu.data.eflags=flags;
            failed|=core_machine_memory_write(state.machine,0x2000u,codes[index],sizeof(codes[index]))!=TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine,0u);
            failed|=core_machine_run(state.machine,(core_machine_run_budget){1u,0u},&result)!=TYPE_STATUS_FAULT||
                result.reason!=CORE_MACHINE_STOP_FAULT||core_machine_get_cpu_diagnostic(state.machine,&diagnostic)!=TYPE_STATUS_OK;
            after=test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed|=!diagnostic.first_fault.valid||!TYPE_GET_BIT(diagnostic.first_fault.exception_mask,VCPUINS_EXCEPT_DF)||
                after.data.ecx!=0xaabbccddu||after.data.eflags!=flags||after.data.eip!=0u;
        }
        core_machine_destroy(state.machine);if(failed)return 0;
    }
    return 1;
}

C_INT main(C_VOID)
{
    if(!scan_test_forms()||!scan_test_profile()||!scan_test_read_failure())return 1;
    STD_PRINTF("M5:T310:S7:BIT-SCAN:OK\n");
    STD_PRINTF("M5:T401:S63:BIT-SCAN-PROFILES:OK\n");return 0;
}
