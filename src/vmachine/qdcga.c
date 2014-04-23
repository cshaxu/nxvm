/* This file is a part of NXVM project. */

#include "stdlib.h"
#include "memory.h"

#include "vapi.h"
#include "vcpu.h"
#include "qdcga.h"


t_cga qdcga;

static int ModeBufSize[0x14] = {
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
	} else if(qdcgaVarCursorPosRow(page) < qdcga.colsize -1) {
		qdcgaVarCursorPosCol(page) = 0;
		qdcgaVarCursorPosRow(page)++;
	}
}
static void CursorNewLine(t_nubit8 page)
{
	qdcgaVarCursorPosCol(page) = 0;
	if (qdcgaVarCursorPosRow(page) < qdcga.colsize - 1)
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
	t_nubit8 charprop, t_nubit8 page, t_nubit8 x, t_nubit8 y)
{
	t_nubitcc i;
	qdcgaVarCursorPosRow(page) = x;
	qdcgaVarCursorPosCol(page) = y;
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
			qdcgaVarChar(page, x, y)     = d_nubit8(string);
			qdcgaVarCharProp(page, x, y) = charprop;
			CursorForward(page);
			break;
		}
		if (!dup) string++;
	}
	//vapiDisplayPaint();
}

t_bool vapiCallBackDisplayGetCursorPosChange()
{
	if (qdcga.curcompx != qdcgaVarCursorPosRow(qdcgaVarPageNum) ||
		qdcga.curcompy != qdcgaVarCursorPosCol(qdcgaVarPageNum)) {
		qdcga.curcompx = qdcgaVarCursorPosRow(qdcgaVarPageNum);
		qdcga.curcompy = qdcgaVarCursorPosCol(qdcgaVarPageNum);
		return 0x01;
	} else return 0x00;
}
t_bool vapiCallBackDisplayGetBufferChange()
{
	if (memcmp((void *)qdcga.bufcomp, (void *)qdcgaGetTextMemAddr, qdcgaVarRagenSize)) {
		memcpy((void *)qdcga.bufcomp, (void *)qdcgaGetTextMemAddr, qdcgaVarRagenSize);
		return 0x01;
	} else return 0x00;

}
t_nubit16 vapiCallBackDisplayGetRowSize()
{return qdcgaVarRowSize;}
t_nubit16 vapiCallBackDisplayGetColSize()
{return qdcga.colsize;}
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

