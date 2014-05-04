/* This file is a part of NXVM project. */

/* DEBUG is the debug console, which is highly coupled with device components. */

#include "utils.h"

#include "device/vport.h"
#include "device/vcpu.h"
#include "device/vcpuins.h"
#include "device/vram.h"
#include "device/vdebug.h"

#include "device/device.h"
#include "machine.h"

#include "debug.h"

#define DEBUG_MAXNARG 256
#define DEBUG_MAXNASMARG 4

static t_nubitcc nErrPos;
static t_nubitcc narg;
static char **arg;
static t_bool flagexit;
static t_string strCmdBuff, strCmdCopy, strFileName;

static void seterr(t_nubitcc pos)
{
	nErrPos = (t_nubitcc)(arg[pos] - strCmdCopy + strlen(arg[pos]) + 1);
}
static t_nubit8 scannubit8(const t_strptr s)
{
	t_nubit8 ans = 0;
	int i = 0;
	if(s[0] == '\'' && s[2] == '\'') return s[1];
	utilsLowerStr(s);
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
static t_nubit16 scannubit16(const t_strptr s)
{
	t_nubit16 ans = 0;
	int i = 0;
	utilsLowerStr(s);
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
static t_nubit32 scannubit32(const t_strptr s)
{
	t_nubit32 ans = 0;
	int i = 0;
	utilsLowerStr(s);
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

static t_nubit16 dumpSegRec;
static t_nubit16 dumpPtrRec;
static t_nubit16 asmSegRec;
static t_nubit16 asmPtrRec;
static t_nubit16 uasmSegRec;
static t_nubit16 uasmPtrRec;

static t_nubit16 seg;
static t_nubit16 ptr;
static void addrparse(t_nubit16 defseg, const t_strptr addr)
{
	t_strptr cseg,cptr;
	char ccopy[MAXLINE];
	STRCPY(ccopy,addr);
	cseg = STRTOK(ccopy,":");
	cptr = STRTOK(NULL,"");
	if(!cptr) {
		seg = defseg;
		ptr = scannubit16(cseg);
	} else {
		if(!STRCMP(cseg,"es")) seg = vcpu.es.selector;
		else if(!STRCMP(cseg,"cs")) seg = vcpu.cs.selector;
		else if(!STRCMP(cseg,"ss")) seg = vcpu.ss.selector;
		else if(!STRCMP(cseg,"ds")) seg = vcpu.ds.selector;
		else seg = scannubit16(cseg);
		ptr = scannubit16(cptr);
	}
}

/* DEBUG CMD BEGIN */
/* assemble */
static void aconsole()
{
	t_nubitcc i;
	t_nubit8 len;
	char cmdAsmBuff[MAXLINE];
	t_nubit8 acode[15];
	t_nubitcc errAsmPos;
	t_bool flagexitasm = 0;
	while(!flagexitasm) {
		PRINTF("%04X:%04X ", asmSegRec, asmPtrRec);
		fflush(stdin);
		FGETS(cmdAsmBuff,MAXLINE,stdin);
		utilsLowerStr(cmdAsmBuff);
		if(!strlen(cmdAsmBuff)) {
			flagexitasm = 1;
			continue;
		}
		if(cmdAsmBuff[0] == ';' ) continue;
		errAsmPos = 0;
		len = utilsAasm32(cmdAsmBuff, acode, vcpu.cs.seg.exec.defsize);
		if(!len) errAsmPos = (t_nubitcc)strlen(cmdAsmBuff) + 9;
		else {
			if (vcpuinsWriteLinear((asmSegRec << 4) + asmPtrRec, (t_vaddrcc)acode, len)) {
				PRINTF("debug: fail to write to L%08X\n", (asmSegRec << 4) + asmPtrRec);
				return;
			}
			asmPtrRec += len;
		}
		if(errAsmPos) {
			for(i = 0;i < errAsmPos;++i) PRINTF(" ");
			PRINTF("^ Error\n");
		}
	}
}
static void a()
{
	if(narg == 1) {
		aconsole();
	} else if(narg == 2) {
		addrparse(vcpu.cs.selector, arg[1]);
		if(nErrPos) return;
		asmSegRec = seg;
		asmPtrRec = ptr;
		aconsole();
	} else seterr(2);
}
/* compare */
static void c()
{
	t_nubit8 val1,val2;
	t_nubit16 i,seg1,ptr1,range,seg2,ptr2;
	if(narg != 4) seterr(narg-1);
	else {
		addrparse(vcpu.ds.selector,arg[1]);
		seg1 = seg;
		ptr1 = ptr;
		addrparse(vcpu.ds.selector,arg[3]);
		seg2 = seg;
		ptr2 = ptr;
		range = scannubit16(arg[2])-ptr1;
		if(!nErrPos) {
			for(i = 0;i <= range;++i) {
				val1 = vramRealByte(seg1,ptr1+i);
				val2 = vramRealByte(seg2,ptr2+i);
				if(val1 != val2) {
					PRINTF("%04X:%04X  ", seg1, ptr1 + i);
					PRINTF("%02X  %02X", val1, val2);
					PRINTF("  %04X:%04X\n", seg2, ptr2 + i);
				}
			}
		}
	}
}
/* dump */
static void dprint(t_nubit16 segment,t_nubit16 start,t_nubit16 end)
{
	char t,c[0x11];
	t_nubit16 i;
	if(start > end) end = 0xffff;
	if((t_nubit32)((segment<<4)+end) > 0xfffff) end = (0xfffff - (segment << 4));
	c[0x10] = '\0';
	if(end < start) end = 0xffff;
	for(i = start-(start%0x10);i <= end+(0x10-end%0x10)-1;++i) {
		if(i%0x10 == 0) PRINTF("%04X:%04X  ", segment, i);
		if(i < start || i > end) {
			PRINTF("  ");
			c[i%0x10] = ' ';
		} else {
			c[i%0x10] = vramRealByte(segment,i);
			PRINTF("%02X",c[i%0x10] & 0xff);
			t = c[i%0x10];
			if((t >=1 && t <= 7) || t == ' ' ||
				(t >=11 && t <= 12) ||
				(t >=14 && t <= 31) ||
				(t >=33)) ;
			else c[i%0x10] = '.';
		}
		PRINTF(" ");
		if(i%0x10 == 7 && i >= start && i < end) PRINTF("\b-");
		if((i+1)%0x10 == 0) {
			PRINTF("  %s\n",c);
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
		addrparse(vcpu.ds.selector,arg[1]);
		if(nErrPos) return;
		dprint(seg,ptr,ptr+0x7f);
	} else if(narg == 3) {
		addrparse(vcpu.ds.selector,arg[1]);
		ptr2 = scannubit16(arg[2]);
		if(nErrPos) return;
		if(ptr > ptr2) seterr(2);
		else dprint(seg,ptr,ptr2);
	} else seterr(3);
}
/* enter */
static void e()
{
	t_nubit8 i, val;
	char s[MAXLINE];
	if(narg == 1) seterr(0);
	else if(narg == 2) {
		addrparse(vcpu.ds.selector,arg[1]);
		if(nErrPos) return;
		PRINTF("%04X:%04X  ", seg, ptr);
		PRINTF("%02X",vramRealByte(seg,ptr));
		PRINTF(".");
		FGETS(s,MAXLINE,stdin);
		utilsLowerStr(s); /* MARK */
		val = scannubit8(s); /* MARK */
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			vramRealByte(seg,ptr) = val;
	} else if(narg > 2) {
		addrparse(vcpu.ds.selector,arg[1]);
		if(nErrPos) return;
		for(i = 2;i < narg;++i) {
			val = scannubit8(arg[i]); /* MARK */
			if(!nErrPos) vramRealByte(seg,ptr) = val;
			else break;
			ptr++;
		}
	}
}
/* fill */
static void f()
{
	t_nubit8 nbyte;
	t_nubit8 val;
	t_nubit16 i,j,end;
	if(narg < 4) seterr(narg-1);
	else {
		addrparse(vcpu.ds.selector,arg[1]);
		end = scannubit16(arg[2]);
		if(end < ptr) seterr(2);
		if(!nErrPos) {
			nbyte = (t_nubit8) narg - 3;
			for(i = ptr,j = 0;i <= end;++i,++j) {
				val = scannubit8(arg[j%nbyte+3]);
				if(!nErrPos) vramRealByte(seg,i) = val;
				else return;
			}
		}
	}
}
/* go */
static void rprintregs();
static void g()
{
	if (device.flagRun) {
		PRINTF("NXVM is already running.\n");
		return;
	}
	switch(narg) {
	case 1:
		vdebug.flagBreak = 0;
		break;
	case 2:
		vdebug.flagBreak = 1;
		addrparse(vcpu.cs.selector,arg[1]);
		vdebug.breakCS = seg;
		vdebug.breakIP = ptr;
		break;
	case 3:
		vdebug.flagBreak = 1;
		addrparse(vcpu.cs.selector,arg[1]);
		if (vcpuinsLoadSreg(&vcpu.cs, seg)) {
			PRINTF("debug: fail to load cs from %04X\n", seg);
			return;
		}
		_ip = ptr;
		addrparse(vcpu.cs.selector,arg[2]);
		vdebug.breakCS = seg;
		vdebug.breakIP = ptr;
		break;
	default:seterr(narg-1);break;}
	if(nErrPos) return;
	machineResume();
	while (device.flagRun) utilsSleep(1);
	vdebug.flagBreak = 0;
	rprintregs();
}
/* hex */
static void h()
{
	t_nubit16 val1,val2;
	if(narg != 3) seterr(narg-1);
	else {
		val1 = scannubit16(arg[1]);
		val2 = scannubit16(arg[2]);
		if(!nErrPos) {
			PRINTF("%04X",GetMax16(val1+val2));
			PRINTF("  ");
			PRINTF("%04X",GetMax16(val1-val2));
			PRINTF("\n");
		}
	}
}
/* input */
static void i()
{
	t_nubit16 in;
	if(narg != 2) seterr(narg-1);
	else {
		in = scannubit16(arg[1]);
		if (nErrPos) return;
		ExecFun(vport.in[in]);
		PRINTF("%08X\n", vport.ioDWord);
	}
}
/* load */
static void l()
{
	t_nubit8 c;
	t_nubit16 i = 0;
	t_nubit32 len = 0;
	FILE *load = FOPEN(strFileName,"rb");
	if(!load) PRINTF("File not found\n");
	else {
		switch(narg) {
		case 1:
			seg = vcpu.cs.selector;
			ptr = 0x100;
			break;
		case 2:
			addrparse(vcpu.cs.selector,arg[1]);
			break;
		default:seterr(narg-1);break;}
		if(!nErrPos) {
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
		FCLOSE(load);
	}
}
/* move */
static void m()
{
	int i;
	t_nubit16 seg1,ptr1,range,seg2,ptr2;
	if(narg != 4) seterr(narg-1);
	else {
		addrparse(vcpu.ds.selector,arg[1]);
		seg1 = seg;
		ptr1 = ptr;
		addrparse(vcpu.ds.selector,arg[3]);
		seg2 = seg;
		ptr2 = ptr;
		range = scannubit16(arg[2])-ptr1;
		if(!nErrPos) {
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
/* name */
static void n()
{
	if(narg != 2) seterr(narg-1);
	else STRCPY(strFileName,arg[1]);
}
/* output */
static void o()
{
	t_nubit16 out;
	if(narg != 3) seterr(narg-1);
	else {
		out = scannubit16(arg[1]);
		if (nErrPos) return;
		vport.ioDWord = scannubit32(arg[2]);
		if (nErrPos) return;
		ExecFun(vport.out[out]);
	}
}
/* quit */
static void q()
{flagexit = 1;}
/* register */
static t_nubit8 uprintins(t_nubit16 seg, t_nubit16 off)
{
	t_nubitcc i;
	t_nubit8 len;
	t_nubit8  ucode[15];
	t_string str, stmt, sbin;
	if (vcpuinsReadLinear((seg << 4) + off, (t_vaddrcc)ucode, 15)) {
		len = 0;
		SPRINTF(str, "%04X:%04X <ERROR>", seg, off);
	} else {
		len = utilsDasm32(stmt, ucode, vcpu.cs.seg.exec.defsize);
		sbin[0] = 0;
		for (i = 0;i < len;++i) SPRINTF(sbin, "%s%02X", sbin, GetMax8(ucode[i]));
		SPRINTF(str, "%04X:%04X %s", seg, off, sbin);
		for (i = strlen(str);i < 24;++i) STRCAT(str, " ");
		STRCAT(str, stmt);
	}
	PRINTF("%s\n", str);
	return len;
}
static void rprintflags()
{
	PRINTF("%s ", _GetEFLAGS_OF ? "OV" : "NV");
	PRINTF("%s ", _GetEFLAGS_DF ? "DN" : "UP");
	PRINTF("%s ", _GetEFLAGS_IF ? "EI" : "DI");
	PRINTF("%s ", _GetEFLAGS_SF ? "NG" : "PL");
	PRINTF("%s ", _GetEFLAGS_ZF ? "ZR" : "NZ");
	PRINTF("%s ", _GetEFLAGS_AF ? "AC" : "NA");
	PRINTF("%s ", _GetEFLAGS_PF ? "PE" : "PO");
	PRINTF("%s ", _GetEFLAGS_CF ? "CY" : "NC");
}
static void rprintregs()
{
	PRINTF(  "AX=%04X", _ax);
	PRINTF("  BX=%04X", _bx);
	PRINTF("  CX=%04X", _cx);
	PRINTF("  DX=%04X", _dx);
	PRINTF("  SP=%04X", _sp);
	PRINTF("  BP=%04X", _bp);
	PRINTF("  SI=%04X", _si);
	PRINTF("  DI=%04X", _di);
	PRINTF("\nDS=%04X", vcpu.ds.selector);
	PRINTF("  ES=%04X", vcpu.es.selector);
	PRINTF("  SS=%04X", vcpu.ss.selector);
	PRINTF("  CS=%04X", vcpu.cs.selector);
	PRINTF("  IP=%04X", _ip);
	PRINTF("   ");
	rprintflags();
	PRINTF("\n");
	uprintins(vcpu.cs.selector, _ip);
	uasmSegRec = vcpu.cs.selector;
	uasmPtrRec = _ip;
}
static void rscanregs()
{
	t_nubit16 value;
	char s[MAXLINE];
	if(!STRCMP(arg[1],"ax")) {
		PRINTF("AX ");
		PRINTF("%04X",_ax);
		PRINTF("\n:");
		FGETS(s,MAXLINE,stdin);
		value = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			_ax = value;
	} else if(!STRCMP(arg[1],"bx")) {
		PRINTF("BX ");
		PRINTF("%04X",_bx);
		PRINTF("\n:");
		FGETS(s,MAXLINE,stdin);
		value = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			_bx = value;
	} else if(!STRCMP(arg[1],"cx")) {
		PRINTF("CX ");
		PRINTF("%04X",_cx);
		PRINTF("\n:");
		FGETS(s,MAXLINE,stdin);
		value = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			_cx = value;
	} else if(!STRCMP(arg[1],"dx")) {
		PRINTF("DX ");
		PRINTF("%04X",_dx);
		PRINTF("\n:");
		FGETS(s,MAXLINE,stdin);
		value = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			_dx = value;
	} else if(!STRCMP(arg[1],"bp")) {
		PRINTF("BP ");
		PRINTF("%04X",_bp);
		PRINTF("\n:");
		FGETS(s,MAXLINE,stdin);
		value = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			_bp = value;
	} else if(!STRCMP(arg[1],"sp")) {
		PRINTF("SP ");
		PRINTF("%04X",_sp);
		PRINTF("\n:");
		FGETS(s,MAXLINE,stdin);
		value = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			_sp = value;
	} else if(!STRCMP(arg[1],"si")) {
		PRINTF("SI ");
		PRINTF("%04X",_si);
		PRINTF("\n:");
		FGETS(s,MAXLINE,stdin);
		value = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			_si = value;
	} else if(!STRCMP(arg[1],"di")) {
		PRINTF("DI ");
		PRINTF("%04X",_di);
		PRINTF("\n:");
		FGETS(s,MAXLINE,stdin);
		value = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			_di = value;
	} else if(!STRCMP(arg[1],"ss")) {
		PRINTF("SS ");
		PRINTF("%04X",vcpu.ss.selector);
		PRINTF("\n:");
		FGETS(s,MAXLINE,stdin);
		value = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			if (vcpuinsLoadSreg(&vcpu.ss, GetMax16(value)))
				PRINTF("debug: fail to load ss from %04X\n", GetMax16(value));
	} else if(!STRCMP(arg[1],"cs")) {
		PRINTF("CS ");
		PRINTF("%04X",vcpu.cs.selector);
		PRINTF("\n:");
		FGETS(s,MAXLINE,stdin);
		value = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			if (vcpuinsLoadSreg(&vcpu.cs, GetMax16(value)))
				PRINTF("debug: fail to load cs from %04X\n", GetMax16(value));
	} else if(!STRCMP(arg[1],"ds")) {
		PRINTF("DS ");
		PRINTF("%04X",vcpu.ds.selector);
		PRINTF("\n:");
		FGETS(s,MAXLINE,stdin);
		value = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			if (vcpuinsLoadSreg(&vcpu.ds, GetMax16(value)))
				PRINTF("debug: fail to load ds from %04X\n", GetMax16(value));
	} else if(!STRCMP(arg[1],"es")) {
		PRINTF("ES ");
		PRINTF("%04X",vcpu.es.selector);
		PRINTF("\n:");
		FGETS(s,MAXLINE,stdin);
		value = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			if (vcpuinsLoadSreg(&vcpu.es, GetMax16(value)))
				PRINTF("debug: fail to load es from %04X\n", GetMax16(value));
	} else if(!STRCMP(arg[1],"ip")) {
		PRINTF("IP ");
		PRINTF("%04X",_ip);
		PRINTF("\n:");
		FGETS(s,MAXLINE,stdin);
		value = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			_ip = value;
	} else if(!STRCMP(arg[1],"f")) {
		rprintflags();
		PRINTF(" -");
		FGETS(s,MAXLINE,stdin);
		utilsLowerStr(s);
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
		else PRINTF("bf Error\n");
	} else PRINTF("br Error\n");
}
static void r()
{
	if(narg == 1) {
		rprintregs();
	} else if(narg == 2) {
		rscanregs();
	} else seterr(2);
}
/* search */
static void s()
{
	t_nubit8 i;
	t_bool flagfound = 0;
	t_nubit16 p,pfront,start,end;
	t_nubit8 cstart;
	if(narg < 4) seterr(narg-1);
	else {
		addrparse(vcpu.ds.selector,arg[1]);
		start = ptr;
		end = scannubit16(arg[2]);
		if(!nErrPos) {
			p = start;
			cstart = scannubit8(arg[3]);
			while(p <= end) {
				if(vramRealByte(seg,p) == cstart) {
					pfront = p;
					flagfound = 1;
					for(i = 3;i < narg;++i) {
						if(vramRealByte(seg,p) != scannubit8(arg[i])) {
							flagfound = 0;
							p = pfront+1;
							break;
						} else ++p;
					}
					if(flagfound) {
						PRINTF("%04X:%04X  ", seg, pfront);
						PRINTF("\n");
					}
				} else ++p;
			}
		}
	}
}
/* trace */
static void t()
{
	t_nubit16 i;
	t_nubit16 count;
	if (device.flagRun) {
		PRINTF("NXVM is already running.\n");
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
		addrparse(vcpu.cs.selector,arg[1]);
		if (vcpuinsLoadSreg(&vcpu.cs, seg)) {
			PRINTF("debug: fail to load cs from %04X\n", seg);
			return;
		}
		_ip = ptr;
		count = scannubit16(arg[2]);
		break;
	default:seterr(narg-1);break;}
	if(nErrPos) return;
	if (count < 0x0100) {
		for(i = 0;i < count;++i) {
			vdebug.traceCount = 0x01;
			machineResume();
			while (device.flagRun) utilsSleep(10);
			rprintregs();
			if (i != count - 1) PRINTF("\n");
		}
	} else {
		vdebug.traceCount = count;
		machineResume();
		while (device.flagRun) utilsSleep(10);
		rprintregs();
	}
	vdebug.traceCount = 0x00;
	/* gexec(ptr1,ptr2); */
}
/* unassemble */
static void uprint(t_nubit16 segment,t_nubit16 start,t_nubit16 end)
{
	t_nubit8 len;
	t_nubit32 boundary;
	if(start > end) end = 0xffff;
	if ((t_nubit32)((segment<<4) + end) > 0xfffff) end = (0xfffff-(segment<<4));
	while(start <= end) {
		len = uprintins(segment, start);
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
		addrparse(vcpu.cs.selector,arg[1]);
		if(nErrPos) return;
		uprint(seg,ptr,ptr+0x1f);
	} else if(narg == 3) {
		addrparse(vcpu.ds.selector,arg[1]);
		ptr2 = scannubit16(arg[2]);
		if(nErrPos) return;
		if(ptr > ptr2) seterr(2);
		else uprint(seg,ptr,ptr2);
	} else seterr(3);
}
/* verbal */
static void v()
{
	t_nubitcc i;
	char str[MAXLINE];
	PRINTF(":");
	FGETS(str,MAXLINE,stdin);
	str[strlen(str)-1] = '\0';
	for(i = 0;i < strlen(str);++i) {
		PRINTF("%02X",str[i]);
		if(!((i+1)%0x10)) PRINTF("\n"); 
		else if(!((i+1)%0x08)&&(str[i+1]!='\0')) PRINTF("-");
		else PRINTF(" ");
	}
	if(i%0x10) PRINTF("\n");
}
/* write */
static void w()
{
	t_nubit16 i = 0;
	t_nubit32 len = (_bx<<16)+_cx;
	FILE *write;
	if(!strlen(strFileName)) {PRINTF("(W)rite error, no destination defined\n");return;}
	else write= FOPEN(strFileName,"wb");
	if(!write) PRINTF("File not found\n");
	else {
		PRINTF("Writing ");
		PRINTF("%04X",_bx);
		PRINTF("%04X",_cx);
		PRINTF(" bytes\n");
		switch(narg) {
		case 1:
			seg = vcpu.cs.selector;
			ptr = 0x100;
			break;
		case 2:
			addrparse(vcpu.cs.selector,arg[1]);
			break;
		default:	seterr(narg-1);break;}
		if(!nErrPos)
			while(i < len)
				fputc(vramRealByte(seg,ptr+i++),write);
		FCLOSE(write);
	}
}
/* DEBUG CMD END */

/* EXTENDED DEBUG CMD BEGIN */
t_nubit32 xalin;
t_nubit32 xdlin;
t_nubit32 xulin;
/* print */
static t_nubit8 xuprintins(t_nubit32 linear)
{
	t_nubitcc i;
	t_nubit8 len;
	t_nubit8  ucode[15];
	t_string str, stmt, sbin;
	if (vcpuinsReadLinear(linear, (t_vaddrcc)ucode, 15)) {
		len = 0;
		SPRINTF(str, "L%08X <ERROR>", linear);
	} else {
		len = utilsDasm32(stmt, ucode, vcpu.cs.seg.exec.defsize);
		sbin[0] = 0;
		for (i = 0;i < len;++i) SPRINTF(sbin, "%s%02X", sbin, GetMax8(ucode[i]));
		SPRINTF(str, "L%08X %s ", linear, sbin);
		for (i = strlen(str);i < 24;++i) STRCAT(str, " ");
		STRCAT(str, stmt);
	}
	PRINTF("%s\n", str);
	return len;
}
static void xrprintreg()
{
	devicePrintCpuReg();
	xulin = vcpu.cs.base + _eip;
	xuprintins(xulin);
}
/* assemble */
static void xaconsole(t_nubit32 linear)
{
	t_nubitcc i;
	t_nubit8 len;
	t_string astmt;
	t_nubit8 acode[15];
	t_nubitcc errAsmPos;
	t_bool flagexitasm = 0;
	while(!flagexitasm) {
		PRINTF("L%08X ", linear);
		FGETS(astmt, MAXLINE, stdin);
		fflush(stdin);
		astmt[strlen(astmt) - 1] = 0;
		if(!strlen(astmt)) {
			flagexitasm = 1;
			continue;
		}
		errAsmPos = 0;
		len = utilsAasm32(astmt, acode, vcpu.cs.seg.exec.defsize);
		if(!len) errAsmPos = (t_nubitcc)strlen(astmt) + 9;
		else {
			if (vcpuinsWriteLinear(linear, (t_vaddrcc)acode, len)) {
				PRINTF("debug: fail to write to L%08X\n", linear);
				return;
			}
			linear += len;
		}
		if(errAsmPos) {
			for(i = 0;i < errAsmPos;++i) PRINTF(" ");
			PRINTF("^ Error\n");
		}
	}
	xalin = linear;
}
static void xa()
{
	if(narg == 1) xaconsole(xalin);
	else if(narg == 2) {
		xalin = scannubit32(arg[1]);
		if(nErrPos) return;
		xaconsole(xalin);
	} else seterr(2);
}
/* compare */
static void xc()
{
	t_nubit32 i;
	t_nubit32 count;
	t_nubit32 lin1, lin2;
	t_nubit8 val1, val2;
	if(narg != 4) seterr(narg-1);
	else {
		lin1 = scannubit32(arg[1]);
		if(nErrPos) return;
		lin2 = scannubit32(arg[2]);
		if(nErrPos) return;
		count = scannubit32(arg[3]);
		if(nErrPos) return;
		if (!count) return;
		for (i = 0;i < count;++i) {
			if (vcpuinsReadLinear(lin1 + i, GetRef(val1), 1)) {
				PRINTF("debug: fail to read from L%08X.\n", lin1 + i);
				return;
			}
			if (vcpuinsReadLinear(lin2 + i, GetRef(val2), 1)) {
				PRINTF("debug: fail to read from L%08X.\n", lin2 + i);
				return;
			}
			if(val1 != val2)
				PRINTF("L%08X  %02X  %02X  L%08X\n",
					lin1 + i, val1, val2, lin2 + i);
		}
	}
}
/* dump */
static void xdprint(t_nubit32 linear,t_nubit32 count)
{
	char t,c[0x11];
	t_nubit32 ilinear;
	t_nubit32 start = linear;
	t_nubit32 end = linear + count - 1;
	c[0x10] = '\0';
	if (!count) return;
	if (end < start) end = 0xffffffff;
	for(ilinear = start - (start % 0x10);ilinear <= end + 0x0f - (end % 0x10);++ilinear) {
		if (ilinear % 0x10 == 0) PRINTF("L%08X  ", ilinear);
		if (ilinear < start || ilinear > end) {
			PRINTF("  ");
			c[ilinear % 0x10] = ' ';
		} else {
			if (vcpuinsReadLinear(ilinear, GetRef(c[ilinear % 0x10]), 1)) {
				PRINTF("debug: fail to read from L%08X\n", ilinear);
				return;
			} else {
				PRINTF("%02X",c[ilinear % 0x10] & 0xff);
				t = c[ilinear % 0x10];
				if((t >=1 && t <= 7) || t == ' ' ||
					(t >=11 && t <= 12) ||
					(t >=14 && t <= 31) ||
					(t >=33)) ;
				else c[ilinear%0x10] = '.';
			}
		}
		PRINTF(" ");
		if(ilinear % 0x10 == 7 && ilinear >= start && ilinear < end) PRINTF("\b-");
		if((ilinear + 1) % 0x10 == 0) {
			PRINTF("  %s\n",c);
		}
		if (ilinear == 0xffffffff) break;
	}
	xdlin = ilinear;
}
static void xd()
{
	t_nubit32 count;
	if(narg == 1) xdprint(xdlin, 0x80);
	else if(narg == 2) {
		xdlin = scannubit32(arg[1]);
		if(nErrPos) return;
		xdprint(xdlin, 0x80);
	} else if(narg == 3) {
		xdlin = scannubit32(arg[1]);
		count = scannubit32(arg[2]);
		if(nErrPos) return;
		xdprint(xdlin, count);
	} else seterr(3);
}
/* enter */
static void xe()
{
	t_nubit8 i, val;
	t_nubit32 linear;
	t_string s;
	if(narg == 1) seterr(0);
	else if(narg == 2) {
		linear = scannubit32(arg[1]);
		if(nErrPos) return;
		if (vcpuinsReadLinear(linear, GetRef(val), 1)) {
			PRINTF("debug: fail to read from L%08X.\n", linear);
			return;
		}
		PRINTF("L%08X  %02X.", linear, val);
		FGETS(s,MAXLINE,stdin);
		utilsLowerStr(s);
		val = scannubit8(s);
		if(nErrPos) return;
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			if (vcpuinsWriteLinear(linear, GetRef(val), 1))
				PRINTF("debug: fail to write to L%08X.\n", linear);
	} else if(narg > 2) {
		linear = scannubit32(arg[1]);
		if(nErrPos) return;
		for(i = 2;i < narg;++i) {
			val = scannubit8(arg[i]);
			if(!nErrPos) {
				if (vcpuinsWriteLinear(linear, GetRef(val), 1)) {
					PRINTF("debug: fail to write to L%08X.\n", linear);
					return;
				}
			} else break;
			linear++;
		}
	}
}
/* fill */
static void xf()
{
	t_nubit8  val;
	t_nubit32 count;
	t_nubit32 i, j;
	t_nubit32 bcount;
	t_nubit32 linear;
	if(narg < 4) seterr(narg-1);
	else {
		linear = scannubit32(arg[1]);
		if (nErrPos) return;
		count = scannubit32(arg[2]);
		if (nErrPos) return;
		bcount = (t_nubit32) narg - 3;
		for (i = 0, j = 0;i < count;++i, ++j) {
			val = scannubit8(arg[j % bcount + 3]);
			if (nErrPos) return;
			if (vcpuinsWriteLinear(linear + i, GetRef(val), 1)) {
				PRINTF("debug: fail to write to L%08X.\n", linear + i);
				return;
			}
		}
	}
}
/* go */
static void xg()
{
	t_nubit32 i;
	t_nubit32 count = 0;
	if (device.flagRun) {
		PRINTF("NXVM is already running.\n");
		return;
	}
	switch(narg) {
	case 1:
		vdebug.flagBreak32 = 0;
		count = 1;
		break;
	case 2:
		vdebug.flagBreak32 = 1;
		vdebug.breakLinear = scannubit32(arg[1]);
		count = 1;
		break;
	case 3:
		vdebug.flagBreak32 = 1;
		vdebug.breakLinear = scannubit32(arg[1]);
		count = scannubit32(arg[2]);
		break;
	default:seterr(narg-1);break;}
	if(nErrPos) return;
	for (i = 0;i < count;++i) {
		vdebug.breakCount = 0;
		machineResume();
		while (device.flagRun) utilsSleep(1);
		PRINTF("%d instructions executed before the break point.\n", vdebug.breakCount);
		xrprintreg();
	}
	vdebug.flagBreak32 = 0;
}
/* move */
static void xm()
{
	t_nubit8 val;
	t_nubit32 i, lin1, lin2, count;
	if(narg != 4) seterr(narg-1);
	else {
		lin1 = scannubit32(arg[1]);
		if (nErrPos) return;
		lin2 = scannubit32(arg[2]);
		if (nErrPos) return;
		count = scannubit32(arg[3]);
		if (nErrPos) return;
		for (i = 0;i < count;++i) {
			if (vcpuinsReadLinear(lin1 + i, GetRef(val), 1)) {
				PRINTF("debug: fail to read from L%08X.\n", lin1 + i);
				return;
			}
			if (vcpuinsWriteLinear(lin2 + i, GetRef(val), 1)) {
				PRINTF("debug: fail to write to L%08X.\n", lin2 + i);
				return;
			}
		}
	}
}
/* search */
static void xs()
{
	t_nubit32 linear, count, bcount, i;
	t_nubit8 val, mem[256], line[256];
	if(narg < 4) seterr(narg-1);
	else {
		linear = scannubit32(arg[1]);
		if (nErrPos) return;
		count = scannubit32(arg[2]);
		if (nErrPos) return;
		addrparse(vcpu.ds.selector,arg[1]);
		bcount = (t_nubit32) narg - 3;
		for (i = 0;i < bcount;++i) {
			val = scannubit8(arg[i + 3]);
			if (nErrPos) return;
			line[i] = val;
		}
		for (i = 0;i < count;++i) {
			if (vcpuinsReadLinear(linear + i, (t_vaddrcc)mem, bcount)) {
				PRINTF("debug: fail to read from L%08X.\n", linear + i);
				return;
			}
			if (!MEMCMP(mem, line, bcount)) {
				PRINTF("L%08X\n", linear + i);
			}
		}
	}
}
/* trace */
static void xt()
{
	t_nubit32 i;
	t_nubit32 count;
	if (device.flagRun) {
		PRINTF("NXVM is already running.\n");
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
	if(nErrPos) return;
	if (count < 0x0100) {
		for(i = 0;i < count;++i) {
			vdebug.traceCount = 0x01;
			machineResume();
			while (device.flagRun) utilsSleep(10);
			devicePrintCpuMem();
			xrprintreg();
			if (i != count - 1) PRINTF("\n");
		}
	} else {
		vdebug.traceCount = count;
		machineResume();
		while (device.flagRun) utilsSleep(10);
		devicePrintCpuMem();
		xrprintreg();
	}
	vdebug.traceCount = 0x00;
	/* gexec(ptr1,ptr2); */
}
/* register */
static void xrscanreg()
{
	t_nubit32 value;
	t_string s;
	if(!STRCMP(arg[1], "eax")) {
		PRINTF("EAX ");
		PRINTF("%08X", _eax);
		PRINTF("\n:");
		FGETS(s, MAXLINE, stdin);
		value = scannubit32(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			_eax = value;
	} else if(!STRCMP(arg[1], "ecx")) {
		PRINTF("ECX ");
		PRINTF("%08X", _ecx);
		PRINTF("\n:");
		FGETS(s, MAXLINE, stdin);
		value = scannubit32(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			_ecx = value;
	} else if(!STRCMP(arg[1], "edx")) {
		PRINTF("EDX ");
		PRINTF("%08X", _edx);
		PRINTF("\n:");
		FGETS(s, MAXLINE, stdin);
		value = scannubit32(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			_edx = value;
	} else if(!STRCMP(arg[1], "ebx")) {
		PRINTF("EBX ");
		PRINTF("%08X", _ebx);
		PRINTF("\n:");
		FGETS(s, MAXLINE, stdin);
		value = scannubit32(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			_ebx = value;
	} else if(!STRCMP(arg[1], "esp")) {
		PRINTF("ESP ");
		PRINTF("%08X", vcpu.esp);
		PRINTF("\n:");
		FGETS(s, MAXLINE, stdin);
		value = scannubit32(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			vcpu.esp = value;
	} else if(!STRCMP(arg[1], "ebp")) {
		PRINTF("EBP ");
		PRINTF("%08X", _ebp);
		PRINTF("\n:");
		FGETS(s, MAXLINE, stdin);
		value = scannubit32(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			_ebp = value;
	} else if(!STRCMP(arg[1], "esi")) {
		PRINTF("ESI ");
		PRINTF("%08X", vcpu.esi);
		PRINTF("\n:");
		FGETS(s, MAXLINE, stdin);
		value = scannubit32(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			vcpu.esi = value;
	} else if(!STRCMP(arg[1], "edi")) {
		PRINTF("EDI ");
		PRINTF("%08X", _edi);
		PRINTF("\n:");
		FGETS(s, MAXLINE, stdin);
		value = scannubit32(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			_edi = value;
	} else if(!STRCMP(arg[1], "eip")) {
		PRINTF("EIP ");
		PRINTF("%08X", _eip);
		PRINTF("\n:");
		FGETS(s, MAXLINE, stdin);
		value = scannubit32(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			_eip = value;
	} else if(!STRCMP(arg[1], "eflags")) {
		PRINTF("EFLAGS ");
		PRINTF("%08X", _eflags);
		PRINTF("\n:");
		FGETS(s, MAXLINE, stdin);
		value = scannubit32(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			_eflags = value;
	} else if(!STRCMP(arg[1], "es")) {
		devicePrintCpuSreg();
		PRINTF(":");
		FGETS(s, MAXLINE, stdin);
		value = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			if (vcpuinsLoadSreg(&vcpu.es, GetMax16(value)))
				PRINTF("debug: fail to load es from %04X\n", GetMax16(value));
	} else if(!STRCMP(arg[1], "cs")) {
		devicePrintCpuSreg();
		PRINTF(":");
		FGETS(s, MAXLINE, stdin);
		value = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			if (vcpuinsLoadSreg(&vcpu.cs, GetMax16(value)))
				PRINTF("debug: fail to load cs from %04X\n", GetMax16(value));
	}  else if(!STRCMP(arg[1], "ss")) {
		devicePrintCpuSreg();
		PRINTF(":");
		FGETS(s, MAXLINE, stdin);
		value = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			if (vcpuinsLoadSreg(&vcpu.ss, GetMax16(value)))
				PRINTF("debug: fail to load ss from %04X\n", GetMax16(value));
	} else if(!STRCMP(arg[1], "ds")) {
		devicePrintCpuSreg();
		PRINTF(":");
		FGETS(s,MAXLINE,stdin);
		value = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos) 
			if (vcpuinsLoadSreg(&vcpu.ds, GetMax16(value)))
				PRINTF("debug: fail to load ds from %04X\n", GetMax16(value));
	} else if(!STRCMP(arg[1], "fs")) {
		devicePrintCpuSreg();
		PRINTF(":");
		FGETS(s,MAXLINE,stdin);
		value = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos) 
			if (vcpuinsLoadSreg(&vcpu.fs, GetMax16(value)))
				PRINTF("debug: fail to load fs from %04X\n", GetMax16(value));
	} else if(!STRCMP(arg[1], "gs")) {
		devicePrintCpuSreg();
		PRINTF(":");
		FGETS(s,MAXLINE,stdin);
		value = scannubit16(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos) 
			if (vcpuinsLoadSreg(&vcpu.gs, GetMax16(value)))
				PRINTF("debug: fail to load gs from %04X\n", GetMax16(value));
	} else if(!STRCMP(arg[1], "cr0")) {
		PRINTF("CR0 ");
		PRINTF("%08X", vcpu.cr1);
		PRINTF("\n:");
		FGETS(s, MAXLINE, stdin);
		value = scannubit32(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			vcpu.cr0 = value;
	} else if(!STRCMP(arg[1], "cr2")) {
		PRINTF("CR2 ");
		PRINTF("%08X", vcpu.cr2);
		PRINTF("\n:");
		FGETS(s, MAXLINE, stdin);
		value = scannubit32(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			vcpu.cr2 = value;
	} else if(!STRCMP(arg[1], "cr3")) {
		PRINTF("CR3 ");
		PRINTF("%08X", vcpu.cr3);
		PRINTF("\n:");
		FGETS(s, MAXLINE, stdin);
		value = scannubit32(s);
		if(s[0] != '\0' && s[0] != '\n' && !nErrPos)
			vcpu.cr3 = value;
	} else PRINTF("br Error\n");
}
static void xr()
{
	if(narg == 1) {
		xrprintreg();
	} else if(narg == 2) {
		xrscanreg();
	} else seterr(2);
}
/* unassemble */
static void xuprint(t_nubit32 linear, t_nubit8 count)
{
	t_nubit32 len = 0;
	t_nubit8 i;
	for (i = 0;i < count;++i) {
		len = xuprintins(linear);
		if (!len) break;
		linear += len;
	}
	xulin = linear;
}
static void xu()
{
	t_nubit32 count;
	if(narg == 1) xuprint(xulin, 10);
	else if(narg == 2) {
		xulin = scannubit32(arg[1]);
		if(nErrPos) return;
		xuprint(xulin, 0x10);
	} else if(narg == 3) {
		xulin = scannubit32(arg[1]);
		count = scannubit32(arg[2]);
		if(nErrPos) return;
		xuprint(xulin, count);
	} else seterr(3);
}
/* watch */
static void xw()
{
	switch(narg) {
	case 1:
		if (vcpuins.flagwr) PRINTF("Watch-read point: Lin=%08x\n", vcpuins.wrlin);
		if (vcpuins.flagww) PRINTF("Watch-write point: Lin=%08x\n", vcpuins.wwlin);
		if (vcpuins.flagwe) PRINTF("Watch-exec point: Lin=%08x\n", vcpuins.welin);
		break;
	case 2:
		switch (arg[1][0]) {
		case 'r':
			vcpuins.flagwr = 0;
			PRINTF("Watch-read point removed.\n");
			break;
		case 'w':
			vcpuins.flagww = 0;
			PRINTF("Watch-write point removed.\n");
			break;
		case 'e':
			vcpuins.flagwe = 0;
			PRINTF("Watch-exec point removed.\n");
			break;
		case 'u':
			vcpuins.flagwr = 0;
			vcpuins.flagww = 0;
			vcpuins.flagwe = 0;
			PRINTF("All watch points removed.\n");
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
static void xhelp()
{
	PRINTF("assemble        XA [address]\n");
	PRINTF("compare         XC addr1 addr2 count_byte\n");
	PRINTF("dump            XD [address [count_byte]]\n");
	PRINTF("enter           XE address [byte_list]\n");
	PRINTF("fill            XF address count_byte byte_list\n");
	PRINTF("go              XG [address [count_instr]]\n");
	PRINTF("move            XM addr1 addr2 count_byte\n");
	PRINTF("register        XR [register]\n");
	PRINTF("  regular         XREG\n");
	PRINTF("  segment         XSREG\n");
	PRINTF("  control         XCREG\n");
	PRINTF("search          XS address count_byte byte_list\n");
	PRINTF("trace           XT [count_instr]\n");
	PRINTF("unassemble      XU [address [count_instr]]\n");
	PRINTF("watch           XW r/w/e address\n");
}
static void x()
{
	t_nubit8 i;
	arg[narg] = arg[0];
	for (i = 1;i < narg;++i) arg[i - 1] = arg[i];
	arg[narg - 1] = arg[narg];
	arg[narg] = NULL;
	narg--;
	if (!STRCMP(arg[0], "\?")) xhelp();
	else if (!STRCMP(arg[0], "a"))    xa();
	else if (!STRCMP(arg[0], "c"))    xc();
	else if (!STRCMP(arg[0], "d"))    xd();
	else if (!STRCMP(arg[0], "e"))    xe();
	else if (!STRCMP(arg[0], "f"))    xf();
	else if (!STRCMP(arg[0], "g"))    xg();
	else if (!STRCMP(arg[0], "m"))    xm();
	else if (!STRCMP(arg[0], "r"))    xr();
	else if (!STRCMP(arg[0], "s"))    xs();
	else if (!STRCMP(arg[0], "t"))    xt();
	else if (!STRCMP(arg[0], "u"))    xu();
	else if (!STRCMP(arg[0], "w"))    xw();
	else if (!STRCMP(arg[0], "reg"))  xrprintreg();
	else if (!STRCMP(arg[0], "sreg")) devicePrintCpuSreg();
	else if (!STRCMP(arg[0], "creg")) devicePrintCpuCreg();
	else {
		arg[0] = arg[narg];
		seterr(0);
	}
}
/* EXTENDED DEBUG CMD END */

/* main routines */
static void help() {
	PRINTF("assemble        A [address]\n");
	PRINTF("compare         C range address\n");
	PRINTF("dump            D [range]\n");
	PRINTF("enter           E address [list]\n");
	PRINTF("fill            F range list\n");
	PRINTF("go              G [[address] breakpoint]\n");
	/* PRINTF("go              G [=address] [addresses]\n"); */
	PRINTF("hex             H value1 value2\n");
	PRINTF("input           I port\n");
	PRINTF("load            L [address]\n");
	/* PRINTF("load            L [address] [drive] [firstsector] [number]\n"); */
	PRINTF("move            M range address\n");
	PRINTF("name            N pathname\n");
	/* PRINTF("name            N [pathname] [arglist]\n"); */
	PRINTF("output          O port byte\n");
	/* !PRINTF("proceed           P [nx=address] [number]\n"); */
	PRINTF("quit            Q \n");
	PRINTF("register        R [register]\n");
	PRINTF("search          S range list\n");
	PRINTF("trace           T [[address] value]\n");
	/* PRINTF("trace           T [=address] [value]\n"); */
	PRINTF("unassemble      U [range]\n");
	PRINTF("verbal          V \n");
	PRINTF("write           W [address]\n");
	PRINTF("debug32         X?\n");
	/* PRINTF("write           W [address] [drive] [firstsector] [number]\n"); */
	/* PRINTF("allocate expanded memory        XA [#pages]\n"); */
	/* PRINTF("deallocate expanded memory      XD [handle]\n"); */
	/* PRINTF("map expanded memory pages       XM [Lpage] [Ppage] [handle]\n"); */
	/* PRINTF("display expanded memory status  XS\n"); */
}

static void parse() {
	STRCPY(strCmdCopy, strCmdBuff);
	narg = 0;
	arg[0] = STRTOK(strCmdCopy, " ,\t\n\r\f");
	if (arg[narg]) {
		utilsLowerStr(arg[narg]);
		narg++;
	} else {
		return;
	}
	if (strlen(arg[narg-1]) != 1) {
		arg[narg] = arg[narg-1]+1;
		narg++;
	}
	while (narg < DEBUG_MAXNARG) {
		arg[narg] = STRTOK(NULL," ,\t\n\r\f");
		if (arg[narg]) {
			utilsLowerStr(arg[narg]);
			narg++;
		} else {
			break;
		}
	}
}

static void exec() {
	nErrPos = 0;
	if (!arg[0]) {
		return;
	}
	switch(arg[0][0]) {
	case '\?': help(); break;
	case 'a': a(); break;
	case 'c': c(); break;
	case 'd': d(); break;
	case 'e': e(); break;
	case 'f': f(); break;
	case 'g': g(); break;
	case 'h': h(); break;
	case 'i': i(); break;
	case 'l': l(); break;
	case 'm': m(); break;
	case 'n': n(); break;
	case 'o': o(); break;
	case 'q': q(); break;
	case 'r': r(); break;
	case 's': s(); break;
	case 't': t(); break;
	case 'u': u(); break;
	case 'v': v(); break;
	case 'w': w(); break;
	case 'x': x(); break;
	default: seterr(0); break;
	}
}

void debugMain() {
	t_nubitcc i;
	strFileName[0] = '\0';
	asmSegRec = uasmSegRec = vcpu.cs.selector;
	asmPtrRec = uasmPtrRec = _ip;
	dumpSegRec = vcpu.ds.selector;
	dumpPtrRec = (t_nubit16)(_ip) / 0x10 * 0x10;
	xalin = 0;
	xdlin = 0;
	xulin = vcpu.cs.base + _eip;
	arg = (char **)MALLOC(DEBUG_MAXNARG * sizeof(char *));
	flagexit = 0;
	while (!flagexit) {
		fflush(stdin);
		PRINTF("-");
		FGETS(strCmdBuff,MAXLINE,stdin);
		parse();
		exec();
		if (nErrPos) {
			for (i = 0;i < nErrPos;++i) PRINTF(" ");
			PRINTF("^ Error\n");
		}
	}
	FREE((void *) arg);
}

/* recorder */
void debugRecordStart(const t_strptr fileName) {
	if (vdebug.recordFile) {
		FCLOSE(vdebug.recordFile);
	}
	vdebug.recordFile = FOPEN(fileName, "w");
	if (!vdebug.recordFile) {
		PRINTF("ERROR:\tcannot write dump file.\n");
	} else {
		PRINTF("Record started.\n");
	}
}
void debugRecordStop() {
	if (!vdebug.recordFile) {
		PRINTF("ERROR:\trecorder not turned on.\n");
	} else {
		PRINTF("Record finished.\n");
		FCLOSE(vdebug.recordFile);
		vdebug.recordFile = (FILE *) NULL;
	}
}
