#include "stdlib.h"
#include "memory.h"

#include "ccpuapi.h"
#include "../vapi.h"

#if CCPU_RAM == CRAM
t_ram cram;
#endif

CentreProcessorUnit ccpu;

void ccpu_init()
{
	memset(&ccpu, 0x00, sizeof(CentreProcessorUnit));
#if CCPU_RAM == CRAM
	cram.size = vram.size;
	cram.base = (t_vaddrcc)malloc(cram.size);
	memcpy((void *)cram.base, (void *)vram.base, vram.size);
#endif
	ccpu.exeCodeBlock.exeMethodArray[0x00]=&ins_method_ADD_Eb_Gb;
	ccpu.exeCodeBlock.exeMethodArray[0x01]=&ins_method_ADD_Ev_Gv;
	ccpu.exeCodeBlock.exeMethodArray[0x02]=&ins_method_ADD_Gb_Eb;
	ccpu.exeCodeBlock.exeMethodArray[0x03]=&ins_method_ADD_Gv_Ev;
	ccpu.exeCodeBlock.exeMethodArray[0x04]=&ins_method_ADD_AL_Ib;
	ccpu.exeCodeBlock.exeMethodArray[0x05]=&ins_method_ADD_eAX_Iv;
	ccpu.exeCodeBlock.exeMethodArray[0x06]=&ins_method_PUSH_ES;
	ccpu.exeCodeBlock.exeMethodArray[0x07]=&ins_method_POP_ES;
	ccpu.exeCodeBlock.exeMethodArray[0x10]=&ins_method_ADC_Eb_Gb;
	ccpu.exeCodeBlock.exeMethodArray[0x11]=&ins_method_ADC_Ev_Gv;
	ccpu.exeCodeBlock.exeMethodArray[0x12]=&ins_method_ADC_Gb_Eb;
	ccpu.exeCodeBlock.exeMethodArray[0x13]=&ins_method_ADC_Gv_Ev;
	ccpu.exeCodeBlock.exeMethodArray[0x14]=&ins_method_ADC_AL_Ib;
	ccpu.exeCodeBlock.exeMethodArray[0x15]=&ins_method_ADC_eAX_Iv;
	ccpu.exeCodeBlock.exeMethodArray[0x16]=&ins_method_PUSH_SS;
	ccpu.exeCodeBlock.exeMethodArray[0x17]=&ins_method_POP_SS;
	ccpu.exeCodeBlock.exeMethodArray[0x20]=&ins_method_AND_Eb_Gb;
	ccpu.exeCodeBlock.exeMethodArray[0x21]=&ins_method_AND_Ev_Gv;
	ccpu.exeCodeBlock.exeMethodArray[0x22]=&ins_method_AND_Gb_Eb;
	ccpu.exeCodeBlock.exeMethodArray[0x23]=&ins_method_AND_Gv_Ev;
	ccpu.exeCodeBlock.exeMethodArray[0x24]=&ins_method_AND_AL_Ib;
	ccpu.exeCodeBlock.exeMethodArray[0x25]=&ins_method_AND_eAX_Iv;
	ccpu.exeCodeBlock.exeMethodArray[0x26]=&ins_method_SEG_ES;
	ccpu.exeCodeBlock.exeMethodArray[0x27]=&ins_method_DAA;
	ccpu.exeCodeBlock.exeMethodArray[0x30]=&ins_method_XOR_Eb_Gb;
	ccpu.exeCodeBlock.exeMethodArray[0x31]=&ins_method_XOR_Ev_Gv;
	ccpu.exeCodeBlock.exeMethodArray[0x32]=&ins_method_XOR_Gb_Eb;
	ccpu.exeCodeBlock.exeMethodArray[0x33]=&ins_method_XOR_Gv_Ev;
	ccpu.exeCodeBlock.exeMethodArray[0x34]=&ins_method_XOR_AL_Ib;
	ccpu.exeCodeBlock.exeMethodArray[0x35]=&ins_method_XOR_eAX_Iv;
	ccpu.exeCodeBlock.exeMethodArray[0x36]=&ins_method_SEG_SS;
	ccpu.exeCodeBlock.exeMethodArray[0x37]=&ins_method_AAA;
	ccpu.exeCodeBlock.exeMethodArray[0x40]=&ins_method_INC_eAX;
	ccpu.exeCodeBlock.exeMethodArray[0x41]=&ins_method_INC_eCX;
	ccpu.exeCodeBlock.exeMethodArray[0x42]=&ins_method_INC_eDX;
	ccpu.exeCodeBlock.exeMethodArray[0x43]=&ins_method_INC_eBX;
	ccpu.exeCodeBlock.exeMethodArray[0x44]=&ins_method_INC_eSP;
	ccpu.exeCodeBlock.exeMethodArray[0x45]=&ins_method_INC_eBP;
	ccpu.exeCodeBlock.exeMethodArray[0x46]=&ins_method_INC_eSI;
	ccpu.exeCodeBlock.exeMethodArray[0x47]=&ins_method_INC_eDI;
	ccpu.exeCodeBlock.exeMethodArray[0x50]=&ins_method_PUSH_eAX;
	ccpu.exeCodeBlock.exeMethodArray[0x51]=&ins_method_PUSH_eCX;
	ccpu.exeCodeBlock.exeMethodArray[0x52]=&ins_method_PUSH_eDX;
	ccpu.exeCodeBlock.exeMethodArray[0x53]=&ins_method_PUSH_eBX;
	ccpu.exeCodeBlock.exeMethodArray[0x54]=&ins_method_PUSH_eSP;
	ccpu.exeCodeBlock.exeMethodArray[0x55]=&ins_method_PUSH_eBP;
	ccpu.exeCodeBlock.exeMethodArray[0x56]=&ins_method_PUSH_eSI;
	ccpu.exeCodeBlock.exeMethodArray[0x57]=&ins_method_PUSH_eDI;
	ccpu.exeCodeBlock.exeMethodArray[0x60]=&ins_method_PUSHA_PUSHAD;
	ccpu.exeCodeBlock.exeMethodArray[0x61]=&ins_method_POPA_POPAD;
	ccpu.exeCodeBlock.exeMethodArray[0x62]=&ins_method_BOUND_Gv_Ma;
	ccpu.exeCodeBlock.exeMethodArray[0x63]=&ins_method_ARPL_Ew_Gw;
	ccpu.exeCodeBlock.exeMethodArray[0x64]=&ins_method_SEG_FS;
	ccpu.exeCodeBlock.exeMethodArray[0x65]=&ins_method_SEG_GS;
	ccpu.exeCodeBlock.exeMethodArray[0x66]=&ins_method_Operand_Size;
	ccpu.exeCodeBlock.exeMethodArray[0x67]=&ins_method_Address_Size;
	ccpu.exeCodeBlock.exeMethodArray[0x70]=&ins_method_ShortJump_JO;
	ccpu.exeCodeBlock.exeMethodArray[0x71]=&ins_method_ShortJump_JNO;
	ccpu.exeCodeBlock.exeMethodArray[0x72]=&ins_method_ShortJump_JB;
	ccpu.exeCodeBlock.exeMethodArray[0x73]=&ins_method_ShortJump_JNB;
	ccpu.exeCodeBlock.exeMethodArray[0x74]=&ins_method_ShortJump_JZ;
	ccpu.exeCodeBlock.exeMethodArray[0x75]=&ins_method_ShortJump_JNZ;
	ccpu.exeCodeBlock.exeMethodArray[0x76]=&ins_method_ShortJump_JBE;
	ccpu.exeCodeBlock.exeMethodArray[0x77]=&ins_method_ShortJump_JNBE;
	ccpu.exeCodeBlock.exeMethodArray[0x80]=&ins_method_IMMGroup_Eb_Ib;
	ccpu.exeCodeBlock.exeMethodArray[0x81]=&ins_method_IMMGroup_Ev_Iz;
	ccpu.exeCodeBlock.exeMethodArray[0x82]=&ins_method_IMMGroup_Eb_IbX;
	ccpu.exeCodeBlock.exeMethodArray[0x83]=&ins_method_IMMGroup_Ev_Ib;
	ccpu.exeCodeBlock.exeMethodArray[0x84]=&ins_method_TEST_Eb_Gb;
	ccpu.exeCodeBlock.exeMethodArray[0x85]=&ins_method_TEST_Ev_Gv;
	ccpu.exeCodeBlock.exeMethodArray[0x86]=&ins_method_XCHG_Eb_Gb;
	ccpu.exeCodeBlock.exeMethodArray[0x87]=&ins_method_XCHG_Ev_Gv;
	ccpu.exeCodeBlock.exeMethodArray[0x90]=&ins_method_NOP;
	ccpu.exeCodeBlock.exeMethodArray[0x91]=&ins_method_XCHG_eCX;
	ccpu.exeCodeBlock.exeMethodArray[0x92]=&ins_method_XCHG_eDX;
	ccpu.exeCodeBlock.exeMethodArray[0x93]=&ins_method_XCHG_eBX;
	ccpu.exeCodeBlock.exeMethodArray[0x94]=&ins_method_XCHG_eSP;
	ccpu.exeCodeBlock.exeMethodArray[0x95]=&ins_method_XCHG_eBP;
	ccpu.exeCodeBlock.exeMethodArray[0x96]=&ins_method_XCHG_eSI;
	ccpu.exeCodeBlock.exeMethodArray[0x97]=&ins_method_XCHG_eDI;
	ccpu.exeCodeBlock.exeMethodArray[0xA0]=&ins_method_MOV_AL_Ob;
	ccpu.exeCodeBlock.exeMethodArray[0xA1]=&ins_method_MOV_eAX_Ov;
	ccpu.exeCodeBlock.exeMethodArray[0xA2]=&ins_method_MOV_Ob_AL;
	ccpu.exeCodeBlock.exeMethodArray[0xA3]=&ins_method_MOV_Ov_eAX;
	ccpu.exeCodeBlock.exeMethodArray[0xA4]=&ins_method_MOVSB_Xb_Yb;
	ccpu.exeCodeBlock.exeMethodArray[0xA5]=&ins_method_MOVSW_Xv_Yv;
	ccpu.exeCodeBlock.exeMethodArray[0xA6]=&ins_method_CMPSB_Xb_Yb;
	ccpu.exeCodeBlock.exeMethodArray[0xA7]=&ins_method_CMPSW_Xv_Yv;
	ccpu.exeCodeBlock.exeMethodArray[0xB0]=&ins_method_MOV_AL;
	ccpu.exeCodeBlock.exeMethodArray[0xB1]=&ins_method_MOV_CL;
	ccpu.exeCodeBlock.exeMethodArray[0xB2]=&ins_method_MOV_DL;
	ccpu.exeCodeBlock.exeMethodArray[0xB3]=&ins_method_MOV_BL;
	ccpu.exeCodeBlock.exeMethodArray[0xB4]=&ins_method_MOV_AH;
	ccpu.exeCodeBlock.exeMethodArray[0xB5]=&ins_method_MOV_CH;
	ccpu.exeCodeBlock.exeMethodArray[0xB6]=&ins_method_MOV_DH;
	ccpu.exeCodeBlock.exeMethodArray[0xB7]=&ins_method_MOV_BH;
	ccpu.exeCodeBlock.exeMethodArray[0xC0]=&ins_method_ShiftGroup_Eb_Ib;
	ccpu.exeCodeBlock.exeMethodArray[0xC1]=&ins_method_ShiftGroup_Ev_Ib;
	ccpu.exeCodeBlock.exeMethodArray[0xC2]=&ins_method_RET_near_Iw;
	//that would be something
	ccpu.exeCodeBlock.exeMethodArray[0xC3]=&ins_method_RET_near_None;
	ccpu.exeCodeBlock.exeMethodArray[0xC4]=&ins_method_LES_Gv_Mp;
	ccpu.exeCodeBlock.exeMethodArray[0xC5]=&ins_method_LDS_Gv_Mp;
	ccpu.exeCodeBlock.exeMethodArray[0xC6]=&ins_method_MOV_Eb_Ib;
	ccpu.exeCodeBlock.exeMethodArray[0xC7]=&ins_method_MOV_Ev_Iv;
	ccpu.exeCodeBlock.exeMethodArray[0xD0]=&ins_method_ShiftGroup2_Eb_1;
	ccpu.exeCodeBlock.exeMethodArray[0xD1]=&ins_method_ShiftGroup2_Ev_1;
	ccpu.exeCodeBlock.exeMethodArray[0xD2]=&ins_method_ShiftGroup2_Eb_CL;
	ccpu.exeCodeBlock.exeMethodArray[0xD3]=&ins_method_ShiftGroup2_Ev_CL;
	ccpu.exeCodeBlock.exeMethodArray[0xD4]=&ins_method_AMM;
	ccpu.exeCodeBlock.exeMethodArray[0xD5]=&ins_method_AAD;
  //FOUCE ON IT
	ccpu.exeCodeBlock.exeMethodArray[0xD7]=&ins_method_XLAT;
	ccpu.exeCodeBlock.exeMethodArray[0xE0]=&ins_method_LOOPN_Jb;
	ccpu.exeCodeBlock.exeMethodArray[0xE1]=&ins_method_LOOPE_Jb;
	ccpu.exeCodeBlock.exeMethodArray[0xE2]=&ins_method_LOOP_Jb;
	ccpu.exeCodeBlock.exeMethodArray[0xE3]=&ins_method_JCXZ_Jb;
	ccpu.exeCodeBlock.exeMethodArray[0xE4]=&ins_method_IN_AL_Ib;
	ccpu.exeCodeBlock.exeMethodArray[0xE5]=&ins_method_IN_eAX_Ib;
	ccpu.exeCodeBlock.exeMethodArray[0xE6]=&ins_method_OUT_Ib_AL;
	ccpu.exeCodeBlock.exeMethodArray[0xE7]=&ins_method_OUT_Ib_eAX;
	ccpu.exeCodeBlock.exeMethodArray[0xF0]=&ins_method_LOCK;
	//SOME THING...
	ccpu.exeCodeBlock.exeMethodArray[0xF1]=&ins_method_Unknow;
	ccpu.exeCodeBlock.exeMethodArray[0xF2]=&ins_method_REPNE;
	ccpu.exeCodeBlock.exeMethodArray[0xF3]=&ins_method_REP_REPE;
	ccpu.exeCodeBlock.exeMethodArray[0xF4]=&ins_method_HLT;
	ccpu.exeCodeBlock.exeMethodArray[0xF5]=&ins_method_CMC;
	ccpu.exeCodeBlock.exeMethodArray[0xF6]=&ins_method_UnaryGroup_Eb;
	ccpu.exeCodeBlock.exeMethodArray[0xF7]=&ins_method_UnaryGroup_Ev;
	ccpu.exeCodeBlock.exeMethodArray[0x08]=&ins_method_OR_Eb_Gb;
	ccpu.exeCodeBlock.exeMethodArray[0x09]=&ins_method_OR_Ev_Gv;
	ccpu.exeCodeBlock.exeMethodArray[0x0A]=&ins_method_OR_Gb_Eb;
	ccpu.exeCodeBlock.exeMethodArray[0x0B]=&ins_method_OR_Gv_Ev;
	ccpu.exeCodeBlock.exeMethodArray[0x0C]=&ins_method_OR_AL_Ib;
	ccpu.exeCodeBlock.exeMethodArray[0x0D]=&ins_method_OR_eAX_Iv;
	ccpu.exeCodeBlock.exeMethodArray[0x0E]=&ins_method_PUSH_CS;
	ccpu.exeCodeBlock.exeMethodArray[0x0F]=&ins_method_EscapeCode;
	ccpu.exeCodeBlock.exeMethodArray[0x18]=&ins_method_SBB_Eb_Gb;
	ccpu.exeCodeBlock.exeMethodArray[0x19]=&ins_method_SBB_Ev_Gv;
	ccpu.exeCodeBlock.exeMethodArray[0x1A]=&ins_method_SBB_Gb_Eb;
	ccpu.exeCodeBlock.exeMethodArray[0x1B]=&ins_method_SBB_Gv_Ev;
	ccpu.exeCodeBlock.exeMethodArray[0x1C]=&ins_method_SBB_AL_Ib;
	ccpu.exeCodeBlock.exeMethodArray[0x1D]=&ins_method_SBB_eAX_Iv;
	ccpu.exeCodeBlock.exeMethodArray[0x1E]=&ins_method_PUSH_DS;
	ccpu.exeCodeBlock.exeMethodArray[0x1F]=&ins_method_POP_DS;
	ccpu.exeCodeBlock.exeMethodArray[0x28]=&ins_method_SUB_Eb_Gb;
	ccpu.exeCodeBlock.exeMethodArray[0x29]=&ins_method_SUB_Ev_Gv;
	ccpu.exeCodeBlock.exeMethodArray[0x2A]=&ins_method_SUB_Gb_Eb;
	ccpu.exeCodeBlock.exeMethodArray[0x2B]=&ins_method_SUB_Gv_Ev;
	ccpu.exeCodeBlock.exeMethodArray[0x2C]=&ins_method_SUB_AL_Ib;
	ccpu.exeCodeBlock.exeMethodArray[0x2D]=&ins_method_SUB_eAX_Iv;
	ccpu.exeCodeBlock.exeMethodArray[0x2E]=&ins_method_SEG_CS;
	ccpu.exeCodeBlock.exeMethodArray[0x2F]=&ins_method_DAS;
	ccpu.exeCodeBlock.exeMethodArray[0x38]=&ins_method_CMP_Eb_Gb;
	ccpu.exeCodeBlock.exeMethodArray[0x39]=&ins_method_CMP_Ev_Gv;
	ccpu.exeCodeBlock.exeMethodArray[0x3A]=&ins_method_CMP_Gb_Eb;
	ccpu.exeCodeBlock.exeMethodArray[0x3B]=&ins_method_CMP_Gv_Ev;
	ccpu.exeCodeBlock.exeMethodArray[0x3C]=&ins_method_CMP_AL_Ib;
	ccpu.exeCodeBlock.exeMethodArray[0x3D]=&ins_method_CMP_eAX_Iv;
	ccpu.exeCodeBlock.exeMethodArray[0x3E]=&ins_method_SEG_DS;
	ccpu.exeCodeBlock.exeMethodArray[0x3F]=&ins_method_AAS;
	ccpu.exeCodeBlock.exeMethodArray[0x48]=&ins_method_DEC_eAX;
	ccpu.exeCodeBlock.exeMethodArray[0x49]=&ins_method_DEC_eCX;
	ccpu.exeCodeBlock.exeMethodArray[0x4A]=&ins_method_DEC_eDX;
	ccpu.exeCodeBlock.exeMethodArray[0x4B]=&ins_method_DEC_eBX;
	ccpu.exeCodeBlock.exeMethodArray[0x4C]=&ins_method_DEC_eSP;
	ccpu.exeCodeBlock.exeMethodArray[0x4D]=&ins_method_DEC_eBP;
	ccpu.exeCodeBlock.exeMethodArray[0x4E]=&ins_method_DEC_eSI;
	ccpu.exeCodeBlock.exeMethodArray[0x4F]=&ins_method_DEC_eDI;
	ccpu.exeCodeBlock.exeMethodArray[0x58]=&ins_method_POP_eAX;
	ccpu.exeCodeBlock.exeMethodArray[0x59]=&ins_method_POP_eCX;
	ccpu.exeCodeBlock.exeMethodArray[0x5A]=&ins_method_POP_eDX;
	ccpu.exeCodeBlock.exeMethodArray[0x5B]=&ins_method_POP_eBX;
	ccpu.exeCodeBlock.exeMethodArray[0x5C]=&ins_method_POP_eSP;
	ccpu.exeCodeBlock.exeMethodArray[0x5D]=&ins_method_POP_eBP;
	ccpu.exeCodeBlock.exeMethodArray[0x5E]=&ins_method_POP_eSI;
	ccpu.exeCodeBlock.exeMethodArray[0x5F]=&ins_method_POP_eDI;
	ccpu.exeCodeBlock.exeMethodArray[0x68]=&ins_method_PUSH_Iv;
	ccpu.exeCodeBlock.exeMethodArray[0x69]=&ins_method_IMUL_Gv_Ev_Iv;
	ccpu.exeCodeBlock.exeMethodArray[0x6A]=&ins_method_PUSH_Ib;
	ccpu.exeCodeBlock.exeMethodArray[0x6B]=&ins_method_IMUL_Gv_Ev_Ib;
	ccpu.exeCodeBlock.exeMethodArray[0x6C]=&ins_method_INSB_Yb_DX;
	ccpu.exeCodeBlock.exeMethodArray[0x6D]=&ins_method_INSW_Yv_DX;
	ccpu.exeCodeBlock.exeMethodArray[0x6E]=&ins_method_OUTSB_DX_Xb;
	ccpu.exeCodeBlock.exeMethodArray[0x6F]=&ins_method_OUTSW_DX_Xv;
	ccpu.exeCodeBlock.exeMethodArray[0x78]=&ins_method_ShortJumpCondition_JS;
	ccpu.exeCodeBlock.exeMethodArray[0x79]=&ins_method_ShortJumpCondition_JNS;
	ccpu.exeCodeBlock.exeMethodArray[0x7A]=&ins_method_ShortJumpCondition_JP;
	ccpu.exeCodeBlock.exeMethodArray[0x7B]=&ins_method_ShortJumpCondition_JNP;
	ccpu.exeCodeBlock.exeMethodArray[0x7C]=&ins_method_ShortJumpCondition_JL;
	ccpu.exeCodeBlock.exeMethodArray[0x7D]=&ins_method_ShortJumpCondition_JNL;
	ccpu.exeCodeBlock.exeMethodArray[0x7E]=&ins_method_ShortJumpCondition_JLE;
	ccpu.exeCodeBlock.exeMethodArray[0x7F]=&ins_method_ShortJumpCondition_JNLE;
	ccpu.exeCodeBlock.exeMethodArray[0x88]=&ins_method_MOV_Eb_Gb;
	ccpu.exeCodeBlock.exeMethodArray[0x89]=&ins_method_MOV_Ev_Gv;
	ccpu.exeCodeBlock.exeMethodArray[0x8A]=&ins_method_MOV_Gb_Eb;
	ccpu.exeCodeBlock.exeMethodArray[0x8B]=&ins_method_MOV_Gv_Ev;
	ccpu.exeCodeBlock.exeMethodArray[0x8C]=&ins_method_MOV_Ew_Sw;
	ccpu.exeCodeBlock.exeMethodArray[0x8D]=&ins_method_LEA_Gv_M;
	ccpu.exeCodeBlock.exeMethodArray[0x8E]=&ins_method_MOV_Sw_Ew;
	ccpu.exeCodeBlock.exeMethodArray[0x8F]=&ins_method_POP_Ev;
	ccpu.exeCodeBlock.exeMethodArray[0x98]=&ins_method_CBW;
	ccpu.exeCodeBlock.exeMethodArray[0x99]=&ins_method_CWD;
	ccpu.exeCodeBlock.exeMethodArray[0x9A]=&ins_method_CALL_aP;
	ccpu.exeCodeBlock.exeMethodArray[0x9B]=&ins_method_WAIT;
	ccpu.exeCodeBlock.exeMethodArray[0x9C]=&ins_method_PUSHF_Fv;
	ccpu.exeCodeBlock.exeMethodArray[0x9D]=&ins_method_POP_Fv;
	ccpu.exeCodeBlock.exeMethodArray[0x9E]=&ins_method_SAHF;
	ccpu.exeCodeBlock.exeMethodArray[0x9F]=&ins_method_LAHF;
	ccpu.exeCodeBlock.exeMethodArray[0xA8]=&ins_method_TEST_AL_Ib;
	ccpu.exeCodeBlock.exeMethodArray[0xA9]=&ins_method_TEST_eAX_Iv;
	ccpu.exeCodeBlock.exeMethodArray[0xAA]=&ins_method_STOSB_Yb_AL;
	ccpu.exeCodeBlock.exeMethodArray[0xAB]=&ins_method_STOSW_Yv_eAX;
	ccpu.exeCodeBlock.exeMethodArray[0xAC]=&ins_method_LODSB_AL_Xb;
	ccpu.exeCodeBlock.exeMethodArray[0xAD]=&ins_method_LODSW_eAX_Xv;
	ccpu.exeCodeBlock.exeMethodArray[0xAE]=&ins_method_SCASB_AL_Yb;
	ccpu.exeCodeBlock.exeMethodArray[0xAF]=&ins_method_SCASW_eAX_Yv;
	ccpu.exeCodeBlock.exeMethodArray[0xB8]=&ins_method_MOV_eAX;
	ccpu.exeCodeBlock.exeMethodArray[0xB9]=&ins_method_MOV_eCX;
	ccpu.exeCodeBlock.exeMethodArray[0xBA]=&ins_method_MOV_eDX;
	ccpu.exeCodeBlock.exeMethodArray[0xBB]=&ins_method_MOV_eBX;
	ccpu.exeCodeBlock.exeMethodArray[0xBC]=&ins_method_MOV_eSP;
	ccpu.exeCodeBlock.exeMethodArray[0xBD]=&ins_method_MOV_eBP;
	ccpu.exeCodeBlock.exeMethodArray[0xBE]=&ins_method_MOV_eSI;
	ccpu.exeCodeBlock.exeMethodArray[0xBF]=&ins_method_MOV_eDI;
	ccpu.exeCodeBlock.exeMethodArray[0xC8]=&ins_method_ENTER_Iw_Ib;
	ccpu.exeCodeBlock.exeMethodArray[0xC9]=&ins_method_LEAVE;
	ccpu.exeCodeBlock.exeMethodArray[0xCA]=&ins_method_RET_far_Iw;
	ccpu.exeCodeBlock.exeMethodArray[0xCB]=&ins_method_RET_far;
	ccpu.exeCodeBlock.exeMethodArray[0xCC]=&ins_method_INT_3;
	ccpu.exeCodeBlock.exeMethodArray[0xCD]=&ins_method_INT_Ib;
	ccpu.exeCodeBlock.exeMethodArray[0xCE]=&ins_method_INTO;
	ccpu.exeCodeBlock.exeMethodArray[0xCF]=&ins_method_IRET;
	ccpu.exeCodeBlock.exeMethodArray[0xD8]=&ins_method_ESC;
	ccpu.exeCodeBlock.exeMethodArray[0xD9]=&ins_method_ESC;
	ccpu.exeCodeBlock.exeMethodArray[0xDA]=&ins_method_ESC;
	ccpu.exeCodeBlock.exeMethodArray[0xDB]=&ins_method_ESC;
	ccpu.exeCodeBlock.exeMethodArray[0xDC]=&ins_method_ESC;
	ccpu.exeCodeBlock.exeMethodArray[0xDD]=&ins_method_ESC;
	ccpu.exeCodeBlock.exeMethodArray[0xDE]=&ins_method_ESC;
	ccpu.exeCodeBlock.exeMethodArray[0xDF]=&ins_method_ESC;
	ccpu.exeCodeBlock.exeMethodArray[0xE8]=&ins_method_CALL_Jv;
	ccpu.exeCodeBlock.exeMethodArray[0xE9]=&ins_method_JMP_Jv;
	ccpu.exeCodeBlock.exeMethodArray[0xEA]=&ins_method_JMP_Ap;
	ccpu.exeCodeBlock.exeMethodArray[0xEB]=&ins_method_JMP_Jb;
	ccpu.exeCodeBlock.exeMethodArray[0xEC]=&ins_method_IN_AL_DX;
	ccpu.exeCodeBlock.exeMethodArray[0xED]=&ins_method_IN_eAX_DX;
	ccpu.exeCodeBlock.exeMethodArray[0xEE]=&ins_method_OUT_DX_AL;
	ccpu.exeCodeBlock.exeMethodArray[0xEF]=&ins_method_OUT_DX_eAX;
	ccpu.exeCodeBlock.exeMethodArray[0xF8]=&ins_method_CLC;
	ccpu.exeCodeBlock.exeMethodArray[0xF9]=&ins_method_STC;
	ccpu.exeCodeBlock.exeMethodArray[0xFA]=&ins_method_CLI;
	ccpu.exeCodeBlock.exeMethodArray[0xFB]=&ins_method_STI;
	ccpu.exeCodeBlock.exeMethodArray[0xFC]=&ins_method_CLD;
	ccpu.exeCodeBlock.exeMethodArray[0xFD]=&ins_method_STD;
	ccpu.exeCodeBlock.exeMethodArray[0xFE]=&ins_method_INC_Group4;
	ccpu.exeCodeBlock.exeMethodArray[0xFF]=&ins_method_INC_Group5;
}
void ccpu_final()
{
#if CCPU_RAM == CRAM
	if (cram.base) free((void *)(cram.base));
	cram.base = (t_vaddrcc)NULL;
	cram.size = 0x00;
#endif
}

