/* This file is a part of NXVM project. */

/*
test code
	mov	di,bp
	add	di,4
	mov	si,di
	push	bp
	mov	bp,si
	mov	ax,word ptr [bp]
	pop	bp
	mov	bx,ax
	push	bx
	mov	ax,0
	pop	bx
	cmp	bx,ax
*/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdarg.h"

#include "../vapi.h"

#include "aasm.h"
#include "dasm.h"
#include "debug.h"

#define MAXLINE 256
#define MAXNARG 256
#define MAXNASMARG 4

static t_nubitcc errPos;
static t_nubitcc narg;
static char **arg;
static t_bool exitFlag;
static char cmdBuff[MAXLINE];
static char cmdCopy[MAXLINE];
static char filename[MAXLINE];

static t_nubit16 dumpSegRec;
static t_nubit16 dumpPtrRec;
static t_nubit16 asmSegRec;
static t_nubit16 asmPtrRec;
static t_nubit16 uasmSegRec;
static t_nubit16 uasmPtrRec;

static t_nubit16 seg;
static t_nubit16 ptr;

static void lcase(t_string s)
{
	t_nubitcc i = 0;
	if(s[0] == '\'') return;
	while(s[i] != '\0') {
		if(s[i] == '\n') s[i] = '\0';
		else if(s[i] > 0x40 && s[i] < 0x5b)
			s[i] += 0x20;
		i++;
	}
}
static void seterr(t_nubitcc pos)
{
	errPos = (t_nubitcc)(arg[pos] - cmdCopy + strlen(arg[pos]) + 1);
}
static t_nubit8 scannubit8(t_string s)
{
	t_nubit8 ans = 0;
	int i = 0;
	if(s[0] == '\'' && s[2] == '\'') return s[1];
	lcase(s);
	while(s[i] != '\0' && s[i] != '\n') {
		if(i > 1) {seterr(narg-1);break;}
		ans <<= 4;
		if(s[i] > 0x2f && s[i] < 0x3a) ans += s[i] - 0x30;
		else if(s[i] > 0x60 && s[i] < 0x67) ans += s[i] - 0x57;
		else seterr(narg-1);
		++i;
	}
	return ans;
}
static t_nubit16 scannubit16(t_string s)
{
	t_nubit16 ans = 0;
	int i = 0;
	lcase(s);
	while(s[i] != '\0' && s[i] != '\n') {
		if(i > 3) {seterr(narg-1);break;}
		ans <<= 4;
		if(s[i] > 0x2f && s[i] < 0x3a) ans += s[i] - 0x30;
		else if(s[i] > 0x60 && s[i] < 0x67) ans += s[i] - 0x57;
		else seterr(narg-1);
		++i;
	}
	return ans;
}
static t_nubit32 scannubit32(t_string s)
{
	t_nubit32 ans = 0;
	int i = 0;
	lcase(s);
	while(s[i] != '\0' && s[i] != '\n') {
		if(i > 7) {seterr(narg-1);break;}
		ans <<= 4;
		if(s[i] > 0x2f && s[i] < 0x3a) ans += s[i] - 0x30;
		else if(s[i] > 0x60 && s[i] < 0x67) ans += s[i] - 0x57;
		else seterr(narg-1);
		++i;
	}
	return ans;
}
static void addrparse(t_nubit16 defseg,const t_string addr)
{
	t_string cseg,cptr;
	char ccopy[MAXLINE];
	STRCPY(ccopy,addr);
	cseg = STRTOK(ccopy,":");
	cptr = STRTOK(NULL,"");
	if(!cptr) {
		seg = defseg;
		ptr = scannubit16(cseg);
	} else {
		if(!STRCMP(cseg,"cs")) seg = _cs;
		else if(!STRCMP(cseg,"ss")) seg = _ss;
		else if(!STRCMP(cseg,"ds")) seg = _ds;
		else if(!STRCMP(cseg,"es")) seg = _es;
		else seg = scannubit16(cseg);
		ptr = scannubit16(cptr);
	}
}
static void addrprint(t_nubit16 segment,t_nubit16 pointer)
{	
	vapiPrint("%04X",segment);
	vapiPrint(":");
	vapiPrint("%04X",pointer);
	vapiPrint("  ");
}

