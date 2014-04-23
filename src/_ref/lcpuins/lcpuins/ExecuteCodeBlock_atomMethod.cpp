
#include "../vmachine.h"
#include "../lcpuins.h"
#include "CentreProcessorUnit.h"
//#include "../_main/MotherBoard.h"
void ExecuteCodeBlock::atomMethod_MOV_8bit(t_nubit32 logicAddr_des, const t_nubit8 data_src)
{
	d_nubit8(p_nubit8(vramGetAddr(0,0)) + logicAddr_des) = data_src;
}
void ExecuteCodeBlock::atomMethod_MOV_16bit(t_nubit32 logicAddr_des, const t_nubit16 data_src)
{
	d_nubit16(p_nubit8(vramGetAddr(0,0)) + logicAddr_des) = data_src;
}


void ExecuteCodeBlock::atomMethod_ADD_8bit(t_nubit8& des, const t_nubit8 src)
{
	t_nubit8 operand1 = des;
	des += src;
	pCPU->storeCaculate(ADD_8bit, 8, 
							operand1,  src, des,
							MASK_FLAG_METHOD_ADD);
}
void ExecuteCodeBlock::atomMethod_ADD_16bit(t_nubit16& des, const t_nubit16 src)
{
	t_nubit16 operand1 = des;
	des += src;
	pCPU->storeCaculate(ADD_16bit, 16, 
							operand1,  src, des,
							MASK_FLAG_METHOD_ADD);
}
void ExecuteCodeBlock::atomMethod_INC_8bit(t_nubit8& des)
{
	t_nubit8 operand1 = des;
	des++;
	pCPU->storeCaculate(ADD_8bit, 8,
							operand1, 1, des,
							MASK_FLAG_METHOD_INC);
}
void ExecuteCodeBlock::atomMethod_INC_16bit(t_nubit16& des)
{
	t_nubit16 operand1 = des;
	des++;
	pCPU->storeCaculate(ADD_16bit, 16,
							operand1, 1, des,
							MASK_FLAG_METHOD_INC);
}
void ExecuteCodeBlock::atomMethod_ADC_8bit(t_nubit8& des, const t_nubit8 src)
{
	t_nubit8 operand1 = des;
	if(pCPU->getCF_Flag())
	{
		des += src +1;
		pCPU->storeCaculate(ADC_8bit, 8, 
							operand1,  src, des,//参考bochs作法..悲剧的ADC
							MASK_FLAG_METHOD_ADD);
	}
	else
	{
		des += src;
		pCPU->storeCaculate(ADD_8bit, 8, 
							operand1,  src, des,//如果CF为假时候则相当于执行ADD操作
							MASK_FLAG_METHOD_ADD);
	}
}
void ExecuteCodeBlock::atomMethod_ADC_16bit(t_nubit16& des, const t_nubit16 src)
{
	t_nubit16 operand1 = des;
	if(pCPU->getCF_Flag())
	{
		des += src +1;
		pCPU->storeCaculate(ADC_16bit, 16, 
							operand1,  src, des,//参考bochs作法..悲剧的ADC
							MASK_FLAG_METHOD_ADD);
	}
	else
	{
		des += src;
		pCPU->storeCaculate(ADD_16bit, 16, 
							operand1,  src, des,//如果CF为假时候则相当于执行ADD操作
							MASK_FLAG_METHOD_ADD);
	}
}
void ExecuteCodeBlock::atomMethod_SUB_8bit(t_nubit8& des, const t_nubit8 src)
{
	t_nubit8 operand1 = des;
	des -= src;
	pCPU->storeCaculate(SUB_8bit, 8, 
							operand1,  src, des,
							MASK_FLAG_METHOD_SUB);
}
void ExecuteCodeBlock::atomMethod_SUB_16bit(t_nubit16& des, const t_nubit16 src)
{
	t_nubit16 operand1 = des;
	des -= src;
	pCPU->storeCaculate(SUB_16bit, 16, 
							operand1,  src, des,
							MASK_FLAG_METHOD_SUB);
}
void ExecuteCodeBlock::atomMethod_DEC_8bit(t_nubit8& des)
{
	t_nubit8 operand1 = des;
	des--;
	pCPU->storeCaculate(SUB_8bit, 8,
							operand1, 1, des,
							MASK_FLAG_METHOD_DEC);
}
void ExecuteCodeBlock::atomMethod_DEC_16bit(t_nubit16& des)
{
	t_nubit16 operand1 = des;
	des--;
	pCPU->storeCaculate(SUB_16bit, 16,
							operand1, 1, des,
							MASK_FLAG_METHOD_DEC);
}
void ExecuteCodeBlock::atomMethod_SBB_8bit(t_nubit8& des, const t_nubit8 src)
{
	t_nubit8 operand1 = des;
	if(pCPU->getCF_Flag())
	{
		des -= src +1;
		pCPU->storeCaculate(SBB_8bit, 8, 
							operand1,  src, des,//参考bochs作法..悲剧的SBB
							MASK_FLAG_METHOD_ADD);
	}
	else
	{
		des -= src;
		pCPU->storeCaculate(SUB_8bit, 8, 
							operand1,  src, des,//如果CF为假时候则相当于执行SUB操作
							MASK_FLAG_METHOD_ADD);
	}
}
void ExecuteCodeBlock::atomMethod_SBB_16bit(t_nubit16& des, const t_nubit16 src)
{
	t_nubit16 operand1 = des;
	if(pCPU->getCF_Flag())
	{
		des -= src +1;
		pCPU->storeCaculate(SBB_16bit, 16, 
							operand1,  src, des,//参考bochs作法..悲剧的SBB
							MASK_FLAG_METHOD_ADD);
	}
	else
	{
		des -= src;
		pCPU->storeCaculate(SUB_16bit, 16, 
							operand1,  src, des,//如果CF为假时候则相当于执行SUB操作
							MASK_FLAG_METHOD_ADD);
	}
}
void ExecuteCodeBlock::atomMethod_OR_8bit(t_nubit8& des, const t_nubit8 src)
{
	t_nubit8 operand1 = des;
	des |= src;
	pCPU->storeCaculate(OR_8bit, 8, 
							operand1,  src, des,
							MASK_FLAG_METHOD_OR);
	pCPU->setOF_Flag(false);
	pCPU->setCF_Flag(false);
	pCPU->setAF_Flag(false);//Intel中说是不影响，但是debug中会影响
}
void ExecuteCodeBlock::atomMethod_OR_16bit(t_nubit16& des, const t_nubit16 src)
{
	t_nubit16 operand1 = des;
	des |= src;
	pCPU->storeCaculate(OR_16bit, 16, 
							operand1,  src, des,
							MASK_FLAG_METHOD_OR);
	pCPU->setOF_Flag(false);
	pCPU->setCF_Flag(false);
	pCPU->setAF_Flag(false);//Intel中说是不影响，但是debug中会影响
}