/*void qdcgaCheckVideoRam(t_vaddrcc addr)
{
	t_nubit16 offset1 = vram.base + QDCGA_VBIOS_ADDR_CGA_DISPLAY_RAM;
	t_nubit16 offset2 = offset1 + QDCGA_SIZE_TEXT_MEMORY;
	if ((addr >= offset1) && (addr < offset2)) vapiDisplayPaint();
}*/

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
		qdcga.colsize = 0x19; // 25
		qdcga.color   = 0x00;
		vapiDisplaySetScreen();
		break;
	case 0x01:
		qdcgaVarRowSize = 0x28; // 40
		qdcga.colsize = 0x19; // 25
		qdcga.color   = 0x01;
		vapiDisplaySetScreen();
		break;
	case 0x02:
	case 0x07:
		qdcgaVarRowSize = 0x50; // 80
		qdcga.colsize = 0x19; // 25
		qdcga.color   = 0x00;
		vapiDisplaySetScreen();
		break;
	case 0x03:
		qdcgaVarRowSize = 0x50; // 80
		qdcga.colsize = 0x19; // 25
		qdcga.color   = 0x01;
		vapiDisplaySetScreen();
		break;
	default:
		break;
	}
	qdcgaVarRagenSize = ModeBufSize[qdcgaVarMode];
	qdcga.bufcomp = (t_vaddrcc)malloc(qdcgaVarRagenSize);
	memcpy((void *)qdcga.bufcomp, (void *)qdcgaGetTextMemAddr, qdcgaVarRagenSize);
	ClearTextMemory();
}
void qdcgaSetCursorShape()
{
	qdcgaVarCursorBottom = _cl & 0x0f;
	qdcgaVarCursorTop    = _ch & 0x0f;
}
void qdcgaSetCursorPos()
{
//	qdcgaVarPageNum = _bh;
//	qdcgaVarPageOffset = _bh * ModeBufSize[qdcgaVarMode];
	qdcgaVarCursorPosRow(_bh) = _dh;
	qdcgaVarCursorPosCol(_bh) = _dl;
}
void qdcgaGetCursorPos()
{
//	qdcgaVarPageNum = _bh;
//	qdcgaVarPageOffset = _bh * ModeBufSize[qdcgaVarMode];
	_dh = qdcgaVarCursorPosRow(_bh);
	_dl = qdcgaVarCursorPosCol(_bh);
	_ch = qdcgaVarCursorTop;
	_cl = qdcgaVarCursorBottom;
}
void qdcgaSetDisplayPage()
{
	qdcgaVarPageNum = _al;
	qdcgaVarPageOffset = _al * ModeBufSize[qdcgaVarMode];
}
void qdcgaScrollUp()
{
	t_nsbitcc i, j;
/*	if (!_al) {
		ClearTextMemory();
		return;
	}
	for (i = 0x00;i < _dh - _ch - _al + 1;++i)
		memcpy((void *)qdcgaGetCharAddr(qdcgaVarPageNum, (_ch + i), _cl),
		       (void *)qdcgaGetCharAddr(qdcgaVarPageNum, (_ch + i + _al), _cl),
		       (_dl - _cl + 1) * 2);*/
/*	for(i = 0x00;i < _dh - _ch - _al + 1;++i) {
		// TODO: a bug.
		for(j = 0x00;j < _dl - _cl + 1;++j) {
			qdcgaVarChar(qdcgaVarPageNum, (_ch + i + _al), _cl) = ' ';
			qdcgaVarCharProp(qdcgaVarPageNum, (_ch + i + _al), _cl) = _bh;
			//bh决定了空白行的属性
		}
	}*/
	if (!_al) {
		for (i = 0;i < qdcga.colsize;++i) {
			for (j = 0;j < qdcgaVarRowSize;++j) {
				qdcgaVarChar(qdcgaVarPageNum, i, j) = 0x20;
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
//				*(short *)(MemoryStart+TextMemory+((i+_ch)*qdcgaVarRowSize+j)*2)=*(short *)(MemoryStart+TextMemory+((i+_ch+_al)*qdcgaVarRowSize+j)*2);
			}
		}
		for (i = 0;i < _al;++i) {
			for (j = _cl;j <= _dl;++j) {
				qdcgaVarChar(0, (i + _dh - _al + 1), j) = ' ';
				qdcgaVarCharProp(0, (i + _dh - _al + 1), j) = _bh;
//				*(char *)(MemoryStart+TextMemory+((i+_dh-_al+1)*qdcgaVarRowSize+j)*2)=' ';
//				*(char *)(MemoryStart+TextMemory+((i+_dh-_al+1)*qdcgaVarRowSize+j)*2+1)=_bh;
			}
		}
	}
}
void qdcgaScrollDown()
{
	t_nsbitcc i, j;
/*	if (!_al) {
		ClearTextMemory();
		return;
	}
	for (i = 0x00;i < _dh - _ch - _al + 1;++i)
		memcpy((void *)qdcgaGetCharAddr(qdcgaVarPageNum, (_ch + i + _al), _cl),
		       (void *)qdcgaGetCharAddr(qdcgaVarPageNum, (_ch + i), _cl),
		       (_dl - _cl + 1) * 2);*/
/*	for(i = 0x00;i < _dh - _ch - _al + 1;++i) {
		// TODO: a bug.
		for(j = 0x00;j < _dl - _cl + 1;++j) {
			qdcgaVarChar(qdcgaVarPageNum, (_ch + i), _cl) = ' ';
			qdcgaVarCharProp(qdcgaVarPageNum, (_ch + i), _cl) = _bh;
			//bh决定了空白行的属性
		}
	}*/
	if (!_al) {
		for (i = 0;i < qdcga.colsize;++i) {
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
void qdcgaDisplayCharProp()
{
	InsertString((t_vaddrcc)(&_al), _cx, 0x01, _bl, _bh,
		qdcgaVarCursorPosRow(_bh), qdcgaVarCursorPosCol(_bh));
}
void qdcgaDisplayChar()
{
	InsertString((t_vaddrcc)(&_al), _cx, 0x01, 0x0f, _bh,
		qdcgaVarCursorPosRow(_bh), qdcgaVarCursorPosCol(_bh));
}
//void qdcgaSetPalette() {}
//void qdcgaDisplayPixel() {}
//void qdcgaGetPixel() {}
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
		_dl = qdcga.colsize - 0x01;
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
	InsertString(vramGetAddr(_es, _bp), _cl, 0x00, _bl, _bh,
		_dh,_dl);
}

void qdcgaInit()
{
//	t_nubit8 i;
	memset(&qdcga, 0x00, sizeof(t_cga));
	qdcga.color   = 0x01;
	qdcgaVarRowSize = 0x50; // 80
	qdcga.colsize = 0x19; // 25
	qdcgaVarPageNum = 0x00;
	qdcgaVarMode = 0x03;
	qdcgaVarRagenSize = ModeBufSize[qdcgaVarMode];
	qdcgaVarCursorPosRow(0) = 0x05;
	qdcgaVarCursorPosCol(0) = 0x00;
	qdcgaVarCursorTop       = 0x06;
	qdcgaVarCursorBottom    = 0x07;
	memset((void *)qdcgaGetTextMemAddr, 0x00, QDCGA_SIZE_TEXT_MEMORY);
	qdcga.bufcomp = (t_vaddrcc)malloc(qdcgaVarRagenSize);
	qdcga.curcompx = qdcga.curcompy = 0x00;
}
void qdcgaFinal()
{}