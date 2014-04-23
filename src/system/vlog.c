
#include "vapi.h"
#include "vlog.h"

#include "../vmachine/vcpu.h"
#include "../vmachine/vram.h"

t_log vlog;

void vlogInit()
{

	vlog.line = 0;
	vlog.fp = fopen("d:/nxvm.log","w");
}
void vlogFinal()
{
	if (vlog.fp) fclose(vlog.fp);
	vlog.fp = NULL;
}
void vlogExec()
{
#define _ax    (vcpu.ax)
#define _bx    (vcpu.bx)
#define _cx    (vcpu.cx)
#define _dx    (vcpu.dx)
#define _sp    (vcpu.sp)
#define _bp    (vcpu.bp)
#define _si    (vcpu.si)
#define _di    (vcpu.di)
#define _ip    (vcpu.ip)
#define _ds    (vcpu.ds)
#define _cs    (vcpu.cs)
#define _es    (vcpu.es)
#define _ss    (vcpu.ss)
#define _fg    (vcpu.flags)
#define _of    (GetBit(vcpu.flags, VCPU_FLAG_OF))
#define _sf    (GetBit(vcpu.flags, VCPU_FLAG_SF))
#define _zf    (GetBit(vcpu.flags, VCPU_FLAG_ZF))
#define _cf    (GetBit(vcpu.flags, VCPU_FLAG_CF))
#define _af    (GetBit(vcpu.flags, VCPU_FLAG_AF))
#define _pf    (GetBit(vcpu.flags, VCPU_FLAG_PF))
#define _df    (GetBit(vcpu.flags, VCPU_FLAG_DF))
#define _tf    (GetBit(vcpu.flags, VCPU_FLAG_TF))
#define _if    (GetBit(vcpu.flags, VCPU_FLAG_IF))
#define _expression "%d\tcs:ip=%x:%x opcode=%x %x %x %x %x %x %x %x \
ax=%x bx=%x cx=%x dx=%x sp=%x bp=%x si=%x di=%x ds=%x es=%x ss=%x \
of=%1x sf=%1x zf=%1x cf=%1x af=%1x pf=%1x df=%1x if=%1x tf=%1x ram=%x\n"

	if ((vlog.line >= VLOG_COUNT_MAX) || (!vlog.fp)) {
		if (vlog.line == VLOG_COUNT_MAX) vapiPrint("Log ends.\n");
		return;
	}
	vramDWord(0x0040,0x006c) = 0x00000000;
	fprintf(vlog.fp, _expression,
		vlog.line, _cs, _ip,
		vramByte(_cs,_ip+0),vramByte(_cs,_ip+1),
		vramByte(_cs,_ip+2),vramByte(_cs,_ip+3),
		vramByte(_cs,_ip+4),vramByte(_cs,_ip+5),
		vramByte(_cs,_ip+6),vramByte(_cs,_ip+7),
		_ax,_bx,_cx,_dx,_sp,_bp,_si,_di,_ds,_es,_ss,
		_of,_sf,_zf,_cf,_af,_pf,_df,_if,_tf,vramWord(_ss,_sp));
}
