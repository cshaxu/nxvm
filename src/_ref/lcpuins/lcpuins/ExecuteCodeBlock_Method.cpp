#include "../vmachine.h"
#include "../vapi.h"
#include "../lcpuins.h"

void ExecuteCodeBlock::method_ADD_Eb_Gb()
{
	this->atomMethod_ADD_8bit(d_nubit8(pdeCodeBlock->prm), d_nubit8(pdeCodeBlock->preg));
}
void ExecuteCodeBlock::method_ADD_Ev_Gv()
{
	this->atomMethod_ADD_16bit(d_nubit16(pdeCodeBlock->prm), d_nubit16(pdeCodeBlock->preg));
}
void ExecuteCodeBlock::method_ADD_Gb_Eb()
{
	this->atomMethod_ADD_8bit(d_nubit8(pdeCodeBlock->preg), d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::method_ADD_Gv_Ev()
{
	this->atomMethod_ADD_16bit(d_nubit16(pdeCodeBlock->preg), d_nubit16(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::method_ADD_AL_Ib()
{
	this->atomMethod_ADD_8bit(lcpu.al, d_nubit8(&pdeCodeBlock->immData_8Bit));
}
void ExecuteCodeBlock::method_ADD_eAX_Iv()
{
	this->atomMethod_ADD_16bit(lcpu.ax, d_nubit16(&pdeCodeBlock->immData_16Bit));
}

void ExecuteCodeBlock::method_PUSH_ES()//0x06
{
	this->atomMethod_PUSH(pCPU->es);
}
void ExecuteCodeBlock::method_POP_ES()
{
	pCPU->es = this->atomMethod_POP();
}


void ExecuteCodeBlock::method_ADC_Eb_Gb()
{
	this->atomMethod_ADC_8bit(d_nubit8(pdeCodeBlock->prm), d_nubit8(pdeCodeBlock->preg));
}
void ExecuteCodeBlock::method_ADC_Ev_Gv()
{
	this->atomMethod_ADC_16bit(d_nubit16(pdeCodeBlock->prm), d_nubit16(pdeCodeBlock->preg));
}
void ExecuteCodeBlock::method_ADC_Gb_Eb()
{
	this->atomMethod_ADC_8bit(d_nubit8(pdeCodeBlock->preg), d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::method_ADC_Gv_Ev()
{
	this->atomMethod_ADC_16bit(d_nubit16(pdeCodeBlock->preg), d_nubit16(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::method_ADC_AL_Ib()
{
	this->atomMethod_ADC_8bit(pCPU->al, pdeCodeBlock->immData_8Bit);
}
void ExecuteCodeBlock::method_ADC_eAX_Iv()
{
	this->atomMethod_ADC_16bit(pCPU->ax, pdeCodeBlock->immData_16Bit);
}

void ExecuteCodeBlock::method_PUSH_SS()
{
	this->atomMethod_PUSH(pCPU->ss);
}
void ExecuteCodeBlock::method_POP_SS()
{
	pCPU->ss = this->atomMethod_POP();
}

void ExecuteCodeBlock::method_AND_Eb_Gb()
{
	this->atomMethod_AND_8bit(d_nubit8(pdeCodeBlock->prm), d_nubit8(pdeCodeBlock->preg));
}
void ExecuteCodeBlock::method_AND_Ev_Gv()
{
	this->atomMethod_AND_16bit(d_nubit16(pdeCodeBlock->prm), d_nubit16(pdeCodeBlock->preg));
}
void ExecuteCodeBlock::method_AND_Gb_Eb()
{
	this->atomMethod_AND_8bit(d_nubit8(pdeCodeBlock->preg), d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::method_AND_Gv_Ev()
{
	this->atomMethod_AND_16bit(d_nubit16(pdeCodeBlock->preg), d_nubit16(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::method_AND_AL_Ib()
{
	this->atomMethod_AND_8bit(pCPU->al, pdeCodeBlock->immData_8Bit);
}
void ExecuteCodeBlock::method_AND_eAX_Iv()
{
	this->atomMethod_AND_16bit(pCPU->ax, pdeCodeBlock->immData_16Bit);
}

void ExecuteCodeBlock::method_SEG_ES()
{
	pdeCodeBlock->prefix_SegmentOverrideOpCode = ES_SEGMENT_OVERRIDE_PREFIX;
}
void ExecuteCodeBlock::method_DAA()//0x27
{
	t_nubit8 old_al = pCPU->al;
	bool old_CF = pCPU->getCF_Flag();

	pCPU->setCF_Flag(false);
	if((pCPU->al & 0x0f)>9 || (true ==pCPU->getAF_Flag()))
	{
		this->atomMethod_ADD_8bit(pCPU->al, 6);
		pCPU->setCF_Flag(old_CF); //这里可能会有点问题
		pCPU->setAF_Flag(true);
	}
	else
	{
		pCPU->setAF_Flag(false);
	}
    //*****
	if((old_al>0x99) || (true == old_CF))
	{
		this->atomMethod_ADD_8bit(pCPU->al, 0x60);
		pCPU->setCF_Flag(true);

	}
	else
	{
		pCPU->setCF_Flag(false);
	}

}

void ExecuteCodeBlock::method_XOR_Eb_Gb()
{
	this->atomMethod_XOR_8bit(d_nubit8(pdeCodeBlock->prm), d_nubit8(pdeCodeBlock->preg));
}
void ExecuteCodeBlock::method_XOR_Ev_Gv()
{
	this->atomMethod_XOR_16bit(d_nubit16(pdeCodeBlock->prm), d_nubit16(pdeCodeBlock->preg));
}
void ExecuteCodeBlock::method_XOR_Gb_Eb()
{
	this->atomMethod_XOR_8bit(d_nubit8(pdeCodeBlock->preg), d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::method_XOR_Gv_Ev()
{
	this->atomMethod_XOR_16bit(d_nubit16(pdeCodeBlock->preg), d_nubit16(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::method_XOR_AL_Ib()
{
	this->atomMethod_XOR_8bit(pCPU->al, pdeCodeBlock->immData_8Bit);
}
void ExecuteCodeBlock::method_XOR_eAX_Iv()
{
	this->atomMethod_XOR_16bit(pCPU->ax, pdeCodeBlock->immData_16Bit);
}

void ExecuteCodeBlock::method_SEG_SS()
{
	pdeCodeBlock->prefix_SegmentOverrideOpCode = SS_SEGMENT_OVERRIDE_PREFIX;
}
void ExecuteCodeBlock::method_AAA()
{
	if((pCPU->al & 0x0f) > 9 || true==pCPU->getAF_Flag())
	{
		pCPU->al += 6;
		pCPU->ah++;
		pCPU->setAF_Flag(true);
		pCPU->setCF_Flag(true);
		pCPU->al &= 0x0f;
	}
	else
	{
		pCPU->setAF_Flag(false);
		pCPU->setCF_Flag(false);
		pCPU->al &= 0x0f;

	}
}

void ExecuteCodeBlock::method_INC_eAX()
{
	this->atomMethod_INC_16bit(pCPU->ax);
}
void ExecuteCodeBlock::method_INC_eCX()
{
	this->atomMethod_INC_16bit(pCPU->cx);
}
void ExecuteCodeBlock::method_INC_eDX()
{
	this->atomMethod_INC_16bit(pCPU->dx);
}
void ExecuteCodeBlock::method_INC_eBX()
{
	this->atomMethod_INC_16bit(pCPU->bx);
}
void ExecuteCodeBlock::method_INC_eSP()
{
	this->atomMethod_INC_16bit(pCPU->sp);
}
void ExecuteCodeBlock::method_INC_eBP()
{
	this->atomMethod_INC_16bit(pCPU->bp);
}
void ExecuteCodeBlock::method_INC_eSI()
{
	this->atomMethod_INC_16bit(pCPU->si);
}
void ExecuteCodeBlock::method_INC_eDI()
{
	this->atomMethod_INC_16bit(pCPU->di);
}

void ExecuteCodeBlock::method_PUSH_eAX()//0x50
{
	this->atomMethod_PUSH(pCPU->ax);
}
void ExecuteCodeBlock::method_PUSH_eCX()
{
	this->atomMethod_PUSH(pCPU->cx);
}
void ExecuteCodeBlock::method_PUSH_eDX()
{
	this->atomMethod_PUSH(pCPU->dx);
}
void ExecuteCodeBlock::method_PUSH_eBX()
{
	this->atomMethod_PUSH(pCPU->bx);
}
void ExecuteCodeBlock::method_PUSH_eSP()
{
	this->atomMethod_PUSH(pCPU->sp);
}
void ExecuteCodeBlock::method_PUSH_eBP()
{
	this->atomMethod_PUSH(pCPU->bp);
}
void ExecuteCodeBlock::method_PUSH_eSI()
{
	this->atomMethod_PUSH(pCPU->si);
}
void ExecuteCodeBlock::method_PUSH_eDI()
{
	this->atomMethod_PUSH(pCPU->di);
}

void ExecuteCodeBlock::method_PUSHA_PUSHAD(){/*do nothing*/}//80x86没有
void ExecuteCodeBlock::method_POPA_POPAD(){/*do nothing*/}//80x86没有
void ExecuteCodeBlock::method_BOUND_Gv_Ma(){/*do nothing*/}//80x86没有
void ExecuteCodeBlock::method_ARPL_Ew_Gw(){/*do nothing*/}//80x86没有
void ExecuteCodeBlock::method_SEG_FS(){/*do nothing*/}
void ExecuteCodeBlock::method_SEG_GS(){/*do nothing*/}
void ExecuteCodeBlock::method_Operand_Size(){/*do nothing*/}//80x86没有
void ExecuteCodeBlock::method_Address_Size(){/*do nothing*/}//80x86没有

void ExecuteCodeBlock::method_ShortJump_JO()//0x70
{
	if(true == pCPU->getOF_Flag())
	{
		atomMethod_shortJMP();
	}
}
void ExecuteCodeBlock::method_ShortJump_JNO()//0x71
{
	if(false == pCPU->getOF_Flag())
	{
		atomMethod_shortJMP();
	}
}
void ExecuteCodeBlock::method_ShortJump_JB()//0x72
{
	if(true == pCPU->getCF_Flag())
	{
		atomMethod_shortJMP();
	}
}
void ExecuteCodeBlock::method_ShortJump_JNB()//0x73
{
	if(false == pCPU->getCF_Flag())
	{
		atomMethod_shortJMP();
	}
}
void ExecuteCodeBlock::method_ShortJump_JZ()//0x74
{
	if(true == pCPU->getZF_Flag())
	{
		atomMethod_shortJMP();
	}
}
void ExecuteCodeBlock::method_ShortJump_JNZ()//0x75
{
	if(false == pCPU->getZF_Flag())
	{
		atomMethod_shortJMP();
	}
}
void ExecuteCodeBlock::method_ShortJump_JBE()//0x76
{
	if((true == pCPU->getCF_Flag()) || (true == pCPU->getZF_Flag()))
	{
		atomMethod_shortJMP();
	}
}
void ExecuteCodeBlock::method_ShortJump_JNBE()//0
{
	if(false == pCPU->getCF_Flag() && false == pCPU->getZF_Flag())
	{
		atomMethod_shortJMP();
	}

}

void ExecuteCodeBlock::method_IMMGroup_Eb_Ib()
{
	switch(this->pdeCodeBlock->nnn)
	{
	case 0:
			methodGroup_ADD_Eb_Ib();
			break;
	case 1:
			methodGroup_OR_Eb_Ib();
			break;
	case 2:
		methodGroup_ADC_Eb_Ib();
		break;
	case 3:
		methodGroup_SBB_Eb_Ib();
		break;
	case 4:
		methodGroup_AND_Eb_Ib();
		break;
	case 5:
		methodGroup_SUB_Eb_Ib();
		break;
	case 6:
		methodGroup_XOR_Eb_Ib();
		break;
	case 7:
		methodGroup_CMP_Eb_Ib();
		break;
	}
}
void ExecuteCodeBlock::method_IMMGroup_Ev_Iz()
{
	switch(this->pdeCodeBlock->nnn)
	{
	case 0:
			methodGroup_ADD_Ev_Iv();
			break;
	case 1:
			methodGroup_OR_Ev_Iv();
			break;
	case 2:
		methodGroup_ADC_Ev_Iv();
		break;
	case 3:
		methodGroup_SBB_Ev_Iv();
		break;
	case 4:
		methodGroup_AND_Ev_Iv();
		break;
	case 5:
		methodGroup_SUB_Ev_Iv();
		break;
	case 6:
		methodGroup_XOR_Ev_Iv();
		break;
	case 7:
		methodGroup_CMP_Ev_Iv();
		break;
	}
}
void ExecuteCodeBlock::method_IMMGroup_Eb_IbX()
{
	method_IMMGroup_Eb_Ib();
}
void ExecuteCodeBlock::method_IMMGroup_Ev_Ib()
{
	switch(this->pdeCodeBlock->nnn)
	{
	case 0:
			methodGroup_ADD_Ev_Iv();
			break;
	case 1:
			methodGroup_OR_Ev_Iv();
			break;
	case 2:
		methodGroup_ADC_Ev_Iv();
		break;
	case 3:
		methodGroup_SBB_Ev_Iv();
		break;
	case 4:
		methodGroup_AND_Ev_Iv();
		break;
	case 5:
		methodGroup_SUB_Ev_Iv();
		break;
	case 6:
		methodGroup_XOR_Ev_Iv();
		break;
	case 7:
		methodGroup_CMP_Ev_Iv();
		break;
	}
}

void ExecuteCodeBlock::method_TEST_Eb_Gb()//0x84
{
	t_nubit8 tmp = d_nubit8(pdeCodeBlock->prm);
	this->atomMethod_AND_8bit(tmp, d_nubit8(pdeCodeBlock->preg));
}
void ExecuteCodeBlock::method_TEST_Ev_Gv()
{
	t_nubit16 tmp = d_nubit16(pdeCodeBlock->prm);
	this->atomMethod_AND_16bit(tmp, d_nubit16(pdeCodeBlock->preg));
}

void ExecuteCodeBlock::method_XCHG_Eb_Gb()//0x86
{
	t_nubit8 tmp = d_nubit8(pdeCodeBlock->preg);
	d_nubit8(pdeCodeBlock->preg) = d_nubit8(pdeCodeBlock->prm);
	d_nubit8(pdeCodeBlock->prm)= tmp;
}
void ExecuteCodeBlock::method_XCHG_Ev_Gv()
{
	t_nubit16 tmp = d_nubit16(pdeCodeBlock->preg);
	d_nubit16(pdeCodeBlock->preg) = d_nubit16(pdeCodeBlock->prm);
	d_nubit16(pdeCodeBlock->prm)= tmp;
}

void ExecuteCodeBlock::method_NOP(){/* do nothing*/}

void ExecuteCodeBlock::method_XCHG_eCX()//0x91
{
	t_nubit16 tmp = pCPU->cx;
	pCPU->cx = pCPU->ax;
	pCPU->ax = tmp;
}
void ExecuteCodeBlock::method_XCHG_eDX()
{
	t_nubit16 tmp = pCPU->dx;
	pCPU->dx = pCPU->ax;
	pCPU->ax = tmp;
}
void ExecuteCodeBlock::method_XCHG_eBX()
{
	t_nubit16 tmp = pCPU->bx;
	pCPU->bx = pCPU->ax;
	pCPU->ax = tmp;
}
void ExecuteCodeBlock::method_XCHG_eSP()
{
	t_nubit16 tmp = pCPU->sp;
	pCPU->sp = pCPU->ax;
	pCPU->ax = tmp;
}
void ExecuteCodeBlock::method_XCHG_eBP()
{
	t_nubit16 tmp = pCPU->bp;
	pCPU->bp = pCPU->ax;
	pCPU->ax = tmp;
}
void ExecuteCodeBlock::method_XCHG_eSI()
{
	t_nubit16 tmp = pCPU->si;
	pCPU->si = pCPU->ax;
	pCPU->ax = tmp;
}
void ExecuteCodeBlock::method_XCHG_eDI()
{
	t_nubit16 tmp = pCPU->di;
	pCPU->di = pCPU->ax;
	pCPU->ax = tmp;
}

void ExecuteCodeBlock::method_MOV_AL_Ob()//tested
{
	lcpu.al = d_nubit8(this->pdeCodeBlock->prm);
}
void ExecuteCodeBlock::method_MOV_eAX_Ov()//tested
{
	lcpu.ax = d_nubit16(this->pdeCodeBlock->prm);
}
void ExecuteCodeBlock::method_MOV_Ob_AL()//tested
{
	d_nubit8(this->pdeCodeBlock->prm) = lcpu.al;
}
void ExecuteCodeBlock::method_MOV_Ov_eAX()//tested
{
	d_nubit16(this->pdeCodeBlock->prm) = lcpu.ax;
}

void ExecuteCodeBlock::method_MOVSB_Xb_Yb()
{
	if(pdeCodeBlock->prefix_RepeatOpCode == REPNE_PREFIX ||
		pdeCodeBlock->prefix_RepeatOpCode == REPE_PREFIX)
	{
		repeatMethod = &ExecuteCodeBlock::atomMethod_MOVSB;//绑定指令操作函数
		this->chooseRepeatExeStringMethod();//选择指令执行方式
		return;
	}
	else
		this->atomMethod_MOVSB();
}
void ExecuteCodeBlock::method_MOVSW_Xv_Yv()
{
	if(pdeCodeBlock->prefix_RepeatOpCode == REPNE_PREFIX ||
		pdeCodeBlock->prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		repeatMethod = &ExecuteCodeBlock::atomMethod_MOVSW;
		this->chooseRepeatExeStringMethod();
		return;
	}
	else
		this->atomMethod_MOVSW();
}
void ExecuteCodeBlock::method_CMPSB_Xb_Yb()//0xa6
{
	if(pdeCodeBlock->prefix_RepeatOpCode == REPNE_PREFIX ||
		pdeCodeBlock->prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		repeatMethod = &ExecuteCodeBlock::atomMethod_CMPSB;
		this->chooseRepeatExeStringMethod();
		return;
	}
	else
		this->atomMethod_CMPSB();
}
void ExecuteCodeBlock::method_CMPSW_Xv_Yv()//0xa7
{
	if(pdeCodeBlock->prefix_RepeatOpCode == REPNE_PREFIX ||
		pdeCodeBlock->prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进
		repeatMethod = &ExecuteCodeBlock::atomMethod_CMPSW;
		this->chooseRepeatExeStringMethod();
		return;
	}
	else
		this->atomMethod_CMPSW();
}

void ExecuteCodeBlock::method_MOV_AL()//tested
{
	lcpu.al = this->pdeCodeBlock->immData_8Bit;
}
void ExecuteCodeBlock::method_MOV_CL()//tested
{
	lcpu.cl = this->pdeCodeBlock->immData_8Bit;
}
void ExecuteCodeBlock::method_MOV_DL()//tested
{
	lcpu.dl = this->pdeCodeBlock->immData_8Bit;
}
void ExecuteCodeBlock::method_MOV_BL()//tested
{
	lcpu.bl = this->pdeCodeBlock->immData_8Bit;
}
void ExecuteCodeBlock::method_MOV_AH()//tested
{
	lcpu.ah =this->pdeCodeBlock->immData_8Bit;
}
void ExecuteCodeBlock::method_MOV_CH()//tested
{
	lcpu.ch =this->pdeCodeBlock->immData_8Bit;
}
void ExecuteCodeBlock::method_MOV_DH()//tested
{
	lcpu.dh = this->pdeCodeBlock->immData_8Bit;
}
void ExecuteCodeBlock::method_MOV_BH()//tested
{
	lcpu.bh =this->pdeCodeBlock->immData_8Bit;
}

void ExecuteCodeBlock::method_ShiftGroup_Eb_Ib(){/*do nothing*/}//80x86没有
void ExecuteCodeBlock::method_ShiftGroup_Ev_Ib(){/*do nothing*/}//80x86没有
void ExecuteCodeBlock::method_RET_near_Iw() //带立即数的段内返回
{
	pCPU->ip = this->atomMethod_POP();
	pCPU->sp += pdeCodeBlock->opContactData_16bit;
}
//that would be something
void ExecuteCodeBlock::method_RET_near_None()//段内返回
{
	pCPU->ip = this->atomMethod_POP();
}

void ExecuteCodeBlock::method_LES_Gv_Mp()
{
	d_nubit16(pdeCodeBlock->preg) = d_nubit16(pdeCodeBlock->prm);
	pCPU->es = d_nubit16((t_nubit8 *)(pdeCodeBlock->prm) + 2);
}
void ExecuteCodeBlock::method_LDS_Gv_Mp()
{
	d_nubit16(pdeCodeBlock->preg) = d_nubit16(pdeCodeBlock->prm);
	pCPU->ds = d_nubit16((t_nubit8 *)(pdeCodeBlock->prm) + 2);
}

void ExecuteCodeBlock::method_MOV_Eb_Ib()//tested
{
	d_nubit8(this->pdeCodeBlock->prm) = this->pdeCodeBlock->immData_8Bit;
}
void ExecuteCodeBlock::method_MOV_Ev_Iv()//tested
{
	d_nubit16(this->pdeCodeBlock->prm) = this->pdeCodeBlock->immData_16Bit;
}

void ExecuteCodeBlock::method_ShiftGroup2_Eb_1()
{
	switch(pdeCodeBlock->nnn)
	{
	case 0://nnn=000
		this->methodGroup_ROL_Eb_1();
		break;
	case 1://nnn=001
		this->methodGroup_ROR_Eb_1();
		break;
	case 2://nnn=010
		this->methodGroup_RCL_Eb_1();
		break;
	case 3://nnn=011
		this->methodGroup_RCR_Eb_1();
		break;
	case 4://nnn=100
		this->methodGroup_SHL_Eb_1();
		break;
	case 5://nnn=101
		this->methodGroup_SHR_Eb_1();
		break;
	case 6://nnn=110
		showMessage("method_ShiftGroup2_Eb_1 UnHandle the nnn bits");
		assert(false);
		return;
	case 7://nnn=111
		this->methodGroup_SAR_Eb_1();
		break;
	}
}
void ExecuteCodeBlock::method_ShiftGroup2_Ev_1()
{
	switch(pdeCodeBlock->nnn)
	{
	case 0://nnn=000
		this->methodGroup_ROL_Ev_1();
		break;
	case 1://nnn=001
		this->methodGroup_ROR_Ev_1();
		break;
	case 2://nnn=010
		this->methodGroup_RCL_Ev_1();
		break;
	case 3://nnn=011
		this->methodGroup_RCR_Ev_1();
		break;
	case 4://nnn=100
		this->methodGroup_SHL_Ev_1();
		break;
	case 5://nnn=101
		this->methodGroup_SHR_Ev_1();
		break;
	case 6://nnn=110
		showMessage("method_ShiftGroup2_Ev_1 UnHandle the nnn bits");
		assert(false);
		return;
	case 7://nnn=111
		this->methodGroup_SAR_Ev_1();
		break;
	}
}
void ExecuteCodeBlock::method_ShiftGroup2_Eb_CL()
{
	switch(pdeCodeBlock->nnn)
	{
	case 0://nnn=000
		this->methodGroup_ROL_Eb_CL();
		break;
	case 1://nnn=001
		this->methodGroup_ROR_Eb_CL();
		break;
	case 2://nnn=010
		this->methodGroup_RCL_Eb_CL();
		break;
	case 3://nnn=011
		this->methodGroup_RCR_Eb_CL();
		break;
	case 4://nnn=100
		this->methodGroup_SHL_Eb_CL();
		break;
	case 5://nnn=101
		this->methodGroup_SHR_Eb_CL();
		break;
	case 6://nnn=110
		showMessage("method_ShiftGroup2_Eb_CL UnHandle the nnn bits");
		assert(false);
		return;
	case 7://nnn=111
		this->methodGroup_SAR_Eb_CL();
		break;
	}
}
void ExecuteCodeBlock::method_ShiftGroup2_Ev_CL()
{
	switch(pdeCodeBlock->nnn)
	{
	case 0://nnn=000
		this->methodGroup_ROL_Ev_CL();
		break;
	case 1://nnn=001
		this->methodGroup_ROR_Ev_CL();
		break;
	case 2://nnn=010
		this->methodGroup_RCL_Ev_CL();
		break;
	case 3://nnn=011
		this->methodGroup_RCR_Ev_CL();
		break;
	case 4://nnn=100
		this->methodGroup_SHL_Ev_CL();
		break;
	case 5://nnn=101
		this->methodGroup_SHR_Ev_CL();
		break;
	case 6://nnn=110
		showMessage("method_ShiftGroup2_Ev_CL UnHandle the nnn bits");
		assert(false);
		return;
	case 7://nnn=111
		this->methodGroup_SAR_Ev_CL();
		break;
	}
}

void ExecuteCodeBlock::method_AMM()
{
	pCPU->ah = pCPU->al / 10;
	t_nubit8 operand1 = pCPU->al;
	pCPU->al %= 10;
	pCPU->storeCaculate(MOD_8bit, 8, operand1, 10, pCPU->al, MASK_FLAG_SF | MASK_FLAG_ZF | MASK_FLAG_PF);
}
void ExecuteCodeBlock::method_AAD()
{
	const bool tmpAF = pCPU->getAF_Flag();
	const bool tmpCF = pCPU->getCF_Flag();
	const bool tmpOF = pCPU->getOF_Flag();
	const t_nubit8 tmp = pCPU->ah * 10;
	this->atomMethod_ADD_8bit(pCPU->al,tmp);
	pCPU->ah = 0;
	pCPU->setAF_Flag(tmpAF);
	pCPU->setCF_Flag(tmpCF);
	pCPU->setOF_Flag(tmpOF);
}

void ExecuteCodeBlock::method_XLAT()
{
		pCPU->al = getMemData_byte(pCPU->al + pCPU->bx + (pdeCodeBlock->getDefaultSegment_DS() <<4));
}

void ExecuteCodeBlock::method_LOOPN_Jb()//0xe0
{
	if((0 != --(pCPU->cx)) && (false == pCPU->getZF_Flag()))
		this->atomMethod_shortJMP();
}
void ExecuteCodeBlock::method_LOOPE_Jb()
{
	if((0 != --(pCPU->cx)) && (true == pCPU->getZF_Flag()))
		this->atomMethod_shortJMP();
}
void ExecuteCodeBlock::method_LOOP_Jb()
{
	if(0 != --(pCPU->cx))
		this->atomMethod_shortJMP();
}
void ExecuteCodeBlock::method_JCXZ_Jb()
{
	if(0 == pCPU->cx )
		this->atomMethod_shortJMP();
}

void ExecuteCodeBlock::method_IN_AL_Ib()
{
	//ExecFun(vport.in[pdeCodeBlock->opContactData_8bit]);
	lcpu.iobyte = vcpu.iobyte;
	lcpu.al = _al;
}
void ExecuteCodeBlock::method_IN_eAX_Ib(){
	lcpu.iobyte = vcpu.iobyte;
	lcpu.ax = _ax;
}
void ExecuteCodeBlock::method_OUT_Ib_AL()
{
	lcpu.iobyte = lcpu.al;
// NEKO LOG
//	ExecFun(vport.out[pdeCodeBlock->opContactData_8bit]);
}
void ExecuteCodeBlock::method_OUT_Ib_eAX(){
	lcpu.iobyte = lcpu.al;
}

void ExecuteCodeBlock::method_LOCK(){/*在这个版本的虚拟机内，此指令什么都不做*/}
void ExecuteCodeBlock::method_REPNE()
{
	pdeCodeBlock->prefix_RepeatOpCode = REPNE_PREFIX;
}
void ExecuteCodeBlock::method_REP_REPE()
{
	pdeCodeBlock->prefix_RepeatOpCode = REP_PREFIX;
}
void ExecuteCodeBlock::method_HLT(){}//还不清楚
void ExecuteCodeBlock::method_CMC()//0xf5
{
	pCPU->setCF_Flag( !pCPU->getCF_Flag() );
}
void ExecuteCodeBlock::method_UnaryGroup_Eb()
{
	switch(pdeCodeBlock->nnn)
	{
	case 0://nnn=000
		this->methodGroup_TEST_Ib();
		break;
	case 1://nnn=001
		showMessage("method_UnaryGroup_Eb UnHandle the nnn bits");
		assert(false);
		break;
	case 2://nnn=010
		this->methodGroup_NOT_8bit();
		break;
	case 3://nnn=011
		this->methodGroup_NEG_8bit();
		break;
	case 4://nnn=100
		this->methodGroup_MUL_AL();
		break;
	case 5://nnn=101
		this->methodGroup_IMUL_AL();
		break;
	case 6://nnn=110
		this->methodGroup_DIV_AL();
		return;
	case 7://nnn=111
		this->methodGroup_IDIV_AL();
		break;
	}
}
void ExecuteCodeBlock::method_UnaryGroup_Ev()
{
	switch(pdeCodeBlock->nnn)
	{
	case 0://nnn=000
		this->methodGroup_TEST_Iv();
		break;
	case 1://nnn=001
		showMessage("method_UnaryGroup_Ev UnHandle the nnn bits");
		assert(false);
		break;
	case 2://nnn=010
		this->methodGroup_NOT_16bit();
		break;
	case 3://nnn=011
		this->methodGroup_NEG_16bit();
		break;
	case 4://nnn=100
		this->methodGroup_MUL_eAX();
		break;
	case 5://nnn=101
		this->methodGroup_IMUL_eAX();
		break;
	case 6://nnn=110
		this->methodGroup_DIV_eAX();
		return;
	case 7://nnn=111
		this->methodGroup_IDIV_eAX();
		break;
	}
}

void ExecuteCodeBlock::method_OR_Eb_Gb()
{
	this->atomMethod_OR_8bit(d_nubit8(pdeCodeBlock->prm), d_nubit8(pdeCodeBlock->preg));

}
void ExecuteCodeBlock::method_OR_Ev_Gv()
{
	this->atomMethod_OR_16bit(d_nubit16(pdeCodeBlock->prm), d_nubit16(pdeCodeBlock->preg));
}
void ExecuteCodeBlock::method_OR_Gb_Eb()
{
	this->atomMethod_OR_8bit(d_nubit8(pdeCodeBlock->preg), d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::method_OR_Gv_Ev()
{
	this->atomMethod_OR_16bit(d_nubit16(pdeCodeBlock->preg), d_nubit16(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::method_OR_AL_Ib()
{
	this->atomMethod_OR_8bit(pCPU->al, pdeCodeBlock->immData_8Bit);
}
void ExecuteCodeBlock::method_OR_eAX_Iv()
{
	this->atomMethod_OR_16bit(pCPU->ax, pdeCodeBlock->immData_16Bit);
}

void ExecuteCodeBlock::method_PUSH_CS()//0x0e
{
	this->atomMethod_PUSH(pCPU->cs);
}
void ExecuteCodeBlock::method_EscapeCode(){}

void ExecuteCodeBlock::method_SBB_Eb_Gb()//tested
{
	this->atomMethod_SBB_8bit(d_nubit8(pdeCodeBlock->prm), d_nubit8(pdeCodeBlock->preg));

}
void ExecuteCodeBlock::method_SBB_Ev_Gv()//tested
{
	this->atomMethod_SBB_16bit(d_nubit16(pdeCodeBlock->prm), d_nubit16(pdeCodeBlock->preg));
}
void ExecuteCodeBlock::method_SBB_Gb_Eb()//tested
{
	this->atomMethod_SBB_8bit(d_nubit8(pdeCodeBlock->preg), d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::method_SBB_Gv_Ev()//tested
{
	this->atomMethod_SBB_16bit(d_nubit16(pdeCodeBlock->preg), d_nubit16(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::method_SBB_AL_Ib()//tested
{
	this->atomMethod_SBB_8bit(pCPU->al, pdeCodeBlock->immData_8Bit);
}
void ExecuteCodeBlock::method_SBB_eAX_Iv()//tested
{
	this->atomMethod_SBB_16bit(pCPU->ax, pdeCodeBlock->immData_16Bit);
}

void ExecuteCodeBlock::method_PUSH_DS()
{
	this->atomMethod_PUSH(pCPU->ds);
}
void ExecuteCodeBlock::method_POP_DS()
{
	pCPU->ds = this->atomMethod_POP();
}

void ExecuteCodeBlock::method_SUB_Eb_Gb()//tested
{
	this->atomMethod_SUB_8bit(d_nubit8(pdeCodeBlock->prm), d_nubit8(pdeCodeBlock->preg));
}
void ExecuteCodeBlock::method_SUB_Ev_Gv()//tested
{
	this->atomMethod_SUB_16bit(d_nubit16(pdeCodeBlock->prm), d_nubit16(pdeCodeBlock->preg));
}
void ExecuteCodeBlock::method_SUB_Gb_Eb()//tested
{
	this->atomMethod_SUB_8bit(d_nubit8(pdeCodeBlock->preg), d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::method_SUB_Gv_Ev()//tested
{
	this->atomMethod_SUB_16bit(d_nubit16(pdeCodeBlock->preg), d_nubit16(pdeCodeBlock->prm));
}

void ExecuteCodeBlock::method_SUB_AL_Ib()//tested
{
	this->atomMethod_SUB_8bit(pCPU->al, pdeCodeBlock->immData_8Bit);
}
void ExecuteCodeBlock::method_SUB_eAX_Iv()//tested
{
	this->atomMethod_SUB_16bit(pCPU->ax, pdeCodeBlock->immData_16Bit);
}

void ExecuteCodeBlock::method_SEG_CS()
{
	this->pdeCodeBlock->prefix_SegmentOverrideOpCode = CS_SEGMENT_OVERRIDE_PREFIX;
}
void ExecuteCodeBlock::method_DAS()//0x2f
{
	t_nubit8 old_al = pCPU->al;
	bool old_CF = pCPU->getCF_Flag();

	pCPU->setCF_Flag(false);
	if((pCPU->al & 0x0f)>9 || (true ==pCPU->getAF_Flag()))
	{
		this->atomMethod_SUB_8bit(pCPU->al, 6);
		pCPU->setCF_Flag(old_CF); //这里可能会有点问题
		pCPU->setAF_Flag(true);
	}
	else
	{
		pCPU->setAF_Flag(false);
	}
    //*****
	if((old_al>0x99) || (true == old_CF))
	{
		this->atomMethod_SUB_8bit(pCPU->al, 0x60);
		pCPU->setCF_Flag(true);

	}
	else
	{
		pCPU->setCF_Flag(false);//可能有点问题
	}
}

void ExecuteCodeBlock::method_CMP_Eb_Gb()
{
	this->atomMethod_CMP_8bit(d_nubit8(pdeCodeBlock->prm), d_nubit8(pdeCodeBlock->preg));
}
void ExecuteCodeBlock::method_CMP_Ev_Gv()
{
	this->atomMethod_CMP_16bit(d_nubit16(pdeCodeBlock->prm), d_nubit16(pdeCodeBlock->preg));
}
void ExecuteCodeBlock::method_CMP_Gb_Eb()
{
	this->atomMethod_CMP_8bit(d_nubit8(pdeCodeBlock->preg), d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::method_CMP_Gv_Ev()
{
	this->atomMethod_CMP_16bit(d_nubit16(pdeCodeBlock->preg), d_nubit16(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::method_CMP_AL_Ib()
{
	this->atomMethod_CMP_8bit(pCPU->al, pdeCodeBlock->immData_8Bit);
}
void ExecuteCodeBlock::method_CMP_eAX_Iv()
{
	this->atomMethod_CMP_16bit(pCPU->ax, pdeCodeBlock->immData_16Bit);
}

void ExecuteCodeBlock::method_SEG_DS()
{
	pdeCodeBlock->prefix_SegmentOverrideOpCode = DS_SEGMENT_OVERRIDE_PREFIX;
}
void ExecuteCodeBlock::method_AAS()
{
	if((pCPU->al & 0x0f) > 9 || true==pCPU->getAF_Flag())
	{
		pCPU->al -= 6;
		pCPU->ah--;
		pCPU->setAF_Flag(true);
		pCPU->setCF_Flag(true);
		pCPU->al &= 0x0f;
	}
	else
	{
		pCPU->setAF_Flag(false);
		pCPU->setCF_Flag(false);
		pCPU->al &= 0x0f;

	}
}

void ExecuteCodeBlock::method_DEC_eAX()
{	
	this->atomMethod_DEC_16bit(pCPU->ax);
}
void ExecuteCodeBlock::method_DEC_eCX()
{
	this->atomMethod_DEC_16bit(pCPU->cx);
}
void ExecuteCodeBlock::method_DEC_eDX()
{
	this->atomMethod_DEC_16bit(pCPU->dx);
}
void ExecuteCodeBlock::method_DEC_eBX()
{
	this->atomMethod_DEC_16bit(pCPU->bx);
}
void ExecuteCodeBlock::method_DEC_eSP()
{
	this->atomMethod_DEC_16bit(pCPU->sp);
}
void ExecuteCodeBlock::method_DEC_eBP()
{
	this->atomMethod_DEC_16bit(pCPU->bp);
}
void ExecuteCodeBlock::method_DEC_eSI()
{
	this->atomMethod_DEC_16bit(pCPU->si);
}
void ExecuteCodeBlock::method_DEC_eDI()
{
	this->atomMethod_DEC_16bit(pCPU->di);
}

void ExecuteCodeBlock::method_POP_eAX()
{
	pCPU->ax = this->atomMethod_POP();
}
void ExecuteCodeBlock::method_POP_eCX()
{
	pCPU->cx = this->atomMethod_POP();
}
void ExecuteCodeBlock::method_POP_eDX()
{
	pCPU->dx = this->atomMethod_POP();
}
void ExecuteCodeBlock::method_POP_eBX()
{
	pCPU->bx = this->atomMethod_POP();
}
void ExecuteCodeBlock::method_POP_eSP()
{
	pCPU->sp = this->atomMethod_POP();
}
void ExecuteCodeBlock::method_POP_eBP()
{
	pCPU->bp = this->atomMethod_POP();
}
void ExecuteCodeBlock::method_POP_eSI()
{
	pCPU->si = this->atomMethod_POP();
}
void ExecuteCodeBlock::method_POP_eDI()
{
	pCPU->di = this->atomMethod_POP();
}

void ExecuteCodeBlock::method_PUSH_Iv()//0x68  16位中没有，不管了，先做了
{
	this->atomMethod_PUSH(pdeCodeBlock->opContactData_16bit);//估计是这样。。具体还看解码
}
void ExecuteCodeBlock::method_IMUL_Gv_Ev_Iv(){/*16位机没有这个指令*/}
void ExecuteCodeBlock::method_PUSH_Ib()//0x6a
{
	this->atomMethod_PUSH(pdeCodeBlock->opContactData_8bit);//估计是这样。。具体还看解码
}
void ExecuteCodeBlock::method_IMUL_Gv_Ev_Ib(){/*16位机没有这个指令*/}
void ExecuteCodeBlock::method_INSB_Yb_DX(){/*16位机没有这个指令*/}
void ExecuteCodeBlock::method_INSW_Yv_DX(){/*16位机没有这个指令*/}
void ExecuteCodeBlock::method_OUTSB_DX_Xb(){}//无效
void ExecuteCodeBlock::method_OUTSW_DX_Xv(){}//无效

void ExecuteCodeBlock::method_ShortJumpCondition_JS()
{
	if(true == pCPU->getSF_Flag())
	{
		atomMethod_shortJMP();
	}
}
void ExecuteCodeBlock::method_ShortJumpCondition_JNS()
{
	if(false == pCPU->getSF_Flag())
	{
		atomMethod_shortJMP();
	}
}
void ExecuteCodeBlock::method_ShortJumpCondition_JP()
{
	if(true == pCPU->getPF_Flag())
	{
		atomMethod_shortJMP();
	}
}
void ExecuteCodeBlock::method_ShortJumpCondition_JNP()
{
	if(false == pCPU->getPF_Flag())
	{
		atomMethod_shortJMP();
	}
}
void ExecuteCodeBlock::method_ShortJumpCondition_JL()
{
	if(pCPU->getSF_Flag() != pCPU->getOF_Flag())
	{
		atomMethod_shortJMP();
	}
}
void ExecuteCodeBlock::method_ShortJumpCondition_JNL()
{
	if(pCPU->getSF_Flag() == pCPU->getOF_Flag())
	{
		atomMethod_shortJMP();
	}
}
void ExecuteCodeBlock::method_ShortJumpCondition_JLE()
{
	if((true == pCPU->getZF_Flag()) || (pCPU->getSF_Flag() != pCPU->getOF_Flag()))
	{
		atomMethod_shortJMP();
	}
}
void ExecuteCodeBlock::method_ShortJumpCondition_JNLE()
{
	if((false == pCPU->getZF_Flag()) && (pCPU->getSF_Flag() == pCPU->getOF_Flag()))
	{
		atomMethod_shortJMP();
	}
}

void ExecuteCodeBlock::method_MOV_Eb_Gb()//tested
{
	d_nubit8(this->pdeCodeBlock->prm) = d_nubit8(this->pdeCodeBlock->preg);
}
void ExecuteCodeBlock::method_MOV_Ev_Gv()//tested
{
	d_nubit16(this->pdeCodeBlock->prm) = d_nubit16(this->pdeCodeBlock->preg);
}
void ExecuteCodeBlock::method_MOV_Gb_Eb()//tested
{
	d_nubit8(this->pdeCodeBlock->preg) = d_nubit8(this->pdeCodeBlock->prm);
}
void ExecuteCodeBlock::method_MOV_Gv_Ev()//tested
{
	d_nubit16(this->pdeCodeBlock->preg) = d_nubit16(this->pdeCodeBlock->prm);
}
void ExecuteCodeBlock::method_MOV_Ew_Sw()//tested
{
	d_nubit16(this->pdeCodeBlock->prm) = d_nubit16(this->pdeCodeBlock->pseg);
}

void ExecuteCodeBlock::method_LEA_Gv_M()
{
	d_nubit16(pdeCodeBlock->preg) = (t_nubit16)((t_nubit8 *)(pdeCodeBlock->prm) - p_nubit8(vramGetAddr(0,0)) - (pdeCodeBlock->segData<<4));
}

void ExecuteCodeBlock::method_MOV_Sw_Ew()//tested
{
	d_nubit16(this->pdeCodeBlock->pseg) = d_nubit16(this->pdeCodeBlock->prm);
}
void ExecuteCodeBlock::method_POP_Ev()
{
	d_nubit16(pdeCodeBlock->prm) = this->atomMethod_POP();
}

void ExecuteCodeBlock::method_CBW()//0x98
{
	if(pCPU->al >= 0x80)
	{
		pCPU->ah = 0xff;
	}
	else
	{
		pCPU->ah = 0x00;
	}
}
void ExecuteCodeBlock::method_CWD() //0x99
{
	if(pCPU->ax >= 0x8000)
	{
		pCPU->dx = 0xffff;
	}
	else
	{
		pCPU->dx = 0x0000;
	}
}
void ExecuteCodeBlock::method_CALL_aP()//0x9a 段间直接调用
{
	this->atomMethod_PUSH(pCPU->cs);
	this->atomMethod_PUSH(pCPU->ip); // 没有PUSH IP 。。所以用这个来代替
	
	lcpu.cs = this->pdeCodeBlock->opContactData_16bitE;
	lcpu.ip = this->pdeCodeBlock->opContactData_16bit;
}
void ExecuteCodeBlock::method_WAIT(){}//还不清楚
void ExecuteCodeBlock::method_PUSHF_Fv()
{
	this->atomMethod_PUSH(pCPU->generateFLAG());
}
void ExecuteCodeBlock::method_POP_Fv()//0x8d  POPF 将标志弹出
{
	pCPU->setFLAG(this->atomMethod_POP());
}
void ExecuteCodeBlock::method_SAHF()//0x9e
{
	pCPU->set_SF_ZF_AF_PF_CF_Flag((t_nubit16)(pCPU->ah));
}
void ExecuteCodeBlock::method_LAHF()
{
	pCPU->ah = (t_nubit8)pCPU->generateFLAG();
}

void ExecuteCodeBlock::method_TEST_AL_Ib()
{
	t_nubit8 tmp = pCPU->al;
	this->atomMethod_AND_8bit(tmp, pdeCodeBlock->immData_8Bit);
}
void ExecuteCodeBlock::method_TEST_eAX_Iv()
{
	t_nubit16 tmp = pCPU->ax;
	this->atomMethod_AND_16bit(tmp, pdeCodeBlock->immData_16Bit);
}
void ExecuteCodeBlock::method_STOSB_Yb_AL()//0xaa
{
	if(pdeCodeBlock->prefix_RepeatOpCode == REPNE_PREFIX ||
		pdeCodeBlock->prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		repeatMethod = &ExecuteCodeBlock::atomMethod_STOSB;
		this->chooseRepeatExeStringMethod();
		return;
	}
	else
		this->atomMethod_STOSB();
}
void ExecuteCodeBlock::method_STOSW_Yv_eAX()
{
	if(pdeCodeBlock->prefix_RepeatOpCode == REPNE_PREFIX ||
		pdeCodeBlock->prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		repeatMethod = &ExecuteCodeBlock::atomMethod_STOSW;
		this->chooseRepeatExeStringMethod();
		return;
	}
	else
		this->atomMethod_STOSW();
}
void ExecuteCodeBlock::method_LODSB_AL_Xb()
{
	if(pdeCodeBlock->prefix_RepeatOpCode == REPNE_PREFIX ||
		pdeCodeBlock->prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		repeatMethod = &ExecuteCodeBlock::atomMethod_LODSB;
		this->chooseRepeatExeStringMethod();
		return;
	}
	else
		this->atomMethod_LODSB();
}
void ExecuteCodeBlock::method_LODSW_eAX_Xv()
{
	if(pdeCodeBlock->prefix_RepeatOpCode == REPNE_PREFIX ||
		pdeCodeBlock->prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		repeatMethod = &ExecuteCodeBlock::atomMethod_LODSW;
		this->chooseRepeatExeStringMethod();
		return;
	}
	else
		this->atomMethod_LODSW();
}
void ExecuteCodeBlock::method_SCASB_AL_Yb()
{
	if(pdeCodeBlock->prefix_RepeatOpCode == REPNE_PREFIX ||
		pdeCodeBlock->prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		repeatMethod = &ExecuteCodeBlock::atomMethod_SCASB;
		this->chooseRepeatExeStringMethod();
		return;
	}
	else
		this->atomMethod_SCASB();
}
void ExecuteCodeBlock::method_SCASW_eAX_Yv()
{
	if(pdeCodeBlock->prefix_RepeatOpCode == REPNE_PREFIX ||
		pdeCodeBlock->prefix_RepeatOpCode == REPE_PREFIX)
	{//低效，以后改进 ...检测是否是需要repeat的
	
		repeatMethod = &ExecuteCodeBlock::atomMethod_SCASW;
		this->chooseRepeatExeStringMethod();
		return;
	}
	else
		this->atomMethod_SCASW();
}

void ExecuteCodeBlock::method_MOV_eAX()//tested
{
	lcpu.ax = this->pdeCodeBlock->immData_16Bit;
}
void ExecuteCodeBlock::method_MOV_eCX()//tested
{
	lcpu.cx = this->pdeCodeBlock->immData_16Bit;
}
void ExecuteCodeBlock::method_MOV_eDX()//tested
{
	lcpu.dx =this->pdeCodeBlock->immData_16Bit;
}
void ExecuteCodeBlock::method_MOV_eBX()//tested
{
	lcpu.bx = this->pdeCodeBlock->immData_16Bit;
}
void ExecuteCodeBlock::method_MOV_eSP()//tested
{
	lcpu.sp = this->pdeCodeBlock->immData_16Bit;
}
void ExecuteCodeBlock::method_MOV_eBP()//tested
{
	lcpu.bp =this->pdeCodeBlock->immData_16Bit;
}
void ExecuteCodeBlock::method_MOV_eSI()//tested
{
	lcpu.si = this->pdeCodeBlock->immData_16Bit;
}
void ExecuteCodeBlock::method_MOV_eDI()//tested
{
	lcpu.di = this->pdeCodeBlock->immData_16Bit;
}

void ExecuteCodeBlock::method_ENTER_Iw_Ib(){/*十六位机子没这个指令*/}
void ExecuteCodeBlock::method_LEAVE(){/*十六位机子没这个指令*/}
void ExecuteCodeBlock::method_RET_far_Iw()
{
	pCPU->ip = this->atomMethod_POP();
	pCPU->cs = this->atomMethod_POP();
	pCPU->sp += pdeCodeBlock->opContactData_16bit;
}
void ExecuteCodeBlock::method_RET_far()
{
	pCPU->ip = this->atomMethod_POP();
	pCPU->cs = this->atomMethod_POP();
}
void ExecuteCodeBlock::method_INT_3()
{//暂时没有
}
void ExecuteCodeBlock::method_INT_Ib()
{
	t_nubit8 intid = pdeCodeBlock->opContactData_8bit;
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
		lcpu.ax = _ax;
		lcpu.bx = _bx;
		lcpu.cx = _cx;
		lcpu.dx = _dx;
		lcpu.sp = _sp;
		lcpu.bp = _bp;
		lcpu.si = _si;
		lcpu.di = _di;
		lcpu.ip = _ip;
		lcpu.cs = _cs;
		lcpu.ds = _ds;
		lcpu.es = _es;
		lcpu.ss = _ss;
		lcpu.overds = vcpu.overds;
		lcpu.overss = vcpu.overss;
		lcpu.iobyte = vcpu.iobyte;
		lcpu.setAF_Flag(_af);
		lcpu.setCF_Flag(_cf);
		lcpu.setDF_Flag(_df);
		lcpu.setIF_Flag(_if);
		lcpu.setOF_Flag(_of);
		lcpu.setPF_Flag(_pf);
		lcpu.setSF_Flag(_sf);
		lcpu.setTF_Flag(_tf);
		lcpu.setZF_Flag(_zf);
		break;
	default: this->atomMethod_INT(intid);
	}
// NEKOLOG
//	if (qdbiosExecInt(INT_num)) return;
//	else this->atomMethod_INT(intid);
}
void ExecuteCodeBlock::method_INTO()
{
	if(pCPU->getOF_Flag())
	{//OF为1 时执行4号中断
		this->atomMethod_INT(4);
	}
}
void ExecuteCodeBlock::method_IRET()
{
	pCPU->ip = this->atomMethod_POP();
	pCPU->cs = this->atomMethod_POP();
	pCPU->setFLAG(this->atomMethod_POP());
}

void ExecuteCodeBlock::method_ESC(){}//还不清楚

void ExecuteCodeBlock::method_CALL_Jv()//0xe8 段内直接调用
{
	this->atomMethod_PUSH(pCPU->ip); // 没有PUSH IP 。。所以用这个来代替
	pCPU->ip += (t_nsbit16)(pdeCodeBlock->opContactData_16bit);
}
void ExecuteCodeBlock::method_JMP_Jv()//段内直接转移之一
{
	pCPU->ip += (t_nsbit16)(pdeCodeBlock->opContactData_16bit);
}
void ExecuteCodeBlock::method_JMP_Ap()//段间直接转移
{
	pCPU->ip = pdeCodeBlock->opContactData_16bit;
	pCPU->cs = pdeCodeBlock->opContactData_16bitE;
}
void ExecuteCodeBlock::method_JMP_Jb()//段内直接转移之二
{
	this->atomMethod_shortJMP();
}
void ExecuteCodeBlock::method_IN_AL_DX(){
	lcpu.al = _al;
	lcpu.iobyte = vcpu.iobyte;
}//暂时没有
void ExecuteCodeBlock::method_IN_eAX_DX(){
	lcpu.ax = _ax;
	lcpu.iobyte = vcpu.iobyte;
}//暂时没有
void ExecuteCodeBlock::method_OUT_DX_AL(){
	lcpu.iobyte = lcpu.al;
}//暂时没有
void ExecuteCodeBlock::method_OUT_DX_eAX(){
	lcpu.iobyte = lcpu.al;
}//暂时没有

void ExecuteCodeBlock::method_CLC()//0xf8
{
	lcpu.setCF_Flag(false);
}
void ExecuteCodeBlock::method_STC()
{
	lcpu.setCF_Flag(true);
}
void ExecuteCodeBlock::method_CLI()//0xfa
{
	lcpu.setIF_Flag(false);
}
void ExecuteCodeBlock::method_STI()
{
	lcpu.setIF_Flag(true);
}
void ExecuteCodeBlock::method_CLD()//0xfc
{
	lcpu.setDF_Flag(false);
}
void ExecuteCodeBlock::method_STD()
{
	lcpu.setDF_Flag(true);
}
void ExecuteCodeBlock::method_INC_Group4()
{
	switch(pdeCodeBlock->nnn)
	{
	case 0://nnn=000
		this->methodGroup_INC_Eb();
		break;
	case 1://nnn=001
		this->methodGroup_DEC_Eb();
		break;
	default:
		showMessage("method_INC_Group4 UnHandle the nnn bits");
		assert(false);
		break;
	}
}
void ExecuteCodeBlock::method_INC_Group5()
{
	switch(pdeCodeBlock->nnn)
	{
	case 0://nnn=000
		this->methodGroup_INC_Ev();
		break;
	case 1://nnn=001
		this->methodGroup_DEC_Ev();
		break;
	case 2://nnn=010
		this->methodGroup_CALL_Ev();
		break;
	case 3://nnn=011
		this->methodGroup_CALL_Ep();
		break;
	case 4://nnn=100
		this->methodGroup_JMP_Ev();
		break;
	case 5://nnn=101
		this->methodGroup_JMP_Ep();
		break;
	case 6://nnn=110
		this->methodGroup_PUSH_Ev();
		return;
	case 7://nnn=111
		showMessage("method_INC_Group5 UnHandle the nnn bits");
		assert(false);
		break;
	}
}






void ExecuteCodeBlock::selfMethod_PUSH(const t_nubit8 byteData)
{
	//need finish;
}
void ExecuteCodeBlock::selfMethod_PUSH(const t_nubit16 wordData)
{//need finish;
}



void ExecuteCodeBlock::chooseRepeatExeStringMethod()
{

	if(repeatMethod == &ExecuteCodeBlock::atomMethod_MOVSB ||
		repeatMethod == &ExecuteCodeBlock::atomMethod_MOVSW ||
		repeatMethod == &ExecuteCodeBlock::atomMethod_LODSB ||
		repeatMethod == &ExecuteCodeBlock::atomMethod_LODSW ||
		repeatMethod == &ExecuteCodeBlock::atomMethod_STOSB ||
		repeatMethod == &ExecuteCodeBlock::atomMethod_STOSW ||
		repeatMethod == &ExecuteCodeBlock::atomMethod_INS   ||
		repeatMethod == &ExecuteCodeBlock::atomMethod_OUTS )
	{
		
	
		self_REP_StringIns();
	
	}
	else
	{
		if(pdeCodeBlock->prefix_RepeatOpCode == REPE_PREFIX) //用于区分REPNE 和REPE 。。。。
			self_REPE_StringIns();  //除此则都和REPE 
		else
			self_REPNE_StringIns();	
	}
	
}

void ExecuteCodeBlock::self_REP_StringIns()
{
	while(0 != pCPU->cx)
	{
		((*this).*(repeatMethod))();
		pCPU->cx--;
	}
}
void ExecuteCodeBlock::self_REPE_StringIns()
{
	while(0 != pCPU->cx)
	{
		((*this).*(repeatMethod))();
		pCPU->cx--;
		if(false == pCPU->getZF_Flag())
			break;
	}
}
void ExecuteCodeBlock::self_REPNE_StringIns()
{
	while(0 != pCPU->cx)
	{
		((*this).*(repeatMethod))();
		pCPU->cx--;
		if(true == pCPU->getZF_Flag())
			break;
	}
}


void ExecuteCodeBlock::method_Unknow(){vapiPrint("unkonwnMethod");}

void ExecuteCodeBlock::method_outsideINT(t_nubit8 INT_num)
{
	this->atomMethod_INT(INT_num);
}