/* DEBUG CMD BEGIN */
// assemble
static void aconsole()
{
	t_nubitcc i;
	t_nubit16 len;
	char cmdAsmBuff[MAXLINE];
	t_nubitcc errAsmPos;
	t_bool exitAsmFlag = 0;
	while(!exitAsmFlag) {
		addrprint(asmSegRec,asmPtrRec);
		fflush(stdin);
		vapiPrint("\b");
		/*fflush(stdout);*/
		FGETS(cmdAsmBuff,MAXLINE,stdin);
		lcase(cmdAsmBuff);
		if(!strlen(cmdAsmBuff)) {
			exitAsmFlag = 1;
			continue;
		}
		if(cmdAsmBuff[0] == ';' ) continue;
		errAsmPos = 0;
		len = (t_nubit16)aasm(cmdAsmBuff, asmSegRec, asmPtrRec);
		if(!len) errAsmPos = (t_nubitcc)strlen(cmdAsmBuff) + 9;
		else asmPtrRec += len;
		if(errAsmPos) {
			for(i = 0;i < errAsmPos;++i) vapiPrint(" ");
			vapiPrint("^ Error\n");
		}
	}
}
static void a()
{
	if(narg == 1) {
		aconsole();
	} else if(narg == 2) {
		addrparse(_cs, arg[1]);
		if(errPos) return;
		asmSegRec = seg;
		asmPtrRec = ptr;
		aconsole();
	} else seterr(2);
}
// compare
static void c()
{
	t_nubit8 val1,val2;
	t_nubit16 i,seg1,ptr1,range,seg2,ptr2;
	if(narg != 4) seterr(narg-1);
	else {
		addrparse(_ds,arg[1]);
		seg1 = seg;
		ptr1 = ptr;
		addrparse(_ds,arg[3]);
		seg2 = seg;
		ptr2 = ptr;
		range = scannubit16(arg[2])-ptr1;
		if(!errPos) {
			for(i = 0;i <= range;++i) {
				val1 = vramRealByte(seg1,ptr1+i);
				val2 = vramRealByte(seg2,ptr2+i);
				if(val1 != val2) {
					addrprint(seg1,ptr1+i);
					vapiPrint("  ");
					vapiPrint("%02X",val1 & 0xff);
					vapiPrint("  ");
					vapiPrint("%02X",val2 & 0xff);
					vapiPrint("  ");
					addrprint(seg2,ptr2+i);
					vapiPrint("\n");
				}
			}
		}
	}
}
// dump
static void dprint(t_nubit16 segment,t_nubit16 start,t_nubit16 end)
{
	char t,c[0x11];
	t_nubit16 i;
	if(start > end) end = 0xffff;
	if((t_nubit32)((segment<<4)+end) > 0xfffff) end = (0xfffff - (segment << 4));
	c[0x10] = '\0';
	if(end < start) end = 0xffff;
	for(i = start-(start%0x10);i <= end+(0x10-end%0x10)-1;++i) {
		if(i%0x10 == 0) addrprint(segment,i);
		if(i < start || i > end) {
			vapiPrint("  ");
			c[i%0x10] = ' ';
		} else {
			c[i%0x10] = vramRealByte(segment,i);
			vapiPrint("%02X",c[i%0x10] & 0xff);
			t = c[i%0x10];
			if((t >=1 && t <= 7) || t == ' ' ||
				(t >=11 && t <= 12) ||
				(t >=14 && t <= 31) ||
				(t >=33 && t <= 128)) ;
			else c[i%0x10] = '.';
		}
		vapiPrint(" ");
		if(i%0x10 == 7 && i >= start && i < end) vapiPrint("\b-");
		if((i+1)%0x10 == 0) {
			vapiPrint("  %s\n",c);
		}
		if(i == 0xffff) break;
	}
	dumpSegRec = segment;
	dumpPtrRec = end+1;
}
static void d()
{
	t_nubit16 ptr2;
	if(narg == 1) dprint(dumpSegRec,dumpPtrRec,dumpPtrRec+0x7f);
	else if(narg == 2) {
		addrparse(_ds,arg[1]);
		if(errPos) return;
		dprint(seg,ptr,ptr+0x7f);
	} else if(narg == 3) {
		addrparse(_ds,arg[1]);
		ptr2 = scannubit16(arg[2]);
		if(errPos) return;
		if(ptr > ptr2) seterr(2);
		else dprint(seg,ptr,ptr2);
	} else seterr(3);
}
// enter
static void e()
{
	t_nubitcc i;
	t_nubit8 val;
	char s[MAXLINE];
	if(narg == 1) seterr(0);
	else if(narg == 2) {
		addrparse(_ds,arg[1]);
		if(errPos) return;
		addrprint(seg,ptr);
		vapiPrint("%02X",vramRealByte(seg,ptr));
		vapiPrint(".");
		FGETS(s,MAXLINE,stdin);
		lcase(s);//!!
		val = scannubit8(s);//!!
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			vramRealByte(seg,ptr) = val;
	} else if(narg > 2) {
		addrparse(_ds,arg[1]);
		if(errPos) return;
		for(i = 2;i < narg;++i) {
			val = scannubit8(arg[i]);//!!
			if(!errPos) vramRealByte(seg,ptr) = val;
			else break;
			ptr++;
		}
	}
}
// fill
static void f()
{
	t_nubitcc nbyte;
	t_nubit8 val;
	t_nubit16 i,j,end;
	if(narg < 4) seterr(narg-1);
	else {
		addrparse(_ds,arg[1]);
		end = scannubit16(arg[2]);
		if(end < ptr) seterr(2);
		if(!errPos) {
			nbyte = narg - 3;
			for(i = ptr,j = 0;i <= end;++i,++j) {
				val = scannubit8(arg[j%nbyte+3]);
				if(!errPos) vramRealByte(seg,i) = val;
				else return;
			}
		}
	}
}
// go
static void rprintregs();
static void g()
{
	if (vmachine.flagrun) {
		vapiPrint("NXVM is already running.\n");
		return;
	}
	switch(narg) {
	case 1:
		vmachine.flagbreak = 0x00;
		break;
	case 2:
		vmachine.flagbreak = 0x01;
		addrparse(_cs,arg[1]);
		vmachine.breakcs = seg;
		vmachine.breakip = ptr;
		break;
	case 3:
		vmachine.flagbreak = 0x01;
		addrparse(_cs,arg[1]);
		_cs = seg;
		_ip = ptr;
		addrparse(_cs,arg[2]);
		vmachine.breakcs = seg;
		vmachine.breakip = ptr;
		break;
	default:seterr(narg-1);break;}
	if(errPos) return;
	vmachineResume();
	while (vmachine.flagrun) vapiSleep(1);
	vmachine.flagbreak = 0x00;
	rprintregs();
}
// hex
static void h()
{
	t_nubit16 val1,val2;
	if(narg != 3) seterr(narg-1);
	else {
		val1 = scannubit16(arg[1]);
		val2 = scannubit16(arg[2]);
		if(!errPos) {
			vapiPrint("%04X",val1+val2);
			vapiPrint("  ");
			vapiPrint("%04X",val1-val2);
			vapiPrint("\n");
		}
	}
}
// input
static void i()
{
	t_nubit16 in;
	if(narg != 2) seterr(narg-1);
	else {
		in = scannubit16(arg[1]);
		if(!errPos) {
			ExecFun(vport.in[in]);
			vapiPrint("%02X",vport.iobyte);
			vapiPrint("\n");
		}
	}
}
// load
static void l()
{
	t_nubit8 c;
	t_nubit16 i = 0;
	t_nubit32 len = 0;
	FILE *load = FOPEN(filename,"rb");
	if(!load) vapiPrint("File not found\n");
	else {
		switch(narg) {
		case 1:
			seg = _cs;
			ptr = 0x100;
			break;
		case 2:
			addrparse(_cs,arg[1]);
			break;
		default:seterr(narg-1);break;}
		if(!errPos) {
			c = fgetc(load);
			while(!feof(load)) {
				vramRealByte(seg+i,ptr+len++) = c;
				i = len / 0x10000;
				c = fgetc(load);
			}
			_cx = len&0xffff;
			if(len > 0xffff) _bx = (len>>16);
			else _bx = 0x0000;
		}
		fclose(load);
	}
}
// move
static void m()
{
	int i;
	t_nubit16 seg1,ptr1,range,seg2,ptr2;
	if(narg != 4) seterr(narg-1);
	else {
		addrparse(_ds,arg[1]);
		seg1 = seg;
		ptr1 = ptr;
		addrparse(_ds,arg[3]);
		seg2 = seg;
		ptr2 = ptr;
		range = scannubit16(arg[2])-ptr1;
		if(!errPos) {
			if(((seg1<<4)+ptr1) < ((seg2<<4)+ptr2)) {
				for(i = range;i >= 0;--i)
					vramRealByte(seg2,ptr2+i) = vramRealByte(seg1,ptr1+i);
			} else if(((seg1<<4)+ptr1) > ((seg2<<4)+ptr2)) {
				for(i = 0;i <= range;++i)
					vramRealByte(seg2,ptr2+i) = vramRealByte(seg1,ptr1+i);
			}
		}
	}
}
// name
static void n()
{
	if(narg != 2) seterr(narg-1);
	else STRCPY(filename,arg[1]);
}
// output
static void o()
{
	t_nubit16 out;
	if(narg != 3) seterr(narg-1);
	else {
		out = scannubit16(arg[1]);
		vport.iobyte = scannubit8(arg[2]);
		if(!errPos) ExecFun(vport.out[out]);
	}
}
// quit
static void q()
{exitFlag = 1;}
// register
static void uprint(t_nubit16,t_nubit16,t_nubit16);
static void rprintflags()
{
	/*if (bit == 32) {
		vapiPrint("ID%c ",   _GetID ? '+' : '-');
		vapiPrint("VIP%c ",  _GetVIP ? '+' : '-');
		vapiPrint("VIF%c ",  _GetVIF ? '+' : '-');
		vapiPrint("AC%c ",   _GetAC ? '+' : '-');
		vapiPrint("VM%c ",   _GetEFLAGS_VM ? '+' : '-');
		vapiPrint("RF%c ",   _GetEFLAGS_RF ? '+' : '-');
		vapiPrint("NT%c ",   _GetEFLAGS_NT ? '+' : '-');
		vapiPrint("IOPL=%1d ", _GetEFLAGS_IOPL);
	}*/
	vapiPrint("%s ", _GetEFLAGS_OF ? "OV" : "NV");
	vapiPrint("%s ", _GetEFLAGS_DF ? "DN" : "UP");
	vapiPrint("%s ", _GetEFLAGS_IF ? "EI" : "DI");
	//vapiPrint("%s ", _GetEFLAGS_TF ? "TR" : "DT");
	vapiPrint("%s ", _GetEFLAGS_SF ? "NG" : "PL");
	vapiPrint("%s ", _GetEFLAGS_ZF ? "ZR" : "NZ");
	vapiPrint("%s ", _GetEFLAGS_AF ? "AC" : "NA");
	vapiPrint("%s ", _GetEFLAGS_PF ? "PE" : "PO");
	vapiPrint("%s ", _GetEFLAGS_CF ? "CY" : "NC");
}
static void rprintregs()
{
	char str[MAXLINE];
/*
	32-bit
	vapiPrint( "EAX=%08X", _eax);
	vapiPrint(" EBX=%08X", _ebx);
	vapiPrint(" ECX=%08X", _ecx);
	vapiPrint(" EDX=%08X", _edx);
	vapiPrint("  SS=%04X", _ss);
	vapiPrint( " CS=%04X", _cs);
	vapiPrint( " DS=%04X", _ds);
	vapiPrint("\nESP=%08X",_esp);
	vapiPrint(" EBP=%08X", _ebp);
	vapiPrint(" ESI=%08X", _esi);
	vapiPrint(" EDI=%08X", _edi);
	vapiPrint("  ES=%04X", _es);
	vapiPrint( " FS=%04X", _fs);
	vapiPrint( " GS=%04X", _gs);
	vapiPrint("\nEIP=%08X", _ip);
	vapiPrint("  \n");
//*/
	vapiPrint(  "AX=%04X", _ax);
	vapiPrint("  BX=%04X", _bx);
	vapiPrint("  CX=%04X", _cx);
	vapiPrint("  DX=%04X", _dx);
	vapiPrint("  SP=%04X", _sp);
	vapiPrint("  BP=%04X", _bp);
	vapiPrint("  SI=%04X", _si);
	vapiPrint("  DI=%04X", _di);
	vapiPrint("\nDS=%04X", _ds);
	vapiPrint("  ES=%04X", _es);
	vapiPrint("  SS=%04X", _ss);
	vapiPrint("  CS=%04X", _cs);
	vapiPrint("  IP=%04X", _ip);
	vapiPrint("   ");
	rprintflags();
	vapiPrint("\n");
	dasm(str, _cs, _ip, 0x02);
	uasmSegRec = _cs;
	uasmPtrRec = _ip;
	vapiPrint("%s", str);
}
static void rscanregs()
{
	t_nubit16 t;
	char s[MAXLINE];
	if(!STRCMP(arg[1],"ax")) {
		vapiPrint("AX ");
		vapiPrint("%04X",_ax);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_ax = t;
	} else if(!STRCMP(arg[1],"bx")) {
		vapiPrint("BX ");
		vapiPrint("%04X",_bx);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_bx = t;
	} else if(!STRCMP(arg[1],"cx")) {
		vapiPrint("CX ");
		vapiPrint("%04X",_cx);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_cx = t;
	} else if(!STRCMP(arg[1],"dx")) {
		vapiPrint("DX ");
		vapiPrint("%04X",_dx);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_dx = t;
	} else if(!STRCMP(arg[1],"bp")) {
		vapiPrint("BP ");
		vapiPrint("%04X",_bp);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_bp = t;
	} else if(!STRCMP(arg[1],"sp")) {
		vapiPrint("SP ");
		vapiPrint("%04X",_sp);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_sp = t;
	} else if(!STRCMP(arg[1],"si")) {
		vapiPrint("SI ");
		vapiPrint("%04X",_si);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_si = t;
	} else if(!STRCMP(arg[1],"di")) {
		vapiPrint("DI ");
		vapiPrint("%04X",_di);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_di = t;
	} else if(!STRCMP(arg[1],"ss")) {
		vapiPrint("SS ");
		vapiPrint("%04X",_ss);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos) {
			_ss = t;
		}
	} else if(!STRCMP(arg[1],"cs")) {
		vapiPrint("CS ");
		vapiPrint("%04X",_cs);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_cs = t;
	} else if(!STRCMP(arg[1],"ds")) {
		vapiPrint("DS ");
		vapiPrint("%04X",_ds);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos) {
			_ds = t;
		}
	} else if(!STRCMP(arg[1],"es")) {
		vapiPrint("ES ");
		vapiPrint("%04X",_es);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_es = t;
	} else if(!STRCMP(arg[1],"ip")) {
		vapiPrint("IP ");
		vapiPrint("%04X",_ip);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_ip = t;
	} else if(!STRCMP(arg[1],"f")) {
		rprintflags();
		vapiPrint(" -");
		FGETS(s,MAXLINE,stdin);
		lcase(s);
		if(!STRCMP(s,"ov"))      _SetEFLAGS_OF;
		else if(!STRCMP(s,"nv")) _ClrEFLAGS_OF;
		else if(!STRCMP(s,"dn")) _SetEFLAGS_DF;
		else if(!STRCMP(s,"up")) _ClrEFLAGS_DF;
		else if(!STRCMP(s,"ei")) _SetEFLAGS_IF;
		else if(!STRCMP(s,"di")) _ClrEFLAGS_IF;
		else if(!STRCMP(s,"ng")) _SetEFLAGS_SF;
		else if(!STRCMP(s,"pl")) _ClrEFLAGS_SF;
		else if(!STRCMP(s,"zr")) _SetEFLAGS_ZF;
		else if(!STRCMP(s,"nz")) _ClrEFLAGS_ZF;
		else if(!STRCMP(s,"ac")) _SetEFLAGS_AF;
		else if(!STRCMP(s,"na")) _ClrEFLAGS_AF;
		else if(!STRCMP(s,"pe")) _SetEFLAGS_PF;
		else if(!STRCMP(s,"po")) _ClrEFLAGS_PF;
		else if(!STRCMP(s,"cy")) _SetEFLAGS_CF;
		else if(!STRCMP(s,"nc")) _ClrEFLAGS_CF;
		else vapiPrint("bf Error\n");
	} else vapiPrint("br Error\n");
}
static void r()
{
	if(narg == 1) {
		rprintregs();
	} else if(narg == 2) {
		rscanregs();
	} else seterr(2);
}
// search
static void s()
{
	t_nubitcc i;
	t_bool flag = 0x00;
	t_nubit16 p,pfront,start,end;
	if(narg < 4) seterr(narg-1);
	else {
		addrparse(_ds,arg[1]);
		start = ptr;
		end = scannubit16(arg[2]);
		if(!errPos) {
			p = start;
			while(p <= end) {
				if(vramRealByte(seg,p) == scannubit8(arg[3])) {
					pfront = p;
					flag = 0x01;
					for(i = 3;i < narg;++i) {
						if(vramRealByte(seg,p) != scannubit8(arg[i])) {
							flag = 0x00;
							p = pfront+1;
							break;
						} else ++p;
					}
					if(flag) {
						addrprint(seg,pfront);
						vapiPrint("\n");
					}
				} else ++p;
			}
		}
	}
}
// trace
static void t()
{
	t_nubit16 i;
	t_nubit16 count;
	if (vmachine.flagrun) {
		vapiPrint("NXVM is already running.\n");
		return;
	}
	switch(narg) {
	case 1:
		count = 1;
		break;
	case 2:
		count = scannubit16(arg[1]);
		break;
	case 3:
		addrparse(_cs,arg[1]);
		_cs = seg;
		_ip = ptr;
		count = scannubit16(arg[2]);
		break;
	default:seterr(narg-1);break;}
	if(errPos) return;
	if (count < 0x0100) {
		for(i = 0;i < count;++i) {
			vmachine.tracecnt = 0x01;
			vmachineResume();
			while (vmachine.flagrun) vapiSleep(10);
			rprintregs();
			if (i != count - 1) vapiPrint("\n");
		}
	} else {
		vmachine.tracecnt = count;
		vmachineResume();
		while (vmachine.flagrun) vapiSleep(10);
		rprintregs();
	}
	vmachine.tracecnt = 0x00;
//	gexec(ptr1,ptr2);
}
// unassemble
static void uprint(t_nubit16 segment,t_nubit16 start,t_nubit16 end)
{
	char str[MAXLINE];
	t_nubit16 len = 0;
	t_nubit32 boundary;
	if(start > end) end = 0xffff;
	if ((t_nubit32)((segment<<4) + end) > 0xfffff) end = (0xfffff-(segment<<4));
	while(start <= end) {
		len = (t_nubit16)dasm(str, segment, start, 0x01);
		vapiPrint("%s", str);
		start += len;
		boundary = (t_nubit32)start + (t_nubit32)len;
		if (boundary > 0xffff) break;
	}
	uasmSegRec = segment;
	uasmPtrRec = start;
	return;
}
static void u()
{
	t_nubit16 ptr2;
	if(narg == 1) uprint(uasmSegRec,uasmPtrRec,uasmPtrRec+0x1f);
	else if(narg == 2) {
		addrparse(_cs,arg[1]);
		if(errPos) return;
		uprint(seg,ptr,ptr+0x1f);
	} else if(narg == 3) {
		addrparse(_ds,arg[1]);
		ptr2 = scannubit16(arg[2]);
		if(errPos) return;
		if(ptr > ptr2) seterr(2);
		else uprint(seg,ptr,ptr2);
	} else seterr(3);
}
// verbal
static void v()
{
	t_nubitcc i;
	char str[MAXLINE];
	vapiPrint(":");
	FGETS(str,MAXLINE,stdin);
	str[strlen(str)-1] = '\0';
	for(i = 0;i < strlen(str);++i) {
		vapiPrint("%02X",str[i]);
		if(!((i+1)%0x10)) vapiPrint("\n"); 
		else if(!((i+1)%0x08)&&(str[i+1]!='\0')) vapiPrint("-");
		else vapiPrint(" ");
	}
	if(i%0x10) vapiPrint("\n");
}
// write
static void w()
{
	t_nubit16 i = 0;
	t_nubit32 len = (_bx<<16)+_cx;
	FILE *write;
	if(!strlen(filename)) {vapiPrint("(W)rite error, no destination defined\n");return;}
	else write= FOPEN(filename,"wb");
	if(!write) vapiPrint("File not found\n");
	else {
		vapiPrint("Writing ");
		vapiPrint("%04X",_bx);
		vapiPrint("%04X",_cx);
		vapiPrint(" bytes\n");
		switch(narg) {
		case 1:
			seg = _cs;
			ptr = 0x100;
			break;
		case 2:
			addrparse(_cs,arg[1]);
			break;
		default:	seterr(narg-1);break;}
		if(!errPos)
			while(i < len)
				fputc(vramRealByte(seg,ptr+i++),write);
		fclose(write);
	}
}
/* DEBUG CMD END */

