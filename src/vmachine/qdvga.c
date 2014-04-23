#include "../system/vapi.h"
#include "vcpu.h"
#include "qdvga.h"

#include "memory.h"

#define _ah (vcpu.ah)
#define _al (vcpu.al)
#define _bh (vcpu.bh)
#define _bl (vcpu.bl)
#define _ch (vcpu.ch)
#define _cl (vcpu.cl)
#define _dh (vcpu.dh)
#define _dl (vcpu.dl)

t_vga qdvga;

static void ClearTextMemory()
{ 
	memset((void*)qdvgaGetTextMemAddrPageCur, 0x00, qdvgaGetPageSize);
}

void qdvgaSetDisplayMode()
{
	switch (vcpu.al) {
	case 0x00:
		qdvga.rowsize = 0x28; // 40
		qdvga.colsize = 0x19; // 25
		qdvga.color   = 0x00;
		qddispSetScreen();
		break;
	case 0x01:
		qdvga.rowsize = 0x28; // 40
		qdvga.colsize = 0x19; // 25
		qdvga.color   = 0x01;
		qddispSetScreen();
		break;
	case 0x02:
	case 0x07:
		qdvga.rowsize = 0x50; // 80
		qdvga.colsize = 0x19; // 25
		qdvga.color   = 0x00;
		qddispSetScreen();
		break;
	case 0x03:
		qdvga.rowsize = 0x50; // 80
		qdvga.colsize = 0x19; // 25
		qdvga.color   = 0x01;
		qddispSetScreen();
		break;
	default:
		break;
	}
	qdvga.vgamode = _al;
	ClearTextMemory();
}
void qdvgaSetCursorShape() {
	qdvgaVarCursorBottom = _cl & 0x0f;
	qdvgaVarCursorTop    = _ch & 0x0f;
}
void qdvgaSetCursorPos()
{
	if (_bh < QDVGA_COUNT_MAX_PAGE) {
		qdvga.cursor[_bh].x       = _dl;
		qdvga.cursor[_bh].y       = _dh;
		qdvgaVarCursorPosRow(_bh) = _dl;
		qdvgaVarCursorPosCol(_bh) = _dh;
	}
}
void qdvgaGetCursorPos()
{
	if (vcpu.bh < QDVGA_COUNT_MAX_PAGE) {
		qdvgaVarCursorPosRow(_bh) = qdvga.cursor[_bh].x;
		qdvgaVarCursorPosCol(_bh) = qdvga.cursor[_bh].y;
		_dl = qdvgaVarCursorPosRow(_bh);
		_dh = qdvgaVarCursorPosCol(_bh);
		_cl = qdvgaVarCursorBottom;
		_ch = qdvgaVarCursorTop;
	}
}
void qdvgaSetDisplayPage()
{
	qdvgaVarPageNum = _al;
}
void qdvgaScrollUp()
{
	t_nsbitcc i, j;
	if (!_al) {
		ClearTextMemory();
		return;
	}
	for (i = 0x00;i < _dh - _ch - _al + 1;++i) {
		memcpy((void *)(qdvgaGetTextMemAddrPageCur +
		                ((_ch + i) * qdvga.rowsize + _cl) * 2),
		       (void *)(qdvgaGetTextMemAddrPageCur +
		                ((_ch + i + _al) * qdvga.rowsize + _cl) * 2),
		       (_dl - _cl + 1) * 2);
	}
	for(i = 0x00;i < _dh - _ch - _al + 1; ++i) {
		for(j = 0x00; j < _dl - _cl + 1; ++j) {
			*(t_nubit8 *)(qdvgaGetTextMemAddrPageCur +
				((_ch + i + _al) * qdvga.rowsize + _cl ) * 2 + 0) = ' ';
			*(t_nubit8 *)(qdvgaGetTextMemAddrPageCur +
				((_ch + i + _al) * qdvga.rowsize + _cl ) * 2 + 1) = _bh;//bh决定了空白行的属性
		}
	}
}
void qdvgaScrollDown()
{
	t_nsbitcc i, j;
	if (!_al) {
		ClearTextMemory();
		return;
	}
	for (i = 0x00;i < _dh - _ch - _al + 1;++i) {
		memcpy((void *)(qdvgaGetTextMemAddrPageCur +
		                ((_ch + i + _al) * qdvga.rowsize + _cl) * 2),
		       (void *)(qdvgaGetTextMemAddrPageCur +
		                ((_ch + i) * qdvga.rowsize + _cl) * 2),
		       (_dl - _cl + 1) * 2);
	}
	for(i = 0x00;i < _dh - _ch - _al + 1;++i) {
		for(j = 0x00;j < _dl - _cl + 1;++j) {
			*(t_nubit8 *)(qdvgaGetTextMemAddrPageCur +
				((_ch + i) * qdvga.rowsize + _cl ) * 2 + 0) = ' ';
			*(t_nubit8 *)(qdvgaGetTextMemAddrPageCur +
				((_ch + i) * qdvga.rowsize + _cl ) * 2 + 1) = _bh;//bh决定了空白行的属性
		}
	}
}
void qdvgaGetCharProp()
{
/* TODO: verify */
	_al = *(t_nubit8 *)(
		qdvgaGetTextMemAddrPage(_bh) + 
		(qdvga.cursor[_bh].x * qdvga.rowsize + qdvga.cursor[_bh].y) * 2 + 0);
	_ah = *(t_nubit8 *)(
		qdvgaGetTextMemAddrPage(_bh) + 
		(qdvga.cursor[_bh].x * qdvga.rowsize + qdvga.cursor[_bh].y) * 2 + 1);

}
void qdvgaDisplayCharProp(t_nubit16 count)
{}
void qdvgaDisplayChar() {}
//void qdvgaSetPalette() {}
//void qdvgaDisplayPixel() {}
//void qdvgaGetPixel() {}
void qdvgaGetAdapterStatus() {}
void qdvgaGenerateChar() {}
void qdvgaGetAdapterInfo()
{
	_bh = 0x00;
	_bl = 0x03;
	_cl = 0x07;
//	_ch = 0x??;
}
void qdvgaDisplayStr() {}

void qdvgaInit()
{
//	t_nubit8 i;
	memset(&qdvga, 0x00, sizeof(t_vga));
	qdvga.color   = 0x01;
	qdvga.colsize = 0x19; // 25
	qdvga.rowsize = 0x50; // 80
	qdvgaVarPageNum = 0x00;
	qdvga.vgamode = 0x03;
/*	for (i = 0x00;i < QDVGA_COUNT_MAX_PAGE; ++i) {
		qdvga.cursor[i].x      = 0x00;
		qdvga.cursor[i].y      = 0x00;
		qdvga.cursor[i].visual = 0x01;
		qdvga.cursor[i].top    = 0x05;
		qdvga.cursor[i].bottom = 0x00;
	}
	qdvga.cursor[i].y      = 0x05;
	qdvga.cursor[i].top    = 0x06;
	qdvga.cursor[i].bottom = 0x07;*/
	qdvgaVarCursorBottom = 0x07;
	qdvgaVarCursorTop    = 0x06;
	memset((void *)qdvgaGetTextMemAddr, 0x00, QDVGA_SIZE_TEXT_MEMORY);
	// TODO: init screen: setScreen_Widht_Height(charWidth, charHeight, true);
}
void qdvgaFinal()
{}