void ccpu_storeCaculate(const Method opMethod,
	const int bitLength, const t_nubit32 operand1, const t_nubit32 operand2,
	const t_nubit32 result, const t_nubit16 mask_Flag)
{
	if(mask_Flag & MASK_FLAG_SF)
	{
		ccpu.SF_lazyTable.bitLength = bitLength;
		ccpu.SF_lazyTable.result = result;
	}
	if(mask_Flag & MASK_FLAG_ZF)
	{
		ccpu.ZF_lazyTable.result = result;
	}
	if(mask_Flag & MASK_FLAG_PF)
	{
		ccpu.PF_lazyTable.opMethod = opMethod;
		ccpu.PF_lazyTable.bitLength = bitLength;
		ccpu.PF_lazyTable.operand1 = operand1;
		ccpu.PF_lazyTable.operand2 = operand2;
		ccpu.PF_lazyTable.result = result;
	}
	if(mask_Flag & MASK_FLAG_CF)
	{
		ccpu.CF_lazyTable.opMethod = opMethod;
		ccpu.CF_lazyTable.bitLength = bitLength;
		ccpu.CF_lazyTable.operand1 = operand1;
		ccpu.CF_lazyTable.operand2 = operand2;
		ccpu.CF_lazyTable.result = result;
	}
	if(mask_Flag & MASK_FLAG_OF)
	{
		ccpu.OF_lazyTable.opMethod = opMethod;
		ccpu.OF_lazyTable.bitLength = bitLength;
		ccpu.OF_lazyTable.operand1 = operand1;
		ccpu.OF_lazyTable.operand2 = operand2;
		ccpu.OF_lazyTable.result = result;
	}
	if(mask_Flag & MASK_FLAG_AF)
	{
		ccpu.AF_lazyTable.opMethod = opMethod;
		ccpu.AF_lazyTable.bitLength = bitLength;
		ccpu.AF_lazyTable.operand1 = operand1;
		ccpu.AF_lazyTable.operand2 = operand2;
		ccpu.AF_lazyTable.result = result;
	}

	ccpu.needCaculateFlag_mask |= mask_Flag;
}

