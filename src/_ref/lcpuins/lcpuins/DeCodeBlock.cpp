
#include "../lcpuins.h"
#include "CentreProcessorUnit.h"
#include "../vmachine.h"

#include<iostream>
using namespace std;

t_bool IsPrefix(t_nubit8 opcode)
{
	switch(opcode) {
	case 0xf0: case 0xf2: case 0xf3:
	case 0x2e: case 0x36: case 0x3e: case 0x26:
	//case 0x64: case 0x65: case 0x66: case 0x67:
				return 0x01;break;
	default:	return 0x00;break;
	}
}
void  ClrPrefix()
{
	lcpu.deCodeBlock.prefix_SegmentOverrideOpCode = 0;//消除prefix的影响，防止影响下一条指令
	lcpu.deCodeBlock.prefix_LockOpCode = 0;
	lcpu.deCodeBlock.prefix_RepeatOpCode = 0;
}
DeCodeBlock::DeCodeBlock(CentreProcessorUnit* pCPU)
{
	this->pCPU=pCPU;
	this->pexeCodeBlock=&(pCPU->exeCodeBlock);
}
static int count =0;
const unsigned int MIN_COUNT=0xffff*38;
const unsigned int MAX_COUNT=MIN_COUNT+0xffff;


void DeCodeBlock::refresh()
{
	this->pOpCode = p_nubit8(vramGetAddr(_cs, _ip));
	this->prefix_SegmentOverrideOpCode = 0;//消除prefix的影响，防止影响下一条指令
	this->prefix_LockOpCode = 0;
	this->prefix_RepeatOpCode = 0;
	//34 位那些prefix暂时不处理
}
void DeCodeBlock::setExecuteMethod()
{
	pexeCodeBlock->setNeedExeMethod( pexeCodeBlock->exeMethodArray[*pOpCode] );
}
void DeCodeBlock::deCodeInstruction()
{
	//refresh();
	t_nubit8 opcode = vramVarByte(lcpu.cs, lcpu.ip);
	this->pOpCode = p_nubit8(vramGetAddr(lcpu.cs, lcpu.ip));

	setExecuteMethod();//设定执行的指令
	deCodeInsStuff();//翻译后续操作码
	((*(this->pexeCodeBlock)).*(this->pexeCodeBlock->needExeMethod))();

	if(!IsPrefix(opcode)) ClrPrefix();

//	if (GetBit(_flags, VCPU_FLAG_IF) && vpicScanINTR())
//			this->pexeCodeBlock->atomMethod_INT(vpicGetINTR());	
/* end point of record */
}
void DeCodeBlock::deCodeInsStuff()
{
	t_nubit8 tmpOpCode ;
//LOOP_Decode:
/*  start point of record */
	switch(*pOpCode)
	{
	case MASK_00110111://AAA
	case MASK_00111111://AAS
	case MASK_10011000://CBW
	case MASK_11111000://CLC
	case MASK_11111100://CLD
	case MASK_11111010://CLI
	case MASK_11110101://CMC
	case MASK_10100110://CMPSB
	case MASK_10100111://CMPSW
	case MASK_10011001://CWD
	case MASK_00100111://DAA
	case MASK_00101111://DAS
	case MASK_01001000://DEC AX    <DEC>
	case MASK_01001001://DEC CX
	case MASK_01001010://DEC DX
	case MASK_01001011://DEC BX
	case MASK_01001100://DEC SP
	case MASK_01001101://DEC BP
	case MASK_01001110://DEC SI
	case MASK_01001111://DEC DI    </DEC>
	case MASK_11110100://HLT
	case MASK_11101100:// IN ,w=0
	case MASK_11101101:// IN, w=1
	case MASK_01000000:// INC AX
	case MASK_01000001:// INC CX
	case MASK_01000010:// INC DX
	case MASK_01000011:// INC BX
	case MASK_01000100:// INC SP
	case MASK_01000101:// INC BP
	case MASK_01000110:// INC SI
	case MASK_01000111:// INC DI
	case MASK_11001100:// INT ,v=0 没有后面数据
	case MASK_11001110://INTO
	case MASK_11001111://IRET
	case MASK_10011111://LAHF
	case MASK_10101100://LODSB ,w=0
	case MASK_10101101://LODSW, w=1
	case MASK_10100100://MOVSB, w=0
	case MASK_10100101://MOVSW, w=1
	case MASK_10010000://NOP
	case MASK_11101110://OUT port=al, w=0
	case MASK_11101111://OUT port=ax, w=1

	case 0x50://PUSH AX
	case 0x51://PUSH CX
	case 0x52://PUSH DX
	case 0x53://PUSH BX
	case 0x54://PUSH SP
	case 0x55://PUSH BP
	case 0x56://PUSH SI
	case 0x57://PUSH DI
	case 0x58://POP AX
	case 0x59://POP CX
	case 0x5A://POP DX
	case 0x5B://POP BX
	case 0x5C://POP SP
	case 0x5D://POP BP
	case 0x5E://POP SI
	case 0x5F://POP DI
	case MASK_00000111://POP ES, 没有POP CS，
	case MASK_00010111://POP SS
	case MASK_00011111://POP DS
	case MASK_00000110://PUSH ES
	case MASK_00001110://PUSH CS
	case MASK_00010110://PUSH SS
	case MASK_00011110://PUSH DS

	//case MASK_11110010://REP ,z=0
	//case MASK_11110011://REP ,z=1
	case MASK_10011101://POPF
	case MASK_10011100://PUSHF
	case MASK_11000011://RET
	case MASK_11001011://RET
	case MASK_10011110://SAHF
	case MASK_10101110://SCASB , w=0
	case MASK_10101111://SCASW,  w=1
	case MASK_11111001://STC 
	case MASK_11111101://STD
	case MASK_11111011://STI
	case MASK_10101010://STOSB , w=0
	case MASK_10101011://STOSW , w=1
	case MASK_10011011://WAIT
	//case MASK_10010000://XCHG AX,AX  不存在
	case MASK_10010001://XCHG CX
	case MASK_10010010://XCHG DX
	case MASK_10010011://XCHG BX
	case MASK_10010100://XCHG SP
	case MASK_10010101://XCHG BP
	case MASK_10010110://XCHG SI
	case MASK_10010111://XCHG DI
	case MASK_11010111://XLAT
		this->deOpCode_singleDefine();
		break;
	case MASK_11010101://AAD
	case MASK_11010100://AAM
	case MASK_11011000://ESC
	case MASK_11011001:
	case MASK_11011010:
	case MASK_11011011:
	case MASK_11011100:
	case MASK_11011101:
	case MASK_11011110:
	case MASK_11011111:
		this->deOpCode_doubleDefine();
		break;
	case MASK_00010000://ADC
	case MASK_00010001:
	case MASK_00010010:
	case MASK_00010011:
	case MASK_00000000://ADD
	case MASK_00000001:
	case MASK_00000010:
	case MASK_00000011:
	case MASK_00100000://AND
	case MASK_00100001:
	case MASK_00100010:
	case MASK_00100011:
	case MASK_00111000://CMP
	case MASK_00111001:
	case MASK_00111010:
	case MASK_00111011:
	case MASK_11000101://LDS
	case MASK_10001101://LEA
	case MASK_00001000://OR
	case MASK_00001001:
	case MASK_00001010:
	case MASK_00001011:
	case MASK_10001111://POP Ev
	case MASK_00011000://SBB
	case MASK_00011001:
	case MASK_00011010:
	case MASK_00011011:
	case MASK_00101000://SUB
	case MASK_00101001:
	case MASK_00101010:
	case MASK_00101011:
	case MASK_10000100://TEST,因为方向基本没用，所以只有两个
	case MASK_10000101:
	case MASK_10000110://XCHG,同上
	case MASK_10000111:
	case MASK_00110000://XOR
	case MASK_00110001:
	case MASK_00110010:
	case MASK_00110011:
		this->deOpCode_modRegRm();
		break;
	case MASK_11000100://LES
		tmpOpCode = *pOpCode;
		*pOpCode = MASK_11000101;//由于LES的opcode的最后w位是0，所以必须换成1，改成LDS
		this->deOpCode_modRegRm();
		*pOpCode = tmpOpCode;
		break;
	case MASK_00010100://ADC
	case MASK_00010101:
	case MASK_00000100://ADD
	case MASK_00000101:
	case MASK_00100100://AND
	case MASK_00100101:
	case MASK_00111100://CMP
	case MASK_00111101:
	case MASK_00001100://OR
	case MASK_00001101:
	case MASK_00011100://SBB
	case MASK_00011101:
	case MASK_00101100://SUB
	case MASK_00101101:
	case MASK_10101000://TEST
	case MASK_10101001:
	case MASK_00110100://XOR
	case MASK_00110101:
		this->deOpCode_IMM();
	    break;

	case MASK_10000000://Immdiate group
	case MASK_10000001:
	case MASK_10000010:
	case MASK_10000011:
		this->deOpCode_IMMGroup();
		break;
	
	case MASK_11110110://group3
	case MASK_11110111:
		if(0 == (*(pOpCode +1)&MASK_00111000) )//TEST Ib/Iv
			this->deOpCode_TEST_Ib_Iv();
		else
			this->deOpCode_ModRM();
		break;
	case MASK_11010000://group 2
	case MASK_11010001:
	case MASK_11010010:
	case MASK_11010011:
	case MASK_11111110://group4
	case MASK_11111111:
		this->deOpCode_ModRM();
		break;
	case MASK_11100100://IN port w=0
	case MASK_11100101://IN port w=1
	case MASK_11001101://INT type v=1
	case MASK_11101011://JMP 段内直接转移二
	case 0x70://JO
	case 0x71://JNO
	case 0x72://JB/JNAE/JC
	case 0x73://JNB/JAE/JNC
	case 0x74://JZ
	case 0x75://JNZ
	case 0x76://JBE
	case 0x77://JNBE
	case 0x78://JS
	case 0x79://JNS
	case 0x7A://JP
	case 0x7B://JNP
	case 0x7C://JL
	case 0x7D://JNL
	case 0x7E://JLE
	case 0x7F://JNLE
	case 0xe3://JCXZ
	case MASK_11100001://LOOPE/lOOPZ
	case MASK_11100000://LOOPNE/LOOPNZ
	case MASK_11100010://LOOP
	case MASK_11100110://OUT port w=0
	case MASK_11100111://OUT port w=1
		this->deOpCode_contactData_8bit();
		break;
	case MASK_11101000://CALL---段内直接调用
	case MASK_11101001://JMP--段内直接转移
	case MASK_11000010://RET--带立即数的段内返回
	case MASK_11001010://RET--带立即数的段间返回
		this->deOpCode_contactData_16bit();
		break;
	case MASK_10011010://CALL--段间之间调用
	case MASK_11101010://JMP--段间直接转移
		this->deOpCode_contactData_32bit();
		break;
	case MASK_11000110://MOV--立即数到 存储器/寄存器
	case MASK_11000111:
		this->deOpCode_Mov_imm_rm();
		break;
	case MASK_10001110://MOV--存储器，寄存器
	case MASK_10001100:
		this->deOpCode_Mov_rm_seg();
		break;
	case MASK_10001000://MOV--
	case MASK_10001001:
	case MASK_10001010:
	case MASK_10001011:
		this->deOpCode_modRegRm();
		break;
	case MASK_10100000://MOV-- 累加器，存储器
	case MASK_10100001:
	case MASK_10100010:
	case MASK_10100011:
		this->deOpCode_Mov_mem_AX();
		break;
	case MASK_10110000://MOV--立即数到寄存器
	case MASK_10110001:
	case MASK_10110010:
	case MASK_10110011:
	case MASK_10110100:
	case MASK_10110101:
	case MASK_10110110:
	case MASK_10110111:
	case MASK_10111000:
	case MASK_10111001:
	case MASK_10111010:
	case MASK_10111011:
	case MASK_10111100:
	case MASK_10111101:
	case MASK_10111110:
	case MASK_10111111:
		this->deOpCode_Mov_imm_reg();
		break;
	case 0xf0:
	case 0xf2:
	case 0xf3:
	case 0x2e:
	case 0x36:
	case 0x3e:
	case 0x26:
		this->deOpCode_singleDefine();
		break;
	default:
		char _str[5];
		sprintf(_str,"%X",*pOpCode);
		showMessage("deCodeInsStuff Unhandle a OpCode " + string(_str));
		assert(false);
		break;
	}
}

