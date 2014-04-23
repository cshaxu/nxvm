
#include "../vmachine.h"
#include "../vapi.h"
#include "ccpuapi.h"

void ins_atomMethod_MOV_8bit(t_nubit32 logicAddr_des, const t_nubit8 data_src)
{
#if CCPU_RAM == VRAM
	if (im(vramGetAddr(0, logicAddr_des))) return;
#endif
	d_nubit8(vramGetAddr(0, logicAddr_des)) = data_src;
}
void ins_atomMethod_MOV_16bit(t_nubit32 logicAddr_des, const t_nubit16 data_src)
{
#if CCPU_RAM == VRAM
	if (im(vramGetAddr(0, logicAddr_des))) return;
#endif
	d_nubit16(vramGetAddr(0, logicAddr_des)) = data_src;
}

void ins_atomMethod_ADD_8bit(t_nubit8 *pdes, const t_nubit8 src)
{
	t_nubit8 operand1 = *pdes;
	*pdes += src;
	ccpu_storeCaculate(ADD_8bit, 8, 
							operand1,  src, *pdes,
							MASK_FLAG_METHOD_ADD);
#if CCPU_RAM == VRAM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ins_atomMethod_ADD_16bit(t_nubit16 *pdes, const t_nubit16 src)
{
	t_nubit16 operand1 = *pdes;
	*pdes += src;
	ccpu_storeCaculate(ADD_16bit, 16, 
							operand1,  src, *pdes,
							MASK_FLAG_METHOD_ADD);
#if CCPU_RAM == VRAM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ins_atomMethod_INC_8bit(t_nubit8 *pdes)
{
	t_nubit8 operand1 = *pdes;
	(*pdes)++;
	ccpu_storeCaculate(ADD_8bit, 8,
							operand1, 1, *pdes,
							MASK_FLAG_METHOD_INC);
#if CCPU_RAM == VRAM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ins_atomMethod_INC_16bit(t_nubit16 *pdes)
{
	t_nubit16 operand1 = *pdes;
	(*pdes)++;
	ccpu_storeCaculate(ADD_16bit, 16,
							operand1, 1, *pdes,
							MASK_FLAG_METHOD_INC);
#if CCPU_RAM == VRAM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ins_atomMethod_ADC_8bit(t_nubit8 *pdes, const t_nubit8 src)
{
	t_nubit8 operand1 = *pdes;
	if(ccpu_getCF_Flag())
	{
		*pdes += src +1;
		ccpu_storeCaculate(ADC_8bit, 8, 
							operand1,  src, *pdes,//参考bochs作法..悲剧的ADC
							MASK_FLAG_METHOD_ADD);
	}
	else
	{
		*pdes += src;
		ccpu_storeCaculate(ADD_8bit, 8, 
							operand1,  src, *pdes,//如果CF为假时候则相当于执行ADD操作
							MASK_FLAG_METHOD_ADD);
	}
#if CCPU_RAM == VRAM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ins_atomMethod_ADC_16bit(t_nubit16 *pdes, const t_nubit16 src)
{
	t_nubit16 operand1 = *pdes;
	if(ccpu_getCF_Flag())
	{
		*pdes += src +1;
		ccpu_storeCaculate(ADC_16bit, 16, 
							operand1,  src, *pdes,//参考bochs作法..悲剧的ADC
							MASK_FLAG_METHOD_ADD);
	}
	else
	{
		*pdes += src;
		ccpu_storeCaculate(ADD_16bit, 16, 
							operand1,  src, *pdes,//如果CF为假时候则相当于执行ADD操作
							MASK_FLAG_METHOD_ADD);
	}
#if CCPU_RAM == VRAM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ins_atomMethod_SUB_8bit(t_nubit8 *pdes, const t_nubit8 src)
{
	t_nubit8 operand1 = *pdes;
	*pdes -= src;
	ccpu_storeCaculate(SUB_8bit, 8, 
							operand1,  src, *pdes,
							MASK_FLAG_METHOD_SUB);
#if CCPU_RAM == VRAM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ins_atomMethod_SUB_16bit(t_nubit16 *pdes, const t_nubit16 src)
{
	t_nubit16 operand1 = *pdes;
	*pdes -= src;
	ccpu_storeCaculate(SUB_16bit, 16, 
							operand1,  src, *pdes,
							MASK_FLAG_METHOD_SUB);
#if CCPU_RAM == VRAM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ins_atomMethod_DEC_8bit(t_nubit8 *pdes)
{
	t_nubit8 operand1 = *pdes;
	(*pdes)--;
	ccpu_storeCaculate(SUB_8bit, 8,
							operand1, 1, *pdes,
							MASK_FLAG_METHOD_DEC);
#if CCPU_RAM == VRAM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ins_atomMethod_DEC_16bit(t_nubit16 *pdes)
{
	t_nubit16 operand1 = *pdes;
	(*pdes)--;
	ccpu_storeCaculate(SUB_16bit, 16,
							operand1, 1, *pdes,
							MASK_FLAG_METHOD_DEC);
#if CCPU_RAM == VRAM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ins_atomMethod_SBB_8bit(t_nubit8 *pdes, const t_nubit8 src)
{
	t_nubit8 operand1 = *pdes;
	if(ccpu_getCF_Flag())
	{
		*pdes -= src +1;
		ccpu_storeCaculate(SBB_8bit, 8, 
							operand1,  src, *pdes,//参考bochs作法..悲剧的SBB
							MASK_FLAG_METHOD_ADD);
	}
	else
	{
		*pdes -= src;
		ccpu_storeCaculate(SUB_8bit, 8, 
							operand1,  src, *pdes,//如果CF为假时候则相当于执行SUB操作
							MASK_FLAG_METHOD_ADD);
	}
#if CCPU_RAM == VRAM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ins_atomMethod_SBB_16bit(t_nubit16 *pdes, const t_nubit16 src)
{
	t_nubit16 operand1 = *pdes;
	if(ccpu_getCF_Flag())
	{
		*pdes -= src +1;
		ccpu_storeCaculate(SBB_16bit, 16, 
							operand1,  src, *pdes,//参考bochs作法..悲剧的SBB
							MASK_FLAG_METHOD_ADD);
	}
	else
	{
		*pdes -= src;
/* NEKO DEBUG: SUB_16bit -> SBB_16bit*/
		ccpu_storeCaculate(SBB_16bit, 16, 
							operand1,  src, *pdes,//如果CF为假时候则相当于执行SUB操作
							MASK_FLAG_METHOD_ADD);
	}
#if CCPU_RAM == VRAM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ins_atomMethod_OR_8bit(t_nubit8 *pdes, const t_nubit8 src)
{
	t_nubit8 operand1 = *pdes;
	*pdes |= src;
	ccpu_storeCaculate(OR_8bit, 8, 
							operand1,  src, *pdes,
							MASK_FLAG_METHOD_OR);
	ccpu_setOF_Flag_flag(0x00);
	ccpu_setCF_Flag_flag(0x00);
	ccpu_setAF_Flag_flag(0x00);//Intel中说是不影响，但是debug中会影响
#if CCPU_RAM == VRAM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ins_atomMethod_OR_16bit(t_nubit16 *pdes, const t_nubit16 src)
{
	t_nubit16 operand1 = *pdes;
	*pdes |= src;
	ccpu_storeCaculate(OR_16bit, 16, 
							operand1,  src, *pdes,
							MASK_FLAG_METHOD_OR);
	ccpu_setOF_Flag_flag(0x00);
	ccpu_setCF_Flag_flag(0x00);
	ccpu_setAF_Flag_flag(0x00);//Intel中说是不影响，但是debug中会影响
#if CCPU_RAM == VRAM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ins_atomMethod_AND_8bit(t_nubit8 *pdes, const t_nubit8 src)
{
	t_nubit8 operand1 = *pdes;
	*pdes &= src;
	ccpu_storeCaculate(AND_8bit, 8, 
							operand1,  src, *pdes,
							MASK_FLAG_METHOD_AND);
	ccpu_setOF_Flag_flag(0x00);
	ccpu_setCF_Flag_flag(0x00);
	ccpu_setAF_Flag_flag(0x00);//Intel中说是不影响，但是debug中会影响
#if CCPU_RAM == VRAM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ins_atomMethod_AND_16bit(t_nubit16 *pdes, const t_nubit16 src)
{
	t_nubit16 operand1 = *pdes;
	*pdes &= src;
	ccpu_storeCaculate(AND_16bit, 16, 
							operand1,  src, *pdes,
							MASK_FLAG_METHOD_AND);
	ccpu_setOF_Flag_flag(0x00);
	ccpu_setCF_Flag_flag(0x00);
	ccpu_setAF_Flag_flag(0x00);//Intel中说是不影响，但是debug中会影响
#if CCPU_RAM == VRAM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ins_atomMethod_TEST_8bit(t_nubit8 *pdes, const t_nubit8 src)
{
	t_nubit8 operand1 = *pdes;
	ccpu_storeCaculate(AND_8bit, 8, 
							operand1,  src, (*pdes & src),
							MASK_FLAG_METHOD_AND);
	ccpu_setOF_Flag_flag(0x00);
	ccpu_setCF_Flag_flag(0x00);
	ccpu_setAF_Flag_flag(0x00);//Intel中说是不影响，但是debug中会影响
}
void ins_atomMethod_TEST_16bit(t_nubit16 *pdes, const t_nubit16 src)
{
	t_nubit16 operand1 = *pdes;
	ccpu_storeCaculate(AND_16bit, 16, 
							operand1,  src, (*pdes & src),
							MASK_FLAG_METHOD_AND);
	ccpu_setOF_Flag_flag(0x00);
	ccpu_setCF_Flag_flag(0x00);
	ccpu_setAF_Flag_flag(0x00);//Intel中说是不影响，但是debug中会影响
}
void ins_atomMethod_XOR_8bit(t_nubit8 *pdes, const t_nubit8 src)
{
	t_nubit8 operand1 = *pdes;
	*pdes ^= src;
	ccpu_storeCaculate(XOR_8bit, 8, 
							operand1,  src, *pdes,
							MASK_FLAG_METHOD_XOR);
	ccpu_setOF_Flag_flag(0x00);
	ccpu_setCF_Flag_flag(0x00);
	ccpu_setAF_Flag_flag(0x00);//Intel中说是不影响，但是debug中会影响
#if CCPU_RAM == VRAM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ins_atomMethod_XOR_16bit(t_nubit16 *pdes, const t_nubit16 src)
{
	t_nubit16 operand1 = *pdes;
	*pdes ^= src;
	ccpu_storeCaculate(XOR_16bit, 16, 
							operand1,  src, *pdes,
							MASK_FLAG_METHOD_XOR);
	ccpu_setOF_Flag_flag(0x00);
	ccpu_setCF_Flag_flag(0x00);
	ccpu_setAF_Flag_flag(0x00);//根据debug得出的，虽然Intel的文档说是undefine
#if CCPU_RAM == VRAM
	if (im(pdes)) *pdes = operand1;
#endif
}
void ins_atomMethod_CMP_8bit(t_nubit8 des, const t_nubit8 src)
{
	t_nubit8 result = des - src;
	ccpu_storeCaculate(CMP_8bit, 8, 
							des,  src, result,
							MASK_FLAG_METHOD_CMP);
}
void ins_atomMethod_CMP_16bit(t_nubit16 des, const t_nubit16 src)
{
	t_nubit16 result = des - src;
	ccpu_storeCaculate(CMP_16bit, 16, 
							des,  src, result,
							MASK_FLAG_METHOD_CMP);
}
void ins_atomMethod_CMPSB()
{
	t_nubit8 des, src;
	des = getMemData_byte(ccpu.di + (ccpu.es<<4));
	src = getMemData_byte(ccpu.si + (decode_getDefaultSegment_DS() <<4));
	ins_atomMethod_CMP_8bit(src, des);
	ins_self_string_affect_DI_SI(dataType_U1);
}
void ins_atomMethod_CMPSW()
{
	t_nubit16 des, src;
	des = getMemData_word(ccpu.di + (ccpu.es<<4) );
	src = getMemData_word(ccpu.si + (decode_getDefaultSegment_DS() <<4));
	ins_atomMethod_CMP_16bit(src, des);
	ins_self_string_affect_DI_SI(dataType_U2);
}
void ins_atomMethod_MOVSB()
{
	t_nubit8 data = getMemData_byte(ccpu.si + (decode_getDefaultSegment_DS() <<4));
#if CCPU_RAM == CRAM
	writeMem_byte(ccpu.di + (ccpu.es<<4), data);
#endif
	ins_self_string_affect_DI_SI(dataType_U1); //串操作对DI， SI修改
}
void ins_atomMethod_MOVSW()
{
	t_nubit16 data = getMemData_word(ccpu.si + (decode_getDefaultSegment_DS() <<4));
#if CCPU_RAM == CRAM
	writeMem_word(ccpu.di + (ccpu.es<<4), data);
#endif
	ins_self_string_affect_DI_SI(dataType_U2);//串操作对DI， SI修改
}

void ins_atomMethod_STOSB()
{
#if CCPU_RAM == CRAM
	writeMem_byte(ccpu.di + (ccpu.es<<4), ccpu.al);
#endif
	ins_self_string_affect_DI(dataType_U1);//串操作对DI修改
}
void ins_atomMethod_STOSW()
{
#if CCPU_RAM == CRAM
	writeMem_word(ccpu.di + (ccpu.es<<4), ccpu.ax);
#endif
	ins_self_string_affect_DI(dataType_U2);//串操作对DI修改
}
void ins_atomMethod_LODSB()
{
	ccpu.al = getMemData_byte(ccpu.si + (decode_getDefaultSegment_DS()<<4));
	ins_self_string_affect_SI(dataType_U1);//串操作对 SI修改
}
void ins_atomMethod_LODSW()
{
	ccpu.ax = getMemData_word(ccpu.si + (decode_getDefaultSegment_DS()<<4));
	ins_self_string_affect_SI(dataType_U2);//串操作对 SI修改
}
void ins_atomMethod_SCASB()
{
	ins_atomMethod_CMP_8bit(ccpu.al, getMemData_byte(ccpu.di + (ccpu.es<<4)));
	ins_self_string_affect_DI(dataType_U1);//串操作对DI， SI修改
}
void ins_atomMethod_SCASW()
{
	ins_atomMethod_CMP_16bit(ccpu.ax, getMemData_word(ccpu.di + (ccpu.es<<4)));
	ins_self_string_affect_DI(dataType_U2);//串操作对DI， SI修改
}
void ins_atomMethod_INS() {}
void ins_atomMethod_OUTS() {}

void ins_atomMethod_shortJMP()
{
	ccpu.ip += (t_nsbit8)(ccpu.deCodeBlock.opContactData_8bit);
}
void ins_atomMethod_PUSH(const t_nubit16 data)
{
	ccpu.sp -= 2;
#if CCPU_RAM == CRAM
	writeMem_word((ccpu.sp + ((ccpu.ss)<<4)), data);
#endif
}
t_nubit16   ins_atomMethod_POP()
{
	t_nubit16 data = getMemData_word((ccpu.sp + ((ccpu.ss)<<4)));
	ccpu.sp += 2;
	return data;
}
void ins_atomMethod_INT(const t_nubit8 INT_num)
{
	if (INT_num == 0x16) vapiPrint("...");
	ins_atomMethod_PUSH(ccpu_generateFLAG());//FLAGS入栈	
	ins_atomMethod_PUSH(ccpu.cs);
	ins_atomMethod_PUSH(ccpu.ip);
	ccpu_setIF_Flag_flag(0x00);
	ccpu_setTF_Flag_flag(0x00); //禁止硬件中断的单步中断
	ccpu.ip = getMemData_word(INT_num*4);
	ccpu.cs = getMemData_word(INT_num*4 + 2);
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
	if(ccpu_getDF_Flag())
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
	if(ccpu_getDF_Flag())
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
	if(ccpu_getDF_Flag())
		ccpu.di-=step;
	else
		ccpu.di+=step;

}