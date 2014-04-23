#ifndef _CENTREPROCESSORUNIT_H
#define _CENTREPROCESSORUNIT_H

#include "../vglobal.h"
#include <assert.h>
#include <string>

using namespace std;

#define getMemData_byte(addr) vramVarByte(0,(addr))
#define getMemData_word(addr) vramVarWord(0,(addr))
#define writeMem_byte(addr, data) (vramVarByte(0,(addr)) = (data))
#define writeMem_word(addr, data) (vramVarWord(0,(addr)) = (data))

enum DataTypeLength{dataType_U1=1, dataType_S1=1, dataType_U2=2, dataType_S2=2, dataType_U4=4, dataType_S4=4};
#define MASK_00000000  0
#define MASK_00000001  1
#define MASK_00000010  2
#define MASK_00000011  3
#define MASK_00000100  4
#define MASK_00000101  5
#define MASK_00000110  6
#define MASK_00000111  7
#define MASK_00001000  8
#define MASK_00001001  9
#define MASK_00001010  10
#define MASK_00001011  11
#define MASK_00001100  12
#define MASK_00001101  13
#define MASK_00001110  14
#define MASK_00001111  15
#define MASK_00010000  16
#define MASK_00010001  17
#define MASK_00010010  18
#define MASK_00010011  19
#define MASK_00010100  20
#define MASK_00010101  21
#define MASK_00010110  22
#define MASK_00010111  23
#define MASK_00011000  24
#define MASK_00011001  25
#define MASK_00011010  26
#define MASK_00011011  27
#define MASK_00011100  28
#define MASK_00011101  29
#define MASK_00011110  30
#define MASK_00011111  31
#define MASK_00100000  32
#define MASK_00100001  33
#define MASK_00100010  34
#define MASK_00100011  35
#define MASK_00100100  36
#define MASK_00100101  37
#define MASK_00100110  38
#define MASK_00100111  39
#define MASK_00101000  40
#define MASK_00101001  41
#define MASK_00101010  42
#define MASK_00101011  43
#define MASK_00101100  44
#define MASK_00101101  45
#define MASK_00101110  46
#define MASK_00101111  47
#define MASK_00110000  48
#define MASK_00110001  49
#define MASK_00110010  50
#define MASK_00110011  51
#define MASK_00110100  52
#define MASK_00110101  53
#define MASK_00110110  54
#define MASK_00110111  55
#define MASK_00111000  56
#define MASK_00111001  57
#define MASK_00111010  58
#define MASK_00111011  59
#define MASK_00111100  60
#define MASK_00111101  61
#define MASK_00111110  62
#define MASK_00111111  63
#define MASK_01000000  64
#define MASK_01000001  65
#define MASK_01000010  66
#define MASK_01000011  67
#define MASK_01000100  68
#define MASK_01000101  69
#define MASK_01000110  70
#define MASK_01000111  71
#define MASK_01001000  72
#define MASK_01001001  73
#define MASK_01001010  74
#define MASK_01001011  75
#define MASK_01001100  76
#define MASK_01001101  77
#define MASK_01001110  78
#define MASK_01001111  79
#define MASK_01010000  80
#define MASK_01010001  81
#define MASK_01010010  82
#define MASK_01010011  83
#define MASK_01010100  84
#define MASK_01010101  85
#define MASK_01010110  86
#define MASK_01010111  87
#define MASK_01011000  88
#define MASK_01011001  89
#define MASK_01011010  90
#define MASK_01011011  91
#define MASK_01011100  92
#define MASK_01011101  93
#define MASK_01011110  94
#define MASK_01011111  95
#define MASK_01100000  96
#define MASK_01100001  97
#define MASK_01100010  98
#define MASK_01100011  99
#define MASK_01100100  100
#define MASK_01100101  101
#define MASK_01100110  102
#define MASK_01100111  103
#define MASK_01101000  104
#define MASK_01101001  105
#define MASK_01101010  106
#define MASK_01101011  107
#define MASK_01101100  108
#define MASK_01101101  109
#define MASK_01101110  110
#define MASK_01101111  111
#define MASK_01110000  112
#define MASK_01110001  113
#define MASK_01110010  114
#define MASK_01110011  115
#define MASK_01110100  116
#define MASK_01110101  117
#define MASK_01110110  118
#define MASK_01110111  119
#define MASK_01111000  120
#define MASK_01111001  121
#define MASK_01111010  122
#define MASK_01111011  123
#define MASK_01111100  124
#define MASK_01111101  125
#define MASK_01111110  126
#define MASK_01111111  127
#define MASK_10000000  128
#define MASK_10000001  129
#define MASK_10000010  130
#define MASK_10000011  131
#define MASK_10000100  132
#define MASK_10000101  133
#define MASK_10000110  134
#define MASK_10000111  135
#define MASK_10001000  136
#define MASK_10001001  137
#define MASK_10001010  138
#define MASK_10001011  139
#define MASK_10001100  140
#define MASK_10001101  141
#define MASK_10001110  142
#define MASK_10001111  143
#define MASK_10010000  144
#define MASK_10010001  145
#define MASK_10010010  146
#define MASK_10010011  147
#define MASK_10010100  148
#define MASK_10010101  149
#define MASK_10010110  150
#define MASK_10010111  151
#define MASK_10011000  152
#define MASK_10011001  153
#define MASK_10011010  154
#define MASK_10011011  155
#define MASK_10011100  156
#define MASK_10011101  157
#define MASK_10011110  158
#define MASK_10011111  159
#define MASK_10100000  160
#define MASK_10100001  161
#define MASK_10100010  162
#define MASK_10100011  163
#define MASK_10100100  164
#define MASK_10100101  165
#define MASK_10100110  166
#define MASK_10100111  167
#define MASK_10101000  168
#define MASK_10101001  169
#define MASK_10101010  170
#define MASK_10101011  171
#define MASK_10101100  172
#define MASK_10101101  173
#define MASK_10101110  174
#define MASK_10101111  175
#define MASK_10110000  176
#define MASK_10110001  177
#define MASK_10110010  178
#define MASK_10110011  179
#define MASK_10110100  180
#define MASK_10110101  181
#define MASK_10110110  182
#define MASK_10110111  183
#define MASK_10111000  184
#define MASK_10111001  185
#define MASK_10111010  186
#define MASK_10111011  187
#define MASK_10111100  188
#define MASK_10111101  189
#define MASK_10111110  190
#define MASK_10111111  191
#define MASK_11000000  192
#define MASK_11000001  193
#define MASK_11000010  194
#define MASK_11000011  195
#define MASK_11000100  196
#define MASK_11000101  197
#define MASK_11000110  198
#define MASK_11000111  199
#define MASK_11001000  200
#define MASK_11001001  201
#define MASK_11001010  202
#define MASK_11001011  203
#define MASK_11001100  204
#define MASK_11001101  205
#define MASK_11001110  206
#define MASK_11001111  207
#define MASK_11010000  208
#define MASK_11010001  209
#define MASK_11010010  210
#define MASK_11010011  211
#define MASK_11010100  212
#define MASK_11010101  213
#define MASK_11010110  214
#define MASK_11010111  215
#define MASK_11011000  216
#define MASK_11011001  217
#define MASK_11011010  218
#define MASK_11011011  219
#define MASK_11011100  220
#define MASK_11011101  221
#define MASK_11011110  222
#define MASK_11011111  223
#define MASK_11100000  224
#define MASK_11100001  225
#define MASK_11100010  226
#define MASK_11100011  227
#define MASK_11100100  228
#define MASK_11100101  229
#define MASK_11100110  230
#define MASK_11100111  231
#define MASK_11101000  232
#define MASK_11101001  233
#define MASK_11101010  234
#define MASK_11101011  235
#define MASK_11101100  236
#define MASK_11101101  237
#define MASK_11101110  238
#define MASK_11101111  239
#define MASK_11110000  240
#define MASK_11110001  241
#define MASK_11110010  242
#define MASK_11110011  243
#define MASK_11110100  244
#define MASK_11110101  245
#define MASK_11110110  246
#define MASK_11110111  247
#define MASK_11111000  248
#define MASK_11111001  249
#define MASK_11111010  250
#define MASK_11111011  251
#define MASK_11111100  252
#define MASK_11111101  253
#define MASK_11111110  254
#define MASK_11111111  255

