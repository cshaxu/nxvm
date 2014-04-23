/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "record.h"

#ifndef VGLOBAL_BOCHS
#include "../vapi.h"
#include "dasm.h"
#else
#include "../vcpuapi.h"
#define vapiPrint vcpuapiPrint
#define FOPEN fopen
#endif

/* Record */
t_record vrecord;

#define _rec (vrecord.rec[(i + vrecord.start) % RECORD_SIZE])
#define _recpu     (_rec.rcpu)
#define _restmt    (_rec.dstmt)
#define _rec_of    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_OF))
#define _rec_sf    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_SF))
#define _rec_zf    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_ZF))
#define _rec_cf    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_CF))
#define _rec_af    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_AF))
#define _rec_pf    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_PF))
#define _rec_df    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_DF))
#define _rec_tf    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_TF))
#define _rec_if    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_IF))
#define _rec_vm    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_VM))
#define _rec_rf    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_RF))
#define _rec_nt    (GetBit(_rec.rcpu.eflags, VCPU_EFLAGS_NT))
#define _rec_ptr_last ((vrecord.start + vrecord.size) % RECORD_SIZE)

#define _expression "%scs:eip=%04x:%08x(L%08x) opcode=%02x %02x %02x %02x %02x %02x %02x %02x \
ss:esp=%04x:%08x(L%08x) stack=%04x %04x %04x %04x \
eax=%08x ecx=%08x edx=%08x ebx=%08x ebp=%08x esi=%08x edi=%08x ds=%04x es=%04x fs=%04x gs=%04x \
eflags=%08x %s %s %s %s %s %s %s %s %s %s %s %s \
| cs:eip=%04x:%08x(L%08x) %s"
#define _printexp \
do { \
	fprintf(vrecord.fp, "%04x:%08x(L%08x)\n", _recpu.cs.selector, _recpu.eip, _recpu.cs.base + _recpu.eip);\
/*	fprintf(vrecord.fp, _expression, \
	_rec.svcextl ? "* " : "", \
	_recpu.cs.selector, _recpu.eip, _recpu.cs.base + _recpu.eip, \
	GetMax8(_rec.opcode >> 0), GetMax8(_rec.opcode >> 8), \
	GetMax8(_rec.opcode >> 16), GetMax8(_rec.opcode >> 24), \
	GetMax8(_rec.opcode >> 32), GetMax8(_rec.opcode >> 40), \
	GetMax8(_rec.opcode >> 48), GetMax8(_rec.opcode >> 56), \
	_recpu.ss.selector, _recpu.esp, _recpu.ss.base + _recpu.esp, \
	GetMax16(_rec.opcode >> 0), GetMax16(_rec.opcode >> 16), \
	GetMax16(_rec.opcode >> 32), GetMax16(_rec.opcode >> 48), \
	_recpu.eax,_recpu.ecx,_recpu.edx,_recpu.ebx, \
	_recpu.ebp,_recpu.esi,_recpu.edi, \
	_recpu.ds.selector,_recpu.es.selector, \
	_recpu.fs.selector,_recpu.gs.selector, \
	_recpu.eflags, \
	_rec_of ? "OF" : "of", \
	_rec_sf ? "SF" : "sf", \
	_rec_zf ? "ZF" : "zf", \
	_rec_cf ? "CF" : "cf", \
	_rec_af ? "AF" : "af", \
	_rec_pf ? "PF" : "pf", \
	_rec_df ? "DF" : "df", \
	_rec_if ? "IF" : "if", \
	_rec_tf ? "TF" : "tf", \
	_rec_vm ? "VM" : "vm", \
	_rec_rf ? "RF" : "rf", \
	_rec_nt ? "NT" : "nt", \
	_recpu.cs.selector, _recpu.eip, _recpu.cs.base + _recpu.eip, _restmt); \
	for (j = strlen(_restmt);j < 0x21;++j) \
		fprintf(vrecord.fp, " "); \
	for (j = 0;j < _rec.msize;++j) \
		fprintf(vrecord.fp, "[%c:L%08x/%1d/%08x] ", \
			_rec.mem[j].flagwrite ? 'W' : 'R', _rec.mem[j].linear, \
			_rec.mem[j].byte, _rec.mem[j].data); \
	fprintf(vrecord.fp, "\n");*/\
} while (0)

void recordNow(const t_string fname)
{
	vrecord.flagnow = 1;
	if (!fname) {
		vapiPrint("ERROR:\tinvalid file name.\n");
		return;
	}
	vrecord.fp = FOPEN(fname, "w");
	if (!vrecord.fp) {
		vapiPrint("ERROR:\tcannot write dump file.\n");
		return;
	}
}
void recordDump(const t_string fname)
{
	t_nubitcc i = 0, j;
	if (vrecord.fp) fclose(vrecord.fp);
	vrecord.fp = FOPEN(fname, "w");
	if (!vrecord.fp) {
		vapiPrint("ERROR:\tcannot write dump file.\n");
		return;
	}
	if (!vrecord.size) {
		vapiPrint("ERROR:\tno record to dump.\n");
		return;
	}
	while (i < vrecord.size) {
		for (j = 0;j < strlen(_restmt);++j)
			if (_restmt[j] == '\n') _restmt[j] = ' ';
		_printexp;
		++i;
	}
	vapiPrint("Record dumped to '%s'.\n", fname);
	fclose(vrecord.fp);
}
void recordInit()
{
	vrecord.start = 0;
	vrecord.size = 0;
	if (vrecord.flagnow) {
		if (!vrecord.fp) {
			vapiPrint("ERROR:\tcannot write dump file.\n");
		} else {
			vapiPrint("Record dumping began.\n");
		}
	} else {
		if (vrecord.fp) fclose(vrecord.fp);
		vrecord.fp = NULL;
		vapiPrint("Record began.\n");
	}
}
void recordExec(t_cpurec *rcpurec)
{
	t_nubitcc i, j;
#if RECORD_SELECT_FIRST == 1
	if (vrecord.size == RECORD_SIZE) {
		vmachine.flagrecord = 0x00;
		return;
	}
#endif
	if (rcpurec->rcpu.flaghalt) return;
	if (rcpurec->linear == vrecord.rec[(vrecord.start + vrecord.size - 1) % RECORD_SIZE].linear) return;
#ifndef VGLOBAL_BOCHS
	dasm(rcpurec->dstmt, rcpurec->rcpu.cs.selector, rcpurec->rcpu.ip, 0x00);
#else
	rcpurec->dstmt[0] = 0;
#endif
	vrecord.rec[_rec_ptr_last] = (*rcpurec);
	if (vrecord.flagnow) {
		i = vrecord.size;
		for (j = 0;j < strlen(_restmt);++j)
			if (_restmt[j] == '\n') _restmt[j] = ' ';
		_printexp;
	}
	if (vrecord.size == RECORD_SIZE)
		vrecord.start++;
	else
		vrecord.size++;
}
void recordFinal()
{
	if (vrecord.flagnow && vrecord.fp) {
		fclose(vrecord.fp);
		vapiPrint("Record dumping ended.\n");
	} else {
		vapiPrint("Record ended.\n");
	}
	vrecord.flagnow = 0;
	vrecord.fp = NULL;
}
