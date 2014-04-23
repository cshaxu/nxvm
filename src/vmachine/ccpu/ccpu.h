#ifndef NXVM_CCPU_H
#define NXVM_CCPU_H

#include "../vglobal.h"
#include "assert.h"
#include "../vram.h"

#define CRAM 0
#define VRAM 1

#define CPU2 0
#define CCPU_RAM VRAM

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

#define OPCODE_WIDTH      1
#define MOD_REG_RM_LENGTH 1

#define MASK_FLAG_OF (0x0800)
#define MASK_FLAG_DF (0x0400)
#define MASK_FLAG_IF (0x0200)
#define MASK_FLAG_TF (0x0100)
#define MASK_FLAG_SF (0x0080)
#define MASK_FLAG_ZF (0x0040)
#define MASK_FLAG_AF (0x0010)
#define MASK_FLAG_PF (0x0004)
#define MASK_FLAG_CF (0x0001)
#define MASK_FLAG_METHOD_ADD  (MASK_FLAG_OF | MASK_FLAG_SF | MASK_FLAG_ZF | MASK_FLAG_AF | MASK_FLAG_PF | MASK_FLAG_CF)
#define MASK_FLAG_METHOD_INC  (MASK_FLAG_OF | MASK_FLAG_SF | MASK_FLAG_ZF | MASK_FLAG_AF | MASK_FLAG_PF)//inc不会影响CF
#define MASK_FLAG_METHOD_SUB  (MASK_FLAG_METHOD_ADD)
#define MASK_FLAG_METHOD_DEC  (MASK_FLAG_METHOD_INC)
#define MASK_FLAG_METHOD_ADC  (MASK_FLAG_METHOD_ADD)
#define MASK_FLAG_METHOD_CMP  (MASK_FLAG_METHOD_ADD)
#define MASK_FLAG_METHOD_CMPS (MASK_FLAG_METHOD_ADD)
#define MASK_FLAG_METHOD_NEG  (MASK_FLAG_METHOD_ADD)
#define MASK_FLAG_METHOD_SBB  (MASK_FLAG_METHOD_ADD)
#define MASK_FLAG_METHOD_SCAS (MASK_FLAG_METHOD_ADD)
#define MASK_FLAG_METHOD_OR   (MASK_FLAG_SF | MASK_FLAG_ZF | MASK_FLAG_PF )
#define MASK_FLAG_METHOD_AND  (MASK_FLAG_METHOD_OR)
#define MASK_FLAG_METHOD_XOR  (MASK_FLAG_METHOD_OR)

typedef enum {
	dataType_U1=1, dataType_S1=1, dataType_U2=2,
	dataType_S2=2, dataType_U4=4, dataType_S4=4
} DataTypeLength;

typedef enum {
	ADD_8bit, ADD_16bit,
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
	MOD_8bit,/*为了AAM设的*/
} Method;

#define CS_SEGMENT_OVERRIDE_PREFIX  0x2E
#define SS_SEGMENT_OVERRIDE_PREFIX  0x36
#define DS_SEGMENT_OVERRIDE_PREFIX  0x3E
#define ES_SEGMENT_OVERRIDE_PREFIX  0x26
#define LOCK_PREFIX  0xf0
#define REPNE_PREFIX 0xf2
#define REP_PREFIX   0xf3//和下面这个值相同
#define REPE_PREFIX  0xf3

typedef struct {
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
	t_nubit32 SPC, TPC;
	t_nubit8 *pOpCode;
	t_bool isWidth; //当前翻译指令w位是否。。。
} DeCodeBlock;

