#include "../vmachine.h"
#include "../vapi.h"
#include "ccpuapi.h"

void ins_method_ADD_Eb_Gb()
{
	ins_atomMethod_ADD_8bit(p_nubit8(ccpu.deCodeBlock.prm), d_nubit8(ccpu.deCodeBlock.preg));
}
void ins_method_ADD_Ev_Gv()
{
	ins_atomMethod_ADD_16bit(p_nubit16(ccpu.deCodeBlock.prm), d_nubit16(ccpu.deCodeBlock.preg));
}
void ins_method_ADD_Gb_Eb()
{
	ins_atomMethod_ADD_8bit(p_nubit8(ccpu.deCodeBlock.preg), d_nubit8(ccpu.deCodeBlock.prm));
}
void ins_method_ADD_Gv_Ev()
{
	ins_atomMethod_ADD_16bit(p_nubit16(ccpu.deCodeBlock.preg), d_nubit16(ccpu.deCodeBlock.prm));
}
void ins_method_ADD_AL_Ib()
{
	ins_atomMethod_ADD_8bit(&ccpu.al, d_nubit8(&ccpu.deCodeBlock.immData_8Bit));
}
void ins_method_ADD_eAX_Iv()
{
	ins_atomMethod_ADD_16bit(&ccpu.ax, d_nubit16(&ccpu.deCodeBlock.immData_16Bit));
}

void ins_method_PUSH_ES()//0x06
{
	ins_atomMethod_PUSH(ccpu.es);
}
void ins_method_POP_ES()
{
	ccpu.es = ins_atomMethod_POP();
}


void ins_method_ADC_Eb_Gb()
{
	ins_atomMethod_ADC_8bit(p_nubit8(ccpu.deCodeBlock.prm), d_nubit8(ccpu.deCodeBlock.preg));
}
void ins_method_ADC_Ev_Gv()
{
	ins_atomMethod_ADC_16bit(p_nubit16(ccpu.deCodeBlock.prm), d_nubit16(ccpu.deCodeBlock.preg));
}
void ins_method_ADC_Gb_Eb()
{
	ins_atomMethod_ADC_8bit(p_nubit8(ccpu.deCodeBlock.preg), d_nubit8(ccpu.deCodeBlock.prm));
}
void ins_method_ADC_Gv_Ev()
{
	ins_atomMethod_ADC_16bit(p_nubit16(ccpu.deCodeBlock.preg), d_nubit16(ccpu.deCodeBlock.prm));
}
void ins_method_ADC_AL_Ib()
{
	ins_atomMethod_ADC_8bit(&ccpu.al, ccpu.deCodeBlock.immData_8Bit);
}
void ins_method_ADC_eAX_Iv()
{
	ins_atomMethod_ADC_16bit(&ccpu.ax, ccpu.deCodeBlock.immData_16Bit);
}

void ins_method_PUSH_SS()
{
	ins_atomMethod_PUSH(ccpu.ss);
}
void ins_method_POP_SS()
{
	ccpu.ss = ins_atomMethod_POP();
}

void ins_method_AND_Eb_Gb()
{
	ins_atomMethod_AND_8bit(p_nubit8(ccpu.deCodeBlock.prm), d_nubit8(ccpu.deCodeBlock.preg));
}
void ins_method_AND_Ev_Gv()
{
	ins_atomMethod_AND_16bit(p_nubit16(ccpu.deCodeBlock.prm), d_nubit16(ccpu.deCodeBlock.preg));
}
void ins_method_AND_Gb_Eb()
{
	ins_atomMethod_AND_8bit(p_nubit8(ccpu.deCodeBlock.preg), d_nubit8(ccpu.deCodeBlock.prm));
}
void ins_method_AND_Gv_Ev()
{
	ins_atomMethod_AND_16bit(p_nubit16(ccpu.deCodeBlock.preg), d_nubit16(ccpu.deCodeBlock.prm));
}
void ins_method_AND_AL_Ib()
{
	ins_atomMethod_AND_8bit(&ccpu.al, ccpu.deCodeBlock.immData_8Bit);
}
void ins_method_AND_eAX_Iv()
{
	ins_atomMethod_AND_16bit(&ccpu.ax, ccpu.deCodeBlock.immData_16Bit);
}

void ins_method_SEG_ES()
{
	ccpu.deCodeBlock.prefix_SegmentOverrideOpCode = ES_SEGMENT_OVERRIDE_PREFIX;
}
void ins_method_DAA()//0x27
{
	t_nubit8 old_al = ccpu.al;
	t_bool old_CF = ccpu_getCF_Flag();

	ccpu_setCF_Flag_flag(0x00);
	if((ccpu.al & 0x0f)>9 || (0x01 ==ccpu_getAF_Flag()))
	{
		ins_atomMethod_ADD_8bit(&ccpu.al, 6);
		ccpu_setCF_Flag_flag(old_CF); //这里可能会有点问题
		ccpu_setAF_Flag_flag(0x01);
	}
	else
	{
		ccpu_setAF_Flag_flag(0x00);
	}
    //*****
	if((old_al>0x99) || (0x01 == old_CF))
	{
		ins_atomMethod_ADD_8bit(&ccpu.al, 0x60);
		ccpu_setCF_Flag_flag(0x01);

	}
	else
	{
		ccpu_setCF_Flag_flag(0x00);
	}

}

void ins_method_XOR_Eb_Gb()
{
	ins_atomMethod_XOR_8bit(p_nubit8(ccpu.deCodeBlock.prm), d_nubit8(ccpu.deCodeBlock.preg));
}
void ins_method_XOR_Ev_Gv()
{
	ins_atomMethod_XOR_16bit(p_nubit16(ccpu.deCodeBlock.prm), d_nubit16(ccpu.deCodeBlock.preg));
}
void ins_method_XOR_Gb_Eb()
{
	ins_atomMethod_XOR_8bit(p_nubit8(ccpu.deCodeBlock.preg), d_nubit8(ccpu.deCodeBlock.prm));
}
void ins_method_XOR_Gv_Ev()
{
	ins_atomMethod_XOR_16bit(p_nubit16(ccpu.deCodeBlock.preg), d_nubit16(ccpu.deCodeBlock.prm));
}
void ins_method_XOR_AL_Ib()
{
	ins_atomMethod_XOR_8bit(&ccpu.al, ccpu.deCodeBlock.immData_8Bit);
}
void ins_method_XOR_eAX_Iv()
{
	ins_atomMethod_XOR_16bit(&ccpu.ax, ccpu.deCodeBlock.immData_16Bit);
}

