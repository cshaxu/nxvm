
#include "../lcpuins.h"
#include "../vapi.h"

void showMessage(string msg) {vapiPrint("%s\n",msg.c_str());}

void CentreProcessorUnit::init()
{
	OF_Flag=DF_Flag=IF_Flag=TF_Flag=SF_Flag=
		ZF_Flag=AF_Flag= PF_Flag= CF_Flag=false;
	SF_Flag=CF_Flag=AF_Flag=false;
	isOFCaculated= isDFCaculated= isIFCaculated= isTFCaculated=	isSFCaculated=
		isZFCaculated=isAFCaculated= isPFCaculated= isCFCaculated = false;
	this->flag_extern = 0;
	this->needCaculateFlag_mask = 0;
	icount = 0;flagterm = 0;
	ax = bx = cx = dx = sp = bp = si = di = cs = ds = es = ss = iobyte = ip = sp = 0;
	overss = overds = 0;
	this->deCodeBlock.init();
	this->exeCodeBlock.init();
}

void CentreProcessorUnit::storeCaculate(const Method opMethod,
	const int bitLength, const t_nubit32 operand1, const t_nubit32 operand2,
	const t_nubit32 result, const t_nubit16 mask_Flag)
{
	if(mask_Flag & MASK_FLAG_SF)
	{
		this->SF_lazyTable.bitLength = bitLength;
		this->SF_lazyTable.result = result;
	}
	if(mask_Flag & MASK_FLAG_ZF)
	{
		this->ZF_lazyTable.result = result;
	}
	if(mask_Flag & MASK_FLAG_PF)
	{
		this->PF_lazyTable.opMethod = opMethod;
		this->PF_lazyTable.bitLength = bitLength;
		this->PF_lazyTable.operand1 = operand1;
		this->PF_lazyTable.operand2 = operand2;
		this->PF_lazyTable.result = result;
	}
	if(mask_Flag & MASK_FLAG_CF)
	{
		this->CF_lazyTable.opMethod = opMethod;
		this->CF_lazyTable.bitLength = bitLength;
		this->CF_lazyTable.operand1 = operand1;
		this->CF_lazyTable.operand2 = operand2;
		this->CF_lazyTable.result = result;
	}
	if(mask_Flag & MASK_FLAG_OF)
	{
		this->OF_lazyTable.opMethod = opMethod;
		this->OF_lazyTable.bitLength = bitLength;
		this->OF_lazyTable.operand1 = operand1;
		this->OF_lazyTable.operand2 = operand2;
		this->OF_lazyTable.result = result;
	}
	if(mask_Flag & MASK_FLAG_AF)
	{
		this->AF_lazyTable.opMethod = opMethod;
		this->AF_lazyTable.bitLength = bitLength;
		this->AF_lazyTable.operand1 = operand1;
		this->AF_lazyTable.operand2 = operand2;
		this->AF_lazyTable.result = result;
	}

	this->needCaculateFlag_mask |= mask_Flag;
}

