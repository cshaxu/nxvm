#include "vapi.h"
#include "vcpu.h"
#include "qdvga.h"

#include "memory.h"

t_vga qdvga;

static void ClearTextMemory()
{ 
	memset((void*)qdvgaGetTextMemAddrPageCur, 0x00, qdvgaGetPageSize);
}
static void CursorBackward(t_nubit8 page)
{
	if (qdvga.cursor[page].y)
		qdvga.cursor[page].y--;
	else if (qdvga.cursor[page].x) {
		qdvga.cursor[page].y = qdvga.rowsize - 1;
		qdvga.cursor[page].x--;
	}
//	qdvgaVarCursorPosRow(_bh) = qdvga.cursor[page].x;
//	qdvgaVarCursorPosCol(_bh) = qdvga.cursor[page].y;
}
static void CursorForward(t_nubit8 page)
{
	if(qdvga.cursor[page].y < qdvga.rowsize -1) {
		qdvga.cursor[page].y++;
	} else if(qdvga.cursor[page].x < qdvga.colsize -1) {
		qdvga.cursor[page].y = 0;
		qdvga.cursor[page].x++;
	}
	//qdvgaVarCursorPosRow(_bh) = qdvga.cursor[page].x;
	//qdvgaVarCursorPosCol(_bh) = qdvga.cursor[page].y;
}
static void CursorNewLine(t_nubit8 page)
{
	qdvga.cursor[page].y = 0;
	if (qdvga.cursor[page].x < qdvga.colsize - 1)
		qdvga.cursor[page].x++;
	else {
		memcpy((void *)qdvgaGetCharAddr(page, 0, 0),
		       (void *)qdvgaGetCharAddr(page, 1, 0),
		       qdvgaGetPageSize - 2 * qdvga.rowsize);//显存内容上移
		memset((void *)qdvgaGetCharAddr(page + 1, -1, 0),
		       0x00, 2 * qdvga.rowsize);//最后一行置零
	}
}
static void InsertString(t_vaddrcc string, t_nubitcc count, t_bool dup,
	t_nubit8 charprop, t_nubit8 page, t_nubit8 x, t_nubit8 y)
{
	t_nubitcc i;
	qdvga.cursor[page].x = x;
	qdvga.cursor[page].y = y;
	for (i = 0;i < count;++i) {
		switch (d_nubit8(string)) {
		case 0x07: /* bell */
			break;
		case 0x08: /* backspace */
			CursorBackward(page);
			break;
		case 0x0a: /* new line */
			CursorNewLine(page);
			break;
		case 0x0d:
			break;
		default:
			qdvgaVarChar(page, x, y)     = d_nubit8(string);
			qdvgaVarCharProp(page, x, y) = charprop;
			CursorForward(page);
			break;
		}
		if (!dup) string++;
	}
}

t_nubit8 vapiCallBackDisplayGetRowSize()
{return qdvga.rowsize;}
t_nubit8 vapiCallBackDisplayGetColSize()
{return qdvga.colsize;}
t_nubit8 vapiCallBackDisplayGetCursorTop()
{return qdvga.cursortop;}
t_nubit8 vapiCallBackDisplayGetCursorBottom()
{return qdvga.cursorbottom;}
t_nubit8 vapiCallBackDisplayGetCurrentCursorPosX()
{return qdvga.cursor[qdvga.page].x;}
t_nubit8 vapiCallBackDisplayGetCurrentCursorPosY()
{return qdvga.cursor[qdvga.page].y;}
t_nubit8 vapiCallBackDisplayGetCurrentChar(t_nubit8 x, t_nubit8 y)
{return qdvgaVarChar(qdvga.page, x, y);}
t_nubit8 vapiCallBackDisplayGetCurrentCharProp(t_nubit8 x, t_nubit8 y)
{return qdvgaVarCharProp(qdvga.page, x, y);}

