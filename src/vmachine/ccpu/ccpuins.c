#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#include "../vmachine.h"
#include "../vapi.h"
#include "cglobal.h"
#include "ccpuins.h"
#include "ccpuapi.h"

#define OPCODE_WIDTH      1
#define MOD_REG_RM_LENGTH 1

#define CCPU_FLAG_ADD  (VCPU_FLAG_OF | VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_AF | VCPU_FLAG_PF | VCPU_FLAG_CF)
#define CCPU_FLAG_INC  (VCPU_FLAG_OF | VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_AF | VCPU_FLAG_PF)//inc不会影响CF
#define CCPU_FLAG_SUB  (CCPU_FLAG_ADD)
#define CCPU_FLAG_DEC  (CCPU_FLAG_INC)
#define CCPU_FLAG_ADC  (CCPU_FLAG_ADD)
#define CCPU_FLAG_CMP  (CCPU_FLAG_ADD)
#define CCPU_FLAG_CMPS (CCPU_FLAG_ADD)
#define CCPU_FLAG_NEG  (CCPU_FLAG_ADD)
#define CCPU_FLAG_SBB  (CCPU_FLAG_ADD)
#define CCPU_FLAG_SCAS (CCPU_FLAG_ADD)
#define CCPU_FLAG_OR   (VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_PF )
#define CCPU_FLAG_AND  (CCPU_FLAG_OR)
#define CCPU_FLAG_XOR  (CCPU_FLAG_OR)

#define GetOF (GetBit(ccpu.flags, VCPU_FLAG_OF))
#define GetSF (GetBit(ccpu.flags, VCPU_FLAG_SF))
#define GetZF (GetBit(ccpu.flags, VCPU_FLAG_ZF))
#define GetAF (GetBit(ccpu.flags, VCPU_FLAG_AF))
#define GetCF (GetBit(ccpu.flags, VCPU_FLAG_CF))
#define GetPF (GetBit(ccpu.flags, VCPU_FLAG_PF))
#define GetDF (GetBit(ccpu.flags, VCPU_FLAG_DF))
#define GetTF (GetBit(ccpu.flags, VCPU_FLAG_TF))
#define GetIF (GetBit(ccpu.flags, VCPU_FLAG_IF))

#define im(n) cramIsAddrInMem((t_vaddrcc)(n))
#define getMemData_byte(addr) cramVarByte(0,(addr))
#define getMemData_word(addr) cramVarWord(0,(addr))
#define writeMem_byte(addr, data) (cramVarByte(0,(addr)) = (data))
#define writeMem_word(addr, data) (cramVarWord(0,(addr)) = (data))

#define CS_SEGMENT_OVERRIDE_PREFIX  0x2E
#define SS_SEGMENT_OVERRIDE_PREFIX  0x36
#define DS_SEGMENT_OVERRIDE_PREFIX  0x3E
#define ES_SEGMENT_OVERRIDE_PREFIX  0x26
#define LOCK_PREFIX  0xf0
#define REPNE_PREFIX 0xf2
#define REP_PREFIX   0xf3//和下面这个值相同
#define REPE_PREFIX  0xf3

t_cpuins ccpuins;
Method   ccpuins_type;

static void ccpu_flag_CalcOF()
{	
	switch(ccpuins_type) {
	case ADD_8bit:
	case ADC_8bit:
		MakeBit(ccpu.flags, VCPU_FLAG_OF, 
			(((ccpuins.opr1 & 0x80) == (ccpuins.opr2 & 0x80)) && ((ccpuins.opr1 & 0x80) != (ccpuins.result & 0x80))));
		break;
	case ADD_16bit:
	case ADC_16bit:
		MakeBit(ccpu.flags, VCPU_FLAG_OF, 
			(((ccpuins.opr1 & 0x8000) == (ccpuins.opr2 & 0x8000)) && ((ccpuins.opr1 & 0x8000) != (ccpuins.result & 0x8000))));
		break;
	case SUB_8bit:
	case SBB_8bit:
	case CMP_8bit:
		MakeBit(ccpu.flags, VCPU_FLAG_OF, 
			(((ccpuins.opr1 & 0x80) != (ccpuins.opr2 & 0x80)) && ((ccpuins.opr2 & 0x80) == (ccpuins.result & 0x80))));
		break;
	case SUB_16bit:
	case SBB_16bit:
	case CMP_16bit:
		MakeBit(ccpu.flags, VCPU_FLAG_OF, 
			(((ccpuins.opr1 & 0x8000) != (ccpuins.opr2 & 0x8000)) && ((ccpuins.opr2 & 0x8000) == (ccpuins.result & 0x8000))));
		break;
	default:
		vapiPrint("Case error in ccpu_flag_CalcOF::ccpuins_type\n");
		vapiCallBackMachineStop();
		break;
	}
}
static void ccpu_flag_CalcSF()
{
	switch(ccpuins.bit) {
	case 8:
		MakeBit(ccpu.flags, VCPU_FLAG_SF, (ccpuins.result & 0x0080));
		break;
	case 16:
		MakeBit(ccpu.flags, VCPU_FLAG_SF, (ccpuins.result & 0x8000));
		break;
	default:
		vapiPrint("Case error in ccpu_flag_CalcSF::ccpuins.bit\n");
		vapiCallBackMachineStop();
		break;
	}
}
static void ccpu_flag_CalcZF()
{
	MakeBit(ccpu.flags, VCPU_FLAG_ZF, (!ccpuins.result));
}
static void ccpu_flag_CalcAF()
{
	switch(ccpuins_type) {
	case ADD_8bit:
	case ADD_16bit:
	case ADC_8bit:
	case ADC_16bit:
	case SUB_8bit:
	case SUB_16bit:
	case SBB_8bit:
	case SBB_16bit:
	case CMP_8bit:
	case CMP_16bit:
		MakeBit(ccpu.flags, VCPU_FLAG_AF, (((ccpuins.opr1 ^ ccpuins.opr2) ^ ccpuins.result) & 0x10));
		break;
	default:
		vapiPrint("Case error in ccpu_flag_CalcAF::ccpuins_type\n");
		vapiCallBackMachineStop();
		break;
	}
}
static void ccpu_flag_CalcCF()
{
	switch(ccpuins_type) {
	case ADD_8bit:
	case ADD_16bit:
		MakeBit(ccpu.flags, VCPU_FLAG_CF, (ccpuins.result < ccpuins.opr1 || ccpuins.result < ccpuins.opr2));
		break;
	case ADC_8bit:
	case ADC_16bit:
		MakeBit(ccpu.flags, VCPU_FLAG_CF, ccpuins.result <= ccpuins.opr1);
		break;
	case SUB_8bit:
	case SUB_16bit:
	case CMP_8bit:
	case CMP_16bit:
		MakeBit(ccpu.flags, VCPU_FLAG_CF, (ccpuins.opr1 < ccpuins.opr2));
		break;
	case SBB_8bit:
		MakeBit(ccpu.flags, VCPU_FLAG_CF, (ccpuins.opr1 < ccpuins.result) || (0xff == ccpuins.opr2));
		break;
	case SBB_16bit:
		MakeBit(ccpu.flags, VCPU_FLAG_CF, (ccpuins.opr1 < ccpuins.result) || (0xffff == ccpuins.opr2)); 
		break;
	default:
		vapiPrint("Case error in ccpu_flag_CalcCF::ccpuins_type");
		vapiCallBackMachineStop();
		break;
	}
}
static void ccpu_flag_CalcPF()
{
	t_nubitcc count = 0;
	t_nubit8  r8 = (t_nubit8)(ccpuins.result);
	while(r8) {
		r8 &= (r8-1); 
		count++;
	}
	MakeBit(ccpu.flags, VCPU_FLAG_PF, !(count % 2));//运算结果中的低8位有偶数个1，PF为0x01； 奇数个1，PF为0x00
}
static void ccpu_flag_CalcDF() {}
static void ccpu_flag_CalcTF() {}
static void ccpu_flag_CalcIF() {}
void ccpu_flag_SetOF(t_bool f) {MakeBit(ccpu.flags, VCPU_FLAG_OF, f);}
void ccpu_flag_SetSF(t_bool f) {MakeBit(ccpu.flags, VCPU_FLAG_SF, f);}
void ccpu_flag_SetZF(t_bool f) {MakeBit(ccpu.flags, VCPU_FLAG_ZF, f);}
void ccpu_flag_SetAF(t_bool f) {MakeBit(ccpu.flags, VCPU_FLAG_AF, f);}
void ccpu_flag_SetCF(t_bool f) {MakeBit(ccpu.flags, VCPU_FLAG_CF, f);}
void ccpu_flag_SetPF(t_bool f) {MakeBit(ccpu.flags, VCPU_FLAG_PF, f);}
void ccpu_flag_SetDF(t_bool f) {MakeBit(ccpu.flags, VCPU_FLAG_DF, f);}
void ccpu_flag_SetTF(t_bool f) {MakeBit(ccpu.flags, VCPU_FLAG_TF, f);}
void ccpu_flag_SetIF(t_bool f) {MakeBit(ccpu.flags, VCPU_FLAG_IF, f);}
void ccpu_storeCaculate(const Method opMethod,
	const int bitLength, const t_nubit32 operand1, const t_nubit32 operand2,
	const t_nubit32 result, const t_nubit16 mask_Flag)
{
	ccpuins.bit = bitLength;
	ccpuins.opr1 = operand1;
	ccpuins.opr2 = operand2;
	ccpuins.result = result;
	ccpuins_type = opMethod;
	if(mask_Flag & VCPU_FLAG_OF) ccpu_flag_CalcOF();
	if(mask_Flag & VCPU_FLAG_SF) ccpu_flag_CalcSF();
	if(mask_Flag & VCPU_FLAG_ZF) ccpu_flag_CalcZF();
	if(mask_Flag & VCPU_FLAG_AF) ccpu_flag_CalcAF();
	if(mask_Flag & VCPU_FLAG_CF) ccpu_flag_CalcCF();
	if(mask_Flag & VCPU_FLAG_PF) ccpu_flag_CalcPF();
}
void ccpu_flag_SetSF_data8(t_nubit8 data)   {MakeBit(ccpu.flags, VCPU_FLAG_SF, (data & 0x80));}
void ccpu_flag_SetSF_data16(t_nubit16 data) {MakeBit(ccpu.flags, VCPU_FLAG_SF, (data & 0x8000));}
void ccpu_flag_SetZF_data8(t_nubit8 data)   {MakeBit(ccpu.flags, VCPU_FLAG_ZF, !data);}
void ccpu_flag_SetZF_data16(t_nubit16 data) {MakeBit(ccpu.flags, VCPU_FLAG_ZF, !data);}
void ccpu_flag_SetPF_data8(t_nubit8 data)
{
	int count = 0;
	t_nubit8 r8 = (t_nubit8)data;
	while(r8) {
		r8 &= (r8-1); 
		count++;
	}
	MakeBit(ccpu.flags, VCPU_FLAG_PF, !(count % 2)); //运算结果中的低8位有偶数个1，PF为0x01； 奇数个1，PF为0x00
}
void ccpu_flag_SetPF_data16(t_nubit16 data)
{
	ccpu_flag_SetPF_data8((t_nubit8)data);
}
void ccpu_flag_SetSF_ZF_PF_data8(t_nubit8 result)
{
	ccpu_flag_SetZF_data8(result);
	ccpu_flag_SetSF_data8(result);
	ccpu_flag_SetPF_data8(result);
}
void ccpu_flag_SetSF_ZF_PF_data16(t_nubit16 result)
{
	ccpu_flag_SetZF_data16(result);
	ccpu_flag_SetSF_data16(result);
	ccpu_flag_SetPF_data16(result);
}
void ccpu_flag_SetFlags(t_nubit16 f)
{
	ccpu.flags = f;
/*	PROBLEM
	ccpu_flag_SetOF(!!(f & VCPU_FLAG_OF));
	ccpu_flag_SetSF(!!(f & VCPU_FLAG_SF));
	ccpu_flag_SetZF(!!(f & VCPU_FLAG_ZF));
	ccpu_flag_SetAF(!!(f & VCPU_FLAG_AF));
	ccpu_flag_SetCF(!!(f & VCPU_FLAG_CF));
	ccpu_flag_SetPF(!!(f & VCPU_FLAG_PF));
	ccpu_flag_SetDF(!!(f & VCPU_FLAG_DF));
	ccpu_flag_SetTF(!!(f & VCPU_FLAG_TF));
	ccpu_flag_SetIF(!!(f & VCPU_FLAG_IF));*/
}
void ccpu_flag_Set_SF_ZF_AF_PF_CF(t_nubit16 f)
{
	ccpu_flag_SetSF(!!(f & VCPU_FLAG_SF));
	ccpu_flag_SetZF(!!(f & VCPU_FLAG_ZF));
	ccpu_flag_SetAF(!!(f & VCPU_FLAG_AF));
	ccpu_flag_SetPF(!!(f & VCPU_FLAG_PF));
	ccpu_flag_SetCF(!!(f & VCPU_FLAG_CF));
}

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
	ccpuins_deCodeBlock.prefix_SegmentOverrideOpCode = 0;//消除prefix的影响，防止影响下一条指令
	ccpuins_deCodeBlock.prefix_LockOpCode = 0;
	ccpuins_deCodeBlock.prefix_RepeatOpCode = 0;
}

void* decode_getRegAddr(t_bool w, const t_nubit8 reg);
t_nubit16* decode_getSegAddr(const t_nubit8 sreg);
int  decode_deOpCode_modRegRm();
void * decode_deCodeEffectiveMemAddr(const t_nubit8 modRegRm, const void * pdispData, int* pdispLength);
void decode_deOpCode_IMM();
void decode_deOpCode_TEST_Ib_Iv();
void decode_deOpCode_IMMGroup();
void decode_deOpCode_contactData_8bit();
void decode_deOpCode_contactData_16bit();
void decode_deOpCode_contactData_32bit();
void decode_deOpCode_Mov_mem_AX();
void decode_deOpCode_Mov_imm_rm();
void decode_deOpCode_Mov_rm_seg();
void decode_deOpCode_Mov_imm_reg();
void decode_deOpCode_ModRM();
t_nubit16 decode_getDefaultSegment_DS();
t_nubit16 decode_getDefaultSegment_SS();
void ins_self_string_affect_DI_SI(const DataTypeLength type);
void ins_self_string_affect_SI(const DataTypeLength type);
void ins_self_string_affect_DI(const DataTypeLength type);
void ins_chooseRepeatExeStringMethod();
void ins_self_REP_StringIns();
void ins_self_REPE_StringIns();
void ins_self_REPNE_StringIns();