void ins_method_SEG_SS()
{
	ccpu.deCodeBlock.prefix_SegmentOverrideOpCode = SS_SEGMENT_OVERRIDE_PREFIX;
}
void ins_method_AAA()
{
	if((ccpu.al & 0x0f) > 9 || 0x01==ccpu_getAF_Flag())
	{
		ccpu.al += 6;
		ccpu.ah++;
		ccpu_setAF_Flag_flag(0x01);
		ccpu_setCF_Flag_flag(0x01);
		ccpu.al &= 0x0f;
	}
	else
	{
		ccpu_setAF_Flag_flag(0x00);
		ccpu_setCF_Flag_flag(0x00);
		ccpu.al &= 0x0f;

	}
}

void ins_method_INC_eAX()
{
	ins_atomMethod_INC_16bit(&ccpu.ax);
}
void ins_method_INC_eCX()
{
	ins_atomMethod_INC_16bit(&ccpu.cx);
}
void ins_method_INC_eDX()
{
	ins_atomMethod_INC_16bit(&ccpu.dx);
}
void ins_method_INC_eBX()
{
	ins_atomMethod_INC_16bit(&ccpu.bx);
}
void ins_method_INC_eSP()
{
	ins_atomMethod_INC_16bit(&ccpu.sp);
}
void ins_method_INC_eBP()
{
	ins_atomMethod_INC_16bit(&ccpu.bp);
}
void ins_method_INC_eSI()
{
	ins_atomMethod_INC_16bit(&ccpu.si);
}
void ins_method_INC_eDI()
{
	ins_atomMethod_INC_16bit(&ccpu.di);
}

void ins_method_PUSH_eAX()//0x50
{
	ins_atomMethod_PUSH(ccpu.ax);
}
void ins_method_PUSH_eCX()
{
	ins_atomMethod_PUSH(ccpu.cx);
}
void ins_method_PUSH_eDX()
{
	ins_atomMethod_PUSH(ccpu.dx);
}
void ins_method_PUSH_eBX()
{
	ins_atomMethod_PUSH(ccpu.bx);
}
void ins_method_PUSH_eSP()
{
	ins_atomMethod_PUSH(ccpu.sp);
}
void ins_method_PUSH_eBP()
{
	ins_atomMethod_PUSH(ccpu.bp);
}
void ins_method_PUSH_eSI()
{
	ins_atomMethod_PUSH(ccpu.si);
}
void ins_method_PUSH_eDI()
{
	ins_atomMethod_PUSH(ccpu.di);
}

void ins_method_PUSHA_PUSHAD(){/*do nothing*/}//80x86没有
void ins_method_POPA_POPAD(){/*do nothing*/}//80x86没有
void ins_method_BOUND_Gv_Ma(){/*do nothing*/}//80x86没有
void ins_method_ARPL_Ew_Gw(){/*do nothing*/}//80x86没有
void ins_method_SEG_FS(){/*do nothing*/}
void ins_method_SEG_GS(){/*do nothing*/}
void ins_method_Operand_Size(){/*do nothing*/}//80x86没有
void ins_method_Address_Size(){/*do nothing*/}//80x86没有

void ins_method_ShortJump_JO()//0x70
{
	if(0x01 == ccpu_getOF_Flag())
	{
		ins_atomMethod_shortJMP();
	}
}
void ins_method_ShortJump_JNO()//0x71
{
	if(0x00 == ccpu_getOF_Flag())
	{
		ins_atomMethod_shortJMP();
	}
}
void ins_method_ShortJump_JB()//0x72
{
	if(0x01 == ccpu_getCF_Flag())
	{
		ins_atomMethod_shortJMP();
	}
}
void ins_method_ShortJump_JNB()//0x73
{
	if(0x00 == ccpu_getCF_Flag())
	{
		ins_atomMethod_shortJMP();
	}
}
void ins_method_ShortJump_JZ()//0x74
{
	if(0x01 == ccpu_getZF_Flag())
	{
		ins_atomMethod_shortJMP();
	}
}
void ins_method_ShortJump_JNZ()//0x75
{
	if(0x00 == ccpu_getZF_Flag())
	{
		ins_atomMethod_shortJMP();
	}
}
void ins_method_ShortJump_JBE()//0x76
{
	if((0x01 == ccpu_getCF_Flag()) || (0x01 == ccpu_getZF_Flag()))
	{
		ins_atomMethod_shortJMP();
	}
}
void ins_method_ShortJump_JNBE()//0
{
	if(0x00 == ccpu_getCF_Flag() && 0x00 == ccpu_getZF_Flag())
	{
		ins_atomMethod_shortJMP();
	}

}

void ins_method_IMMGroup_Eb_Ib()
{
	switch(ccpu.deCodeBlock.nnn)
	{
	case 0:
			ins_methodGroup_ADD_Eb_Ib();
			break;
	case 1:
			ins_methodGroup_OR_Eb_Ib();
			break;
	case 2:
		ins_methodGroup_ADC_Eb_Ib();
		break;
	case 3:
		ins_methodGroup_SBB_Eb_Ib();
		break;
	case 4:
		ins_methodGroup_AND_Eb_Ib();
		break;
	case 5:
		ins_methodGroup_SUB_Eb_Ib();
		break;
	case 6:
		ins_methodGroup_XOR_Eb_Ib();
		break;
	case 7:
		ins_methodGroup_CMP_Eb_Ib();
		break;
	}
}
void ins_method_IMMGroup_Ev_Iz()
{
	switch(ccpu.deCodeBlock.nnn)
	{
	case 0:
			ins_methodGroup_ADD_Ev_Iv();
			break;
	case 1:
			ins_methodGroup_OR_Ev_Iv();
			break;
	case 2:
		ins_methodGroup_ADC_Ev_Iv();
		break;
	case 3:
		ins_methodGroup_SBB_Ev_Iv();
		break;
	case 4:
		ins_methodGroup_AND_Ev_Iv();
		break;
	case 5:
		ins_methodGroup_SUB_Ev_Iv();
		break;
	case 6:
		ins_methodGroup_XOR_Ev_Iv();
		break;
	case 7:
		ins_methodGroup_CMP_Ev_Iv();
		break;
	}
}
void ins_method_IMMGroup_Eb_IbX()
{
	ins_method_IMMGroup_Eb_Ib();
}
void ins_method_IMMGroup_Ev_Ib()
{
	switch(ccpu.deCodeBlock.nnn)
	{
	case 0:
			ins_methodGroup_ADD_Ev_Iv();
			break;
	case 1:
			ins_methodGroup_OR_Ev_Iv();
			break;
	case 2:
		ins_methodGroup_ADC_Ev_Iv();
		break;
	case 3:
		ins_methodGroup_SBB_Ev_Iv();
		break;
	case 4:
		ins_methodGroup_AND_Ev_Iv();
		break;
	case 5:
		ins_methodGroup_SUB_Ev_Iv();
		break;
	case 6:
		ins_methodGroup_XOR_Ev_Iv();
		break;
	case 7:
		ins_methodGroup_CMP_Ev_Iv();
		break;
	}
}

