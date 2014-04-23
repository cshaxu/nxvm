/* This file is a part of NXVM project. */

#include "stdlib.h"
#include "memory.h"

#include "../vapi.h"
#include "../vcpu.h"

#include "qdbios.h"
#include "qdcga.h"

t_nubit32 qdcgaModeBufSize[0x14] = {
	2048,2048,4096,4096,16000,16000,16000,
	4096,16000,32000,32000,
	0,0,32000,64000,
	28000,224000,38400,153600,64000};

static void ClearTextMemory()
{ 
	memset((void*)qdcgaGetTextMemAddrPageCur, 0x00, qdcgaGetPageSize);
}
static void CursorBackward(t_nubit8 page)
{
	if (qdcgaVarCursorPosCol(page))
		qdcgaVarCursorPosCol(page)--;
	else if (qdcgaVarCursorPosRow(page)) {
		qdcgaVarCursorPosCol(page) = qdcgaVarRowSize - 1;
		qdcgaVarCursorPosRow(page)--;
	}
}
static void CursorForward(t_nubit8 page)
{
	if(qdcgaVarCursorPosCol(page) < qdcgaVarRowSize -1) {
		qdcgaVarCursorPosCol(page)++;
	} else if(qdcgaVarCursorPosRow(page) < vvadp.colsize -1) {
		qdcgaVarCursorPosCol(page) = 0;
		qdcgaVarCursorPosRow(page)++;
	}
}
static void CursorNewLine(t_nubit8 page)
{
	qdcgaVarCursorPosCol(page) = 0;
	if (qdcgaVarCursorPosRow(page) < vvadp.colsize - 1)
		qdcgaVarCursorPosRow(page)++;
	else {
		memcpy((void *)qdcgaGetCharAddr(page, 0, 0),
		       (void *)qdcgaGetCharAddr(page, 1, 0),
		       qdcgaGetPageSize - 2 * qdcgaVarRowSize);//显存内容上移
		memset((void *)qdcgaGetCharAddr(page + 1, -1, 0),
		       0x00, 2 * qdcgaVarRowSize);//最后一行置零
	}
}
static void CursorCarriageReturn(t_nubit8 page)
{
	qdcgaVarCursorPosCol(page) = 0;
}
static void CursorLineFeed(t_nubit8 page)
{
	if (qdcgaVarCursorPosRow(page) < vvadp.colsize - 1)
		qdcgaVarCursorPosRow(page)++;
	else {
		memcpy((void *)qdcgaGetCharAddr(page, 0, 0),
		       (void *)qdcgaGetCharAddr(page, 1, 0),
		       qdcgaGetPageSize - 2 * qdcgaVarRowSize);//显存内容上移
		memset((void *)qdcgaGetCharAddr(page + 1, -1, 0),
		       0x00, 2 * qdcgaVarRowSize);//最后一行置零
	}
}
static void InsertString(t_vaddrcc string, t_nubitcc count, t_bool dup,
	t_bool move, t_nubit8 charprop, t_nubit8 page, t_nubit8 x, t_nubit8 y)
{
	t_nubitcc i;
//	vapiPrint("%c", d_nubit8(string));
	qdcgaVarCursorPosRow(page) = x;
	qdcgaVarCursorPosCol(page) = y;
	for (i = 0;i < count;++i) {
		switch (d_nubit8(string)) {
		case 0x07: /* bell */
			break;
		case 0x08: /* backspace */
			if (move) CursorBackward(page);
			break;
		case 0x0a: /* new line */
			if (move) CursorLineFeed(page);
			break;
		case 0x0d:
			if (move) CursorCarriageReturn(page);
			break;
		default:
			qdcgaVarChar(page, qdcgaVarCursorPosRow(page),
				qdcgaVarCursorPosCol(page)) = d_nubit8(string);
			qdcgaVarCharProp(page, qdcgaVarCursorPosRow(page),
				qdcgaVarCursorPosCol(page)) = charprop;
			if (move) CursorForward(page);
			break;
		}
		if (!dup) string++;
	}
	//vapiDisplayPaint();
}

