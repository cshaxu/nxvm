
#ifndef NXVM_CCPUAPI_H
#define NXVM_CCPUAPI_H

#include "../vglobal.h"
#include "../vcpu.h"
#include "../vram.h"

#define cramIsAddrInMem(addr) \
	(((addr) >= cram.base) && ((addr) < (cram.base + cram.size)))
#define cramGetAddr(segment, offset)  (cram.base + \
	((((segment) << 4) + (offset)) % cram.size))
#define cramVarByte(segment, offset)  (d_nubit8(cramGetAddr(segment, offset)))
#define cramVarWord(segment, offset)  (d_nubit16(cramGetAddr(segment, offset)))
#define cramVarDWord(segment, offset) (d_nubit32(cramGetAddr(segment, offset)))

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

extern DeCodeBlock ccpuins_deCodeBlock;
extern t_faddrcc ccpuins_repeatMethod;
extern t_bool ccpu_flagignore;
extern t_nubitcc ccpu_icount;
extern t_cpu ccpu;
#if DEBUGMODE == BCRM
extern t_ram cram;
#else
#define cram vram
#endif

void ccpuapiDebugPrintRegs();
void ccpuapiSyncRegs();
void ccpuapiExecIns();
void ccpuapiExecInt(t_nubit8 intid);
t_bool ccpuapiHasDiff();

void ccpuapiInit();
void ccpuapiFinal();

#endif
