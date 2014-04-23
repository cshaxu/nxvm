/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "memory.h"

#include "vmachine.h"
#include "vapi.h"

/* General Functions */
t_nubit32 vapiPrint(const t_string format, ...)
{
	t_nubit32 nWrittenBytes = 0;
	va_list arg_ptr;
	va_start(arg_ptr, format);
	nWrittenBytes = vfprintf(stdout, format,arg_ptr);
	//nWrittenBytes = vsprintf(stringBuffer,format,arg_ptr);
	va_end(arg_ptr);
	return nWrittenBytes;
}
void vapiPrintIns(t_nubit16 segment, t_nubit16 offset, t_string ins)
{
	vapiPrint("%04X:%04X  %s\n",segment,offset,ins);
}

/* Record */

t_apirecord vapirecord;

#define _expression "cs:ip=%x:%x opcode=%x %x %x %x %x %x %x %x \
ax=%x bx=%x cx=%x dx=%x sp=%x bp=%x si=%x di=%x ds=%x es=%x ss=%x \
of=%1x sf=%1x zf=%1x cf=%1x af=%1x pf=%1x df=%1x if=%1x tf=%1x\n"
#define _rec (vapirecord.rec[(i + vapirecord.start) % VAPI_RECORD_SIZE])
#define _rec_of    (GetBit(_rec.flags, VCPU_FLAG_OF))
#define _rec_sf    (GetBit(_rec.flags, VCPU_FLAG_SF))
#define _rec_zf    (GetBit(_rec.flags, VCPU_FLAG_ZF))
#define _rec_cf    (GetBit(_rec.flags, VCPU_FLAG_CF))
#define _rec_af    (GetBit(_rec.flags, VCPU_FLAG_AF))
#define _rec_pf    (GetBit(_rec.flags, VCPU_FLAG_PF))
#define _rec_df    (GetBit(_rec.flags, VCPU_FLAG_DF))
#define _rec_tf    (GetBit(_rec.flags, VCPU_FLAG_TF))
#define _rec_if    (GetBit(_rec.flags, VCPU_FLAG_IF))

void vapiRecordDump(const t_string fname)
{
	t_nubitcc i = 0;
	FILE *dump = fopen(fname, "w");
	if (!dump) {
		vapiPrint("ERROR:\tcannot write dump file.\n");
		return;
	}
	if (!vapirecord.size) {
		vapiPrint("ERROR:\tno record to dump.\n");
		return;
	}
	while (i < vapirecord.size) {
		fprintf(dump, _expression,
			_rec.cs, _rec.ip,
			vramVarByte(_rec.cs,_rec.ip+0),vramVarByte(_rec.cs,_rec.ip+1),
			vramVarByte(_rec.cs,_rec.ip+2),vramVarByte(_rec.cs,_rec.ip+3),
			vramVarByte(_rec.cs,_rec.ip+4),vramVarByte(_rec.cs,_rec.ip+5),
			vramVarByte(_rec.cs,_rec.ip+6),vramVarByte(_rec.cs,_rec.ip+7),
			_rec.ax,_rec.bx,_rec.cx,_rec.dx,
			_rec.sp,_rec.bp,_rec.si,_rec.di,
			_rec.ds,_rec.es,_rec.ss,
			_rec_of,_rec_sf,_rec_zf,_rec_cf,
			_rec_af,_rec_pf,_rec_df,_rec_if,_rec_tf);
		++i;
	}
	fclose(dump);
}
void vapiRecordStart()
{
	vapirecord.start = 0;
	vapirecord.size = 0;
}
void vapiRecordExec()
{
#if VAPI_RECORD_SELECT_FIRST == 1
	if (vapirecord.size == VAPI_RECORD_SIZE) {
		vmachine.flagrecord = 0x00;
		return;
	}
#endif
	vapirecord.rec[(vapirecord.start + vapirecord.size) % VAPI_RECORD_SIZE]
		= vcpu;
	if (vapirecord.size == VAPI_RECORD_SIZE)
		vapirecord.start++;
	else vapirecord.size++;
}
void vapiRecordEnd() {}

/* Floppy Disk */
#include "vfdd.h"

void vapiFloppyInsert(const t_string fname)
{
	t_nubitcc count;
	FILE *image = fopen(fname, "rb");
	if (image) {
		count = fread((void *)vfdd.base, sizeof(t_nubit8), 0x00168000, image);
		vfdd.flagexist = 0x01;
		fclose(image);
		vapiPrint("Floppy disk inserted.\n");
	} else
		vapiPrint("Cannot read floppy image from '%s'.\n", fname);
}
void vapiFloppyRemove(const t_string fname)
{
	t_nubitcc count;
	FILE *image;
	if (fname) {
		image = fopen(fname, "wb");
		if(image) {
			if (!vfdd.flagro)
				count = fwrite((void *)vfdd.base, sizeof(t_nubit8), 0x00168000, image);
			vfdd.flagexist = 0x00;
			fclose(image);
		} else {
			vapiPrint("Cannot write floppy image to '%s'.\n", fname);
			return;
		}
	}
	vfdd.flagexist = 0x00;
	memset((void *)vfdd.base, 0x00, 0x00168000);
	vapiPrint("Floppy disk removed.\n");
}

/* Platform Related */
#if VGLOBAL_PLATFORM == VGLOBAL_VAR_WIN32
	#include "system/win32con.h"
	#include "system/win32app.h"
	void vapiSleep(t_nubit32 milisec) {Sleep(milisec);}
	void vapiDisplaySetScreen() {
		if (!vmachine.flagmode)
			win32conDisplaySetScreen();
		else
			win32appDisplaySetScreen();
	}
	void vapiDisplayPaint() {
		if (!vmachine.flagmode)
			win32conDisplayPaint();
		else
			win32appDisplayPaint();
	}
	void vapiStartMachine() {
		if (!vmachine.flagmode)
			win32conStartMachine();
		else
			win32appStartMachine();
	}
#elif VGLOBAL_PLATFORM == VGLOBAL_VAR_LINUX
	#include "system.h"
	#include "system/linuxcon.h"
	#include "system/linuxapp.h"
	void vapiSleep(t_nubit32 milisec) {usleep(milisec);}
	void vapiDisplaySetScreen() {
		if (!vmachine.flagmode)
			linuxconDisplaySetScreen();
		else
			linuxappDisplaySetScreen();
		}
	}
	void vapiDisplayPaint() {
		if (!vmachine.flagmode)
			linuxconDisplayPaint();
		else
			linuxappDisplayPaint();
		}
	}
	void vapiStartMachine() {
		if (!vmachine.flagmode)
			linuxconStartMachine();
		else
			linuxappStartMachine();
		}
	}
#endif

void vapiInit() {memset(&vapirecord, 0x00, sizeof(t_apirecord));}
void vapiFinal() {}