void ins_method_TEST_Eb_Gb()//0x84
{
	ins_atomMethod_TEST_8bit(p_nubit8(ccpu.deCodeBlock.prm), d_nubit8(ccpu.deCodeBlock.preg));
}
void ins_method_TEST_Ev_Gv()
{
	ins_atomMethod_TEST_16bit(p_nubit16(ccpu.deCodeBlock.prm), d_nubit16(ccpu.deCodeBlock.preg));
}

void ins_method_XCHG_Eb_Gb()//0x86
{
	t_nubit8 tmp = d_nubit8(ccpu.deCodeBlock.preg);
	d_nubit8(ccpu.deCodeBlock.preg) = d_nubit8(ccpu.deCodeBlock.prm);
	if (!im(ccpu.deCodeBlock.prm)) d_nubit8(ccpu.deCodeBlock.prm)= tmp;
}
void ins_method_XCHG_Ev_Gv()
{
	t_nubit16 tmp = d_nubit16(ccpu.deCodeBlock.preg);
	d_nubit16(ccpu.deCodeBlock.preg) = d_nubit16(ccpu.deCodeBlock.prm);
	if (!im(ccpu.deCodeBlock.prm)) d_nubit16(ccpu.deCodeBlock.prm)= tmp;
}

void ins_method_NOP(){/* do nothing*/}

void ins_method_XCHG_eCX()//0x91
{
	t_nubit16 tmp = ccpu.cx;
	ccpu.cx = ccpu.ax;
	ccpu.ax = tmp;
}
void ins_method_XCHG_eDX()
{
	t_nubit16 tmp = ccpu.dx;
	ccpu.dx = ccpu.ax;
	ccpu.ax = tmp;
}
void ins_method_XCHG_eBX()
{
	t_nubit16 tmp = ccpu.bx;
	ccpu.bx = ccpu.ax;
	ccpu.ax = tmp;
}
void ins_method_XCHG_eSP()
{
	t_nubit16 tmp = ccpu.sp;
	ccpu.sp = ccpu.ax;
	ccpu.ax = tmp;
}
void ins_method_XCHG_eBP()
{
	t_nubit16 tmp = ccpu.bp;
	ccpu.bp = ccpu.ax;
	ccpu.ax = tmp;
}
void ins_method_XCHG_eSI()
{
	t_nubit16 tmp = ccpu.si;
	ccpu.si = ccpu.ax;
	ccpu.ax = tmp;
}
void ins_method_XCHG_eDI()
{
	t_nubit16 tmp = ccpu.di;
	ccpu.di = ccpu.ax;
	ccpu.ax = tmp;
}

void ins_method_MOV_AL_Ob()//tested
{
	ccpu.al = d_nubit8(ccpu.deCodeBlock.prm);
}
void ins_method_MOV_eAX_Ov()//tested
{
	ccpu.ax = d_nubit16(ccpu.deCodeBlock.prm);
}
void ins_method_MOV_Ob_AL()//tested
{
	d_nubit8(ccpu.deCodeBlock.prm) = ccpu.al;
}
void ins_method_MOV_Ov_eAX()//tested
{
	d_nubit16(ccpu.deCodeBlock.prm) = ccpu.ax;
}

void ins_method_MOVSB_Xb_Yb()
{
	if(ccpu.deCodeBlock.prefix_RepeatOpCode == REPNE_PREFIX ||
		ccpu.deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX)
	{
		ccpu.exeCodeBlock.repeatMethod = &ins_atomMethod_MOVSB;//绑定指令操作函数
		ins_chooseRepeatExeStringMethod();//选择指令执行方式
		return;
	}
	else
		ins_atomMethod_MOVSB();
}
void ins_method_MOVSW_Xv_Yv()
{
	if(ccpu.deCodeBlock.prefix_RepeatOpCode == REPNE_PREFIX ||
		ccpu.deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		ccpu.exeCodeBlock.repeatMethod = &ins_atomMethod_MOVSW;
		ins_chooseRepeatExeStringMethod();
		return;
	}
	else
		ins_atomMethod_MOVSW();
}
void ins_method_CMPSB_Xb_Yb()//0xa6
{
	if(ccpu.deCodeBlock.prefix_RepeatOpCode == REPNE_PREFIX ||
		ccpu.deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		ccpu.exeCodeBlock.repeatMethod = &ins_atomMethod_CMPSB;
		ins_chooseRepeatExeStringMethod();
		return;
	}
	else
		ins_atomMethod_CMPSB();
}
void ins_method_CMPSW_Xv_Yv()//0xa7
{
	if(ccpu.deCodeBlock.prefix_RepeatOpCode == REPNE_PREFIX ||
		ccpu.deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进
		ccpu.exeCodeBlock.repeatMethod = &ins_atomMethod_CMPSW;
		ins_chooseRepeatExeStringMethod();
		return;
	}
	else
		ins_atomMethod_CMPSW();
}

void ins_method_MOV_AL()//tested
{
	ccpu.al = ccpu.deCodeBlock.immData_8Bit;
}
void ins_method_MOV_CL()//tested
{
	ccpu.cl = ccpu.deCodeBlock.immData_8Bit;
}
void ins_method_MOV_DL()//tested
{
	ccpu.dl = ccpu.deCodeBlock.immData_8Bit;
}
void ins_method_MOV_BL()//tested
{
	ccpu.bl = ccpu.deCodeBlock.immData_8Bit;
}
void ins_method_MOV_AH()//tested
{
	ccpu.ah =ccpu.deCodeBlock.immData_8Bit;
}
void ins_method_MOV_CH()//tested
{
	ccpu.ch =ccpu.deCodeBlock.immData_8Bit;
}
void ins_method_MOV_DH()//tested
{
	ccpu.dh = ccpu.deCodeBlock.immData_8Bit;
}
void ins_method_MOV_BH()//tested
{
	ccpu.bh =ccpu.deCodeBlock.immData_8Bit;
}

void ins_method_ShiftGroup_Eb_Ib(){/*do nothing*/}//80x86没有
void ins_method_ShiftGroup_Ev_Ib(){/*do nothing*/}//80x86没有
void ins_method_RET_near_Iw() //带立即数的段内返回
{
	ccpu.ip = ins_atomMethod_POP();
	ccpu.sp += ccpu.deCodeBlock.opContactData_16bit;
}
//that would be something
void ins_method_RET_near_None()//段内返回
{
	ccpu.ip = ins_atomMethod_POP();
}