void   DeCodeBlock::init() {refresh();}
void * DeCodeBlock::deCodeEffectiveMemAddr(const t_nubit8 modRegRm, const void * pdispData, int* pdispLength)
{
	*pdispLength = 0;
	t_nubit32 memAddr = 0;
	t_nubit8 mod = (modRegRm & MASK_11000000)>>6;
	t_nubit16 tmpDS = getDefaultSegment_DS();
	t_nubit16 tmpSS = getDefaultSegment_SS();
	switch(modRegRm & MASK_00000111)
	{
	case 0://RM=000
		memAddr =   pCPU->bx + pCPU->si + ((segData = tmpDS) <<4 );
		break;
	case 1://RM=001
		memAddr =   pCPU->bx + pCPU->di + ((segData = tmpDS) <<4 );
		break;
	case 2://RM=010
		memAddr =   pCPU->bp + pCPU->si + ((segData = tmpSS) <<4);
		break;
	case 3://RM=011
		memAddr =   pCPU->bp + pCPU->di + ((segData = tmpSS) <<4);
		break;
	case 4://RM=100
		memAddr =   pCPU->si + ((segData = tmpDS) <<4);
		break;
	case 5://RM=101
		memAddr =   pCPU->di + ((segData = tmpDS) <<4);
		break;
	case 6://RM=110
		if( 0 == mod ) //MOD=00
		{
			memAddr = d_nubit16(pdispData) + ((segData = tmpDS) <<4);
			*pdispLength=2;
		}
		else
			memAddr = pCPU->bp + ((segData = tmpSS) <<4);
		break;
	case 7://RM=111
		memAddr =   pCPU->bx + ((segData = tmpDS) <<4);
		break;
	default://谁知道呢，，，可能宇宙射线
		showMessage("deCodeEffectiveMemAddr UnHandle the rm bits");
		assert(false);
		return 0;
	}
	
	switch(mod)
	{
	case 0://MOD=00
		break;
	case 1://MOD=01
		memAddr += *(t_nsbit8*)pdispData;
		*pdispLength = 1;
		break;
	case 2://MOD=10
		memAddr += d_nubit16(pdispData);
		*pdispLength = 2;
		break;
	case 3://MOD=11  取得的是reg的地址，直接返回即可
		return (void *)getRegAddr(this->isWidth, (modRegRm & MASK_00000111) );
	default:
		showMessage("deCodeEffectiveMemAddr UnHandle the mod bits");
		assert(false);
		return 0;
	}
	return (void *)(p_nubit8(vramGetAddr(0,0)) + memAddr);

}
void * DeCodeBlock::getRegAddr(const bool w, const t_nubit8 reg) const
{// 表示是否是字
	if(false==w)
	{
		switch( reg)
		{
		case 0://REG=000
			return (void *)(&pCPU->al);
		case 1://REG=001
			return (void *)(&pCPU->cl);
		case 2://REG=010
			return (void *)(&pCPU->dl);
		case 3://REG=011
			return (void *)(&pCPU->bl);
		case 4://REG=100
			return (void *)(&pCPU->ah);
		case 5://REG=101
			return (void *)(&pCPU->ch);
		case 6://REG=110
			return (void *)(&pCPU->dh);
		case 7://REG=111
			return (void *)(&pCPU->bh);
		default://谁知道呢。。哎
			return NULL;
		}

	}
	else
	{
		switch( reg)
		{
		case 0://REG=000
			return (void *)(&pCPU->ax);
		case 1://REG=001
			return (void *)(&pCPU->cx);
		case 2://REG=010
			return (void *)(&pCPU->dx);
		case 3://REG=011
			return (void *)(&pCPU->bx);
		case 4://REG=100
			return (void *)(&pCPU->sp);
		case 5://REG=101
			return (void *)(&pCPU->bp);
		case 6://REG=110
			return (void *)(&pCPU->si);
		case 7://REG=111
			return (void *)(&pCPU->di);
		default://谁知道呢。。哎
			return NULL;
		}

	}
}

