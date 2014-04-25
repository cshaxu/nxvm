/* Copyright 2012-2014 Neko. */

/* RECORD implements cpu instruction recorder. */

#include "../vapi.h"

#include "record.h"
#include "dasm32.h"

#ifndef VGLOBAL_BOCHS
#include "../vmachine.h"
#else
#include "../vcpu.h"
#endif

/* Record */
t_record vrecord;

#define _rec_ptr_last ((vrecord.start + vrecord.size) % RECORD_SIZE)
#define _recpu (vrecord.recpu[(i + vrecord.start) % RECORD_SIZE])
#define _recpu_of    (GetBit(_recpu.eflags, VCPU_EFLAGS_OF))
#define _recpu_sf    (GetBit(_recpu.eflags, VCPU_EFLAGS_SF))
#define _recpu_zf    (GetBit(_recpu.eflags, VCPU_EFLAGS_ZF))
#define _recpu_cf    (GetBit(_recpu.eflags, VCPU_EFLAGS_CF))
#define _recpu_af    (GetBit(_recpu.eflags, VCPU_EFLAGS_AF))
#define _recpu_pf    (GetBit(_recpu.eflags, VCPU_EFLAGS_PF))
#define _recpu_df    (GetBit(_recpu.eflags, VCPU_EFLAGS_DF))
#define _recpu_tf    (GetBit(_recpu.eflags, VCPU_EFLAGS_TF))
#define _recpu_if    (GetBit(_recpu.eflags, VCPU_EFLAGS_IF))
#define _recpu_vm    (GetBit(_recpu.eflags, VCPU_EFLAGS_VM))
#define _recpu_rf    (GetBit(_recpu.eflags, VCPU_EFLAGS_RF))
#define _recpu_nt    (GetBit(_recpu.eflags, VCPU_EFLAGS_NT))

#define _expression "%scs:eip=%04x:%08x(L%08x) ss:esp=%04x:%08x(L%08x) \
eax=%08x ecx=%08x edx=%08x ebx=%08x ebp=%08x esi=%08x edi=%08x ds=%04x es=%04x fs=%04x gs=%04x \
eflags=%08x %s %s %s %s %s %s %s %s %s %s %s %s \
| cs:eip=%04x:%08x(L%08x) "
#define _printexp \
do { \
/*	fprintf(vrecord.fp, "%04x:%08x(L%08x)\n", _recpu.cs.selector, _recpu.eip, _recpu.cs.base + _recpu.eip);*/\
	fprintf(vrecord.fp, _expression, \
	_recpu.svcextl ? "* " : "", \
	_recpu.cs.selector, _recpu.eip, _recpu.cs.base + _recpu.eip, \
	_recpu.ss.selector, _recpu.esp, _recpu.ss.base + _recpu.esp, \
	_recpu.eax,_recpu.ecx,_recpu.edx,_recpu.ebx, \
	_recpu.ebp,_recpu.esi,_recpu.edi, \
	_recpu.ds.selector,_recpu.es.selector, \
	_recpu.fs.selector,_recpu.gs.selector, \
	_recpu.eflags, \
	_recpu_of ? "OF" : "of", \
	_recpu_sf ? "SF" : "sf", \
	_recpu_zf ? "ZF" : "zf", \
	_recpu_cf ? "CF" : "cf", \
	_recpu_af ? "AF" : "af", \
	_recpu_pf ? "PF" : "pf", \
	_recpu_df ? "DF" : "df", \
	_recpu_if ? "IF" : "if", \
	_recpu_tf ? "TF" : "tf", \
	_recpu_vm ? "VM" : "vm", \
	_recpu_rf ? "RF" : "rf", \
	_recpu_nt ? "NT" : "nt", \
	_recpu.reccs, _recpu.receip, _recpu.linear); \
	if (_recpu.oplen) { \
		t_cpu oldcpu = vcpu; \
		vcpu = _recpu; \
		_recpu.oplen = dasm32(_recpu.stmt, (t_vaddrcc) _recpu.opcodes); \
		vcpu = oldcpu; \
		for (j = 0;j < _recpu.oplen;++j) \
			fprintf(vrecord.fp, "%02X", _recpu.opcodes[j]); \
	} else { \
		SPRINTF(_recpu.stmt, "<ERROR>"); \
	} \
	for (j = _recpu.oplen;j < 8;++j) fprintf(vrecord.fp, "  "); \
	fprintf(vrecord.fp, "%s ", _recpu.stmt); \
	for (j = strlen(_recpu.stmt);j < 40;++j) \
		fprintf(vrecord.fp, " "); \
	for (j = 0;j < _recpu.msize;++j) \
		fprintf(vrecord.fp, "[%c:L%08x/%1d/%016llx] ", \
			_recpu.mem[j].flagwrite ? 'W' : 'R', _recpu.mem[j].linear, \
			_recpu.mem[j].byte, _recpu.mem[j].data); \
	fprintf(vrecord.fp, "\n");\
} while (0)