t_bool vapiCallBackDisplayGetCursorVisible()
{return qdcgaGetCursorVisible;}
t_bool vapiCallBackDisplayGetCursorChange()
{
	if (vvadp.oldcurposx != qdcgaVarCursorPosRow(qdcgaVarPageNum) ||
		vvadp.oldcurposy != qdcgaVarCursorPosCol(qdcgaVarPageNum) ||
		vvadp.oldcurtop  != qdcgaVarCursorTop ||
		vvadp.oldcurbottom != qdcgaVarCursorBottom) {
		vvadp.oldcurposx = qdcgaVarCursorPosRow(qdcgaVarPageNum);
		vvadp.oldcurposy = qdcgaVarCursorPosCol(qdcgaVarPageNum);
		vvadp.oldcurtop  = qdcgaVarCursorTop;
		vvadp.oldcurbottom = qdcgaVarCursorBottom;
		return 0x01;
	} else return 0x00;
}
t_bool vapiCallBackDisplayGetBufferChange()
{
	if (memcmp((void *)vvadp.bufcomp, (void *)qdcgaGetTextMemAddr, qdcgaVarRagenSize)) {
		memcpy((void *)vvadp.bufcomp, (void *)qdcgaGetTextMemAddr, qdcgaVarRagenSize);
		return 0x01;
	} else return 0x00;
}
t_nubit16 vapiCallBackDisplayGetRowSize()
{return qdcgaVarRowSize;}
t_nubit16 vapiCallBackDisplayGetColSize()
{return vvadp.colsize;}
t_nubit8 vapiCallBackDisplayGetCursorTop()
{return qdcgaVarCursorTop;}
t_nubit8 vapiCallBackDisplayGetCursorBottom()
{return qdcgaVarCursorBottom;}
t_nubit8 vapiCallBackDisplayGetCurrentCursorPosX()
{return qdcgaVarCursorPosRow(qdcgaVarPageNum);}
t_nubit8 vapiCallBackDisplayGetCurrentCursorPosY()
{return qdcgaVarCursorPosCol(qdcgaVarPageNum);}
t_nubit8 vapiCallBackDisplayGetCurrentChar(t_nubit8 x, t_nubit8 y)
{return qdcgaVarChar(qdcgaVarPageNum, x, y);}
t_nubit8 vapiCallBackDisplayGetCurrentCharProp(t_nubit8 x, t_nubit8 y)
{return qdcgaVarCharProp(qdcgaVarPageNum, x, y);}

