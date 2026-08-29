#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define IMUL_PROVIDER_ADDRESS 0x5000u
typedef struct imul_provider { type_unsigned_32 reads; } imul_provider;
typedef struct imul_machine { core_machine *machine; } imul_machine;
static type_status imul_read(C_VOID *owner,type_unsigned_32 physical,type_virtual_address dst,type_native_unsigned bytes)
{ imul_provider *p=(imul_provider *)owner;(C_VOID)dst;if(p==STD_NULL||physical!=IMUL_PROVIDER_ADDRESS||(bytes!=2u&&bytes!=4u))return TYPE_STATUS_INVALID_ARGUMENT;++p->reads;return TYPE_STATUS_OK; }
static type_status imul_write(C_VOID *o,type_unsigned_32 p,type_virtual_address s,type_native_unsigned b)
{(C_VOID)o;(C_VOID)p;(C_VOID)s;(C_VOID)b;return TYPE_STATUS_UNSUPPORTED;}
static type_status imul_query(C_VOID *o,type_unsigned_32 p,type_native_unsigned b,core_machine_memory_access a)
{(C_VOID)o;return p==IMUL_PROVIDER_ADDRESS&&(b==2u||b==4u)&&a==CORE_MACHINE_MEMORY_ACCESS_READ?TYPE_STATUS_OK:TYPE_STATUS_UNSUPPORTED;}
static C_VOID imul_reset(C_VOID *o){imul_machine *s=(imul_machine *)o;if(s!=STD_NULL)(C_VOID)test_core_machine_fixture_reset_real_mode(s->machine);}
static const core_machine_execution_provider imul_execution={imul_reset,STD_NULL};
static C_INT imul_prepare(core_machine_cpu_profile p,imul_provider *provider,imul_machine *s)
{
 const core_machine_config c={.memory_bytes=CORE_MACHINE_MINIMUM_MEMORY_BYTES,.cpu_profile=p,.fpu_profile=CORE_MACHINE_FPU_PROFILE_NONE};if(s==STD_NULL)return 0;STD_MEMSET(s,0,sizeof(*s));
 if(core_machine_create(&c,&s->machine)!=TYPE_STATUS_OK||(provider!=STD_NULL&&test_core_machine_fixture_register_memory_device_provider(s->machine,IMUL_PROVIDER_ADDRESS,4u,imul_read,imul_write,imul_query,provider)!=TYPE_STATUS_OK)||!test_core_machine_fixture_bind_freeze_reset(s->machine,&imul_execution,s)){core_machine_destroy(s->machine);s->machine=STD_NULL;return 0;}return 1;
}
static C_INT imul_run(imul_machine *s,const type_unsigned_8 *code,STD_SIZE_T bytes,C_INT fault,t_cpu *out,core_machine_cpu_diagnostic *d)
{core_machine_run_result r;type_status status;if(s==STD_NULL||s->machine==STD_NULL||!test_core_machine_fixture_prepare_real_mode_execution(s->machine,0u)||core_machine_memory_write(s->machine,0u,code,bytes)!=TYPE_STATUS_OK)return 0;if(fault&&!test_core_machine_fixture_preflight_real_ud_terminal(s->machine))return 0;status=core_machine_run(s->machine,(core_machine_run_budget){1u,0u},&r);if(status!=(fault?TYPE_STATUS_FAULT:TYPE_STATUS_OK)||r.reason!=(fault?CORE_MACHINE_STOP_FAULT:CORE_MACHINE_STOP_BUDGET)||core_machine_get_cpu_diagnostic(s->machine,d)!=TYPE_STATUS_OK)return 0;*out=test_core_machine_fixture_capture_cpu_after_run(s->machine);return 1;}

