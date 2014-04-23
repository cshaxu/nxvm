
#include "../lcpuins.h"
#include "CentreProcessorUnit.h"

#define rotl(x,n) ((((x)<<(n))&(-(1<<(n))))|(((x)>>(sizeof(x)*8-(n)))&((1<<(n))-1)))

#define ror(x) rotl((x),1)
//循环右移的宏
bool getSignByData(const t_nubit8 data)
{
	return !!(data & MASK_10000000);
}
bool getSignByData(const t_nubit16 data)
{
	return !!(data & 0x8000);
}
bool getFinalBit(const t_nubit8 data)
{
	return !!(data & MASK_00000001);
}
bool getFinalBit(const t_nubit16 data)
{
	return !!(data & 1);
}
bool isSignBitChange(const t_nubit8 fir, const t_nubit8 sec)
{
	if(getSignByData(fir) == getSignByData(sec))
		return false;
	return true;
}
bool isSignBitChange(const t_nubit16 fir, const t_nubit16 sec)
{
	if(getSignByData(fir) == getSignByData(sec))
		return false;
	return true;
}
////////////////////////////////////////////<GROUP1>
void ExecuteCodeBlock::methodGroup_ADD_Eb_Ib()//tested
{
	this->atomMethod_ADD_8bit(d_nubit8(pdeCodeBlock->prm), pdeCodeBlock->immData_8Bit);
}
void ExecuteCodeBlock::methodGroup_ADD_Ev_Iv()//tested
{
	this->atomMethod_ADD_16bit(d_nubit16(pdeCodeBlock->prm), pdeCodeBlock->immData_16Bit);
}
void ExecuteCodeBlock::methodGroup_ADD_Ev_Ib()//tested
{
	t_nubit16 tmp = pdeCodeBlock->immData_8Bit;
	this->atomMethod_ADD_16bit(d_nubit16(pdeCodeBlock->prm), tmp);
}

void ExecuteCodeBlock::methodGroup_OR_Eb_Ib()
{
	this->atomMethod_OR_8bit(d_nubit8(pdeCodeBlock->prm), pdeCodeBlock->immData_8Bit);
}
void ExecuteCodeBlock::methodGroup_OR_Ev_Iv()
{
	this->atomMethod_OR_16bit(d_nubit16(pdeCodeBlock->prm), pdeCodeBlock->immData_16Bit);
}
void ExecuteCodeBlock::methodGroup_OR_Ev_Ib()
{
	//assert(false);//may be some problem
	t_nubit16 tmp = pdeCodeBlock->immData_8Bit;
	this->atomMethod_OR_16bit(d_nubit16(pdeCodeBlock->prm), tmp);
}

void ExecuteCodeBlock::methodGroup_ADC_Eb_Ib()//tested
{
	this->atomMethod_ADC_8bit(d_nubit8(pdeCodeBlock->prm), pdeCodeBlock->immData_8Bit);
}
void ExecuteCodeBlock::methodGroup_ADC_Ev_Iv()//tested
{
	this->atomMethod_ADC_16bit(d_nubit16(pdeCodeBlock->prm), pdeCodeBlock->immData_16Bit);
}
void ExecuteCodeBlock::methodGroup_ADC_Ev_Ib()//tested
{
	//assert(false);//may be some problem
	t_nubit16 tmp = pdeCodeBlock->immData_8Bit;
	this->atomMethod_ADC_16bit(d_nubit16(pdeCodeBlock->prm), tmp);
}

void ExecuteCodeBlock::methodGroup_SBB_Eb_Ib()
{
	this->atomMethod_SBB_8bit(d_nubit8(pdeCodeBlock->prm), pdeCodeBlock->immData_8Bit);
}
void ExecuteCodeBlock::methodGroup_SBB_Ev_Iv()
{
	this->atomMethod_SBB_16bit(d_nubit16(pdeCodeBlock->prm), pdeCodeBlock->immData_16Bit);
}
void ExecuteCodeBlock::methodGroup_SBB_Ev_Ib()
{
	//assert(false);//may be some problem
	t_nubit16 tmp = pdeCodeBlock->immData_8Bit;
	this->atomMethod_SBB_16bit(d_nubit16(pdeCodeBlock->prm), tmp);
}

