/* This file is a part of NXVM project. */

#include "stdlib.h"
#include "memory.h"

#include "vapi.h"
#include "vcpu.h"
#include "qdvga.h"


t_vga qdvga;

static int ModeBufSize[0x14] = {
	2048,2048,4096,4096,16000,16000,16000,
	4096,16000,32000,32000,
	0,0,32000,64000,
	28000,224000,38400,153600,64000};

static void ClearTextMemory()
{ 
	memset((void*)qdvgaGetTextMemAddrPageCur, 0x00, qdvgaGetPageSize);
}
static void CursorBackward(t_nubit8 page)
{
	if (qdvgaVarCursorPosCol(page))
		qdvgaVarCursorPosCol(page)--;
	else if (qdvgaVarCursorPosRow(page)) {
		qdvgaVarCursorPosCol(page) = qdvgaVarRowSize - 1;
		qdvgaVarCursorPosRow(page)--;
	}
}
static void CursorForward(t_nubit8 page)
{
	if(qdvgaVarCursorPosCol(page) < qdvgaVarRowSize -1) {
		qdvgaVarCursorPosCol(page)++;
	} else if(qdvgaVarCursorPosRow(page) < qdvga.colsize -1) {
		qdvgaVarCursorPosCol(page) = 0;
		qdvgaVarCursorPosRow(page)++;
	}
}
static void CursorNewLine(t_nubit8 page)
{
	qdvgaVarCursorPosCol(page) = 0;
	if (qdvgaVarCursorPosRow(page) < qdvga.colsize - 1)
		qdvgaVarCursorPosRow(page)++;
	else {
		memcpy((void *)qdvgaGetCharAddr(page, 0, 0),
		       (void *)qdvgaGetCharAddr(page, 1, 0),
		       qdvgaGetPageSize - 2 * qdvgaVarRowSize);//显存内容上移
		memset((void *)qdvgaGetCharAddr(page + 1, -1, 0),
		       0x00, 2 * qdvgaVarRowSize);//最后一行置零
	}
}
static void InsertString(t_vaddrcc string, t_nubitcc count, t_bool dup,
	t_nubit8 charprop, t_nubit8 page, t_nubit8 x, t_nubit8 y)
{
	t_nubitcc i;
	qdvgaVarCursorPosRow(page) = x;
	qdvgaVarCursorPosCol(page) = y;
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
	//vapiDisplayPaint();
}

t_bool vapiCallBackDisplayGetCursorPosChange()
{
	if (qdvga.curcompx != qdvgaVarCursorPosRow(qdvgaVarPageNum) ||
		qdvga.curcompy != qdvgaVarCursorPosCol(qdvgaVarPageNum)) {
		qdvga.curcompx = qdvgaVarCursorPosRow(qdvgaVarPageNum);
		qdvga.curcompy = qdvgaVarCursorPosCol(qdvgaVarPageNum);
		return 0x01;
	} else return 0x00;
}
t_bool vapiCallBackDisplayGetBufferChange()
{
	if (memcmp((void *)qdvga.bufcomp, (void *)qdvgaGetTextMemAddr, qdvgaVarRagenSize)) {
		memcpy((void *)qdvga.bufcomp, (void *)qdvgaGetTextMemAddr, qdvgaVarRagenSize);
		return 0x01;
	} else return 0x00;

}
t_nubit16 vapiCallBackDisplayGetRowSize()
{return qdvgaVarRowSize;}
t_nubit16 vapiCallBackDisplayGetColSize()
{return qdvga.colsize;}
t_nubit8 vapiCallBackDisplayGetCursorTop()
{return qdvgaVarCursorTop;}
t_nubit8 vapiCallBackDisplayGetCursorBottom()
{return qdvgaVarCursorBottom;}
t_nubit8 vapiCallBackDisplayGetCurrentCursorPosX()
{return qdvgaVarCursorPosRow(qdvgaVarPageNum);}
t_nubit8 vapiCallBackDisplayGetCurrentCursorPosY()
{return qdvgaVarCursorPosCol(qdvgaVarPageNum);}
t_nubit8 vapiCallBackDisplayGetCurrentChar(t_nubit8 x, t_nubit8 y)
{return qdvgaVarChar(qdvgaVarPageNum, x, y);}
t_nubit8 vapiCallBackDisplayGetCurrentCharProp(t_nubit8 x, t_nubit8 y)
{return qdvgaVarCharProp(qdvgaVarPageNum, x, y);}

/*void qdvgaCheckVideoRam(t_vaddrcc addr)
{
	t_nubit16 offset1 = vram.base + QDVGA_VBIOS_ADDR_CGA_DISPLAY_RAM;
	t_nubit16 offset2 = offset1 + QDVGA_SIZE_TEXT_MEMORY;
	if ((addr >= offset1) && (addr < offset2)) vapiDisplayPaint();
}*/