void ExecuteCodeBlock::atomMethod_AND_8bit(t_nubit8& des, const t_nubit8 src)
{
	t_nubit8 operand1 = des;
	des &= src;
	pCPU->storeCaculate(AND_8bit, 8, 
							operand1,  src, des,
							MASK_FLAG_METHOD_AND);
	pCPU->setOF_Flag(false);
	pCPU->setCF_Flag(false);
	pCPU->setAF_Flag(false);//Intel中说是不影响，但是debug中会影响

}
void ExecuteCodeBlock::atomMethod_AND_16bit(t_nubit16& des, const t_nubit16 src)
{
	t_nubit16 operand1 = des;
	des &= src;
	pCPU->storeCaculate(AND_16bit, 16, 
							operand1,  src, des,
							MASK_FLAG_METHOD_AND);
	pCPU->setOF_Flag(false);
	pCPU->setCF_Flag(false);
	pCPU->setAF_Flag(false);//Intel中说是不影响，但是debug中会影响
}

void ExecuteCodeBlock::atomMethod_XOR_8bit(t_nubit8& des, const t_nubit8 src)
{
	t_nubit8 operand1 = des;
	des ^= src;
	pCPU->storeCaculate(XOR_8bit, 8, 
							operand1,  src, des,
							MASK_FLAG_METHOD_XOR);
	pCPU->setOF_Flag(false);
	pCPU->setCF_Flag(false);
	pCPU->setAF_Flag(false);//Intel中说是不影响，但是debug中会影响
}
void ExecuteCodeBlock::atomMethod_XOR_16bit(t_nubit16& des, const t_nubit16 src)
{
	t_nubit16 operand1 = des;
	des ^= src;
	pCPU->storeCaculate(XOR_16bit, 16, 
							operand1,  src, des,
							MASK_FLAG_METHOD_XOR);
	pCPU->setOF_Flag(false);
	pCPU->setCF_Flag(false);
	pCPU->setAF_Flag(false);//根据debug得出的，虽然Intel的文档说是undefine

}
void ExecuteCodeBlock::atomMethod_CMP_8bit(t_nubit8 des, const t_nubit8 src)
{
	t_nubit8 result = des - src;
	pCPU->storeCaculate(CMP_8bit, 8, 
							des,  src, result,
							MASK_FLAG_METHOD_CMP);
}
void ExecuteCodeBlock::atomMethod_CMP_16bit(t_nubit16 des, const t_nubit16 src)
{
	t_nubit16 result = des - src;
	pCPU->storeCaculate(CMP_16bit, 16, 
							des,  src, result,
							MASK_FLAG_METHOD_CMP);
}
void ExecuteCodeBlock::atomMethod_CMPSB()
{
	t_nubit8 des, src;
	des = getMemData_byte(pCPU->di + (pCPU->es<<4));
	src = getMemData_byte(pCPU->si + (pdeCodeBlock->getDefaultSegment_DS() <<4));
	atomMethod_CMP_8bit(src, des);
	self_string_affect_DI_SI(dataType_U1);
}
void ExecuteCodeBlock::atomMethod_CMPSW()
{
	t_nubit16 des, src;
	des = getMemData_word(pCPU->di + (pCPU->es<<4) );
	src = getMemData_word(pCPU->si + (pdeCodeBlock->getDefaultSegment_DS() <<4));
	atomMethod_CMP_16bit(src, des);
	self_string_affect_DI_SI(dataType_U2);
}
void ExecuteCodeBlock::atomMethod_MOVSB()
{
	t_nubit8 data = getMemData_byte(pCPU->si + (pdeCodeBlock->getDefaultSegment_DS() <<4));
	writeMem_byte(pCPU->di + (pCPU->es<<4), data);
	self_string_affect_DI_SI(dataType_U1); //串操作对DI， SI修改
}
void ExecuteCodeBlock::atomMethod_MOVSW()
{
	t_nubit16 data = getMemData_word(pCPU->si + (pdeCodeBlock->getDefaultSegment_DS() <<4));
	writeMem_word(pCPU->di + (pCPU->es<<4), data);
	self_string_affect_DI_SI(dataType_U2);//串操作对DI， SI修改
}