void  decode_deCodeInstruction();
void  decode_refresh();//获取编译下条指令的必要数据
void* decode_deCodeEffectiveMemAddr(const t_nubit8 modRegRm, const void *decode_pdispData, int* pdispLength );
void* decode_getRegAddr(const t_bool w, const t_nubit8 reg);
t_nubit16* decode_getSegAddr(const t_nubit8 sreg);
void  decode_deCodeInsStuff();
void  decode_setExecuteMethod();
void  decode_deOpCode_singleDefine();//done
void  decode_deOpCode_doubleDefine();//done
int   decode_deOpCode_modRegRm();//done
void  decode_deOpCode_IMM();//done
void  decode_deOpCode_IMMGroup();//done
void  decode_deOpCode_ModRM();
void  decode_deOpCode_contactData_8bit();
void  decode_deOpCode_contactData_16bit();//done
void  decode_deOpCode_contactData_32bit();//done
void  decode_deOpCode_Mov_mem_AX();//存储器到累加器，或累加器到存储器(互传)  done
void  decode_deOpCode_Mov_imm_rm();//第七种，立即数到存储器/寄存器 done
void  decode_deOpCode_Mov_rm_seg();//第三种，从寄存器/存储器到段存储器传送（互传）done
void  decode_deOpCode_Mov_imm_reg();//第六种，立即数到寄存器
void  decode_deOpCode_modRm();
void  decode_deOpCode_TEST_Ib_Iv();
t_nubit16 decode_getDefaultSegment_DS();
t_nubit16 decode_getDefaultSegment_SS();

typedef void (*ExecuteMethod)();
typedef struct {
	ExecuteMethod exeMethodArray[0x100];
	ExecuteMethod needExeMethod;
	ExecuteMethod repeatMethod;
} ExecuteCodeBlock;

