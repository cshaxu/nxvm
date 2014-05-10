/* This file is a part of NXVM project. */

/* DEBUG is the debug console for users to break, trace, lookup,
 * and print virtual machine devices. */

#include "utils.h"

#include "device/device.h"
#include "machine.h"

#include "debug.h"

#define DEBUG_MAXNARG 256
#define DEBUG_MAXNASMARG 4

static size_t nErrPos;
static size_t narg;
static char **arg;
static int flagExit;
static char strCmdBuff[0x100], strCmdCopy[0x100], strFileName[0x100];

static void seterr(size_t pos) {
    nErrPos = (size_t)(arg[pos] - strCmdCopy + strlen(arg[pos]) + 1);
}
static uint8_t scannubit8(char *s) {
    uint8_t ans = 0;
    size_t i = 0;
    if (s[0] == '\'' && s[2] == '\'') {
        return s[1];
    }
    utilsLowerStr(s);
    while (s[i] != '\0' && s[i] != '\n') {
        if (i > 1) {
            seterr(narg - 1);
            break;
        }
        ans <<= 4;
        if (s[i] > 0x2f && s[i] < 0x3a) {
            ans += s[i] - 0x30;
        } else if (s[i] > 0x60 && s[i] < 0x67) {
            ans += s[i] - 0x57;
        } else {
            seterr(narg - 1);
        }
        ++i;
    }
    return ans;
}
static uint16_t scannubit16(char *s) {
    uint16_t ans = 0;
    size_t i = 0;
    utilsLowerStr(s);
    while (s[i] != '\0' && s[i] != '\n') {
        if (i > 3) {
            seterr(narg - 1);
            break;
        }
        ans <<= 4;
        if (s[i] > 0x2f && s[i] < 0x3a) {
            ans += s[i] - 0x30;
        } else if (s[i] > 0x60 && s[i] < 0x67) {
            ans += s[i] - 0x57;
        } else {
            seterr(narg - 1);
        }
        ++i;
    }
    return ans;
}
static uint32_t scannubit32(char *s) {
    uint32_t ans = 0;
    size_t i = 0;
    utilsLowerStr(s);
    while (s[i] != '\0' && s[i] != '\n') {
        if (i > 7) {
            seterr(narg - 1);
            break;
        }
        ans <<= 4;
        if (s[i] > 0x2f && s[i] < 0x3a) {
            ans += s[i] - 0x30;
        } else if (s[i] > 0x60 && s[i] < 0x67) {
            ans += s[i] - 0x57;
        } else {
            seterr(narg - 1);
        }
        ++i;
    }
    return ans;
}

static uint16_t dumpSegRec;
static uint16_t dumpPtrRec;
static uint16_t asmSegRec;
static uint16_t asmPtrRec;
static uint16_t uasmSegRec;
static uint16_t uasmPtrRec;
static uint16_t seg;
static uint16_t ptr;

static void addrparse(uint16_t defseg, const char *addr) {
    char *cseg, *cptr;
    char ccopy[0x100];
    STRCPY(ccopy, addr);
    cseg = STRTOK(ccopy,":");
    cptr = STRTOK(NULL,"");
    if (!cptr) {
        seg = defseg;
        ptr = scannubit16(cseg);
    } else {
        if (!STRCMP(cseg,"es")) {
            seg = _es;
        } else if (!STRCMP(cseg,"cs")) {
            seg = _cs;
        } else if (!STRCMP(cseg,"ss")) {
            seg = _ss;
        } else if (!STRCMP(cseg,"ds")) {
            seg = _ds;
        } else {
            seg = scannubit16(cseg);
        }
        ptr = scannubit16(cptr);
    }
}