void qdcgaSetDisplayMode()
{
	qdcgaVarMode = _al;
	qdcgaVarPageNum = 0x00;
	qdcgaVarPageOffset = 0x00;
	qdcgaVarCursorPosRow(qdcgaVarPageNum) = 0x00;
	qdcgaVarCursorPosCol(qdcgaVarPageNum) = 0x00;
	switch (_al) {
	case 0x00:
		qdcgaVarRowSize = 0x28; // 40
		vvadp.colsize = 0x19; // 25
		vvadp.color   = 0x00;
		vapiDisplaySetScreen();
		break;
	case 0x01:
		qdcgaVarRowSize = 0x28; // 40
		vvadp.colsize = 0x19; // 25
		vvadp.color   = 0x01;
		vapiDisplaySetScreen();
		break;
	case 0x02:
	case 0x07:
		qdcgaVarRowSize = 0x50; // 80
		vvadp.colsize = 0x19; // 25
		vvadp.color   = 0x00;
		vapiDisplaySetScreen();
		break;
	case 0x03:
		qdcgaVarRowSize = 0x50; // 80
		vvadp.colsize = 0x19; // 25
		vvadp.color   = 0x01;
		vapiDisplaySetScreen();
		break;
	default:
		break;
	}
	qdcgaVarRagenSize = qdcgaModeBufSize[qdcgaVarMode];
	memcpy((void *)vvadp.bufcomp, (void *)qdcgaGetTextMemAddr, qdcgaVarRagenSize);
	ClearTextMemory();
}
void qdcgaSetCursorShape()
{
	qdcgaVarCursorBottom = _cl;// & 0x0f;
	qdcgaVarCursorTop    = _ch;// & 0x0f;
}
void qdcgaSetCursorPos()
{
//	qdcgaVarPageNum = _bh;
//	qdcgaVarPageOffset = _bh * qdcgaModeBufSize[qdcgaVarMode];
	qdcgaVarCursorPosRow(_bh) = _dh;
	qdcgaVarCursorPosCol(_bh) = _dl;
}
void qdcgaGetCursorPos()
{
//	qdcgaVarPageNum = _bh;
//	qdcgaVarPageOffset = _bh * qdcgaModeBufSize[qdcgaVarMode];
	_dh = qdcgaVarCursorPosRow(_bh);
	_dl = qdcgaVarCursorPosCol(_bh);
	_ch = qdcgaVarCursorTop;
	_cl = qdcgaVarCursorBottom;
}
void qdcgaSetDisplayPage()
{
	qdcgaVarPageNum = _al;
	qdcgaVarPageOffset = _al * qdcgaModeBufSize[qdcgaVarMode];
}
void qdcgaScrollUp()
{
	t_nsbitcc i, j;
	if (!_al) {
		for (i = _ch;i <= _dh;++i) {
			for (j = _cl;j <= _dl;++j) {
				qdcgaVarChar(qdcgaVarPageNum, i, j) = ' ';
				qdcgaVarCharProp(qdcgaVarPageNum, i, j) = _bh;
			}
		}
	} else {
		for (i = 0;i < _dh - _ch;++i) {
			for (j = _cl;j <= _dl;++j) {
				qdcgaVarChar(qdcgaVarPageNum, (i + _ch), j) =
					qdcgaVarChar(qdcgaVarPageNum, (i + _ch + _al), j);
				qdcgaVarCharProp(qdcgaVarPageNum, (i + _ch), j) =
					qdcgaVarCharProp(qdcgaVarPageNum, (i + _ch + _al), j);
			}
		}
		for (i = 0;i < _al;++i) {
			for (j = _cl;j <= _dl;++j) {
				qdcgaVarChar(0, (i + _dh - _al + 1), j) = ' ';
				qdcgaVarCharProp(0, (i + _dh - _al + 1), j) = _bh;
			}
		}
	}
}
void qdcgaScrollDown()
{
	t_nsbitcc i, j;
	if (!_al) {
		for (i = 0;i < vvadp.colsize;++i) {
			for (j = 0;j < qdcgaVarRowSize;++j) {
				qdcgaVarChar(qdcgaVarPageNum, i, j) = 0x20;
				qdcgaVarCharProp(qdcgaVarPageNum, i, j) = _bh;
			}
		}
	} else {
		for (i = _dh - _ch - 1;i >=0;--i) {
			for (j = _cl;j <= _dl;++j) {
				qdcgaVarChar(qdcgaVarPageNum, (i + _ch + _al), j) =
					qdcgaVarChar(qdcgaVarPageNum, (i + _ch), j);
				qdcgaVarCharProp(qdcgaVarPageNum, (i + _ch + _al), j) =
					qdcgaVarCharProp(qdcgaVarPageNum, (i + _ch), j);
			}
		}
		for (i = _al - 1;i >= 0;--i) {
			for (j = _cl;j <= _dl;++j) {
				qdcgaVarChar(0, (i + _ch), j) = ' ';
				qdcgaVarCharProp(0, (i + _ch), j) = _bh;
			}
		}
	}
}
void qdcgaGetCharProp()
{
	_al = qdcgaVarChar(_bh, qdcgaVarCursorPosRow(_bh), qdcgaVarCursorPosCol(_bh));
	_ah = qdcgaVarCharProp(_bh, qdcgaVarCursorPosRow(_bh), qdcgaVarCursorPosCol(_bh));
}
void qdcgaDisplayCharProp9()
{
	InsertString((t_vaddrcc)(&_al), _cx, 0x01, 0x00, _bl, _bh,
		qdcgaVarCursorPosRow(_bh), qdcgaVarCursorPosCol(_bh));
}
void qdcgaDisplayCharPropE()
{
	InsertString((t_vaddrcc)(&_al), 0x01, 0x01, 0x01, _bl, _bh,
		qdcgaVarCursorPosRow(_bh), qdcgaVarCursorPosCol(_bh));
}
void qdcgaDisplayChar()
{
	InsertString((t_vaddrcc)(&_al), _cx, 0x01, 0x01, 0x0f, _bh,
		qdcgaVarCursorPosRow(_bh), qdcgaVarCursorPosCol(_bh));
}
void qdcgaGetAdapterStatus()
{
	_ah = (t_nubit8)qdcgaVarRowSize;
	_al = qdcgaVarMode;
	_bh = qdcgaVarPageNum;
}
void qdcgaGenerateChar()
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
		_dl = vvadp.colsize - 0x01;
	}
}
void qdcgaGetAdapterInfo()
{
	_bh = 0x00;
	_bl = 0x03;
	_cl = 0x07;
//	_ch = 0x??;
}
void qdcgaDisplayStr()
{
	InsertString(vramGetAddr(_es, _bp), _cl, 0x00, 0x01, _bl, _bh,
		_dh,_dl);
}