void ins_atomMethod_MOV_8bit(t_nubit32 logicAddr_des, const t_nubit8 data_src);
void ins_atomMethod_MOV_16bit(t_nubit32 logicAddr_des, const t_nubit16 data_src);
void ins_atomMethod_ADD_8bit(t_nubit8 *pdes, const t_nubit8 src);
void ins_atomMethod_ADD_16bit(t_nubit16 *pdes, const t_nubit16 src);
void ins_atomMethod_INC_8bit(t_nubit8 *pdes);
void ins_atomMethod_INC_16bit(t_nubit16 *pdes);
void ins_atomMethod_ADC_8bit(t_nubit8 *pdes, const t_nubit8 src);
void ins_atomMethod_ADC_16bit(t_nubit16 *pdes, const t_nubit16 src);
void ins_atomMethod_SUB_8bit(t_nubit8 *pdes, const t_nubit8 src);
void ins_atomMethod_SUB_16bit(t_nubit16 *pdes, const t_nubit16 src);
void ins_atomMethod_DEC_8bit(t_nubit8 *pdes);
void ins_atomMethod_DEC_16bit(t_nubit16 *pdes);
void ins_atomMethod_SBB_8bit(t_nubit8 *pdes, const t_nubit8 src);
void ins_atomMethod_SBB_16bit(t_nubit16 *pdes, const t_nubit16 src);
void ins_atomMethod_OR_8bit(t_nubit8 *pdes, const t_nubit8 src);
void ins_atomMethod_OR_16bit(t_nubit16 *pdes, const t_nubit16 src);
void ins_atomMethod_AND_8bit(t_nubit8 *pdes, const t_nubit8 src);
void ins_atomMethod_AND_16bit(t_nubit16 *pdes, const t_nubit16 src);
void ins_atomMethod_TEST_8bit(t_nubit8 *pdes, const t_nubit8 src);
void ins_atomMethod_TEST_16bit(t_nubit16 *pdes, const t_nubit16 src);
void ins_atomMethod_XOR_8bit(t_nubit8 *pdes, const t_nubit8 src);
void ins_atomMethod_XOR_16bit(t_nubit16 *pdes, const t_nubit16 src);
void ins_atomMethod_CMP_8bit(t_nubit8 des, const t_nubit8 src);
void ins_atomMethod_CMP_16bit(t_nubit16 des, const t_nubit16 src);
void ins_atomMethod_CMPSB(); 
void ins_atomMethod_CMPSW();
void ins_atomMethod_MOVSB();
void ins_atomMethod_MOVSW();
void ins_atomMethod_STOSB();
void ins_atomMethod_STOSW();
void ins_atomMethod_LODSB();
void ins_atomMethod_LODSW();
void ins_atomMethod_SCASB();
void ins_atomMethod_SCASW();
void ins_atomMethod_INS();
void ins_atomMethod_OUTS();
void ins_atomMethod_shortJMP();
void ins_atomMethod_PUSH(const t_nubit16 data);
t_nubit16 ins_atomMethod_POP();
void ins_atomMethod_INT(const t_nubit8 INT_num);
void ins_method_ADD_Eb_Gb();
void ins_method_ADD_Ev_Gv();
void ins_method_ADD_Gb_Eb();
void ins_method_ADD_Gv_Ev();
void ins_method_ADD_AL_Ib();
void ins_method_ADD_eAX_Iv();
void ins_method_PUSH_ES();
void ins_method_POP_ES();
void ins_method_ADC_Eb_Gb();
void ins_method_ADC_Ev_Gv();
void ins_method_ADC_Gb_Eb();
void ins_method_ADC_Gv_Ev();
void ins_method_ADC_AL_Ib();
void ins_method_ADC_eAX_Iv();
void ins_method_PUSH_SS();
void ins_method_POP_SS();
void ins_method_AND_Eb_Gb();
void ins_method_AND_Ev_Gv();
void ins_method_AND_Gb_Eb();
void ins_method_AND_Gv_Ev();
void ins_method_AND_AL_Ib();
void ins_method_AND_eAX_Iv();
void ins_method_SEG_ES();
void ins_method_DAA();
void ins_method_XOR_Eb_Gb();
void ins_method_XOR_Ev_Gv();
void ins_method_XOR_Gb_Eb();
void ins_method_XOR_Gv_Ev();
void ins_method_XOR_AL_Ib();
void ins_method_XOR_eAX_Iv();
void ins_method_SEG_SS();
void ins_method_AAA();
void ins_method_INC_eAX();
void ins_method_INC_eCX();
void ins_method_INC_eDX();
void ins_method_INC_eBX();
void ins_method_INC_eSP();
void ins_method_INC_eBP();
void ins_method_INC_eSI();
void ins_method_INC_eDI();
void ins_method_PUSH_eAX();
void ins_method_PUSH_eCX();
void ins_method_PUSH_eDX();
void ins_method_PUSH_eBX();
void ins_method_PUSH_eSP();
void ins_method_PUSH_eBP();
void ins_method_PUSH_eSI();
void ins_method_PUSH_eDI();
void ins_method_PUSHA_PUSHAD();
void ins_method_POPA_POPAD();
void ins_method_BOUND_Gv_Ma();
void ins_method_ARPL_Ew_Gw();
void ins_method_SEG_FS();
void ins_method_SEG_GS();
void ins_method_Operand_Size();
void ins_method_Address_Size();
void ins_method_ShortJump_JO();
void ins_method_ShortJump_JNO();
void ins_method_ShortJump_JB();
void ins_method_ShortJump_JNB();
void ins_method_ShortJump_JZ();
void ins_method_ShortJump_JNZ();
void ins_method_ShortJump_JBE();
void ins_method_ShortJump_JNBE();
void ins_method_IMMGroup_Eb_Ib();
void ins_method_IMMGroup_Ev_Iz();
void ins_method_IMMGroup_Eb_IbX();
void ins_method_IMMGroup_Ev_Ib();
void ins_method_TEST_Eb_Gb();
void ins_method_TEST_Ev_Gv();
void ins_method_XCHG_Eb_Gb();
void ins_method_XCHG_Ev_Gv();
void ins_method_NOP();
void ins_method_XCHG_eCX();
void ins_method_XCHG_eDX();
void ins_method_XCHG_eBX();
void ins_method_XCHG_eSP();
void ins_method_XCHG_eBP();
void ins_method_XCHG_eSI();
void ins_method_XCHG_eDI();
void ins_method_MOV_AL_Ob();
void ins_method_MOV_eAX_Ov();
void ins_method_MOV_Ob_AL();
void ins_method_MOV_Ov_eAX();
void ins_method_MOVSB_Xb_Yb();
void ins_method_MOVSW_Xv_Yv();
void ins_method_CMPSB_Xb_Yb();
void ins_method_CMPSW_Xv_Yv();
void ins_method_MOV_AL();
void ins_method_MOV_CL();
void ins_method_MOV_DL();
void ins_method_MOV_BL();
void ins_method_MOV_AH();
void ins_method_MOV_CH();
void ins_method_MOV_DH();
void ins_method_MOV_BH();
void ins_method_ShiftGroup_Eb_Ib();
void ins_method_ShiftGroup_Ev_Ib();
void ins_method_RET_near_Iw();
//that would be something
void ins_method_RET_near_None();
void ins_method_LES_Gv_Mp();
void ins_method_LDS_Gv_Mp();
void ins_method_MOV_Eb_Ib();
void ins_method_MOV_Ev_Iv();
void ins_method_ShiftGroup2_Eb_1();
void ins_method_ShiftGroup2_Ev_1();
void ins_method_ShiftGroup2_Eb_CL();
void ins_method_ShiftGroup2_Ev_CL();
void ins_method_AMM();
void ins_method_AAD();
void ins_method_XLAT();
void ins_method_LOOPN_Jb();
void ins_method_LOOPE_Jb();
void ins_method_LOOP_Jb();
void ins_method_JCXZ_Jb();
void ins_method_IN_AL_Ib();
void ins_method_IN_eAX_Ib();
void ins_method_OUT_Ib_AL();
void ins_method_OUT_Ib_eAX();
void ins_method_LOCK();
void ins_method_REPNE();
void ins_method_REP_REPE();
void ins_method_HLT();
void ins_method_CMC();
void ins_method_UnaryGroup_Eb();
void ins_method_UnaryGroup_Ev();
void ins_method_OR_Eb_Gb();
void ins_method_OR_Ev_Gv();
void ins_method_OR_Gb_Eb();
void ins_method_OR_Gv_Ev();
void ins_method_OR_AL_Ib();
void ins_method_OR_eAX_Iv();
void ins_method_PUSH_CS();
void ins_method_EscapeCode();
void ins_method_SBB_Eb_Gb();
void ins_method_SBB_Ev_Gv();
void ins_method_SBB_Gb_Eb();
void ins_method_SBB_Gv_Ev();
void ins_method_SBB_AL_Ib();
void ins_method_SBB_eAX_Iv();
void ins_method_PUSH_DS();
void ins_method_POP_DS();
void ins_method_SUB_Eb_Gb();
void ins_method_SUB_Ev_Gv();
void ins_method_SUB_Gb_Eb();
void ins_method_SUB_Gv_Ev();
void ins_method_SUB_AL_Ib();
void ins_method_SUB_eAX_Iv();
void ins_method_SEG_CS();
void ins_method_DAS();
void ins_method_CMP_Eb_Gb();
void ins_method_CMP_Ev_Gv();
void ins_method_CMP_Gb_Eb();
void ins_method_CMP_Gv_Ev();
void ins_method_CMP_AL_Ib();
void ins_method_CMP_eAX_Iv();
void ins_method_SEG_DS();
void ins_method_AAS();
void ins_method_DEC_eAX();
void ins_method_DEC_eCX();
void ins_method_DEC_eDX();
void ins_method_DEC_eBX();
void ins_method_DEC_eSP();
void ins_method_DEC_eBP();
void ins_method_DEC_eSI();
void ins_method_DEC_eDI();
void ins_method_POP_eAX();
void ins_method_POP_eCX();
void ins_method_POP_eDX();
void ins_method_POP_eBX();
void ins_method_POP_eSP();
void ins_method_POP_eBP();
void ins_method_POP_eSI();
void ins_method_POP_eDI();
void ins_method_PUSH_Iv();
void ins_method_IMUL_Gv_Ev_Iv();
void ins_method_PUSH_Ib();
void ins_method_IMUL_Gv_Ev_Ib();
void ins_method_INSB_Yb_DX();
void ins_method_INSW_Yv_DX();
void ins_method_OUTSB_DX_Xb();
void ins_method_OUTSW_DX_Xv();
void ins_method_ShortJumpCondition_JS();
void ins_method_ShortJumpCondition_JNS();
void ins_method_ShortJumpCondition_JP();
void ins_method_ShortJumpCondition_JNP();
void ins_method_ShortJumpCondition_JL();
void ins_method_ShortJumpCondition_JNL();
void ins_method_ShortJumpCondition_JLE();
void ins_method_ShortJumpCondition_JNLE();
void ins_method_MOV_Eb_Gb();
void ins_method_MOV_Ev_Gv();
void ins_method_MOV_Gb_Eb();
void ins_method_MOV_Gv_Ev();
void ins_method_MOV_Ew_Sw();
void ins_method_LEA_Gv_M();
void ins_method_MOV_Sw_Ew();
void ins_method_POP_Ev();
void ins_method_CBW();
void ins_method_CWD();
void ins_method_CALL_aP();
void ins_method_WAIT();
void ins_method_PUSHF_Fv();
void ins_method_POP_Fv();
void ins_method_SAHF();
void ins_method_LAHF();
void ins_method_TEST_AL_Ib();
void ins_method_TEST_eAX_Iv();
void ins_method_STOSB_Yb_AL();
void ins_method_STOSW_Yv_eAX();
void ins_method_LODSB_AL_Xb();
void ins_method_LODSW_eAX_Xv();
void ins_method_SCASB_AL_Yb();
void ins_method_SCASW_eAX_Yv();
void ins_method_MOV_eAX();
void ins_method_MOV_eCX();
void ins_method_MOV_eDX();
void ins_method_MOV_eBX();
void ins_method_MOV_eSP();
void ins_method_MOV_eBP();
void ins_method_MOV_eSI();
void ins_method_MOV_eDI();
void ins_method_ENTER_Iw_Ib();
void ins_method_LEAVE();
void ins_method_RET_far_Iw();
void ins_method_RET_far();
void ins_method_INT_3();
void ins_method_INT_Ib();
void ins_method_INTO();
void ins_method_IRET();
void ins_method_ESC();
void ins_method_CALL_Jv();
void ins_method_JMP_Jv();
void ins_method_JMP_Ap();
void ins_method_JMP_Jb();
void ins_method_IN_AL_DX();
void ins_method_IN_eAX_DX();
void ins_method_OUT_DX_AL();
void ins_method_OUT_DX_eAX();
void ins_method_CLC();
void ins_method_STC();
void ins_method_CLI();
void ins_method_STI();
void ins_method_CLD();
void ins_method_STD();
void ins_method_INC_Group4();
void ins_method_INC_Group5();
////////////////////////////////////////////<GROUP1>
void ins_methodGroup_ADD_Eb_Ib();
void ins_methodGroup_ADD_Ev_Iv();
void ins_methodGroup_ADD_Ev_Ib();
void ins_methodGroup_OR_Eb_Ib();
void ins_methodGroup_OR_Ev_Iv();
void ins_methodGroup_OR_Ev_Ib();
void ins_methodGroup_ADC_Eb_Ib();
void ins_methodGroup_ADC_Ev_Iv();
void ins_methodGroup_ADC_Ev_Ib();
void ins_methodGroup_SBB_Eb_Ib();
void ins_methodGroup_SBB_Ev_Iv();
void ins_methodGroup_SBB_Ev_Ib();
void ins_methodGroup_AND_Eb_Ib();
void ins_methodGroup_AND_Ev_Iv();
void ins_methodGroup_AND_Ev_Ib();
void ins_methodGroup_SUB_Eb_Ib();
void ins_methodGroup_SUB_Ev_Iv();
void ins_methodGroup_SUB_Ev_Ib();
void ins_methodGroup_XOR_Eb_Ib();
void ins_methodGroup_XOR_Ev_Iv();
void ins_methodGroup_XOR_Ev_Ib();
void ins_methodGroup_CMP_Eb_Ib();
void ins_methodGroup_CMP_Ev_Iv();
void ins_methodGroup_CMP_Ev_Ib();
////////////////////////////////////////////</GROUP1>	
////////////////////////////////////////////<GROUP2>
void ins_methodGroup_ROL_Eb_Ib();
void ins_methodGroup_ROL_Ev_Ib();
void ins_methodGroup_ROL_Eb_1();
void ins_methodGroup_ROL_Ev_1();
void ins_methodGroup_ROL_Eb_CL();
void ins_methodGroup_ROL_Ev_CL();
void ins_methodGroup_ROR_Eb_Ib();
void ins_methodGroup_ROR_Ev_Ib();
void ins_methodGroup_ROR_Eb_1();
void ins_methodGroup_ROR_Ev_1();
void ins_methodGroup_ROR_Eb_CL();
void ins_methodGroup_ROR_Ev_CL();
void ins_methodGroup_RCL_Eb_Ib();
void ins_methodGroup_RCL_Ev_Ib();
void ins_methodGroup_RCL_Eb_1();
void ins_methodGroup_RCL_Ev_1();
void ins_methodGroup_RCL_Eb_CL();
void ins_methodGroup_RCL_Ev_CL();
void ins_methodGroup_RCR_Eb_Ib();
void ins_methodGroup_RCR_Ev_Ib();
void ins_methodGroup_RCR_Eb_1();
void ins_methodGroup_RCR_Ev_1();
void ins_methodGroup_RCR_Eb_CL();
void ins_methodGroup_RCR_Ev_CL();
void ins_methodGroup_SHL_Eb_Ib();
void ins_methodGroup_SHL_Ev_Ib();
void ins_methodGroup_SHL_Eb_1();
void ins_methodGroup_SHL_Ev_1();
void ins_methodGroup_SHL_Eb_CL();
void ins_methodGroup_SHL_Ev_CL();
void ins_methodGroup_SHR_Eb_Ib();
void ins_methodGroup_SHR_Ev_Ib();
void ins_methodGroup_SHR_Eb_1();
void ins_methodGroup_SHR_Ev_1();
void ins_methodGroup_SHR_Eb_CL();
void ins_methodGroup_SHR_Ev_CL();
void ins_methodGroup_SAR_Eb_Ib();
void ins_methodGroup_SAR_Ev_Ib();
void ins_methodGroup_SAR_Eb_1();
void ins_methodGroup_SAR_Ev_1();
void ins_methodGroup_SAR_Eb_CL();
void ins_methodGroup_SAR_Ev_CL();
////////////////////////////////////////////</GROUP2>
////////////////////////////////////////////<GROUP3>
void ins_methodGroup_TEST_Ib();
void ins_methodGroup_TEST_Iv();
void ins_methodGroup_NOT_8bit();
void ins_methodGroup_NOT_16bit();
void ins_methodGroup_NEG_8bit();
void ins_methodGroup_NEG_16bit();
void ins_methodGroup_MUL_AL();
void ins_methodGroup_MUL_eAX();
void ins_methodGroup_IMUL_AL();
void ins_methodGroup_IMUL_eAX();
void ins_methodGroup_DIV_AL();
void ins_methodGroup_DIV_eAX();
void ins_methodGroup_IDIV_AL();
void ins_methodGroup_IDIV_eAX();
////////////////////////////////////////////</GROUP3>
//////////////////////////////////////////<Group4--5>
void ins_methodGroup_INC_Eb();
void ins_methodGroup_INC_Ev();
void ins_methodGroup_DEC_Eb();
void ins_methodGroup_DEC_Ev();
void ins_methodGroup_CALL_Ev();
void ins_methodGroup_CALL_Ep();
void ins_methodGroup_JMP_Ev();
void ins_methodGroup_JMP_Ep();
void ins_methodGroup_PUSH_Ev();
//////////////////////////////////////////</Group4--5>
void ins_chooseRepeatExeStringMethod();
void ins_self_REP_StringIns();
void ins_self_REPE_StringIns();
void ins_self_REPNE_StringIns();
void ins_self_string_affect_DI_SI(const DataTypeLength type);
void ins_self_string_affect_SI(const DataTypeLength type);
void ins_self_string_affect_DI(const DataTypeLength type);
void ins_method_Unknow();