/* EXTENDED DEBUG CMD BEGIN */

t_nubit32 _dbgm_addr_physical_linear(t_nubit32 linear)
{
	/* paging not implemented */
	return linear;
}
t_nubit32 _dbgm_addr_linear_logical(t_cpu_sreg *rsreg, t_nubit32 offset)
{
	return (rsreg->base + offset);
}
t_nubit32 _dbgm_addr_physical_logical(t_cpu_sreg *rsreg, t_nubit32 offset)
{
	/* paging not implemented */
	return _dbgm_addr_physical_linear(_dbgm_addr_linear_logical(rsreg, offset));
}
t_nubit64 _dbgm_read_physical(t_nubit32 physical, t_nubit8 byte)
{
	switch (byte) {
	case 1: return vramByte(physical);
	case 2: return vramWord(physical);
	case 4: return vramDWord(physical);
	case 8: return vramQWord(physical);
	default:return 0;
	}
}
t_nubit64 _dbgm_read_linear(t_nubit32 linear, t_nubit8 byte)
{
	return _dbgm_read_physical(_dbgm_addr_physical_linear(linear), byte);
}
t_nubit64 _dbgm_read_logical(t_cpu_sreg *rsreg, t_nubit32 offset, t_nubit8 byte)
{
	return _dbgm_read_physical(_dbgm_addr_physical_logical(rsreg, offset), byte);
}