const int OPCODE_WIDTH = 1;
const int MOD_REG_RM_LENGTH = 1;
class DeCodeBlock;
class ExecuteCodeBlock;
class CentreProcessorUnit;
class MotherBoard;

const t_nubit16 MASK_FLAG_OF = 0x800;
const t_nubit16 MASK_FLAG_DF = 0x400;
const t_nubit16 MASK_FLAG_IF = 0x200;
const t_nubit16 MASK_FLAG_TF = 0x100;
const t_nubit16 MASK_FLAG_SF = 0x80;
const t_nubit16 MASK_FLAG_ZF = 0x40;
const t_nubit16 MASK_FLAG_AF = 0x10;
const t_nubit16 MASK_FLAG_PF = 0x4;
const t_nubit16 MASK_FLAG_CF = 0x1;


const t_nubit16 MASK_FLAG_METHOD_ADD = MASK_FLAG_OF | MASK_FLAG_SF | MASK_FLAG_ZF | MASK_FLAG_AF | MASK_FLAG_PF | MASK_FLAG_CF;
const t_nubit16 MASK_FLAG_METHOD_INC = MASK_FLAG_OF | MASK_FLAG_SF | MASK_FLAG_ZF | MASK_FLAG_AF | MASK_FLAG_PF;//inc不会影响CF
const t_nubit16 MASK_FLAG_METHOD_SUB = MASK_FLAG_METHOD_ADD;
const t_nubit16 MASK_FLAG_METHOD_DEC = MASK_FLAG_METHOD_INC;
const t_nubit16 MASK_FLAG_METHOD_ADC = MASK_FLAG_METHOD_ADD;
const t_nubit16 MASK_FLAG_METHOD_CMP = MASK_FLAG_METHOD_ADD;
const t_nubit16 MASK_FLAG_METHOD_CMPS = MASK_FLAG_METHOD_ADD;
const t_nubit16 MASK_FLAG_METHOD_NEG = MASK_FLAG_METHOD_ADD;
const t_nubit16 MASK_FLAG_METHOD_SBB = MASK_FLAG_METHOD_ADD;
const t_nubit16 MASK_FLAG_METHOD_SCAS = MASK_FLAG_METHOD_ADD;