typedef struct {
	t_nubit32 operand1, operand2, result;
	Method opMethod;
	int  bitLength;
} LazyTable;

typedef struct {
	DeCodeBlock deCodeBlock;
	ExecuteCodeBlock exeCodeBlock;
	LazyTable OF_lazyTable, SF_lazyTable, PF_lazyTable, ZF_lazyTable, CF_lazyTable, AF_lazyTable;
		//DF,IF,TF 不会需要这个，因为相对来说计算它们的值比存储lazytable 要快。
	t_bool OF_Flag, DF_Flag, IF_Flag, TF_Flag, SF_Flag, ZF_Flag, AF_Flag, PF_Flag, CF_Flag;
	t_bool isOFCaculated, isDFCaculated, isIFCaculated, isTFCaculated;
	t_bool isSFCaculated, isZFCaculated, isAFCaculated, isPFCaculated, isCFCaculated;
	t_nubit16 needCaculateFlag_mask;
	t_nubit16 flag_extern;	//存储flag寄存器的14--11位的数据，主要用以解决popf，pushf
	union {struct {t_nubit8 al,ah;};t_nubit16 ax;};
	union {struct {t_nubit8 bl,bh;};t_nubit16 bx;};
	union {struct {t_nubit8 cl,ch;};t_nubit16 cx;};
	union {struct {t_nubit8 dl,dh;};t_nubit16 dx;};
	t_nubit8  iobyte;
	t_nubit16 sp,bp,si,di,ip,flags,cs,ds,es,ss,overds, overss;
	t_bool flagnmi, flagignore;
	t_nubitcc icount;
} CentreProcessorUnit;