void qdvgaSetDisplayMode()
{
	qdvgaVarMode = _al;
	qdvgaVarPageNum = 0x00;
	qdvgaVarPageOffset = 0x00;
	qdvgaVarCursorPosRow(qdvgaVarPageNum) = 0x00;
	qdvgaVarCursorPosCol(qdvgaVarPageNum) = 0x00;
	switch (_al) {
	case 0x00:
		qdvgaVarRowSize = 0x28; // 40
		qdvga.colsize = 0x19; // 25
		qdvga.color   = 0x00;
		vapiDisplaySetScreen();
		break;
	case 0x01:
		qdvgaVarRowSize = 0x28; // 40
		qdvga.colsize = 0x19; // 25
		qdvga.color   = 0x01;
		vapiDisplaySetScreen();
		break;
	case 0x02:
	case 0x07:
		qdvgaVarRowSize = 0x50; // 80
		qdvga.colsize = 0x19; // 25
		qdvga.color   = 0x00;
		vapiDisplaySetScreen();
		break;
	case 0x03:
		qdvgaVarRowSize = 0x50; // 80
		qdvga.colsize = 0x19; // 25
		qdvga.color   = 0x01;
		vapiDisplaySetScreen();
		break;
	default:
		break;
	}
	qdvgaVarRagenSize = ModeBufSize[qdvgaVarMode];
	qdvga.bufcomp = (t_vaddrcc)malloc(qdvgaVarRagenSize);
	memcpy((void *)qdvga.bufcomp, (void *)qdvgaGetTextMemAddr, qdvgaVarRagenSize);
	ClearTextMemory();
}
void qdvgaSetCursorShape()
{
	qdvgaVarCursorBottom = _cl & 0x0f;
	qdvgaVarCursorTop    = _ch & 0x0f;
}
void qdvgaSetCursorPos()
{
//	qdvgaVarPageNum = _bh;
//	qdvgaVarPageOffset = _bh * ModeBufSize[qdvgaVarMode];
	qdvgaVarCursorPosRow(_bh) = _dh;
	qdvgaVarCursorPosCol(_bh) = _dl;
}
void qdvgaGetCursorPos()
{
//	qdvgaVarPageNum = _bh;
//	qdvgaVarPageOffset = _bh * ModeBufSize[qdvgaVarMode];
	_dh = qdvgaVarCursorPosRow(_bh);
	_dl = qdvgaVarCursorPosCol(_bh);
	_ch = qdvgaVarCursorTop;
	_cl = qdvgaVarCursorBottom;
}
void qdvgaSetDisplayPage()
{
	qdvgaVarPageNum = _al;
	qdvgaVarPageOffset = _al * ModeBufSize[qdvgaVarMode];
}
void qdvgaScrollUp()
{
	t_nsbitcc i, j;
/*	if (!_al) {
		ClearTextMemory();
		return;
	}
	for (i = 0x00;i < _dh - _ch - _al + 1;++i)
		memcpy((void *)qdvgaGetCharAddr(qdvgaVarPageNum, (_ch + i), _cl),
		       (void *)qdvgaGetCharAddr(qdvgaVarPageNum, (_ch + i + _al), _cl),
		       (_dl - _cl + 1) * 2);*/
/*	for(i = 0x00;i < _dh - _ch - _al + 1;++i) {
		// TODO: a bug.
		for(j = 0x00;j < _dl - _cl + 1;++j) {
			qdvgaVarChar(qdvgaVarPageNum, (_ch + i + _al), _cl) = ' ';
			qdvgaVarCharProp(qdvgaVarPageNum, (_ch + i + _al), _cl) = _bh;
			//bh决定了空白行的属性
		}
	}*/
	if (!_al) {
		for (i = 0;i < qdvga.colsize;++i) {
			for (j = 0;j < qdvgaVarRowSize;++j) {
				qdvgaVarChar(qdvgaVarPageNum, i, j) = 0x20;
				qdvgaVarCharProp(qdvgaVarPageNum, i, j) = _bh;
			}
		}
	} else {
		for (i = 0;i < _dh - _ch;++i) {
			for (j = _cl;j <= _dl;++j) {
				qdvgaVarChar(qdvgaVarPageNum, (i + _ch), j) =
					qdvgaVarChar(qdvgaVarPageNum, (i + _ch + _al), j);
				qdvgaVarCharProp(qdvgaVarPageNum, (i + _ch), j) =
					qdvgaVarCharProp(qdvgaVarPageNum, (i + _ch + _al), j);
//				*(short *)(MemoryStart+TextMemory+((i+_ch)*qdvgaVarRowSize+j)*2)=*(short *)(MemoryStart+TextMemory+((i+_ch+_al)*qdvgaVarRowSize+j)*2);
			}
		}
		for (i = 0;i < _al;++i) {
			for (j = _cl;j <= _dl;++j) {
				qdvgaVarChar(0, (i + _dh - _al + 1), j) = ' ';
				qdvgaVarCharProp(0, (i + _dh - _al + 1), j) = _bh;
//				*(char *)(MemoryStart+TextMemory+((i+_dh-_al+1)*qdvgaVarRowSize+j)*2)=' ';
//				*(char *)(MemoryStart+TextMemory+((i+_dh-_al+1)*qdvgaVarRowSize+j)*2+1)=_bh;
			}
		}
	}
}
void qdvgaScrollDown()
{
	t_nsbitcc i, j;
/*	if (!_al) {
		ClearTextMemory();
		return;
	}
	for (i = 0x00;i < _dh - _ch - _al + 1;++i)
		memcpy((void *)qdvgaGetCharAddr(qdvgaVarPageNum, (_ch + i + _al), _cl),
		       (void *)qdvgaGetCharAddr(qdvgaVarPageNum, (_ch + i), _cl),
		       (_dl - _cl + 1) * 2);*/
/*	for(i = 0x00;i < _dh - _ch - _al + 1;++i) {
		// TODO: a bug.
		for(j = 0x00;j < _dl - _cl + 1;++j) {
			qdvgaVarChar(qdvgaVarPageNum, (_ch + i), _cl) = ' ';
			qdvgaVarCharProp(qdvgaVarPageNum, (_ch + i), _cl) = _bh;
			//bh决定了空白行的属性
		}
	}*/
	if (!_al) {
		for (i = 0;i < qdvga.colsize;++i) {
			for (j = 0;j < qdvgaVarRowSize;++j) {
				qdvgaVarChar(qdvgaVarPageNum, i, j) = 0x20;
				qdvgaVarCharProp(qdvgaVarPageNum, i, j) = _bh;
			}
		}
	} else {
		for (i = _dh - _ch - 1;i >=0;--i) {
			for (j = _cl;j <= _dl;++j) {
				qdvgaVarChar(qdvgaVarPageNum, (i + _ch + _al), j) =
					qdvgaVarChar(qdvgaVarPageNum, (i + _ch), j);
				qdvgaVarCharProp(qdvgaVarPageNum, (i + _ch + _al), j) =
					qdvgaVarCharProp(qdvgaVarPageNum, (i + _ch), j);
			}
		}
		for (i = _al - 1;i >= 0;--i) {
			for (j = _cl;j <= _dl;++j) {
				qdvgaVarChar(0, (i + _ch), j) = ' ';
				qdvgaVarCharProp(0, (i + _ch), j) = _bh;
			}
		}
	}
}
void qdvgaGetCharProp()
{
	_al = qdvgaVarChar(_bh, qdvgaVarCursorPosRow(_bh), qdvgaVarCursorPosCol(_bh));
	_ah = qdvgaVarCharProp(_bh, qdvgaVarCursorPosRow(_bh), qdvgaVarCursorPosCol(_bh));
}
void qdvgaDisplayCharProp()
{
	InsertString((t_vaddrcc)(&_al), _cx, 0x01, _bl, _bh,
		qdvgaVarCursorPosRow(_bh), qdvgaVarCursorPosCol(_bh));
}
void qdvgaDisplayChar()
{
	InsertString((t_vaddrcc)(&_al), _cx, 0x01, 0x0f, _bh,
		qdvgaVarCursorPosRow(_bh), qdvgaVarCursorPosCol(_bh));
}
//void qdvgaSetPalette() {}
//void qdvgaDisplayPixel() {}
//void qdvgaGetPixel() {}
void qdvgaGetAdapterStatus()
{
	_ah = (t_nubit8)qdvgaVarRowSize;
	_al = qdvgaVarMode;
	_bh = qdvgaVarPageNum;
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
	qdvgaVarRowSize = 0x50; // 80
	qdvga.colsize = 0x19; // 25
	qdvgaVarPageNum = 0x00;
	qdvgaVarMode = 0x03;
	qdvgaVarRagenSize = ModeBufSize[qdvgaVarMode];
	qdvgaVarCursorPosRow(0) = 0x05;
	qdvgaVarCursorPosCol(0) = 0x00;
	qdvgaVarCursorTop       = 0x06;
	qdvgaVarCursorBottom    = 0x07;
	memset((void *)qdvgaGetTextMemAddr, 0x00, QDVGA_SIZE_TEXT_MEMORY);
	qdvga.bufcomp = (t_vaddrcc)malloc(qdvgaVarRagenSize);
	qdvga.curcompx = qdvga.curcompy = 0x00;
}
void qdvgaFinal()
{}