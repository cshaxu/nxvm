
#include "../vapi.h"
#include "../vmachine.h"
#include "ccpuapi.h"

static t_bool IsPrefix(t_nubit8 opcode)
{
	switch(opcode) {
	case 0xf0: case 0xf2: case 0xf3:
	case 0x2e: case 0x36: case 0x3e: case 0x26:
	//case 0x64: case 0x65: case 0x66: case 0x67:
				return 0x01;break;
	default:	return 0x00;break;
	}
}
static void  ClrPrefix()
{
	ccpu.deCodeBlock.prefix_SegmentOverrideOpCode = 0;//消除prefix的影响，防止影响下一条指令
	ccpu.deCodeBlock.prefix_LockOpCode = 0;
	ccpu.deCodeBlock.prefix_RepeatOpCode = 0;
}

void decode_setExecuteMethod() {ccpu.exeCodeBlock.needExeMethod = ccpu.exeCodeBlock.exeMethodArray[*ccpu.deCodeBlock.pOpCode];}
void decode_deCodeInstruction()
{
	t_nubit8 opcode = cramVarByte(ccpu.cs, ccpu.ip);
	ccpu.deCodeBlock.pOpCode = p_nubit8(cramGetAddr(ccpu.cs, ccpu.ip));
	decode_setExecuteMethod();
	decode_deCodeInsStuff();
	(*(ccpu.exeCodeBlock.needExeMethod))();
	if(!IsPrefix(opcode)) ClrPrefix();
}
void decode_deCodeInsStuff()
{
	t_nubit8 tmpOpCode ;
	switch(*ccpu.deCodeBlock.pOpCode) {
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
		decode_deOpCode_singleDefine();
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
		decode_deOpCode_doubleDefine();
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
		decode_deOpCode_modRegRm();
		break;
	case MASK_11000100://LES
		tmpOpCode = *ccpu.deCodeBlock.pOpCode;
		*ccpu.deCodeBlock.pOpCode = MASK_11000101;//由于LES的opcode的最后w位是0，所以必须换成1，改成LDS
		decode_deOpCode_modRegRm();
		*ccpu.deCodeBlock.pOpCode = tmpOpCode;
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
		decode_deOpCode_IMM();
	    break;

	case MASK_10000000://Immdiate group
	case MASK_10000001:
	case MASK_10000010:
	case MASK_10000011:
		decode_deOpCode_IMMGroup();
		break;
	
	case MASK_11110110://group3
	case MASK_11110111:
		if(0 == (*(ccpu.deCodeBlock.pOpCode +1)&MASK_00111000) )//TEST Ib/Iv
			decode_deOpCode_TEST_Ib_Iv();
		else
			decode_deOpCode_ModRM();
		break;
	case MASK_11010000://group 2
	case MASK_11010001:
	case MASK_11010010:
	case MASK_11010011:
	case MASK_11111110://group4
	case MASK_11111111:
		decode_deOpCode_ModRM();
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
		decode_deOpCode_contactData_8bit();
		break;
	case MASK_11101000://CALL---段内直接调用
	case MASK_11101001://JMP--段内直接转移
	case MASK_11000010://RET--带立即数的段内返回
	case MASK_11001010://RET--带立即数的段间返回
		decode_deOpCode_contactData_16bit();
		break;
	case MASK_10011010://CALL--段间之间调用
	case MASK_11101010://JMP--段间直接转移
		decode_deOpCode_contactData_32bit();
		break;
	case MASK_11000110://MOV--立即数到 存储器/寄存器
	case MASK_11000111:
		decode_deOpCode_Mov_imm_rm();
		break;
	case MASK_10001110://MOV--存储器，寄存器
	case MASK_10001100:
		decode_deOpCode_Mov_rm_seg();
		break;
	case MASK_10001000://MOV--
	case MASK_10001001:
	case MASK_10001010:
	case MASK_10001011:
		decode_deOpCode_modRegRm();
		break;
	case MASK_10100000://MOV-- 累加器，存储器
	case MASK_10100001:
	case MASK_10100010:
	case MASK_10100011:
		decode_deOpCode_Mov_mem_AX();
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
		decode_deOpCode_Mov_imm_reg();
		break;
	case 0xf0:
	case 0xf2:
	case 0xf3:
	case 0x2e:
	case 0x36:
	case 0x3e:
	case 0x26:
		decode_deOpCode_singleDefine();
		break;
	default:
		vapiPrint("deCodeInsStuff Unhandle a OpCode %x\n", *ccpu.deCodeBlock.pOpCode);
		assert(0x00);
		break;
	}
}
void * decode_deCodeEffectiveMemAddr(const t_nubit8 modRegRm, const void * pdispData, int* pdispLength)
{
	t_nubit32 memAddr = 0;
	t_nubit8 mod = (modRegRm & MASK_11000000)>>6;
	t_nubit16 tmpDS = decode_getDefaultSegment_DS();
	t_nubit16 tmpSS = decode_getDefaultSegment_SS();
	*pdispLength = 0;
	switch(modRegRm & MASK_00000111)
	{
	case 0://RM=000
		memAddr =   ccpu.bx + ccpu.si + ((ccpu.deCodeBlock.segData = tmpDS) <<4 );
		break;
	case 1://RM=001
		memAddr =   ccpu.bx + ccpu.di + ((ccpu.deCodeBlock.segData = tmpDS) <<4 );
		break;
	case 2://RM=010
		memAddr =   ccpu.bp + ccpu.si + ((ccpu.deCodeBlock.segData = tmpSS) <<4);
		break;
	case 3://RM=011
		memAddr =   ccpu.bp + ccpu.di + ((ccpu.deCodeBlock.segData = tmpSS) <<4);
		break;
	case 4://RM=100
		memAddr =   ccpu.si + ((ccpu.deCodeBlock.segData = tmpDS) <<4);
		break;
	case 5://RM=101
		memAddr =   ccpu.di + ((ccpu.deCodeBlock.segData = tmpDS) <<4);
		break;
	case 6://RM=110
		if( 0 == mod ) //MOD=00
		{
			memAddr = d_nubit16(pdispData) + ((ccpu.deCodeBlock.segData = tmpDS) <<4);
			*pdispLength=2;
		}
		else
			memAddr = ccpu.bp + ((ccpu.deCodeBlock.segData = tmpSS) <<4);
		break;
	case 7://RM=111
		memAddr =   ccpu.bx + ((ccpu.deCodeBlock.segData = tmpDS) <<4);
		break;
	default://谁知道呢，，，可能宇宙射线
		vapiPrint("deCodeEffectiveMemAddr UnHandle the rm bits");
		assert(0x00);
		return 0;
	}
	
	switch(mod)
	{
	case 0://MOD=00
		break;
	case 1://MOD=01
		memAddr += d_nsbit8(pdispData);
		*pdispLength = 1;
		break;
	case 2://MOD=10
		memAddr += d_nubit16(pdispData);
		*pdispLength = 2;
		break;
	case 3://MOD=11  取得的是reg的地址，直接返回即可
		return (void *)decode_getRegAddr(ccpu.deCodeBlock.isWidth, (modRegRm & MASK_00000111) );
	default:
		vapiPrint("deCodeEffectiveMemAddr UnHandle the mod bits");
		assert(0x00);
		return 0;
	}
	return (void *)(p_nubit8(cramGetAddr(0,0)) + memAddr);

}
void* decode_getRegAddr(const t_bool w, const t_nubit8 reg)
{// 表示是否是字
	if(0x00==w)
	{
		switch( reg)
		{
		case 0://REG=000
			return (void *)(&ccpu.al);
		case 1://REG=001
			return (void *)(&ccpu.cl);
		case 2://REG=010
			return (void *)(&ccpu.dl);
		case 3://REG=011
			return (void *)(&ccpu.bl);
		case 4://REG=100
			return (void *)(&ccpu.ah);
		case 5://REG=101
			return (void *)(&ccpu.ch);
		case 6://REG=110
			return (void *)(&ccpu.dh);
		case 7://REG=111
			return (void *)(&ccpu.bh);
		default://谁知道呢。。哎
			return NULL;
		}

	}
	else
	{
		switch( reg)
		{
		case 0://REG=000
			return (void *)(&ccpu.ax);
		case 1://REG=001
			return (void *)(&ccpu.cx);
		case 2://REG=010
			return (void *)(&ccpu.dx);
		case 3://REG=011
			return (void *)(&ccpu.bx);
		case 4://REG=100
			return (void *)(&ccpu.sp);
		case 5://REG=101
			return (void *)(&ccpu.bp);
		case 6://REG=110
			return (void *)(&ccpu.si);
		case 7://REG=111
			return (void *)(&ccpu.di);
		default://谁知道呢。。哎
			return NULL;
		}

	}
}
t_nubit16* decode_getSegAddr(const t_nubit8 sreg)
{
	switch( sreg )
		{//2-Bit sreg2 Field
		case 0://REG=00
			return (t_nubit16*)(&ccpu.es);
		case 1://REG=01
			return (t_nubit16*)(&ccpu.cs);
		case 2://REG=10
			return (t_nubit16*)(&ccpu.ss);
		case 3://REG=11
			return (t_nubit16*)(&ccpu.ds);
		default://谁知道呢。。哎
			return NULL;
		}
}
void decode_deOpCode_singleDefine() {ccpu.ip += sizeof(t_nubit8);}
void decode_deOpCode_doubleDefine() {ccpu.ip += sizeof(t_nubit16);}
int  decode_deOpCode_modRegRm()
{
	int insDispLength = 0;
	int insLength = OPCODE_WIDTH + MOD_REG_RM_LENGTH;
	t_nubit8 * pModRegRm = ccpu.deCodeBlock.pOpCode+OPCODE_WIDTH;
	t_nubit8 * pDisp = pModRegRm + MOD_REG_RM_LENGTH;
	ccpu.deCodeBlock.isWidth = !!(*ccpu.deCodeBlock.pOpCode & MASK_00000001);
	ccpu.deCodeBlock.prm = decode_deCodeEffectiveMemAddr(*pModRegRm,(void *)pDisp, &insDispLength );
	ccpu.deCodeBlock.preg = decode_getRegAddr(ccpu.deCodeBlock.isWidth, ((*pModRegRm)&MASK_00111000)>>3 );
	insLength +=  insDispLength;
	ccpu.ip +=  insLength ;//更新cpu中的ip，令其指向下一条指
	return insLength;
}
void decode_deOpCode_IMM()
{
	int insLength = OPCODE_WIDTH ;
	t_nubit8 * pimmData = ccpu.deCodeBlock.pOpCode + OPCODE_WIDTH;
	t_bool w_width = !!((*ccpu.deCodeBlock.pOpCode)&MASK_00000001);
	if(w_width)
	{
		ccpu.deCodeBlock.immData_16Bit = d_nubit16(pimmData);
		insLength += 2;
	}
	else
	{
		ccpu.deCodeBlock.immData_8Bit = *(pimmData);
		insLength++;
	}
	ccpu.ip += insLength ;//更新cpu中的ip，令其指向下一条指令

}
void decode_deOpCode_TEST_Ib_Iv()
{
	int insLength = decode_deOpCode_modRegRm();//cpu.ip已经变化
	//抄deOpCode_IMMGroup， 但是不检测s位
	switch( (*ccpu.deCodeBlock.pOpCode)&MASK_00000001 )
	{
	case MASK_00000001:
		ccpu.deCodeBlock.immData_16Bit =  d_nubit16(ccpu.deCodeBlock.pOpCode + insLength);
		ccpu.ip += 2;
		break;
	default:
		ccpu.deCodeBlock.immData_8Bit =  *(ccpu.deCodeBlock.pOpCode + insLength);
		ccpu.ip++;
	}
	ccpu.deCodeBlock.nnn = (*(ccpu.deCodeBlock.pOpCode + OPCODE_WIDTH) & MASK_00111000)>>3 ; //迟绑定，到执行的时候再确定到底该执行什么函数
	
	return ;
}
void decode_deOpCode_IMMGroup()
{//未完成品

	int insLength = decode_deOpCode_modRegRm();//cpu.ip已经变化
	
	switch( (*ccpu.deCodeBlock.pOpCode)&MASK_00000011 )
	{//s位的处理
	case MASK_00000001:
		ccpu.deCodeBlock.immData_16Bit =  d_nubit16(ccpu.deCodeBlock.pOpCode + insLength);
		ccpu.ip += 2;
		break;
	case MASK_00000011:
		ccpu.deCodeBlock.immData_16Bit =  (t_nsbit16)(d_nsbit8(ccpu.deCodeBlock.pOpCode + insLength));
		ccpu.ip += 1;
		break;
	default:
		ccpu.deCodeBlock.immData_8Bit =  *(ccpu.deCodeBlock.pOpCode + insLength);
		ccpu.ip++;
	}
	ccpu.deCodeBlock.nnn = (*(ccpu.deCodeBlock.pOpCode + OPCODE_WIDTH) & MASK_00111000)>>3 ; //迟绑定，到执行的时候再确定到底该执行什么函数
	
	return ;

}
void decode_deOpCode_contactData_8bit()
{
	int insLength = OPCODE_WIDTH + sizeof(t_nubit8);
	ccpu.deCodeBlock.opContactData_8bit = d_nubit8(ccpu.deCodeBlock.pOpCode + OPCODE_WIDTH);
	ccpu.ip += insLength;
	return;
}
void decode_deOpCode_contactData_16bit()
{
	int insLength = OPCODE_WIDTH + sizeof(t_nubit16);//已经确定指令长度
	ccpu.deCodeBlock.opContactData_16bit = d_nubit16(ccpu.deCodeBlock.pOpCode + OPCODE_WIDTH);
	ccpu.ip += insLength ;//更新cpu中的ip，令其指向下一条指令
	return ;
}
void decode_deOpCode_contactData_32bit()
{
	int insLength = OPCODE_WIDTH + sizeof(t_nubit32);//已经确定指令长度
	ccpu.deCodeBlock.opContactData_16bit = d_nubit16(ccpu.deCodeBlock.pOpCode + OPCODE_WIDTH);
	ccpu.deCodeBlock.opContactData_16bitE = d_nubit16(ccpu.deCodeBlock.pOpCode + OPCODE_WIDTH +sizeof(t_nubit16));
	ccpu.ip += insLength ;//更新cpu中的ip，令其指向下一条指令
	return;
}
void decode_deOpCode_Mov_mem_AX()
{
	int insLength = OPCODE_WIDTH + sizeof(t_nubit16);//已经确定指令长度
	int nothing=0;
	ccpu.deCodeBlock.isWidth = !!(*ccpu.deCodeBlock.pOpCode & MASK_00000001);
	ccpu.deCodeBlock.prm = decode_deCodeEffectiveMemAddr(MASK_00000110, ccpu.deCodeBlock.pOpCode+OPCODE_WIDTH, &nothing );
	ccpu.ip += insLength ;//更新cpu中的ip，令其指向下一条指令
	return ;

}
void decode_deOpCode_Mov_imm_rm()
{

	int insLength = decode_deOpCode_modRegRm();//cpu.ip已经变化
	
	if(ccpu.deCodeBlock.isWidth)
	{
		ccpu.deCodeBlock.immData_16Bit =  d_nubit16(ccpu.deCodeBlock.pOpCode + insLength);
		ccpu.ip += 2;
	}
	else
	{
		ccpu.deCodeBlock.immData_8Bit =  *(ccpu.deCodeBlock.pOpCode + insLength);
		ccpu.ip++;
	}
	return ;
	
}
void decode_deOpCode_Mov_rm_seg()
{
	int insDispLength = 0;
	int insLength = OPCODE_WIDTH + MOD_REG_RM_LENGTH;
	t_nubit8 * pModRegRm = ccpu.deCodeBlock.pOpCode+OPCODE_WIDTH;
	t_nubit8 * pDisp = pModRegRm + MOD_REG_RM_LENGTH;
	ccpu.deCodeBlock.isWidth = 0x01; //一定要是真
	ccpu.deCodeBlock.prm = decode_deCodeEffectiveMemAddr(*pModRegRm,(void *)pDisp, &insDispLength );
	ccpu.deCodeBlock.pseg = decode_getSegAddr( ((*pModRegRm)&MASK_00011000)>>3 ); //segment...只需要两个bit就可以确定																//就这个和deOpCodeModRegRm（）不一
	insLength += insDispLength;
	ccpu.ip += insLength ;//更新cpu中的ip，令其指向下一条指令
}
void decode_deOpCode_Mov_imm_reg()
{ //reg 是哪个会在执行时候确定
	int insLength = OPCODE_WIDTH + sizeof(t_nubit8); //至少有这么多
	ccpu.deCodeBlock.isWidth = !!((*ccpu.deCodeBlock.pOpCode) & MASK_00001000);
	if(ccpu.deCodeBlock.isWidth)
	{
		ccpu.deCodeBlock.immData_16Bit = d_nubit16(ccpu.deCodeBlock.pOpCode + OPCODE_WIDTH);
		insLength++;
	}
	else
	{
		ccpu.deCodeBlock.immData_8Bit = d_nubit8(ccpu.deCodeBlock.pOpCode + OPCODE_WIDTH);
	}
	ccpu.ip += insLength ;//更新cpu中的ip，令其指向下一条指针
}
void decode_deOpCode_ModRM()
{
	int insLength = decode_deOpCode_modRegRm();//cpu.ip已经变化
	ccpu.deCodeBlock.nnn = (*(ccpu.deCodeBlock.pOpCode + OPCODE_WIDTH) & MASK_00111000)>>3 ; //迟绑定，到执行的时候再确定到底该执行什么函数
	return ;
}
t_nubit16 decode_getDefaultSegment_DS()
{
	switch(ccpu.deCodeBlock.prefix_SegmentOverrideOpCode)
	{
	case CS_SEGMENT_OVERRIDE_PREFIX:
		return ccpu.cs;
	case SS_SEGMENT_OVERRIDE_PREFIX:
		return ccpu.ss;
	case ES_SEGMENT_OVERRIDE_PREFIX:
		return ccpu.es;
	default:
		return ccpu.ds;
	}
}
t_nubit16 decode_getDefaultSegment_SS()
{
	switch(ccpu.deCodeBlock.prefix_SegmentOverrideOpCode)
	{
	case CS_SEGMENT_OVERRIDE_PREFIX:
		return ccpu.cs;
	case DS_SEGMENT_OVERRIDE_PREFIX:
		return ccpu.ds;
	case ES_SEGMENT_OVERRIDE_PREFIX:
		return ccpu.es;
	default:
		return ccpu.ss;
	}
}