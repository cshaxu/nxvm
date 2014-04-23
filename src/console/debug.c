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

#include "../vmachine/vmachine.h"
#include "../vmachine/vapi.h"

#include "asm86.h"
#include "debug.h"

#define MAXLINE 256
#define MAXNARG 256
#define MAXNASMARG 4

static int errPos;
static int narg;
static char **arg;
static int exitFlag;
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
static t_nubit32 debugprint(const t_string format, ...)
{
	t_nubit32 nWrittenBytes = 0;
	va_list arg_ptr;
	va_start(arg_ptr, format);
	nWrittenBytes = vfprintf(stdout,format,arg_ptr);
	//nWrittenBytes = vsprintf(stringBuffer,format,arg_ptr);
	va_end(arg_ptr);
	fflush(stdout);
	return nWrittenBytes;
}
static void lcase(char *s)
{
	int i = 0;
	if(s[0] == '\'') return;
	while(s[i] != '\0') {
		if(s[i] == '\n') s[i] = '\0';
		else if(s[i] > 0x40 && s[i] < 0x5b)
			s[i] += 0x20;
		i++;
	}
}
static void seterr(int pos)
{
	errPos = (int)(arg[pos] - cmdCopy + strlen(arg[pos]) + 1);
}
static t_nubit8 scannubit8(char *s)
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
static t_nubit16 scannubit16(char *s)
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
static void printnubit8(t_nubit8 n)
{
/*	char c;
	int i;
	for(i = 1;i >= 0;--i) {
		c = ((n>>(i*4))&0x0f)+0x30;
		if(c > 0x39) c += 0x07;
		debugprint("%c",c);
	}*/
	debugprint("%02X",n);
}
static void printnubit16(t_nubit16 n)
{
/*	char c;
	int i;
	for(i = 3;i >= 0;--i) {
		c = ((n>>(i*4))&0x0f)+0x30;
		if(c > 0x39) c += 0x07;
		debugprint("%c",c);
	}*/
	debugprint("%04X",n);
}
static void addrparse(t_nubit16 defseg,const char *addr)
{
	char *cseg,*cptr,ccopy[MAXLINE];
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
	printnubit16(segment);
	debugprint(":");
	printnubit16(pointer);
	debugprint("  ");
	/*fflush(stdout);*/
}
static void setbyte(t_nubit16 segment,t_nubit16 pointer,t_nubit8 value)
{
	vramVarByte(segment, pointer) = value;
}
static t_nubit8 getbyte(t_nubit16 segment,t_nubit16 pointer)
{
	return vramVarByte(segment, pointer);
}
static t_nubit16 getword(t_nubit16 segment,t_nubit16 pointer)
{
	return vramVarWord(segment, pointer);
}
static t_bool isprefix(t_nubit8 n)
{
	switch(n) {
	case 0xf0: case 0xf2: case 0xf3:
	case 0x2e: case 0x36: case 0x3e: case 0x26:
	//case 0x64: case 0x65: case 0x66: case 0x67:
				return 1;break;
	default:	return 0;break;
	}
}