bool CentreProcessorUnit::getOF_Flag()
{//将会继续添加
	if( !(needCaculateFlag_mask & MASK_FLAG_OF) )
		return this->OF_Flag;
	needCaculateFlag_mask &= ~(MASK_FLAG_OF) ;
	
	switch(OF_lazyTable.opMethod)
	{
	case ADD_8bit:
	case ADC_8bit:
		if( ((OF_lazyTable.operand1&MASK_10000000) == (OF_lazyTable.operand2&MASK_10000000)) &&
			((OF_lazyTable.operand1&MASK_10000000) !=(OF_lazyTable.result&MASK_10000000)))
			return OF_Flag = true;
		else
			return OF_Flag = false;
		break;
	case ADD_16bit:
	case ADC_16bit:
		if( ((OF_lazyTable.operand1&0x8000) == (OF_lazyTable.operand2&0x8000)) &&
			((OF_lazyTable.operand1&0x8000) !=(OF_lazyTable.result&0x8000)))
			return OF_Flag = true;
		else
			return OF_Flag = false;
		break;
	case SUB_8bit:
	case SBB_8bit:
	case CMP_8bit:
		if( ((OF_lazyTable.operand1&MASK_10000000) != (OF_lazyTable.operand2&MASK_10000000)) && 
			((OF_lazyTable.operand2&MASK_10000000) ==(OF_lazyTable.result&MASK_10000000)))
			return OF_Flag = true;
		else
			return OF_Flag = false;
		break;
	case SUB_16bit:
	case SBB_16bit:
	case CMP_16bit:
		if( ((OF_lazyTable.operand1&0x8000) != (OF_lazyTable.operand2&0x8000)) &&
			((OF_lazyTable.operand2&0x8000) ==(OF_lazyTable.result&0x8000)))
			return OF_Flag = true;
		else
			return OF_Flag = false;
		break;
	default:
		showMessage("OF flags lazy comput wrong");
		assert(false);
		break;
	}
		return false;
}
bool CentreProcessorUnit::getDF_Flag()
{
	//assert(false);简单运算，都会没有什么函数会难为它的~~
	if( !(needCaculateFlag_mask & MASK_FLAG_DF) )
		return this->DF_Flag;
	needCaculateFlag_mask &= ~(MASK_FLAG_DF) ;
		return this->DF_Flag;
}
bool CentreProcessorUnit::getIF_Flag()
{
	//assert(false);简单运算，都会没有什么函数会难为它的~~
	if( !(needCaculateFlag_mask & MASK_FLAG_IF) )
		return this->IF_Flag;
	needCaculateFlag_mask &= ~(MASK_FLAG_IF) ;
		return this->IF_Flag;
}
bool CentreProcessorUnit::getTF_Flag()
{
	//assert(false);简单运算，都会没有什么函数会难为它的~~
	if( !(needCaculateFlag_mask & MASK_FLAG_TF) )
		return this->TF_Flag;
	needCaculateFlag_mask &= ~(MASK_FLAG_TF) ;
		return this->TF_Flag;
}
bool CentreProcessorUnit::getSF_Flag()
{
	if( !(needCaculateFlag_mask & MASK_FLAG_SF) )
		return this->SF_Flag;
	needCaculateFlag_mask &= ~(MASK_FLAG_SF) ;
	switch(SF_lazyTable.bitLength)
	{
	case 8:
		SF_Flag = !!(SF_lazyTable.result & 0x0080);
		break;
	case 16:
		SF_Flag = !!(SF_lazyTable.result & 0x8000);
		break;
	default:
		showMessage("when comput SF flags , found the bitLength out of handle ");
		assert(false);
		break;
	}
	return this->SF_Flag;
}
bool CentreProcessorUnit::getZF_Flag()
{
	if( !(needCaculateFlag_mask & MASK_FLAG_ZF) )
		return this->ZF_Flag;
	needCaculateFlag_mask &= ~(MASK_FLAG_ZF) ;
	return ZF_Flag = !ZF_lazyTable.result;
}
bool CentreProcessorUnit::getAF_Flag()
{
	if( !(needCaculateFlag_mask & MASK_FLAG_AF) )
		return this->AF_Flag;
	needCaculateFlag_mask &= ~(MASK_FLAG_AF) ;
	bool operand1_thirdBit, operand2_thirdBit, result_thirdBit;
	operand1_thirdBit = !!(AF_lazyTable.operand1 & MASK_00001000);
	operand2_thirdBit = !!(AF_lazyTable.operand2 & MASK_00001000);
	result_thirdBit   = !!(AF_lazyTable.result   & MASK_00001000);
	switch(AF_lazyTable.opMethod)
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
		AF_Flag = !!(((AF_lazyTable.operand1 ^ AF_lazyTable.operand2) ^ AF_lazyTable.result) & 0x10);
		break;
	default:
		showMessage("when comput AF flags , found the bitLength out of handle");
		assert(false);
		break;
	}
	return AF_Flag;
}
bool CentreProcessorUnit::getPF_Flag()
{
	if( !(needCaculateFlag_mask & MASK_FLAG_PF) )
		return this->PF_Flag;
	needCaculateFlag_mask &= ~(MASK_FLAG_PF) ;
	int oneNum = 0;
	t_nubit8 result_8bit = (t_nubit8)(PF_lazyTable.result);

	while(result_8bit)
	{
		result_8bit &= (result_8bit-1); 
		oneNum++;

	}
	return PF_Flag=(0==oneNum%2);//运算结果中的低8位有偶数个1，PF为true； 奇数个1，PF为false
}
bool CentreProcessorUnit::getCF_Flag()
{
	if( !(needCaculateFlag_mask & MASK_FLAG_CF) )
		return this->CF_Flag;
	needCaculateFlag_mask &= ~(MASK_FLAG_CF) ;
	switch(CF_lazyTable.opMethod)
	{//还会后续添加
	case ADD_8bit:
	case ADD_16bit:
		if( CF_lazyTable.result < CF_lazyTable.operand1 || CF_lazyTable.result < CF_lazyTable.operand2)
			return CF_Flag = true;
		else
			return CF_Flag = false;
		break;
	case ADC_8bit://当出现ADC时候，都是表示CF为真，如果CF为假，ADC操作执行的将是ADD操作，SBB同理~~
	case ADC_16bit:
		return CF_Flag =(CF_lazyTable.result <= CF_lazyTable.operand1);
		break;
	case SUB_8bit:
	case SUB_16bit:
	case CMP_8bit:
	case CMP_16bit:
		return CF_Flag = (CF_lazyTable.operand1 < CF_lazyTable.operand2);
		break;
	case SBB_8bit:
		return CF_Flag = (CF_lazyTable.operand1 < CF_lazyTable.result) || (0xff == CF_lazyTable.operand2);
		break;
	case SBB_16bit:
		return CF_Flag = (CF_lazyTable.operand1 < CF_lazyTable.result) || (0xffff == CF_lazyTable.operand2); 
		break;
	default:
		showMessage("when comput CF flags , found the bitLength out of handle");
		assert(false);
		break;
	}
	return CF_Flag;
}