/* DEBUG CMD BEGIN */
/* assemble */
static void aconsole() {
    size_t i, len, errAsmPos;
    char cmdAsmBuff[0x100];
    uint8_t acode[15];
    int flagExitAsm = 0;
    while (!flagExitAsm) {
        PRINTF("%04X:%04X ", asmSegRec, asmPtrRec);
        fflush(stdin);
        FGETS(cmdAsmBuff, 0x100, stdin);
        utilsLowerStr(cmdAsmBuff);
        if (!strlen(cmdAsmBuff)) {
            flagExitAsm = 1;
            continue;
        }
        if (cmdAsmBuff[0] == ';' ) {
            continue;
        }
        errAsmPos = 0;
        len = utilsAasm32(cmdAsmBuff, acode, deviceConnectCpuGetCsDefSize());
        if (!len) {
            errAsmPos = strlen(cmdAsmBuff) + 9;
        } else {
            if (deviceConnectCpuWriteLinear((asmSegRec << 4) + asmPtrRec, (void *) acode, (uint8_t) len)) {
                PRINTF("debug: fail to write to L%08X\n", (asmSegRec << 4) + asmPtrRec);
                return;
            }
            asmPtrRec += (uint16_t) len;
        }
        if (errAsmPos) {
            for (i = 0; i < errAsmPos; ++i) {
                PRINTF(" ");
            }
            PRINTF("^ Error\n");
        }
    }
}
static void a() {
    if (narg == 1) {
        aconsole();
    } else if (narg == 2) {
        addrparse(_cs, arg[1]);
        if (nErrPos) {
            return;
        }
        asmSegRec = seg;
        asmPtrRec = ptr;
        aconsole();
    } else {
        seterr(2);
    }
}
/* compare */
static void c() {
    size_t i;
    uint8_t val1, val2;
    uint16_t seg1, ptr1, seg2, ptr2, range;
    if (narg != 4) {
        seterr(narg - 1);
    } else {
        addrparse(_ds, arg[1]);
        seg1 = seg;
        ptr1 = ptr;
        addrparse(_ds,arg[3]);
        seg2 = seg;
        ptr2 = ptr;
        range = scannubit16(arg[2])-ptr1;
        if (!nErrPos) {
            for (i = 0; i <= range; ++i) {
                deviceConnectRamRealRead(seg1, (uint16_t)(ptr1 + i), (void *)(&val1), 1);
                deviceConnectRamRealRead(seg2, (uint16_t)(ptr2 + i), (void *)(&val2), 1);
                if (val1 != val2) {
                    PRINTF("%04X:%04X  ", seg1, (uint16_t)(ptr1 + i));
                    PRINTF("%02X  %02X", val1, val2);
                    PRINTF("  %04X:%04X\n", seg2, (uint16_t)(ptr2 + i));
                }
            }
        }
    }
}
/* dump */
static void dprint(uint16_t segment, uint16_t start, uint16_t end) {
    char t, c[0x11];
    uint16_t iaddr;
    if (start > end) end = 0xffff;
    if ((uint32_t)((segment << 4) + end) > 0x000fffff) {
        end = (uint16_t)(0x000fffff - (segment << 4));
    }
    c[0x10] = '\0';
    if (end < start) {
        end = 0xffff;
    }
    for (iaddr = start - (start % 0x10); iaddr <= end + (0x10 - end % 0x10) - 1; ++iaddr) {
        if (iaddr % 0x10 == 0) {
            PRINTF("%04X:%04X  ", segment, iaddr);
        }
        if (iaddr < start || iaddr > end) {
            PRINTF("  ");
            c[iaddr % 0x10] = ' ';
        } else {
            deviceConnectRamRealRead(segment, iaddr, (void *)(&c[iaddr % 0x10]), 1);
            PRINTF("%02X", c[iaddr % 0x10] & 0xff);
            t = c[iaddr % 0x10];
            if ((t >=1 && t <= 7) || t == ' ' ||
                    (t >=11 && t <= 12) ||
                    (t >=14 && t <= 31) ||
                    (t >=33)) {
            } else {
                c[iaddr % 0x10] = '.';
            }
        }
        PRINTF(" ");
        if (iaddr % 0x10 == 7 && iaddr >= start && iaddr < end) {
            PRINTF("\b-");
        }
        if ((iaddr + 1) % 0x10 == 0) {
            PRINTF("  %s\n",c);
        }
        if (iaddr == 0xffff) {
            break;
        }
    }
    dumpSegRec = segment;
    dumpPtrRec = end + 1;
}
static void d() {
    uint16_t ptr2;
    if (narg == 1) {
        dprint(dumpSegRec, dumpPtrRec, dumpPtrRec + 0x7f);
    } else if (narg == 2) {
        addrparse(_ds, arg[1]);
        if (nErrPos) {
            return;
        }
        dprint(seg, ptr, ptr + 0x7f);
    } else if (narg == 3) {
        addrparse(_ds, arg[1]);
        ptr2 = scannubit16(arg[2]);
        if (nErrPos) {
            return;
        }
        if (ptr > ptr2) {
            seterr(2);
        } else {
            dprint(seg, ptr, ptr2);
        }
    } else {
        seterr(3);
    }
}
/* enter */
static void e() {
    size_t i;
    uint8_t val;
    char s[0x100];
    if (narg == 1) {
        seterr(0);
    } else if (narg == 2) {
        addrparse(_ds, arg[1]);
        if (nErrPos) {
            return;
        }
        PRINTF("%04X:%04X  ", seg, ptr);
        deviceConnectRamRealRead(seg, ptr, (void *)(&val), 1);
        PRINTF("%02X", val);
        PRINTF(".");
        FGETS(s, 0x100, stdin);
        utilsLowerStr(s); /* MARK */
        val = scannubit8(s); /* MARK */
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            deviceConnectRamRealWrite(seg, ptr, (void *)(&val), 1);
        }
    } else if (narg > 2) {
        addrparse(_ds, arg[1]);
        if (nErrPos) {
            return;
        }
        for (i = 2; i < narg; ++i) {
            val = scannubit8(arg[i]); /* MARK */
            if (!nErrPos) {
                deviceConnectRamRealWrite(seg, ptr, (void *)(&val), 1);
            } else {
                break;
            }
            ptr++;
        }
    }
}
/* fill */
static void f() {
    uint8_t nbyte;
    uint8_t val;
    size_t i, j;
    uint16_t end;
    if (narg < 4) {
        seterr(narg - 1);
    } else {
        addrparse(_ds, arg[1]);
        end = scannubit16(arg[2]);
        if (end < ptr) {
            seterr(2);
        }
        if (!nErrPos) {
            nbyte = (uint8_t) narg - 3;
            for (i = ptr, j = 0; i <= end; ++i, ++j) {
                val = scannubit8(arg[j % nbyte + 3]);
                if (!nErrPos) {
                    deviceConnectRamRealWrite(seg, (uint16_t) i, (void *)(&val), 1);
                } else {
                    return;
                }
            }
        }
    }
}
/* go */
static void rprintregs();
static void g() {
    if (device.flagRun) {
        PRINTF("NXVM is already running.\n");
        return;
    }
    switch (narg) {
    case 1:
        deviceConnectDebugClearBreak();
        break;
    case 2:
        addrparse(_cs, arg[1]);
        deviceConnectDebugSetBreak(seg, ptr);
        break;
    case 3:
        addrparse(_cs, arg[1]);
        if (deviceConnectCpuLoadCS(seg)) {
            PRINTF("debug: fail to load cs from %04X\n", seg);
            return;
        }
        _ip = ptr;
        addrparse(_cs, arg[2]);
        deviceConnectDebugSetBreak(seg, ptr);
        break;
    default:
        seterr(narg - 1);
        break;
    }
    if (nErrPos) {
        return;
    }
    machineResume();
    while (device.flagRun) {
        utilsSleep(10);
    }
    deviceConnectDebugClearBreak();
    rprintregs();
}
/* hex */
static void h() {
    uint16_t val1, val2;
    if (narg != 3) {
        seterr(narg - 1);
    } else {
        val1 = scannubit16(arg[1]);
        val2 = scannubit16(arg[2]);
        if (!nErrPos) {
            PRINTF("%04X", (uint16_t)(val1 + val2));
            PRINTF("  ");
            PRINTF("%04X", (uint16_t)(val1 - val2));
            PRINTF("\n");
        }
    }
}
/* input */
static void i() {
    uint16_t in;
    if (narg != 2) {
        seterr(narg - 1);
    } else {
        in = scannubit16(arg[1]);
        if (nErrPos) {
            return;
        }
        deviceConnectPortRead(in);
        PRINTF("%08X\n", deviceConnectPortGetValue());
    }
}
/* load */
static void l() {
    uint8_t c;
    uint16_t i = 0;
    uint32_t len = 0;
    FILE *load = FOPEN(strFileName, "rb");
    if (!load) PRINTF("File not found\n");
    else {
        switch (narg) {
        case 1:
            seg = _cs;
            ptr = 0x100;
            break;
        case 2:
            addrparse(_cs, arg[1]);
            break;
        default:
            seterr(narg - 1);
            break;
        }
        if (!nErrPos) {
            c = fgetc(load);
            while (!feof(load)) {
                deviceConnectRamRealWrite(seg + i, ptr + len++, (void *)(&c), 1);
                i = len / 0x10000;
                c = fgetc(load);
            }
            _cx = len & 0xffff;
            if (len > 0xffff) _bx = (len>>16);
            else _bx = 0x0000;
        }
        FCLOSE(load);
    }
}
/* move */
static void m() {
    size_t i;
    uint8_t val;
    uint16_t seg1, ptr1, range, seg2, ptr2;
    if (narg != 4) seterr(narg - 1);
    else {
        addrparse(_ds, arg[1]);
        seg1 = seg;
        ptr1 = ptr;
        addrparse(_ds, arg[3]);
        seg2 = seg;
        ptr2 = ptr;
        range = scannubit16(arg[2]) - ptr1;
        if (!nErrPos) {
            if (((seg1 << 4) + ptr1) < ((seg2 << 4) + ptr2)) {
                for (i = 0; i <= range; ++i) {
                    deviceConnectRamRealRead(seg1, (uint16_t)(ptr1 + range - i), (void *)(&val), 1);
                    deviceConnectRamRealWrite(seg2, (uint16_t)(ptr2 + range - i), (void *)(&val), 1);
                }
            } else if (((seg1 << 4) + ptr1) > ((seg2 << 4) + ptr2)) {
                for (i = 0; i <= range; ++i) {
                    deviceConnectRamRealRead(seg1, (uint16_t)(ptr1 + i), (void *)(&val), 1);
                    deviceConnectRamRealWrite(seg2, (uint16_t)(ptr2 + i), (void *)(&val), 1);
                }
            }
        }
    }
}
/* name */
static void n() {
    if (narg != 2) seterr(narg - 1);
    else STRCPY(strFileName,arg[1]);
}
/* output */
static void o() {
    uint16_t out;
    uint32_t value;
    if (narg != 3) seterr(narg - 1);
    else {
        out = scannubit16(arg[1]);
        if (nErrPos) return;
        value = scannubit32(arg[2]);
        if (nErrPos) return;
        deviceConnectPortSetValue(value);
        deviceConnectPortWrite(out);
    }
}
/* quit */
static void q() {
    flagExit = 1;
}
/* register */
static uint8_t uprintins(uint16_t seg, uint16_t off) {
    size_t i;
    uint8_t len;
    uint8_t ucode[15];
    char str[0x100], stmt[0x100], sbin[0x100];
    if (deviceConnectCpuReadLinear((seg << 4) + off, (void *) ucode, 15)) {
        len = 0;
        SPRINTF(str, "%04X:%04X <ERROR>", seg, off);
    } else {
        len = utilsDasm32(stmt, ucode, deviceConnectCpuGetCsDefSize());
        sbin[0] = 0;
        for (i = 0; i < len; ++i) {
            SPRINTF(sbin, "%s%02X", sbin, (uint8_t) ucode[i]);
        }
        SPRINTF(str, "%04X:%04X %s", seg, off, sbin);
        for (i = strlen(str); i < 24; ++i) {
            STRCAT(str, " ");
        }
        STRCAT(str, stmt);
    }
    PRINTF("%s\n", str);
    return len;
}
static void rprintflags() {
    PRINTF("%s ", deviceConnectCpuGetOF() ? "OV" : "NV");
    PRINTF("%s ", deviceConnectCpuGetDF() ? "DN" : "UP");
    PRINTF("%s ", deviceConnectCpuGetIF() ? "EI" : "DI");
    PRINTF("%s ", deviceConnectCpuGetSF() ? "NG" : "PL");
    PRINTF("%s ", deviceConnectCpuGetZF() ? "ZR" : "NZ");
    PRINTF("%s ", deviceConnectCpuGetAF() ? "AC" : "NA");
    PRINTF("%s ", deviceConnectCpuGetPF() ? "PE" : "PO");
    PRINTF("%s ", deviceConnectCpuGetCF() ? "CY" : "NC");
}
static void rprintregs() {
    PRINTF(  "AX=%04X", _ax);
    PRINTF("  BX=%04X", _bx);
    PRINTF("  CX=%04X", _cx);
    PRINTF("  DX=%04X", _dx);
    PRINTF("  SP=%04X", _sp);
    PRINTF("  BP=%04X", _bp);
    PRINTF("  SI=%04X", _si);
    PRINTF("  DI=%04X", _di);
    PRINTF("\nDS=%04X", _ds);
    PRINTF("  ES=%04X", _es);
    PRINTF("  SS=%04X", _ss);
    PRINTF("  CS=%04X", _cs);
    PRINTF("  IP=%04X", _ip);
    PRINTF("   ");
    rprintflags();
    PRINTF("\n");
    uprintins(_cs, _ip);
    uasmSegRec = _cs;
    uasmPtrRec = _ip;
}
static void rscanregs() {
    uint16_t value;
    char s[0x100];
    if (!STRCMP(arg[1], "ax")) {
        PRINTF("AX ");
        PRINTF("%04X", _ax);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit16(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _ax = value;
        }
    } else if (!STRCMP(arg[1], "bx")) {
        PRINTF("BX ");
        PRINTF("%04X", _bx);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit16(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _bx = value;
        }
    } else if (!STRCMP(arg[1], "cx")) {
        PRINTF("CX ");
        PRINTF("%04X", _cx);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit16(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _cx = value;
        }
    } else if (!STRCMP(arg[1], "dx")) {
        PRINTF("DX ");
        PRINTF("%04X",_dx);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit16(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _dx = value;
        }
    } else if (!STRCMP(arg[1], "bp")) {
        PRINTF("BP ");
        PRINTF("%04X",_bp);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit16(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _bp = value;
        }
    } else if (!STRCMP(arg[1], "sp")) {
        PRINTF("SP ");
        PRINTF("%04X",_sp);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit16(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _sp = value;
        }
    } else if (!STRCMP(arg[1], "si")) {
        PRINTF("SI ");
        PRINTF("%04X",_si);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit16(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _si = value;
        }
    } else if (!STRCMP(arg[1], "di")) {
        PRINTF("DI ");
        PRINTF("%04X",_di);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit16(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _di = value;
        }
    } else if (!STRCMP(arg[1], "ss")) {
        PRINTF("SS ");
        PRINTF("%04X", _ss);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit16(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            if (deviceConnectCpuLoadSS((uint16_t) value)) {
                PRINTF("debug: fail to load ss from %04X\n", (uint16_t) value);
            }
        }
    } else if (!STRCMP(arg[1], "cs")) {
        PRINTF("CS ");
        PRINTF("%04X", _cs);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit16(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            if (deviceConnectCpuLoadCS((uint16_t) value)) {
                PRINTF("debug: fail to load cs from %04X\n", (uint16_t) value);
            }
        }
    } else if (!STRCMP(arg[1], "ds")) {
        PRINTF("DS ");
        PRINTF("%04X", _ds);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit16(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            if (deviceConnectCpuLoadDS((uint16_t) value)) {
                PRINTF("debug: fail to load ds from %04X\n", (uint16_t) value);
            }
        }
    } else if (!STRCMP(arg[1], "es")) {
        PRINTF("ES ");
        PRINTF("%04X", _es);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit16(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            if (deviceConnectCpuLoadES((uint16_t) value)) {
                PRINTF("debug: fail to load es from %04X\n", (uint16_t) value);
            }
        }
    } else if (!STRCMP(arg[1], "ip")) {
        PRINTF("IP ");
        PRINTF("%04X",_ip);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit16(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _ip = value;
        }
    } else if (!STRCMP(arg[1], "f")) {
        rprintflags();
        PRINTF(" -");
        FGETS(s, 0x100, stdin);
        utilsLowerStr(s);
        if (!STRCMP(s,"ov")) {
            deviceConnectCpuSetOF();
        } else if (!STRCMP(s,"nv")) {
            deviceConnectCpuClearOF();
        } else if (!STRCMP(s,"dn")) {
            deviceConnectCpuSetDF();
        } else if (!STRCMP(s,"up")) {
            deviceConnectCpuClearDF();
        } else if (!STRCMP(s,"ei")) {
            deviceConnectCpuSetIF();
        } else if (!STRCMP(s,"di")) {
            deviceConnectCpuClearIF();
        } else if (!STRCMP(s,"ng")) {
            deviceConnectCpuSetSF();
        } else if (!STRCMP(s,"pl")) {
            deviceConnectCpuClearSF();
        } else if (!STRCMP(s,"zr")) {
            deviceConnectCpuSetZF();
        } else if (!STRCMP(s,"nz")) {
            deviceConnectCpuClearZF();
        } else if (!STRCMP(s,"ac")) {
            deviceConnectCpuSetAF();
        } else if (!STRCMP(s,"na")) {
            deviceConnectCpuClearAF();
        } else if (!STRCMP(s,"pe")) {
            deviceConnectCpuSetPF();
        } else if (!STRCMP(s,"po")) {
            deviceConnectCpuClearPF();
        } else if (!STRCMP(s,"cy")) {
            deviceConnectCpuSetCF();
        } else if (!STRCMP(s,"nc")) {
            deviceConnectCpuClearCF();
        } else {
            PRINTF("bf Error\n");
        }
    } else {
        PRINTF("br Error\n");
    }
}
static void r() {
    if (narg == 1) {
        rprintregs();
    } else if (narg == 2) {
        rscanregs();
    } else seterr(2);
}
/* search */
static void s() {
    size_t i;
    int flagFound = 0;
    uint16_t p, pfront, start, end;
    uint8_t cstart, val;
    if (narg < 4) {
        seterr(narg - 1);
    } else {
        addrparse(_ds, arg[1]);
        start = ptr;
        end = scannubit16(arg[2]);
        if (!nErrPos) {
            p = start;
            cstart = scannubit8(arg[3]);
            while (p <= end) {
                deviceConnectRamRealRead(seg, p, (void *)(&val), 1);
                if (val == cstart) {
                    pfront = p;
                    flagFound = 1;
                    for (i = 3; i < narg; ++i) {
                        deviceConnectRamRealRead(seg, p, (void *)(&val), 1);
                        if (val != scannubit8(arg[i])) {
                            flagFound = 0;
                            p = pfront + 1;
                            break;
                        } else {
                            ++p;
                        }
                    }
                    if (flagFound) {
                        PRINTF("%04X:%04X  ", seg, pfront);
                        PRINTF("\n");
                    }
                } else ++p;
            }
        }
    }
}
/* trace */
static void t() {
    size_t i;
    uint16_t count;
    if (device.flagRun) {
        PRINTF("NXVM is already running.\n");
        return;
    }
    switch (narg) {
    case 1:
        count = 1;
        break;
    case 2:
        count = scannubit16(arg[1]);
        break;
    case 3:
        addrparse(_cs, arg[1]);
        if (deviceConnectCpuLoadCS(seg)) {
            PRINTF("debug: fail to load cs from %04X\n", seg);
            return;
        }
        _ip = ptr;
        count = scannubit16(arg[2]);
        break;
    default:
        seterr(narg - 1);
        break;
    }
    if (nErrPos) {
        return;
    }
    if (count < 0x100) {
        for (i = 0; i < count; ++i) {
            deviceConnectDebugSetTrace(1);
            machineResume();
            while (device.flagRun) {
                utilsSleep(10);
            }
            rprintregs();
            if (i != count - 1) {
                PRINTF("\n");
            }
        }
    } else {
        deviceConnectDebugSetTrace(count);
        machineResume();
        while (device.flagRun) {
            utilsSleep(10);
        }
        rprintregs();
    }
    deviceConnectDebugClearTrace();
}
/* unassemble */
static void uprint(uint16_t segment, uint16_t start, uint16_t end) {
    uint8_t len;
    uint32_t boundary;
    if (start > end) {
        end = 0xffff;
    }
    if ((uint32_t)((segment << 4) + end) > 0xfffff) {
        end = (0xfffff - (segment << 4));
    }
    while (start <= end) {
        len = uprintins(segment, start);
        start += len;
        boundary = (uint32_t) start + (uint32_t) len;
        if (boundary > 0xffff) {
            break;
        }
    }
    uasmSegRec = segment;
    uasmPtrRec = start;
    return;
}
static void u() {
    uint16_t ptr2;
    if (narg == 1) {
        uprint(uasmSegRec, uasmPtrRec, uasmPtrRec + 0x1f);
    } else if (narg == 2) {
        addrparse(_cs, arg[1]);
        if (nErrPos) {
            return;
        }
        uprint(seg, ptr,  ptr + 0x1f);
    } else if (narg == 3) {
        addrparse(_ds, arg[1]);
        ptr2 = scannubit16(arg[2]);
        if (nErrPos) {
            return;
        }
        if (ptr > ptr2) {
            seterr(2);
        } else {
            uprint(seg, ptr, ptr2);
        }
    } else {
        seterr(3);
    }
}
/* verbal */
static void v() {
    size_t i;
    char str[0x100];
    PRINTF(":");
    FGETS(str, 0x100, stdin);
    str[strlen(str) - 1] = '\0';
    for (i = 0; i < strlen(str); ++i) {
        PRINTF("%02X", str[i]);
        if (!((i + 1) % 0x10)) {
            PRINTF("\n");
        } else if (!((i + 1) % 0x08) && (str[i + 1] != '\0')) {
            PRINTF("-");
        } else {
            PRINTF(" ");
        }
    }
    if (i % 0x10) {
        PRINTF("\n");
    }
}
/* write */
static void w() {
    size_t i = 0;
    uint8_t val;
    uint32_t len = (_bx << 16) + _cx;
    FILE *write;
    if (!strlen(strFileName)) {
        PRINTF("(W)rite error, no destination defined\n");
        return;
    } else {
        write = FOPEN(strFileName, "wb");
    }
    if (!write) {
        PRINTF("File not found\n");
    } else {
        PRINTF("Writing ");
        PRINTF("%04X", _bx);
        PRINTF("%04X", _cx);
        PRINTF(" bytes\n");
        switch (narg) {
        case 1:
            seg = _cs;
            ptr = 0x100;
            break;
        case 2:
            addrparse(_cs, arg[1]);
            break;
        default:
            seterr(narg - 1);
            break;
        }
        if (!nErrPos) {
            while (i < len) {
                deviceConnectRamRealRead(seg, (uint8_t)(ptr + i++), (void *)(&val), 1);
                fputc(val, write);
            }
        }
        FCLOSE(write);
    }
}
/* DEBUG CMD END */

