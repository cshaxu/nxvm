//////////////////////////////////////////////////////////////////////////
// 名称	：CPU.cpp
// 功能	：模拟CPU
// 日期	：2008年4月20日
// 作者	：梁一信
//////////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "memory.h"

#include "../vapi.h"
#include "../vpic.h"
#include "../vcpu.h"
#include "cpu.h"
#include "Instruction.h"

#define eIMS (evIP+MemoryStart)
#define EvSP (t=eCPU.ss,(t<<4)+eCPU.sp)

t_cpu eCPU;
//unsigned int InstructionCount=0;

int CPUInit()
{
	memset(&eCPU,0,sizeof(t_cpu));
	eCPU.cs=0xf000;
	eCPU.ip=0xfff0;
	ecpuinsInit();
	evIP=eCPU.cs;
	evIP<<=4;
	evIP+=eCPU.ip;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// 执行evIP所指向的那一条指令
int ExecIns()
{
	t_nubit8 opcode = d_nubit8(evIP+MemoryStart);
	t_nubit8 op1 = d_nubit8(evIP+MemoryStart+1);
	t_nubit8 op2 = d_nubit8(evIP+MemoryStart+2);
	t_nubit8 op3 = d_nubit8(evIP+MemoryStart+3);
	t_nubit8 op4 = d_nubit8(evIP+MemoryStart+4);
//	t_faddrcc InsFucAddr = InsTable[opcode];
//	vapiPrint("execins.1: %04X:%04X, INT=%1X\n",_cs,_ip,GetBit(_flags,VCPU_FLAG_IF));
	evIP++;

	//InstructionCount++;
	ExecFun(InsTable[opcode]);
//	__asm call InsFucAddr;
	eCPU.ip=(evIP - (eCPU.cs << 4)) % 0x10000;
	tmpDs=eCPU.ds;
	tmpSs=eCPU.ss;
//	vapiPrint("execins.2: %04X:%04X, INT=%1X\n",_cs,_ip,GetBit(_flags,VCPU_FLAG_IF));
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// 前往某个中断服务程序
void GoInt(int r)
{
	//LongCallNewIP(1);;					//因为ExcIns里面是先执行指令再修改CS:IP的，所以运行到这里的CS:IP是未被执行的，所以不用再LongCallNewIP了。
	t_vaddrcc t;
//	vapiPrint("goint.1: %04X:%04X, INT=%1X\n",_cs,_ip,GetBit(_flags,VCPU_FLAG_IF));
	eCPU.sp -= 2;
	d_nubit16(EvSP + MemoryStart) = eCPU.flags;
	eCPU.flags &= ~IF;
	eCPU.flags &= ~TF;
	eCPU.flags &= ~AF;
	eCPU.sp -= 2;
	d_nubit16(EvSP + MemoryStart) = eCPU.cs;				//先压CS，再压IP，而且是要压CALL指令之后的IP
	eCPU.sp -= 2;
	d_nubit16(EvSP + MemoryStart) = eCPU.ip;
	eCPU.ip = d_nubit16((r) * 4 + MemoryStart + 0);
	eCPU.cs = d_nubit16((r) * 4 + MemoryStart + 2);
	evIP = (eCPU.cs << 4) + eCPU.ip;
//	vapiPrint("goint.2: %04X:%04X, INT=%1X\n",_cs,_ip,GetBit(_flags,VCPU_FLAG_IF));
}

//////////////////////////////////////////////////////////////////////////
// 检查是否有中断请求，并且作出相应处理
int ExecInt()
{	
	char intR;
	unsigned int tmpGlobINT;
	unsigned int IntNo;
//	vapiPrint("execint.1: %04X:%04X, INT=%1X\n",_cs,_ip,GetBit(_flags,VCPU_FLAG_IF));
	if (eCPU.flags&TF)
	{
		GlobINT|=0x2;
		eCPU.flags&=~TF;
	}
	tmpGlobINT=GlobINT;
	IntNo=0;
	while (!(tmpGlobINT&1) && IntNo<8)		//所有少于7的中断（即不连接到8259上的中断）
	{
		IntNo++;
	}

	if (IntNo==2)
	{
		GoInt(2);							//不可屏蔽中断
		GlobINT&=~0x2;
		return IntNo;
	}

	if (eCPU.flags&IF)
	{	
		if (IntNo<8)
		{
			GoInt(IntNo);
			GlobINT&=~(unsigned int)(1<<IntNo);
			return IntNo;
		}
		if (!vpicScanINTR()) return -1;
		intR=vpicGetINTR();
		
//		vapiPrint("Enable INT: %02X\n",intR);
		GoInt(intR);
		return intR;
	}
//	vapiPrint("execint.2: %04X:%04X, INT=%1X\n",_cs,_ip,GetBit(_flags,VCPU_FLAG_IF));
	return -1;
}

//////////////////////////////////////////////////////////////////////////
// easyVM关机时被调用
void CPUTerminate() {ecpuinsFinal();}