t_bool ccpu_getOF_Flag()
{
	if( !(ccpu.needCaculateFlag_mask & MASK_FLAG_OF) )
		return ccpu.OF_Flag;
	ccpu.needCaculateFlag_mask &= ~(MASK_FLAG_OF) ;
	
	switch(ccpu.OF_lazyTable.opMethod)
	{
	case ADD_8bit:
	case ADC_8bit:
		if( ((ccpu.OF_lazyTable.operand1&MASK_10000000) == (ccpu.OF_lazyTable.operand2&MASK_10000000)) &&
			((ccpu.OF_lazyTable.operand1&MASK_10000000) !=(ccpu.OF_lazyTable.result&MASK_10000000)))
			return ccpu.OF_Flag = 0x01;
		else
			return ccpu.OF_Flag = 0x00;
		break;
	case ADD_16bit:
	case ADC_16bit:
		if( ((ccpu.OF_lazyTable.operand1&0x8000) == (ccpu.OF_lazyTable.operand2&0x8000)) &&
			((ccpu.OF_lazyTable.operand1&0x8000) !=(ccpu.OF_lazyTable.result&0x8000)))
			return ccpu.OF_Flag = 0x01;
		else
			return ccpu.OF_Flag = 0x00;
		break;
	case SUB_8bit:
	case SBB_8bit:
	case CMP_8bit:
		if( ((ccpu.OF_lazyTable.operand1&MASK_10000000) != (ccpu.OF_lazyTable.operand2&MASK_10000000)) && 
			((ccpu.OF_lazyTable.operand2&MASK_10000000) ==(ccpu.OF_lazyTable.result&MASK_10000000)))
			return ccpu.OF_Flag = 0x01;
		else
			return ccpu.OF_Flag = 0x00;
		break;
	case SUB_16bit:
	case SBB_16bit:
	case CMP_16bit:
		if( ((ccpu.OF_lazyTable.operand1&0x8000) != (ccpu.OF_lazyTable.operand2&0x8000)) &&
			((ccpu.OF_lazyTable.operand2&0x8000) ==(ccpu.OF_lazyTable.result&0x8000)))
			return ccpu.OF_Flag = 0x01;
		else
			return ccpu.OF_Flag = 0x00;
		break;
	default:
		vapiPrint("OF flags lazy comput wrong");
		assert(0x00);
		break;
	}
		return 0x00;
}
t_bool ccpu_getDF_Flag()
{
	//assert(0x00);简单运算，都会没有什么函数会难为它的~~
	if( !(ccpu.needCaculateFlag_mask & MASK_FLAG_DF) )
		return ccpu.DF_Flag;
	ccpu.needCaculateFlag_mask &= ~(MASK_FLAG_DF) ;
		return ccpu.DF_Flag;
}
t_bool ccpu_getIF_Flag()
{
	//assert(0x00);简单运算，都会没有什么函数会难为它的~~
	if( !(ccpu.needCaculateFlag_mask & MASK_FLAG_IF) )
		return ccpu.IF_Flag;
	ccpu.needCaculateFlag_mask &= ~(MASK_FLAG_IF) ;
		return ccpu.IF_Flag;
}
t_bool ccpu_getTF_Flag()
{
	//assert(0x00);简单运算，都会没有什么函数会难为它的~~
	if( !(ccpu.needCaculateFlag_mask & MASK_FLAG_TF) )
		return ccpu.TF_Flag;
	ccpu.needCaculateFlag_mask &= ~(MASK_FLAG_TF) ;
		return ccpu.TF_Flag;
}
t_bool ccpu_getSF_Flag()
{
	if( !(ccpu.needCaculateFlag_mask & MASK_FLAG_SF) )
		return ccpu.SF_Flag;
	ccpu.needCaculateFlag_mask &= ~(MASK_FLAG_SF) ;
	switch(ccpu.SF_lazyTable.bitLength)
	{
	case 8:
		ccpu.SF_Flag = !!(ccpu.SF_lazyTable.result & 0x0080);
		break;
	case 16:
		ccpu.SF_Flag = !!(ccpu.SF_lazyTable.result & 0x8000);
		break;
	default:
		vapiPrint("when comput SF flags , found the bitLength out of handle ");
		assert(0x00);
		break;
	}
	return ccpu.SF_Flag;
}
t_bool ccpu_getZF_Flag()
{
	if( !(ccpu.needCaculateFlag_mask & MASK_FLAG_ZF) )
		return ccpu.ZF_Flag;
	ccpu.needCaculateFlag_mask &= ~(MASK_FLAG_ZF) ;
	return ccpu.ZF_Flag = !ccpu.ZF_lazyTable.result;
}
t_bool ccpu_getAF_Flag()
{
	t_bool operand1_thirdBit, operand2_thirdBit, result_thirdBit;
	if( !(ccpu.needCaculateFlag_mask & MASK_FLAG_AF) )
		return ccpu.AF_Flag;
	ccpu.needCaculateFlag_mask &= ~(MASK_FLAG_AF) ;
	operand1_thirdBit = !!(ccpu.AF_lazyTable.operand1 & MASK_00001000);
	operand2_thirdBit = !!(ccpu.AF_lazyTable.operand2 & MASK_00001000);
	result_thirdBit   = !!(ccpu.AF_lazyTable.result   & MASK_00001000);
	switch(ccpu.AF_lazyTable.opMethod)
	{
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
		ccpu.AF_Flag = !!(((ccpu.AF_lazyTable.operand1 ^ ccpu.AF_lazyTable.operand2) ^ ccpu.AF_lazyTable.result) & 0x10);
		break;
	default:
		vapiPrint("when comput AF flags , found the bitLength out of handle");
		assert(0x00);
		break;
	}
	return ccpu.AF_Flag;
}
t_bool ccpu_getPF_Flag()
{
	t_nubitcc oneNum = 0;
	t_nubit8 result_8bit;
	if( !(ccpu.needCaculateFlag_mask & MASK_FLAG_PF) )
		return ccpu.PF_Flag;
	ccpu.needCaculateFlag_mask &= ~(MASK_FLAG_PF) ;
	result_8bit = (t_nubit8)(ccpu.PF_lazyTable.result);

	while(result_8bit)
	{
		result_8bit &= (result_8bit-1); 
		oneNum++;

	}
	return ccpu.PF_Flag=(0==oneNum%2);//运算结果中的低8位有偶数个1，PF为0x01； 奇数个1，PF为0x00
}
t_bool ccpu_getCF_Flag()
{
	if( !(ccpu.needCaculateFlag_mask & MASK_FLAG_CF) )
		return ccpu.CF_Flag;
	ccpu.needCaculateFlag_mask &= ~(MASK_FLAG_CF) ;
	switch(ccpu.CF_lazyTable.opMethod)
	{//还会后续添加
	case ADD_8bit:
	case ADD_16bit:
		if(ccpu.CF_lazyTable.result < ccpu.CF_lazyTable.operand1 || ccpu.CF_lazyTable.result < ccpu.CF_lazyTable.operand2)
			return ccpu.CF_Flag = 0x01;
		else
			return ccpu.CF_Flag = 0x00;
		break;
	case ADC_8bit://当出现ADC时候，都是表示CF为真，如果CF为假，ADC操作执行的将是ADD操作，SBB同理~~
	case ADC_16bit:
		return ccpu.CF_Flag =(ccpu.CF_lazyTable.result <= ccpu.CF_lazyTable.operand1);
		break;
	case SUB_8bit:
	case SUB_16bit:
	case CMP_8bit:
	case CMP_16bit:
		return ccpu.CF_Flag = (ccpu.CF_lazyTable.operand1 < ccpu.CF_lazyTable.operand2);
		break;
	case SBB_8bit:
		return ccpu.CF_Flag = (ccpu.CF_lazyTable.operand1 < ccpu.CF_lazyTable.result) || (0xff == ccpu.CF_lazyTable.operand2);
		break;
	case SBB_16bit:
		return ccpu.CF_Flag = (ccpu.CF_lazyTable.operand1 < ccpu.CF_lazyTable.result) || (0xffff == ccpu.CF_lazyTable.operand2); 
		break;
	default:
		vapiPrint("when comput CF flags , found the bitLength out of handle");
		assert(0x00);
		break;
	}
	return ccpu.CF_Flag;
}

