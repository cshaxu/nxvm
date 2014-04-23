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

#include "../vmachine.h"
#include "../vapi.h"

#include "aasm.h"
#include "dasm.h"
#include "debug.h"

#define MAXLINE 256
#define MAXNARG 256
#define MAXNASMARG 4

static t_nubit8 dbgbit;
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
		if(!STRCMP(cseg,"cs")) seg = vcpu.cs;
		else if(!STRCMP(cseg,"ss")) seg = vcpu.ss;
		else if(!STRCMP(cseg,"ds")) seg = vcpu.ds;
		else if(!STRCMP(cseg,"es")) seg = vcpu.es;
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
		addrparse(vcpu.cs,arg[1]);
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
		addrparse(vcpu.ds,arg[1]);
		seg1 = seg;
		ptr1 = ptr;
		addrparse(vcpu.ds,arg[3]);
		seg2 = seg;
		ptr2 = ptr;
		range = scannubit16(arg[2])-ptr1;
		if(!errPos) {
			for(i = 0;i <= range;++i) {
				val1 = vramVarByte(seg1,ptr1+i);
				val2 = vramVarByte(seg2,ptr2+i);
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
			c[i%0x10] = vramVarByte(segment,i);
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
		addrparse(vcpu.ds,arg[1]);
		if(errPos) return;
		dprint(seg,ptr,ptr+0x7f);
	} else if(narg == 3) {
		addrparse(vcpu.ds,arg[1]);
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
		addrparse(vcpu.ds,arg[1]);
		if(errPos) return;
		addrprint(seg,ptr);
		vapiPrint("%02X",vramVarByte(seg,ptr));
		vapiPrint(".");
		FGETS(s,MAXLINE,stdin);
		lcase(s);//!!
		val = scannubit8(s);//!!
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			vramVarByte(seg,ptr) = val;
	} else if(narg > 2) {
		addrparse(vcpu.ds,arg[1]);
		if(errPos) return;
		for(i = 2;i < narg;++i) {
			val = scannubit8(arg[i]);//!!
			if(!errPos) vramVarByte(seg,ptr) = val;
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
		addrparse(vcpu.ds,arg[1]);
		end = scannubit16(arg[2]);
		if(end < ptr) seterr(2);
		if(!errPos) {
			nbyte = narg - 3;
			for(i = ptr,j = 0;i <= end;++i,++j) {
				val = scannubit8(arg[j%nbyte+3]);
				if(!errPos) vramVarByte(seg,i) = val;
				else return;
			}
		}
	}
}
// go
static void rprintregs(t_nubit8 bit);
/*static void gexec(t_nubit16 ptr1,t_nubit16 ptr2)
{
	if(ptr1 < ptr2) {
		vcpu.eip = ptr1;
		while(ptr1 < ptr2 && vmachine.flagrun) {
			vmachineRefresh();
			ptr1 = vcpu.eip;
		}
		//vcpu.eip = ptr2;
	}
	if(!vmachine.flagrun) {
		vapiPrint("\nProgram terminated\n");
	} else {
		vapiPrint("\n");
		rprintregs();
	}
	return;
}*/
static void g()
{
//	t_nubit16 ptr1,ptr2;
	if (vmachine.flagrun) {
		vapiPrint("NXVM is already running.\n");
		return;
	}
	switch(narg) {
	case 1:
		vmachine.flagbreak = 0x00;
	//	ptr1 = vcpu.eip;ptr2 = 0xffff;
		break;
	case 2:
		vmachine.flagbreak = 0x01;
		addrparse(vcpu.cs,arg[1]);
		vmachine.breakcs = seg;
		vmachine.breakip = ptr;
	//	ptr1 = vcpu.eip;ptr2 = ptr;
		break;
	case 3:
		vmachine.flagbreak = 0x01;
		addrparse(vcpu.cs,arg[1]);
		vcpu.cs = seg;
		vcpu.eip = ptr;
	//	ptr1 = ptr;
		addrparse(vcpu.cs,arg[2]);
		vmachine.breakcs = seg;
		vmachine.breakip = ptr;
	//	ptr2 = ptr;
		break;
	default:seterr(narg-1);break;}
	if(errPos) return;
	vmachineResume();
	while (vmachine.flagrun) vapiSleep(1);
	vmachine.flagbreak = 0x00;
	rprintregs(16);
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
			seg = vcpu.cs;
			ptr = 0x100;
			break;
		case 2:
			addrparse(vcpu.cs,arg[1]);
			break;
		default:seterr(narg-1);break;}
		if(!errPos) {
			c = fgetc(load);
			while(!feof(load)) {
				vramVarByte(seg+i,ptr+len++) = c;
				i = len / 0x10000;
				c = fgetc(load);
			}
			vcpu.cx = len&0xffff;
			if(len > 0xffff) vcpu.bx = (len>>16);
			else vcpu.bx = 0x0000;
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
		addrparse(vcpu.ds,arg[1]);
		seg1 = seg;
		ptr1 = ptr;
		addrparse(vcpu.ds,arg[3]);
		seg2 = seg;
		ptr2 = ptr;
		range = scannubit16(arg[2])-ptr1;
		if(!errPos) {
			if(((seg1<<4)+ptr1) < ((seg2<<4)+ptr2)) {
				for(i = range;i >= 0;--i)
					vramVarByte(seg2,ptr2+i) = vramVarByte(seg1,ptr1+i);
			} else if(((seg1<<4)+ptr1) > ((seg2<<4)+ptr2)) {
				for(i = 0;i <= range;++i)
					vramVarByte(seg2,ptr2+i) = vramVarByte(seg1,ptr1+i);
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
static void rprintflags(t_nubit8 bit)
{
	if (bit == 32) {
		/*vapiPrint("ID%c ",   _GetID ? '+' : '-');
		vapiPrint("VIP%c ",  _GetVIP ? '+' : '-');
		vapiPrint("VIF%c ",  _GetVIF ? '+' : '-');
		vapiPrint("AC%c ",   _GetAC ? '+' : '-');*/
		vapiPrint("VM%c ",   _GetVM ? '+' : '-');
		vapiPrint("RF%c ",   _GetRF ? '+' : '-');
		vapiPrint("NT%c ",   _GetNT ? '+' : '-');
		vapiPrint("IOPL=%1d ", _GetIOPL);
	}
	vapiPrint("%s ", _GetOF ? "OV" : "NV");
	vapiPrint("%s ", _GetDF ? "DN" : "UP");
	vapiPrint("%s ", _GetIF ? "EI" : "DI");
	vapiPrint("%s ", _GetTF ? "TR" : "DT");
	vapiPrint("%s ", _GetSF ? "NG" : "PL");
	vapiPrint("%s ", _GetZF ? "ZR" : "NZ");
	vapiPrint("%s ", _GetAF ? "AC" : "NA");
	vapiPrint("%s ", _GetPF ? "PE" : "PO");
	vapiPrint("%s ", _GetCF ? "CY" : "NC");
}
static void rprintregs(t_nubit8 bit)
{
	char str[MAXLINE];
	switch (bit) {
	case 16:
		vapiPrint(  "AX=%04X", vcpu.ax);
		vapiPrint("  BX=%04X", vcpu.bx);
		vapiPrint("  CX=%04X", vcpu.cx);
		vapiPrint("  DX=%04X", vcpu.dx);
		vapiPrint("  SP=%04X", vcpu.sp);
		vapiPrint("  BP=%04X", vcpu.bp);
		vapiPrint("  SI=%04X", vcpu.si);
		vapiPrint("  DI=%04X", vcpu.di);
		vapiPrint("\nDS=%04X", vcpu.ds);
		vapiPrint("  ES=%04X", vcpu.es);
		vapiPrint("  SS=%04X", vcpu.ss);
		vapiPrint("  CS=%04X", vcpu.cs);
		vapiPrint("  IP=%04X", vcpu.eip);
		vapiPrint("   ");
		break;
	case 32:
		vapiPrint( "EAX=%08X", vcpu.eax);
		vapiPrint(" EBX=%08X", vcpu.ebx);
		vapiPrint(" ECX=%08X", vcpu.ecx);
		vapiPrint(" EDX=%08X", vcpu.edx);
		vapiPrint("  SS=%04X", vcpu.ss);
		vapiPrint( " CS=%04X", vcpu.cs);
		vapiPrint( " DS=%04X", vcpu.ds);
		vapiPrint("\nESP=%08X",vcpu.esp);
		vapiPrint(" EBP=%08X", vcpu.ebp);
		vapiPrint(" ESI=%08X", vcpu.esi);
		vapiPrint(" EDI=%08X", vcpu.edi);
		vapiPrint("  ES=%04X", vcpu.es);
		vapiPrint( " FS=%04X", vcpu.fs);
		vapiPrint( " GS=%04X", vcpu.gs);
		vapiPrint("\nEIP=%08X", vcpu.eip);
		vapiPrint("  ");
	default:
		break;
	}
	rprintflags(bit);
	vapiPrint("\n");
	dasm(str, vcpu.cs, vcpu.eip, 0x02);
	uasmSegRec = vcpu.cs;
	uasmPtrRec = vcpu.eip;
	vapiPrint("%s", str);
}
static void rscanregs()
{
	t_nubit16 t;
	char s[MAXLINE];
	if(!STRCMP(arg[1],"ax")) {
		vapiPrint("AX ");
		vapiPrint("%04X",vcpu.ax);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			vcpu.ax = t;
	} else if(!STRCMP(arg[1],"bx")) {
		vapiPrint("BX ");
		vapiPrint("%04X",vcpu.bx);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			vcpu.bx = t;
	} else if(!STRCMP(arg[1],"cx")) {
		vapiPrint("CX ");
		vapiPrint("%04X",vcpu.cx);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			vcpu.cx = t;
	} else if(!STRCMP(arg[1],"dx")) {
		vapiPrint("DX ");
		vapiPrint("%04X",vcpu.dx);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			vcpu.dx = t;
	} else if(!STRCMP(arg[1],"bp")) {
		vapiPrint("BP ");
		vapiPrint("%04X",vcpu.bp);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			vcpu.bp = t;
	} else if(!STRCMP(arg[1],"sp")) {
		vapiPrint("SP ");
		vapiPrint("%04X",vcpu.sp);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			vcpu.sp = t;
	} else if(!STRCMP(arg[1],"si")) {
		vapiPrint("SI ");
		vapiPrint("%04X",vcpu.si);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			vcpu.si = t;
	} else if(!STRCMP(arg[1],"di")) {
		vapiPrint("DI ");
		vapiPrint("%04X",vcpu.di);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			vcpu.di = t;
	} else if(!STRCMP(arg[1],"ss")) {
		vapiPrint("SS ");
		vapiPrint("%04X",vcpu.ss);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos) {
			vcpu.overss = vcpu.ss = t;
		}
	} else if(!STRCMP(arg[1],"cs")) {
		vapiPrint("CS ");
		vapiPrint("%04X",vcpu.cs);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			vcpu.cs = t;
	} else if(!STRCMP(arg[1],"ds")) {
		vapiPrint("DS ");
		vapiPrint("%04X",vcpu.ds);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos) {
			vcpu.overds = vcpu.ds = t;
		}
	} else if(!STRCMP(arg[1],"es")) {
		vapiPrint("ES ");
		vapiPrint("%04X",vcpu.es);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			vcpu.es = t;
	} else if(!STRCMP(arg[1],"ip")) {
		vapiPrint("IP ");
		vapiPrint("%04X",vcpu.eip);
		vapiPrint("\n:");
		FGETS(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			vcpu.eip = t;
	} else if(!STRCMP(arg[1],"f")) {
		rprintflags(16);
		vapiPrint(" -");
		FGETS(s,MAXLINE,stdin);
		lcase(s);
		if(!STRCMP(s,"ov"))      _SetOF;
		else if(!STRCMP(s,"nv")) _ClrOF;
		else if(!STRCMP(s,"dn")) _SetDF;
		else if(!STRCMP(s,"up")) _ClrDF;
		else if(!STRCMP(s,"ei")) _SetIF;
		else if(!STRCMP(s,"di")) _ClrIF;
		else if(!STRCMP(s,"ng")) _SetSF;
		else if(!STRCMP(s,"pl")) _ClrSF;
		else if(!STRCMP(s,"zr")) _SetZF;
		else if(!STRCMP(s,"nz")) _ClrZF;
		else if(!STRCMP(s,"ac")) _SetAF;
		else if(!STRCMP(s,"na")) _ClrAF;
		else if(!STRCMP(s,"pe")) _SetPF;
		else if(!STRCMP(s,"po")) _ClrPF;
		else if(!STRCMP(s,"cy")) _SetCF;
		else if(!STRCMP(s,"nc")) _ClrCF;
		else vapiPrint("bf Error\n");
	} else vapiPrint("br Error\n");
}
static void r()
{
	if(narg == 1) {
		rprintregs(dbgbit);
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
		addrparse(vcpu.ds,arg[1]);
		start = ptr;
		end = scannubit16(arg[2]);
		if(!errPos) {
			p = start;
			while(p <= end) {
				if(vramVarByte(seg,p) == scannubit8(arg[3])) {
					pfront = p;
					flag = 0x01;
					for(i = 3;i < narg;++i) {
						if(vramVarByte(seg,p) != scannubit8(arg[i])) {
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
	t_nubitcc i, count;
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
		addrparse(vcpu.cs,arg[1]);
		vcpu.cs = seg;
		vcpu.eip = ptr;
		count = scannubit16(arg[2]);
		break;
	default:seterr(narg-1);break;}
	if(errPos) return;
	if (count < 0x0100) {
		for(i = 0;i < count;++i) {
			vmachine.tracecnt = 0x01;
			vmachineResume();
			while (vmachine.flagrun) vapiSleep(10);
			rprintregs(16);
			if (i != count - 1) vapiPrint("\n");
		}
	} else {
		vmachine.tracecnt = count;
		vmachineResume();
		while (vmachine.flagrun) vapiSleep(10);
		rprintregs(16);
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
		addrparse(vcpu.cs,arg[1]);
		if(errPos) return;
		uprint(seg,ptr,ptr+0x1f);
	} else if(narg == 3) {
		addrparse(vcpu.ds,arg[1]);
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
	t_nubit32 len = (vcpu.bx<<16)+vcpu.cx;
	FILE *write;
	if(!strlen(filename)) {vapiPrint("(W)rite error, no destination defined\n");return;}
	else write= FOPEN(filename,"wb");
	if(!write) vapiPrint("File not found\n");
	else {
		vapiPrint("Writing ");
		vapiPrint("%04X",vcpu.bx);
		vapiPrint("%04X",vcpu.cx);
		vapiPrint(" bytes\n");
		switch(narg) {
		case 1:
			seg = vcpu.cs;
			ptr = 0x100;
			break;
		case 2:
			addrparse(vcpu.cs,arg[1]);
			break;
		default:	seterr(narg-1);break;}
		if(!errPos)
			while(i < len)
				fputc(vramVarByte(seg,ptr+i++),write);
		fclose(write);
	}
}
/* DEBUG CMD END */

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
	asmSegRec = uasmSegRec = vcpu.cs;
	asmPtrRec = uasmPtrRec = vcpu.eip;
	dumpSegRec = vcpu.ds;
	dumpPtrRec = (t_nubit16)(vcpu.eip) / 0x10 * 0x10;
/*	vcpu.ax = vcpu.bx = vcpu.cx = vcpu.dx = 0x0000;
	vcpu.si = vcpu.di = vcpu.bp = 0x0000;
	vcpu.sp = 0xffee;
	vcpu.ds = vcpu.es = vcpu.ss = vcpu.cs =
		asmSegRec = dumpSegRec = uasmSegRec = 0x0001;
	vcpu.eip = asmPtrRec = dumpPtrRec = uasmPtrRec = 0x0100;*/
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

void vapiCallBackDebugPrintRegs(t_nubit8 bit) {rprintregs(bit);}
void debug(t_nubit8 bit)
{
	t_nubitcc i;
	dbgbit = bit;
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