const t_nubit16 MASK_FLAG_METHOD_OR = MASK_FLAG_SF | MASK_FLAG_ZF | MASK_FLAG_PF ;
const t_nubit16 MASK_FLAG_METHOD_AND = MASK_FLAG_METHOD_OR;
const t_nubit16 MASK_FLAG_METHOD_XOR = MASK_FLAG_METHOD_OR;
enum Method{ADD_8bit, ADD_16bit,
			SUB_8bit, SUB_16bit,
			ADC_8bit, ADC_16bit,
			SBB_8bit, SBB_16bit,
			NEG_8bit, NEG_16bit,
			INC_8bit, INC_16bit,
			DEC_8bit, DEC_16bit,
			OR_8bit,  OR_16bit,
			AND_8bit, AND_16bit,
			XOR_8bit, XOR_16bit,
			CMP_8bit, CMP_16bit,
			MOD_8bit,/*为了AAM设的*/};

const t_nubit8 CS_SEGMENT_OVERRIDE_PREFIX = 0x2E;
const t_nubit8 SS_SEGMENT_OVERRIDE_PREFIX = 0x36;
const t_nubit8 DS_SEGMENT_OVERRIDE_PREFIX = 0x3E;
const t_nubit8 ES_SEGMENT_OVERRIDE_PREFIX = 0x26;

const t_nubit8 LOCK_PREFIX = 0xf0;
const t_nubit8 REPNE_PREFIX = 0xf2;
const t_nubit8 REP_PREFIX = 0xf3;//和下面这个值相同
const t_nubit8 REPE_PREFIX = 0xf3;

class DeCodeBlock
{
public:
	DeCodeBlock(CentreProcessorUnit* pCPU);
	void init();
	void deCodeInstruction();
	CentreProcessorUnit* pCPU;
	ExecuteCodeBlock* pexeCodeBlock;

public:
	void *prm;
	void *preg;
	void *pseg;
	t_nubit8 immData_8Bit, disp_8Bit;
	t_nubit16 immData_16Bit, disp_16Bit;
	t_nubit8 opContactData_8bit;//紧跟在opcode后的数据
	t_nubit16 opContactData_16bit,opContactData_16bitE;
	t_nubit8 nnn; //Group中ModRegRm 中的nnn
	t_nubit32 segData;//for LEA..存储了段寄存器的值

	t_nubit8 prefix_LockOpCode, prefix_RepeatOpCode; //Lock and repeat prefixes
	t_nubit8 prefix_SegmentOverrideOpCode; //更改默认段的指令
	t_nubit8 prefix_OperandSize;
	t_nubit8 prefix_AddressSize;

private:
	void refresh();//获取编译下条指令的必要数据
	void * deCodeEffectiveMemAddr(const t_nubit8 modRegRm, const void * pdispData, int* pdispLength ) ;
	void * getRegAddr(const bool w, const t_nubit8 reg) const;
	t_nubit16* getSegAddr(const t_nubit8 sreg) const;
	void deCodeInsStuff();
	void setExecuteMethod();