void ExecuteCodeBlock::methodGroup_AND_Eb_Ib()
{
	this->atomMethod_AND_8bit(d_nubit8(pdeCodeBlock->prm), pdeCodeBlock->immData_8Bit);
}
void ExecuteCodeBlock::methodGroup_AND_Ev_Iv()
{
	this->atomMethod_AND_16bit(d_nubit16(pdeCodeBlock->prm), pdeCodeBlock->immData_16Bit);
}
void ExecuteCodeBlock::methodGroup_AND_Ev_Ib()
{
	//assert(false);//may be some problem
	t_nubit16 tmp = pdeCodeBlock->immData_8Bit;
	this->atomMethod_AND_16bit(d_nubit16(pdeCodeBlock->prm), tmp);
}

void ExecuteCodeBlock::methodGroup_SUB_Eb_Ib()//tested
{
	this->atomMethod_SUB_8bit(d_nubit8(pdeCodeBlock->prm), pdeCodeBlock->immData_8Bit);
}
void ExecuteCodeBlock::methodGroup_SUB_Ev_Iv()//tested
{
	this->atomMethod_SUB_16bit(d_nubit16(pdeCodeBlock->prm), pdeCodeBlock->immData_16Bit);
}
void ExecuteCodeBlock::methodGroup_SUB_Ev_Ib()//tested
{
	//assert(false);//may be some problem
	t_nubit16 tmp = pdeCodeBlock->immData_8Bit;
	this->atomMethod_SUB_16bit(d_nubit16(pdeCodeBlock->prm), tmp);
}

void ExecuteCodeBlock::methodGroup_XOR_Eb_Ib()
{
	this->atomMethod_XOR_8bit(d_nubit8(pdeCodeBlock->prm), pdeCodeBlock->immData_8Bit);
}
void ExecuteCodeBlock::methodGroup_XOR_Ev_Iv()
{
	this->atomMethod_XOR_16bit(d_nubit16(pdeCodeBlock->prm), pdeCodeBlock->immData_16Bit);
}
void ExecuteCodeBlock::methodGroup_XOR_Ev_Ib()
{
	//assert(false);//may be some problem
	t_nubit16 tmp = pdeCodeBlock->immData_8Bit;
	this->atomMethod_XOR_16bit(d_nubit16(pdeCodeBlock->prm), tmp);
}

void ExecuteCodeBlock::methodGroup_CMP_Eb_Ib()
{
	this->atomMethod_CMP_8bit(d_nubit8(pdeCodeBlock->prm), d_nsbit8(&pdeCodeBlock->immData_8Bit));
}
void ExecuteCodeBlock::methodGroup_CMP_Ev_Iv()
{
/*	if ( (d_nsbit16(&pdeCodeBlock->immData_16Bit)  == 0x5052)) {
			FILE *fp = fopen("d:/temp2.log","w");
			fprintf(fp,"%x\n", (t_nubit8 *)pdeCodeBlock->prm - (t_nubit8 *)p_nubit8(vramGetAddr(0,0)) );
			fclose(fp);
	}*/
	this->atomMethod_CMP_16bit(d_nubit16(pdeCodeBlock->prm), d_nsbit16(&pdeCodeBlock->immData_16Bit));
}
void ExecuteCodeBlock::methodGroup_CMP_Ev_Ib()
{
	//assert(false);//may be some problem
	this->atomMethod_CMP_16bit(d_nubit16(pdeCodeBlock->prm),  (t_nsbit8)pdeCodeBlock->immData_8Bit);
}
////////////////////////////////////////////</GROUP1>

////////////////////////////////////////////<GROUP2>