void recordNow(const t_strptr fname) {
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

void recordDump(const t_strptr fname) {
	t_nubitcc i = 0, j;
	if (vrecord.fp) {
		fclose(vrecord.fp);
	}
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
		for (j = 0;j < strlen(_recpu.stmt);++j) {
			if (_recpu.stmt[j] == '\n') {
				_recpu.stmt[j] = ' ';
			}
		}
		_printexp;
		++i;
	}
	vapiPrint("Record dumped to '%s'.\n", fname);
	fclose(vrecord.fp);
}

void recordExec(t_cpu *rcpu) {
	t_nubitcc i, j;
#if RECORD_SELECT_FIRST == 1
	if (vrecord.size == RECORD_SIZE) {
		vrecord.flagrecord = 0;
		return;
	}
#endif
	if (rcpu->flaghalt) {
		return;
	}
	if (rcpu->linear == vrecord.recpu[(vrecord.start + vrecord.size - 1) % RECORD_SIZE].linear) {
		return;
	}

	vrecord.recpu[_rec_ptr_last] = (*rcpu);

	if (vrecord.flagnow) {
		i = vrecord.size;
		for (j = 0;j < strlen(_recpu.stmt);++j) {
			if (_recpu.stmt[j] == '\n') {
				_recpu.stmt[j] = ' ';
			}
		}
		_printexp;
	}
	if (vrecord.size == RECORD_SIZE) {
		vrecord.start++;
	} else {
		vrecord.size++;
	}
}

static void init() {
	memset(&vrecord, 0x00, sizeof(t_record));
}

static void reset() {}

static void refresh() {
#ifndef VGLOBAL_BOCHS
	if (!vrecord.flagrecord) {
		return;
	}
	if (!vrecord.flagready) {
		/* prepare the recorder */
		vrecord.start = 0;
		vrecord.size = 0;
		if (vrecord.flagnow) {
			if (!vrecord.fp) {
				vapiPrint("ERROR:\tcannot write dump file.\n");
				vrecord.flagrecord = 0;
				vrecord.flagready = 0;
			} else {
				vapiPrint("Record dumping began.\n");
				vrecord.flagready = 1;
			}
		} else {
			if (vrecord.fp) fclose(vrecord.fp);
			vrecord.fp = NULL;
			vapiPrint("Record began.\n");
			vrecord.flagready = 1;
		}
	}
	if (vrecord.flagready) {
		recordExec(&vcpu);
		if (!vmachine.flagrun) {
			/* terminate the recorder */
			if (vrecord.flagnow && vrecord.fp) {
				fclose(vrecord.fp);
				vapiPrint("Record dumping ended.\n");
			} else {
				vapiPrint("Record ended.\n");
			}
			vrecord.flagrecord = 0;
			vrecord.flagnow = 0;
			vrecord.fp = NULL;
			vrecord.flagready = 0;
		}
	}
#endif
}

static void final() {}

void recordRegister() {
	vmachine.deviceTable[VMACHINE_DEVICE_INIT][vmachine.numDevices] = (t_faddrcc) init;
	vmachine.deviceTable[VMACHINE_DEVICE_RESET][vmachine.numDevices] = (t_faddrcc) reset;
	vmachine.deviceTable[VMACHINE_DEVICE_REFRESH][vmachine.numDevices] = (t_faddrcc) refresh;
	vmachine.deviceTable[VMACHINE_DEVICE_FINAL][vmachine.numDevices] = (t_faddrcc) final;
	vmachine.numDevices++;
}
