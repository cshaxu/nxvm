
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
	t_nubit16 _ax = vcpu.ax;
	t_nubit16 _bx = vcpu.bx;
	t_nubit16 _cx = vcpu.cx;
	t_nubit16 _dx = vcpu.dx;
	t_nubit16 _sp = vcpu.sp;
	t_nubit16 _bp = vcpu.bp;
	t_nubit16 _si = vcpu.si;
	t_nubit16 _di = vcpu.di;
	t_nubit16 _ip = vcpu.ip;
	t_nubit16 _ds = vcpu.ds;
	t_nubit16 _cs = vcpu.cs;
	t_nubit16 _es = vcpu.es;
	t_nubit16 _ss = vcpu.ss;
	t_nubit16 _flags = vcpu.flags;
	t_bool    _of = GetBit(vcpu.flags, VCPU_FLAG_OF);
	t_bool    _sf = GetBit(vcpu.flags, VCPU_FLAG_SF);
	t_bool    _zf = GetBit(vcpu.flags, VCPU_FLAG_ZF);
	t_bool    _cf = GetBit(vcpu.flags, VCPU_FLAG_CF);
	t_bool    _af = GetBit(vcpu.flags, VCPU_FLAG_AF);
	t_bool    _pf = GetBit(vcpu.flags, VCPU_FLAG_PF);
	t_bool    _df = GetBit(vcpu.flags, VCPU_FLAG_DF);
	t_bool    _tf = GetBit(vcpu.flags, VCPU_FLAG_TF);
	t_bool    _if = GetBit(vcpu.flags, VCPU_FLAG_IF);

	if ((vlog.line >= VLOG_COUNT_MAX) || (!vlog.fp)) {
		if (vlog.line == VLOG_COUNT_MAX) vapiPrint("Log ends.\n");
		return;
	}
	vramDWord(0x0040,0x006c) = 0x00000000;
	fprintf(vlog.fp, "%d\tcs:ip=%x:%x opcode=%x %x %x %x %x %x %x %x ",
		vlog.line, _cs, _ip,
		vramByte(_cs,_ip+0),
		vramByte(_cs,_ip+1),
		vramByte(_cs,_ip+2),
		vramByte(_cs,_ip+3),
		vramByte(_cs,_ip+4),
		vramByte(_cs,_ip+5),
		vramByte(_cs,_ip+6),
		vramByte(_cs,_ip+7));
	fprintf(vlog.fp, 
"ax=%x bx=%x cx=%x dx=%x \
sp=%x bp=%x si=%x di=%x \
ds=%x es=%x ss=%x \
of=%1x sf=%1x zf=%1x cf=%1x af=%1x \
pf=%1x df=%1x if=%1x tf=%1x ram=%x\n",
		_ax,_bx,_cx,_dx,
		_sp,_bp,_si,_di,
		_ds,_es,_ss,
		_of,_sf,_zf,_cf,_af,
		_pf,_df,_if,_tf,vramWord(_ss,_sp));
}