void INT_10()
{
	t_nubit16 tmpCX = _cx;
	/*switch (_ah) {
	case 0x02:
	case 0x09:
	case 0x0e:
		break;
	default:
		vapiPrint("int 10, ax=%04x, bx=%04x, cx=%04x, dx=%04x, al='%c'\n", _ax, _bx, _cx, _dx, _al);
		break;
	}*/
	switch (_ah) {
	case 0x00:
		qdcgaSetDisplayMode();
		break;
	case 0x01:
		qdcgaSetCursorShape();
		break;
	case 0x02:
		qdcgaSetCursorPos();
		break;
	case 0x03:
		qdcgaGetCursorPos();
		break;
	case 0x04:
		break;
	case 0x05:
		qdcgaSetDisplayPage();
		break;
	case 0x06:
		qdcgaScrollUp();
		break;
	case 0x07:
		qdcgaScrollDown();
		break;
	case 0x08:
		qdcgaGetCharProp();
		break;
	case 0x09:
		qdcgaDisplayCharProp9();
		break;
	case 0x0a:
		qdcgaDisplayChar();
		break;
	case 0x0e:
		qdcgaDisplayCharPropE();
		break;
	case 0x0f:
		qdcgaGetAdapterStatus();
		break;
	case 0x10:
		break;
	case 0x11:
		qdcgaGenerateChar();
		break;
	case 0x12:
		if (_bl == 0x10)
			qdcgaGetAdapterInfo();
		break;
	case 0x13:
		qdcgaDisplayStr();
		break;
	case 0x1a:
		if (_al == 0x00) {
			_al = 0x1a;
			_bh = 0x00;
			_bl = 0x08;
		}
		break;
	default:
		break;
	}
}

void qdcgaReset()
{
	qdbiosInt[0x10] = (t_faddrcc)INT_10; /* soft cga*/
	qdbiosMakeInt(0x10, "qdx 10\niret");
	vvadp.color   = 0x01;
	qdcgaVarRowSize = 0x50; // 80
	vvadp.colsize = 0x19; // 25
	qdcgaVarPageNum = 0x00;
	qdcgaVarMode = 0x03;
	qdcgaVarRagenSize = qdcgaModeBufSize[qdcgaVarMode];
	qdcgaVarCursorPosRow(0) = 0x05;
	qdcgaVarCursorPosCol(0) = 0x00;
	qdcgaVarCursorTop       = 0x06;
	qdcgaVarCursorBottom    = 0x07;
	vvadp.oldcurposx = vvadp.oldcurposy = 0x00;
	vvadp.oldcurtop  = vvadp.oldcurbottom = 0x00;
}