	void deOpCode_singleDefine();//done
	void deOpCode_doubleDefine();//done
	int deOpCode_modRegRm();//done
	void deOpCode_IMM();//done
	void deOpCode_IMMGroup();//done
	void deOpCode_ModRM();
	void deOpCode_contactData_8bit();
	void deOpCode_contactData_16bit();//done
	void deOpCode_contactData_32bit();//done
	void deOpCode_Mov_mem_AX();//存储器到累加器，或累加器到存储器(互传)  done
	void deOpCode_Mov_imm_rm();//第七种，立即数到存储器/寄存器 done
	void deOpCode_Mov_rm_seg();//第三种，从寄存器/存储器到段存储器传送（互传）done
	void deOpCode_Mov_imm_reg();//第六种，立即数到寄存器
	void deOpCode_modRm();
	void deOpCode_TEST_Ib_Iv();


private:
	t_nubit32 SPC, TPC;
	t_nubit8 * pOpCode;
	bool isWidth; //当前翻译指令w位是否。。。
	
public:
	t_nubit16 getDefaultSegment_DS();
	t_nubit16 getDefaultSegment_SS();
};


typedef void (ExecuteCodeBlock::*ExecuteMethod)();
class ExecuteCodeBlock
{
public:
	ExecuteCodeBlock(CentreProcessorUnit* pCPU);
	void setNeedExeMethod(const ExecuteMethod pMethod);
	void initExecuteMethod();
	void init();	CentreProcessorUnit* pCPU;
	DeCodeBlock* pdeCodeBlock;
	ExecuteMethod exeMethodArray[0xff+1];
	ExecuteMethod needExeMethod;
	ExecuteMethod repeatMethod;
	void atomMethod_MOV_8bit(t_nubit32 logicAddr_des, const t_nubit8 data_src);
	void atomMethod_MOV_16bit(t_nubit32 logicAddr_des, const t_nubit16 data_src);
	void atomMethod_ADD_8bit(t_nubit8& des, const t_nubit8 src);
	void atomMethod_ADD_16bit(t_nubit16& des, const t_nubit16 src);
	void atomMethod_INC_8bit(t_nubit8& des);
	void atomMethod_INC_16bit(t_nubit16& des);
	void atomMethod_ADC_8bit(t_nubit8& des, const t_nubit8 src);
	void atomMethod_ADC_16bit(t_nubit16& des, const t_nubit16 src);
	void atomMethod_SUB_8bit(t_nubit8& des, const t_nubit8 src);
	void atomMethod_SUB_16bit(t_nubit16& des, const t_nubit16 src);
	void atomMethod_DEC_8bit(t_nubit8& des);
	void atomMethod_DEC_16bit(t_nubit16& des);
	void atomMethod_SBB_8bit(t_nubit8& des, const t_nubit8 src);
	void atomMethod_SBB_16bit(t_nubit16& des, const t_nubit16 src);
	void atomMethod_OR_8bit(t_nubit8& des, const t_nubit8 src);
	void atomMethod_OR_16bit(t_nubit16& des, const t_nubit16 src);
	void atomMethod_AND_8bit(t_nubit8& des, const t_nubit8 src);
	void atomMethod_AND_16bit(t_nubit16& des, const t_nubit16 src);
	void atomMethod_XOR_8bit(t_nubit8& des, const t_nubit8 src);
	void atomMethod_XOR_16bit(t_nubit16& des, const t_nubit16 src);
	void atomMethod_CMP_8bit(t_nubit8 des, const t_nubit8 src);
	void atomMethod_CMP_16bit(t_nubit16 des, const t_nubit16 src);
	void atomMethod_CMPSB(); 
	void atomMethod_CMPSW();
	void atomMethod_MOVSB();
	void atomMethod_MOVSW();
	void atomMethod_STOSB();
	void atomMethod_STOSW();
	void atomMethod_LODSB();
	void atomMethod_LODSW();
	void atomMethod_SCASB();
	void atomMethod_SCASW();
	void atomMethod_INS();
	void atomMethod_OUTS();
	void atomMethod_shortJMP();
	void atomMethod_PUSH(const t_nubit16 data);
	t_nubit16   atomMethod_POP();
	void atomMethod_INT(const t_nubit8 INT_num);
	
