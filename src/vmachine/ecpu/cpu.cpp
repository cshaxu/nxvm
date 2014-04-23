//////////////////////////////////////////////////////////////////////////
// 名称	：CPU.cpp
// 功能	：模拟CPU
// 日期	：2008年4月20日
// 作者	：梁一信
//////////////////////////////////////////////////////////////////////////

#include "stdio.h"
#include "memory.h"

#include "../vpic.h"
#include "../vcpu.h"
#include "cpu.h"
#include "Instruction.h"

#define eIMS (evIP+MemoryStart)
#define EvSP (t=eCPU.ss,(t<<4)+eCPU.sp)

t_cpu eCPU;					//虚拟机的CPU的实体
unsigned int evIP;			//evIP读指令时的指针
unsigned int GlobINT;		//所有非INT指令产生的中断
t_bool bStartRecord=0x00;			//从某个位置开始，进入调试状态
unsigned int InstructionCount=0;

void IO_NOP()		//未实现的IO口
{
	__asm mov eax,0xcccccccc				//Debug版本会有这一句，以使函数返回cc，但是Release版本则没有了。
											//端口返回CC刚好给使某些软件正常运行，这是运气。正确的做法应该是实现那个端口。例如3DAh端口
	__asm nop
}

int CPUInit()
{
	memset(&eCPU,0,sizeof(t_cpu));
	SetupInstructionTable();
	eCPU.cs=0xf000;
	eCPU.ip=0xfff0;
	evIP=eCPU.cs;
	evIP<<=4;
	evIP+=eCPU.ip;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// 执行evIP所指向的那一条指令
int ExecIns()
{
	unsigned char OpC=*(unsigned char *)(evIP+MemoryStart);
	unsigned int InsFucAddr=InsTable[OpC];
	unsigned int tcs=eCPU.cs;
	evIP++;	
	InstructionCount++;
	__asm call InsFucAddr;
	eCPU.ip=(evIP - (tcs << 4)) % 0x10000;
	tmpDs=eCPU.ds;
	tmpSs=eCPU.ss;
	return 0;
}


//////////////////////////////////////////////////////////////////////////
// 前往某个中断服务程序
void GoInt(int r)
{
	//LongCallNewIP(1);					//因为ExcIns里面是先执行指令再修改CS:IP的，所以运行到这里的CS:IP是未被执行的，所以不用再LongCallNewIP了。
	unsigned int t;	
	eCPU.sp-=2;
	*(unsigned short*)( EvSP +MemoryStart)=eCPU.flags;
	eCPU.flags&=~IF;
	eCPU.flags&=~TF;
	eCPU.flags&=~AF;
	eCPU.sp-=2;
	*(unsigned short*)( EvSP +MemoryStart)=eCPU.cs;				//先压CS，再压IP，而且是要压CALL指令之后的IP
	eCPU.sp-=2;
	*(unsigned short*)( EvSP +MemoryStart)=eCPU.ip;
	eCPU.ip=*(unsigned short *)((r)*4+MemoryStart);
	eCPU.cs=*(unsigned short *)((r)*4+2+MemoryStart);
	evIP=((t=eCPU.cs,t<<4))+eCPU.ip;
}


//////////////////////////////////////////////////////////////////////////
// 检查是否有中断请求，并且作出相应处理
int ExecInt()
{	
	char intR;
	unsigned int tmpGlobINT;
	unsigned int IntNo;
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
		GoInt(intR+8);						//PIC的IR0接中断号8
		return intR+8;
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////
// easyVM关机时被调用
void CPUTerminate() {}