void qdvgaSetDisplayMode()
{
	switch (_al) {
	case 0x00:
		qdvga.rowsize = 0x28; // 40
		qdvga.colsize = 0x19; // 25
		qdvga.color   = 0x00;
		vapiDisplaySetScreen();
		break;
	case 0x01:
		qdvga.rowsize = 0x28; // 40
		qdvga.colsize = 0x19; // 25
		qdvga.color   = 0x01;
		vapiDisplaySetScreen();
		break;
	case 0x02:
	case 0x07:
		qdvga.rowsize = 0x50; // 80
		qdvga.colsize = 0x19; // 25
		qdvga.color   = 0x00;
		vapiDisplaySetScreen();
		break;
	case 0x03:
		qdvga.rowsize = 0x50; // 80
		qdvga.colsize = 0x19; // 25
		qdvga.color   = 0x01;
		vapiDisplaySetScreen();
		break;
	default:
		break;
	}
	qdvga.vgamode = _al;
	ClearTextMemory();
}
void qdvgaSetCursorShape() {
	qdvga.cursorbottom = _cl & 0x0f;
	qdvga.cursortop    = _ch & 0x0f;
}
void qdvgaSetCursorPos()
{
	if (_bh < QDVGA_COUNT_MAX_PAGE) {
		qdvga.cursor[_bh].x       = _dh;
		qdvga.cursor[_bh].y       = _dl;
	}
//	qdvgaVarCursorPosRow(_bh) = qdvga.cursor[_bh].x;
//	qdvgaVarCursorPosCol(_bh) = qdvga.cursor[_bh].y;
}
void qdvgaGetCursorPos()
{
	if (_bh < QDVGA_COUNT_MAX_PAGE) {
		_dh = qdvga.cursor[_bh].x;
		_dl = qdvga.cursor[_bh].y;
		_ch = qdvga.cursortop;
		_cl = qdvga.cursorbottom;
	}
//	qdvgaVarCursorPosRow(_bh) = qdvga.cursor[_bh].x;
//	qdvgaVarCursorPosCol(_bh) = qdvga.cursor[_bh].y;
}
void qdvgaSetDisplayPage()
{
	qdvga.page = _al;
}
void qdvgaScrollUp()
{
	t_nsbitcc i;
//	t_nsbitcc j;
	if (!_al) {
		ClearTextMemory();
		return;
	}
	for (i = 0x00;i < _dh - _ch - _al + 1;++i)
		memcpy((void *)qdvgaGetCharAddr(qdvga.page, (_ch + i), _cl),
		       (void *)qdvgaGetCharAddr(qdvga.page, (_ch + i + _al), _cl),
		       (_dl - _cl + 1) * 2);
/*	for(i = 0x00;i < _dh - _ch - _al + 1;++i) {
		// TODO: a bug.
		for(j = 0x00;j < _dl - _cl + 1;++j) {
			qdvgaVarChar(qdvga.page, (_ch + i + _al), _cl) = ' ';
			qdvgaVarCharProp(qdvga.page, (_ch + i + _al), _cl) = _bh;
			//bh决定了空白行的属性
		}
	}*/
}
void qdvgaScrollDown()
{
	t_nsbitcc i;
//	t_nsbitcc j;
	if (!_al) {
		ClearTextMemory();
		return;
	}
	for (i = 0x00;i < _dh - _ch - _al + 1;++i)
		memcpy((void *)qdvgaGetCharAddr(qdvga.page, (_ch + i + _al), _cl),
		       (void *)qdvgaGetCharAddr(qdvga.page, (_ch + i), _cl),
		       (_dl - _cl + 1) * 2);
/*	for(i = 0x00;i < _dh - _ch - _al + 1;++i) {
		// TODO: a bug.
		for(j = 0x00;j < _dl - _cl + 1;++j) {
			qdvgaVarChar(qdvga.page, (_ch + i), _cl) = ' ';
			qdvgaVarCharProp(qdvga.page, (_ch + i), _cl) = _bh;
			//bh决定了空白行的属性
		}
	}*/
}
void qdvgaGetCharProp()
{
	_al = qdvgaVarChar(_bh, qdvga.cursor[_bh].x, qdvga.cursor[_bh].y);
	_ah = qdvgaVarCharProp(_bh, qdvga.cursor[_bh].x, qdvga.cursor[_bh].y);
}
void qdvgaDisplayCharProp()
{
	InsertString((t_vaddrcc)(&_al), _cx, 0x01, _bl, _bh,
		qdvga.cursor[_bh].x, qdvga.cursor[_bh].y);
}
void qdvgaDisplayChar()
{
	InsertString((t_vaddrcc)(&_al), _cx, 0x01, 0x0f, _bh,
		qdvga.cursor[_bh].x, qdvga.cursor[_bh].y);
}
//void qdvgaSetPalette() {}
//void qdvgaDisplayPixel() {}
//void qdvgaGetPixel() {}
void qdvgaGetAdapterStatus()
{
	_ah = qdvga.rowsize;
	_al = qdvga.vgamode;
	_bh = qdvga.page;
}
void qdvgaGenerateChar()
{
	if (_al == 0x30) {
		switch (_bh) {
		case 0x00:
			_bp = vramVarWord(0x0000, 0x001f * 4 + 0);
			_es = vramVarWord(0x0000, 0x001f * 4 + 2);
			break;
		case 0x01:
			_bp = vramVarWord(0x0000, 0x0043 * 4 + 0);
			_es = vramVarWord(0x0000, 0x0043 * 4 + 2);
			break;
		default:
			break;
		}
		_cx = 0x0010;
		_dl = qdvga.colsize - 0x01;
	}
}
void qdvgaGetAdapterInfo()
{
	_bh = 0x00;
	_bl = 0x03;
	_cl = 0x07;
//	_ch = 0x??;
}
void qdvgaDisplayStr()
{
	InsertString(vramGetAddr(_es, _bp), _cl, 0x00, _bl, _bh,
		_dh,_dl);
}

void qdvgaInit()
{
//	t_nubit8 i;
	memset(&qdvga, 0x00, sizeof(t_vga));
	qdvga.color   = 0x01;
	qdvga.rowsize = 0x50; // 80
	qdvga.colsize = 0x19; // 25
	qdvga.page = 0x00;
	qdvga.vgamode = 0x03;
	qdvga.cursor[qdvga.page].x = 0x05;
/*	for (i = 0x00;i < QDVGA_COUNT_MAX_PAGE; ++i) {
		qdvga.cursor[i].y      = 0x00;
		qdvga.cursor[i].x      = 0x00;
		qdvga.cursor[i].visual = 0x01;
		qdvga.cursor[i].top    = 0x05;
		qdvga.cursor[i].bottom = 0x00;
	}
	qdvga.cursor[i].x      = 0x05;
	qdvga.cursor[i].top    = 0x06;
	qdvga.cursor[i].bottom = 0x07;*/
	qdvga.cursortop    = 0x06;
	qdvga.cursorbottom = 0x07;
	memset((void *)qdvgaGetTextMemAddr, 0x00, QDVGA_SIZE_TEXT_MEMORY);
}
void qdvgaFinal()
{}