void decode_deOpCode_singleDefine() {ccpu.ip += 1;}
void decode_deOpCode_doubleDefine() {ccpu.ip += 2;}
void decode_deCodeInsStuff()
{
	t_nubit8 tmpOpCode ;
	switch(*ccpuins_deCodeBlock.pOpCode) {
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
		tmpOpCode = *ccpuins_deCodeBlock.pOpCode;
		*ccpuins_deCodeBlock.pOpCode = MASK_11000101;//由于LES的opcode的最后w位是0，所以必须换成1，改成LDS
		decode_deOpCode_modRegRm();
		*ccpuins_deCodeBlock.pOpCode = tmpOpCode;
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
		if(0 == (*(ccpuins_deCodeBlock.pOpCode +1)&MASK_00111000) )//TEST Ib/Iv
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
		vapiPrint("deCodeInsStuff Unhandle a OpCode %x\n", *ccpuins_deCodeBlock.pOpCode);
		vapiCallBackMachineStop();
		//vapiCallBackMachineStop();;
		break;
	}
}
void* decode_getRegAddr(t_bool w, const t_nubit8 reg)
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
int  decode_deOpCode_modRegRm()
{
	int insDispLength = 0;
	int insLength = OPCODE_WIDTH + MOD_REG_RM_LENGTH;
	t_nubit8 * pModRegRm = ccpuins_deCodeBlock.pOpCode+OPCODE_WIDTH;
	t_nubit8 * pDisp = pModRegRm + MOD_REG_RM_LENGTH;
	ccpuins_deCodeBlock.isWidth = !!(*ccpuins_deCodeBlock.pOpCode & MASK_00000001);
	ccpuins_deCodeBlock.prm = decode_deCodeEffectiveMemAddr(*pModRegRm,(void *)pDisp, &insDispLength );
	ccpuins_deCodeBlock.preg = decode_getRegAddr(ccpuins_deCodeBlock.isWidth, ((*pModRegRm)&MASK_00111000)>>3 );
	insLength +=  insDispLength;
	ccpu.ip +=  insLength;//更新cpu中的ip，令其指向下一条指
	return insLength;
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
		memAddr =   ccpu.bx + ccpu.si + ((ccpuins_deCodeBlock.segData = tmpDS) <<4 );
		break;
	case 1://RM=001
		memAddr =   ccpu.bx + ccpu.di + ((ccpuins_deCodeBlock.segData = tmpDS) <<4 );
		break;
	case 2://RM=010
		memAddr =   ccpu.bp + ccpu.si + ((ccpuins_deCodeBlock.segData = tmpSS) <<4);
		break;
	case 3://RM=011
		memAddr =   ccpu.bp + ccpu.di + ((ccpuins_deCodeBlock.segData = tmpSS) <<4);
		break;
	case 4://RM=100
		memAddr =   ccpu.si + ((ccpuins_deCodeBlock.segData = tmpDS) <<4);
		break;
	case 5://RM=101
		memAddr =   ccpu.di + ((ccpuins_deCodeBlock.segData = tmpDS) <<4);
		break;
	case 6://RM=110
		if( 0 == mod ) //MOD=00
		{
			memAddr = d_nubit16(pdispData) + ((ccpuins_deCodeBlock.segData = tmpDS) <<4);
			*pdispLength=2;
		}
		else
			memAddr = ccpu.bp + ((ccpuins_deCodeBlock.segData = tmpSS) <<4);
		break;
	case 7://RM=111
		memAddr =   ccpu.bx + ((ccpuins_deCodeBlock.segData = tmpDS) <<4);
		break;
	default://谁知道呢，，，可能宇宙射线
		vapiPrint("deCodeEffectiveMemAddr UnHandle the rm bits");
		vapiCallBackMachineStop();;
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
		return (void *)decode_getRegAddr(ccpuins_deCodeBlock.isWidth, (modRegRm & MASK_00000111) );
	default:
		vapiPrint("deCodeEffectiveMemAddr UnHandle the mod bits");
		vapiCallBackMachineStop();;
		return 0;
	}
	return (void *)(p_nubit8(cramGetAddr(0,0)) + memAddr);

}
void decode_deOpCode_IMM()
{
	int insLength = OPCODE_WIDTH ;
	t_nubit8 * pimmData = ccpuins_deCodeBlock.pOpCode + OPCODE_WIDTH;
	t_bool w_width = !!((*ccpuins_deCodeBlock.pOpCode)&MASK_00000001);
	if(w_width)
	{
		ccpuins_deCodeBlock.immData_16Bit = d_nubit16(pimmData);
		insLength += 2;
	}
	else
	{
		ccpuins_deCodeBlock.immData_8Bit = *(pimmData);
		insLength++;
	}
	ccpu.ip += insLength ;//更新cpu中的ip，令其指向下一条指令

}
void decode_deOpCode_TEST_Ib_Iv()
{
	int insLength = decode_deOpCode_modRegRm();//cpu.ip已经变化
	//抄deOpCode_IMMGroup， 但是不检测s位
	switch( (*ccpuins_deCodeBlock.pOpCode)&MASK_00000001 )
	{
	case MASK_00000001:
		ccpuins_deCodeBlock.immData_16Bit =  d_nubit16(ccpuins_deCodeBlock.pOpCode + insLength);
		ccpu.ip += 2;
		break;
	default:
		ccpuins_deCodeBlock.immData_8Bit =  *(ccpuins_deCodeBlock.pOpCode + insLength);
		ccpu.ip++;
	}
	ccpuins_deCodeBlock.nnn = (*(ccpuins_deCodeBlock.pOpCode + OPCODE_WIDTH) & MASK_00111000)>>3 ; //迟绑定，到执行的时候再确定到底该执行什么函数
	
	return ;
}
void decode_deOpCode_IMMGroup()
{//未完成品

	int insLength = decode_deOpCode_modRegRm();//cpu.ip已经变化
	
	switch( (*ccpuins_deCodeBlock.pOpCode)&MASK_00000011 )
	{//s位的处理
	case MASK_00000001:
		ccpuins_deCodeBlock.immData_16Bit =  d_nubit16(ccpuins_deCodeBlock.pOpCode + insLength);
		ccpu.ip += 2;
		break;
	case MASK_00000011:
		ccpuins_deCodeBlock.immData_16Bit =  (t_nsbit16)(d_nsbit8(ccpuins_deCodeBlock.pOpCode + insLength));
		ccpu.ip += 1;
		break;
	default:
		ccpuins_deCodeBlock.immData_8Bit =  *(ccpuins_deCodeBlock.pOpCode + insLength);
		ccpu.ip++;
	}
	ccpuins_deCodeBlock.nnn = (*(ccpuins_deCodeBlock.pOpCode + OPCODE_WIDTH) & MASK_00111000)>>3 ; //迟绑定，到执行的时候再确定到底该执行什么函数
	
	return ;

}
void decode_deOpCode_contactData_8bit()
{
	int insLength = OPCODE_WIDTH + sizeof(t_nubit8);
	ccpuins_deCodeBlock.opContactData_8bit = d_nubit8(ccpuins_deCodeBlock.pOpCode + OPCODE_WIDTH);
	ccpu.ip += insLength;
	return;
}
void decode_deOpCode_contactData_16bit()
{
	int insLength = OPCODE_WIDTH + sizeof(t_nubit16);//已经确定指令长度
	ccpuins_deCodeBlock.opContactData_16bit = d_nubit16(ccpuins_deCodeBlock.pOpCode + OPCODE_WIDTH);
	ccpu.ip += insLength ;//更新cpu中的ip，令其指向下一条指令
	return ;
}
void decode_deOpCode_contactData_32bit()
{
	int insLength = OPCODE_WIDTH + sizeof(t_nubit32);//已经确定指令长度
	ccpuins_deCodeBlock.opContactData_16bit = d_nubit16(ccpuins_deCodeBlock.pOpCode + OPCODE_WIDTH);
	ccpuins_deCodeBlock.opContactData_16bitE = d_nubit16(ccpuins_deCodeBlock.pOpCode + OPCODE_WIDTH +sizeof(t_nubit16));
	ccpu.ip += insLength ;//更新cpu中的ip，令其指向下一条指令
	return;
}
void decode_deOpCode_Mov_mem_AX()
{
	int insLength = OPCODE_WIDTH + sizeof(t_nubit16);//已经确定指令长度
	int nothing=0;
	ccpuins_deCodeBlock.isWidth = !!(*ccpuins_deCodeBlock.pOpCode & MASK_00000001);
	ccpuins_deCodeBlock.prm = decode_deCodeEffectiveMemAddr(MASK_00000110, ccpuins_deCodeBlock.pOpCode+OPCODE_WIDTH, &nothing );
	ccpu.ip += insLength ;//更新cpu中的ip，令其指向下一条指令
	return ;

}
void decode_deOpCode_Mov_imm_rm()
{

	int insLength = decode_deOpCode_modRegRm();//cpu.ip已经变化
	
	if(ccpuins_deCodeBlock.isWidth)
	{
		ccpuins_deCodeBlock.immData_16Bit =  d_nubit16(ccpuins_deCodeBlock.pOpCode + insLength);
		ccpu.ip += 2;
	}
	else
	{
		ccpuins_deCodeBlock.immData_8Bit =  *(ccpuins_deCodeBlock.pOpCode + insLength);
		ccpu.ip++;
	}
	return ;
	
}
void decode_deOpCode_Mov_rm_seg()
{
	int insDispLength = 0;
	int insLength = OPCODE_WIDTH + MOD_REG_RM_LENGTH;
	t_nubit8 * pModRegRm = ccpuins_deCodeBlock.pOpCode+OPCODE_WIDTH;
	t_nubit8 * pDisp = pModRegRm + MOD_REG_RM_LENGTH;
	ccpuins_deCodeBlock.isWidth = 0x01; //一定要是真
	ccpuins_deCodeBlock.prm = decode_deCodeEffectiveMemAddr(*pModRegRm,(void *)pDisp, &insDispLength );
	ccpuins_deCodeBlock.pseg = decode_getSegAddr( ((*pModRegRm)&MASK_00011000)>>3 ); //segment...只需要两个bit就可以确定																//就这个和deOpCodeModRegRm（）不一
	insLength += insDispLength;
	ccpu.ip += insLength ;//更新cpu中的ip，令其指向下一条指令
}
void decode_deOpCode_Mov_imm_reg()
{ //reg 是哪个会在执行时候确定
	int insLength = OPCODE_WIDTH + sizeof(t_nubit8); //至少有这么多
	ccpuins_deCodeBlock.isWidth = !!((*ccpuins_deCodeBlock.pOpCode) & MASK_00001000);
	if(ccpuins_deCodeBlock.isWidth)
	{
		ccpuins_deCodeBlock.immData_16Bit = d_nubit16(ccpuins_deCodeBlock.pOpCode + OPCODE_WIDTH);
		insLength++;
	}
	else
	{
		ccpuins_deCodeBlock.immData_8Bit = d_nubit8(ccpuins_deCodeBlock.pOpCode + OPCODE_WIDTH);
	}
	ccpu.ip += insLength ;//更新cpu中的ip，令其指向下一条指针
}
void decode_deOpCode_ModRM()
{
	int insLength = decode_deOpCode_modRegRm();//cpu.ip已经变化
	ccpuins_deCodeBlock.nnn = (*(ccpuins_deCodeBlock.pOpCode + OPCODE_WIDTH) & MASK_00111000)>>3 ; //迟绑定，到执行的时候再确定到底该执行什么函数
	return ;
}
t_nubit16 decode_getDefaultSegment_DS()
{
	switch(ccpuins_deCodeBlock.prefix_SegmentOverrideOpCode)
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
	switch(ccpuins_deCodeBlock.prefix_SegmentOverrideOpCode)
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
void ins_self_string_affect_DI_SI(const DataTypeLength type)
{//执行串操作会对 di，si进行改变
	unsigned step =0;
	switch(type)
	{
	case dataType_U1:
		step = 1;
		break;
	case dataType_U2:
		step = 2;
		break;
	}
	if(GetDF)
	{
		ccpu.si-=step;
		ccpu.di-=step;
	}
	else
	{
		ccpu.si+=step;
		ccpu.di+=step;
	}
}
void ins_self_string_affect_SI(const DataTypeLength type) 
{
	unsigned step =0;
	switch(type)
	{
	case dataType_U1:
		step = 1;
		break;
	case dataType_U2:
		step = 2;
		break;
	}
	if(GetDF)
		ccpu.si-=step;
	else
		ccpu.si+=step;
}
void ins_self_string_affect_DI(const DataTypeLength type) 
{
	unsigned step =0;
	switch(type)
	{
	case dataType_U1:
		step = 1;
		break;
	case dataType_U2:
		step = 2;
		break;
	}
	if(GetDF)
		ccpu.di-=step;
	else
		ccpu.di+=step;

}

void ccpuins_atom_MOV_8bit(t_nubit32 logicAddr_des, const t_nubit8 data_src)
{
#if DEBUGMODE == BVRM
	if (im(vramGetAddr(0, logicAddr_des))) return;
#endif
	d_nubit8(vramGetAddr(0, logicAddr_des)) = data_src;
}
void ccpuins_atom_MOV_16bit(t_nubit32 logicAddr_des, const t_nubit16 data_src)
{
#if DEBUGMODE == BVRM
	if (im(vramGetAddr(0, logicAddr_des))) return;
#endif
	d_nubit16(vramGetAddr(0, logicAddr_des)) = data_src;
}
void ccpuins_atom_ADD_8bit(t_nubit8 *pdes, const t_nubit8 src)
{
	t_nubit8 operand1 = *pdes;
	*pdes += src;
	ccpu_storeCaculate(ADD_8bit, 8, 
							operand1,  src, *pdes,
							CCPU_FLAG_ADD);
#if DEBUGMODE == BVRM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ccpuins_atom_ADD_16bit(t_nubit16 *pdes, const t_nubit16 src)
{
	t_nubit16 operand1 = *pdes;
	*pdes += src;
	ccpu_storeCaculate(ADD_16bit, 16, 
							operand1,  src, *pdes,
							CCPU_FLAG_ADD);
#if DEBUGMODE == BVRM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ccpuins_atom_INC_8bit(t_nubit8 *pdes)
{
	t_nubit8 operand1 = *pdes;
	(*pdes)++;
	ccpu_storeCaculate(ADD_8bit, 8,
							operand1, 1, *pdes,
							CCPU_FLAG_INC);
#if DEBUGMODE == BVRM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ccpuins_atom_INC_16bit(t_nubit16 *pdes)
{
	t_nubit16 operand1 = *pdes;
	(*pdes)++;
	ccpu_storeCaculate(ADD_16bit, 16,
							operand1, 1, *pdes,
							CCPU_FLAG_INC);
#if DEBUGMODE == BVRM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ccpuins_atom_ADC_8bit(t_nubit8 *pdes, const t_nubit8 src)
{
	t_nubit8 operand1 = *pdes;
	if(GetCF)
	{
		*pdes += src +1;
		ccpu_storeCaculate(ADC_8bit, 8, 
							operand1,  src, *pdes,//参考bochs作法..悲剧的ADC
							CCPU_FLAG_ADD);
	}
	else
	{
		*pdes += src;
		ccpu_storeCaculate(ADD_8bit, 8, 
							operand1,  src, *pdes,//如果CF为假时候则相当于执行ADD操作
							CCPU_FLAG_ADD);
	}
#if DEBUGMODE == BVRM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ccpuins_atom_ADC_16bit(t_nubit16 *pdes, const t_nubit16 src)
{
	t_nubit16 operand1 = *pdes;
	if(GetCF)
	{
		*pdes += src +1;
		ccpu_storeCaculate(ADC_16bit, 16, 
							operand1,  src, *pdes,//参考bochs作法..悲剧的ADC
							CCPU_FLAG_ADD);
	}
	else
	{
		*pdes += src;
		ccpu_storeCaculate(ADD_16bit, 16, 
							operand1,  src, *pdes,//如果CF为假时候则相当于执行ADD操作
							CCPU_FLAG_ADD);
	}
#if DEBUGMODE == BVRM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ccpuins_atom_SUB_8bit(t_nubit8 *pdes, const t_nubit8 src)
{
	t_nubit8 operand1 = *pdes;
	*pdes -= src;
	ccpu_storeCaculate(SUB_8bit, 8, 
							operand1,  src, *pdes,
							CCPU_FLAG_SUB);
#if DEBUGMODE == BVRM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ccpuins_atom_SUB_16bit(t_nubit16 *pdes, const t_nubit16 src)
{
	t_nubit16 operand1 = *pdes;
	*pdes -= src;
	ccpu_storeCaculate(SUB_16bit, 16, 
							operand1,  src, *pdes,
							CCPU_FLAG_SUB);
#if DEBUGMODE == BVRM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ccpuins_atom_DEC_8bit(t_nubit8 *pdes)
{
	t_nubit8 operand1 = *pdes;
	(*pdes)--;
	ccpu_storeCaculate(SUB_8bit, 8,
							operand1, 1, *pdes,
							CCPU_FLAG_DEC);
#if DEBUGMODE == BVRM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ccpuins_atom_DEC_16bit(t_nubit16 *pdes)
{
	t_nubit16 operand1 = *pdes;
	(*pdes)--;
	ccpu_storeCaculate(SUB_16bit, 16,
							operand1, 1, *pdes,
							CCPU_FLAG_DEC);
#if DEBUGMODE == BVRM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ccpuins_atom_SBB_8bit(t_nubit8 *pdes, const t_nubit8 src)
{
	t_nubit8 operand1 = *pdes;
	if(GetCF)
	{
		*pdes -= src +1;
		ccpu_storeCaculate(SBB_8bit, 8, 
							operand1,  src, *pdes,//参考bochs作法..悲剧的SBB
							CCPU_FLAG_ADD);
	}
	else
	{
		*pdes -= src;
		ccpu_storeCaculate(SUB_8bit, 8, 
							operand1,  src, *pdes,//如果CF为假时候则相当于执行SUB操作
							CCPU_FLAG_ADD);
	}
#if DEBUGMODE == BVRM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ccpuins_atom_SBB_16bit(t_nubit16 *pdes, const t_nubit16 src)
{
	t_nubit16 operand1 = *pdes;
	if(GetCF)
	{
		*pdes -= src +1;
		ccpu_storeCaculate(SBB_16bit, 16, 
							operand1,  src, *pdes,//参考bochs作法..悲剧的SBB
							CCPU_FLAG_ADD);
	}
	else
	{
		*pdes -= src;
/* NEKO DEBUG: SUB_16bit -> SBB_16bit*/
		ccpu_storeCaculate(SBB_16bit, 16, 
							operand1,  src, *pdes,//如果CF为假时候则相当于执行SUB操作
							CCPU_FLAG_ADD);
	}
#if DEBUGMODE == BVRM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ccpuins_atom_OR_8bit(t_nubit8 *pdes, const t_nubit8 src)
{
	t_nubit8 operand1 = *pdes;
	*pdes |= src;
	ccpu_storeCaculate(OR_8bit, 8, 
							operand1,  src, *pdes,
							CCPU_FLAG_OR);
	ccpu_flag_SetOF(0x00);
	ccpu_flag_SetCF(0x00);
	ccpu_flag_SetAF(0x00);//Intel中说是不影响，但是debug中会影响
#if DEBUGMODE == BVRM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ccpuins_atom_OR_16bit(t_nubit16 *pdes, const t_nubit16 src)
{
	t_nubit16 operand1 = *pdes;
	*pdes |= src;
	ccpu_storeCaculate(OR_16bit, 16, 
							operand1,  src, *pdes,
							CCPU_FLAG_OR);
	ccpu_flag_SetOF(0x00);
	ccpu_flag_SetCF(0x00);
	ccpu_flag_SetAF(0x00);//Intel中说是不影响，但是debug中会影响
#if DEBUGMODE == BVRM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ccpuins_atom_AND_8bit(t_nubit8 *pdes, const t_nubit8 src)
{
	t_nubit8 operand1 = *pdes;
	*pdes &= src;
	ccpu_storeCaculate(AND_8bit, 8, 
							operand1,  src, *pdes,
							CCPU_FLAG_AND);
	ccpu_flag_SetOF(0x00);
	ccpu_flag_SetCF(0x00);
	ccpu_flag_SetAF(0x00);//Intel中说是不影响，但是debug中会影响
#if DEBUGMODE == BVRM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ccpuins_atom_AND_16bit(t_nubit16 *pdes, const t_nubit16 src)
{
	t_nubit16 operand1 = *pdes;
	*pdes &= src;
	ccpu_storeCaculate(AND_16bit, 16, 
							operand1,  src, *pdes,
							CCPU_FLAG_AND);
	ccpu_flag_SetOF(0x00);
	ccpu_flag_SetCF(0x00);
	ccpu_flag_SetAF(0x00);//Intel中说是不影响，但是debug中会影响
#if DEBUGMODE == BVRM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ccpuins_atom_TEST_8bit(t_nubit8 *pdes, const t_nubit8 src)
{
	t_nubit8 operand1 = *pdes;
	ccpu_storeCaculate(AND_8bit, 8, 
							operand1,  src, (*pdes & src),
							CCPU_FLAG_AND);
	ccpu_flag_SetOF(0x00);
	ccpu_flag_SetCF(0x00);
	ccpu_flag_SetAF(0x00);//Intel中说是不影响，但是debug中会影响
}
void ccpuins_atom_TEST_16bit(t_nubit16 *pdes, const t_nubit16 src)
{
	t_nubit16 operand1 = *pdes;
	ccpu_storeCaculate(AND_16bit, 16, 
							operand1,  src, (*pdes & src),
							CCPU_FLAG_AND);
	ccpu_flag_SetOF(0x00);
	ccpu_flag_SetCF(0x00);
	ccpu_flag_SetAF(0x00);//Intel中说是不影响，但是debug中会影响
}
void ccpuins_atom_XOR_8bit(t_nubit8 *pdes, const t_nubit8 src)
{
	t_nubit8 operand1 = *pdes;
	*pdes ^= src;
	ccpu_storeCaculate(XOR_8bit, 8, 
							operand1,  src, *pdes,
							CCPU_FLAG_XOR);
	ccpu_flag_SetOF(0x00);
	ccpu_flag_SetCF(0x00);
	ccpu_flag_SetAF(0x00);//Intel中说是不影响，但是debug中会影响
#if DEBUGMODE == BVRM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ccpuins_atom_XOR_16bit(t_nubit16 *pdes, const t_nubit16 src)
{
	t_nubit16 operand1 = *pdes;
	*pdes ^= src;
	ccpu_storeCaculate(XOR_16bit, 16, 
							operand1,  src, *pdes,
							CCPU_FLAG_XOR);
	ccpu_flag_SetOF(0x00);
	ccpu_flag_SetCF(0x00);
	ccpu_flag_SetAF(0x00);//根据debug得出的，虽然Intel的文档说是undefine
#if DEBUGMODE == BVRM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ccpuins_atom_CMP_8bit(t_nubit8 des, const t_nubit8 src)
{
	t_nubit8 result = des - src;
	ccpu_storeCaculate(CMP_8bit, 8, 
							des,  src, result,
							CCPU_FLAG_CMP);
}
void ccpuins_atom_CMP_16bit(t_nubit16 des, const t_nubit16 src)
{
	t_nubit16 result = des - src;
	ccpu_storeCaculate(CMP_16bit, 16, 
							des,  src, result,
							CCPU_FLAG_CMP);
}
void ccpuins_atom_CMPSB()
{
	t_nubit8 des, src;
	des = getMemData_byte(ccpu.di + (ccpu.es<<4));
	src = getMemData_byte(ccpu.si + (decode_getDefaultSegment_DS() <<4));
	ccpuins_atom_CMP_8bit(src, des);
	ins_self_string_affect_DI_SI(dataType_U1);
}
void ccpuins_atom_CMPSW()
{
	t_nubit16 des, src;
	des = getMemData_word(ccpu.di + (ccpu.es<<4) );
	src = getMemData_word(ccpu.si + (decode_getDefaultSegment_DS() <<4));
	ccpuins_atom_CMP_16bit(src, des);
	ins_self_string_affect_DI_SI(dataType_U2);
}
void ccpuins_atom_MOVSB()
{
	t_nubit8 data = getMemData_byte(ccpu.si + (decode_getDefaultSegment_DS() <<4));
#if (DEBUGMODE == CCPU || DEBUGMODE == BCRM)
	writeMem_byte(ccpu.di + (ccpu.es<<4), data);
#endif
	ins_self_string_affect_DI_SI(dataType_U1); //串操作对DI， SI修改
}
void ccpuins_atom_MOVSW()
{
	
	t_nubit16 data = getMemData_word(ccpu.si + (decode_getDefaultSegment_DS() <<4));
#if (DEBUGMODE == CCPU || DEBUGMODE == BCRM)
	writeMem_word(ccpu.di + (ccpu.es<<4), data);
#endif
	ins_self_string_affect_DI_SI(dataType_U2);//串操作对DI， SI修改
}

void ccpuins_atom_STOSB()
{
#if (DEBUGMODE == CCPU || DEBUGMODE == BCRM)
	writeMem_byte(ccpu.di + (ccpu.es<<4), ccpu.al);
#endif
	ins_self_string_affect_DI(dataType_U1);//串操作对DI修改
}
void ccpuins_atom_STOSW()
{
#if (DEBUGMODE == CCPU || DEBUGMODE == BCRM)
	writeMem_word(ccpu.di + (ccpu.es<<4), ccpu.ax);
#endif
	ins_self_string_affect_DI(dataType_U2);//串操作对DI修改
}
void ccpuins_atom_LODSB()
{
	ccpu.al = getMemData_byte(ccpu.si + (decode_getDefaultSegment_DS()<<4));
	ins_self_string_affect_SI(dataType_U1);//串操作对 SI修改
}
void ccpuins_atom_LODSW()
{
	ccpu.ax = getMemData_word(ccpu.si + (decode_getDefaultSegment_DS()<<4));
	ins_self_string_affect_SI(dataType_U2);//串操作对 SI修改
}
void ccpuins_atom_SCASB()
{
	ccpuins_atom_CMP_8bit(ccpu.al, getMemData_byte(ccpu.di + (ccpu.es<<4)));
	ins_self_string_affect_DI(dataType_U1);//串操作对DI， SI修改
}
void ccpuins_atom_SCASW()
{
	ccpuins_atom_CMP_16bit(ccpu.ax, getMemData_word(ccpu.di + (ccpu.es<<4)));
	ins_self_string_affect_DI(dataType_U2);//串操作对DI， SI修改
}
void ccpuins_atom_INS() {}
void ccpuins_atom_OUTS() {}

void ccpuins_atom_shortJMP()
{
	ccpu.ip += (t_nsbit8)(ccpuins_deCodeBlock.opContactData_8bit);
}
void ccpuins_atom_PUSH(const t_nubit16 data)
{
	ccpu.sp -= 2;
#if (DEBUGMODE == CCPU || DEBUGMODE == BCRM)
//	vapiPrint("C: PUSH %04X to %08X\n",data,((ccpu.ss<<4)+ccpu.sp));
	writeMem_word((ccpu.sp + ((ccpu.ss)<<4)), data);
#endif
}
t_nubit16 ccpuins_atom_POP()
{
	t_nubit16 data = getMemData_word((ccpu.sp + ((ccpu.ss)<<4)));
	ccpu.sp += 2;
	return data;
}
void ccpuins_atom_INT(const t_nubit8 INT_num)
{
	switch (INT_num) {
	case 0x13:
	case 0x16:
	case 0x10:
	case 0x15:
	case 0x1a:
	case 0x11:
	case 0x12:
	case 0x14:
	case 0x17:
#if DEBUGMODE != CCPU
		ccpu_flagignore = 0x01;
#else
		qdbiosExecInt(INT_num);
#endif
		return;
	}
	ccpuins_atom_PUSH(ccpu.flags);//FLAGS入栈	
	ccpuins_atom_PUSH(ccpu.cs);
	ccpuins_atom_PUSH(ccpu.ip);
	ccpu_flag_SetIF(0x00);
	ccpu_flag_SetTF(0x00); //禁止硬件中断的单步中断
	ccpu.ip = getMemData_word(INT_num*4);
	ccpu.cs = getMemData_word(INT_num*4 + 2);
}

////////////////////////////////////////////////////////////////////////
#define rotl(x,n) ((((x)<<(n))&(-(1<<(n))))|(((x)>>(sizeof(x)*8-(n)))&((1<<(n))-1)))
#define ror(x) rotl((x),1)
static t_bool getSignByData_data8(const t_nubit8 data) {return !!(data & MASK_10000000);}
static t_bool getSignByData_data16(const t_nubit16 data) {return !!(data & 0x8000);}
static t_bool getFinalBit_data8(const t_nubit8 data) {return !!(data & MASK_00000001);}
static t_bool getFinalBit_data16(const t_nubit16 data) {return !!(data & 1);}
static t_bool isSignBitChange_data8(const t_nubit8 fir, const t_nubit8 sec)
{
	if(getSignByData_data8(fir) == getSignByData_data8(sec))
		return 0x00;
	return 0x01;
}
static t_bool isSignBitChange_data16(const t_nubit16 fir, const t_nubit16 sec)
{
	if(getSignByData_data16(fir) == getSignByData_data16(sec))
		return 0x00;
	return 0x01;
}
////////////////////////////////////////////<GROUP1> :DONE
void ccpuins_group_ADD_Eb_Ib()//tested
{
	ccpuins_atom_ADD_8bit(p_nubit8(ccpuins_deCodeBlock.prm), ccpuins_deCodeBlock.immData_8Bit);
}
void ccpuins_group_ADD_Ev_Iv()//tested
{
	ccpuins_atom_ADD_16bit(p_nubit16(ccpuins_deCodeBlock.prm), ccpuins_deCodeBlock.immData_16Bit);
}
void ccpuins_group_ADD_Ev_Ib()//tested
{
	t_nubit16 tmp = ccpuins_deCodeBlock.immData_8Bit;
	ccpuins_atom_ADD_16bit(p_nubit16(ccpuins_deCodeBlock.prm), tmp);
}

void ccpuins_group_OR_Eb_Ib()
{
	ccpuins_atom_OR_8bit(p_nubit8(ccpuins_deCodeBlock.prm), ccpuins_deCodeBlock.immData_8Bit);
}
void ccpuins_group_OR_Ev_Iv()
{
	ccpuins_atom_OR_16bit(p_nubit16(ccpuins_deCodeBlock.prm), ccpuins_deCodeBlock.immData_16Bit);
}
void ccpuins_group_OR_Ev_Ib()
{
	//vapiCallBackMachineStop();;//may be some problem
	t_nubit16 tmp = ccpuins_deCodeBlock.immData_8Bit;
	ccpuins_atom_OR_16bit(p_nubit16(ccpuins_deCodeBlock.prm), tmp);
}

void ccpuins_group_ADC_Eb_Ib()//tested
{
	ccpuins_atom_ADC_8bit(p_nubit8(ccpuins_deCodeBlock.prm), ccpuins_deCodeBlock.immData_8Bit);
}
void ccpuins_group_ADC_Ev_Iv()//tested
{
	ccpuins_atom_ADC_16bit(p_nubit16(ccpuins_deCodeBlock.prm), ccpuins_deCodeBlock.immData_16Bit);
}
void ccpuins_group_ADC_Ev_Ib()//tested
{
	//vapiCallBackMachineStop();;//may be some problem
	t_nubit16 tmp = ccpuins_deCodeBlock.immData_8Bit;
	ccpuins_atom_ADC_16bit(p_nubit16(ccpuins_deCodeBlock.prm), tmp);
}

void ccpuins_group_SBB_Eb_Ib()
{
	ccpuins_atom_SBB_8bit(p_nubit8(ccpuins_deCodeBlock.prm), ccpuins_deCodeBlock.immData_8Bit);
}
void ccpuins_group_SBB_Ev_Iv()
{
	ccpuins_atom_SBB_16bit(p_nubit16(ccpuins_deCodeBlock.prm), ccpuins_deCodeBlock.immData_16Bit);
}
void ccpuins_group_SBB_Ev_Ib()
{
	//vapiCallBackMachineStop();;//may be some problem
	t_nubit16 tmp = ccpuins_deCodeBlock.immData_8Bit;
	ccpuins_atom_SBB_16bit(p_nubit16(ccpuins_deCodeBlock.prm), tmp);
}

void ccpuins_group_AND_Eb_Ib()
{
	ccpuins_atom_AND_8bit(p_nubit8(ccpuins_deCodeBlock.prm), ccpuins_deCodeBlock.immData_8Bit);
}
void ccpuins_group_AND_Ev_Iv()
{
	ccpuins_atom_AND_16bit(p_nubit16(ccpuins_deCodeBlock.prm), ccpuins_deCodeBlock.immData_16Bit);
}
void ccpuins_group_AND_Ev_Ib()
{
	//vapiCallBackMachineStop();;//may be some problem
	t_nubit16 tmp = ccpuins_deCodeBlock.immData_8Bit;
	ccpuins_atom_AND_16bit(p_nubit16(ccpuins_deCodeBlock.prm), tmp);
}

void ccpuins_group_SUB_Eb_Ib()//tested
{
	ccpuins_atom_SUB_8bit(p_nubit8(ccpuins_deCodeBlock.prm), ccpuins_deCodeBlock.immData_8Bit);
}
void ccpuins_group_SUB_Ev_Iv()//tested
{
	ccpuins_atom_SUB_16bit(p_nubit16(ccpuins_deCodeBlock.prm), ccpuins_deCodeBlock.immData_16Bit);
}
void ccpuins_group_SUB_Ev_Ib()//tested
{
	//vapiCallBackMachineStop();;//may be some problem
	t_nubit16 tmp = ccpuins_deCodeBlock.immData_8Bit;
	ccpuins_atom_SUB_16bit(p_nubit16(ccpuins_deCodeBlock.prm), tmp);
}

void ccpuins_group_XOR_Eb_Ib()
{
	ccpuins_atom_XOR_8bit(p_nubit8(ccpuins_deCodeBlock.prm), ccpuins_deCodeBlock.immData_8Bit);
}
void ccpuins_group_XOR_Ev_Iv()
{
	ccpuins_atom_XOR_16bit(p_nubit16(ccpuins_deCodeBlock.prm), ccpuins_deCodeBlock.immData_16Bit);
}
void ccpuins_group_XOR_Ev_Ib()
{
	//vapiCallBackMachineStop();;//may be some problem
	t_nubit16 tmp = ccpuins_deCodeBlock.immData_8Bit;
	ccpuins_atom_XOR_16bit(p_nubit16(ccpuins_deCodeBlock.prm), tmp);
}

void ccpuins_group_CMP_Eb_Ib()
{
	ccpuins_atom_CMP_8bit(d_nubit8(ccpuins_deCodeBlock.prm), d_nsbit8(&ccpuins_deCodeBlock.immData_8Bit));
}
void ccpuins_group_CMP_Ev_Iv()
{
/*	if ( (d_nsbit16(&ccpuins_deCodeBlock.immData_16Bit)  == 0x5052)) {
			FILE *fp = fopen("d:/temp2.log","w");
			fprintf(fp,"%x\n", (t_nubit8 *)ccpuins_deCodeBlock.prm - (t_nubit8 *)p_nubit8(cramGetAddr(0,0)) );
			fclose(fp);
	}*/
	ccpuins_atom_CMP_16bit(d_nubit16(ccpuins_deCodeBlock.prm), d_nsbit16(&ccpuins_deCodeBlock.immData_16Bit));
}
void ccpuins_group_CMP_Ev_Ib()
{
	//vapiCallBackMachineStop();;//may be some problem
	ccpuins_atom_CMP_16bit(d_nubit16(ccpuins_deCodeBlock.prm),  (t_nsbit8)ccpuins_deCodeBlock.immData_8Bit);
}
////////////////////////////////////////////</GROUP1>

////////////////////////////////////////////<GROUP2> :DONE
static void ccpuins_group_SHL_Eb_1_atom()
{
	t_nubit8  oldEb  = d_nubit8(ccpuins_deCodeBlock.prm);
	t_bool old_sign = getSignByData_data8(oldEb);
	ccpu_flag_SetCF(old_sign);
	(d_nubit8(ccpuins_deCodeBlock.prm)) <<= 1;
	ccpu_flag_SetOF(isSignBitChange_data8(oldEb, d_nubit8(ccpuins_deCodeBlock.prm)));
	ccpu_flag_SetSF_ZF_PF_data8(d_nubit8(ccpuins_deCodeBlock.prm));
}
static void ccpuins_group_SHL_Ev_1_atom()
{
	t_nubit16  oldEv  = d_nubit16(ccpuins_deCodeBlock.prm);
	t_bool old_sign = getSignByData_data16( oldEv );
	ccpu_flag_SetCF(old_sign);
	(d_nubit16(ccpuins_deCodeBlock.prm)) <<= 1;
	ccpu_flag_SetOF(isSignBitChange_data16(oldEv, d_nubit16(ccpuins_deCodeBlock.prm)));
	ccpu_flag_SetSF_ZF_PF_data16(d_nubit16(ccpuins_deCodeBlock.prm));
}
static void ccpuins_group_SHR_Eb_1_atom()
{
	t_nubit8 oldEb = d_nubit8(ccpuins_deCodeBlock.prm);
	t_bool finalBit = getFinalBit_data8(oldEb);
	d_nubit8(ccpuins_deCodeBlock.prm) >>= 1;
	ccpu_flag_SetCF(finalBit);
	ccpu_flag_SetOF(isSignBitChange_data8(oldEb, d_nubit8(ccpuins_deCodeBlock.prm)));
	ccpu_flag_SetSF_ZF_PF_data8(d_nubit8(ccpuins_deCodeBlock.prm));
}
static void ccpuins_group_SHR_Ev_1_atom()
{
	t_nsbit16 oldEv = d_nsbit16(ccpuins_deCodeBlock.prm);
	t_bool finalBit = getFinalBit_data16( oldEv );
	d_nubit16(ccpuins_deCodeBlock.prm) >>= 1;
	ccpu_flag_SetCF(finalBit);
	ccpu_flag_SetOF(isSignBitChange_data16(oldEv, d_nubit16(ccpuins_deCodeBlock.prm)));
	ccpu_flag_SetSF_ZF_PF_data16(d_nubit16(ccpuins_deCodeBlock.prm));
}
static void ccpuins_group_ROL_Eb_1_atom()
{
	t_nubit8  oldEb  = d_nubit8(ccpuins_deCodeBlock.prm);
	t_bool old_sign = getSignByData_data8( oldEb );
	ccpu_flag_SetCF(old_sign);
	(d_nubit8(ccpuins_deCodeBlock.prm)) = ((d_nubit8(ccpuins_deCodeBlock.prm))<<1) | ((d_nubit8(ccpuins_deCodeBlock.prm))>>7) ;
	ccpu_flag_SetOF(isSignBitChange_data8(oldEb, d_nubit8(ccpuins_deCodeBlock.prm)));
}
static void ccpuins_group_ROL_Ev_1_atom()
{
	t_nubit16  oldEv  = d_nubit16(ccpuins_deCodeBlock.prm);
	t_bool old_sign = getSignByData_data16( oldEv );
	ccpu_flag_SetCF(old_sign);
	(d_nubit16(ccpuins_deCodeBlock.prm)) = ((d_nubit16(ccpuins_deCodeBlock.prm))<<1) | ((d_nubit16(ccpuins_deCodeBlock.prm))>>15) ;
	ccpu_flag_SetOF(isSignBitChange_data16(oldEv, d_nubit16(ccpuins_deCodeBlock.prm)));
}
static void ccpuins_group_ROR_Eb_1_atom()
{
	t_nubit8  oldEb  = d_nubit8(ccpuins_deCodeBlock.prm);
	t_bool finalBit = getFinalBit_data8( oldEb );
	ccpu_flag_SetCF( finalBit );
	(d_nubit8(ccpuins_deCodeBlock.prm)) = ((d_nubit8(ccpuins_deCodeBlock.prm))>>1) | ((d_nubit8(ccpuins_deCodeBlock.prm))<<7) ;
	ccpu_flag_SetOF(isSignBitChange_data8(oldEb, d_nubit8(ccpuins_deCodeBlock.prm)));
	//ccpu_flag_SetSF_ZF_PF_data8(d_nubit8(ccpuins_deCodeBlock.prm));
}
static void ccpuins_group_ROR_Ev_1_atom()
{
	t_nubit16  oldEv  = d_nubit16(ccpuins_deCodeBlock.prm);
	t_bool finalBit = getFinalBit_data16( oldEv );
	ccpu_flag_SetCF( finalBit );
	(d_nubit16(ccpuins_deCodeBlock.prm)) = ((d_nubit16(ccpuins_deCodeBlock.prm))>>1) | ((d_nubit16(ccpuins_deCodeBlock.prm))<<15) ;
	ccpu_flag_SetOF(isSignBitChange_data16(oldEv, d_nubit16(ccpuins_deCodeBlock.prm)));
	//ccpu_flag_SetSF_ZF_PF_data16(d_nubit16(ccpuins_deCodeBlock.prm));
}
static void ccpuins_group_RCL_Eb_1_atom()
{
	t_bool newCF, newOF;
	t_nubit16 tmpFlags = ccpu.flags;
	ccpuins_group_SHL_Eb_1_atom();
	newCF = GetCF;
	newOF = GetOF;
	ccpu_flag_SetFlags(tmpFlags);
	if(GetCF)
	{
		d_nubit8(ccpuins_deCodeBlock.prm) |= MASK_00000001;
	}
	ccpu_flag_SetCF(newCF);
	ccpu_flag_SetOF(newOF);
	//ccpu_flag_SetSF_ZF_PF_data8(d_nubit8(ccpuins_deCodeBlock.prm));
}
static void ccpuins_group_RCL_Ev_1_atom()
{
	t_bool newCF, newOF;
	t_nubit16 tmpFlags = ccpu.flags;
	ccpuins_group_SHL_Ev_1_atom();
	newCF = GetCF;
	newOF = GetOF;
	ccpu_flag_SetFlags(tmpFlags);
	if(GetCF)
	{
		d_nubit16(ccpuins_deCodeBlock.prm) |= 1;
	}
	ccpu_flag_SetCF(newCF);
	ccpu_flag_SetOF(newOF);
	//ccpu_flag_SetSF_ZF_PF_data16(d_nubit16(ccpuins_deCodeBlock.prm));
}
static void ccpuins_group_RCR_Eb_1_atom()
{
	t_nubit8 oldEb = d_nubit8(ccpuins_deCodeBlock.prm);
	t_bool newCF, oldCF = GetCF;
	t_nubit16 tmpFlags = ccpu.flags;
	ccpuins_group_SHR_Eb_1_atom();
	newCF = GetCF;
	ccpu_flag_SetFlags(tmpFlags);//因为ccpuins_group_SHR_Eb_1函数会改变Flags值，而实际上RCR是不影响那几位的。。
	if(oldCF)
	{
		d_nubit8(ccpuins_deCodeBlock.prm) |= MASK_10000000;
	}
	ccpu_flag_SetOF(isSignBitChange_data8(oldEb, d_nubit8(ccpuins_deCodeBlock.prm)));
	ccpu_flag_SetCF(newCF);
	//ccpu_flag_SetSF_ZF_PF_data8(d_nubit8(ccpuins_deCodeBlock.prm));
}
static void ccpuins_group_RCR_Ev_1_atom()
{
	t_nubit16 oldEv = d_nubit16(ccpuins_deCodeBlock.prm);
	t_nubit16 tmpFlags = ccpu.flags;
	t_bool newCF, oldCF = GetCF;
	ccpuins_group_SHR_Ev_1_atom();
	newCF = GetCF;
	ccpu_flag_SetFlags(tmpFlags);
	if(oldCF)
	{
		d_nubit16(ccpuins_deCodeBlock.prm) |= 0x8000;
	}
	ccpu_flag_SetOF(isSignBitChange_data16(oldEv, d_nubit16(ccpuins_deCodeBlock.prm)));
	ccpu_flag_SetCF(newCF);
	//ccpu_flag_SetSF_ZF_PF_data16(d_nubit16(ccpuins_deCodeBlock.prm));
}

void ccpuins_group_ROL_Eb_Ib(){}
void ccpuins_group_ROL_Ev_Ib(){}
void ccpuins_group_ROL_Eb_1()
{
	t_nubit8  oldEb  = d_nubit8(ccpuins_deCodeBlock.prm);
	ccpuins_group_ROL_Eb_1_atom();
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit8(ccpuins_deCodeBlock.prm) = oldEb;
#endif
}
void ccpuins_group_ROL_Ev_1()
{
	t_nubit16  oldEv  = d_nubit16(ccpuins_deCodeBlock.prm);
	ccpuins_group_ROL_Ev_1_atom();
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit16(ccpuins_deCodeBlock.prm) = oldEv;
#endif
}
void ccpuins_group_ROL_Eb_CL()
{
	t_nubit8  oldEb  = d_nubit8(ccpuins_deCodeBlock.prm);
	t_bool oldOF = GetOF;
	t_nubit8 tmpCL = ccpu.cl;
	if(0 == ccpu.cl)
		return;
	while(tmpCL)
	{
		ccpuins_group_ROL_Eb_1_atom();
		tmpCL--;
	}
	ccpu_flag_SetOF(oldOF);
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit8(ccpuins_deCodeBlock.prm) = oldEb;
#endif
	//ccpu_flag_SetSF_ZF_PF_data8(d_nubit8(ccpuins_deCodeBlock.prm));
}
void ccpuins_group_ROL_Ev_CL()
{
	t_nubit16  oldEv  = d_nubit16(ccpuins_deCodeBlock.prm);
	t_bool oldOF = GetOF;
	t_nubit8 tmpCL = ccpu.cl;
	if(0 == ccpu.cl)
		return;
	while(tmpCL)
	{
		ccpuins_group_ROL_Ev_1_atom();
		tmpCL--;
	}
	ccpu_flag_SetOF(oldOF);
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit16(ccpuins_deCodeBlock.prm) = oldEv;
#endif
	//ccpu_flag_SetSF_ZF_PF_data16(d_nubit16(ccpuins_deCodeBlock.prm));
}

void ccpuins_group_ROR_Eb_Ib(){}
void ccpuins_group_ROR_Ev_Ib(){}
void ccpuins_group_ROR_Eb_1()
{
	t_nubit8  oldEb  = d_nubit8(ccpuins_deCodeBlock.prm);
	ccpuins_group_ROR_Eb_1_atom();
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit8(ccpuins_deCodeBlock.prm) = oldEb;
#endif
}
void ccpuins_group_ROR_Ev_1()
{
	t_nubit16  oldEv  = d_nubit16(ccpuins_deCodeBlock.prm);
	ccpuins_group_ROR_Ev_1_atom();
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit16(ccpuins_deCodeBlock.prm) = oldEv;
#endif
}
void ccpuins_group_ROR_Eb_CL()
{
	t_nubit8  oldEb  = d_nubit8(ccpuins_deCodeBlock.prm);
	t_bool oldOF = GetOF;
	t_nubit8 tmpCL = ccpu.cl;
	if(0 == ccpu.cl)
		return;
	while(tmpCL)
	{
		ccpuins_group_ROR_Eb_1_atom();
		tmpCL--;
	}
	ccpu_flag_SetOF(oldOF);
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit8(ccpuins_deCodeBlock.prm) = oldEb;
#endif
}
void ccpuins_group_ROR_Ev_CL()
{
	t_nubit16  oldEv  = d_nubit16(ccpuins_deCodeBlock.prm);
	t_bool oldOF = GetOF;
	t_nubit8 tmpCL = ccpu.cl;
	if(0 == ccpu.cl)
		return;
	while(tmpCL)
	{
		ccpuins_group_ROR_Ev_1_atom();
		tmpCL--;
	}
	ccpu_flag_SetOF(oldOF);
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit16(ccpuins_deCodeBlock.prm) = oldEv;
#endif
}

void ccpuins_group_RCL_Eb_Ib(){}
void ccpuins_group_RCL_Ev_Ib(){}
void ccpuins_group_RCL_Eb_1()
{
	t_nubit8 oldEb = d_nubit8(ccpuins_deCodeBlock.prm);
	ccpuins_group_RCL_Eb_1_atom();
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit8(ccpuins_deCodeBlock.prm) = oldEb;
#endif
}
void ccpuins_group_RCL_Ev_1()
{
	t_nubit16 oldEv = d_nubit16(ccpuins_deCodeBlock.prm);
	ccpuins_group_RCL_Ev_1_atom();
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit16(ccpuins_deCodeBlock.prm) = oldEv;
#endif
}
void ccpuins_group_RCL_Eb_CL()
{
	t_nubit8 oldEb = d_nubit8(ccpuins_deCodeBlock.prm);
	t_bool oldOF = GetOF;
	t_nubit8 tmpCL = ccpu.cl;
	if(0 == ccpu.cl)
		return;
	while(tmpCL)
	{
		ccpuins_group_RCL_Eb_1_atom();
		tmpCL--;
	}
	if (ccpu.cl != 1) ccpu_flag_SetOF(oldOF);
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit8(ccpuins_deCodeBlock.prm) = oldEb;
#endif
}
void ccpuins_group_RCL_Ev_CL()
{
	t_nubit16 oldEv = d_nubit16(ccpuins_deCodeBlock.prm);
	t_bool oldOF = GetOF;
	t_nubit8 tmpCL = ccpu.cl;
	if(0 == ccpu.cl)
		return;
	while(tmpCL)
	{
		ccpuins_group_RCL_Ev_1_atom();
		tmpCL--;
	}
	if (ccpu.cl != 1) ccpu_flag_SetOF(oldOF);
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit16(ccpuins_deCodeBlock.prm) = oldEv;
#endif
}

void ccpuins_group_RCR_Eb_Ib(){}
void ccpuins_group_RCR_Ev_Ib(){}
void ccpuins_group_RCR_Eb_1()
{
	t_nubit8 oldEb = d_nubit8(ccpuins_deCodeBlock.prm);
	ccpuins_group_RCR_Eb_1_atom();
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit8(ccpuins_deCodeBlock.prm) = oldEb;
#endif
}
void ccpuins_group_RCR_Ev_1()
{
	t_nubit16 oldEv = d_nubit16(ccpuins_deCodeBlock.prm);
	ccpuins_group_RCR_Ev_1_atom();
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit16(ccpuins_deCodeBlock.prm) = oldEv;
#endif
}
void ccpuins_group_RCR_Eb_CL()
{
	t_nubit8 oldEb = d_nubit8(ccpuins_deCodeBlock.prm);
	t_bool oldOF = GetOF;
	t_nubit8 tmpCL = ccpu.cl;
	if(0 == ccpu.cl)
		return;
// NEKO DEBUG
	while(tmpCL)
	{
		ccpuins_group_RCR_Eb_1_atom();
		tmpCL--;
	}
// NEKO DEBUG
	if(1 != ccpu.cl) ccpu_flag_SetOF(oldOF);
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit8(ccpuins_deCodeBlock.prm) = oldEb;
#endif
}
void ccpuins_group_RCR_Ev_CL()
{
	t_nubit16 oldEv = d_nubit16(ccpuins_deCodeBlock.prm);
	t_bool oldOF = GetOF;
	t_nubit8 tmpCL = ccpu.cl;
	if(0 == ccpu.cl)
		return;
	while(tmpCL)
	{
		ccpuins_group_RCR_Ev_1_atom();
		tmpCL--;
	}
// NEKO DEBUG
	if(1 != ccpu.cl) ccpu_flag_SetOF(oldOF);
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit16(ccpuins_deCodeBlock.prm) = oldEv;
#endif
}

void ccpuins_group_SHL_Eb_Ib(){}
void ccpuins_group_SHL_Ev_Ib(){}
void ccpuins_group_SHL_Eb_1()
{	
	t_nubit8  oldEb  = d_nubit8(ccpuins_deCodeBlock.prm);
	ccpuins_group_SHL_Eb_1_atom();
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit8(ccpuins_deCodeBlock.prm) = oldEb;
#endif
}
void ccpuins_group_SHL_Ev_1()
{
	t_nubit16  oldEv  = d_nubit16(ccpuins_deCodeBlock.prm);
	ccpuins_group_SHL_Ev_1_atom();
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit16(ccpuins_deCodeBlock.prm) = oldEv;
#endif
}
void ccpuins_group_SHL_Eb_CL()
{
	t_nubit8 oldEb = d_nubit8(ccpuins_deCodeBlock.prm);
	t_bool sign = 0x00;
	t_nubit8 tmpCL = ccpu.cl;
	if(0 == ccpu.cl)
		return;
	while(tmpCL)
	{
		sign = getSignByData_data8( d_nubit8(ccpuins_deCodeBlock.prm) );
		(d_nubit8(ccpuins_deCodeBlock.prm)) <<= 1; //低速实现，以后修改
		tmpCL--;
	}
	ccpu_flag_SetCF(sign);
	ccpu_flag_SetSF_ZF_PF_data8(d_nubit8(ccpuins_deCodeBlock.prm));
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit8(ccpuins_deCodeBlock.prm) = oldEb;
#endif
}
void ccpuins_group_SHL_Ev_CL()
{
	t_nsbit16 oldEv = d_nsbit16(ccpuins_deCodeBlock.prm);
	t_bool sign = 0x00;
	t_nubit8 tmpCL = ccpu.cl;
	if(0 == ccpu.cl)
		return;
	while(tmpCL)
	{
		sign = getSignByData_data16( d_nubit16(ccpuins_deCodeBlock.prm) );
		(d_nubit16(ccpuins_deCodeBlock.prm)) <<= 1; //低速实现，以后修改
		tmpCL--;
	}
	ccpu_flag_SetCF(sign);
	ccpu_flag_SetSF_ZF_PF_data16(d_nubit16(ccpuins_deCodeBlock.prm));
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit16(ccpuins_deCodeBlock.prm) = oldEv;
#endif
}

void ccpuins_group_SHR_Eb_Ib(){}
void ccpuins_group_SHR_Ev_Ib(){}
void ccpuins_group_SHR_Eb_1()
{
	t_nubit8 oldEb = d_nubit8(ccpuins_deCodeBlock.prm);
	ccpuins_group_SHR_Eb_1_atom();
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit8(ccpuins_deCodeBlock.prm) = oldEb;
#endif
}
void ccpuins_group_SHR_Ev_1()
{
	t_nsbit16 oldEv = d_nsbit16(ccpuins_deCodeBlock.prm);
	ccpuins_group_SHR_Ev_1_atom();
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit16(ccpuins_deCodeBlock.prm) = oldEv;
#endif
}
void ccpuins_group_SHR_Eb_CL()
{
	t_nubit8 oldEb = d_nubit8(ccpuins_deCodeBlock.prm);
	t_bool finalBit = 0x00;
	t_nubit8 tmpCL = ccpu.cl;
	if(0 == ccpu.cl)
		return;
	while(tmpCL)
	{
		finalBit = getFinalBit_data8(d_nubit8(ccpuins_deCodeBlock.prm));
		d_nubit8(ccpuins_deCodeBlock.prm) >>= 1;
		tmpCL--;
	}
	ccpu_flag_SetCF(finalBit);
// NEKO DEBUG: ADD THE NEXT LINE
	if (ccpu.cl == 1) ccpu_flag_SetOF(isSignBitChange_data8(oldEb, d_nubit8(ccpuins_deCodeBlock.prm)));
	ccpu_flag_SetSF_ZF_PF_data8(d_nubit8(ccpuins_deCodeBlock.prm));
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit8(ccpuins_deCodeBlock.prm) = oldEb;
#endif
}
void ccpuins_group_SHR_Ev_CL()
{
	t_nsbit16 oldEv = d_nsbit16(ccpuins_deCodeBlock.prm);
	t_bool finalBit = 0x00;
	t_nubit8 tmpCL = ccpu.cl;
	if(0 == ccpu.cl)
		return;
	while(tmpCL)
	{
		finalBit = getFinalBit_data16(d_nubit16(ccpuins_deCodeBlock.prm));
		d_nubit16(ccpuins_deCodeBlock.prm) >>= 1;
		tmpCL--;
	}
	ccpu_flag_SetCF(finalBit);
// NEKO DEBUG: ADD THE NEXT LINE
	if (ccpu.cl == 1) ccpu_flag_SetOF(isSignBitChange_data16(oldEv, d_nubit16(ccpuins_deCodeBlock.prm)));
	ccpu_flag_SetSF_ZF_PF_data16(d_nubit16(ccpuins_deCodeBlock.prm));
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit16(ccpuins_deCodeBlock.prm) = oldEv;
#endif
}

void ccpuins_group_SAR_Eb_Ib(){}
void ccpuins_group_SAR_Ev_Ib(){}
void ccpuins_group_SAR_Eb_1()
{
	t_nsbit8 oldEb = d_nsbit8(ccpuins_deCodeBlock.prm);
	t_bool finalBit = getFinalBit_data8( (t_nubit8)oldEb );
	d_nsbit8(ccpuins_deCodeBlock.prm) >>= 1;
	ccpu_flag_SetCF(finalBit);
	ccpu_flag_SetOF(0x00);//算术右移，部队符号位改变
	ccpu_flag_SetSF_ZF_PF_data8(d_nubit8(ccpuins_deCodeBlock.prm));
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit8(ccpuins_deCodeBlock.prm) = oldEb;
#endif
}
void ccpuins_group_SAR_Ev_1()
{
	t_nsbit16 oldEv = d_nsbit16(ccpuins_deCodeBlock.prm);
	t_bool finalBit = getFinalBit_data8( (t_nubit8)oldEv );
	d_nsbit16(ccpuins_deCodeBlock.prm) >>= 1;
	ccpu_flag_SetCF(finalBit);
	ccpu_flag_SetOF(0x00);
	ccpu_flag_SetSF_ZF_PF_data16(d_nubit16(ccpuins_deCodeBlock.prm));
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit16(ccpuins_deCodeBlock.prm) = oldEv;
#endif
}
void ccpuins_group_SAR_Eb_CL()
{
	t_nubit8 oldEb = d_nubit8(ccpuins_deCodeBlock.prm);
	t_bool finalBit = 0x00;
	t_nubit8 tmpCL = ccpu.cl;
	if(0 == ccpu.cl)
		return;
	while(tmpCL)
	{
		finalBit = getFinalBit_data8(d_nubit8(ccpuins_deCodeBlock.prm));
		d_nsbit8(ccpuins_deCodeBlock.prm) >>= 1;
		tmpCL--;
	}
	ccpu_flag_SetCF(finalBit);
	ccpu_flag_SetOF(0x00);
	ccpu_flag_SetSF_ZF_PF_data8(d_nubit8(ccpuins_deCodeBlock.prm));
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit8(ccpuins_deCodeBlock.prm) = oldEb;
#endif
}
void ccpuins_group_SAR_Ev_CL()
{
	t_nubit16 oldEv = d_nubit16(ccpuins_deCodeBlock.prm);
	t_bool finalBit = 0x00;
	t_nubit8 tmpCL = ccpu.cl;
	if(0 == ccpu.cl)
		return;
	while(tmpCL)
	{
		finalBit = getFinalBit_data16(d_nubit16(ccpuins_deCodeBlock.prm));
		d_nsbit16(ccpuins_deCodeBlock.prm) >>= 1;
		tmpCL--;
	}
	ccpu_flag_SetCF(finalBit);
	ccpu_flag_SetOF(0x00);
	ccpu_flag_SetSF_ZF_PF_data16(d_nubit16(ccpuins_deCodeBlock.prm));
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) d_nubit16(ccpuins_deCodeBlock.prm) = oldEv;
#endif
}
///////////////////////////////////////</Group2>

////////////////////////////////////<Group3> :DONE
void ccpuins_group_TEST_Ib()
{
	ccpuins_atom_TEST_8bit(p_nubit8(ccpuins_deCodeBlock.prm), ccpuins_deCodeBlock.immData_8Bit);
}
void ccpuins_group_TEST_Iv()
{
	ccpuins_atom_TEST_16bit(p_nubit16(ccpuins_deCodeBlock.prm), ccpuins_deCodeBlock.immData_16Bit);
}
void ccpuins_group_NOT_8bit()
{
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) return;
#endif
	d_nubit8(ccpuins_deCodeBlock.prm) = ~d_nubit8(ccpuins_deCodeBlock.prm);
}
void ccpuins_group_NOT_16bit()
{
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) return;
#endif
	d_nubit16(ccpuins_deCodeBlock.prm) = ~d_nubit16(ccpuins_deCodeBlock.prm);
}
void ccpuins_group_NEG_8bit()
{
	t_nubit8 zero = 0;
	ccpuins_atom_SUB_8bit(&zero, d_nubit8(ccpuins_deCodeBlock.prm));
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) return;
#endif
	d_nubit8(ccpuins_deCodeBlock.prm) = zero;
}
void ccpuins_group_NEG_16bit()
{
	t_nubit16 zero = 0;
	ccpuins_atom_SUB_16bit(&zero, d_nubit16(ccpuins_deCodeBlock.prm));
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) return;
#endif
	d_nubit16(ccpuins_deCodeBlock.prm) = zero;
}
void ccpuins_group_MUL_AL()
{
	ccpu.ax = d_nubit8(ccpuins_deCodeBlock.prm) * ccpu.al;
	ccpu_flag_SetCF(!!(ccpu.ah));
	ccpu_flag_SetOF(!!(ccpu.ah));
}
void ccpuins_group_MUL_eAX()
{
	t_nubit32 result = d_nubit16(ccpuins_deCodeBlock.prm) * ccpu.ax;
	ccpu.dx = (t_nubit16)(result >> 16);
	ccpu.ax = (t_nubit16)(result & 0x0000ffff);

	ccpu_flag_SetCF(!!(ccpu.dx));
	ccpu_flag_SetOF(!!(ccpu.dx));
}
void ccpuins_group_IMUL_AL()
{
	ccpu.ax = d_nsbit8(ccpuins_deCodeBlock.prm) * (t_nsbit8)(ccpu.al);
	if(ccpu.ax == ccpu.al)
	{
		ccpu_flag_SetCF(0x00);
		ccpu_flag_SetOF(0x00);
	}
	else
	{
		ccpu_flag_SetCF(0x01);
		ccpu_flag_SetOF(0x01);
	}
}
void ccpuins_group_IMUL_eAX()
{
	t_nsbit32 result = d_nsbit16(ccpuins_deCodeBlock.prm) * (t_nsbit16)(ccpu.ax);
	ccpu.dx = (t_nubit16)(result >> 16);
	ccpu.ax = (t_nubit16)(result & 0x0000ffff);
	if(result == (t_nsbit32)(ccpu.ax))
	{
		ccpu_flag_SetCF(0x00);
		ccpu_flag_SetOF(0x00);
	}
	else
	{
		ccpu_flag_SetCF(0x01);
		ccpu_flag_SetOF(0x01);
	}
}
void ccpuins_group_DIV_AL()
{
	t_nubit16 tmpAX_unsigned;
	if(0 == (d_nubit8(ccpuins_deCodeBlock.prm)) )
	{//如果除数是零，产生零号中断
		ccpuins_atom_INT(0);
		return ;
	}
	tmpAX_unsigned = ccpu.ax;
	ccpu.al = tmpAX_unsigned / (d_nubit8(ccpuins_deCodeBlock.prm)); //商
	ccpu.ah = tmpAX_unsigned % (d_nubit8(ccpuins_deCodeBlock.prm));//余数
}
void ccpuins_group_DIV_eAX()
{
	t_nubit32 data;
	if(0 == (d_nubit16(ccpuins_deCodeBlock.prm)) )
	{//如果除数是零，产生零号中断
		ccpuins_atom_INT(0);
		return;
	}
	data = (((t_nubit32)(ccpu.dx))<<16) + ccpu.ax;
	ccpu.ax = data / (d_nubit16(ccpuins_deCodeBlock.prm)); //商
	ccpu.dx = data % (d_nubit16(ccpuins_deCodeBlock.prm));//余数

}
void ccpuins_group_IDIV_AL()
{
	t_nsbit16 tmpAX_signed;
	if(0 == (d_nsbit8(ccpuins_deCodeBlock.prm)) )
	{//如果除数是零，产生零号中断
		ccpuins_atom_INT(0);
		return ;
	}
	tmpAX_signed = ccpu.ax;
	ccpu.al = tmpAX_signed / (d_nsbit8(ccpuins_deCodeBlock.prm)); //商
	ccpu.ah = tmpAX_signed % (d_nsbit8(ccpuins_deCodeBlock.prm));//余数
}
void ccpuins_group_IDIV_eAX()
{
	t_nsbit32 data;
	if(0 == (d_nsbit16(ccpuins_deCodeBlock.prm)) )
	{//如果除数是零，产生零号中断
		ccpuins_atom_INT(0);
		return ;
	}

	data = (((t_nubit32)(ccpu.dx))<<16) + ccpu.ax;
	ccpu.ax = data / (d_nsbit16(ccpuins_deCodeBlock.prm)); //商
	ccpu.dx = data % (d_nsbit16(ccpuins_deCodeBlock.prm));//余数
}
//////////////////////////////////////</Group3>

//////////////////////////////////////////<Group4--5> :DONE
void ccpuins_group_INC_Eb()
{
	ccpuins_atom_INC_8bit(p_nubit8(ccpuins_deCodeBlock.prm));
}
void ccpuins_group_INC_Ev()
{
	ccpuins_atom_INC_16bit(p_nubit16(ccpuins_deCodeBlock.prm));
}
void ccpuins_group_DEC_Eb()
{
	ccpuins_atom_DEC_8bit(p_nubit8(ccpuins_deCodeBlock.prm));
}
void ccpuins_group_DEC_Ev()
{
	ccpuins_atom_DEC_16bit(p_nubit16(ccpuins_deCodeBlock.prm));
}
void ccpuins_group_CALL_Ev()//nnn=010 段内间接调用
{
	ccpuins_atom_PUSH(ccpu.ip); // 没有PUSH IP 。。所以用这个来代替
	ccpu.ip = d_nubit16(ccpuins_deCodeBlock.prm);//near call absolute
}
void ccpuins_group_CALL_Ep()//nnn=011 段间间接调用
{
	ccpuins_atom_PUSH(ccpu.cs);
	ccpuins_atom_PUSH(ccpu.ip); // 没有PUSH IP 。。所以用这个来代替
	ccpu.ip = d_nubit16(ccpuins_deCodeBlock.prm);
	ccpu.cs = d_nubit16((t_vaddrcc)ccpuins_deCodeBlock.prm + 2);
}
void ccpuins_group_JMP_Ev()//段内间接转移
{
	ccpu.ip = d_nubit16(ccpuins_deCodeBlock.prm);
}
void ccpuins_group_JMP_Ep()//段间间接转移
{
	ccpu.cs = *(((t_nubit16*)ccpuins_deCodeBlock.prm) + 1);
	ccpu.ip = d_nubit16(ccpuins_deCodeBlock.prm);

}
void ccpuins_group_PUSH_Ev()
{
	ccpuins_atom_PUSH(d_nubit16(ccpuins_deCodeBlock.prm));
}
//////////////////////////////////////////</Group4--5>

//////////////////////////////////////////////////////////////////////////////

void ccpuins_extern_ADD_Eb_Gb()
{
	ccpuins_atom_ADD_8bit(p_nubit8(ccpuins_deCodeBlock.prm), d_nubit8(ccpuins_deCodeBlock.preg));
}
void ccpuins_extern_ADD_Ev_Gv()
{
	ccpuins_atom_ADD_16bit(p_nubit16(ccpuins_deCodeBlock.prm), d_nubit16(ccpuins_deCodeBlock.preg));
}
void ccpuins_extern_ADD_Gb_Eb()
{
	ccpuins_atom_ADD_8bit(p_nubit8(ccpuins_deCodeBlock.preg), d_nubit8(ccpuins_deCodeBlock.prm));
}
void ccpuins_extern_ADD_Gv_Ev()
{
	ccpuins_atom_ADD_16bit(p_nubit16(ccpuins_deCodeBlock.preg), d_nubit16(ccpuins_deCodeBlock.prm));
}
void ccpuins_extern_ADD_AL_Ib()
{
	ccpuins_atom_ADD_8bit(&ccpu.al, d_nubit8(&ccpuins_deCodeBlock.immData_8Bit));
}
void ccpuins_extern_ADD_eAX_Iv()
{
	ccpuins_atom_ADD_16bit(&ccpu.ax, d_nubit16(&ccpuins_deCodeBlock.immData_16Bit));
}

void ccpuins_extern_PUSH_ES()//0x06
{
	ccpuins_atom_PUSH(ccpu.es);
}
void ccpuins_extern_POP_ES()
{
	ccpu.es = ccpuins_atom_POP();
}

void ccpuins_extern_ADC_Eb_Gb()
{
	ccpuins_atom_ADC_8bit(p_nubit8(ccpuins_deCodeBlock.prm), d_nubit8(ccpuins_deCodeBlock.preg));
}
void ccpuins_extern_ADC_Ev_Gv()
{
	ccpuins_atom_ADC_16bit(p_nubit16(ccpuins_deCodeBlock.prm), d_nubit16(ccpuins_deCodeBlock.preg));
}
void ccpuins_extern_ADC_Gb_Eb()
{
	ccpuins_atom_ADC_8bit(p_nubit8(ccpuins_deCodeBlock.preg), d_nubit8(ccpuins_deCodeBlock.prm));
}
void ccpuins_extern_ADC_Gv_Ev()
{
	ccpuins_atom_ADC_16bit(p_nubit16(ccpuins_deCodeBlock.preg), d_nubit16(ccpuins_deCodeBlock.prm));
}
void ccpuins_extern_ADC_AL_Ib()
{
	ccpuins_atom_ADC_8bit(&ccpu.al, ccpuins_deCodeBlock.immData_8Bit);
}
void ccpuins_extern_ADC_eAX_Iv()
{
	ccpuins_atom_ADC_16bit(&ccpu.ax, ccpuins_deCodeBlock.immData_16Bit);
}

void ccpuins_extern_PUSH_SS()
{
	ccpuins_atom_PUSH(ccpu.ss);
}
void ccpuins_extern_POP_SS()
{
	ccpu.ss = ccpuins_atom_POP();
}

void ccpuins_extern_AND_Eb_Gb()
{
	ccpuins_atom_AND_8bit(p_nubit8(ccpuins_deCodeBlock.prm), d_nubit8(ccpuins_deCodeBlock.preg));
}
void ccpuins_extern_AND_Ev_Gv()
{
	ccpuins_atom_AND_16bit(p_nubit16(ccpuins_deCodeBlock.prm), d_nubit16(ccpuins_deCodeBlock.preg));
}
void ccpuins_extern_AND_Gb_Eb()
{
	ccpuins_atom_AND_8bit(p_nubit8(ccpuins_deCodeBlock.preg), d_nubit8(ccpuins_deCodeBlock.prm));
}
void ccpuins_extern_AND_Gv_Ev()
{
	ccpuins_atom_AND_16bit(p_nubit16(ccpuins_deCodeBlock.preg), d_nubit16(ccpuins_deCodeBlock.prm));
}
void ccpuins_extern_AND_AL_Ib()
{
	ccpuins_atom_AND_8bit(&ccpu.al, ccpuins_deCodeBlock.immData_8Bit);
}
void ccpuins_extern_AND_eAX_Iv()
{
	ccpuins_atom_AND_16bit(&ccpu.ax, ccpuins_deCodeBlock.immData_16Bit);
}

void ccpuins_extern_SEG_ES()
{
	ccpuins_deCodeBlock.prefix_SegmentOverrideOpCode = ES_SEGMENT_OVERRIDE_PREFIX;
}
void ccpuins_extern_DAA()//0x27
{
	t_nubit8 old_al = ccpu.al;
	t_bool old_CF = GetCF;

	ccpu_flag_SetCF(0x00);
	if((ccpu.al & 0x0f)>9 || (0x01 ==GetAF))
	{
		ccpuins_atom_ADD_8bit(&ccpu.al, 6);
		ccpu_flag_SetCF(old_CF); //这里可能会有点问题
		ccpu_flag_SetAF(0x01);
	}
	else
	{
		ccpu_flag_SetAF(0x00);
	}
    //*****
	if((old_al>0x99) || (0x01 == old_CF))
	{
		ccpuins_atom_ADD_8bit(&ccpu.al, 0x60);
		ccpu_flag_SetCF(0x01);

	}
	else
	{
		ccpu_flag_SetCF(0x00);
	}

}

void ccpuins_extern_XOR_Eb_Gb()
{
	ccpuins_atom_XOR_8bit(p_nubit8(ccpuins_deCodeBlock.prm), d_nubit8(ccpuins_deCodeBlock.preg));
}
void ccpuins_extern_XOR_Ev_Gv()
{
	ccpuins_atom_XOR_16bit(p_nubit16(ccpuins_deCodeBlock.prm), d_nubit16(ccpuins_deCodeBlock.preg));
}
void ccpuins_extern_XOR_Gb_Eb()
{
	ccpuins_atom_XOR_8bit(p_nubit8(ccpuins_deCodeBlock.preg), d_nubit8(ccpuins_deCodeBlock.prm));
}
void ccpuins_extern_XOR_Gv_Ev()
{
	ccpuins_atom_XOR_16bit(p_nubit16(ccpuins_deCodeBlock.preg), d_nubit16(ccpuins_deCodeBlock.prm));
}
void ccpuins_extern_XOR_AL_Ib()
{
	ccpuins_atom_XOR_8bit(&ccpu.al, ccpuins_deCodeBlock.immData_8Bit);
}
void ccpuins_extern_XOR_eAX_Iv()
{
	ccpuins_atom_XOR_16bit(&ccpu.ax, ccpuins_deCodeBlock.immData_16Bit);
}

void ccpuins_extern_SEG_SS()
{
	ccpuins_deCodeBlock.prefix_SegmentOverrideOpCode = SS_SEGMENT_OVERRIDE_PREFIX;
}
void ccpuins_extern_AAA()
{
	if((ccpu.al & 0x0f) > 9 || 0x01==GetAF)
	{
		ccpu.al += 6;
		ccpu.ah++;
		ccpu_flag_SetAF(0x01);
		ccpu_flag_SetCF(0x01);
		ccpu.al &= 0x0f;
	}
	else
	{
		ccpu_flag_SetAF(0x00);
		ccpu_flag_SetCF(0x00);
		ccpu.al &= 0x0f;

	}
}

void ccpuins_extern_INC_eAX()
{
	ccpuins_atom_INC_16bit(&ccpu.ax);
}
void ccpuins_extern_INC_eCX()
{
	ccpuins_atom_INC_16bit(&ccpu.cx);
}
void ccpuins_extern_INC_eDX()
{
	ccpuins_atom_INC_16bit(&ccpu.dx);
}
void ccpuins_extern_INC_eBX()
{
	ccpuins_atom_INC_16bit(&ccpu.bx);
}
void ccpuins_extern_INC_eSP()
{
	ccpuins_atom_INC_16bit(&ccpu.sp);
}
void ccpuins_extern_INC_eBP()
{
	ccpuins_atom_INC_16bit(&ccpu.bp);
}
void ccpuins_extern_INC_eSI()
{
	ccpuins_atom_INC_16bit(&ccpu.si);
}
void ccpuins_extern_INC_eDI()
{
	ccpuins_atom_INC_16bit(&ccpu.di);
}

void ccpuins_extern_PUSH_eAX()//0x50
{
	ccpuins_atom_PUSH(ccpu.ax);
}
void ccpuins_extern_PUSH_eCX()
{
	ccpuins_atom_PUSH(ccpu.cx);
}
void ccpuins_extern_PUSH_eDX()
{
	ccpuins_atom_PUSH(ccpu.dx);
}
void ccpuins_extern_PUSH_eBX()
{
	ccpuins_atom_PUSH(ccpu.bx);
}
void ccpuins_extern_PUSH_eSP()
{
	ccpuins_atom_PUSH(ccpu.sp);
}
void ccpuins_extern_PUSH_eBP()
{
	ccpuins_atom_PUSH(ccpu.bp);
}
void ccpuins_extern_PUSH_eSI()
{
	ccpuins_atom_PUSH(ccpu.si);
}
void ccpuins_extern_PUSH_eDI()
{
	ccpuins_atom_PUSH(ccpu.di);
}

void ccpuins_extern_PUSHA_PUSHAD(){/*do nothing*/}//80x86没有
void ccpuins_extern_POPA_POPAD(){/*do nothing*/}//80x86没有
void ccpuins_extern_BOUND_Gv_Ma(){/*do nothing*/}//80x86没有
void ccpuins_extern_ARPL_Ew_Gw(){/*do nothing*/}//80x86没有
void ccpuins_extern_SEG_FS(){/*do nothing*/}
void ccpuins_extern_SEG_GS(){/*do nothing*/}
void ccpuins_extern_Operand_Size(){/*do nothing*/}//80x86没有
void ccpuins_extern_Address_Size(){/*do nothing*/}//80x86没有

void ccpuins_extern_ShortJump_JO()//0x70
{
	if(0x01 == GetOF)
	{
		ccpuins_atom_shortJMP();
	}
}
void ccpuins_extern_ShortJump_JNO()//0x71
{
	if(0x00 == GetOF)
	{
		ccpuins_atom_shortJMP();
	}
}
void ccpuins_extern_ShortJump_JB()//0x72
{
	if(0x01 == GetCF)
	{
		ccpuins_atom_shortJMP();
	}
}
void ccpuins_extern_ShortJump_JNB()//0x73
{
	if(0x00 == GetCF)
	{
		ccpuins_atom_shortJMP();
	}
}
void ccpuins_extern_ShortJump_JZ()//0x74
{
	if(0x01 == GetZF)
	{
		ccpuins_atom_shortJMP();
	}
}
void ccpuins_extern_ShortJump_JNZ()//0x75
{
	if(0x00 == GetZF)
	{
		ccpuins_atom_shortJMP();
	}
}
void ccpuins_extern_ShortJump_JBE()//0x76
{
	if((0x01 == GetCF) || (0x01 == GetZF))
	{
		ccpuins_atom_shortJMP();
	}
}
void ccpuins_extern_ShortJump_JNBE()//0
{
	if(0x00 == GetCF && 0x00 == GetZF)
	{
		ccpuins_atom_shortJMP();
	}

}

void ccpuins_extern_IMMGroup_Eb_Ib()
{
	switch(ccpuins_deCodeBlock.nnn)
	{
	case 0:
			ccpuins_group_ADD_Eb_Ib();
			break;
	case 1:
			ccpuins_group_OR_Eb_Ib();
			break;
	case 2:
		ccpuins_group_ADC_Eb_Ib();
		break;
	case 3:
		ccpuins_group_SBB_Eb_Ib();
		break;
	case 4:
		ccpuins_group_AND_Eb_Ib();
		break;
	case 5:
		ccpuins_group_SUB_Eb_Ib();
		break;
	case 6:
		ccpuins_group_XOR_Eb_Ib();
		break;
	case 7:
		ccpuins_group_CMP_Eb_Ib();
		break;
	}
}
void ccpuins_extern_IMMGroup_Ev_Iz()
{
	switch(ccpuins_deCodeBlock.nnn)
	{
	case 0:
			ccpuins_group_ADD_Ev_Iv();
			break;
	case 1:
			ccpuins_group_OR_Ev_Iv();
			break;
	case 2:
		ccpuins_group_ADC_Ev_Iv();
		break;
	case 3:
		ccpuins_group_SBB_Ev_Iv();
		break;
	case 4:
		ccpuins_group_AND_Ev_Iv();
		break;
	case 5:
		ccpuins_group_SUB_Ev_Iv();
		break;
	case 6:
		ccpuins_group_XOR_Ev_Iv();
		break;
	case 7:
		ccpuins_group_CMP_Ev_Iv();
		break;
	}
}
void ccpuins_extern_IMMGroup_Eb_IbX()
{
	ccpuins_extern_IMMGroup_Eb_Ib();
}
void ccpuins_extern_IMMGroup_Ev_Ib()
{
	switch(ccpuins_deCodeBlock.nnn)
	{
	case 0:
			ccpuins_group_ADD_Ev_Iv();
			break;
	case 1:
			ccpuins_group_OR_Ev_Iv();
			break;
	case 2:
		ccpuins_group_ADC_Ev_Iv();
		break;
	case 3:
		ccpuins_group_SBB_Ev_Iv();
		break;
	case 4:
		ccpuins_group_AND_Ev_Iv();
		break;
	case 5:
		ccpuins_group_SUB_Ev_Iv();
		break;
	case 6:
		ccpuins_group_XOR_Ev_Iv();
		break;
	case 7:
		ccpuins_group_CMP_Ev_Iv();
		break;
	}
}

void ccpuins_extern_TEST_Eb_Gb()//0x84
{
	ccpuins_atom_TEST_8bit(p_nubit8(ccpuins_deCodeBlock.prm), d_nubit8(ccpuins_deCodeBlock.preg));
}
void ccpuins_extern_TEST_Ev_Gv()
{
	ccpuins_atom_TEST_16bit(p_nubit16(ccpuins_deCodeBlock.prm), d_nubit16(ccpuins_deCodeBlock.preg));
}

void ccpuins_extern_XCHG_Eb_Gb()//0x86
{
	t_nubit8 tmp = d_nubit8(ccpuins_deCodeBlock.preg);
	d_nubit8(ccpuins_deCodeBlock.preg) = d_nubit8(ccpuins_deCodeBlock.prm);
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) return;
#endif
	d_nubit8(ccpuins_deCodeBlock.prm)= tmp;
}
void ccpuins_extern_XCHG_Ev_Gv()
{
	t_nubit16 tmp = d_nubit16(ccpuins_deCodeBlock.preg);
	d_nubit16(ccpuins_deCodeBlock.preg) = d_nubit16(ccpuins_deCodeBlock.prm);
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) return;
#endif
	d_nubit16(ccpuins_deCodeBlock.prm)= tmp;
}

void ccpuins_extern_NOP(){/* do nothing*/}

void ccpuins_extern_XCHG_eCX()//0x91
{
	t_nubit16 tmp = ccpu.cx;
	ccpu.cx = ccpu.ax;
	ccpu.ax = tmp;
}
void ccpuins_extern_XCHG_eDX()
{
	t_nubit16 tmp = ccpu.dx;
	ccpu.dx = ccpu.ax;
	ccpu.ax = tmp;
}
void ccpuins_extern_XCHG_eBX()
{
	t_nubit16 tmp = ccpu.bx;
	ccpu.bx = ccpu.ax;
	ccpu.ax = tmp;
}
void ccpuins_extern_XCHG_eSP()
{
	t_nubit16 tmp = ccpu.sp;
	ccpu.sp = ccpu.ax;
	ccpu.ax = tmp;
}
void ccpuins_extern_XCHG_eBP()
{
	t_nubit16 tmp = ccpu.bp;
	ccpu.bp = ccpu.ax;
	ccpu.ax = tmp;
}
void ccpuins_extern_XCHG_eSI()
{
	t_nubit16 tmp = ccpu.si;
	ccpu.si = ccpu.ax;
	ccpu.ax = tmp;
}
void ccpuins_extern_XCHG_eDI()
{
	t_nubit16 tmp = ccpu.di;
	ccpu.di = ccpu.ax;
	ccpu.ax = tmp;
}

void ccpuins_extern_MOV_AL_Ob()//tested
{
	ccpu.al = d_nubit8(ccpuins_deCodeBlock.prm);
}
void ccpuins_extern_MOV_eAX_Ov()//tested
{
	ccpu.ax = d_nubit16(ccpuins_deCodeBlock.prm);
}
void ccpuins_extern_MOV_Ob_AL()//tested
{
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) return;
#endif
	d_nubit8(ccpuins_deCodeBlock.prm) = ccpu.al;
}
void ccpuins_extern_MOV_Ov_eAX()//tested
{
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) return;
#endif
	d_nubit16(ccpuins_deCodeBlock.prm) = ccpu.ax;
}

void ccpuins_extern_MOVSB_Xb_Yb()
{
	if(ccpuins_deCodeBlock.prefix_RepeatOpCode == REPNE_PREFIX ||
		ccpuins_deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX)
	{
		ccpuins_repeatMethod = (t_faddrcc)ccpuins_atom_MOVSB;//绑定指令操作函数
		ins_chooseRepeatExeStringMethod();//选择指令执行方式
		return;
	}
	else
		ccpuins_atom_MOVSB();
}
void ccpuins_extern_MOVSW_Xv_Yv()
{
	if(ccpuins_deCodeBlock.prefix_RepeatOpCode == REPNE_PREFIX ||
		ccpuins_deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		ccpuins_repeatMethod = (t_faddrcc)ccpuins_atom_MOVSW;
		ins_chooseRepeatExeStringMethod();
		return;
	}
	else
		ccpuins_atom_MOVSW();
}
void ccpuins_extern_CMPSB_Xb_Yb()//0xa6
{
	if(ccpuins_deCodeBlock.prefix_RepeatOpCode == REPNE_PREFIX ||
		ccpuins_deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		ccpuins_repeatMethod = (t_faddrcc)ccpuins_atom_CMPSB;
		ins_chooseRepeatExeStringMethod();
		return;
	}
	else
		ccpuins_atom_CMPSB();
}
void ccpuins_extern_CMPSW_Xv_Yv()//0xa7
{
	if(ccpuins_deCodeBlock.prefix_RepeatOpCode == REPNE_PREFIX ||
		ccpuins_deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进
		ccpuins_repeatMethod = (t_faddrcc)ccpuins_atom_CMPSW;
		ins_chooseRepeatExeStringMethod();
		return;
	}
	else
		ccpuins_atom_CMPSW();
}

void ccpuins_extern_MOV_AL()//tested
{
	ccpu.al = ccpuins_deCodeBlock.immData_8Bit;
}
void ccpuins_extern_MOV_CL()//tested
{
	ccpu.cl = ccpuins_deCodeBlock.immData_8Bit;
}
void ccpuins_extern_MOV_DL()//tested
{
	ccpu.dl = ccpuins_deCodeBlock.immData_8Bit;
}
void ccpuins_extern_MOV_BL()//tested
{
	ccpu.bl = ccpuins_deCodeBlock.immData_8Bit;
}
void ccpuins_extern_MOV_AH()//tested
{
	ccpu.ah =ccpuins_deCodeBlock.immData_8Bit;
}
void ccpuins_extern_MOV_CH()//tested
{
	ccpu.ch =ccpuins_deCodeBlock.immData_8Bit;
}
void ccpuins_extern_MOV_DH()//tested
{
	ccpu.dh = ccpuins_deCodeBlock.immData_8Bit;
}
void ccpuins_extern_MOV_BH()//tested
{
	ccpu.bh =ccpuins_deCodeBlock.immData_8Bit;
}

void ccpuins_extern_ShiftGroup_Eb_Ib(){/*do nothing*/}//80x86没有
void ccpuins_extern_ShiftGroup_Ev_Ib(){/*do nothing*/}//80x86没有
void ccpuins_extern_RET_near_Iw() //带立即数的段内返回
{
	ccpu.ip = ccpuins_atom_POP();
	ccpu.sp += ccpuins_deCodeBlock.opContactData_16bit;
}
//that would be something
void ccpuins_extern_RET_near_None()//段内返回
{
	ccpu.ip = ccpuins_atom_POP();
}

void ccpuins_extern_LES_Gv_Mp()
{
	d_nubit16(ccpuins_deCodeBlock.preg) = d_nubit16(ccpuins_deCodeBlock.prm);
	ccpu.es = d_nubit16((t_nubit8 *)(ccpuins_deCodeBlock.prm) + 2);
}
void ccpuins_extern_LDS_Gv_Mp()
{
	d_nubit16(ccpuins_deCodeBlock.preg) = d_nubit16(ccpuins_deCodeBlock.prm);
	ccpu.ds = d_nubit16((t_nubit8 *)(ccpuins_deCodeBlock.prm) + 2);
}

void ccpuins_extern_MOV_Eb_Ib()//tested
{
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) return;
#endif
	d_nubit8(ccpuins_deCodeBlock.prm) = ccpuins_deCodeBlock.immData_8Bit;
}
void ccpuins_extern_MOV_Ev_Iv()//tested
{
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) return;
#endif
	d_nubit16(ccpuins_deCodeBlock.prm) = ccpuins_deCodeBlock.immData_16Bit;
}

void ccpuins_extern_ShiftGroup2_Eb_1()
{
	switch(ccpuins_deCodeBlock.nnn)
	{
	case 0://nnn=000
		ccpuins_group_ROL_Eb_1();
		break;
	case 1://nnn=001
		ccpuins_group_ROR_Eb_1();
		break;
	case 2://nnn=010
		ccpuins_group_RCL_Eb_1();
		break;
	case 3://nnn=011
		ccpuins_group_RCR_Eb_1();
		break;
	case 4://nnn=100
		ccpuins_group_SHL_Eb_1();
		break;
	case 5://nnn=101
		ccpuins_group_SHR_Eb_1();
		break;
	case 6://nnn=110
		vapiPrint("ccpuins_extern_ShiftGroup2_Eb_1 UnHandle the nnn bits");
		vapiCallBackMachineStop();;
		return;
	case 7://nnn=111
		ccpuins_group_SAR_Eb_1();
		break;
	}
}
void ccpuins_extern_ShiftGroup2_Ev_1()
{
	switch(ccpuins_deCodeBlock.nnn)
	{
	case 0://nnn=000
		ccpuins_group_ROL_Ev_1();
		break;
	case 1://nnn=001
		ccpuins_group_ROR_Ev_1();
		break;
	case 2://nnn=010
		ccpuins_group_RCL_Ev_1();
		break;
	case 3://nnn=011
		ccpuins_group_RCR_Ev_1();
		break;
	case 4://nnn=100
		ccpuins_group_SHL_Ev_1();
		break;
	case 5://nnn=101
		ccpuins_group_SHR_Ev_1();
		break;
	case 6://nnn=110
		vapiPrint("ccpuins_extern_ShiftGroup2_Ev_1 UnHandle the nnn bits");
		vapiCallBackMachineStop();;
		return;
	case 7://nnn=111
		ccpuins_group_SAR_Ev_1();
		break;
	}
}
void ccpuins_extern_ShiftGroup2_Eb_CL()
{
	switch(ccpuins_deCodeBlock.nnn)
	{
	case 0://nnn=000
		ccpuins_group_ROL_Eb_CL();
		break;
	case 1://nnn=001
		ccpuins_group_ROR_Eb_CL();
		break;
	case 2://nnn=010
		ccpuins_group_RCL_Eb_CL();
		break;
	case 3://nnn=011
		ccpuins_group_RCR_Eb_CL();
		break;
	case 4://nnn=100
		ccpuins_group_SHL_Eb_CL();
		break;
	case 5://nnn=101
		ccpuins_group_SHR_Eb_CL();
		break;
	case 6://nnn=110
		vapiPrint("ccpuins_extern_ShiftGroup2_Eb_CL UnHandle the nnn bits");
		vapiCallBackMachineStop();;
		return;
	case 7://nnn=111
		ccpuins_group_SAR_Eb_CL();
		break;
	}
}
void ccpuins_extern_ShiftGroup2_Ev_CL()
{
	switch(ccpuins_deCodeBlock.nnn)
	{
	case 0://nnn=000
		ccpuins_group_ROL_Ev_CL();
		break;
	case 1://nnn=001
		ccpuins_group_ROR_Ev_CL();
		break;
	case 2://nnn=010
		ccpuins_group_RCL_Ev_CL();
		break;
	case 3://nnn=011
		ccpuins_group_RCR_Ev_CL();
		break;
	case 4://nnn=100
		ccpuins_group_SHL_Ev_CL();
		break;
	case 5://nnn=101
		ccpuins_group_SHR_Ev_CL();
		break;
	case 6://nnn=110
		vapiPrint("ccpuins_extern_ShiftGroup2_Ev_CL UnHandle the nnn bits");
		vapiCallBackMachineStop();;
		return;
	case 7://nnn=111
		ccpuins_group_SAR_Ev_CL();
		break;
	}
}

void ccpuins_extern_AMM()
{
	t_nubit8 operand1 = ccpu.al;
	ccpu.ah = ccpu.al / 10;
	ccpu.al %= 10;
	ccpu_storeCaculate(MOD_8bit, 8, operand1, 10, ccpu.al, VCPU_FLAG_SF | VCPU_FLAG_ZF | VCPU_FLAG_PF);
}
void ccpuins_extern_AAD()
{
	t_bool tmpAF = GetAF;
	t_bool tmpCF = GetCF;
	t_bool tmpOF = GetOF;
	const t_nubit8 tmp = ccpu.ah * 10;
	ccpuins_atom_ADD_8bit(&ccpu.al,tmp);
	ccpu.ah = 0;
	ccpu_flag_SetAF(tmpAF);
	ccpu_flag_SetCF(tmpCF);
	ccpu_flag_SetOF(tmpOF);
}

void ccpuins_extern_XLAT()
{
		ccpu.al = getMemData_byte(ccpu.al + ccpu.bx + (decode_getDefaultSegment_DS() <<4));
}

void ccpuins_extern_LOOPN_Jb()//0xe0
{
	if((0 != --(ccpu.cx)) && (0x00 == GetZF))
		ccpuins_atom_shortJMP();
}
void ccpuins_extern_LOOPE_Jb()
{
	if((0 != --(ccpu.cx)) && (0x01 == GetZF))
		ccpuins_atom_shortJMP();
}
void ccpuins_extern_LOOP_Jb()
{
	if(0 != --(ccpu.cx))
		ccpuins_atom_shortJMP();
}
void ccpuins_extern_JCXZ_Jb()
{
	if(0 == ccpu.cx )
		ccpuins_atom_shortJMP();
}

void ccpuins_extern_IN_AL_Ib()
{
#if (DEBUGMODE == CCPU)
	ExecFun(vport.in[ccpuins_deCodeBlock.opContactData_8bit]);
	ccpu.al = ccpu.iobyte;
	if (ccpuins_deCodeBlock.opContactData_8bit == 0xbb) {
		if (GetBit(ccpu.flags, VCPU_FLAG_ZF))
			cramVarWord(ccpu.ss,ccpu.sp + 4) |=  VCPU_FLAG_ZF;
		else
			cramVarWord(ccpu.ss,ccpu.sp + 4) &= ~VCPU_FLAG_ZF;
	}
#endif
#if (DEBUGMODE == BVRM || DEBUGMODE == BCRM)
	ccpu_flagignore = 0x01;
#endif
}
void ccpuins_extern_IN_eAX_Ib() {ccpu_flagignore = 0x01;}
void ccpuins_extern_OUT_Ib_AL()
{
#if (DEBUGMODE == CCPU)
	ccpu.iobyte = ccpu.al;
	ExecFun(vport.out[ccpuins_deCodeBlock.opContactData_8bit]);
	if (ccpuins_deCodeBlock.opContactData_8bit == 0xbb) {
		if (GetBit(ccpu.flags, VCPU_FLAG_ZF))
			cramVarWord(ccpu.ss,ccpu.sp + 4) |=  VCPU_FLAG_ZF;
		else
			cramVarWord(ccpu.ss,ccpu.sp + 4) &= ~VCPU_FLAG_ZF;
	}
#endif
#if (DEBUGMODE == BVRM || DEBUGMODE == BCRM)
	ccpu_flagignore = 0x01;
#endif
}
void ccpuins_extern_OUT_Ib_eAX() {ccpu_flagignore = 0x01;}

void ccpuins_extern_LOCK(){/*在这个版本的虚拟机内，此指令什么都不做*/}
void ccpuins_extern_REPNE()
{
	ccpuins_deCodeBlock.prefix_RepeatOpCode = REPNE_PREFIX;
}
void ccpuins_extern_REP_REPE()
{
	ccpuins_deCodeBlock.prefix_RepeatOpCode = REP_PREFIX;
}
void ccpuins_extern_HLT(){}//还不清楚
void ccpuins_extern_CMC()//0xf5
{
	ccpu_flag_SetCF(!GetCF );
}
void ccpuins_extern_UnaryGroup_Eb()
{
	switch(ccpuins_deCodeBlock.nnn)
	{
	case 0://nnn=000
		ccpuins_group_TEST_Ib();
		break;
	case 1://nnn=001
		vapiPrint("ccpuins_extern_UnaryGroup_Eb UnHandle the nnn bits");
		vapiCallBackMachineStop();;
		break;
	case 2://nnn=010
		ccpuins_group_NOT_8bit();
		break;
	case 3://nnn=011
		ccpuins_group_NEG_8bit();
		break;
	case 4://nnn=100
		ccpuins_group_MUL_AL();
		break;
	case 5://nnn=101
		ccpuins_group_IMUL_AL();
		break;
	case 6://nnn=110
		ccpuins_group_DIV_AL();
		return;
	case 7://nnn=111
		ccpuins_group_IDIV_AL();
		break;
	}
}
void ccpuins_extern_UnaryGroup_Ev()
{
	switch(ccpuins_deCodeBlock.nnn)
	{
	case 0://nnn=000
		ccpuins_group_TEST_Iv();
		break;
	case 1://nnn=001
		vapiPrint("ccpuins_extern_UnaryGroup_Ev UnHandle the nnn bits");
		vapiCallBackMachineStop();;
		break;
	case 2://nnn=010
		ccpuins_group_NOT_16bit();
		break;
	case 3://nnn=011
		ccpuins_group_NEG_16bit();
		break;
	case 4://nnn=100
		ccpuins_group_MUL_eAX();
		break;
	case 5://nnn=101
		ccpuins_group_IMUL_eAX();
		break;
	case 6://nnn=110
		ccpuins_group_DIV_eAX();
		return;
	case 7://nnn=111
		ccpuins_group_IDIV_eAX();
		break;
	}
}

void ccpuins_extern_OR_Eb_Gb()
{
	ccpuins_atom_OR_8bit(p_nubit8(ccpuins_deCodeBlock.prm), d_nubit8(ccpuins_deCodeBlock.preg));

}
void ccpuins_extern_OR_Ev_Gv()
{
	ccpuins_atom_OR_16bit(p_nubit16(ccpuins_deCodeBlock.prm), d_nubit16(ccpuins_deCodeBlock.preg));
}
void ccpuins_extern_OR_Gb_Eb()
{
	ccpuins_atom_OR_8bit(p_nubit8(ccpuins_deCodeBlock.preg), d_nubit8(ccpuins_deCodeBlock.prm));
}
void ccpuins_extern_OR_Gv_Ev()
{
	ccpuins_atom_OR_16bit(p_nubit16(ccpuins_deCodeBlock.preg), d_nubit16(ccpuins_deCodeBlock.prm));
}
void ccpuins_extern_OR_AL_Ib()
{
	ccpuins_atom_OR_8bit(&ccpu.al, ccpuins_deCodeBlock.immData_8Bit);
}
void ccpuins_extern_OR_eAX_Iv()
{
	ccpuins_atom_OR_16bit(&ccpu.ax, ccpuins_deCodeBlock.immData_16Bit);
}

void ccpuins_extern_PUSH_CS()//0x0e
{
	ccpuins_atom_PUSH(ccpu.cs);
}
void ccpuins_extern_EscapeCode(){}

void ccpuins_extern_SBB_Eb_Gb()//tested
{
	ccpuins_atom_SBB_8bit(p_nubit8(ccpuins_deCodeBlock.prm), d_nubit8(ccpuins_deCodeBlock.preg));

}
void ccpuins_extern_SBB_Ev_Gv()//tested
{
	ccpuins_atom_SBB_16bit(p_nubit16(ccpuins_deCodeBlock.prm), d_nubit16(ccpuins_deCodeBlock.preg));
}
void ccpuins_extern_SBB_Gb_Eb()//tested
{
	ccpuins_atom_SBB_8bit(p_nubit8(ccpuins_deCodeBlock.preg), d_nubit8(ccpuins_deCodeBlock.prm));
}
void ccpuins_extern_SBB_Gv_Ev()//tested
{
	ccpuins_atom_SBB_16bit(p_nubit16(ccpuins_deCodeBlock.preg), d_nubit16(ccpuins_deCodeBlock.prm));
}
void ccpuins_extern_SBB_AL_Ib()//tested
{
	ccpuins_atom_SBB_8bit(&ccpu.al, ccpuins_deCodeBlock.immData_8Bit);
}
void ccpuins_extern_SBB_eAX_Iv()//tested
{
	ccpuins_atom_SBB_16bit(&ccpu.ax, ccpuins_deCodeBlock.immData_16Bit);
}

void ccpuins_extern_PUSH_DS()
{
	ccpuins_atom_PUSH(ccpu.ds);
}
void ccpuins_extern_POP_DS()
{
	ccpu.ds = ccpuins_atom_POP();
}

void ccpuins_extern_SUB_Eb_Gb()//tested
{
	ccpuins_atom_SUB_8bit(p_nubit8(ccpuins_deCodeBlock.prm), d_nubit8(ccpuins_deCodeBlock.preg));
}
void ccpuins_extern_SUB_Ev_Gv()//tested
{
	ccpuins_atom_SUB_16bit(p_nubit16(ccpuins_deCodeBlock.prm), d_nubit16(ccpuins_deCodeBlock.preg));
}
void ccpuins_extern_SUB_Gb_Eb()//tested
{
	ccpuins_atom_SUB_8bit(p_nubit8(ccpuins_deCodeBlock.preg), d_nubit8(ccpuins_deCodeBlock.prm));
}
void ccpuins_extern_SUB_Gv_Ev()//tested
{
	ccpuins_atom_SUB_16bit(p_nubit16(ccpuins_deCodeBlock.preg), d_nubit16(ccpuins_deCodeBlock.prm));
}

void ccpuins_extern_SUB_AL_Ib()//tested
{
	ccpuins_atom_SUB_8bit(&ccpu.al, ccpuins_deCodeBlock.immData_8Bit);
}
void ccpuins_extern_SUB_eAX_Iv()//tested
{
	ccpuins_atom_SUB_16bit(&ccpu.ax, ccpuins_deCodeBlock.immData_16Bit);
}

void ccpuins_extern_SEG_CS()
{
	ccpuins_deCodeBlock.prefix_SegmentOverrideOpCode = CS_SEGMENT_OVERRIDE_PREFIX;
}
void ccpuins_extern_DAS()//0x2f
{
	t_nubit8 old_al = ccpu.al;
	t_bool old_CF = GetCF;

	ccpu_flag_SetCF(0x00);
	if((ccpu.al & 0x0f)>9 || (0x01 ==GetAF))
	{
		ccpuins_atom_SUB_8bit(&ccpu.al, 6);
		ccpu_flag_SetCF(old_CF); //这里可能会有点问题
		ccpu_flag_SetAF(0x01);
	}
	else
	{
		ccpu_flag_SetAF(0x00);
	}
    //*****
	if((old_al>0x99) || (0x01 == old_CF))
	{
		ccpuins_atom_SUB_8bit(&ccpu.al, 0x60);
		ccpu_flag_SetCF(0x01);

	}
	else
	{
		ccpu_flag_SetCF(0x00);//可能有点问题
	}
}

void ccpuins_extern_CMP_Eb_Gb()
{
	ccpuins_atom_CMP_8bit(d_nubit8(ccpuins_deCodeBlock.prm), d_nubit8(ccpuins_deCodeBlock.preg));
}
void ccpuins_extern_CMP_Ev_Gv()
{
	ccpuins_atom_CMP_16bit(d_nubit16(ccpuins_deCodeBlock.prm), d_nubit16(ccpuins_deCodeBlock.preg));
}
void ccpuins_extern_CMP_Gb_Eb()
{
	ccpuins_atom_CMP_8bit(d_nubit8(ccpuins_deCodeBlock.preg), d_nubit8(ccpuins_deCodeBlock.prm));
}
void ccpuins_extern_CMP_Gv_Ev()
{
	ccpuins_atom_CMP_16bit(d_nubit16(ccpuins_deCodeBlock.preg), d_nubit16(ccpuins_deCodeBlock.prm));
}
void ccpuins_extern_CMP_AL_Ib()
{
	ccpuins_atom_CMP_8bit(ccpu.al, ccpuins_deCodeBlock.immData_8Bit);
}
void ccpuins_extern_CMP_eAX_Iv()
{
	ccpuins_atom_CMP_16bit(ccpu.ax, ccpuins_deCodeBlock.immData_16Bit);
}

void ccpuins_extern_SEG_DS()
{
	ccpuins_deCodeBlock.prefix_SegmentOverrideOpCode = DS_SEGMENT_OVERRIDE_PREFIX;
}
void ccpuins_extern_AAS()
{
	if((ccpu.al & 0x0f) > 9 || 0x01==GetAF)
	{
		ccpu.al -= 6;
		ccpu.ah--;
		ccpu_flag_SetAF(0x01);
		ccpu_flag_SetCF(0x01);
		ccpu.al &= 0x0f;
	}
	else
	{
		ccpu_flag_SetAF(0x00);
		ccpu_flag_SetCF(0x00);
		ccpu.al &= 0x0f;

	}
}

void ccpuins_extern_DEC_eAX()
{	
	ccpuins_atom_DEC_16bit(&ccpu.ax);
}
void ccpuins_extern_DEC_eCX()
{
	ccpuins_atom_DEC_16bit(&ccpu.cx);
}
void ccpuins_extern_DEC_eDX()
{
	ccpuins_atom_DEC_16bit(&ccpu.dx);
}
void ccpuins_extern_DEC_eBX()
{
	ccpuins_atom_DEC_16bit(&ccpu.bx);
}
void ccpuins_extern_DEC_eSP()
{
	ccpuins_atom_DEC_16bit(&ccpu.sp);
}
void ccpuins_extern_DEC_eBP()
{
	ccpuins_atom_DEC_16bit(&ccpu.bp);
}
void ccpuins_extern_DEC_eSI()
{
	ccpuins_atom_DEC_16bit(&ccpu.si);
}
void ccpuins_extern_DEC_eDI()
{
	ccpuins_atom_DEC_16bit(&ccpu.di);
}

void ccpuins_extern_POP_eAX()
{
	ccpu.ax = ccpuins_atom_POP();
}
void ccpuins_extern_POP_eCX()
{
	ccpu.cx = ccpuins_atom_POP();
}
void ccpuins_extern_POP_eDX()
{
	ccpu.dx = ccpuins_atom_POP();
}
void ccpuins_extern_POP_eBX()
{
	ccpu.bx = ccpuins_atom_POP();
}
void ccpuins_extern_POP_eSP()
{
	ccpu.sp = ccpuins_atom_POP();
}
void ccpuins_extern_POP_eBP()
{
	ccpu.bp = ccpuins_atom_POP();
}
void ccpuins_extern_POP_eSI()
{
	ccpu.si = ccpuins_atom_POP();
}
void ccpuins_extern_POP_eDI()
{
	ccpu.di = ccpuins_atom_POP();
}

void ccpuins_extern_PUSH_Iv()//0x68  16位中没有，不管了，先做了
{
	ccpuins_atom_PUSH(ccpuins_deCodeBlock.opContactData_16bit);//估计是这样。。具体还看解码
}
void ccpuins_extern_IMUL_Gv_Ev_Iv(){/*16位机没有这个指令*/}
void ccpuins_extern_PUSH_Ib()//0x6a
{
	ccpuins_atom_PUSH(ccpuins_deCodeBlock.opContactData_8bit);//估计是这样。。具体还看解码
}
void ccpuins_extern_IMUL_Gv_Ev_Ib(){/*16位机没有这个指令*/}
void ccpuins_extern_INSB_Yb_DX(){/*16位机没有这个指令*/}
void ccpuins_extern_INSW_Yv_DX(){/*16位机没有这个指令*/}
void ccpuins_extern_OUTSB_DX_Xb(){}//无效
void ccpuins_extern_OUTSW_DX_Xv(){}//无效

void ccpuins_extern_ShortJumpCondition_JS()
{
	if(0x01 == GetSF)
	{
		ccpuins_atom_shortJMP();
	}
}
void ccpuins_extern_ShortJumpCondition_JNS()
{
	if(0x00 == GetSF)
	{
		ccpuins_atom_shortJMP();
	}
}
void ccpuins_extern_ShortJumpCondition_JP()
{
	if(0x01 == GetPF)
	{
		ccpuins_atom_shortJMP();
	}
}
void ccpuins_extern_ShortJumpCondition_JNP()
{
	if(0x00 == GetPF)
	{
		ccpuins_atom_shortJMP();
	}
}
void ccpuins_extern_ShortJumpCondition_JL()
{
	if(GetSF != GetOF)
	{
		ccpuins_atom_shortJMP();
	}
}
void ccpuins_extern_ShortJumpCondition_JNL()
{
	if(GetSF == GetOF)
	{
		ccpuins_atom_shortJMP();
	}
}
void ccpuins_extern_ShortJumpCondition_JLE()
{
	if((0x01 == GetZF) || (GetSF != GetOF))
	{
		ccpuins_atom_shortJMP();
	}
}
void ccpuins_extern_ShortJumpCondition_JNLE()
{
	if((0x00 == GetZF) && (GetSF == GetOF))
	{
		ccpuins_atom_shortJMP();
	}
}

void ccpuins_extern_MOV_Eb_Gb()//tested
{
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) return;
#endif
	d_nubit8(ccpuins_deCodeBlock.prm) = d_nubit8(ccpuins_deCodeBlock.preg);
}
void ccpuins_extern_MOV_Ev_Gv()//tested
{
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) return;
#endif
	d_nubit16(ccpuins_deCodeBlock.prm) = d_nubit16(ccpuins_deCodeBlock.preg);
}
void ccpuins_extern_MOV_Gb_Eb()//tested
{
//	vapiPrint("MOV_Gb_Eb, %X\n",(t_vaddrcc)ccpuins_deCodeBlock.prm);
	d_nubit8(ccpuins_deCodeBlock.preg) = d_nubit8(ccpuins_deCodeBlock.prm);
}
void ccpuins_extern_MOV_Gv_Ev()//tested
{
	d_nubit16(ccpuins_deCodeBlock.preg) = d_nubit16(ccpuins_deCodeBlock.prm);
}
void ccpuins_extern_MOV_Ew_Sw()//tested
{
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) return;
#endif
	d_nubit16(ccpuins_deCodeBlock.prm) = d_nubit16(ccpuins_deCodeBlock.pseg);
}

void ccpuins_extern_LEA_Gv_M()
{
	d_nubit16(ccpuins_deCodeBlock.preg) = (t_nubit16)(p_nubit8(ccpuins_deCodeBlock.prm) - p_nubit8(cramGetAddr(0,0)) - (ccpuins_deCodeBlock.segData<<4));
}

void ccpuins_extern_MOV_Sw_Ew()//tested
{
	d_nubit16(ccpuins_deCodeBlock.pseg) = d_nubit16(ccpuins_deCodeBlock.prm);
}
void ccpuins_extern_POP_Ev()
{
	t_nubit16 tmp;
	tmp = ccpuins_atom_POP();
#if DEBUGMODE == BVRM
	if (im(ccpuins_deCodeBlock.prm)) return;
#endif
	d_nubit16(ccpuins_deCodeBlock.prm) = tmp;
}

void ccpuins_extern_CBW()//0x98
{
	if(ccpu.al >= 0x80)
	{
		ccpu.ah = 0xff;
	}
	else
	{
		ccpu.ah = 0x00;
	}
}
void ccpuins_extern_CWD() //0x99
{
	if(ccpu.ax >= 0x8000)
	{
		ccpu.dx = 0xffff;
	}
	else
	{
		ccpu.dx = 0x0000;
	}
}
void ccpuins_extern_CALL_aP()//0x9a 段间直接调用
{
	ccpuins_atom_PUSH(ccpu.cs);
	ccpuins_atom_PUSH(ccpu.ip); // 没有PUSH IP 。。所以用这个来代替
	
	ccpu.cs = ccpuins_deCodeBlock.opContactData_16bitE;
	ccpu.ip = ccpuins_deCodeBlock.opContactData_16bit;
}
void ccpuins_extern_WAIT(){}//还不清楚
void ccpuins_extern_PUSHF_Fv()
{
	ccpuins_atom_PUSH(ccpu.flags);
}
void ccpuins_extern_POP_Fv()//0x8d  POPF 将标志弹出
{
	t_nubit16 data = ccpuins_atom_POP();
	ccpu_flag_SetFlags(data);
}
void ccpuins_extern_SAHF()//0x9e
{
	ccpu_flag_Set_SF_ZF_AF_PF_CF((t_nubit16)(ccpu.ah));
}
void ccpuins_extern_LAHF()
{
	ccpu.ah = (t_nubit8)ccpu.flags;
}

void ccpuins_extern_TEST_AL_Ib()
{
	ccpuins_atom_TEST_8bit(&ccpu.al, ccpuins_deCodeBlock.immData_8Bit);
}
void ccpuins_extern_TEST_eAX_Iv()
{
	ccpuins_atom_TEST_16bit(&ccpu.ax, ccpuins_deCodeBlock.immData_16Bit);
}
void ccpuins_extern_STOSB_Yb_AL()//0xaa
{
	if(ccpuins_deCodeBlock.prefix_RepeatOpCode == REPNE_PREFIX ||
		ccpuins_deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		ccpuins_repeatMethod = (t_faddrcc)ccpuins_atom_STOSB;
		ins_chooseRepeatExeStringMethod();
		return;
	}
	else
		ccpuins_atom_STOSB();
}
void ccpuins_extern_STOSW_Yv_eAX()
{
	if(ccpuins_deCodeBlock.prefix_RepeatOpCode == REPNE_PREFIX ||
		ccpuins_deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		ccpuins_repeatMethod = (t_faddrcc)ccpuins_atom_STOSW;
		ins_chooseRepeatExeStringMethod();
		return;
	}
	else
		ccpuins_atom_STOSW();
}
void ccpuins_extern_LODSB_AL_Xb()
{
	if(ccpuins_deCodeBlock.prefix_RepeatOpCode == REPNE_PREFIX ||
		ccpuins_deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		ccpuins_repeatMethod = (t_faddrcc)ccpuins_atom_LODSB;
		ins_chooseRepeatExeStringMethod();
		return;
	}
	else
		ccpuins_atom_LODSB();
}
void ccpuins_extern_LODSW_eAX_Xv()
{
	if(ccpuins_deCodeBlock.prefix_RepeatOpCode == REPNE_PREFIX ||
		ccpuins_deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		ccpuins_repeatMethod = (t_faddrcc)ccpuins_atom_LODSW;
		ins_chooseRepeatExeStringMethod();
		return;
	}
	else
		ccpuins_atom_LODSW();
}
void ccpuins_extern_SCASB_AL_Yb()
{
	if(ccpuins_deCodeBlock.prefix_RepeatOpCode == REPNE_PREFIX ||
		ccpuins_deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		ccpuins_repeatMethod = (t_faddrcc)ccpuins_atom_SCASB;
		ins_chooseRepeatExeStringMethod();
		return;
	}
	else
		ccpuins_atom_SCASB();
}
void ccpuins_extern_SCASW_eAX_Yv()
{
	if(ccpuins_deCodeBlock.prefix_RepeatOpCode == REPNE_PREFIX ||
		ccpuins_deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		ccpuins_repeatMethod = (t_faddrcc)ccpuins_atom_SCASW;
		ins_chooseRepeatExeStringMethod();
		return;
	}
	else
		ccpuins_atom_SCASW();
}

void ccpuins_extern_MOV_eAX()//tested
{
	ccpu.ax = ccpuins_deCodeBlock.immData_16Bit;
}
void ccpuins_extern_MOV_eCX()//tested
{
	ccpu.cx = ccpuins_deCodeBlock.immData_16Bit;
}
void ccpuins_extern_MOV_eDX()//tested
{
	ccpu.dx =ccpuins_deCodeBlock.immData_16Bit;
}
void ccpuins_extern_MOV_eBX()//tested
{
	ccpu.bx = ccpuins_deCodeBlock.immData_16Bit;
}
void ccpuins_extern_MOV_eSP()//tested
{
	ccpu.sp = ccpuins_deCodeBlock.immData_16Bit;
}
void ccpuins_extern_MOV_eBP()//tested
{
	ccpu.bp =ccpuins_deCodeBlock.immData_16Bit;
}
void ccpuins_extern_MOV_eSI()//tested
{
	ccpu.si = ccpuins_deCodeBlock.immData_16Bit;
}
void ccpuins_extern_MOV_eDI()//tested
{
	ccpu.di = ccpuins_deCodeBlock.immData_16Bit;
}

void ccpuins_extern_ENTER_Iw_Ib(){/*十六位机子没这个指令*/}
void ccpuins_extern_LEAVE(){/*十六位机子没这个指令*/}
void ccpuins_extern_RET_far_Iw()
{
	ccpu.ip = ccpuins_atom_POP();
	ccpu.cs = ccpuins_atom_POP();
	ccpu.sp += ccpuins_deCodeBlock.opContactData_16bit;
}
void ccpuins_extern_RET_far()
{
	ccpu.ip = ccpuins_atom_POP();
	ccpu.cs = ccpuins_atom_POP();
}
void ccpuins_extern_INT_3()  {ccpuins_atom_INT(0x03);}
void ccpuins_extern_INT_Ib() {ccpuins_atom_INT(ccpuins_deCodeBlock.opContactData_8bit);}
void ccpuins_extern_INTO()   {if(GetOF) ccpuins_atom_INT(4);}
void ccpuins_extern_IRET()
{
	t_nubit16 data;
	ccpu.ip = ccpuins_atom_POP();
	ccpu.cs = ccpuins_atom_POP();
	data = ccpuins_atom_POP();
	ccpu_flag_SetFlags(data);
}
void ccpuins_extern_ESC(){}//还不清楚
void ccpuins_extern_CALL_Jv()//0xe8 段内直接调用
{
	ccpuins_atom_PUSH(ccpu.ip); // 没有PUSH IP 。。所以用这个来代替
	ccpu.ip += (t_nsbit16)(ccpuins_deCodeBlock.opContactData_16bit);
}
void ccpuins_extern_JMP_Jv()//段内直接转移之一
{
	ccpu.ip += (t_nsbit16)(ccpuins_deCodeBlock.opContactData_16bit);
}
void ccpuins_extern_JMP_Ap()//段间直接转移
{
	ccpu.ip = ccpuins_deCodeBlock.opContactData_16bit;
	ccpu.cs = ccpuins_deCodeBlock.opContactData_16bitE;
}
void ccpuins_extern_JMP_Jb()//段内直接转移之二
{
	ccpuins_atom_shortJMP();
}
void ccpuins_extern_IN_AL_DX()
{
	//vapiPrint("flagignore set in ccpuins_extern_IN_AL_DX\n");
	ccpu_flagignore = 0x01;
}
void ccpuins_extern_IN_eAX_DX()
{
	//vapiPrint("flagignore set in ccpuins_extern_IN_eAX_DX\n");
	ccpu_flagignore = 0x01;
}
void ccpuins_extern_OUT_DX_AL()
{
	if (ccpuins_deCodeBlock.opContactData_8bit == 0xbb) {
		if (GetBit(_flags, VCPU_FLAG_ZF))
			cramVarWord(ccpu.ss,ccpu.sp + 4) |=  VCPU_FLAG_ZF;
		else
			cramVarWord(ccpu.ss,ccpu.sp + 4) &= ~VCPU_FLAG_ZF;
	}
	//vapiPrint("flagignore set in ccpuins_extern_OUT_DX_AL\n");
	ccpu_flagignore = 0x01;
}
void ccpuins_extern_OUT_DX_eAX()
{
	//vapiPrint("flagignore set in ccpuins_extern_OUT_DX_eAX\n");
	ccpu_flagignore = 0x01;
}

void ccpuins_extern_CLC()//0xf8
{
	ccpu_flag_SetCF(0x00);
}
void ccpuins_extern_STC()
{
	ccpu_flag_SetCF(0x01);
}
void ccpuins_extern_CLI()//0xfa
{
	ccpu_flag_SetIF(0x00);
}
void ccpuins_extern_STI()
{
	ccpu_flag_SetIF(0x01);
}
void ccpuins_extern_CLD()//0xfc
{
	ccpu_flag_SetDF(0x00);
}
void ccpuins_extern_STD()
{
	ccpu_flag_SetDF(0x01);
}
void ccpuins_extern_INC_Group4()
{
	switch(ccpuins_deCodeBlock.nnn)
	{
	case 0://nnn=000
		ccpuins_group_INC_Eb();
		break;
	case 1://nnn=001
		ccpuins_group_DEC_Eb();
		break;
	default:
		vapiPrint("ccpuins_extern_INC_Group4 UnHandle the nnn bits");
		vapiCallBackMachineStop();;
		break;
	}
}
void ccpuins_extern_INC_Group5()
{
	switch(ccpuins_deCodeBlock.nnn)
	{
	case 0://nnn=000
		ccpuins_group_INC_Ev();
		break;
	case 1://nnn=001
		ccpuins_group_DEC_Ev();
		break;
	case 2://nnn=010
		ccpuins_group_CALL_Ev();
		break;
	case 3://nnn=011
		ccpuins_group_CALL_Ep();
		break;
	case 4://nnn=100
		ccpuins_group_JMP_Ev();
		break;
	case 5://nnn=101
		ccpuins_group_JMP_Ep();
		break;
	case 6://nnn=110
		ccpuins_group_PUSH_Ev();
		return;
	case 7://nnn=111
		vapiPrint("ccpuins_extern_INC_Group5 UnHandle the nnn bits");
		vapiCallBackMachineStop();;
		break;
	}
}

void ccpuins_extern_Unknow() {vapiPrint("unkonwnMethod");}

void ins_chooseRepeatExeStringMethod()
{
	if(ccpuins_repeatMethod == (t_faddrcc)ccpuins_atom_MOVSB ||
		ccpuins_repeatMethod == (t_faddrcc)ccpuins_atom_MOVSW ||
		ccpuins_repeatMethod == (t_faddrcc)ccpuins_atom_LODSB ||
		ccpuins_repeatMethod == (t_faddrcc)ccpuins_atom_LODSW ||
		ccpuins_repeatMethod == (t_faddrcc)ccpuins_atom_STOSB ||
		ccpuins_repeatMethod == (t_faddrcc)ccpuins_atom_STOSW ||
		ccpuins_repeatMethod == (t_faddrcc)ccpuins_atom_INS   ||
		ccpuins_repeatMethod == (t_faddrcc)ccpuins_atom_OUTS )
	{
		ins_self_REP_StringIns();
	} else {
		if(ccpuins_deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX) //用于区分REPNE 和REPE 。。。。
			ins_self_REPE_StringIns();  //除此则都和REPE 
		else
			ins_self_REPNE_StringIns();	
	}
}
void ins_self_REP_StringIns()
{
	while(0 != ccpu.cx)
	{
		ExecFun(ccpuins_repeatMethod);
		ccpu.cx--;
	}
}
void ins_self_REPE_StringIns()
{
	while(0 != ccpu.cx)
	{
		ExecFun(ccpuins_repeatMethod);
		ccpu.cx--;
		if(0x00 == GetZF)
			break;
	}
}
void ins_self_REPNE_StringIns()
{
	while(0 != ccpu.cx)
	{
		ExecFun(ccpuins_repeatMethod);
		ccpu.cx--;
		if(0x01 == GetZF)
			break;
	}
}

void ccpuinsExecIns()
{
	t_faddrcc func;
	t_nubit8 opcode;

	opcode = cramVarByte(ccpu.cs, ccpu.ip);
	ccpuins_deCodeBlock.pOpCode = p_nubit8(cramGetAddr(ccpu.cs, ccpu.ip));
	func = ccpuins.table[opcode];
	decode_deCodeInsStuff();
	ExecFun(func);
	if (!IsPrefix(opcode)) ClrPrefix();
	else ccpuinsExecIns();
}
void ccpuinsExecInt(t_nubit8 intid) {ccpuins_atom_INT(intid);}

void ccpuinsInit()
{
	memset(&ccpuins, 0x00, sizeof(t_cpuins));
	ccpuins.table[0x00] = (t_faddrcc)ccpuins_extern_ADD_Eb_Gb;
	ccpuins.table[0x01] = (t_faddrcc)ccpuins_extern_ADD_Ev_Gv;
	ccpuins.table[0x02] = (t_faddrcc)ccpuins_extern_ADD_Gb_Eb;
	ccpuins.table[0x03] = (t_faddrcc)ccpuins_extern_ADD_Gv_Ev;
	ccpuins.table[0x04] = (t_faddrcc)ccpuins_extern_ADD_AL_Ib;
	ccpuins.table[0x05] = (t_faddrcc)ccpuins_extern_ADD_eAX_Iv;
	ccpuins.table[0x06] = (t_faddrcc)ccpuins_extern_PUSH_ES;
	ccpuins.table[0x07] = (t_faddrcc)ccpuins_extern_POP_ES;
	ccpuins.table[0x10] = (t_faddrcc)ccpuins_extern_ADC_Eb_Gb;
	ccpuins.table[0x11] = (t_faddrcc)ccpuins_extern_ADC_Ev_Gv;
	ccpuins.table[0x12] = (t_faddrcc)ccpuins_extern_ADC_Gb_Eb;
	ccpuins.table[0x13] = (t_faddrcc)ccpuins_extern_ADC_Gv_Ev;
	ccpuins.table[0x14] = (t_faddrcc)ccpuins_extern_ADC_AL_Ib;
	ccpuins.table[0x15] = (t_faddrcc)ccpuins_extern_ADC_eAX_Iv;
	ccpuins.table[0x16] = (t_faddrcc)ccpuins_extern_PUSH_SS;
	ccpuins.table[0x17] = (t_faddrcc)ccpuins_extern_POP_SS;
	ccpuins.table[0x20] = (t_faddrcc)ccpuins_extern_AND_Eb_Gb;
	ccpuins.table[0x21] = (t_faddrcc)ccpuins_extern_AND_Ev_Gv;
	ccpuins.table[0x22] = (t_faddrcc)ccpuins_extern_AND_Gb_Eb;
	ccpuins.table[0x23] = (t_faddrcc)ccpuins_extern_AND_Gv_Ev;
	ccpuins.table[0x24] = (t_faddrcc)ccpuins_extern_AND_AL_Ib;
	ccpuins.table[0x25] = (t_faddrcc)ccpuins_extern_AND_eAX_Iv;
	ccpuins.table[0x26] = (t_faddrcc)ccpuins_extern_SEG_ES;
	ccpuins.table[0x27] = (t_faddrcc)ccpuins_extern_DAA;
	ccpuins.table[0x30] = (t_faddrcc)ccpuins_extern_XOR_Eb_Gb;
	ccpuins.table[0x31] = (t_faddrcc)ccpuins_extern_XOR_Ev_Gv;
	ccpuins.table[0x32] = (t_faddrcc)ccpuins_extern_XOR_Gb_Eb;
	ccpuins.table[0x33] = (t_faddrcc)ccpuins_extern_XOR_Gv_Ev;
	ccpuins.table[0x34] = (t_faddrcc)ccpuins_extern_XOR_AL_Ib;
	ccpuins.table[0x35] = (t_faddrcc)ccpuins_extern_XOR_eAX_Iv;
	ccpuins.table[0x36] = (t_faddrcc)ccpuins_extern_SEG_SS;
	ccpuins.table[0x37] = (t_faddrcc)ccpuins_extern_AAA;
	ccpuins.table[0x40] = (t_faddrcc)ccpuins_extern_INC_eAX;
	ccpuins.table[0x41] = (t_faddrcc)ccpuins_extern_INC_eCX;
	ccpuins.table[0x42] = (t_faddrcc)ccpuins_extern_INC_eDX;
	ccpuins.table[0x43] = (t_faddrcc)ccpuins_extern_INC_eBX;
	ccpuins.table[0x44] = (t_faddrcc)ccpuins_extern_INC_eSP;
	ccpuins.table[0x45] = (t_faddrcc)ccpuins_extern_INC_eBP;
	ccpuins.table[0x46] = (t_faddrcc)ccpuins_extern_INC_eSI;
	ccpuins.table[0x47] = (t_faddrcc)ccpuins_extern_INC_eDI;
	ccpuins.table[0x50] = (t_faddrcc)ccpuins_extern_PUSH_eAX;
	ccpuins.table[0x51] = (t_faddrcc)ccpuins_extern_PUSH_eCX;
	ccpuins.table[0x52] = (t_faddrcc)ccpuins_extern_PUSH_eDX;
	ccpuins.table[0x53] = (t_faddrcc)ccpuins_extern_PUSH_eBX;
	ccpuins.table[0x54] = (t_faddrcc)ccpuins_extern_PUSH_eSP;
	ccpuins.table[0x55] = (t_faddrcc)ccpuins_extern_PUSH_eBP;
	ccpuins.table[0x56] = (t_faddrcc)ccpuins_extern_PUSH_eSI;
	ccpuins.table[0x57] = (t_faddrcc)ccpuins_extern_PUSH_eDI;
	ccpuins.table[0x60] = (t_faddrcc)ccpuins_extern_PUSHA_PUSHAD;
	ccpuins.table[0x61] = (t_faddrcc)ccpuins_extern_POPA_POPAD;
	ccpuins.table[0x62] = (t_faddrcc)ccpuins_extern_BOUND_Gv_Ma;
	ccpuins.table[0x63] = (t_faddrcc)ccpuins_extern_ARPL_Ew_Gw;
	ccpuins.table[0x64] = (t_faddrcc)ccpuins_extern_SEG_FS;
	ccpuins.table[0x65] = (t_faddrcc)ccpuins_extern_SEG_GS;
	ccpuins.table[0x66] = (t_faddrcc)ccpuins_extern_Operand_Size;
	ccpuins.table[0x67] = (t_faddrcc)ccpuins_extern_Address_Size;
	ccpuins.table[0x70] = (t_faddrcc)ccpuins_extern_ShortJump_JO;
	ccpuins.table[0x71] = (t_faddrcc)ccpuins_extern_ShortJump_JNO;
	ccpuins.table[0x72] = (t_faddrcc)ccpuins_extern_ShortJump_JB;
	ccpuins.table[0x73] = (t_faddrcc)ccpuins_extern_ShortJump_JNB;
	ccpuins.table[0x74] = (t_faddrcc)ccpuins_extern_ShortJump_JZ;
	ccpuins.table[0x75] = (t_faddrcc)ccpuins_extern_ShortJump_JNZ;
	ccpuins.table[0x76] = (t_faddrcc)ccpuins_extern_ShortJump_JBE;
	ccpuins.table[0x77] = (t_faddrcc)ccpuins_extern_ShortJump_JNBE;
	ccpuins.table[0x80] = (t_faddrcc)ccpuins_extern_IMMGroup_Eb_Ib;
	ccpuins.table[0x81] = (t_faddrcc)ccpuins_extern_IMMGroup_Ev_Iz;
	ccpuins.table[0x82] = (t_faddrcc)ccpuins_extern_IMMGroup_Eb_IbX;
	ccpuins.table[0x83] = (t_faddrcc)ccpuins_extern_IMMGroup_Ev_Ib;
	ccpuins.table[0x84] = (t_faddrcc)ccpuins_extern_TEST_Eb_Gb;
	ccpuins.table[0x85] = (t_faddrcc)ccpuins_extern_TEST_Ev_Gv;
	ccpuins.table[0x86] = (t_faddrcc)ccpuins_extern_XCHG_Eb_Gb;
	ccpuins.table[0x87] = (t_faddrcc)ccpuins_extern_XCHG_Ev_Gv;
	ccpuins.table[0x90] = (t_faddrcc)ccpuins_extern_NOP;
	ccpuins.table[0x91] = (t_faddrcc)ccpuins_extern_XCHG_eCX;
	ccpuins.table[0x92] = (t_faddrcc)ccpuins_extern_XCHG_eDX;
	ccpuins.table[0x93] = (t_faddrcc)ccpuins_extern_XCHG_eBX;
	ccpuins.table[0x94] = (t_faddrcc)ccpuins_extern_XCHG_eSP;
	ccpuins.table[0x95] = (t_faddrcc)ccpuins_extern_XCHG_eBP;
	ccpuins.table[0x96] = (t_faddrcc)ccpuins_extern_XCHG_eSI;
	ccpuins.table[0x97] = (t_faddrcc)ccpuins_extern_XCHG_eDI;
	ccpuins.table[0xA0] = (t_faddrcc)ccpuins_extern_MOV_AL_Ob;
	ccpuins.table[0xA1] = (t_faddrcc)ccpuins_extern_MOV_eAX_Ov;
	ccpuins.table[0xA2] = (t_faddrcc)ccpuins_extern_MOV_Ob_AL;
	ccpuins.table[0xA3] = (t_faddrcc)ccpuins_extern_MOV_Ov_eAX;
	ccpuins.table[0xA4] = (t_faddrcc)ccpuins_extern_MOVSB_Xb_Yb;
	ccpuins.table[0xA5] = (t_faddrcc)ccpuins_extern_MOVSW_Xv_Yv;
	ccpuins.table[0xA6] = (t_faddrcc)ccpuins_extern_CMPSB_Xb_Yb;
	ccpuins.table[0xA7] = (t_faddrcc)ccpuins_extern_CMPSW_Xv_Yv;
	ccpuins.table[0xB0] = (t_faddrcc)ccpuins_extern_MOV_AL;
	ccpuins.table[0xB1] = (t_faddrcc)ccpuins_extern_MOV_CL;
	ccpuins.table[0xB2] = (t_faddrcc)ccpuins_extern_MOV_DL;
	ccpuins.table[0xB3] = (t_faddrcc)ccpuins_extern_MOV_BL;
	ccpuins.table[0xB4] = (t_faddrcc)ccpuins_extern_MOV_AH;
	ccpuins.table[0xB5] = (t_faddrcc)ccpuins_extern_MOV_CH;
	ccpuins.table[0xB6] = (t_faddrcc)ccpuins_extern_MOV_DH;
	ccpuins.table[0xB7] = (t_faddrcc)ccpuins_extern_MOV_BH;
	ccpuins.table[0xC0] = (t_faddrcc)ccpuins_extern_ShiftGroup_Eb_Ib;
	ccpuins.table[0xC1] = (t_faddrcc)ccpuins_extern_ShiftGroup_Ev_Ib;
	ccpuins.table[0xC2] = (t_faddrcc)ccpuins_extern_RET_near_Iw;
	//that would be something
	ccpuins.table[0xC3] = (t_faddrcc)ccpuins_extern_RET_near_None;
	ccpuins.table[0xC4] = (t_faddrcc)ccpuins_extern_LES_Gv_Mp;
	ccpuins.table[0xC5] = (t_faddrcc)ccpuins_extern_LDS_Gv_Mp;
	ccpuins.table[0xC6] = (t_faddrcc)ccpuins_extern_MOV_Eb_Ib;
	ccpuins.table[0xC7] = (t_faddrcc)ccpuins_extern_MOV_Ev_Iv;
	ccpuins.table[0xD0] = (t_faddrcc)ccpuins_extern_ShiftGroup2_Eb_1;
	ccpuins.table[0xD1] = (t_faddrcc)ccpuins_extern_ShiftGroup2_Ev_1;
	ccpuins.table[0xD2] = (t_faddrcc)ccpuins_extern_ShiftGroup2_Eb_CL;
	ccpuins.table[0xD3] = (t_faddrcc)ccpuins_extern_ShiftGroup2_Ev_CL;
	ccpuins.table[0xD4] = (t_faddrcc)ccpuins_extern_AMM;
	ccpuins.table[0xD5] = (t_faddrcc)ccpuins_extern_AAD;
  //FOUCE ON IT
	ccpuins.table[0xD7] = (t_faddrcc)ccpuins_extern_XLAT;
	ccpuins.table[0xE0] = (t_faddrcc)ccpuins_extern_LOOPN_Jb;
	ccpuins.table[0xE1] = (t_faddrcc)ccpuins_extern_LOOPE_Jb;
	ccpuins.table[0xE2] = (t_faddrcc)ccpuins_extern_LOOP_Jb;
	ccpuins.table[0xE3] = (t_faddrcc)ccpuins_extern_JCXZ_Jb;
	ccpuins.table[0xE4] = (t_faddrcc)ccpuins_extern_IN_AL_Ib;
	ccpuins.table[0xE5] = (t_faddrcc)ccpuins_extern_IN_eAX_Ib;
	ccpuins.table[0xE6] = (t_faddrcc)ccpuins_extern_OUT_Ib_AL;
	ccpuins.table[0xE7] = (t_faddrcc)ccpuins_extern_OUT_Ib_eAX;
	ccpuins.table[0xF0] = (t_faddrcc)ccpuins_extern_LOCK;
	//SOME THING...
	ccpuins.table[0xF1] = (t_faddrcc)ccpuins_extern_Unknow;
	ccpuins.table[0xF2] = (t_faddrcc)ccpuins_extern_REPNE;
	ccpuins.table[0xF3] = (t_faddrcc)ccpuins_extern_REP_REPE;
	ccpuins.table[0xF4] = (t_faddrcc)ccpuins_extern_HLT;
	ccpuins.table[0xF5] = (t_faddrcc)ccpuins_extern_CMC;
	ccpuins.table[0xF6] = (t_faddrcc)ccpuins_extern_UnaryGroup_Eb;
	ccpuins.table[0xF7] = (t_faddrcc)ccpuins_extern_UnaryGroup_Ev;
	ccpuins.table[0x08] = (t_faddrcc)ccpuins_extern_OR_Eb_Gb;
	ccpuins.table[0x09] = (t_faddrcc)ccpuins_extern_OR_Ev_Gv;
	ccpuins.table[0x0A] = (t_faddrcc)ccpuins_extern_OR_Gb_Eb;
	ccpuins.table[0x0B] = (t_faddrcc)ccpuins_extern_OR_Gv_Ev;
	ccpuins.table[0x0C] = (t_faddrcc)ccpuins_extern_OR_AL_Ib;
	ccpuins.table[0x0D] = (t_faddrcc)ccpuins_extern_OR_eAX_Iv;
	ccpuins.table[0x0E] = (t_faddrcc)ccpuins_extern_PUSH_CS;
	ccpuins.table[0x0F] = (t_faddrcc)ccpuins_extern_EscapeCode;
	ccpuins.table[0x18] = (t_faddrcc)ccpuins_extern_SBB_Eb_Gb;
	ccpuins.table[0x19] = (t_faddrcc)ccpuins_extern_SBB_Ev_Gv;
	ccpuins.table[0x1A] = (t_faddrcc)ccpuins_extern_SBB_Gb_Eb;
	ccpuins.table[0x1B] = (t_faddrcc)ccpuins_extern_SBB_Gv_Ev;
	ccpuins.table[0x1C] = (t_faddrcc)ccpuins_extern_SBB_AL_Ib;
	ccpuins.table[0x1D] = (t_faddrcc)ccpuins_extern_SBB_eAX_Iv;
	ccpuins.table[0x1E] = (t_faddrcc)ccpuins_extern_PUSH_DS;
	ccpuins.table[0x1F] = (t_faddrcc)ccpuins_extern_POP_DS;
	ccpuins.table[0x28] = (t_faddrcc)ccpuins_extern_SUB_Eb_Gb;
	ccpuins.table[0x29] = (t_faddrcc)ccpuins_extern_SUB_Ev_Gv;
	ccpuins.table[0x2A] = (t_faddrcc)ccpuins_extern_SUB_Gb_Eb;
	ccpuins.table[0x2B] = (t_faddrcc)ccpuins_extern_SUB_Gv_Ev;
	ccpuins.table[0x2C] = (t_faddrcc)ccpuins_extern_SUB_AL_Ib;
	ccpuins.table[0x2D] = (t_faddrcc)ccpuins_extern_SUB_eAX_Iv;
	ccpuins.table[0x2E] = (t_faddrcc)ccpuins_extern_SEG_CS;
	ccpuins.table[0x2F] = (t_faddrcc)ccpuins_extern_DAS;
	ccpuins.table[0x38] = (t_faddrcc)ccpuins_extern_CMP_Eb_Gb;
	ccpuins.table[0x39] = (t_faddrcc)ccpuins_extern_CMP_Ev_Gv;
	ccpuins.table[0x3A] = (t_faddrcc)ccpuins_extern_CMP_Gb_Eb;
	ccpuins.table[0x3B] = (t_faddrcc)ccpuins_extern_CMP_Gv_Ev;
	ccpuins.table[0x3C] = (t_faddrcc)ccpuins_extern_CMP_AL_Ib;
	ccpuins.table[0x3D] = (t_faddrcc)ccpuins_extern_CMP_eAX_Iv;
	ccpuins.table[0x3E] = (t_faddrcc)ccpuins_extern_SEG_DS;
	ccpuins.table[0x3F] = (t_faddrcc)ccpuins_extern_AAS;
	ccpuins.table[0x48] = (t_faddrcc)ccpuins_extern_DEC_eAX;
	ccpuins.table[0x49] = (t_faddrcc)ccpuins_extern_DEC_eCX;
	ccpuins.table[0x4A] = (t_faddrcc)ccpuins_extern_DEC_eDX;
	ccpuins.table[0x4B] = (t_faddrcc)ccpuins_extern_DEC_eBX;
	ccpuins.table[0x4C] = (t_faddrcc)ccpuins_extern_DEC_eSP;
	ccpuins.table[0x4D] = (t_faddrcc)ccpuins_extern_DEC_eBP;
	ccpuins.table[0x4E] = (t_faddrcc)ccpuins_extern_DEC_eSI;
	ccpuins.table[0x4F] = (t_faddrcc)ccpuins_extern_DEC_eDI;
	ccpuins.table[0x58] = (t_faddrcc)ccpuins_extern_POP_eAX;
	ccpuins.table[0x59] = (t_faddrcc)ccpuins_extern_POP_eCX;
	ccpuins.table[0x5A] = (t_faddrcc)ccpuins_extern_POP_eDX;
	ccpuins.table[0x5B] = (t_faddrcc)ccpuins_extern_POP_eBX;
	ccpuins.table[0x5C] = (t_faddrcc)ccpuins_extern_POP_eSP;
	ccpuins.table[0x5D] = (t_faddrcc)ccpuins_extern_POP_eBP;
	ccpuins.table[0x5E] = (t_faddrcc)ccpuins_extern_POP_eSI;
	ccpuins.table[0x5F] = (t_faddrcc)ccpuins_extern_POP_eDI;
	ccpuins.table[0x68] = (t_faddrcc)ccpuins_extern_PUSH_Iv;
	ccpuins.table[0x69] = (t_faddrcc)ccpuins_extern_IMUL_Gv_Ev_Iv;
	ccpuins.table[0x6A] = (t_faddrcc)ccpuins_extern_PUSH_Ib;
	ccpuins.table[0x6B] = (t_faddrcc)ccpuins_extern_IMUL_Gv_Ev_Ib;
	ccpuins.table[0x6C] = (t_faddrcc)ccpuins_extern_INSB_Yb_DX;
	ccpuins.table[0x6D] = (t_faddrcc)ccpuins_extern_INSW_Yv_DX;
	ccpuins.table[0x6E] = (t_faddrcc)ccpuins_extern_OUTSB_DX_Xb;
	ccpuins.table[0x6F] = (t_faddrcc)ccpuins_extern_OUTSW_DX_Xv;
	ccpuins.table[0x78] = (t_faddrcc)ccpuins_extern_ShortJumpCondition_JS;
	ccpuins.table[0x79] = (t_faddrcc)ccpuins_extern_ShortJumpCondition_JNS;
	ccpuins.table[0x7A] = (t_faddrcc)ccpuins_extern_ShortJumpCondition_JP;
	ccpuins.table[0x7B] = (t_faddrcc)ccpuins_extern_ShortJumpCondition_JNP;
	ccpuins.table[0x7C] = (t_faddrcc)ccpuins_extern_ShortJumpCondition_JL;
	ccpuins.table[0x7D] = (t_faddrcc)ccpuins_extern_ShortJumpCondition_JNL;
	ccpuins.table[0x7E] = (t_faddrcc)ccpuins_extern_ShortJumpCondition_JLE;
	ccpuins.table[0x7F] = (t_faddrcc)ccpuins_extern_ShortJumpCondition_JNLE;
	ccpuins.table[0x88] = (t_faddrcc)ccpuins_extern_MOV_Eb_Gb;
	ccpuins.table[0x89] = (t_faddrcc)ccpuins_extern_MOV_Ev_Gv;
	ccpuins.table[0x8A] = (t_faddrcc)ccpuins_extern_MOV_Gb_Eb;
	ccpuins.table[0x8B] = (t_faddrcc)ccpuins_extern_MOV_Gv_Ev;
	ccpuins.table[0x8C] = (t_faddrcc)ccpuins_extern_MOV_Ew_Sw;
	ccpuins.table[0x8D] = (t_faddrcc)ccpuins_extern_LEA_Gv_M;
	ccpuins.table[0x8E] = (t_faddrcc)ccpuins_extern_MOV_Sw_Ew;
	ccpuins.table[0x8F] = (t_faddrcc)ccpuins_extern_POP_Ev;
	ccpuins.table[0x98] = (t_faddrcc)ccpuins_extern_CBW;
	ccpuins.table[0x99] = (t_faddrcc)ccpuins_extern_CWD;
	ccpuins.table[0x9A] = (t_faddrcc)ccpuins_extern_CALL_aP;
	ccpuins.table[0x9B] = (t_faddrcc)ccpuins_extern_WAIT;
	ccpuins.table[0x9C] = (t_faddrcc)ccpuins_extern_PUSHF_Fv;
	ccpuins.table[0x9D] = (t_faddrcc)ccpuins_extern_POP_Fv;
	ccpuins.table[0x9E] = (t_faddrcc)ccpuins_extern_SAHF;
	ccpuins.table[0x9F] = (t_faddrcc)ccpuins_extern_LAHF;
	ccpuins.table[0xA8] = (t_faddrcc)ccpuins_extern_TEST_AL_Ib;
	ccpuins.table[0xA9] = (t_faddrcc)ccpuins_extern_TEST_eAX_Iv;
	ccpuins.table[0xAA] = (t_faddrcc)ccpuins_extern_STOSB_Yb_AL;
	ccpuins.table[0xAB] = (t_faddrcc)ccpuins_extern_STOSW_Yv_eAX;
	ccpuins.table[0xAC] = (t_faddrcc)ccpuins_extern_LODSB_AL_Xb;
	ccpuins.table[0xAD] = (t_faddrcc)ccpuins_extern_LODSW_eAX_Xv;
	ccpuins.table[0xAE] = (t_faddrcc)ccpuins_extern_SCASB_AL_Yb;
	ccpuins.table[0xAF] = (t_faddrcc)ccpuins_extern_SCASW_eAX_Yv;
	ccpuins.table[0xB8] = (t_faddrcc)ccpuins_extern_MOV_eAX;
	ccpuins.table[0xB9] = (t_faddrcc)ccpuins_extern_MOV_eCX;
	ccpuins.table[0xBA] = (t_faddrcc)ccpuins_extern_MOV_eDX;
	ccpuins.table[0xBB] = (t_faddrcc)ccpuins_extern_MOV_eBX;
	ccpuins.table[0xBC] = (t_faddrcc)ccpuins_extern_MOV_eSP;
	ccpuins.table[0xBD] = (t_faddrcc)ccpuins_extern_MOV_eBP;
	ccpuins.table[0xBE] = (t_faddrcc)ccpuins_extern_MOV_eSI;
	ccpuins.table[0xBF] = (t_faddrcc)ccpuins_extern_MOV_eDI;
	ccpuins.table[0xC8] = (t_faddrcc)ccpuins_extern_ENTER_Iw_Ib;
	ccpuins.table[0xC9] = (t_faddrcc)ccpuins_extern_LEAVE;
	ccpuins.table[0xCA] = (t_faddrcc)ccpuins_extern_RET_far_Iw;
	ccpuins.table[0xCB] = (t_faddrcc)ccpuins_extern_RET_far;
	ccpuins.table[0xCC] = (t_faddrcc)ccpuins_extern_INT_3;
	ccpuins.table[0xCD] = (t_faddrcc)ccpuins_extern_INT_Ib;
	ccpuins.table[0xCE] = (t_faddrcc)ccpuins_extern_INTO;
	ccpuins.table[0xCF] = (t_faddrcc)ccpuins_extern_IRET;
	ccpuins.table[0xD8] = (t_faddrcc)ccpuins_extern_ESC;
	ccpuins.table[0xD9] = (t_faddrcc)ccpuins_extern_ESC;
	ccpuins.table[0xDA] = (t_faddrcc)ccpuins_extern_ESC;
	ccpuins.table[0xDB] = (t_faddrcc)ccpuins_extern_ESC;
	ccpuins.table[0xDC] = (t_faddrcc)ccpuins_extern_ESC;
	ccpuins.table[0xDD] = (t_faddrcc)ccpuins_extern_ESC;
	ccpuins.table[0xDE] = (t_faddrcc)ccpuins_extern_ESC;
	ccpuins.table[0xDF] = (t_faddrcc)ccpuins_extern_ESC;
	ccpuins.table[0xE8] = (t_faddrcc)ccpuins_extern_CALL_Jv;
	ccpuins.table[0xE9] = (t_faddrcc)ccpuins_extern_JMP_Jv;
	ccpuins.table[0xEA] = (t_faddrcc)ccpuins_extern_JMP_Ap;
	ccpuins.table[0xEB] = (t_faddrcc)ccpuins_extern_JMP_Jb;
	ccpuins.table[0xEC] = (t_faddrcc)ccpuins_extern_IN_AL_DX;
	ccpuins.table[0xED] = (t_faddrcc)ccpuins_extern_IN_eAX_DX;
	ccpuins.table[0xEE] = (t_faddrcc)ccpuins_extern_OUT_DX_AL;
	ccpuins.table[0xEF] = (t_faddrcc)ccpuins_extern_OUT_DX_eAX;
	ccpuins.table[0xF8] = (t_faddrcc)ccpuins_extern_CLC;
	ccpuins.table[0xF9] = (t_faddrcc)ccpuins_extern_STC;
	ccpuins.table[0xFA] = (t_faddrcc)ccpuins_extern_CLI;
	ccpuins.table[0xFB] = (t_faddrcc)ccpuins_extern_STI;
	ccpuins.table[0xFC] = (t_faddrcc)ccpuins_extern_CLD;
	ccpuins.table[0xFD] = (t_faddrcc)ccpuins_extern_STD;
	ccpuins.table[0xFE] = (t_faddrcc)ccpuins_extern_INC_Group4;
	ccpuins.table[0xFF] = (t_faddrcc)ccpuins_extern_INC_Group5;
}
void ccpuinsFinal() {}