void ExecuteCodeBlock::atomMethod_STOSB()
{
	writeMem_byte(pCPU->di + (pCPU->es<<4), pCPU->al);
	self_string_affect_DI(dataType_U1);//串操作对DI修改
}
void ExecuteCodeBlock::atomMethod_STOSW()
{
	writeMem_word(pCPU->di + (pCPU->es<<4), pCPU->ax);
	self_string_affect_DI(dataType_U2);//串操作对DI修改
}
void ExecuteCodeBlock::atomMethod_LODSB()
{
	pCPU->al = getMemData_byte(pCPU->si + (pdeCodeBlock->getDefaultSegment_DS()<<4));
	self_string_affect_SI(dataType_U1);//串操作对 SI修改
}
void ExecuteCodeBlock::atomMethod_LODSW()
{
	pCPU->ax = getMemData_word(pCPU->si + (pdeCodeBlock->getDefaultSegment_DS()<<4));
	self_string_affect_SI(dataType_U2);//串操作对 SI修改
}
void ExecuteCodeBlock::atomMethod_SCASB()
{
	atomMethod_CMP_8bit(pCPU->al, getMemData_byte(pCPU->di + (pCPU->es<<4)));
	self_string_affect_DI(dataType_U1);//串操作对DI， SI修改
}
void ExecuteCodeBlock::atomMethod_SCASW()
{
	atomMethod_CMP_16bit(pCPU->ax, getMemData_word(pCPU->di + (pCPU->es<<4)));
	self_string_affect_DI(dataType_U2);//串操作对DI， SI修改
}
void ExecuteCodeBlock::atomMethod_INS() //80x86没有这个指令
{
	//need finish
}
void ExecuteCodeBlock::atomMethod_OUTS()//80x86没有这个指令
{
	//need finish
}





void ExecuteCodeBlock::atomMethod_shortJMP()
{
	
		pCPU->ip += (t_nsbit8)(pdeCodeBlock->opContactData_8bit);
}

void ExecuteCodeBlock::atomMethod_PUSH(const t_nubit16 data)
{
	pCPU->sp -= 2;
	writeMem_word((pCPU->sp + ((pCPU->ss)<<4)), data);
}

t_nubit16   ExecuteCodeBlock::atomMethod_POP()
{
	t_nubit16 data = getMemData_word((pCPU->sp + ((pCPU->ss)<<4)));
	pCPU->sp += 2;
	return data;
}



void ExecuteCodeBlock::atomMethod_INT(const t_nubit8 INT_num)
{
	this->atomMethod_PUSH(pCPU->generateFLAG());//FLAGS入栈	
	this->atomMethod_PUSH(pCPU->cs);
	this->atomMethod_PUSH(pCPU->ip);
	pCPU->setIF_Flag(false);
	pCPU->setTF_Flag(false); //禁止硬件中断的单步中断
	pCPU->ip = getMemData_word(INT_num*4);
	pCPU->cs = getMemData_word(INT_num*4 + 2);
}



void ExecuteCodeBlock::self_string_affect_DI_SI(const DataTypeLength type)
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
	if(pCPU->getDF_Flag())
	{
		pCPU->si-=step;
		pCPU->di-=step;
	}
	else
	{
		pCPU->si+=step;
		pCPU->di+=step;
	}
}
void ExecuteCodeBlock::self_string_affect_SI(const DataTypeLength type) 
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
	if(pCPU->getDF_Flag())
		pCPU->si-=step;
	else
		pCPU->si+=step;
}
void ExecuteCodeBlock::self_string_affect_DI(const DataTypeLength type) 
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
	if(pCPU->getDF_Flag())
		pCPU->di-=step;
	else
		pCPU->di+=step;

}