static C_INT imul_forms(C_VOID)
{
 static const type_signed_32 left[]={-2,0x7fff,-2,0x7fffffff};static const type_signed_32 right[]={3,2,0x40000000,2};
 type_unsigned_8 width,memory,overflow;
 for(width=0u;width<2u;++width)for(memory=0u;memory<2u;++memory)for(overflow=0u;overflow<2u;++overflow){
  type_unsigned_8 code[6]={0};STD_SIZE_T n=0u;const type_signed_32 l=left[width*2u+overflow],r=right[width*2u+overflow];const type_signed_64 product=(type_signed_64)l*(type_signed_64)r;const type_unsigned_32 expected=(type_unsigned_32)product;const C_INT ov=width?(product>INT32_MAX||product<INT32_MIN):(product>INT16_MAX||product<INT16_MIN);imul_machine s;t_cpu after={0};core_machine_cpu_diagnostic d;C_INT failed=!imul_prepare(CORE_MACHINE_CPU_PROFILE_80386,STD_NULL,&s);
  if(memory&&width)code[n++]=0x67u;
  if(width)code[n++]=0x66u;
  code[n++]=0x0fu;code[n++]=0xafu;if(memory){code[n++]=0x0eu;if(!width){code[n++]=0x00u;code[n++]=0x40u;}}else code[n++]=0xc8u;
  if(!failed){s.machine->executor_cpu.data.eax=(type_unsigned_32)r;s.machine->executor_cpu.data.ecx=width?(type_unsigned_32)l:0xaabb0000u|((type_unsigned_32)l&0xffffu);s.machine->executor_cpu.data.esi=0x4000u;s.machine->executor_cpu.data.eflags=VCPU_EFLAGS_ZF;failed|=(memory&&core_machine_memory_write(s.machine,0x4000u,&r,width?4u:2u)!=TYPE_STATUS_OK)||!imul_run(&s,code,n,0,&after,&d)||d.first_fault.valid;failed|=(width?after.data.ecx:(after.data.ecx&0xffffu))!=(width?expected:(expected&0xffffu))||!!TYPE_GET_BIT(after.data.eflags,VCPU_EFLAGS_CF)!=ov||!!TYPE_GET_BIT(after.data.eflags,VCPU_EFLAGS_OF)!=ov;}
  core_machine_destroy(s.machine);if(failed)return 0;
 }
 return 1;
}
static C_INT imul_profile(C_VOID)
{
 static const type_unsigned_8 code[]={0x0fu,0xafu,0x0eu,0x00u,0x50u};core_machine_cpu_profile ps[]={CORE_MACHINE_CPU_PROFILE_80186,CORE_MACHINE_CPU_PROFILE_80286};type_unsigned_8 i;
 for(i=0u;i<2u;++i){imul_provider p={0u};imul_machine s;t_cpu a;core_machine_cpu_diagnostic d;C_INT f=!imul_prepare(ps[i],&p,&s);if(!f){s.machine->executor_cpu.data.ecx=0xaabbccddu;s.machine->executor_cpu.data.eflags=VCPU_EFLAGS_CF|VCPU_EFLAGS_OF;f|=!imul_run(&s,code,sizeof(code),1,&a,&d)||!TYPE_GET_BIT(d.first_fault.exception_mask,VCPUINS_EXCEPT_UD)||p.reads!=0u||a.data.ecx!=0xaabbccddu||a.data.eflags!=(VCPU_EFLAGS_CF|VCPU_EFLAGS_OF)||a.data.eip!=0u;}core_machine_destroy(s.machine);if(f)return 0;}return 1;
}
static C_INT imul_limit(imul_machine *s)
{
 static const type_unsigned_8 p[]={0x1f,0,0,3,0,0},g[]={0,0,0,0,0,0,0,0,0xff,0xff,0,0x20,0,0x9a,0,0,0x0f,0,0,0x30,0,0x92,0,0,0xff,0xff,0,0x40,0,0x92,0,0},b[]={0x0f,1,0x16,0,1,0xb8,1,0,0x0f,1,0xf0,0xb8,0x10,0,0x8e,0xd8,0x8e,0xc0,0xb8,0x18,0,0x8e,0xd0,0xbc,0,0x80,0xea,0,0,8,0},h[]={0xf4};core_machine_run_result r;
 return imul_prepare(CORE_MACHINE_CPU_PROFILE_80386,STD_NULL,s)&&core_machine_memory_write(s->machine,0x100,p,sizeof(p))==TYPE_STATUS_OK&&core_machine_memory_write(s->machine,0x300,g,sizeof(g))==TYPE_STATUS_OK&&core_machine_memory_write(s->machine,0,b,sizeof(b))==TYPE_STATUS_OK&&core_machine_memory_write(s->machine,0x2000,h,sizeof(h))==TYPE_STATUS_OK&&core_machine_run(s->machine,(core_machine_run_budget){96u,0u},&r)==TYPE_STATUS_OK&&r.reason==CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}
static C_INT imul_read_failure(C_VOID)
{
 static const type_unsigned_8 code[]={0x0f,0xafu,0x0eu,0x10u,0};imul_machine s;t_cpu a;core_machine_cpu_diagnostic d;core_machine_run_result r;const type_unsigned_32 flags=VCPU_EFLAGS_CF|VCPU_EFLAGS_OF;C_INT f=!imul_limit(&s);
 if(!f){s.machine->executor_cpu.data.ecx=0xaabbccddu;s.machine->executor_cpu.data.eflags=flags;f|=core_machine_memory_write(s.machine,0x2000u,code,sizeof(code))!=TYPE_STATUS_OK;test_core_machine_fixture_resume_after_halt_at(s.machine,0u);f|=core_machine_run(s.machine,(core_machine_run_budget){1u,0u},&r)!=TYPE_STATUS_FAULT||r.reason!=CORE_MACHINE_STOP_FAULT||core_machine_get_cpu_diagnostic(s.machine,&d)!=TYPE_STATUS_OK;a=test_core_machine_fixture_capture_cpu_after_run(s.machine);f|=!d.first_fault.valid||!TYPE_GET_BIT(d.first_fault.exception_mask,VCPUINS_EXCEPT_DF)||a.data.ecx!=0xaabbccddu||a.data.eflags!=flags||a.data.eip!=0u;}core_machine_destroy(s.machine);return !f;
}
C_INT main(C_VOID){if(!imul_forms()||!imul_profile()||!imul_read_failure())return 1;STD_PRINTF("M5:T310:S8:IMUL2:OK\n");
STD_PRINTF("M5:T401:S65:IMUL2-PROFILES:OK\n");return 0;}