void CentreProcessorUnit::setOF_Flag(const bool f)
{
	this->OF_Flag = f;
	this->needCaculateFlag_mask &= ~MASK_FLAG_OF;
}
void CentreProcessorUnit::setDF_Flag(const bool f)
{
	this->DF_Flag = f;
	this->needCaculateFlag_mask &= ~MASK_FLAG_DF;
}
void CentreProcessorUnit::setIF_Flag(const bool f)
{
	this->IF_Flag = f;
	this->needCaculateFlag_mask &= ~MASK_FLAG_IF;
}
void CentreProcessorUnit::setTF_Flag(const bool f)
{
	this->TF_Flag = f;
	this->needCaculateFlag_mask &= ~MASK_FLAG_TF;
}
void CentreProcessorUnit::setSF_Flag(const bool f)
{
	this->SF_Flag = f;
	this->needCaculateFlag_mask &= ~MASK_FLAG_SF;
}
void CentreProcessorUnit::setSF_Flag(const t_nubit8 data)
{
	 SF_Flag = !!(data & MASK_10000000);
	this->needCaculateFlag_mask &= ~MASK_FLAG_SF;
}
void CentreProcessorUnit::setSF_Flag(const t_nubit16 data)
{
	 SF_Flag = !!(data & 0x8000);
	this->needCaculateFlag_mask &= ~MASK_FLAG_SF;
}
void CentreProcessorUnit::setZF_Flag(const bool f)
{
	this->ZF_Flag = f;
	this->needCaculateFlag_mask &= ~MASK_FLAG_ZF;
}
void CentreProcessorUnit::setZF_Flag(const t_nubit8 data)
{
	this->ZF_Flag = !data;
	this->needCaculateFlag_mask &= ~MASK_FLAG_ZF;
}
void CentreProcessorUnit::setZF_Flag(const t_nubit16 data)
{
	this->ZF_Flag = !data;
	this->needCaculateFlag_mask &= ~MASK_FLAG_ZF;
}
void CentreProcessorUnit::setAF_Flag(const bool f)
{
	this->AF_Flag = f;
	this->needCaculateFlag_mask &= ~MASK_FLAG_AF;
}
void CentreProcessorUnit::setPF_Flag(const bool f)
{
	this->PF_Flag = f;
	this->needCaculateFlag_mask &= ~MASK_FLAG_PF;
}
void CentreProcessorUnit::setPF_Flag(const t_nubit8 data)
{
	int oneNum = 0;
	t_nubit8 result_8bit = (t_nubit8)(data);

	while(result_8bit)
	{
		result_8bit &= (result_8bit-1); 
		oneNum++;

	}
	PF_Flag=(0==oneNum%2);//运算结果中的低8位有偶数个1，PF为true； 奇数个1，PF为false
	this->needCaculateFlag_mask &= ~MASK_FLAG_PF;
}
void CentreProcessorUnit::setPF_Flag(const t_nubit16 data)
{
	setPF_Flag(t_nubit8(data));
}
void CentreProcessorUnit::setCF_Flag(const bool f)
{
	this->CF_Flag = f;
	this->needCaculateFlag_mask &= ~MASK_FLAG_CF;
}
void CentreProcessorUnit::setSF_ZF_PF_byResult(const t_nubit8 result)
{
	this->setZF_Flag(result);	
	this->setSF_Flag(result);
	this->setPF_Flag(result);

}
void CentreProcessorUnit::setSF_ZF_PF_byResult(const t_nubit16 result)
{
	this->setZF_Flag(result);	
	this->setSF_Flag(result);
	this->setPF_Flag(result);
}