	void method_ADD_Eb_Gb();
	void method_ADD_Ev_Gv();
	void method_ADD_Gb_Eb();
	void method_ADD_Gv_Ev();
	void method_ADD_AL_Ib();
	void method_ADD_eAX_Iv();

	void method_PUSH_ES();
	void method_POP_ES();


	void method_ADC_Eb_Gb();
	void method_ADC_Ev_Gv();
	void method_ADC_Gb_Eb();
	void method_ADC_Gv_Ev();
	void method_ADC_AL_Ib();
	void method_ADC_eAX_Iv();

	void method_PUSH_SS();
	void method_POP_SS();

	void method_AND_Eb_Gb();
	void method_AND_Ev_Gv();
	void method_AND_Gb_Eb();
	void method_AND_Gv_Ev();
	void method_AND_AL_Ib();
	void method_AND_eAX_Iv();

	void method_SEG_ES();
	void method_DAA();

	void method_XOR_Eb_Gb();
	void method_XOR_Ev_Gv();
	void method_XOR_Gb_Eb();
	void method_XOR_Gv_Ev();
	void method_XOR_AL_Ib();
	void method_XOR_eAX_Iv();

	void method_SEG_SS();
	void method_AAA();

	void method_INC_eAX();
	void method_INC_eCX();
	void method_INC_eDX();
	void method_INC_eBX();
	void method_INC_eSP();
	void method_INC_eBP();
	void method_INC_eSI();
	void method_INC_eDI();

	void method_PUSH_eAX();
	void method_PUSH_eCX();
	void method_PUSH_eDX();
	void method_PUSH_eBX();
	void method_PUSH_eSP();
	void method_PUSH_eBP();
	void method_PUSH_eSI();
	void method_PUSH_eDI();

	void method_PUSHA_PUSHAD();
	void method_POPA_POPAD();
	void method_BOUND_Gv_Ma();
	void method_ARPL_Ew_Gw();
	void method_SEG_FS();
	void method_SEG_GS();
	void method_Operand_Size();
	void method_Address_Size();

	void method_ShortJump_JO();
	void method_ShortJump_JNO();
	void method_ShortJump_JB();
	void method_ShortJump_JNB();
	void method_ShortJump_JZ();
	void method_ShortJump_JNZ();
	void method_ShortJump_JBE();
	void method_ShortJump_JNBE();

	void method_IMMGroup_Eb_Ib();
	void method_IMMGroup_Ev_Iz();
	void method_IMMGroup_Eb_IbX();
	void method_IMMGroup_Ev_Ib();

	void method_TEST_Eb_Gb();
	void method_TEST_Ev_Gv();

	void method_XCHG_Eb_Gb();
	void method_XCHG_Ev_Gv();

	void method_NOP();

	void method_XCHG_eCX();
	void method_XCHG_eDX();
	void method_XCHG_eBX();
	void method_XCHG_eSP();
	void method_XCHG_eBP();
	void method_XCHG_eSI();
	void method_XCHG_eDI();

	void method_MOV_AL_Ob();
	void method_MOV_eAX_Ov();
	void method_MOV_Ob_AL();
	void method_MOV_Ov_eAX();

	void method_MOVSB_Xb_Yb();
	void method_MOVSW_Xv_Yv();
	void method_CMPSB_Xb_Yb();
	void method_CMPSW_Xv_Yv();

	void method_MOV_AL();
	void method_MOV_CL();
	void method_MOV_DL();
	void method_MOV_BL();
	void method_MOV_AH();
	void method_MOV_CH();
	void method_MOV_DH();
	void method_MOV_BH();

	void method_ShiftGroup_Eb_Ib();
	void method_ShiftGroup_Ev_Ib();
	void method_RET_near_Iw();
	//that would be something
	void method_RET_near_None();

	void method_LES_Gv_Mp();
	void method_LDS_Gv_Mp();

	void method_MOV_Eb_Ib();
	void method_MOV_Ev_Iv();

	void method_ShiftGroup2_Eb_1();
	void method_ShiftGroup2_Ev_1();
	void method_ShiftGroup2_Eb_CL();
	void method_ShiftGroup2_Ev_CL();

	void method_AMM();
	void method_AAD();

	void method_XLAT();

	void method_LOOPN_Jb();
	void method_LOOPE_Jb();
	void method_LOOP_Jb();
	void method_JCXZ_Jb();