/* DEBUG CMD BEGIN */
// assemble
static void aconsole()
{
	int i;
	t_nubit16 len;
	char cmdAsmBuff[MAXLINE];
	int errAsmPos;
	int exitAsmFlag = 0;
	while(!exitAsmFlag) {
		addrprint(asmSegRec,asmPtrRec);
		fflush(stdin);
		debugprint("\b");
		/*fflush(stdout);*/
		fgets(cmdAsmBuff,MAXLINE,stdin);
		lcase(cmdAsmBuff);
		if(!strlen(cmdAsmBuff)) {
			exitAsmFlag = 1;
			continue;
		}
		if(cmdAsmBuff[0] == ';' ) continue;
		errAsmPos = 0;
		len = assemble(cmdAsmBuff,_cs,
			(void *)vramGetAddr(0x0000,0x0000),asmSegRec,asmPtrRec);
		if(!len) errAsmPos = (int)strlen(cmdAsmBuff) + 9;
		else asmPtrRec += len;
		if(errAsmPos) {
			for(i = 0;i < errAsmPos;++i) debugprint(" ");
			debugprint("^ Error\n");
		}
		/*fflush(stdout);*/
	}
}
static void a()
{
	if(narg == 1) {
		aconsole();
	} else if(narg == 2) {
		addrparse(_cs,arg[1]);
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
				val1 = getbyte(seg1,ptr1+i);
				val2 = getbyte(seg2,ptr2+i);
				if(val1 != val2) {
					addrprint(seg1,ptr1+1);
					debugprint("  ");
					printnubit8(val1);
					debugprint("  ");
					printnubit8(val2);
					debugprint("  ");
					addrprint(seg2,ptr2+1);
					debugprint("\n");
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
			debugprint("  ");
			c[i%0x10] = ' ';
		} else {
			c[i%0x10] = getbyte(segment,i);
			printnubit8(c[i%0x10]);
			t = c[i%0x10];
			if((t >=1 && t <= 7) || t == ' ' ||
				(t >=11 && t <= 12) ||
				(t >=14 && t <= 31) ||
				(t >=33 && t <= 128)) ;
			else c[i%0x10] = '.';
		}
		debugprint(" ");
		if(i%0x10 == 7 && i >= start && i < end) debugprint("\b-");
		if((i+1)%0x10 == 0) {
			debugprint("  %s\n",c);
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
	int i;
	t_nubit8 val;
	char s[MAXLINE];
	if(narg == 1) seterr(0);
	else if(narg == 2) {
		addrparse(_ds,arg[1]);
		if(errPos) return;
		addrprint(seg,ptr);
		printnubit8(getbyte(seg,ptr));
		debugprint(".");
		/*fflush(stdout);*/
		fgets(s,MAXLINE,stdin);
		lcase(s);//!!
		val = scannubit8(s);//!!
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			setbyte(seg,ptr,val);
	} else if(narg > 2) {
		addrparse(_ds,arg[1]);
		if(errPos) return;
		for(i = 2;i < narg;++i) {
			val = scannubit8(arg[i]);//!!
			if(!errPos) setbyte(seg,ptr,val);
			else break;
			ptr++;
		}
	}
}
// fill
static void f()
{
	int nbyte;
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
				if(!errPos) setbyte(seg,i,val);
				else return;
			}
		}
	}
}
// go
static void rprintregs();
/*static void gexec(t_nubit16 ptr1,t_nubit16 ptr2)
{
	if(ptr1 < ptr2) {
		_ip = ptr1;
		while(ptr1 < ptr2 && vmachine.flagrun) {
			vmachineRefresh();
			ptr1 = _ip;
		}
		//_ip = ptr2;
	}
	if(!vmachine.flagrun) {
		debugprint("\nProgram terminated\n");
	} else {
		debugprint("\n");
		rprintregs();
	}
	return;
}*/
static void g()
{
//	t_nubit16 ptr1,ptr2;
	if (vmachine.flagrun) {
		debugprint("NXVM is running.\n");
		return;
	}
	switch(narg) {
	case 1:
		vmachine.flagbreak = 0x00;
	//	ptr1 = _ip;ptr2 = 0xffff;
		break;
	case 2:
		vmachine.flagbreak = 0x01;
		addrparse(_cs,arg[1]);
		vmachine.breakcs = seg;
		vmachine.breakip = ptr;
	//	ptr1 = _ip;ptr2 = ptr;
		break;
	case 3:
		vmachine.flagbreak = 0x01;
		addrparse(_cs,arg[1]);
		_cs = seg;
		_ip = ptr;
	//	ptr1 = ptr;
		addrparse(_cs,arg[2]);
		vmachine.breakcs = seg;
		vmachine.breakip = ptr;
	//	ptr2 = ptr;
		break;
	default:seterr(narg-1);break;}
	if(errPos) return;
	vmachineResume();
	while (vmachine.flagrun) vapiSleep(1);
	vmachine.flagbreak = 0x00;
	rprintregs();
//	gexec(ptr1,ptr2);
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
			printnubit16(val1+val2);
			debugprint("  ");
			printnubit16(val1-val2);
			debugprint("\n");
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
			printnubit8(vport.iobyte);
			debugprint("\n");
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
	if(!load) debugprint("File not found\n");
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
				setbyte(seg+i,ptr+len++,c);
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
	t_nubit8 val;
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
				for(i = range;i >= 0;--i) {
					val = getbyte(seg1,ptr1+i);
					setbyte(seg2,ptr2+i,val);
				}
			} else if(((seg1<<4)+ptr1) > ((seg2<<4)+ptr2)) {
				for(i = 0;i <= range;++i) {
					val = getbyte(seg1,ptr1+i);
					setbyte(seg2,ptr2+i,val);
				}
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
	if(_flags & VCPU_FLAG_OF) debugprint("OV ");
	else                      debugprint("NV ");
	if(_flags & VCPU_FLAG_DF) debugprint("DN ");
	else                      debugprint("UP ");
	if(_flags & VCPU_FLAG_IF) debugprint("EI ");
	else                      debugprint("DI ");
	if(_flags & VCPU_FLAG_SF) debugprint("NG ");
	else                      debugprint("PL ");
	if(_flags & VCPU_FLAG_ZF) debugprint("ZR ");
	else                      debugprint("NZ ");
	if(_flags & VCPU_FLAG_AF) debugprint("AC ");
	else                      debugprint("NA ");
	if(_flags & VCPU_FLAG_PF) debugprint("PE ");
	else                      debugprint("PO ");
	if(_flags & VCPU_FLAG_CF) debugprint("CY ");
	else                      debugprint("NC ");
}
static void rprintregs()
{
	debugprint(  "AX=%04X", _ax);
	debugprint("  BX=%04X", _bx);
	debugprint("  CX=%04X", _cx);
	debugprint("  DX=%04X", _dx);
	debugprint("  SP=%04X", _sp);
	debugprint("  BP=%04X", _bp);
	debugprint("  SI=%04X", _si);
	debugprint("  DI=%04X", _di);
	debugprint("\nDS=%04X", _ds);
	debugprint("  ES=%04X", _es);
	debugprint("  SS=%04X", _ss);
	debugprint("  CS=%04X", _cs);
	debugprint("  IP=%04X", _ip);
	debugprint("   ");
	rprintflags();
	debugprint("\n");
	uprint(_cs,_ip,_ip);
}
static void rscanregs()
{
	t_nubit16 t;
	char s[MAXLINE];
	if(!STRCMP(arg[1],"ax")) {
		debugprint("AX ");
		printnubit16(_ax);
		debugprint("\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_ax = t;
	} else if(!STRCMP(arg[1],"bx")) {
		debugprint("BX ");
		printnubit16(_bx);
		debugprint("\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_bx = t;
	} else if(!STRCMP(arg[1],"cx")) {
		debugprint("CX ");
		printnubit16(_cx);
		debugprint("\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_cx = t;
	} else if(!STRCMP(arg[1],"dx")) {
		debugprint("DX ");
		printnubit16(_dx);
		debugprint("\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_dx = t;
	} else if(!STRCMP(arg[1],"bp")) {
		debugprint("BP ");
		printnubit16(_bp);
		debugprint("\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_bp = t;
	} else if(!STRCMP(arg[1],"sp")) {
		debugprint("SP ");
		printnubit16(_sp);
		debugprint("\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_sp = t;
	} else if(!STRCMP(arg[1],"si")) {
		debugprint("SI ");
		printnubit16(_si);
		debugprint("\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_si = t;
	} else if(!STRCMP(arg[1],"di")) {
		debugprint("DI ");
		printnubit16(_di);
		debugprint("\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_di = t;
	} else if(!STRCMP(arg[1],"ss")) {
		debugprint("SS ");
		printnubit16(_ss);
		debugprint("\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos) {
			vcpu.overss = _ss = t;
		}
	} else if(!STRCMP(arg[1],"cs")) {
		debugprint("CS ");
		printnubit16(_cs);
		debugprint("\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_cs = t;
	} else if(!STRCMP(arg[1],"ds")) {
		debugprint("DS ");
		printnubit16(_ds);
		debugprint("\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos) {
			vcpu.overds = _ds = t;
		}
	} else if(!STRCMP(arg[1],"es")) {
		debugprint("ES ");
		printnubit16(_es);
		debugprint("\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_es = t;
	} else if(!STRCMP(arg[1],"ip")) {
		debugprint("IP ");
		printnubit16(_ip);
		debugprint("\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			_ip = t;
	} else if(!STRCMP(arg[1],"f")) {
		rprintflags();
		debugprint(" -");
		fgets(s,MAXLINE,stdin);
		lcase(s);
		if(!STRCMP(s,"ov"))      SetOF;
		else if(!STRCMP(s,"nv")) ClrOF;
		else if(!STRCMP(s,"dn")) SetDF;
		else if(!STRCMP(s,"up")) ClrDF;
		else if(!STRCMP(s,"ei")) SetIF;
		else if(!STRCMP(s,"di")) ClrIF;
		else if(!STRCMP(s,"ng")) SetSF;
		else if(!STRCMP(s,"pl")) ClrSF;
		else if(!STRCMP(s,"zr")) SetZF;
		else if(!STRCMP(s,"nz")) ClrZF;
		else if(!STRCMP(s,"ac")) SetAF;
		else if(!STRCMP(s,"na")) ClrAF;
		else if(!STRCMP(s,"pe")) SetPF;
		else if(!STRCMP(s,"po")) ClrPF;
		else if(!STRCMP(s,"cy")) SetCF;
		else if(!STRCMP(s,"nc")) ClrCF;
		else debugprint("bf Error\n");
	} else debugprint("br Error\n");
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
	int i;
	int flag;
	t_nubit16 p,pfront,start,end;
	if(narg < 4) seterr(narg-1);
	else {
		addrparse(_ds,arg[1]);
		start = ptr;
		end = scannubit16(arg[2]);
		if(!errPos) {
			p = start;
			while(p <= end) {
				if(getbyte(seg,p) == scannubit8(arg[3])) {
					pfront = p;
					flag = 1;
					for(i = 3;i < narg;++i) {
						if(getbyte(seg,p) != scannubit8(arg[i])) {
							flag = 0;
							p = pfront+1;
							break;
						} else ++p;
					}
					if(flag) {
						addrprint(seg,pfront);
						debugprint("\n");
					}
				} else ++p;
			}
		}
	}
}
// trace
static void t()
{
	unsigned short i, count;
	if (vmachine.flagrun) {
		debugprint("NXVM is running.\n");
		return;
	}
	switch(narg) {
	case 1:
		vmachine.flagtrace = 0x01;
		count = 1;
		break;
	case 2:
		vmachine.flagtrace = 0x01;
		count = scannubit16(arg[1]);
		break;
	case 3:
		vmachine.flagtrace = 0x01;
		addrparse(_cs,arg[1]);
		_cs = seg;
		_ip = ptr;
		count = scannubit16(arg[2]);
		break;
	default:seterr(narg-1);break;}
	if(errPos) return;
	for(i = 0;i < count;++i) {
		vmachineResume();
		while (vmachine.flagrun) vapiSleep(1);
		if (count < 0x0100 || i == count - 0x01)
			rprintregs();
	}
	vmachine.flagtrace = 0x00;
//	gexec(ptr1,ptr2);
}
// unassemble
static t_nubit16 uoffset(Operand opr)
{
	t_nubit16 res = 0;
	switch(opr.mod) {
	case 0:
		switch(opr.rm) {
		case 0:	res = _bx+_si;break;
		case 1:	res = _bx+_di;break;
		case 2:	res = _bp+_si;break;
		case 3:	res = _bp+_di;break;
		case 4:	res = _si;break;
		case 5:	res = _di;break;
		case 6:	res = opr.imm;break;
		case 7:	res = _bx;break;
		default:debugprint("(ERROR:OFFSET)");break;}
		break;
	case 1:
	case 2:
		switch(opr.rm) {
		case 0:	res = _bx+_si+opr.imm;break;
		case 1:	res = _bx+_di+opr.imm;break;
		case 2:	res = _bp+_si+opr.imm;break;
		case 3:	res = _bp+_di+opr.imm;break;
		case 4:	res = _si+opr.imm;break;
		case 5:	res = _di+opr.imm;break;
		case 6:	res = _bp+opr.imm;break;
		case 7:	res = _bx+opr.imm;break;
		default:debugprint("(ERROR:OFFSET)");break;}
		break;
	default:debugprint("(ERROR:OFFSET)");break;}
	return res;
}
static void uprint(t_nubit16 segment,t_nubit16 start,t_nubit16 end)
{
	char str[MAXLINE],*op = NULL, *stmt = NULL;
	t_nubit16 pos = 0,len = 0;
	int i,prefixflag;
	t_nubit16 offset;
	t_nubit32 boundary;
	Operand operand;
	operand.flag = 4;
	operand.seg = 0x00;
	if(start > end) end = 0xffff;
	if ((t_nubit32)((segment<<4) + end) > 0xfffff) end = (0xfffff-(segment<<4));
	while(start <= end) {
		pos = 0;
		prefixflag = 0;
		while(!pos || prefixflag) {
			if(isprefix(getbyte(segment,start+pos))) prefixflag = 1;
			else prefixflag = 0;
			printnubit16(segment);
			debugprint(":");
			printnubit16(start+pos);
			debugprint(" ");
			len = disassemble(str,&operand,
				(void *)vramGetAddr(0x0000,0x0000),segment,start+pos);
			for(i = 0;i < len;++i)
				printnubit8(getbyte(segment,start+pos+i));
			pos += len;
			if(len < 3) debugprint("\t\t");
			else debugprint("\t");
			op = STRTOK(str,"\t");
			if(!len || !op) {debugprint("fail to unassemble\n");return;}
			else debugprint("%s\t",op);
			stmt = STRTOK(NULL,"\0");
			if(stmt) {
				debugprint("%s",stmt);
				if(strlen(stmt) < 8) debugprint("\t\t\t\t");
				else if(strlen(stmt) < 16) debugprint("\t\t\t");
				else if(strlen(stmt) < 24) debugprint("\t\t");
				else if(strlen(stmt) < 30) debugprint("\t");
			}
			//debugprint("opr.seg=%d,opr.flag=%d\n",operand.seg,operand.flag);
			if(operand.seg == 1 && operand.flag != 4) {
				switch(operand.flag) {
				case 0:	debugprint("ES:");offset = uoffset(operand);
					printnubit16(offset);debugprint("=");
					switch(operand.len) {
					case 1:printnubit8(getbyte(_es,offset));break;
					case 2:printnubit16(getword(_es,offset));break;
					default:debugprint("(ERROR:OPERANDES)");break;}
					break;
				case 1:	debugprint("CS:");offset = uoffset(operand);
					printnubit16(offset);debugprint("=");
					switch(operand.len) {
					case 1:printnubit8(getbyte(_cs,offset));break;
					case 2:printnubit16(getword(_cs,offset));break;
					default:debugprint("(ERROR:OPERANDCS)");break;}
					break;
				case 2:	debugprint("SS:");offset = uoffset(operand);
					printnubit16(offset);debugprint("=");
					switch(operand.len) {
					case 1:printnubit8(getbyte(_ss,offset));break;
					case 2:printnubit16(getword(_ss,offset));break;
					default:debugprint("(ERROR:OPERANDSS)");break;}
					break;
				case 3:	debugprint("DS:");offset = uoffset(operand);
					printnubit16(offset);debugprint("=");
					switch(operand.len) {
					case 1:printnubit8(getbyte(_ds,offset));break;
					case 2:printnubit16(getword(_ds,offset));break;
					default:debugprint("(ERROR:OPERANDDS)");break;}
					break;
				default:debugprint("(ERROR:OPERAND)");break;}
				operand.flag = 4;
			}
			debugprint("\n");
		}
		start += pos;
		boundary = (t_nubit32)start + (t_nubit32)pos;
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
	unsigned int i;
	char str[MAXLINE];
	debugprint(":");
	fgets(str,MAXLINE,stdin);
	str[strlen(str)-1] = '\0';
	for(i = 0;i < strlen(str);++i) {
		printnubit8(str[i]);
		if(!((i+1)%0x10)) debugprint("\n"); 
		else if(!((i+1)%0x08)&&(str[i+1]!='\0')) debugprint("-");
		else debugprint(" ");
	}
	if(i%0x10) debugprint("\n");
}
// write
static void w()
{
	t_nubit16 i = 0;
	t_nubit32 len = (_bx<<16)+_cx;
	FILE *write;
	if(!strlen(filename)) {debugprint("(W)rite error, no destination defined\n");return;}
	else write= FOPEN(filename,"wb");
	if(!write) debugprint("File not found\n");
	else {
		debugprint("Writing ");
		printnubit16(_bx);
		printnubit16(_cx);
		debugprint(" bytes\n");
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
				fputc(getbyte(seg,ptr+i++),write);
		fclose(write);
	}
}
/* DEBUG CMD END */

static void help()
{
	debugprint("assemble\tA [address]\n");
	debugprint("compare\t\tC range address\n");
	debugprint("dump\t\tD [range]\n");
	debugprint("enter\t\tE address [list]\n");
	debugprint("fill\t\tF range list\n");
	debugprint("go\t\tG [[address] breakpoint]\n");
	//debugprint("go\t\tG [=address] [addresses]\n");
	debugprint("hex\t\tH value1 value2\n");
	debugprint("input\t\tI port\n");
	debugprint("load\t\tL [address]\n");
	//debugprint("load\t\tL [address] [drive] [firstsector] [number]\n");
	debugprint("move\t\tM range address\n");
	debugprint("name\t\tN [pathname]\n");
	//debugprint("name\t\tN [pathname] [arglist]\n");
	debugprint("output\t\tO port byte\n");
//!	debugprint("proceed\t\tP [=address] [number]\n");
	debugprint("quit\t\tQ\n");
	debugprint("register\tR [register]\n");
	debugprint("search\t\tS range list\n");
	debugprint("trace\t\tT [[address] value]\n");
	//debugprint("trace\t\tT [=address] [value]\n");
	debugprint("unassemble\tU [range]\n");
	debugprint("verbal\t\tV\n");
	debugprint("write\t\tW [address]\n");
	//debugprint("write\t\tW [address] [drive] [firstsector] [number]\n");
	//debugprint("allocate expanded memory\tXA [#pages]\n");
	//debugprint("deallocate expanded memory\tXD [handle]\n");
	//debugprint("map expanded memory pages\tXM [Lpage] [Ppage] [handle]\n");
	//debugprint("display expanded memory status\tXS\n");
}
static void init()
{
	filename[0] = '\0';
	asmSegRec = uasmSegRec = _cs;
	asmPtrRec = uasmPtrRec = _ip;
	dumpSegRec = _ds;
	dumpPtrRec = _ip;
/*	_ax = _bx = _cx = _dx = 0x0000;
	_si = _di = _bp = 0x0000;
	_sp = 0xffee;
	_ds = _es = _ss = _cs =
		asmSegRec = dumpSegRec = uasmSegRec = 0x0001;
	_ip = asmPtrRec = dumpPtrRec = uasmPtrRec = 0x0100;*/
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
	default:
		seterr(0);
		break;
	}
}

void debug()
{
	int i;
	init();
	arg = (char **)malloc(MAXNARG * sizeof(char *));
	exitFlag = 0;
	while(!exitFlag) {
		fflush(stdin);
		debugprint("-");
		fgets(cmdBuff,MAXLINE,stdin);
		parse();
		exec();
		if(errPos) {
			for(i = 0;i < errPos;++i) debugprint(" ");
			debugprint("^ Error\n");
		}
	}
	free(arg);
}