t_nubit16 CentreProcessorUnit::generateFLAG()
{
	//t_nubit16 flags=0x7000 + 2;//根据bochs。。还有，flags的第二位总是1，所以加上2
	t_nubit16 flags = flag_extern&0xf000;
	//flags|=0x2;//第二位要为1
	flags&=0xffd7;//第四位要为0,第六位为0
	if(this->getOF_Flag())
		flags |= MASK_FLAG_OF;
	if(this->getDF_Flag())
		flags |= MASK_FLAG_DF;
	if(this->getIF_Flag())
		flags |= MASK_FLAG_IF;
	if(this->getTF_Flag())
		flags |= MASK_FLAG_TF;
	if(this->getSF_Flag())
		flags |= MASK_FLAG_SF;
	if(this->getZF_Flag())
		flags |= MASK_FLAG_ZF;
	if(this->getAF_Flag())
		flags |= MASK_FLAG_AF;
	if(this->getPF_Flag())
		flags |= MASK_FLAG_PF;
	if(this->getCF_Flag())
		flags |= MASK_FLAG_CF;
	
	return flags;

}
void CentreProcessorUnit::setFLAG(const t_nubit16 flags)
{
	this->setOF_Flag(!!(flags&MASK_FLAG_OF));
	this->setDF_Flag(!!(flags&MASK_FLAG_DF));
	this->setIF_Flag(!!(flags&MASK_FLAG_IF));
	this->setTF_Flag(!!(flags&MASK_FLAG_TF));
	this->setSF_Flag(!!(flags&MASK_FLAG_SF));
	this->setZF_Flag(!!(flags&MASK_FLAG_ZF));
	this->setAF_Flag(!!(flags&MASK_FLAG_AF));
	this->setPF_Flag(!!(flags&MASK_FLAG_PF));
	this->setCF_Flag(!!(flags&MASK_FLAG_CF));
	this->needCaculateFlag_mask = 0;//去不不需要再次计算
	flag_extern = flags;
}
void CentreProcessorUnit::set_SF_ZF_AF_PF_CF_Flag(const t_nubit16 flags)
{
	this->setSF_Flag(!!(flags&MASK_FLAG_SF));
	this->setZF_Flag(!!(flags&MASK_FLAG_ZF));
	this->setAF_Flag(!!(flags&MASK_FLAG_AF));
	this->setPF_Flag(!!(flags&MASK_FLAG_PF));
	this->setCF_Flag(!!(flags&MASK_FLAG_CF));
	this->needCaculateFlag_mask &= ~ (MASK_FLAG_SF|MASK_FLAG_ZF|MASK_FLAG_AF|MASK_FLAG_PF|MASK_FLAG_CF);
}