	void method_IN_AL_Ib();
	void method_IN_eAX_Ib();
	void method_OUT_Ib_AL();
	void method_OUT_Ib_eAX();

	void method_LOCK();
	void method_REPNE();
	void method_REP_REPE();
	void method_HLT();
	void method_CMC();
	void method_UnaryGroup_Eb();
	void method_UnaryGroup_Ev();

	void method_OR_Eb_Gb();
	void method_OR_Ev_Gv();
	void method_OR_Gb_Eb();
	void method_OR_Gv_Ev();
	void method_OR_AL_Ib();
	void method_OR_eAX_Iv();

	void method_PUSH_CS();
	void method_EscapeCode();

	void method_SBB_Eb_Gb();
	void method_SBB_Ev_Gv();
	void method_SBB_Gb_Eb();
	void method_SBB_Gv_Ev();
	void method_SBB_AL_Ib();
	void method_SBB_eAX_Iv();

	void method_PUSH_DS();
	void method_POP_DS();

	void method_SUB_Eb_Gb();
	void method_SUB_Ev_Gv();
	void method_SUB_Gb_Eb();
	void method_SUB_Gv_Ev();
	void method_SUB_AL_Ib();
	void method_SUB_eAX_Iv();

	void method_SEG_CS();
	void method_DAS();

	void method_CMP_Eb_Gb();
	void method_CMP_Ev_Gv();
	void method_CMP_Gb_Eb();
	void method_CMP_Gv_Ev();
	void method_CMP_AL_Ib();
	void method_CMP_eAX_Iv();

	void method_SEG_DS();
	void method_AAS();

	void method_DEC_eAX();
	void method_DEC_eCX();
	void method_DEC_eDX();
	void method_DEC_eBX();
	void method_DEC_eSP();
	void method_DEC_eBP();
	void method_DEC_eSI();
	void method_DEC_eDI();

	void method_POP_eAX();
	void method_POP_eCX();
	void method_POP_eDX();
	void method_POP_eBX();
	void method_POP_eSP();
	void method_POP_eBP();
	void method_POP_eSI();
	void method_POP_eDI();

	void method_PUSH_Iv();
	void method_IMUL_Gv_Ev_Iv();
	void method_PUSH_Ib();
	void method_IMUL_Gv_Ev_Ib();
	void method_INSB_Yb_DX();
	void method_INSW_Yv_DX();
	void method_OUTSB_DX_Xb();
	void method_OUTSW_DX_Xv();

	void method_ShortJumpCondition_JS();
	void method_ShortJumpCondition_JNS();
	void method_ShortJumpCondition_JP();
	void method_ShortJumpCondition_JNP();
	void method_ShortJumpCondition_JL();
	void method_ShortJumpCondition_JNL();
	void method_ShortJumpCondition_JLE();
	void method_ShortJumpCondition_JNLE();

	void method_MOV_Eb_Gb();
	void method_MOV_Ev_Gv();
	void method_MOV_Gb_Eb();
	void method_MOV_Gv_Ev();
	void method_MOV_Ew_Sw();
	void method_LEA_Gv_M();
	void method_MOV_Sw_Ew();
	void method_POP_Ev();

	void method_CBW();
	void method_CWD();
	void method_CALL_aP();
	void method_WAIT();
	void method_PUSHF_Fv();
	void method_POP_Fv();
	void method_SAHF();
	void method_LAHF();

	void method_TEST_AL_Ib();
	void method_TEST_eAX_Iv();
	void method_STOSB_Yb_AL();
	void method_STOSW_Yv_eAX();
	void method_LODSB_AL_Xb();
	void method_LODSW_eAX_Xv();
	void method_SCASB_AL_Yb();
	void method_SCASW_eAX_Yv();

	void method_MOV_eAX();
	void method_MOV_eCX();
	void method_MOV_eDX();
	void method_MOV_eBX();
	void method_MOV_eSP();
	void method_MOV_eBP();
	void method_MOV_eSI();
	void method_MOV_eDI();

	void method_ENTER_Iw_Ib();
	void method_LEAVE();
	void method_RET_far_Iw();
	void method_RET_far();
	void method_INT_3();
	void method_INT_Ib();
	void method_INTO();
	void method_IRET();

	void method_ESC();

	void method_CALL_Jv();
	void method_JMP_Jv();
	void method_JMP_Ap();
	void method_JMP_Jb();
	void method_IN_AL_DX();
	void method_IN_eAX_DX();
	void method_OUT_DX_AL();
	void method_OUT_DX_eAX();