/* EXTENDED DEBUG CMD BEGIN */
uint32_t xalin;
uint32_t xdlin;
uint32_t xulin;
/* print */
static uint8_t xuprintins(uint32_t linear) {
    size_t i;
    uint8_t len;
    uint8_t ucode[15];
    char str[0x100], stmt[0x100], sbin[0x100];
    if (deviceConnectCpuReadLinear(linear, (void *) ucode, 15)) {
        len = 0;
        SPRINTF(str, "L%08X <ERROR>", linear);
    } else {
        len = utilsDasm32(stmt, ucode, deviceConnectCpuGetCsDefSize());
        sbin[0] = 0;
        for (i = 0; i < len; ++i) {
            SPRINTF(sbin, "%s%02X", sbin, (uint8_t) ucode[i]);
        }
        SPRINTF(str, "L%08X %s ", linear, sbin);
        for (i = strlen(str); i < 24; ++i) {
            STRCAT(str, " ");
        }
        STRCAT(str, stmt);
    }
    PRINTF("%s\n", str);
    return len;
}
static void xrprintreg() {
    devicePrintCpuReg();
    xulin = deviceConnectCpuGetCsBase() + _eip;
    xuprintins(xulin);
}
/* assemble */
static void xaconsole(uint32_t linear) {
    size_t i, len, errAsmPos;
    char astmt[0x100];
    uint8_t acode[15];
    int flagExitAsm = 0;
    while (!flagExitAsm) {
        PRINTF("L%08X ", linear);
        FGETS(astmt, 0x100, stdin);
        fflush(stdin);
        astmt[strlen(astmt) - 1] = 0;
        if (!strlen(astmt)) {
            flagExitAsm = 1;
            continue;
        }
        errAsmPos = 0;
        len = utilsAasm32(astmt, acode, deviceConnectCpuGetCsDefSize());
        if (!len) {
            errAsmPos = strlen(astmt) + 9;
        } else {
            if (deviceConnectCpuWriteLinear(linear, (void *) acode, (uint8_t) len)) {
                PRINTF("debug: fail to write to L%08X\n", linear);
                return;
            }
            linear += (uint32_t) len;
        }
        if (errAsmPos) {
            for (i = 0; i < errAsmPos; ++i) {
                PRINTF(" ");
            }
            PRINTF("^ Error\n");
        }
    }
    xalin = linear;
}
static void xa() {
    if (narg == 1) {
        xaconsole(xalin);
    } else if (narg == 2) {
        xalin = scannubit32(arg[1]);
        if (nErrPos) {
            return;
        }
        xaconsole(xalin);
    } else {
        seterr(2);
    }
}
/* compare */
static void xc() {
    size_t i, count;
    uint32_t lin1, lin2;
    uint8_t val1, val2;
    if (narg != 4) {
        seterr(narg - 1);
    } else {
        lin1 = scannubit32(arg[1]);
        if (nErrPos) {
            return;
        }
        lin2 = scannubit32(arg[2]);
        if (nErrPos) {
            return;
        }
        count = scannubit32(arg[3]);
        if (nErrPos) {
            return;
        }
        if (!count) {
            return;
        }
        for (i = 0; i < count; ++i) {
            if (deviceConnectCpuReadLinear((uint32_t)(lin1 + i), (void *)(&val1), 1)) {
                PRINTF("debug: fail to read from L%08X.\n", (uint32_t)(lin1 + i));
                return;
            }
            if (deviceConnectCpuReadLinear((uint32_t)(lin2 + i), (void *)(&val2), 1)) {
                PRINTF("debug: fail to read from L%08X.\n", (uint32_t)(lin2 + i));
                return;
            }
            if (val1 != val2)
                PRINTF("L%08X  %02X  %02X  L%08X\n",
                       (uint32_t)(lin1 + i), val1, val2, (uint32_t)(lin2 + i));
        }
    }
}
/* dump */
static void xdprint(uint32_t linear,uint32_t count) {
    char t, c[0x11];
    uint32_t ilinear;
    uint32_t start = linear;
    uint32_t end = linear + count - 1;
    c[0x10] = '\0';
    if (!count) {
        return;
    }
    if (end < start) end = 0xffffffff;
    for (ilinear = start - (start % 0x10); ilinear <= end + 0x0f - (end % 0x10); ++ilinear) {
        if (ilinear % 0x10 == 0) PRINTF("L%08X  ", ilinear);
        if (ilinear < start || ilinear > end) {
            PRINTF("  ");
            c[ilinear % 0x10] = ' ';
        } else {
            if (deviceConnectCpuReadLinear(ilinear, (void *)(&c[ilinear % 0x10]), 1)) {
                PRINTF("debug: fail to read from L%08X\n", ilinear);
                return;
            } else {
                PRINTF("%02X",c[ilinear % 0x10] & 0xff);
                t = c[ilinear % 0x10];
                if ((t >=1 && t <= 7) || t == ' ' ||
                        (t >=11 && t <= 12) ||
                        (t >=14 && t <= 31) ||
                        (t >=33)) ;
                else c[ilinear%0x10] = '.';
            }
        }
        PRINTF(" ");
        if (ilinear % 0x10 == 7 && ilinear >= start && ilinear < end) PRINTF("\b-");
        if ((ilinear + 1) % 0x10 == 0) {
            PRINTF("  %s\n",c);
        }
        if (ilinear == 0xffffffff) {
            break;
        }
    }
    xdlin = ilinear;
}
static void xd() {
    uint32_t count;
    if (narg == 1) {
        xdprint(xdlin, 0x80);
    } else if (narg == 2) {
        xdlin = scannubit32(arg[1]);
        if (nErrPos) {
            return;
        }
        xdprint(xdlin, 0x80);
    } else if (narg == 3) {
        xdlin = scannubit32(arg[1]);
        count = scannubit32(arg[2]);
        if (nErrPos) {
            return;
        }
        xdprint(xdlin, count);
    } else {
        seterr(3);
    }
}
/* enter */
static void xe() {
    size_t i;
    uint8_t val;
    uint32_t linear;
    char s[0x100];
    if (narg == 1) {
        seterr(0);
    } else if (narg == 2) {
        linear = scannubit32(arg[1]);
        if (nErrPos) {
            return;
        }
        if (deviceConnectCpuReadLinear(linear, (void *)(&val), 1)) {
            PRINTF("debug: fail to read from L%08X.\n", linear);
            return;
        }
        PRINTF("L%08X  %02X.", linear, val);
        FGETS(s, 0x100, stdin);
        utilsLowerStr(s);
        val = scannubit8(s);
        if (nErrPos) {
            return;
        }
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            if (deviceConnectCpuWriteLinear(linear, (void *)(&val), 1)) {
                PRINTF("debug: fail to write to L%08X.\n", linear);
            }
        }
    } else if (narg > 2) {
        linear = scannubit32(arg[1]);
        if (nErrPos) {
            return;
        }
        for (i = 2; i < narg; ++i) {
            val = scannubit8(arg[i]);
            if (!nErrPos) {
                if (deviceConnectCpuWriteLinear(linear, (void *)(&val), 1)) {
                    PRINTF("debug: fail to write to L%08X.\n", linear);
                    return;
                }
            } else {
                break;
            }
            linear++;
        }
    }
}
/* fill */
static void xf() {
    uint8_t val;
    size_t i, j, count, bcount;
    uint32_t linear;
    if (narg < 4) {
        seterr(narg - 1);
    } else {
        linear = scannubit32(arg[1]);
        if (nErrPos) {
            return;
        }
        count = scannubit32(arg[2]);
        if (nErrPos) {
            return;
        }
        bcount = narg - 3;
        for (i = 0, j = 0; i < count; ++i, ++j) {
            val = scannubit8(arg[j % bcount + 3]);
            if (nErrPos) {
                return;
            }
            if (deviceConnectCpuWriteLinear((uint32_t)(linear + i), (void *)(&val), 1)) {
                PRINTF("debug: fail to write to L%08X.\n", (uint32_t)(linear + i));
                return;
            }
        }
    }
}
/* go */
static void xg() {
    size_t i, count = 0;
    uint32_t linear;
    if (device.flagRun) {
        PRINTF("NXVM is already running.\n");
        return;
    }
    switch (narg) {
    case 1:
        count = 0;
        g();
        break;
    case 2:
        linear = scannubit32(arg[1]);
        count = 1;
        break;
    case 3:
        linear = scannubit32(arg[1]);
        count = scannubit32(arg[2]);
        break;
    default:
        seterr(narg - 1);
        break;
    }
    if (nErrPos) {
        return;
    }
    for (i = 0; i < count; ++i) {
        deviceConnectDebugSetBreak32(linear);
        machineResume();
        while (device.flagRun) {
            utilsSleep(10);
        }
        PRINTF("%d instructions executed before the break point.\n",
               deviceConnectDebugGetBreakCount());
        xrprintreg();
    }
    deviceConnectDebugClearBreak32();
}
/* move */
static void xm() {
    uint8_t val;
    size_t i;
    uint32_t lin1, lin2, count;
    if (narg != 4) {
        seterr(narg - 1);
    } else {
        lin1 = scannubit32(arg[1]);
        if (nErrPos) {
            return;
        }
        lin2 = scannubit32(arg[2]);
        if (nErrPos) {
            return;
        }
        count = scannubit32(arg[3]);
        if (nErrPos) {
            return;
        }
        for (i = 0; i < count; ++i) {
            if (deviceConnectCpuReadLinear((uint32_t)(lin1 + i), (void *)(&val), 1)) {
                PRINTF("debug: fail to read from L%08X.\n", lin1 + i);
                return;
            }
            if (deviceConnectCpuWriteLinear((uint32_t)(lin2 + i), (void *)(&val), 1)) {
                PRINTF("debug: fail to write to L%08X.\n", lin2 + i);
                return;
            }
        }
    }
}
/* search */
static void xs() {
    size_t i, count, bcount;
    uint32_t linear;
    uint8_t val, mem[256], line[256];
    if (narg < 4) {
        seterr(narg - 1);
    } else {
        linear = scannubit32(arg[1]);
        if (nErrPos) {
            return;
        }
        count = scannubit32(arg[2]);
        if (nErrPos) {
            return;
        }
        addrparse(_ds, arg[1]);
        bcount = narg - 3;
        for (i = 0; i < bcount; ++i) {
            val = scannubit8(arg[i + 3]);
            if (nErrPos) {
                return;
            }
            line[i] = val;
        }
        for (i = 0; i < count; ++i) {
            if (deviceConnectCpuReadLinear((uint32_t)(linear + i), (void *) mem, (uint8_t) bcount)) {
                PRINTF("debug: fail to read from L%08X.\n", linear + i);
                return;
            }
            if (!MEMCMP((void *) mem, (void *) line, bcount)) {
                PRINTF("L%08X\n", linear + i);
            }
        }
    }
}
/* trace */
static void xt() {
    size_t i;
    uint32_t count;
    if (device.flagRun) {
        PRINTF("NXVM is already running.\n");
        return;
    }
    switch (narg) {
    case 1:
        count = 1;
        break;
    case 2:
        count = scannubit32(arg[1]);
        break;
    default:
        seterr(narg - 1);
        break;
    }
    if (nErrPos) {
        return;
    }
    if (count < 0x0100) {
        for (i = 0; i < count; ++i) {
            deviceConnectDebugSetTrace(1);
            machineResume();
            while (device.flagRun) {
                utilsSleep(10);
            }
            devicePrintCpuMem();
            xrprintreg();
            if (i != count - 1) {
                PRINTF("\n");
            }
        }
    } else {
        deviceConnectDebugSetTrace(count);
        machineResume();
        while (device.flagRun) {
            utilsSleep(10);
        }
        devicePrintCpuMem();
        xrprintreg();
    }
    deviceConnectDebugClearTrace();
}
/* register */
static void xrscanreg() {
    uint32_t value;
    char s[0x100];
    if (!STRCMP(arg[1], "eax")) {
        PRINTF("EAX ");
        PRINTF("%08X", _eax);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit32(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _eax = value;
        }
    } else if (!STRCMP(arg[1], "ecx")) {
        PRINTF("ECX ");
        PRINTF("%08X", _ecx);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit32(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _ecx = value;
        }
    } else if (!STRCMP(arg[1], "edx")) {
        PRINTF("EDX ");
        PRINTF("%08X", _edx);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit32(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _edx = value;
        }
    } else if (!STRCMP(arg[1], "ebx")) {
        PRINTF("EBX ");
        PRINTF("%08X", _ebx);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit32(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _ebx = value;
        }
    } else if (!STRCMP(arg[1], "esp")) {
        PRINTF("ESP ");
        PRINTF("%08X", _esp);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit32(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _esp = value;
        }
    } else if (!STRCMP(arg[1], "ebp")) {
        PRINTF("EBP ");
        PRINTF("%08X", _ebp);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit32(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _ebp = value;
        }
    } else if (!STRCMP(arg[1], "esi")) {
        PRINTF("ESI ");
        PRINTF("%08X", _esi);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit32(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _esi = value;
        }
    } else if (!STRCMP(arg[1], "edi")) {
        PRINTF("EDI ");
        PRINTF("%08X", _edi);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit32(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _edi = value;
        }
    } else if (!STRCMP(arg[1], "eip")) {
        PRINTF("EIP ");
        PRINTF("%08X", _eip);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit32(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _eip = value;
        }
    } else if (!STRCMP(arg[1], "eflags")) {
        PRINTF("EFLAGS ");
        PRINTF("%08X", _eflags);
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit32(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _eflags = value;
        }
    } else if (!STRCMP(arg[1], "es")) {
        devicePrintCpuSreg();
        PRINTF(":");
        FGETS(s, 0x100, stdin);
        value = scannubit16(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            if (deviceConnectCpuLoadES((uint16_t) value)) {
                PRINTF("debug: fail to load es from %04X\n", (uint16_t) value);
            }
        }
    } else if (!STRCMP(arg[1], "cs")) {
        devicePrintCpuSreg();
        PRINTF(":");
        FGETS(s, 0x100, stdin);
        value = scannubit16(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            if (deviceConnectCpuLoadCS((uint16_t) value)) {
                PRINTF("debug: fail to load cs from %04X\n", (uint16_t) value);
            }
        }
    }  else if (!STRCMP(arg[1], "ss")) {
        devicePrintCpuSreg();
        PRINTF(":");
        FGETS(s, 0x100, stdin);
        value = scannubit16(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            if (deviceConnectCpuLoadSS((uint16_t) value)) {
                PRINTF("debug: fail to load ss from %04X\n", (uint16_t) value);
            }
        }
    } else if (!STRCMP(arg[1], "ds")) {
        devicePrintCpuSreg();
        PRINTF(":");
        FGETS(s, 0x100, stdin);
        value = scannubit16(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            if (deviceConnectCpuLoadDS((uint16_t) value)) {
                PRINTF("debug: fail to load ds from %04X\n", (uint16_t) value);
            }
        }
    } else if (!STRCMP(arg[1], "fs")) {
        devicePrintCpuSreg();
        PRINTF(":");
        FGETS(s, 0x100, stdin);
        value = scannubit16(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            if (deviceConnectCpuLoadFS((uint16_t) value)) {
                PRINTF("debug: fail to load fs from %04X\n", (uint16_t) value);
            }
        }
    } else if (!STRCMP(arg[1], "gs")) {
        devicePrintCpuSreg();
        PRINTF(":");
        FGETS(s, 0x100, stdin);
        value = scannubit16(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            if (deviceConnectCpuLoadGS((uint16_t) value)) {
                PRINTF("debug: fail to load gs from %04X\n", (uint16_t) value);
            }
        }
    } else if (!STRCMP(arg[1], "cr0")) {
        PRINTF("CR0 ");
        PRINTF("%08X", _cr(0));
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit32(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _cr(0) = value;
        }
    } else if (!STRCMP(arg[1], "cr1")) {
        PRINTF("CR1 ");
        PRINTF("%08X", _cr(1));
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit32(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _cr(1) = value;
        }
    } else if (!STRCMP(arg[1], "cr2")) {
        PRINTF("CR2 ");
        PRINTF("%08X", _cr(2));
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit32(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _cr(2) = value;
        }
    } else if (!STRCMP(arg[1], "cr3")) {
        PRINTF("CR3 ");
        PRINTF("%08X", _cr(3));
        PRINTF("\n:");
        FGETS(s, 0x100, stdin);
        value = scannubit32(s);
        if (s[0] != '\0' && s[0] != '\n' && !nErrPos) {
            _cr(3) = value;
        }
    } else {
        PRINTF("br Error\n");
    }
}
static void xr() {
    if (narg == 1) {
        xrprintreg();
    } else if (narg == 2) {
        xrscanreg();
    } else {
        seterr(2);
    }
}
/* unassemble */
static void xuprint(uint32_t linear, uint8_t count) {
    uint32_t len = 0;
    size_t i;
    for (i = 0; i < count; ++i) {
        len = xuprintins(linear);
        if (!len) {
            break;
        }
        linear += len;
    }
    xulin = linear;
}
static void xu() {
    uint32_t count;
    if (narg == 1) {
        xuprint(xulin, 10);
    } else if (narg == 2) {
        xulin = scannubit32(arg[1]);
        if (nErrPos) {
            return;
        }
        xuprint(xulin, 0x10);
    } else if (narg == 3) {
        xulin = scannubit32(arg[1]);
        count = scannubit32(arg[2]);
        if (nErrPos) {
            return;
        }
        xuprint(xulin, count);
    } else {
        seterr(3);
    }
}
/* watch */
static void xw() {
    uint32_t linear;
    switch (narg) {
    case 1:
        devicePrintCpuWatch();
        break;
    case 2:
        switch (arg[1][0]) {
        case 'r':
            deviceConnectCpuClearWR();
            PRINTF("Watch-read point removed.\n");
            break;
        case 'w':
            deviceConnectCpuClearWW();
            PRINTF("Watch-write point removed.\n");
            break;
        case 'e':
            deviceConnectCpuClearWE();
            PRINTF("Watch-exec point removed.\n");
            break;
        case 'u':
            deviceConnectCpuClearWR();
            deviceConnectCpuClearWW();
            deviceConnectCpuClearWE();
            PRINTF("All watch points removed.\n");
            break;
        default:
            seterr(1);
            break;
        }
        break;
    case 3:
        switch (arg[1][0]) {
        case 'r':
            linear = scannubit32(arg[2]);
            deviceConnectCpuSetWR(linear);
            break;
        case 'w':
            linear = scannubit32(arg[2]);
            deviceConnectCpuSetWW(linear);
            break;
        case 'e':
            linear = scannubit32(arg[2]);
            deviceConnectCpuSetWE(linear);
            break;
        default:
            seterr(2);
            break;
        }
        break;
    default:
        seterr(narg - 1);
        break;
    }
}
static void xhelp() {
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
static void x() {
    size_t i;
    arg[narg] = arg[0];
    for (i = 1; i < narg; ++i) {
        arg[i - 1] = arg[i];
    }
    arg[narg - 1] = arg[narg];
    arg[narg] = NULL;
    narg--;
    if (!STRCMP(arg[0], "\?")) {
        xhelp();
    } else if (!STRCMP(arg[0], "a")) {
        xa();
    } else if (!STRCMP(arg[0], "c")) {
        xc();
    } else if (!STRCMP(arg[0], "d")) {
        xd();
    } else if (!STRCMP(arg[0], "e")) {
        xe();
    } else if (!STRCMP(arg[0], "f")) {
        xf();
    } else if (!STRCMP(arg[0], "g")) {
        xg();
    } else if (!STRCMP(arg[0], "m")) {
        xm();
    } else if (!STRCMP(arg[0], "r")) {
        xr();
    } else if (!STRCMP(arg[0], "s")) {
        xs();
    } else if (!STRCMP(arg[0], "t")) {
        xt();
    } else if (!STRCMP(arg[0], "u")) {
        xu();
    } else if (!STRCMP(arg[0], "w")) {
        xw();
    } else if (!STRCMP(arg[0], "reg")) {
        xrprintreg();
    } else if (!STRCMP(arg[0], "sreg")) {
        devicePrintCpuSreg();
    } else if (!STRCMP(arg[0], "creg")) {
        devicePrintCpuCreg();
    } else {
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
    if (strlen(arg[narg - 1]) != 1) {
        arg[narg] = arg[narg - 1] + 1;
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
    switch (arg[0][0]) {
    case '\?':
        help();
        break;
    case 'a':
        a();
        break;
    case 'c':
        c();
        break;
    case 'd':
        d();
        break;
    case 'e':
        e();
        break;
    case 'f':
        f();
        break;
    case 'g':
        g();
        break;
    case 'h':
        h();
        break;
    case 'i':
        i();
        break;
    case 'l':
        l();
        break;
    case 'm':
        m();
        break;
    case 'n':
        n();
        break;
    case 'o':
        o();
        break;
    case 'q':
        q();
        break;
    case 'r':
        r();
        break;
    case 's':
        s();
        break;
    case 't':
        t();
        break;
    case 'u':
        u();
        break;
    case 'v':
        v();
        break;
    case 'w':
        w();
        break;
    case 'x':
        x();
        break;
    default:
        seterr(0);
        break;
    }
}

void debugMain() {
    size_t i;
    strFileName[0] = '\0';
    asmSegRec = uasmSegRec = _cs;
    asmPtrRec = uasmPtrRec = _ip;
    dumpSegRec = _ds;
    dumpPtrRec = (uint16_t)(_ip) / 0x10 * 0x10;
    xalin = 0;
    xdlin = 0;
    xulin = deviceConnectCpuGetCsBase() + _eip;
    arg = (char **) MALLOC(DEBUG_MAXNARG * sizeof(char *));
    flagExit = 0;
    while (!flagExit) {
        fflush(stdin);
        PRINTF("-");
        FGETS(strCmdBuff, 0x100, stdin);
        parse();
        exec();
        if (nErrPos) {
            for (i = 0; i < nErrPos; ++i) {
                PRINTF(" ");
            }
            PRINTF("^ Error\n");
        }
    }
    FREE((void *) arg);
}