void ExecuteCodeBlock::methodGroup_ROL_Eb_Ib(){}
void ExecuteCodeBlock::methodGroup_ROL_Ev_Ib(){}
void ExecuteCodeBlock::methodGroup_ROL_Eb_1()
{
	t_nubit8  oldEb  = d_nubit8(pdeCodeBlock->prm);
	bool old_sign = getSignByData( oldEb );
	pCPU->setCF_Flag( old_sign );
	(d_nubit8(pdeCodeBlock->prm)) = ((d_nubit8(pdeCodeBlock->prm))<<1) | ((d_nubit8(pdeCodeBlock->prm))>>7) ;

	pCPU->setOF_Flag(isSignBitChange(oldEb, d_nubit8(pdeCodeBlock->prm)));

	
}
void ExecuteCodeBlock::methodGroup_ROL_Ev_1()
{
	t_nubit16  oldEv  = d_nubit16(pdeCodeBlock->prm);
	bool old_sign = getSignByData( oldEv );
	pCPU->setCF_Flag( old_sign );
	(d_nubit16(pdeCodeBlock->prm)) = ((d_nubit16(pdeCodeBlock->prm))<<1) | ((d_nubit16(pdeCodeBlock->prm))>>15) ;

	pCPU->setOF_Flag(isSignBitChange(oldEv, d_nubit16(pdeCodeBlock->prm)));

	
}
void ExecuteCodeBlock::methodGroup_ROL_Eb_CL()
{
	if(0 == pCPU->cl)
		return;
	bool oldOF = pCPU->getOF_Flag();
	t_nubit8 tmpCL = pCPU->cl;
	while(tmpCL)
	{
		methodGroup_ROL_Eb_1();
		tmpCL--;
	}
	pCPU->setOF_Flag(oldOF);

	//pCPU->setSF_ZF_PF_byResult(d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_ROL_Ev_CL()
{
	if(0 == pCPU->cl)
		return;
	bool oldOF = pCPU->getOF_Flag();
	t_nubit8 tmpCL = pCPU->cl;
	while(tmpCL)
	{
		methodGroup_ROL_Ev_1();
		tmpCL--;
	}
	pCPU->setOF_Flag(oldOF);

	//pCPU->setSF_ZF_PF_byResult(d_nubit16(pdeCodeBlock->prm));
}

void ExecuteCodeBlock::methodGroup_ROR_Eb_Ib(){}
void ExecuteCodeBlock::methodGroup_ROR_Ev_Ib(){}
void ExecuteCodeBlock::methodGroup_ROR_Eb_1()
{
	t_nubit8  oldEb  = d_nubit8(pdeCodeBlock->prm);
	bool finalBit = getFinalBit( oldEb );
	pCPU->setCF_Flag( finalBit );
	(d_nubit8(pdeCodeBlock->prm)) = ((d_nubit8(pdeCodeBlock->prm))>>1) | ((d_nubit8(pdeCodeBlock->prm))<<7) ;

	pCPU->setOF_Flag(isSignBitChange(oldEb, d_nubit8(pdeCodeBlock->prm)));

	//pCPU->setSF_ZF_PF_byResult(d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_ROR_Ev_1()
{
	t_nubit16  oldEv  = d_nubit16(pdeCodeBlock->prm);
	bool finalBit = getFinalBit( oldEv );
	pCPU->setCF_Flag( finalBit );
	(d_nubit16(pdeCodeBlock->prm)) = ((d_nubit16(pdeCodeBlock->prm))>>1) | ((d_nubit16(pdeCodeBlock->prm))<<15) ;

	pCPU->setOF_Flag(isSignBitChange(oldEv, d_nubit16(pdeCodeBlock->prm)));

	//pCPU->setSF_ZF_PF_byResult(d_nubit16(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_ROR_Eb_CL()
{
	if(0 == pCPU->cl)
		return;
	bool oldOF = pCPU->getOF_Flag();
	t_nubit8 tmpCL = pCPU->cl;
	while(tmpCL)
	{
		methodGroup_ROR_Eb_1();
		tmpCL--;
	}
	pCPU->setOF_Flag(oldOF);
	//pCPU->setSF_ZF_PF_byResult(d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_ROR_Ev_CL()
{
	if(0 == pCPU->cl)
		return;
	bool oldOF = pCPU->getOF_Flag();
	t_nubit8 tmpCL = pCPU->cl;
	while(tmpCL)
	{
		methodGroup_ROR_Ev_1();
		tmpCL--;
	}
	pCPU->setOF_Flag(oldOF);
	//pCPU->setSF_ZF_PF_byResult(d_nubit16(pdeCodeBlock->prm));
}

void ExecuteCodeBlock::methodGroup_RCL_Eb_Ib(){}
void ExecuteCodeBlock::methodGroup_RCL_Ev_Ib(){}
void ExecuteCodeBlock::methodGroup_RCL_Eb_1()
{
	t_nubit16 tmpFlags = pCPU->generateFLAG();
	methodGroup_SHL_Eb_1();
	bool newCF = pCPU->getCF_Flag();
	bool newOF = pCPU->getOF_Flag();
	pCPU->setFLAG(tmpFlags);
	if(pCPU->getCF_Flag())
	{
		d_nubit8(pdeCodeBlock->prm) |= MASK_00000001;
	}
	pCPU->setCF_Flag(newCF);
	pCPU->setOF_Flag(newOF);
	//pCPU->setSF_ZF_PF_byResult(d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_RCL_Ev_1()
{
	t_nubit16 tmpFlags = pCPU->generateFLAG();
	methodGroup_SHL_Ev_1();
	bool newCF = pCPU->getCF_Flag();
	bool newOF = pCPU->getOF_Flag();
	pCPU->setFLAG(tmpFlags);
	if(pCPU->getCF_Flag())
	{
		d_nubit16(pdeCodeBlock->prm) |= 1;
	}
	pCPU->setCF_Flag(newCF);
	pCPU->setOF_Flag(newOF);
	//pCPU->setSF_ZF_PF_byResult(d_nubit16(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_RCL_Eb_CL()
{
	if(0 == pCPU->cl)
		return;
	bool oldOF = pCPU->getOF_Flag();
	t_nubit8 tmpCL = pCPU->cl;
	while(tmpCL)
	{
		methodGroup_RCL_Eb_1();
		tmpCL--;
	}
	pCPU->setOF_Flag(oldOF);
	//pCPU->setSF_ZF_PF_byResult(d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_RCL_Ev_CL()
{
	if(0 == pCPU->cl)
		return;
	bool oldOF = pCPU->getOF_Flag();
	t_nubit8 tmpCL = pCPU->cl;
	while(tmpCL)
	{
		methodGroup_RCL_Ev_1();
		tmpCL--;
	}
	pCPU->setOF_Flag(oldOF);
	//pCPU->setSF_ZF_PF_byResult(d_nubit16(pdeCodeBlock->prm));
}

void ExecuteCodeBlock::methodGroup_RCR_Eb_Ib(){}
void ExecuteCodeBlock::methodGroup_RCR_Ev_Ib(){}
void ExecuteCodeBlock::methodGroup_RCR_Eb_1()
{
	t_nubit8 oldEb = d_nubit8(pdeCodeBlock->prm);
	bool oldCF = pCPU->getCF_Flag();
	t_nubit16 tmpFlags = pCPU->generateFLAG();
	methodGroup_SHR_Eb_1();
	bool newCF = pCPU->getCF_Flag();
	pCPU->setFLAG(tmpFlags);//因为methodGroup_SHR_Eb_1函数会改变Flags值，而实际上RCR是不影响那几位的。。
	if(oldCF)
	{
		d_nubit8(pdeCodeBlock->prm) |= MASK_10000000;
	}
	pCPU->setOF_Flag(isSignBitChange(oldEb, d_nubit8(pdeCodeBlock->prm)));
	pCPU->setCF_Flag(newCF);
	//pCPU->setSF_ZF_PF_byResult(d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_RCR_Ev_1()
{
	t_nubit16 oldEb = d_nubit16(pdeCodeBlock->prm);
	t_nubit16 tmpFlags = pCPU->generateFLAG();
	bool oldCF = pCPU->getCF_Flag();
	methodGroup_SHR_Ev_1();
	bool newCF = pCPU->getCF_Flag();
	pCPU->setFLAG(tmpFlags);
	if(oldCF)
	{
		d_nubit16(pdeCodeBlock->prm) |= 0x8000;
	}
	pCPU->setOF_Flag(isSignBitChange(oldEb, d_nubit16(pdeCodeBlock->prm)));
	pCPU->setCF_Flag(newCF);
	//pCPU->setSF_ZF_PF_byResult(d_nubit16(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_RCR_Eb_CL()
{
	if(0 == pCPU->cl)
		return;
	bool oldOF = pCPU->getOF_Flag();
	t_nubit8 tmpCL = pCPU->cl;
	while(tmpCL)
	{
		methodGroup_RCR_Eb_1();
		tmpCL--;
	}
	pCPU->setOF_Flag(oldOF);
	//pCPU->setSF_ZF_PF_byResult(d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_RCR_Ev_CL()
{
	if(0 == pCPU->cl)
		return;
	bool oldOF = pCPU->getOF_Flag();
	t_nubit8 tmpCL = pCPU->cl;
	while(tmpCL)
	{
		methodGroup_RCR_Ev_1();
		tmpCL--;
	}
	pCPU->setOF_Flag(oldOF);
	//pCPU->setSF_ZF_PF_byResult(d_nubit16(pdeCodeBlock->prm));
}

void ExecuteCodeBlock::methodGroup_SHL_Eb_Ib(){}
void ExecuteCodeBlock::methodGroup_SHL_Ev_Ib(){}
void ExecuteCodeBlock::methodGroup_SHL_Eb_1()
{	
	t_nubit8  oldEb  = d_nubit8(pdeCodeBlock->prm);
	bool old_sign = getSignByData( oldEb );
	pCPU->setCF_Flag( old_sign );
	(d_nubit8(pdeCodeBlock->prm)) <<= 1;

	pCPU->setOF_Flag(isSignBitChange(oldEb, d_nubit8(pdeCodeBlock->prm)));

	pCPU->setSF_ZF_PF_byResult(d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_SHL_Ev_1()
{
	t_nubit16  oldEv  = d_nubit16(pdeCodeBlock->prm);
	bool old_sign = getSignByData( oldEv );
	pCPU->setCF_Flag( old_sign );
	(d_nubit16(pdeCodeBlock->prm)) <<= 1;

	pCPU->setOF_Flag(isSignBitChange(oldEv, d_nubit16(pdeCodeBlock->prm)));

	pCPU->setSF_ZF_PF_byResult(d_nubit16(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_SHL_Eb_CL()
{
	if(0 == pCPU->cl)
		return;
	bool sign = false;
	t_nubit8 tmpCL = pCPU->cl;
	while(tmpCL)
	{
		sign = getSignByData( d_nubit8(pdeCodeBlock->prm) );
		(d_nubit8(pdeCodeBlock->prm)) <<= 1; //低速实现，以后修改
		tmpCL--;
	}
	pCPU->setCF_Flag(sign);
	pCPU->setSF_ZF_PF_byResult(d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_SHL_Ev_CL()
{
	if(0 == pCPU->cl)
		return;
	bool sign = false;
	t_nubit8 tmpCL = pCPU->cl;
	while(tmpCL)
	{
		sign = getSignByData( d_nubit16(pdeCodeBlock->prm) );
		(d_nubit16(pdeCodeBlock->prm)) <<= 1; //低速实现，以后修改
		tmpCL--;
	}
	pCPU->setCF_Flag(sign);
	pCPU->setSF_ZF_PF_byResult(d_nubit16(pdeCodeBlock->prm));
}

void ExecuteCodeBlock::methodGroup_SHR_Eb_Ib(){}
void ExecuteCodeBlock::methodGroup_SHR_Ev_Ib(){}
void ExecuteCodeBlock::methodGroup_SHR_Eb_1()
{
	t_nubit8 oldEb = d_nubit8(pdeCodeBlock->prm);
	bool finalBit = getFinalBit( oldEb );
	d_nubit8(pdeCodeBlock->prm) >>= 1;
	pCPU->setCF_Flag(finalBit);
	pCPU->setOF_Flag(isSignBitChange(oldEb, d_nubit8(pdeCodeBlock->prm)));
	pCPU->setSF_ZF_PF_byResult(d_nubit8(pdeCodeBlock->prm));

}
void ExecuteCodeBlock::methodGroup_SHR_Ev_1()
{
	t_nubit16 oldEv = d_nubit16(pdeCodeBlock->prm);
	bool finalBit = getFinalBit( oldEv );
	d_nubit16(pdeCodeBlock->prm) >>= 1;
	pCPU->setCF_Flag(finalBit);
	pCPU->setOF_Flag(isSignBitChange(oldEv, d_nubit16(pdeCodeBlock->prm)));
	pCPU->setSF_ZF_PF_byResult(d_nubit16(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_SHR_Eb_CL()
{
	if(0 == pCPU->cl)
		return;
	bool finalBit = false;
	t_nubit8 tmpCL = pCPU->cl;
	while(tmpCL)
	{
		finalBit = getFinalBit(d_nubit8(pdeCodeBlock->prm));
		d_nubit8(pdeCodeBlock->prm) >>= 1;
		tmpCL--;
	}
	pCPU->setCF_Flag(finalBit);
	pCPU->setSF_ZF_PF_byResult(d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_SHR_Ev_CL()
{
	if(0 == pCPU->cl)
		return;
	bool finalBit = false;
	t_nubit8 tmpCL = pCPU->cl;
	while(tmpCL)
	{
		finalBit = getFinalBit(d_nubit16(pdeCodeBlock->prm));
		d_nubit16(pdeCodeBlock->prm) >>= 1;
		tmpCL--;
	}
	pCPU->setCF_Flag(finalBit);
	pCPU->setSF_ZF_PF_byResult(d_nubit16(pdeCodeBlock->prm));
}

void ExecuteCodeBlock::methodGroup_SAR_Eb_Ib(){}
void ExecuteCodeBlock::methodGroup_SAR_Ev_Ib(){}
void ExecuteCodeBlock::methodGroup_SAR_Eb_1()
{
	t_nsbit8 oldEb = d_nsbit8(pdeCodeBlock->prm);
	bool finalBit = getFinalBit( (t_nubit8)oldEb );
	d_nsbit8(pdeCodeBlock->prm) >>= 1;
	pCPU->setCF_Flag(finalBit);
	pCPU->setOF_Flag(false);//算术右移，部队符号位改变
	pCPU->setSF_ZF_PF_byResult(d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_SAR_Ev_1()
{
	t_nsbit16 oldEv = d_nsbit16(pdeCodeBlock->prm);
	bool finalBit = getFinalBit( (t_nubit8)oldEv );
	d_nsbit16(pdeCodeBlock->prm) >>= 1;
	pCPU->setCF_Flag(finalBit);
	pCPU->setOF_Flag(isSignBitChange(oldEv, d_nubit16(pdeCodeBlock->prm)));
	pCPU->setSF_ZF_PF_byResult(d_nubit16(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_SAR_Eb_CL()
{
	if(0 == pCPU->cl)
		return;
	bool finalBit = false;
	t_nubit8 tmpCL = pCPU->cl;
	while(tmpCL)
	{
		finalBit = getFinalBit(d_nubit8(pdeCodeBlock->prm));
		d_nsbit8(pdeCodeBlock->prm) >>= 1;
		tmpCL--;
	}
	pCPU->setCF_Flag(finalBit);
	pCPU->setSF_ZF_PF_byResult(d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_SAR_Ev_CL()
{
	if(0 == pCPU->cl)
		return;
	bool finalBit = false;
	t_nubit8 tmpCL = pCPU->cl;
	while(tmpCL)
	{
		finalBit = getFinalBit(d_nubit16(pdeCodeBlock->prm));
		d_nsbit16(pdeCodeBlock->prm) >>= 1;
		tmpCL--;
	}
	pCPU->setCF_Flag(finalBit);
	pCPU->setSF_ZF_PF_byResult(d_nubit16(pdeCodeBlock->prm));
}

///////////////////////////////////////</Group2>


////////////////////////////////////<Group3>

void ExecuteCodeBlock::methodGroup_TEST_Ib()
{
	t_nubit8 tmp = d_nubit8(pdeCodeBlock->prm);
	this->atomMethod_AND_8bit(tmp, pdeCodeBlock->immData_8Bit);
}
void ExecuteCodeBlock::methodGroup_TEST_Iv()
{
	t_nubit16 tmp = d_nubit16(pdeCodeBlock->prm);
	this->atomMethod_AND_16bit(tmp, pdeCodeBlock->immData_16Bit);
}
void ExecuteCodeBlock::methodGroup_NOT_8bit()
{
	d_nubit8(pdeCodeBlock->prm) = ~d_nubit8(pdeCodeBlock->prm);
}
void ExecuteCodeBlock::methodGroup_NOT_16bit()
{
	d_nubit16(pdeCodeBlock->prm) = ~d_nubit16(pdeCodeBlock->prm);
}
void ExecuteCodeBlock::methodGroup_NEG_8bit()
{
	//methodGroup_NOT_8bit();//先求反
	//this->atomMethod_ADD_8bit(d_nubit8(pdeCodeBlock->prm), 1);//再加上一
	t_nubit8 zero = 0;
	this->atomMethod_SUB_8bit(zero, d_nubit8(pdeCodeBlock->prm));
	d_nubit8(pdeCodeBlock->prm) = zero;
}
void ExecuteCodeBlock::methodGroup_NEG_16bit()
{
	//methodGroup_NOT_16bit();//先求反
	//this->atomMethod_ADD_16bit(d_nubit16(pdeCodeBlock->prm), 1);//再加上一
	t_nubit16 zero = 0;
	this->atomMethod_SUB_16bit(zero, d_nubit16(pdeCodeBlock->prm));
	d_nubit16(pdeCodeBlock->prm) = zero;
}
void ExecuteCodeBlock::methodGroup_MUL_AL()
{
	pCPU->ax = d_nubit8(pdeCodeBlock->prm) * pCPU->al;

	pCPU->setCF_Flag(!!(pCPU->ah));
	pCPU->setOF_Flag(!!(pCPU->ah));

}
void ExecuteCodeBlock::methodGroup_MUL_eAX()
{
	t_nubit32 result = d_nubit16(pdeCodeBlock->prm) * pCPU->ax;
	pCPU->dx = t_nubit16(result >> 16);
	pCPU->ax = t_nubit16(result & 0x0000ffff);

	pCPU->setCF_Flag(!!(pCPU->dx));
	pCPU->setOF_Flag(!!(pCPU->dx));
}
void ExecuteCodeBlock::methodGroup_IMUL_AL()
{
	pCPU->ax = d_nsbit8(pdeCodeBlock->prm) * (t_nsbit8)(pCPU->al);
	if(pCPU->ax == pCPU->al)
	{
		pCPU->setCF_Flag(false);
		pCPU->setOF_Flag(false);
	}
	else
	{
		pCPU->setCF_Flag(true);
		pCPU->setOF_Flag(true);
	}
}
void ExecuteCodeBlock::methodGroup_IMUL_eAX()
{
	t_nsbit32 result = d_nsbit16(pdeCodeBlock->prm) * (t_nsbit16)(pCPU->ax);
	pCPU->dx = t_nubit16(result >> 16);
	pCPU->ax = t_nubit16(result & 0x0000ffff);
	if(result == (t_nsbit32)(pCPU->ax))
	{
		pCPU->setCF_Flag(false);
		pCPU->setOF_Flag(false);
	}
	else
	{
		pCPU->setCF_Flag(true);
		pCPU->setOF_Flag(true);
	}
}
void ExecuteCodeBlock::methodGroup_DIV_AL()
{
	if(0 == (d_nubit8(pdeCodeBlock->prm)) )
	{//如果除数是零，产生零号中断
		this->atomMethod_INT(0);
		return ;
	}
	t_nubit16 tmpAX_unsigned = pCPU->ax;
	pCPU->al = tmpAX_unsigned / (d_nubit8(pdeCodeBlock->prm)); //商
	pCPU->ah = tmpAX_unsigned % (d_nubit8(pdeCodeBlock->prm));//余数
}
void ExecuteCodeBlock::methodGroup_DIV_eAX()
{
	if(0 == (d_nubit16(pdeCodeBlock->prm)) )
	{//如果除数是零，产生零号中断
		this->atomMethod_INT(0);
		return ;
	}

	t_nubit32 data = (((t_nubit32)(pCPU->dx))<<16) + pCPU->ax;
	pCPU->ax = data / (d_nubit16(pdeCodeBlock->prm)); //商
	pCPU->dx = data % (d_nubit16(pdeCodeBlock->prm));//余数

}
void ExecuteCodeBlock::methodGroup_IDIV_AL()
{
	if(0 == (d_nsbit8(pdeCodeBlock->prm)) )
	{//如果除数是零，产生零号中断
		this->atomMethod_INT(0);
		return ;
	}
	t_nsbit16 tmpAX_signed = pCPU->ax;
	pCPU->al = tmpAX_signed / (d_nsbit8(pdeCodeBlock->prm)); //商
	pCPU->ah = tmpAX_signed % (d_nsbit8(pdeCodeBlock->prm));//余数
}
void ExecuteCodeBlock::methodGroup_IDIV_eAX()
{
	if(0 == (d_nsbit16(pdeCodeBlock->prm)) )
	{//如果除数是零，产生零号中断
		this->atomMethod_INT(0);
		return ;
	}

	t_nsbit32 data = (((t_nubit32)(pCPU->dx))<<16) + pCPU->ax;
	pCPU->ax = data / (d_nsbit16(pdeCodeBlock->prm)); //商
	pCPU->dx = data % (d_nsbit16(pdeCodeBlock->prm));//余数
}



//////////////////////////////////////</Group3>
//////////////////////////////////////////<Group4--5>
void ExecuteCodeBlock::methodGroup_INC_Eb()
{
	this->atomMethod_INC_8bit(d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_INC_Ev()
{
	this->atomMethod_INC_16bit(d_nubit16(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_DEC_Eb()
{
	this->atomMethod_DEC_8bit(d_nubit8(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_DEC_Ev()
{
	this->atomMethod_DEC_16bit(d_nubit16(pdeCodeBlock->prm));
}
void ExecuteCodeBlock::methodGroup_CALL_Ev()//nnn=010 段内间接调用
{
	this->atomMethod_PUSH(pCPU->ip); // 没有PUSH IP 。。所以用这个来代替
	pCPU->ip = d_nubit16(pdeCodeBlock->prm);//near call absolute
}
void ExecuteCodeBlock::methodGroup_CALL_Ep()//nnn=011 段间间接调用
{
	this->atomMethod_PUSH(pCPU->cs);
	this->atomMethod_PUSH(pCPU->ip); // 没有PUSH IP 。。所以用这个来代替

	this->pCPU->cs = *(((t_nubit16*)pdeCodeBlock->prm) + 1);
	this->pCPU->ip = d_nubit16(pdeCodeBlock->prm);
}
void ExecuteCodeBlock::methodGroup_JMP_Ev()//段内间接转移
{
	pCPU->ip = d_nubit16(pdeCodeBlock->prm);
}
void ExecuteCodeBlock::methodGroup_JMP_Ep()//段间间接转移
{
	this->pCPU->cs = *(((t_nubit16*)pdeCodeBlock->prm) + 1);
	this->pCPU->ip = d_nubit16(pdeCodeBlock->prm);

}
void ExecuteCodeBlock::methodGroup_PUSH_Ev()
{
	this->atomMethod_PUSH(d_nubit16(pdeCodeBlock->prm));
}
	

//////////////////////////////////////////</Group4--5>