	void method_CLC();
	void method_STC();
	void method_CLI();
	void method_STI();
	void method_CLD();
	void method_STD();
	void method_INC_Group4();
	void method_INC_Group5();
	
public: //GROUP组都是后绑定的，所以设为public。。便于访问
////////////////////////////////////////////<GROUP1>
	void methodGroup_ADD_Eb_Ib();
	void methodGroup_ADD_Ev_Iv();
	void methodGroup_ADD_Ev_Ib();

	void methodGroup_OR_Eb_Ib();
	void methodGroup_OR_Ev_Iv();
	void methodGroup_OR_Ev_Ib();
	
	void methodGroup_ADC_Eb_Ib();
	void methodGroup_ADC_Ev_Iv();
	void methodGroup_ADC_Ev_Ib();

	void methodGroup_SBB_Eb_Ib();
	void methodGroup_SBB_Ev_Iv();
	void methodGroup_SBB_Ev_Ib();

	void methodGroup_AND_Eb_Ib();
	void methodGroup_AND_Ev_Iv();
	void methodGroup_AND_Ev_Ib();

	void methodGroup_SUB_Eb_Ib();
	void methodGroup_SUB_Ev_Iv();
	void methodGroup_SUB_Ev_Ib();

	void methodGroup_XOR_Eb_Ib();
	void methodGroup_XOR_Ev_Iv();
	void methodGroup_XOR_Ev_Ib();

	void methodGroup_CMP_Eb_Ib();
	void methodGroup_CMP_Ev_Iv();
	void methodGroup_CMP_Ev_Ib();
////////////////////////////////////////////</GROUP1>	
////////////////////////////////////////////<GROUP2>
	void methodGroup_ROL_Eb_Ib();
	void methodGroup_ROL_Ev_Ib();
	void methodGroup_ROL_Eb_1();
	void methodGroup_ROL_Ev_1();
	void methodGroup_ROL_Eb_CL();
	void methodGroup_ROL_Ev_CL();

	void methodGroup_ROR_Eb_Ib();
	void methodGroup_ROR_Ev_Ib();
	void methodGroup_ROR_Eb_1();
	void methodGroup_ROR_Ev_1();
	void methodGroup_ROR_Eb_CL();
	void methodGroup_ROR_Ev_CL();

	void methodGroup_RCL_Eb_Ib();
	void methodGroup_RCL_Ev_Ib();
	void methodGroup_RCL_Eb_1();
	void methodGroup_RCL_Ev_1();
	void methodGroup_RCL_Eb_CL();
	void methodGroup_RCL_Ev_CL();

	void methodGroup_RCR_Eb_Ib();
	void methodGroup_RCR_Ev_Ib();
	void methodGroup_RCR_Eb_1();
	void methodGroup_RCR_Ev_1();
	void methodGroup_RCR_Eb_CL();
	void methodGroup_RCR_Ev_CL();

	void methodGroup_SHL_Eb_Ib();
	void methodGroup_SHL_Ev_Ib();
	void methodGroup_SHL_Eb_1();
	void methodGroup_SHL_Ev_1();
	void methodGroup_SHL_Eb_CL();
	void methodGroup_SHL_Ev_CL();

	void methodGroup_SHR_Eb_Ib();
	void methodGroup_SHR_Ev_Ib();
	void methodGroup_SHR_Eb_1();
	void methodGroup_SHR_Ev_1();
	void methodGroup_SHR_Eb_CL();
	void methodGroup_SHR_Ev_CL();

	void methodGroup_SAR_Eb_Ib();
	void methodGroup_SAR_Ev_Ib();
	void methodGroup_SAR_Eb_1();
	void methodGroup_SAR_Ev_1();
	void methodGroup_SAR_Eb_CL();
	void methodGroup_SAR_Ev_CL();
////////////////////////////////////////////</GROUP2>
////////////////////////////////////////////<GROUP3>
	void methodGroup_TEST_Ib();
	void methodGroup_TEST_Iv();
	void methodGroup_NOT_8bit();
	void methodGroup_NOT_16bit();
	void methodGroup_NEG_8bit();
	void methodGroup_NEG_16bit();
	void methodGroup_MUL_AL();
	void methodGroup_MUL_eAX();
	void methodGroup_IMUL_AL();
	void methodGroup_IMUL_eAX();
	void methodGroup_DIV_AL();
	void methodGroup_DIV_eAX();
	void methodGroup_IDIV_AL();
	void methodGroup_IDIV_eAX();

////////////////////////////////////////////</GROUP3>


//////////////////////////////////////////<Group4--5>
	void methodGroup_INC_Eb();
	void methodGroup_INC_Ev();
	void methodGroup_DEC_Eb();
	void methodGroup_DEC_Ev();
	void methodGroup_CALL_Ev();
	void methodGroup_CALL_Ep();
	void methodGroup_JMP_Ev();
	void methodGroup_JMP_Ep();
	void methodGroup_PUSH_Ev();
	

//////////////////////////////////////////</Group4--5>