void ccpu_setOF_Flag_flag(const t_bool f)
{
	ccpu.OF_Flag = f;
	ccpu.needCaculateFlag_mask &= ~MASK_FLAG_OF;
}
void ccpu_setDF_Flag_flag(const t_bool f)
{
	ccpu.DF_Flag = f;
	ccpu.needCaculateFlag_mask &= ~MASK_FLAG_DF;
}
void ccpu_setIF_Flag_flag(const t_bool f)
{
	ccpu.IF_Flag = f;
	ccpu.needCaculateFlag_mask &= ~MASK_FLAG_IF;
}
void ccpu_setTF_Flag_flag(const t_bool f)
{
	ccpu.TF_Flag = f;
	ccpu.needCaculateFlag_mask &= ~MASK_FLAG_TF;
}
void ccpu_setSF_Flag_flag(const t_bool f)
{
	ccpu.SF_Flag = f;
	ccpu.needCaculateFlag_mask &= ~MASK_FLAG_SF;
}
void ccpu_setSF_Flag_data8(const t_nubit8 data)
{
	ccpu.SF_Flag = !!(data & MASK_10000000);
	ccpu.needCaculateFlag_mask &= ~MASK_FLAG_SF;
}
void ccpu_setSF_Flag_data16(const t_nubit16 data)
{
	ccpu.SF_Flag = !!(data & 0x8000);
	ccpu.needCaculateFlag_mask &= ~MASK_FLAG_SF;
}
void ccpu_setZF_Flag_flag(const t_bool f)
{
	ccpu.ZF_Flag = f;
	ccpu.needCaculateFlag_mask &= ~MASK_FLAG_ZF;
}
void ccpu_setZF_Flag_data8(const t_nubit8 data)
{
	ccpu.ZF_Flag = !data;
	ccpu.needCaculateFlag_mask &= ~MASK_FLAG_ZF;
}
void ccpu_setZF_Flag_data16(const t_nubit16 data)
{
	ccpu.ZF_Flag = !data;
	ccpu.needCaculateFlag_mask &= ~MASK_FLAG_ZF;
}
void ccpu_setAF_Flag_flag(const t_bool f)
{
	ccpu.AF_Flag = f;
	ccpu.needCaculateFlag_mask &= ~MASK_FLAG_AF;
}
void ccpu_setPF_Flag_flag(const t_bool f)
{
	ccpu.PF_Flag = f;
	ccpu.needCaculateFlag_mask &= ~MASK_FLAG_PF;
}
void ccpu_setPF_Flag_data8(const t_nubit8 data)
{
	int oneNum = 0;
	t_nubit8 result_8bit = (t_nubit8)data;

	while(result_8bit)
	{
		result_8bit &= (result_8bit-1); 
		oneNum++;

	}
	ccpu.PF_Flag=(0==oneNum%2);//运算结果中的低8位有偶数个1，PF为0x01； 奇数个1，PF为0x00
	ccpu.needCaculateFlag_mask &= ~MASK_FLAG_PF;
}
void ccpu_setPF_Flag_data16(const t_nubit16 data)
{
	ccpu_setPF_Flag_data8((t_nubit8)data);
}
void ccpu_setCF_Flag_flag(const t_bool f)
{
	ccpu.CF_Flag = f;
	ccpu.needCaculateFlag_mask &= ~MASK_FLAG_CF;
}
void ccpu_setSF_ZF_PF_byResult_data8(const t_nubit8 result)
{
	ccpu_setZF_Flag_data8(result);
	ccpu_setSF_Flag_data8(result);
	ccpu_setPF_Flag_data8(result);

}
void ccpu_setSF_ZF_PF_byResult_data16(const t_nubit16 result)
{
	ccpu_setZF_Flag_data16(result);
	ccpu_setSF_Flag_data16(result);
	ccpu_setPF_Flag_data16(result);
}

