/* This file is a part of NekoVMac project. */

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

#include "nvm/vmachine.h"
#include "nvm/vcpu.h"
#define cpu vCPU
#define memory memoryBase

#include "asm86/asm86.h"

#include "global.h"
#include "debug.h"

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

static void lcase(char *s)
{
	int i = 0;
	while(s[i] != '\0') {
		if(s[i] == '\n') s[i] = '\0';
		else if(s[i] > 0x40 && s[i] < 0x5b)
			s[i] += 0x20;
		i++;
	}
}
static void seterr(int pos)
{
	errPos = arg[pos] - cmdCopy + strlen(arg[pos]) + 1;
}
static t_nubit8 scannubit8(char *s)
{
	t_nubit8 ans = 0;
	int i = 0;
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
	char c;
	int i;
	for(i = 1;i >= 0;--i) {
		c = ((n>>(i*4))&0x0f)+0x30;
		if(c > 0x39) c += 0x07;
		fprintf(stdout,"%c",c);
	}
}
static void printnubit16(t_nubit16 n)
{
	char c;
	int i;
	for(i = 3;i >= 0;--i) {
		c = ((n>>(i*4))&0x0f)+0x30;
		if(c > 0x39) c += 0x07;
		fprintf(stdout,"%c",c);
	}
}
static void addrparse(t_nubit16 defseg,const char *addr)
{
	char *cseg,*cptr,ccopy[MAXLINE];
	strcpy(ccopy,addr);
	cseg = strtok(ccopy,":");
	cptr = strtok(NULL,"");
	if(!cptr) {
		seg = defseg;
		ptr = scannubit16(cseg);
	} else {
		if(!strcmp(cseg,"cs")) seg = cpu.cs;
		else if(!strcmp(cseg,"ss")) seg = cpu.ss;
		else if(!strcmp(cseg,"ds")) seg = cpu.ds;
		else if(!strcmp(cseg,"es")) seg = cpu.es;
		else seg = scannubit16(cseg);
		ptr = scannubit16(cptr);
	}
}
static void addrprint(t_nubit16 segment,t_nubit16 pointer)
{	
	printnubit16(segment);
	fprintf(stdout,":");
	printnubit16(pointer);
	fprintf(stdout,"  ");
}
static void setbyte(t_nubit16 segment,t_nubit16 pointer,t_nubit8 value)
{
	*(t_nubit8 *)(memory+(segment<<4)+pointer) = value;
}
static t_nubit8 getbyte(t_nubit16 segment,t_nubit16 pointer)
{
	return (t_nubit8)(*(t_nubit8 *)(memory+(segment<<4)+pointer));
}
static t_nubit16 getword(t_nubit16 segment,t_nubit16 pointer)
{
	t_nubit16 p1,p2;
	p1 = (*(t_nubit8 *)(memory+(segment<<4)+pointer));
	p2 = (*(t_nubit8 *)(memory+(segment<<4)+pointer+1));
	return (p1+(p2<<8));
}
static int isprefix(t_nubit8 n)
{
	switch(n) {
	case 0xf0: case 0xf2: case 0xf3:
	case 0x2e: case 0x36: case 0x3e: case 0x26: case 0x64: case 0x65:
	case 0x66: case 0x67: return 1;break;
	default: return 0;break;
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
		fprintf(stdout,"\b");
		fgets(cmdAsmBuff,MAXLINE,stdin);
		lcase(cmdAsmBuff);
		if(!strlen(cmdAsmBuff)) {
			exitAsmFlag = 1;
			continue;
		}
		if(cmdAsmBuff[0] == ';' ) continue;
		errAsmPos = 0;
		len = assemble(cmdAsmBuff,cpu.cs,
			(t_nubit8 *)memory,asmSegRec,asmPtrRec);
		if(!len) errAsmPos = strlen(cmdAsmBuff) + 9;
		else asmPtrRec += len;
		if(errAsmPos) {
			for(i = 0;i < errAsmPos;++i) fprintf(stdout," ");
			fprintf(stdout,"^ Error\n");
		}
	}
}
static void a()
{
	if(narg == 1) {
		aconsole();
	} else if(narg == 2) {
		addrparse(cpu.cs,arg[1]);
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
		addrparse(cpu.ds,arg[1]);
		seg1 = seg;
		ptr1 = ptr;
		addrparse(cpu.ds,arg[3]);
		seg2 = seg;
		ptr2 = ptr;
		range = scannubit16(arg[2])-ptr1;
		if(!errPos) {
			for(i = 0;i <= range;++i) {
				val1 = getbyte(seg1,ptr1+i);
				val2 = getbyte(seg2,ptr2+i);
				if(val1 != val2) {
					addrprint(seg1,ptr1+1);
					fprintf(stdout,"  ");
					printnubit8(val1);
					fprintf(stdout,"  ");
					printnubit8(val2);
					fprintf(stdout,"  ");
					addrprint(seg2,ptr2+1);
					fprintf(stdout,"\n");
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
	if(start > end) return;
	c[0x10] = '\0';
	if(end < start) end = 0xffff;
	for(i = start-(start%0x10);i <= end+(0x10-end%0x10)-1;++i) {
		if(i%0x10 == 0) addrprint(segment,i);
		if(i < start || i > end) {
			fprintf(stdout,"  ");
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
		fprintf(stdout," ");
		if(i%0x10 == 7 && i >= start && i < end) fprintf(stdout,"\b-");
		if((i+1)%0x10 == 0) {
			fprintf(stdout,"  %s\n",c);
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
		addrparse(cpu.ds,arg[1]);
		dprint(seg,ptr,ptr+0x7f);
	} else if(narg == 3) {
		addrparse(cpu.ds,arg[1]);
		ptr2 = scannubit16(arg[2]);
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
		addrparse(cpu.ds,arg[1]);
		addrprint(seg,ptr);
		printnubit8(getbyte(seg,ptr));
		fprintf(stdout,".");
		fgets(s,MAXLINE,stdin);
		lcase(s);//!!
		val = scannubit8(s);//!!
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			setbyte(seg,ptr,val);
	} else if(narg > 2) {
		addrparse(cpu.ds,arg[1]);
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
		addrparse(cpu.ds,arg[1]);
		end = scannubit16(arg[2]);
		if(end < ptr) seterr(2);
		if(!errPos) {
			nbyte = narg - 3;
			for(i = ptr,j = 0;i <= end;++i,++j) {
				val = scannubit8(arg[j%nbyte+3]);
				setbyte(seg,i,val);
			}
		}
	}
}
// hex
static void h()
{
	t_nubit16 val1,val2;
	if(narg != 3) seterr(narg-1);
	else {
		val1 = scannubit16(arg[1]);
		val2 = scannubit16(arg[2]);
		printnubit16(val1+val2);
		fprintf(stdout,"  ");
		printnubit16(val1-val2);
		fprintf(stdout,"\n");
	}
}
// load
static void l()
{
	unsigned char c;
	unsigned long len = 0;
	FILE *load = fopen(filename,"rb");
	if(!load) fprintf(stdout,"File not found\n");
	else {
		switch(narg) {
		case 1:
			seg = cpu.cs;
			ptr = 0x100;
			break;
		case 2:
			addrparse(cpu.cs,arg[1]);
			break;
		default:	seterr(narg-1);break;}
		c = fgetc(load);
		while(!feof(load)) {
			setbyte(seg,ptr+len++,c);
			c = fgetc(load);
		}
		fclose(load);
		cpu.cx = len&0xffff;
		if(len > 0xffff) cpu.bx = (len>>16);
		else cpu.bx = 0x0000;
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
		addrparse(cpu.ds,arg[1]);
		seg1 = seg;
		ptr1 = ptr;
		addrparse(cpu.ds,arg[3]);
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
	else strcpy(filename,arg[1]);
}
// register
static void uprint(t_nubit16,t_nubit16,t_nubit16);
static void rprintflags()
{
	t_nubit16 test;
	test = cpu.flags & OF;
	if(test == OF)
		fprintf(stdout,"OV ");
	else
		fprintf(stdout,"NV ");
	test = cpu.flags & DF;
	if(test == DF)
		fprintf(stdout,"DN ");
	else
		fprintf(stdout,"UP ");
	test = cpu.flags & IF;
	if(test == IF)
		fprintf(stdout,"EI ");
	else
		fprintf(stdout,"DI ");
	test = cpu.flags & SF;
	if(test == SF)
		fprintf(stdout,"NG ");
	else
		fprintf(stdout,"PL ");
	test = cpu.flags & ZF;
	if(test == ZF)
		fprintf(stdout,"ZR ");
	else
		fprintf(stdout,"NZ ");
	test = cpu.flags & AF;
	if(test == AF)
		fprintf(stdout,"AC ");
	else
		fprintf(stdout,"NA ");
	test = cpu.flags & PF;
	if(test == PF)
		fprintf(stdout,"PE ");
	else
		fprintf(stdout,"PO ");
	test = cpu.flags & CF;
	if(test == CF)
		fprintf(stdout,"CY ");
	else
		fprintf(stdout,"NC ");
}
static void rprintregs()
{
	fprintf(stdout,"AX=");
	printnubit16(cpu.ax);
	fprintf(stdout,"  BX=");
	printnubit16(cpu.bx);
	fprintf(stdout,"  CX=");
	printnubit16(cpu.cx);
	fprintf(stdout,"  DX=");
	printnubit16(cpu.dx);
	fprintf(stdout,"  SP=");
	printnubit16(cpu.sp);
	fprintf(stdout,"  BP=");
	printnubit16(cpu.bp);
	fprintf(stdout,"  SI=");
	printnubit16(cpu.si);
	fprintf(stdout,"  DI=");
	printnubit16(cpu.di);
	fprintf(stdout,"\nDS=");
	printnubit16(cpu.ds);
	fprintf(stdout,"  ES=");
	printnubit16(cpu.es);
	fprintf(stdout,"  SS=");
	printnubit16(cpu.ss);
	fprintf(stdout,"  CS=");
	printnubit16(cpu.cs);
	fprintf(stdout,"  IP=");
	printnubit16(cpu.ip);
	fprintf(stdout,"   ");
	rprintflags();
	fprintf(stdout,"\n");
	uprint(cpu.cs,cpu.ip,cpu.ip);
}
static void rscanregs()
{
	t_nubit16 t;
	char s[MAXLINE];
	if(!strcmp(arg[1],"ax")) {
		fprintf(stdout,"AX ");
		printnubit16(cpu.ax);
		fprintf(stdout,"\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			cpu.ax = t;
	} else if(!strcmp(arg[1],"bx")) {
		fprintf(stdout,"BX ");
		printnubit16(cpu.bx);
		fprintf(stdout,"\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			cpu.bx = t;
	} else if(!strcmp(arg[1],"cx")) {
		fprintf(stdout,"CX ");
		printnubit16(cpu.cx);
		fprintf(stdout,"\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			cpu.cx = t;
	} else if(!strcmp(arg[1],"dx")) {
		fprintf(stdout,"DX ");
		printnubit16(cpu.dx);
		fprintf(stdout,"\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			cpu.dx = t;
	} else if(!strcmp(arg[1],"bp")) {
		fprintf(stdout,"BP ");
		printnubit16(cpu.bp);
		fprintf(stdout,"\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			cpu.bp = t;
	} else if(!strcmp(arg[1],"sp")) {
		fprintf(stdout,"SP ");
		printnubit16(cpu.sp);
		fprintf(stdout,"\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			cpu.sp = t;
	} else if(!strcmp(arg[1],"si")) {
		fprintf(stdout,"SI ");
		printnubit16(cpu.si);
		fprintf(stdout,"\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			cpu.si = t;
	} else if(!strcmp(arg[1],"di")) {
		fprintf(stdout,"DI ");
		printnubit16(cpu.di);
		fprintf(stdout,"\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			cpu.di = t;
	} else if(!strcmp(arg[1],"ss")) {
		fprintf(stdout,"SS ");
		printnubit16(cpu.ss);
		fprintf(stdout,"\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			cpu.ss = t;
	} else if(!strcmp(arg[1],"cs")) {
		fprintf(stdout,"CS ");
		printnubit16(cpu.cs);
		fprintf(stdout,"\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			cpu.cs = t;
	} else if(!strcmp(arg[1],"ds")) {
		fprintf(stdout,"DS ");
		printnubit16(cpu.ds);
		fprintf(stdout,"\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			cpu.ds = t;
	} else if(!strcmp(arg[1],"es")) {
		fprintf(stdout,"ES ");
		printnubit16(cpu.es);
		fprintf(stdout,"\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			cpu.es = t;
	} else if(!strcmp(arg[1],"ip")) {
		fprintf(stdout,"IP ");
		printnubit16(cpu.ip);
		fprintf(stdout,"\n:");
		fgets(s,MAXLINE,stdin);
		t = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !errPos)
			cpu.ip = t;
	} else if(!strcmp(arg[1],"f")) {
		rprintflags();
		fprintf(stdout," -");
		fgets(s,MAXLINE,stdin);
		lcase(s);
		if(!strcmp(s,"ov")) cpu.flags |= OF;
		else if(!strcmp(s,"nv")) cpu.flags &= ~OF;
		else if(!strcmp(s,"dn")) cpu.flags |= DF;
		else if(!strcmp(s,"up")) cpu.flags &= ~DF;
		else if(!strcmp(s,"ei")) cpu.flags |= IF;
		else if(!strcmp(s,"di")) cpu.flags &= ~IF;
		else if(!strcmp(s,"ng")) cpu.flags |= SF;
		else if(!strcmp(s,"pl")) cpu.flags &= ~SF;
		else if(!strcmp(s,"zr")) cpu.flags |= ZF;
		else if(!strcmp(s,"nz")) cpu.flags &= ~ZF;
		else if(!strcmp(s,"ac")) cpu.flags |= AF;
		else if(!strcmp(s,"na")) cpu.flags &= ~AF;
		else if(!strcmp(s,"pe")) cpu.flags |= PF;
		else if(!strcmp(s,"po")) cpu.flags &= ~PF;
		else if(!strcmp(s,"cy")) cpu.flags |= CF;
		else if(!strcmp(s,"nc")) cpu.flags &= ~CF;
		else fprintf(stdout,"bf Error\n");
	} else fprintf(stdout,"br Error\n");
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
		addrparse(cpu.ds,arg[1]);
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
						fprintf(stdout,"\n");
					}
				} else ++p;
			}
		}
	}
}
// quit
static void q()
{exitFlag = 1;}
// unassemble
static t_nubit16 uoffset(Operand opr)
{
	t_nubit16 res = 0;
	switch(opr.mod) {
	case 0:
		switch(opr.rm) {
		case 0:	res = cpu.bx+cpu.si;break;
		case 1:	res = cpu.bx+cpu.di;break;
		case 2:	res = cpu.bp+cpu.si;break;
		case 3:	res = cpu.bp+cpu.di;break;
		case 4:	res = cpu.si;break;
		case 5:	res = cpu.di;break;
		case 6:	res = opr.imm;break;
		case 7:	res = cpu.bx;break;
		default:fprintf(stdout,"(ERROR:OFFSET)");break;}
		break;
	case 1:
	case 2:
		switch(opr.rm) {
		case 0:	res = cpu.bx+cpu.si+opr.imm;break;
		case 1:	res = cpu.bx+cpu.di+opr.imm;break;
		case 2:	res = cpu.bp+cpu.si+opr.imm;break;
		case 3:	res = cpu.bp+cpu.di+opr.imm;break;
		case 4:	res = cpu.si+opr.imm;break;
		case 5:	res = cpu.di+opr.imm;break;
		case 6:	res = cpu.bp+opr.imm;break;
		case 7:	res = cpu.bx+opr.imm;break;
		default:fprintf(stdout,"(ERROR:OFFSET)");break;}
		break;
	default:fprintf(stdout,"(ERROR:OFFSET)");break;}
	return res;
}
static void uprint(t_nubit16 segment,t_nubit16 start,t_nubit16 end)
{
	char str[MAXLINE],*op = NULL, *stmt = NULL;
	int pos = 0,len = 0;
	int i,prefixflag;
	t_nubit16 offset;
	Operand operand;
	operand.flag = 4;
	operand.seg = 0x00;
	if(start > end) return;
	while(start <= end) {
		pos = 0;
		prefixflag = 0;
		while(!pos || isprefix(getbyte(segment,start+pos)) ||
			(prefixflag && !isprefix(getbyte(segment,start+pos)))) {
			if(isprefix(getbyte(segment,start+pos))) prefixflag = 1;
			else prefixflag = 0;
			printnubit16(segment);
			fprintf(stdout,":");
			printnubit16(start+pos);
			fprintf(stdout," ");
			len = disassemble(str,&operand,
				memory,segment,start+pos);
			for(i = 0;i < len;++i)
				printnubit8(getbyte(segment,start+pos+i));
			pos += len;
			if(len < 3) fprintf(stdout,"\t\t");
			else fprintf(stdout,"\t");
			op = strtok(str,"\t");
			if(!len || !op) {fprintf(stdout,"fail to unassemble\n");return;}
			else fprintf(stdout,"%s\t",op);
			stmt = strtok(NULL,"\0");
			if(stmt) {
				fprintf(stdout,"%s",stmt);
				if(strlen(stmt) < 8) fprintf(stdout,"\t\t\t\t");
				else if(strlen(stmt) < 16) fprintf(stdout,"\t\t\t");
				else if(strlen(stmt) < 24) fprintf(stdout,"\t\t");
				else if(strlen(stmt) < 30) fprintf(stdout,"\t");
			}
			//fprintf(stdout,"opr.seg=%d,opr.flag=%d\n",operand.seg,operand.flag);
			if(operand.seg == 1 && operand.flag != 4) {
				switch(operand.flag) {
				case 0:	fprintf(stdout,"ES:");offset = uoffset(operand);
					printnubit16(offset);fprintf(stdout,"=");
					switch(operand.len) {
					case 1:printnubit8(getbyte(cpu.es,offset));break;
					case 2:printnubit16(getword(cpu.es,offset));break;
					default:fprintf(stdout,"(ERROR:OPERANDES)");break;}
					break;
				case 1:	fprintf(stdout,"CS:");offset = uoffset(operand);
					printnubit16(offset);fprintf(stdout,"=");
					switch(operand.len) {
					case 1:printnubit8(getbyte(cpu.cs,offset));break;
					case 2:printnubit16(getword(cpu.cs,offset));break;
					default:fprintf(stdout,"(ERROR:OPERANDCS)");break;}
					break;
				case 2:	fprintf(stdout,"SS:");offset = uoffset(operand);
					printnubit16(offset);fprintf(stdout,"=");
					switch(operand.len) {
					case 1:printnubit8(getbyte(cpu.ss,offset));break;
					case 2:printnubit16(getword(cpu.ss,offset));break;
					default:fprintf(stdout,"(ERROR:OPERANDSS)");break;}
					break;
				case 3:	fprintf(stdout,"DS:");offset = uoffset(operand);
					printnubit16(offset);fprintf(stdout,"=");
					switch(operand.len) {
					case 1:printnubit8(getbyte(cpu.ds,offset));break;
					case 2:printnubit16(getword(cpu.ds,offset));break;
					default:fprintf(stdout,"(ERROR:OPERANDDS)");break;}
					break;
				default:fprintf(stdout,"(ERROR:OPERAND)");break;}
				operand.flag = 4;
			}
			fprintf(stdout,"\n");
		}
		start += pos;
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
		addrparse(cpu.cs,arg[1]);
		uprint(seg,ptr,ptr+0x1f);
	} else if(narg == 3) {
		addrparse(cpu.ds,arg[1]);
		ptr2 = scannubit16(arg[2]);
		if(ptr > ptr2) seterr(2);
		else uprint(seg,ptr,ptr2);
	} else seterr(3);
}
// write
static void w()
{
	t_nubit16 i = 0;
	t_nubit32 len = (cpu.bx<<16)+cpu.cx;
	FILE *write;
	if(!strlen(filename)) {fprintf(stdout,"(W)rite error, no destination defined\n");return;}
	else write= fopen(filename,"wb");
	if(!write) fprintf(stdout,"File not found\n");
	else {
		fprintf(stdout,"Writing ");
		printnubit16(cpu.bx);
		printnubit16(cpu.cx);
		fprintf(stdout," bytes\n");
		switch(narg) {
		case 1:
			seg = cpu.cs;
			ptr = 0x100;
			break;
		case 2:
			addrparse(cpu.cs,arg[1]);
			break;
		default:	seterr(narg-1);break;}
		while(i < len)
			fputc(getbyte(seg,ptr+i++),write);
		fclose(write);
	}
}
/* DEBUG CMD END */

static void help()
{
	fprintf(stdout,"assemble\tA [address]\n");
	fprintf(stdout,"compare\t\tC range address\n");
	fprintf(stdout,"dump\t\tD [range]\n");
	fprintf(stdout,"enter\t\tE address [list]\n");
	fprintf(stdout,"fill\t\tF range list\n");
//!	fprintf(stdout,"go\t\tG [=address] [addresses]\n");
	fprintf(stdout,"hex\t\tH value1 value2\n");
//!	fprintf(stdout,"input\t\tI port\n");
	fprintf(stdout,"load\t\tL [address]\n");
	//fprintf(stdout,"load\t\tL [address] [drive] [firstsector] [number]\n");
	fprintf(stdout,"move\t\tM range address\n");
	fprintf(stdout,"name\t\tN [pathname]\n");
	//fprintf(stdout,"name\t\tN [pathname] [arglist]\n");
//!	fprintf(stdout,"output\t\tO port byte\n");
//!	fprintf(stdout,"proceed\t\tP [=address] [number]\n");
	fprintf(stdout,"quit\t\tQ\n");
	fprintf(stdout,"register\tR [register]\n");
	fprintf(stdout,"search\t\tS range list\n");
//!	fprintf(stdout,"trace\t\tT [=address] [value]\n");
	fprintf(stdout,"unassemble\tU [range]\n");
	fprintf(stdout,"write\t\tW [address]\n");
	//fprintf(stdout,"write\t\tW [address] [drive] [firstsector] [number]\n");
	//fprintf(stdout,"allocate expanded memory\tXA [#pages]\n");
	//fprintf(stdout,"deallocate expanded memory\tXD [handle]\n");
	//fprintf(stdout,"map expanded memory pages\tXM [Lpage] [Ppage] [handle]\n");
	//fprintf(stdout,"display expanded memory status\tXS\n");
}
static void init()
{
	filename[0] = '\0';
	cpu.ax = cpu.bx = cpu.cx = cpu.dx = 0x0000;
	cpu.si = cpu.di = cpu.sp = cpu.bp = 0x0000;
	cpu.ds = cpu.es = cpu.ss = cpu.cs =
		asmSegRec = dumpSegRec = uasmSegRec = 0x0a50;
	cpu.ip = asmPtrRec = dumpPtrRec = uasmPtrRec = 0x0100;
}
static void parse()
{
	strcpy(cmdCopy,cmdBuff);
	narg = 0;
	arg[narg] = strtok(cmdCopy," ,\t\n\r\f");
	if(arg[narg]) {
		lcase(arg[narg]);
		narg++;
	} else return;
	if(strlen(arg[narg-1]) != 1) {
		arg[narg] = arg[narg-1]+1;
		narg++;
	}
	while(narg < MAXNARG) {
		arg[narg] = strtok(NULL," ,\t\n\r\f");
		if(arg[narg]) {
			lcase(arg[narg]);
			narg++;
		}
		else break;
	}
}
static void exec()
{
	errPos = 0;
	if(!arg[0]) return;
	/*if(arg[0][1] != '\0') {
		seterr(0);
		return;
	}*/
	switch(arg[0][0]) {
	case '\?':	help();break;
	case 'a':	a();break;
	case 'c':	c();break;
	case 'd':	d();break;
	case 'e':	e();break;
	case 'f':	f();break;
	case 'h':	h();break;
	case 'l':	l();break;
	case 'm':	m();break;
	case 'n':	n();break;
	case 'q':	q();break;
	case 'r':	r();break;
	case 's':	s();break;
	case 'u':	u();break;
	case 'w':	w();break;
	default:
		seterr(0);
		break;
	}
}

void NSDebug()
{
	int i;
	init();
	arg = (char **)malloc(MAXNARG * sizeof(char *));
	exitFlag = 0;
	while(!exitFlag) {
		fflush(stdin);
		fprintf(stdout,"-");
		fgets(cmdBuff,MAXLINE,stdin);
		parse();
		exec();
		if(errPos) {
			for(i = 0;i < errPos;++i) fprintf(stdout," ");
			fprintf(stdout,"^ Error\n");
		}
	}
	free(arg);
}