	void selfMethod_PUSH(const t_nubit8 byteData);
	void selfMethod_PUSH(const t_nubit16 wordData);

	void chooseRepeatExeStringMethod();
	void self_REP_StringIns();
	void self_REPE_StringIns();
	void self_REPNE_StringIns();
	void self_string_affect_DI_SI(const DataTypeLength type);
	void self_string_affect_SI(const DataTypeLength type);
	void self_string_affect_DI(const DataTypeLength type);
	void method_Unknow();
	void method_outsideINT(t_nubit8 INT_num);
};

class CentreProcessorUnit {
	struct LazyTalbe {
		t_nubit32 operand1, operand2, result;
		Method opMethod;
		int  bitLength;
		LazyTalbe() {operand1 = operand2 = result = bitLength = 0;}
	};
public:
	CentreProcessorUnit():deCodeBlock(this),exeCodeBlock(this) {}
	void init();
	bool getOF_Flag();
	bool getDF_Flag();
	bool getIF_Flag();
	bool getTF_Flag();
	bool getSF_Flag();
	bool getZF_Flag();
	bool getAF_Flag();
	bool getPF_Flag();
	bool getCF_Flag();
	void setOF_Flag(const bool f);
	void setDF_Flag(const bool f);
	void setIF_Flag(const bool f);
	void setTF_Flag(const bool f);
	void setSF_Flag(const bool f);void setSF_Flag(const t_nubit8 data);void setSF_Flag(const t_nubit16 data);
	void setZF_Flag(const bool f);void setZF_Flag(const t_nubit8 data);void setZF_Flag(const t_nubit16 data);
	void setAF_Flag(const bool f);
	void setPF_Flag(const bool f);void setPF_Flag(const t_nubit8 data);void setPF_Flag(const t_nubit16 data);
	void setCF_Flag(const bool f);
	void setSF_ZF_PF_byResult(const t_nubit8 result);
	void setSF_ZF_PF_byResult(const t_nubit16 result);
	void storeCaculate(const Method opMethod, const int bitLength,  const t_nubit32 operand1, const t_nubit32 operand2, const t_nubit32 result, const t_nubit16 mask_Flag);
	t_nubit16 generateFLAG();
	void setFLAG(const t_nubit16 flags);
	void set_SF_ZF_AF_PF_CF_Flag(const t_nubit16 flags);
	DeCodeBlock deCodeBlock;
	ExecuteCodeBlock exeCodeBlock;
	LazyTalbe OF_lazyTable, SF_lazyTable, PF_lazyTable, ZF_lazyTable, CF_lazyTable, AF_lazyTable;//DF,IF,TF 不会需要这个，因为相对来说计算它们的值比存储lazytable 要快。
	bool OF_Flag, DF_Flag, IF_Flag, TF_Flag, SF_Flag, ZF_Flag, AF_Flag, PF_Flag, CF_Flag;
	bool isOFCaculated, isDFCaculated, isIFCaculated, isTFCaculated;
	bool isSFCaculated, isZFCaculated, isAFCaculated, isPFCaculated, isCFCaculated;
	t_nubit16 needCaculateFlag_mask;
	t_nubit16 flag_extern;//存储flag寄存器的14--11位的数据，主要用以解决popf，pushf
	union {
		struct {t_nubit8 al,ah;};
		t_nubit16 ax;
	};
	union {
		struct {t_nubit8 bl,bh;};
		t_nubit16 bx;
	};
	union {
		struct {t_nubit8 cl,ch;};
		t_nubit16 cx;
	};
	union {
		struct {t_nubit8 dl,dh;};
		t_nubit16 dx;
	};
	t_nubit8  iobyte;                               /* for i/o communication */
	t_nubit16 sp,bp,si,di,ip,flags;
	t_nubit16 cs,ds,es,ss;
	t_nubit16 overds, overss;
	t_bool flagnmi;
	t_bool flagterm;
	t_nubitcc icount;
};

void showMessage(string msg);

#endif