t_nubit32 xdlin;
t_nubit32 xulin;

static void xdprint(t_nubit32 linear,t_nubit8 count)
{
	char t,c[0x11];
	t_nubit32 i;
	t_nubit32 start = linear;
	t_nubit32 end = linear + count - 1;
	c[0x10] = '\0';
	for(i = start - (start % 0x10);i < end + 0x10 - (end % 0x10);++i) {
		if (i % 0x10 == 0) vapiPrint("L%08X  ", i);
		if (i < start || i > end) {
			vapiPrint("  ");
			c[i % 0x10] = ' ';
		} else {
			c[i % 0x10] = vramByte(i);
			vapiPrint("%02X",c[i % 0x10] & 0xff);
			t = c[i % 0x10];
			if((t >=1 && t <= 7) || t == ' ' ||
				(t >=11 && t <= 12) ||
				(t >=14 && t <= 31) ||
				(t >=33 && t <= 128)) ;
			else c[i%0x10] = '.';
		}
		vapiPrint(" ");
		if(i % 0x10 == 7 && i >= start && i < end) vapiPrint("\b-");
		if((i + 1) % 0x10 == 0) {
			vapiPrint("  %s\n",c);
		}
	}
	xdlin = i;
}
static void xd()
{
	t_nubit8 count;
	if(narg == 1) xdprint(xdlin, 0x80);
	else if(narg == 2) {
		xdlin = scannubit32(arg[1]);
		if(errPos) return;
		xdprint(xdlin, 0x80);
	} else if(narg == 3) {
		xdlin = scannubit32(arg[1]);
		count = scannubit8(arg[2]);
		if(errPos) return;
		xdprint(xdlin, count);
	} else seterr(3);
}
static void xreg()
{
	char str[MAXLINE];
	vapiPrint( "EAX=%08X", _eax);
	vapiPrint(" EBX=%08X", _ebx);
	vapiPrint(" ECX=%08X", _ecx);
	vapiPrint(" EDX=%08X", _edx);
	vapiPrint("\nESP=%08X",_esp);
	vapiPrint(" EBP=%08X", _ebp);
	vapiPrint(" ESI=%08X", _esi);
	vapiPrint(" EDI=%08X", _edi);
	vapiPrint("\nEIP=%08X", _ip);
	vapiPrint(" EFL=%08X", _eflags);
	vapiPrint(": ");
	vapiPrint("%s ", _GetEFLAGS_VM ? "VM" : "vm");
	vapiPrint("%s ", _GetEFLAGS_RF ? "RF" : "rf");
	vapiPrint("%s ", _GetEFLAGS_NT ? "NT" : "nt");
	vapiPrint("IOPL=%01X ", _GetEFLAGS_IOPL);
	vapiPrint("%s ", _GetEFLAGS_OF ? "OF" : "of");
	vapiPrint("%s ", _GetEFLAGS_DF ? "DF" : "df");
	vapiPrint("%s ", _GetEFLAGS_IF ? "IF" : "if");
	vapiPrint("%s ", _GetEFLAGS_TF ? "TF" : "tf");
	vapiPrint("%s ", _GetEFLAGS_SF ? "SF" : "sf");
	vapiPrint("%s ", _GetEFLAGS_ZF ? "ZF" : "zf");
	vapiPrint("%s ", _GetEFLAGS_AF ? "AF" : "af");
	vapiPrint("%s ", _GetEFLAGS_PF ? "PF" : "pf");
	vapiPrint("%s ", _GetEFLAGS_CF ? "CF" : "cf");
	vapiPrint("\n");
	dasmx(str, vcpu.cs.base + vcpu.eip, 0x02);
	vapiPrint("%s", str);
}
static void xsregseg(t_cpu_sreg *rsreg, const t_string label)
{
	vapiPrint("%s=%04X, Base=%08X, Limit=%08X, DPL=%01X, %s, ", label,
		rsreg->selector, rsreg->base, rsreg->limit,
		rsreg->dpl, rsreg->seg.accessed ? "A" : "a");
	if (rsreg->seg.executable) {
		vapiPrint("Code, %s, %s, %s\n",
			rsreg->seg.exec.conform ? "C" : "c",
			rsreg->seg.exec.readable ? "Rw" : "rw",
			rsreg->seg.exec.defsize ? "32" : "16");
	} else {
		vapiPrint("Data, %s, %s, %s\n",
			rsreg->seg.data.expdown ? "E" : "e",
			rsreg->seg.data.writable ? "RW" : "Rw",
			rsreg->seg.data.big ? "BIG" : "big");
	} 
}
static void xsregsys(t_cpu_sreg *rsreg, const t_string label)
{
	vapiPrint("%s=%04X, Base=%08X, Limit=%08X, DPL=%01X, Type=%04X\n", label,
		rsreg->selector, rsreg->base, rsreg->limit,
		rsreg->dpl, rsreg->sys.type);
}
static void xsreg()
{
	xsregseg(&vcpu.es, "ES");
	xsregseg(&vcpu.cs, "CS");
	xsregseg(&vcpu.ss, "SS");
	xsregseg(&vcpu.ds, "DS");
	xsregseg(&vcpu.fs, "FS");
	xsregseg(&vcpu.gs, "GS");
	xsregsys(&vcpu.tr, "TR  ");
	xsregsys(&vcpu.ldtr, "LDTR");
	vapiPrint("GDTR Base=%08X, Limit=%04X\n",
		_gdtr.base, _gdtr.limit);
	vapiPrint("IDTR Base=%08X, Limit=%04X\n",
		_idtr.base, _idtr.limit);
}
static void xcreg()
{
	vapiPrint("CR0=%08X: %s %s %s %s %s %s\n", vcpu.cr0,
		_GetCR0_PG ? "PG" : "pg",
		_GetCR0_ET ? "ET" : "et",
		_GetCR0_TS ? "TS" : "ts",
		_GetCR0_EM ? "EM" : "em",
		_GetCR0_MP ? "MP" : "mp",
		_GetCR0_PE ? "PE" : "pe");
	vapiPrint("CR2=PFLR=%08X\n", vcpu.cr2);
	vapiPrint("CR3=PDBR=%08X\n", vcpu.cr3);
}
static void xuprint(t_nubit32 linear, t_nubit8 count)
{
	char str[MAXLINE];
	t_nubit32 len = 0;
	t_nubit8 i;
	for (i = 0;i < count;++i) {
		len = GetMax32(dasmx(str, linear, 0x01));
		vapiPrint("%s", str);
		linear += len;
	}
	xulin = linear;
}
static void xu()
{
	t_nubit8 count;
	if(narg == 1) xuprint(xulin, 10);
	else if(narg == 2) {
		xulin = scannubit32(arg[1]);
		if(errPos) return;
		xuprint(xulin, 10);
	} else if(narg == 3) {
		xulin = scannubit32(arg[1]);
		count = scannubit8(arg[2]);
		if(errPos) return;
		xuprint(xulin, count);
	} else seterr(3);
}
static void xtprintmem()
{
	t_nubit32 i;
	for (i = 0;i < vcpurec.msize;++i) {
		vapiPrint("%s: Lin=%08x, Phy=%08x, Data=%08x, Bytes=%1x\n",
			vcpurec.mem[i].flagwrite ? "Write" : "Read",
			vcpurec.mem[i].linear, vcpurec.mem[i].physical,
			vcpurec.mem[i].data, vcpurec.mem[i].byte);
	}
}
static void xt()
{
	t_nubit32 i;
	t_nubit32 count;
	if (vmachine.flagrun) {
		vapiPrint("NXVM is already running.\n");
		return;
	}
	switch(narg) {
	case 1:
		count = 1;
		break;
	case 2:
		count = scannubit32(arg[1]);
		break;
	default:seterr(narg-1);break;}
	if(errPos) return;
	if (count < 0x0100) {
		for(i = 0;i < count;++i) {
			vmachine.tracecnt = 0x01;
			vmachineResume();
			while (vmachine.flagrun) vapiSleep(10);
			xtprintmem();
			xreg();
			if (i != count - 1) vapiPrint("\n");
		}
	} else {
		vmachine.tracecnt = count;
		vmachineResume();
		while (vmachine.flagrun) vapiSleep(10);
		xtprintmem();
		xreg();
	}
	vmachine.tracecnt = 0x00;
//	gexec(ptr1,ptr2);
}
static void xg()
{
	t_nubit32 i;
	t_nubit32 count = 0;
	if (vmachine.flagrun) {
		vapiPrint("NXVM is already running.\n");
		return;
	}
	switch(narg) {
	case 1:
		vmachine.flagbreakx = 0;
		count = 1;
		break;
	case 2:
		vmachine.flagbreakx = 1;
		vmachine.breaklinear = scannubit32(arg[1]);
		count = 1;
		break;
	case 3:
		vmachine.flagbreakx = 1;
		vmachine.breaklinear = scannubit32(arg[1]);
		count = scannubit32(arg[2]);
		break;
	default:seterr(narg-1);break;}
	if(errPos) return;
	for (i = 0;i < count;++i) {
		vmachine.breakcnt = 0;
		vmachineResume();
		while (vmachine.flagrun) vapiSleep(1);
		vapiPrint("%d instructions executed before the break point.\n", vmachine.breakcnt);
		xreg();
	}
	vmachine.flagbreakx = 0;
}
static void xr() {xreg();}
static void xw()
{
	switch(narg) {
	case 1:
		if (vcpuins.flagwr) vapiPrint("Watch-read point: Lin=%08x\n", vcpuins.wrlin);
		if (vcpuins.flagww) vapiPrint("Watch-write point: Lin=%08x\n", vcpuins.wwlin);
		if (vcpuins.flagwe) vapiPrint("Watch-exec point: Lin=%08x\n", vcpuins.wwlin);
		break;
	case 2:
		switch (arg[1][0]) {
		case 'r':
			vcpuins.flagwr = 0;
			vapiPrint("Watch-read point removed.\n");
			break;
		case 'w':
			vcpuins.flagww = 0;
			vapiPrint("Watch-write point removed.\n");
			break;
		case 'e':
			vcpuins.flagwe = 0;
			vapiPrint("Watch-exec point removed.\n");
			break;
		case 'u':
			vcpuins.flagwr = 0;
			vcpuins.flagww = 0;
			vcpuins.flagwe = 0;
			vapiPrint("All watch points removed.\n");
			break;
		default: seterr(1);break;}
		break;
	case 3:
		switch (arg[1][0]) {
		case 'r':
			vcpuins.flagwr = 1;
			vcpuins.wrlin = scannubit32(arg[2]);
			break;
		case 'w':
			vcpuins.flagww = 1;
			vcpuins.wwlin = scannubit32(arg[2]);
			break;
		case 'e':
			vcpuins.flagwe = 1;
			vcpuins.welin = scannubit32(arg[2]);
			break;
		default:
			seterr(2);
			break;
		}
		break;
	default:seterr(narg-1);break;}
}
static void x()
{
	t_nubitcc i;
	arg[narg] = arg[0];
	for (i = 1;i < narg;++i) arg[i - 1] = arg[i];
	arg[narg - 1] = arg[narg];
	arg[narg] = NULL;
	narg--;
	     if (!STRCMP(arg[0], "d"))    xd();
	else if (!STRCMP(arg[0], "g"))    xg();
	else if (!STRCMP(arg[0], "r"))    xr();
	else if (!STRCMP(arg[0], "t"))    xt();
	else if (!STRCMP(arg[0], "u"))    xu();
	else if (!STRCMP(arg[0], "w"))    xw();
	else if (!STRCMP(arg[0], "reg"))  xreg();
	else if (!STRCMP(arg[0], "sreg")) xsreg();
	else if (!STRCMP(arg[0], "creg")) xcreg();
	else {
		arg[0] = arg[narg];
		seterr(0);
	}
}
/* EXTENDED DEBUG CMD END */