void ins_method_LES_Gv_Mp()
{
	d_nubit16(ccpu.deCodeBlock.preg) = d_nubit16(ccpu.deCodeBlock.prm);
	ccpu.es = d_nubit16((t_nubit8 *)(ccpu.deCodeBlock.prm) + 2);
}
void ins_method_LDS_Gv_Mp()
{
	d_nubit16(ccpu.deCodeBlock.preg) = d_nubit16(ccpu.deCodeBlock.prm);
	ccpu.ds = d_nubit16((t_nubit8 *)(ccpu.deCodeBlock.prm) + 2);
}

void ins_method_MOV_Eb_Ib()//tested
{
	d_nubit8(ccpu.deCodeBlock.prm) = ccpu.deCodeBlock.immData_8Bit;
}
void ins_method_MOV_Ev_Iv()//tested
{
	d_nubit16(ccpu.deCodeBlock.prm) = ccpu.deCodeBlock.immData_16Bit;
}

void ins_method_ShiftGroup2_Eb_1()
{
	switch(ccpu.deCodeBlock.nnn)
	{
	case 0://nnn=000
		ins_methodGroup_ROL_Eb_1();
		break;
	case 1://nnn=001
		ins_methodGroup_ROR_Eb_1();
		break;
	case 2://nnn=010
		ins_methodGroup_RCL_Eb_1();
		break;
	case 3://nnn=011
		ins_methodGroup_RCR_Eb_1();
		break;
	case 4://nnn=100
		ins_methodGroup_SHL_Eb_1();
		break;
	case 5://nnn=101
		ins_methodGroup_SHR_Eb_1();
		break;
	case 6://nnn=110
		vapiPrint("ins_method_ShiftGroup2_Eb_1 UnHandle the nnn bits");
		assert(0x00);
		return;
	case 7://nnn=111
		ins_methodGroup_SAR_Eb_1();
		break;
	}
}
void ins_method_ShiftGroup2_Ev_1()
{
	switch(ccpu.deCodeBlock.nnn)
	{
	case 0://nnn=000
		ins_methodGroup_ROL_Ev_1();
		break;
	case 1://nnn=001
		ins_methodGroup_ROR_Ev_1();
		break;
	case 2://nnn=010
		ins_methodGroup_RCL_Ev_1();
		break;
	case 3://nnn=011
		ins_methodGroup_RCR_Ev_1();
		break;
	case 4://nnn=100
		ins_methodGroup_SHL_Ev_1();
		break;
	case 5://nnn=101
		ins_methodGroup_SHR_Ev_1();
		break;
	case 6://nnn=110
		vapiPrint("ins_method_ShiftGroup2_Ev_1 UnHandle the nnn bits");
		assert(0x00);
		return;
	case 7://nnn=111
		ins_methodGroup_SAR_Ev_1();
		break;
	}
}
void ins_method_ShiftGroup2_Eb_CL()
{
	switch(ccpu.deCodeBlock.nnn)
	{
	case 0://nnn=000
		ins_methodGroup_ROL_Eb_CL();
		break;
	case 1://nnn=001
		ins_methodGroup_ROR_Eb_CL();
		break;
	case 2://nnn=010
		ins_methodGroup_RCL_Eb_CL();
		break;
	case 3://nnn=011
		ins_methodGroup_RCR_Eb_CL();
		break;
	case 4://nnn=100
		ins_methodGroup_SHL_Eb_CL();
		break;
	case 5://nnn=101
		ins_methodGroup_SHR_Eb_CL();
		break;
	case 6://nnn=110
		vapiPrint("ins_method_ShiftGroup2_Eb_CL UnHandle the nnn bits");
		assert(0x00);
		return;
	case 7://nnn=111
		ins_methodGroup_SAR_Eb_CL();
		break;
	}
}
void ins_method_ShiftGroup2_Ev_CL()
{
	switch(ccpu.deCodeBlock.nnn)
	{
	case 0://nnn=000
		ins_methodGroup_ROL_Ev_CL();
		break;
	case 1://nnn=001
		ins_methodGroup_ROR_Ev_CL();
		break;
	case 2://nnn=010
		ins_methodGroup_RCL_Ev_CL();
		break;
	case 3://nnn=011
		ins_methodGroup_RCR_Ev_CL();
		break;
	case 4://nnn=100
		ins_methodGroup_SHL_Ev_CL();
		break;
	case 5://nnn=101
		ins_methodGroup_SHR_Ev_CL();
		break;
	case 6://nnn=110
		vapiPrint("ins_method_ShiftGroup2_Ev_CL UnHandle the nnn bits");
		assert(0x00);
		return;
	case 7://nnn=111
		ins_methodGroup_SAR_Ev_CL();
		break;
	}
}

void ins_method_AMM()
{
	t_nubit8 operand1 = ccpu.al;
	ccpu.ah = ccpu.al / 10;
	ccpu.al %= 10;
	ccpu_storeCaculate(MOD_8bit, 8, operand1, 10, ccpu.al, MASK_FLAG_SF | MASK_FLAG_ZF | MASK_FLAG_PF);
}
void ins_method_AAD()
{
	const t_bool tmpAF = ccpu_getAF_Flag();
	const t_bool tmpCF = ccpu_getCF_Flag();
	const t_bool tmpOF = ccpu_getOF_Flag();
	const t_nubit8 tmp = ccpu.ah * 10;
	ins_atomMethod_ADD_8bit(&ccpu.al,tmp);
	ccpu.ah = 0;
	ccpu_setAF_Flag_flag(tmpAF);
	ccpu_setCF_Flag_flag(tmpCF);
	ccpu_setOF_Flag_flag(tmpOF);
}

void ins_method_XLAT()
{
		ccpu.al = getMemData_byte(ccpu.al + ccpu.bx + (decode_getDefaultSegment_DS() <<4));
}

void ins_method_LOOPN_Jb()//0xe0
{
	if((0 != --(ccpu.cx)) && (0x00 == ccpu_getZF_Flag()))
		ins_atomMethod_shortJMP();
}
void ins_method_LOOPE_Jb()
{
	if((0 != --(ccpu.cx)) && (0x01 == ccpu_getZF_Flag()))
		ins_atomMethod_shortJMP();
}
void ins_method_LOOP_Jb()
{
	if(0 != --(ccpu.cx))
		ins_atomMethod_shortJMP();
}
void ins_method_JCXZ_Jb()
{
	if(0 == ccpu.cx )
		ins_atomMethod_shortJMP();
}

void ins_method_IN_AL_Ib()
{
	//ExecFun(vport.in[ccpu.deCodeBlock.opContactData_8bit]);
	ccpu.flagignore = 0x01;
}
void ins_method_IN_eAX_Ib(){
	ccpu.flagignore = 0x01;
}
void ins_method_OUT_Ib_AL()
{
	ccpu.flagignore = 0x01;
// NEKO LOG
//	ExecFun(vport.out[ccpu.deCodeBlock.opContactData_8bit]);
}
void ins_method_OUT_Ib_eAX(){
	ccpu.flagignore = 0x01;
}