t_nubit16* DeCodeBlock::getSegAddr(const t_nubit8 sreg) const
{
	switch( sreg )
		{//2-Bit sreg2 Field
		case 0://REG=00
			return (t_nubit16*)(&pCPU->es);
		case 1://REG=01
			return (t_nubit16*)(&pCPU->cs);
		case 2://REG=10
			return (t_nubit16*)(&pCPU->ss);
		case 3://REG=11
			return (t_nubit16*)(&pCPU->ds);
		default://谁知道呢。。哎
			return NULL;
		}
}

void DeCodeBlock::deOpCode_singleDefine()
{
	this->pCPU->ip += sizeof(t_nubit8);
}
void DeCodeBlock::deOpCode_doubleDefine()
{
	this->pCPU->ip += sizeof(t_nubit16);
}
int  DeCodeBlock::deOpCode_modRegRm()
{
	int insDispLength = 0;
	int insLength = OPCODE_WIDTH + MOD_REG_RM_LENGTH;
	t_nubit8 * pModRegRm = pOpCode+OPCODE_WIDTH;
	t_nubit8 * pDisp = pModRegRm + MOD_REG_RM_LENGTH;
	this->isWidth = !!(*pOpCode & MASK_00000001);
	prm = deCodeEffectiveMemAddr(*pModRegRm,(void *)pDisp, &insDispLength );
	preg = getRegAddr(isWidth, ((*pModRegRm)&MASK_00111000)>>3 );
	insLength +=  insDispLength;
	this->pCPU->ip +=  insLength ;//更新cpu中的ip，令其指向下一条指
	return insLength;
}
void DeCodeBlock::deOpCode_IMM()
{
	int insLength = OPCODE_WIDTH ;
	t_nubit8 * pimmData = this->pOpCode + OPCODE_WIDTH;
	bool w_width = !!((*pOpCode)&MASK_00000001);
	if(w_width)
	{
		this->immData_16Bit = d_nubit16(pimmData);
		insLength += 2;
	}
	else
	{
		this->immData_8Bit = *(pimmData);
		insLength++;
	}
	this->pCPU->ip += insLength ;//更新cpu中的ip，令其指向下一条指令

}
void DeCodeBlock::deOpCode_TEST_Ib_Iv()
{
	int insLength = this->deOpCode_modRegRm();//cpu.ip已经变化
	//抄deOpCode_IMMGroup， 但是不检测s位
	switch( (*pOpCode)&MASK_00000001 )
	{
	case MASK_00000001:
		this->immData_16Bit =  d_nubit16(pOpCode + insLength);
		this->pCPU->ip += 2;
		break;
	default:
		this->immData_8Bit =  *(pOpCode + insLength);
		this->pCPU->ip++;
	}
	this->nnn = (*(pOpCode + OPCODE_WIDTH) & MASK_00111000)>>3 ; //迟绑定，到执行的时候再确定到底该执行什么函数
	
	return ;
}
void DeCodeBlock::deOpCode_IMMGroup()
{//未完成品

	int insLength = this->deOpCode_modRegRm();//cpu.ip已经变化
	
	switch( (*pOpCode)&MASK_00000011 )
	{//s位的处理
	case MASK_00000001:
		this->immData_16Bit =  d_nubit16(pOpCode + insLength);
		this->pCPU->ip += 2;
		break;
	case MASK_00000011:
		this->immData_16Bit =  (t_nsbit16)(d_nsbit8(pOpCode + insLength));
		this->pCPU->ip += 1;
		break;
	default:
		this->immData_8Bit =  *(pOpCode + insLength);
		this->pCPU->ip++;
	}
	this->nnn = (*(pOpCode + OPCODE_WIDTH) & MASK_00111000)>>3 ; //迟绑定，到执行的时候再确定到底该执行什么函数
	
	return ;

}
void DeCodeBlock::deOpCode_contactData_8bit()
{
	int insLength = OPCODE_WIDTH + sizeof(t_nubit8);
	this->opContactData_8bit = d_nubit8(pOpCode + OPCODE_WIDTH);
	this->pCPU->ip += insLength;
	return;
}
void DeCodeBlock::deOpCode_contactData_16bit()
{
	int insLength = OPCODE_WIDTH + sizeof(t_nubit16);//已经确定指令长度
	this->opContactData_16bit = d_nubit16(pOpCode + OPCODE_WIDTH);
	this->pCPU->ip += insLength ;//更新cpu中的ip，令其指向下一条指令
	return ;
}
void DeCodeBlock::deOpCode_contactData_32bit()
{
	int insLength = OPCODE_WIDTH + sizeof(t_nubit32);//已经确定指令长度
	this->opContactData_16bit = d_nubit16(pOpCode + OPCODE_WIDTH);
	this->opContactData_16bitE = d_nubit16(pOpCode + OPCODE_WIDTH +sizeof(t_nubit16));
	this->pCPU->ip += insLength ;//更新cpu中的ip，令其指向下一条指令
	return;
}
void DeCodeBlock::deOpCode_Mov_mem_AX()
{
	int insLength = OPCODE_WIDTH + sizeof(t_nubit16);//已经确定指令长度
	int nothing=0;
	this->isWidth = !!(*pOpCode & MASK_00000001);
	this->prm = deCodeEffectiveMemAddr(MASK_00000110, pOpCode+OPCODE_WIDTH, &nothing );
	this->pCPU->ip += insLength ;//更新cpu中的ip，令其指向下一条指令
	return ;

}
void DeCodeBlock::deOpCode_Mov_imm_rm()
{

	int insLength = this->deOpCode_modRegRm();//cpu.ip已经变化
	
	if(isWidth)
	{
		this->immData_16Bit =  d_nubit16(pOpCode + insLength);
		this->pCPU->ip += 2;
	}
	else
	{
		this->immData_8Bit =  *(pOpCode + insLength);
		this->pCPU->ip++;
	}
	return ;
	
}
void DeCodeBlock::deOpCode_Mov_rm_seg()
{
	int insDispLength = 0;
	int insLength = OPCODE_WIDTH + MOD_REG_RM_LENGTH;
	t_nubit8 * pModRegRm = pOpCode+OPCODE_WIDTH;
	t_nubit8 * pDisp = pModRegRm + MOD_REG_RM_LENGTH;
	this->isWidth = true; //一定要是真
	prm = deCodeEffectiveMemAddr(*pModRegRm,(void *)pDisp, &insDispLength );
	pseg = getSegAddr( ((*pModRegRm)&MASK_00011000)>>3 ); //segment...只需要两个bit就可以确定																//就这个和deOpCodeModRegRm（）不一
	insLength += insDispLength;
	this->pCPU->ip += insLength ;//更新cpu中的ip，令其指向下一条指令
}
void DeCodeBlock::deOpCode_Mov_imm_reg()
{ //reg 是哪个会在执行时候确定
	int insLength = OPCODE_WIDTH + sizeof(t_nubit8); //至少有这么多
	this->isWidth = !!((*pOpCode) & MASK_00001000);
	if(isWidth)
	{
		this->immData_16Bit = d_nubit16(pOpCode + OPCODE_WIDTH);
		insLength++;
	}
	else
	{
		this->immData_8Bit = d_nubit8(pOpCode + OPCODE_WIDTH);
	}
	this->pCPU->ip += insLength ;//更新cpu中的ip，令其指向下一条指针
}
void DeCodeBlock::deOpCode_ModRM()
{
	int insLength = this->deOpCode_modRegRm();//cpu.ip已经变化
	this->nnn = (*(pOpCode + OPCODE_WIDTH) & MASK_00111000)>>3 ; //迟绑定，到执行的时候再确定到底该执行什么函数
	return ;
}
t_nubit16 DeCodeBlock::getDefaultSegment_DS()
{
	switch(this->prefix_SegmentOverrideOpCode)
	{
	case CS_SEGMENT_OVERRIDE_PREFIX:
		return pCPU->cs;
	case SS_SEGMENT_OVERRIDE_PREFIX:
		return pCPU->ss;
	case ES_SEGMENT_OVERRIDE_PREFIX:
		return pCPU->es;
	default:
		return pCPU->ds;
	}
}
t_nubit16 DeCodeBlock::getDefaultSegment_SS()
{
	switch(this->prefix_SegmentOverrideOpCode)
	{
	case CS_SEGMENT_OVERRIDE_PREFIX:
		return pCPU->cs;
	case DS_SEGMENT_OVERRIDE_PREFIX:
		return pCPU->ds;
	case ES_SEGMENT_OVERRIDE_PREFIX:
		return pCPU->es;
	default:
		return pCPU->ss;
	}
}