static void help()
{
	vapiPrint("assemble\tA [address]\n");
	vapiPrint("compare\t\tC range address\n");
	vapiPrint("dump\t\tD [range]\n");
	vapiPrint("enter\t\tE address [list]\n");
	vapiPrint("fill\t\tF range list\n");
	vapiPrint("go\t\tG [[address] breakpoint]\n");
	//vapiPrint("go\t\tG [=address] [addresses]\n");
	vapiPrint("hex\t\tH value1 value2\n");
	vapiPrint("input\t\tI port\n");
	vapiPrint("load\t\tL [address]\n");
	//vapiPrint("load\t\tL [address] [drive] [firstsector] [number]\n");
	vapiPrint("move\t\tM range address\n");
	vapiPrint("name\t\tN [pathname]\n");
	//vapiPrint("name\t\tN [pathname] [arglist]\n");
	vapiPrint("output\t\tO port byte\n");
//!	vapiPrint("proceed\t\tP [=address] [number]\n");
	vapiPrint("quit\t\tQ\n");
	vapiPrint("register\tR [register]\n");
	vapiPrint("search\t\tS range list\n");
	vapiPrint("trace\t\tT [[address] value]\n");
	//vapiPrint("trace\t\tT [=address] [value]\n");
	vapiPrint("unassemble\tU [range]\n");
	vapiPrint("verbal\t\tV\n");
	vapiPrint("write\t\tW [address]\n");
	//vapiPrint("write\t\tW [address] [drive] [firstsector] [number]\n");
	//vapiPrint("allocate expanded memory\tXA [#pages]\n");
	//vapiPrint("deallocate expanded memory\tXD [handle]\n");
	//vapiPrint("map expanded memory pages\tXM [Lpage] [Ppage] [handle]\n");
	//vapiPrint("display expanded memory status\tXS\n");
}
static void init()
{
	filename[0] = '\0';
	asmSegRec = uasmSegRec = _cs;
	asmPtrRec = uasmPtrRec = _ip;
	dumpSegRec = _ds;
	dumpPtrRec = (t_nubit16)(_ip) / 0x10 * 0x10;
	xulin = vcpu.cs.base + vcpu.eip;
}
static void parse()
{
	STRCPY(cmdCopy,cmdBuff);
	narg = 0;
	arg[narg] = STRTOK(cmdCopy," ,\t\n\r\f");
	if(arg[narg]) {
		lcase(arg[narg]);
		narg++;
	} else return;
	if(strlen(arg[narg-1]) != 1) {
		arg[narg] = arg[narg-1]+1;
		narg++;
	}
	while(narg < MAXNARG) {
		arg[narg] = STRTOK(NULL," ,\t\n\r\f");
		if(arg[narg]) {
			lcase(arg[narg]);
			narg++;
		} else break;
	}
}
static void exec()
{
	errPos = 0;
	if(!arg[0]) return;
	switch(arg[0][0]) {
	case '\?':	help();break;
	case 'a':	a();break;
	case 'c':	c();break;
	case 'd':	d();break;
	case 'e':	e();break;
	case 'f':	f();break;
	case 'g':	g();break;
	case 'h':	h();break;
	case 'i':	i();break;
	case 'l':	l();break;
	case 'm':	m();break;
	case 'n':	n();break;
	case 'o':	o();break;
	case 'q':	q();break;
	case 'r':	r();break;
	case 's':	s();break;
	case 't':	t();break;
	case 'u':	u();break;
	case 'v':	v();break;
	case 'w':	w();break;
	case 'x':	x();break;
	default:
		seterr(0);
		break;
	}
}

void vapiCallBackDebugPrintRegs(t_bool bit32) {if (bit32) xreg(); else rprintregs();}
void debug()
{
	t_nubitcc i;
	init();
	arg = (char **)malloc(MAXNARG * sizeof(char *));
	exitFlag = 0x00;
	while(!exitFlag) {
		fflush(stdin);
		vapiPrint("-");
		FGETS(cmdBuff,MAXLINE,stdin);
		parse();
		exec();
		if(errPos) {
			for(i = 0;i < errPos;++i) vapiPrint(" ");
			vapiPrint("^ Error\n");
		}
	}
	free(arg);
}