void ins_method_LOCK(){/*在这个版本的虚拟机内，此指令什么都不做*/}
void ins_method_REPNE()
{
	ccpu.deCodeBlock.prefix_RepeatOpCode = REPNE_PREFIX;
}
void ins_method_REP_REPE()
{
	ccpu.deCodeBlock.prefix_RepeatOpCode = REP_PREFIX;
}
void ins_method_HLT(){}//还不清楚
void ins_method_CMC()//0xf5
{
	ccpu_setCF_Flag_flag(!ccpu_getCF_Flag() );
}
void ins_method_UnaryGroup_Eb()
{
	switch(ccpu.deCodeBlock.nnn)
	{
	case 0://nnn=000
		ins_methodGroup_TEST_Ib();
		break;
	case 1://nnn=001
		vapiPrint("ins_method_UnaryGroup_Eb UnHandle the nnn bits");
		assert(0x00);
		break;
	case 2://nnn=010
		ins_methodGroup_NOT_8bit();
		break;
	case 3://nnn=011
		ins_methodGroup_NEG_8bit();
		break;
	case 4://nnn=100
		ins_methodGroup_MUL_AL();
		break;
	case 5://nnn=101
		ins_methodGroup_IMUL_AL();
		break;
	case 6://nnn=110
		ins_methodGroup_DIV_AL();
		return;
	case 7://nnn=111
		ins_methodGroup_IDIV_AL();
		break;
	}
}
void ins_method_UnaryGroup_Ev()
{
	switch(ccpu.deCodeBlock.nnn)
	{
	case 0://nnn=000
		ins_methodGroup_TEST_Iv();
		break;
	case 1://nnn=001
		vapiPrint("ins_method_UnaryGroup_Ev UnHandle the nnn bits");
		assert(0x00);
		break;
	case 2://nnn=010
		ins_methodGroup_NOT_16bit();
		break;
	case 3://nnn=011
		ins_methodGroup_NEG_16bit();
		break;
	case 4://nnn=100
		ins_methodGroup_MUL_eAX();
		break;
	case 5://nnn=101
		ins_methodGroup_IMUL_eAX();
		break;
	case 6://nnn=110
		ins_methodGroup_DIV_eAX();
		return;
	case 7://nnn=111
		ins_methodGroup_IDIV_eAX();
		break;
	}
}

void ins_method_OR_Eb_Gb()
{
	ins_atomMethod_OR_8bit(p_nubit8(ccpu.deCodeBlock.prm), d_nubit8(ccpu.deCodeBlock.preg));

}
void ins_method_OR_Ev_Gv()
{
	ins_atomMethod_OR_16bit(p_nubit16(ccpu.deCodeBlock.prm), d_nubit16(ccpu.deCodeBlock.preg));
}
void ins_method_OR_Gb_Eb()
{
	ins_atomMethod_OR_8bit(p_nubit8(ccpu.deCodeBlock.preg), d_nubit8(ccpu.deCodeBlock.prm));
}
void ins_method_OR_Gv_Ev()
{
	ins_atomMethod_OR_16bit(p_nubit16(ccpu.deCodeBlock.preg), d_nubit16(ccpu.deCodeBlock.prm));
}
void ins_method_OR_AL_Ib()
{
	ins_atomMethod_OR_8bit(&ccpu.al, ccpu.deCodeBlock.immData_8Bit);
}
void ins_method_OR_eAX_Iv()
{
	ins_atomMethod_OR_16bit(&ccpu.ax, ccpu.deCodeBlock.immData_16Bit);
}

void ins_method_PUSH_CS()//0x0e
{
	ins_atomMethod_PUSH(ccpu.cs);
}
void ins_method_EscapeCode(){}

void ins_method_SBB_Eb_Gb()//tested
{
	ins_atomMethod_SBB_8bit(p_nubit8(ccpu.deCodeBlock.prm), d_nubit8(ccpu.deCodeBlock.preg));

}
void ins_method_SBB_Ev_Gv()//tested
{
	ins_atomMethod_SBB_16bit(p_nubit16(ccpu.deCodeBlock.prm), d_nubit16(ccpu.deCodeBlock.preg));
}
void ins_method_SBB_Gb_Eb()//tested
{
	ins_atomMethod_SBB_8bit(p_nubit8(ccpu.deCodeBlock.preg), d_nubit8(ccpu.deCodeBlock.prm));
}
void ins_method_SBB_Gv_Ev()//tested
{
	ins_atomMethod_SBB_16bit(p_nubit16(ccpu.deCodeBlock.preg), d_nubit16(ccpu.deCodeBlock.prm));
}
void ins_method_SBB_AL_Ib()//tested
{
	ins_atomMethod_SBB_8bit(&ccpu.al, ccpu.deCodeBlock.immData_8Bit);
}
void ins_method_SBB_eAX_Iv()//tested
{
	ins_atomMethod_SBB_16bit(&ccpu.ax, ccpu.deCodeBlock.immData_16Bit);
}

void ins_method_PUSH_DS()
{
	ins_atomMethod_PUSH(ccpu.ds);
}
void ins_method_POP_DS()
{
	ccpu.ds = ins_atomMethod_POP();
}

void ins_method_SUB_Eb_Gb()//tested
{
	ins_atomMethod_SUB_8bit(p_nubit8(ccpu.deCodeBlock.prm), d_nubit8(ccpu.deCodeBlock.preg));
}
void ins_method_SUB_Ev_Gv()//tested
{
	ins_atomMethod_SUB_16bit(p_nubit16(ccpu.deCodeBlock.prm), d_nubit16(ccpu.deCodeBlock.preg));
}
void ins_method_SUB_Gb_Eb()//tested
{
	ins_atomMethod_SUB_8bit(p_nubit8(ccpu.deCodeBlock.preg), d_nubit8(ccpu.deCodeBlock.prm));
}
void ins_method_SUB_Gv_Ev()//tested
{
	ins_atomMethod_SUB_16bit(p_nubit16(ccpu.deCodeBlock.preg), d_nubit16(ccpu.deCodeBlock.prm));
}

void ins_method_SUB_AL_Ib()//tested
{
	ins_atomMethod_SUB_8bit(&ccpu.al, ccpu.deCodeBlock.immData_8Bit);
}
void ins_method_SUB_eAX_Iv()//tested
{
	ins_atomMethod_SUB_16bit(&ccpu.ax, ccpu.deCodeBlock.immData_16Bit);
}