t_nubit16 ccpu_generateFLAG()
{
	//t_nubit16 flags=0x7000 + 2;//根据bochs。。还有，flags的第二位总是1，所以加上2
	t_nubit16 flags = ccpu.flag_extern&0xf000;
	//flags|=0x2;//第二位要为1
	flags&=0xffd7;//第四位要为0,第六位为0
	if(ccpu_getOF_Flag())
		flags |= MASK_FLAG_OF;
	if(ccpu_getDF_Flag())
		flags |= MASK_FLAG_DF;
	if(ccpu_getIF_Flag())
		flags |= MASK_FLAG_IF;
	if(ccpu_getTF_Flag())
		flags |= MASK_FLAG_TF;
	if(ccpu_getSF_Flag())
		flags |= MASK_FLAG_SF;
	if(ccpu_getZF_Flag())
		flags |= MASK_FLAG_ZF;
	if(ccpu_getAF_Flag())
		flags |= MASK_FLAG_AF;
	if(ccpu_getPF_Flag())
		flags |= MASK_FLAG_PF;
	if(ccpu_getCF_Flag())
		flags |= MASK_FLAG_CF;
	
	return flags;

}
void ccpu_setFLAG(const t_nubit16 flags)
{
	ccpu_setOF_Flag_flag(!!(flags&MASK_FLAG_OF));
	ccpu_setDF_Flag_flag(!!(flags&MASK_FLAG_DF));
	ccpu_setIF_Flag_flag(!!(flags&MASK_FLAG_IF));
	ccpu_setTF_Flag_flag(!!(flags&MASK_FLAG_TF));
	ccpu_setSF_Flag_flag(!!(flags&MASK_FLAG_SF));
	ccpu_setZF_Flag_flag(!!(flags&MASK_FLAG_ZF));
	ccpu_setAF_Flag_flag(!!(flags&MASK_FLAG_AF));
	ccpu_setPF_Flag_flag(!!(flags&MASK_FLAG_PF));
	ccpu_setCF_Flag_flag(!!(flags&MASK_FLAG_CF));
	ccpu.needCaculateFlag_mask = 0;//去不不需要再次计算
	ccpu.flag_extern = flags;
}
void ccpu_set_SF_ZF_AF_PF_CF_Flag(const t_nubit16 flags)
{
	ccpu_setSF_Flag_flag(!!(flags&MASK_FLAG_SF));
	ccpu_setZF_Flag_flag(!!(flags&MASK_FLAG_ZF));
	ccpu_setAF_Flag_flag(!!(flags&MASK_FLAG_AF));
	ccpu_setPF_Flag_flag(!!(flags&MASK_FLAG_PF));
	ccpu_setCF_Flag_flag(!!(flags&MASK_FLAG_CF));
	ccpu.needCaculateFlag_mask &= ~ (MASK_FLAG_SF|MASK_FLAG_ZF|MASK_FLAG_AF|MASK_FLAG_PF|MASK_FLAG_CF);
}