extern CentreProcessorUnit ccpu;

void ccpu_init();
void ccpu_final();
t_bool ccpu_getOF_Flag();
t_bool ccpu_getDF_Flag();
t_bool ccpu_getIF_Flag();
t_bool ccpu_getTF_Flag();
t_bool ccpu_getSF_Flag();
t_bool ccpu_getZF_Flag();
t_bool ccpu_getAF_Flag();
t_bool ccpu_getPF_Flag();
t_bool ccpu_getCF_Flag();
void ccpu_setOF_Flag_flag(const t_bool f);
void ccpu_setDF_Flag_flag(const t_bool f);
void ccpu_setIF_Flag_flag(const t_bool f);
void ccpu_setTF_Flag_flag(const t_bool f);
void ccpu_setSF_Flag_flag(const t_bool f);
void ccpu_setSF_Flag_data8(const t_nubit8 data);
void ccpu_setSF_Flag_data16(const t_nubit16 data);
void ccpu_setZF_Flag_flag(const t_bool f);
void ccpu_setZF_Flag_data8(const t_nubit8 data);
void ccpu_setZF_Flag_data16(const t_nubit16 data);
void ccpu_setAF_Flag_flag(const t_bool f);
void ccpu_setPF_Flag_flag(const t_bool f);
void ccpu_setPF_Flag_data8(const t_nubit8 data);
void ccpu_setPF_Flag_data16(const t_nubit16 data);
void ccpu_setCF_Flag_flag(const t_bool f);
void ccpu_setSF_ZF_PF_byResult_data8(const t_nubit8 result);
void ccpu_setSF_ZF_PF_byResult_data16(const t_nubit16 result);
void ccpu_storeCaculate(const Method opMethod, const int bitLength,  const t_nubit32 operand1, const t_nubit32 operand2, const t_nubit32 result, const t_nubit16 mask_Flag);
t_nubit16 ccpu_generateFLAG();
void ccpu_setFLAG(const t_nubit16 flags);
void ccpu_set_SF_ZF_AF_PF_CF_Flag(const t_nubit16 flags);

#if CCPU_RAM == CRAM
extern t_ram cram;
#else
#define cram vram
#endif

#define cramIsAddrInMem(addr) \
	(((addr) >= cram.base) && ((addr) < (cram.base + cram.size)))
#define cramGetAddr(segment, offset)  (cram.base + \
	((((segment) << 4) + (offset)) % cram.size))
#define cramVarByte(segment, offset)  (d_nubit8(cramGetAddr(segment, offset)))
#define cramVarWord(segment, offset)  (d_nubit16(cramGetAddr(segment, offset)))
#define cramVarDWord(segment, offset) (d_nubit32(cramGetAddr(segment, offset)))

#define im(n) cramIsAddrInMem((t_vaddrcc)(n))
#define getMemData_byte(addr) cramVarByte(0,(addr))
#define getMemData_word(addr) cramVarWord(0,(addr))
#define writeMem_byte(addr, data) (cramVarByte(0,(addr)) = (data))
#define writeMem_word(addr, data) (cramVarWord(0,(addr)) = (data))

#endif