void ins_method_SEG_CS()
{
	ccpu.deCodeBlock.prefix_SegmentOverrideOpCode = CS_SEGMENT_OVERRIDE_PREFIX;
}
void ins_method_DAS()//0x2f
{
	t_nubit8 old_al = ccpu.al;
	t_bool old_CF = ccpu_getCF_Flag();

	ccpu_setCF_Flag_flag(0x00);
	if((ccpu.al & 0x0f)>9 || (0x01 ==ccpu_getAF_Flag()))
	{
		ins_atomMethod_SUB_8bit(&ccpu.al, 6);
		ccpu_setCF_Flag_flag(old_CF); //这里可能会有点问题
		ccpu_setAF_Flag_flag(0x01);
	}
	else
	{
		ccpu_setAF_Flag_flag(0x00);
	}
    //*****
	if((old_al>0x99) || (0x01 == old_CF))
	{
		ins_atomMethod_SUB_8bit(&ccpu.al, 0x60);
		ccpu_setCF_Flag_flag(0x01);

	}
	else
	{
		ccpu_setCF_Flag_flag(0x00);//可能有点问题
	}
}

void ins_method_CMP_Eb_Gb()
{
	ins_atomMethod_CMP_8bit(d_nubit8(ccpu.deCodeBlock.prm), d_nubit8(ccpu.deCodeBlock.preg));
}
void ins_method_CMP_Ev_Gv()
{
	ins_atomMethod_CMP_16bit(d_nubit16(ccpu.deCodeBlock.prm), d_nubit16(ccpu.deCodeBlock.preg));
}
void ins_method_CMP_Gb_Eb()
{
	ins_atomMethod_CMP_8bit(d_nubit8(ccpu.deCodeBlock.preg), d_nubit8(ccpu.deCodeBlock.prm));
}
void ins_method_CMP_Gv_Ev()
{
	ins_atomMethod_CMP_16bit(d_nubit16(ccpu.deCodeBlock.preg), d_nubit16(ccpu.deCodeBlock.prm));
}
void ins_method_CMP_AL_Ib()
{
	ins_atomMethod_CMP_8bit(ccpu.al, ccpu.deCodeBlock.immData_8Bit);
}
void ins_method_CMP_eAX_Iv()
{
	ins_atomMethod_CMP_16bit(ccpu.ax, ccpu.deCodeBlock.immData_16Bit);
}

void ins_method_SEG_DS()
{
	ccpu.deCodeBlock.prefix_SegmentOverrideOpCode = DS_SEGMENT_OVERRIDE_PREFIX;
}
void ins_method_AAS()
{
	if((ccpu.al & 0x0f) > 9 || 0x01==ccpu_getAF_Flag())
	{
		ccpu.al -= 6;
		ccpu.ah--;
		ccpu_setAF_Flag_flag(0x01);
		ccpu_setCF_Flag_flag(0x01);
		ccpu.al &= 0x0f;
	}
	else
	{
		ccpu_setAF_Flag_flag(0x00);
		ccpu_setCF_Flag_flag(0x00);
		ccpu.al &= 0x0f;

	}
}

void ins_method_DEC_eAX()
{	
	ins_atomMethod_DEC_16bit(&ccpu.ax);
}
void ins_method_DEC_eCX()
{
	ins_atomMethod_DEC_16bit(&ccpu.cx);
}
void ins_method_DEC_eDX()
{
	ins_atomMethod_DEC_16bit(&ccpu.dx);
}
void ins_method_DEC_eBX()
{
	ins_atomMethod_DEC_16bit(&ccpu.bx);
}
void ins_method_DEC_eSP()
{
	ins_atomMethod_DEC_16bit(&ccpu.sp);
}
void ins_method_DEC_eBP()
{
	ins_atomMethod_DEC_16bit(&ccpu.bp);
}
void ins_method_DEC_eSI()
{
	ins_atomMethod_DEC_16bit(&ccpu.si);
}
void ins_method_DEC_eDI()
{
	ins_atomMethod_DEC_16bit(&ccpu.di);
}

void ins_method_POP_eAX()
{
	ccpu.ax = ins_atomMethod_POP();
}
void ins_method_POP_eCX()
{
	ccpu.cx = ins_atomMethod_POP();
}
void ins_method_POP_eDX()
{
	ccpu.dx = ins_atomMethod_POP();
}
void ins_method_POP_eBX()
{
	ccpu.bx = ins_atomMethod_POP();
}
void ins_method_POP_eSP()
{
	ccpu.sp = ins_atomMethod_POP();
}
void ins_method_POP_eBP()
{
	ccpu.bp = ins_atomMethod_POP();
}
void ins_method_POP_eSI()
{
	ccpu.si = ins_atomMethod_POP();
}
void ins_method_POP_eDI()
{
	ccpu.di = ins_atomMethod_POP();
}

void ins_method_PUSH_Iv()//0x68  16位中没有，不管了，先做了
{
	ins_atomMethod_PUSH(ccpu.deCodeBlock.opContactData_16bit);//估计是这样。。具体还看解码
}
void ins_method_IMUL_Gv_Ev_Iv(){/*16位机没有这个指令*/}
void ins_method_PUSH_Ib()//0x6a
{
	ins_atomMethod_PUSH(ccpu.deCodeBlock.opContactData_8bit);//估计是这样。。具体还看解码
}
void ins_method_IMUL_Gv_Ev_Ib(){/*16位机没有这个指令*/}
void ins_method_INSB_Yb_DX(){/*16位机没有这个指令*/}
void ins_method_INSW_Yv_DX(){/*16位机没有这个指令*/}
void ins_method_OUTSB_DX_Xb(){}//无效
void ins_method_OUTSW_DX_Xv(){}//无效

void ins_method_ShortJumpCondition_JS()
{
	if(0x01 == ccpu_getSF_Flag())
	{
		ins_atomMethod_shortJMP();
	}
}
void ins_method_ShortJumpCondition_JNS()
{
	if(0x00 == ccpu_getSF_Flag())
	{
		ins_atomMethod_shortJMP();
	}
}
void ins_method_ShortJumpCondition_JP()
{
	if(0x01 == ccpu_getPF_Flag())
	{
		ins_atomMethod_shortJMP();
	}
}
void ins_method_ShortJumpCondition_JNP()
{
	if(0x00 == ccpu_getPF_Flag())
	{
		ins_atomMethod_shortJMP();
	}
}
void ins_method_ShortJumpCondition_JL()
{
	if(ccpu_getSF_Flag() != ccpu_getOF_Flag())
	{
		ins_atomMethod_shortJMP();
	}
}
void ins_method_ShortJumpCondition_JNL()
{
	if(ccpu_getSF_Flag() == ccpu_getOF_Flag())
	{
		ins_atomMethod_shortJMP();
	}
}
void ins_method_ShortJumpCondition_JLE()
{
	if((0x01 == ccpu_getZF_Flag()) || (ccpu_getSF_Flag() != ccpu_getOF_Flag()))
	{
		ins_atomMethod_shortJMP();
	}
}
void ins_method_ShortJumpCondition_JNLE()
{
	if((0x00 == ccpu_getZF_Flag()) && (ccpu_getSF_Flag() == ccpu_getOF_Flag()))
	{
		ins_atomMethod_shortJMP();
	}
}

void ins_method_MOV_Eb_Gb()//tested
{
	d_nubit8(ccpu.deCodeBlock.prm) = d_nubit8(ccpu.deCodeBlock.preg);
}
void ins_method_MOV_Ev_Gv()//tested
{
	d_nubit16(ccpu.deCodeBlock.prm) = d_nubit16(ccpu.deCodeBlock.preg);
}
void ins_method_MOV_Gb_Eb()//tested
{
	d_nubit8(ccpu.deCodeBlock.preg) = d_nubit8(ccpu.deCodeBlock.prm);
}
void ins_method_MOV_Gv_Ev()//tested
{
	d_nubit16(ccpu.deCodeBlock.preg) = d_nubit16(ccpu.deCodeBlock.prm);
}
void ins_method_MOV_Ew_Sw()//tested
{
	d_nubit16(ccpu.deCodeBlock.prm) = d_nubit16(ccpu.deCodeBlock.pseg);
}

void ins_method_LEA_Gv_M()
{
	d_nubit16(ccpu.deCodeBlock.preg) = (t_nubit16)((t_nubit8 *)(ccpu.deCodeBlock.prm) - p_nubit8(vramGetAddr(0,0)) - (ccpu.deCodeBlock.segData<<4));
}

void ins_method_MOV_Sw_Ew()//tested
{
	d_nubit16(ccpu.deCodeBlock.pseg) = d_nubit16(ccpu.deCodeBlock.prm);
}
void ins_method_POP_Ev()
{
	d_nubit16(ccpu.deCodeBlock.prm) = ins_atomMethod_POP();
}

void ins_method_CBW()//0x98
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
void ins_method_CWD() //0x99
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
void ins_method_CALL_aP()//0x9a 段间直接调用
{
	ins_atomMethod_PUSH(ccpu.cs);
	ins_atomMethod_PUSH(ccpu.ip); // 没有PUSH IP 。。所以用这个来代替
	
	ccpu.cs = ccpu.deCodeBlock.opContactData_16bitE;
	ccpu.ip = ccpu.deCodeBlock.opContactData_16bit;
}
void ins_method_WAIT(){}//还不清楚
void ins_method_PUSHF_Fv()
{
	ins_atomMethod_PUSH(ccpu_generateFLAG());
}
void ins_method_POP_Fv()//0x8d  POPF 将标志弹出
{
	ccpu_setFLAG(ins_atomMethod_POP());
}
void ins_method_SAHF()//0x9e
{
	ccpu_set_SF_ZF_AF_PF_CF_Flag((t_nubit16)(ccpu.ah));
}
void ins_method_LAHF()
{
	ccpu.ah = (t_nubit8)ccpu_generateFLAG();
}

void ins_method_TEST_AL_Ib()
{
	ins_atomMethod_TEST_8bit(&ccpu.al, ccpu.deCodeBlock.immData_8Bit);
}
void ins_method_TEST_eAX_Iv()
{
	ins_atomMethod_TEST_16bit(&ccpu.ax, ccpu.deCodeBlock.immData_16Bit);
}
void ins_method_STOSB_Yb_AL()//0xaa
{
	if(ccpu.deCodeBlock.prefix_RepeatOpCode == REPNE_PREFIX ||
		ccpu.deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		ccpu.exeCodeBlock.repeatMethod = &ins_atomMethod_STOSB;
		ins_chooseRepeatExeStringMethod();
		return;
	}
	else
		ins_atomMethod_STOSB();
}
void ins_method_STOSW_Yv_eAX()
{
	if(ccpu.deCodeBlock.prefix_RepeatOpCode == REPNE_PREFIX ||
		ccpu.deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		ccpu.exeCodeBlock.repeatMethod = &ins_atomMethod_STOSW;
		ins_chooseRepeatExeStringMethod();
		return;
	}
	else
		ins_atomMethod_STOSW();
}
void ins_method_LODSB_AL_Xb()
{
	if(ccpu.deCodeBlock.prefix_RepeatOpCode == REPNE_PREFIX ||
		ccpu.deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		ccpu.exeCodeBlock.repeatMethod = &ins_atomMethod_LODSB;
		ins_chooseRepeatExeStringMethod();
		return;
	}
	else
		ins_atomMethod_LODSB();
}
void ins_method_LODSW_eAX_Xv()
{
	if(ccpu.deCodeBlock.prefix_RepeatOpCode == REPNE_PREFIX ||
		ccpu.deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		ccpu.exeCodeBlock.repeatMethod = &ins_atomMethod_LODSW;
		ins_chooseRepeatExeStringMethod();
		return;
	}
	else
		ins_atomMethod_LODSW();
}
void ins_method_SCASB_AL_Yb()
{
	if(ccpu.deCodeBlock.prefix_RepeatOpCode == REPNE_PREFIX ||
		ccpu.deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		ccpu.exeCodeBlock.repeatMethod = &ins_atomMethod_SCASB;
		ins_chooseRepeatExeStringMethod();
		return;
	}
	else
		ins_atomMethod_SCASB();
}
void ins_method_SCASW_eAX_Yv()
{
	if(ccpu.deCodeBlock.prefix_RepeatOpCode == REPNE_PREFIX ||
		ccpu.deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		ccpu.exeCodeBlock.repeatMethod = &ins_atomMethod_SCASW;
		ins_chooseRepeatExeStringMethod();
		return;
	}
	else
		ins_atomMethod_SCASW();
}

void ins_method_MOV_eAX()//tested
{
	ccpu.ax = ccpu.deCodeBlock.immData_16Bit;
}
void ins_method_MOV_eCX()//tested
{
	ccpu.cx = ccpu.deCodeBlock.immData_16Bit;
}
void ins_method_MOV_eDX()//tested
{
	ccpu.dx =ccpu.deCodeBlock.immData_16Bit;
}
void ins_method_MOV_eBX()//tested
{
	ccpu.bx = ccpu.deCodeBlock.immData_16Bit;
}
void ins_method_MOV_eSP()//tested
{
	ccpu.sp = ccpu.deCodeBlock.immData_16Bit;
}
void ins_method_MOV_eBP()//tested
{
	ccpu.bp =ccpu.deCodeBlock.immData_16Bit;
}
void ins_method_MOV_eSI()//tested
{
	ccpu.si = ccpu.deCodeBlock.immData_16Bit;
}
void ins_method_MOV_eDI()//tested
{
	ccpu.di = ccpu.deCodeBlock.immData_16Bit;
}

void ins_method_ENTER_Iw_Ib(){/*十六位机子没这个指令*/}
void ins_method_LEAVE(){/*十六位机子没这个指令*/}
void ins_method_RET_far_Iw()
{
	ccpu.ip = ins_atomMethod_POP();
	ccpu.cs = ins_atomMethod_POP();
	ccpu.sp += ccpu.deCodeBlock.opContactData_16bit;
}
void ins_method_RET_far()
{
	ccpu.ip = ins_atomMethod_POP();
	ccpu.cs = ins_atomMethod_POP();
}
void ins_method_INT_3()
{//暂时没有
}
void ins_method_INT_Ib()
{
	t_nubit8 intid = ccpu.deCodeBlock.opContactData_8bit;
// NEKOLOG
	switch (intid) {
	case 0x10:
	case 0x13:
	case 0x15:
	case 0x16:
	case 0x1a:
	case 0x11:
	case 0x12:
	case 0x14:
	case 0x17:
		ccpu.flagignore = 0x01;
		break;
	default: ins_atomMethod_INT(intid);
	}
// NEKOLOG
//	if (qdbiosExecInt(INT_num)) return;
//	else ins_atomMethod_INT(intid);
}
void ins_method_INTO()
{
	if(ccpu_getOF_Flag())
	{//OF为1 时执行4号中断
		ins_atomMethod_INT(4);
	}
}
void ins_method_IRET()
{
	ccpu.ip = ins_atomMethod_POP();
	ccpu.cs = ins_atomMethod_POP();
	ccpu_setFLAG(ins_atomMethod_POP());
}

void ins_method_ESC(){}//还不清楚

void ins_method_CALL_Jv()//0xe8 段内直接调用
{
	ins_atomMethod_PUSH(ccpu.ip); // 没有PUSH IP 。。所以用这个来代替
	ccpu.ip += (t_nsbit16)(ccpu.deCodeBlock.opContactData_16bit);
}
void ins_method_JMP_Jv()//段内直接转移之一
{
	ccpu.ip += (t_nsbit16)(ccpu.deCodeBlock.opContactData_16bit);
}
void ins_method_JMP_Ap()//段间直接转移
{
	ccpu.ip = ccpu.deCodeBlock.opContactData_16bit;
	ccpu.cs = ccpu.deCodeBlock.opContactData_16bitE;
}
void ins_method_JMP_Jb()//段内直接转移之二
{
	ins_atomMethod_shortJMP();
}
void ins_method_IN_AL_DX() {ccpu.flagignore = 0x01;}//暂时没有
void ins_method_IN_eAX_DX() {ccpu.flagignore = 0x01;}//暂时没有
void ins_method_OUT_DX_AL() {ccpu.flagignore = 0x01;}//暂时没有
void ins_method_OUT_DX_eAX() {ccpu.flagignore = 0x01;}//暂时没有

void ins_method_CLC()//0xf8
{
	ccpu_setCF_Flag_flag(0x00);
}
void ins_method_STC()
{
	ccpu_setCF_Flag_flag(0x01);
}
void ins_method_CLI()//0xfa
{
	ccpu_setIF_Flag_flag(0x00);
}
void ins_method_STI()
{
	ccpu_setIF_Flag_flag(0x01);
}
void ins_method_CLD()//0xfc
{
	ccpu_setDF_Flag_flag(0x00);
}
void ins_method_STD()
{
	ccpu_setDF_Flag_flag(0x01);
}
void ins_method_INC_Group4()
{
	switch(ccpu.deCodeBlock.nnn)
	{
	case 0://nnn=000
		ins_methodGroup_INC_Eb();
		break;
	case 1://nnn=001
		ins_methodGroup_DEC_Eb();
		break;
	default:
		vapiPrint("ins_method_INC_Group4 UnHandle the nnn bits");
		assert(0x00);
		break;
	}
}
void ins_method_INC_Group5()
{
	switch(ccpu.deCodeBlock.nnn)
	{
	case 0://nnn=000
		ins_methodGroup_INC_Ev();
		break;
	case 1://nnn=001
		ins_methodGroup_DEC_Ev();
		break;
	case 2://nnn=010
		ins_methodGroup_CALL_Ev();
		break;
	case 3://nnn=011
		ins_methodGroup_CALL_Ep();
		break;
	case 4://nnn=100
		ins_methodGroup_JMP_Ev();
		break;
	case 5://nnn=101
		ins_methodGroup_JMP_Ep();
		break;
	case 6://nnn=110
		ins_methodGroup_PUSH_Ev();
		return;
	case 7://nnn=111
		vapiPrint("ins_method_INC_Group5 UnHandle the nnn bits");
		assert(0x00);
		break;
	}
}

void ins_chooseRepeatExeStringMethod()
{

	if(ccpu.exeCodeBlock.repeatMethod == &ins_atomMethod_MOVSB ||
		ccpu.exeCodeBlock.repeatMethod == &ins_atomMethod_MOVSW ||
		ccpu.exeCodeBlock.repeatMethod == &ins_atomMethod_LODSB ||
		ccpu.exeCodeBlock.repeatMethod == &ins_atomMethod_LODSW ||
		ccpu.exeCodeBlock.repeatMethod == &ins_atomMethod_STOSB ||
		ccpu.exeCodeBlock.repeatMethod == &ins_atomMethod_STOSW ||
		ccpu.exeCodeBlock.repeatMethod == &ins_atomMethod_INS   ||
		ccpu.exeCodeBlock.repeatMethod == &ins_atomMethod_OUTS )
	{
		ins_self_REP_StringIns();
	} else {
		if(ccpu.deCodeBlock.prefix_RepeatOpCode == REPE_PREFIX) //用于区分REPNE 和REPE 。。。。
			ins_self_REPE_StringIns();  //除此则都和REPE 
		else
			ins_self_REPNE_StringIns();	
	}
	
}

void ins_self_REP_StringIns()
{
	while(0 != ccpu.cx)
	{
		(*(ccpu.exeCodeBlock.repeatMethod))();
		ccpu.cx--;
	}
}
void ins_self_REPE_StringIns()
{
	while(0 != ccpu.cx)
	{
		(*(ccpu.exeCodeBlock.repeatMethod))();
		ccpu.cx--;
		if(0x00 == ccpu_getZF_Flag())
			break;
	}
}
void ins_self_REPNE_StringIns()
{
	while(0 != ccpu.cx)
	{
		(*(ccpu.exeCodeBlock.repeatMethod))();
		ccpu.cx--;
		if(0x01 == ccpu_getZF_Flag())
			break;
	}
}

void ins_method_Unknow() {vapiPrint("unkonwnMethod");}
