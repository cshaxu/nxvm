.8086
.model huge,stdcall
option casemap:none

.code
CodeSeg	equ	0f000h
org	0f0000h		
start:
jmp	FuncBIOSBoot


;***********************************************************************
;内部使用的函数

	
;***********************************************************************
;中断服务程序


INT_00h:
	out 50h,al
	iret
	
	
INT_01h:
	pushf
	pop	ax
	and	ax,0ffffh-020h
	push	ax
	popf
	out 51h,al
	iret
	
INT_02h:
	out 52h,al
	iret
	

INT_03h:
	out 53h,al
	iret
	
	
INT_04h:
	out 54h,al
	iret
	
	
INT_05h:
	out 55h,al
	iret
	
		
INT_06h:
	out 56h,al
	iret
	
	
INT_07h:
	out 57h,al
	iret
	
	
INT_08h:
	out 58h,al
	iret
	
	
INT_09h:
	out 59h,al
	iret
	

INT_0Ah:
	out 5Ah,al
	iret


INT_0Bh:
	out 5Bh,al
	iret


INT_0Ch:
	out 5Ch,al
	iret


INT_0Dh:
	out 5Dh,al
	iret


INT_0Eh:
	out 5Eh,al
	iret


INT_0Fh:
	out 5Fh,al
	iret

	
INT_10h:
	out	0b0h,al			;这里用了一条IO指令来代替整个INT 10H，INT10H的具体程序在Display.cpp里实现
	iret


;INT 11h，设备检测
INT_11h:
	push	ds
	push	bx
	mov	ax,0040h
	mov	ds,ax
	mov	bx,0010h	;BIOS数据区，这里存放着设备信息
	mov	ax,word ptr [bx]
	pop	bx
	pop	ds
	iret



INT_12h:
	push	ds
	push	bx
	mov	ax,0040h
	mov	ds,ax
	mov	bx,0013h	;BIOS数据区，在0040h:0013h的位置，大小两字节，存放着内存的大小，以KB为单位
	mov	ax,word ptr [bx]
	pop	bx
	pop	ds
	iret


INT_13h:
	cmp	ah,00h
	je	DiskReset
		
;	cmp	ah,02h
;	je	ReadSector
	
	;如果是其它的功能号，则在DMA.cpp里已经实现了，直接调用
	out	3,al
	iret
	
	
DiskReset:	
	clc
	mov	ah,0
	iret


;ReadSector这部分已经改由DMA.cpp实现
;ReadSector:	
;	push	ax
;	push	dx
;	
;	;设置软驱的指针
;	mov	dx,03f0h	;选择寄存器		扇区
;	mov	al,0
;	out	dx,al
;	mov	dx,03f1h	;向寄存器输出
;	mov	al,cl
;	out	dx,al
;
;	mov	dx,03f0h	;选择寄存器		磁头
;	mov	al,1
;	out	dx,al
;	pop	ax
;	push	ax
;	mov	al,ah
;	mov	dx,03f1h	;向寄存器输出
;	out	dx,al
;
;	mov	dx,03f0h	;选择寄存器		柱面
;	mov	al,2
;	out	dx,al
;	mov	dx,03f1h	;向寄存器输出
;	mov	al,ch
;	out	dx,al
;	
;
;	mov	dx,03f0h	;选择寄存器
;	mov	al,3
;	out	dx,al
;	pop	ax
;	push	ax
;	mov	dx,03f1h	;向寄存器输出	
;	out	dx,al
;
;
;	;设置DMA
;	mov	al,2		;软驱用2号通道
;	out	0,al
;	mov	al,bl
;	out	1,al
;	mov	al,bh
;	out	1,al
;	mov	ax,es
;	out	1,al
;	mov	al,ah
;	out	1,al
;	
;	mov	al,0
;	out	1,al
;	pop	ax
;	pop	ax
;	mov	ah,0
;	cmp	al,80h
;	jl	INT13_ReadSector_NoMoreThan80h
;	inc	ah
;INT13_ReadSector_NoMoreThan80h:	
;	shl	al,1
;	out	1,al
;	mov	al,ah
;	out	1,al
;	mov	al,0
;	out	1,al
;	
;	mov	al,10010110b		;从DMA通道2读数据！
;	out	2,al
;	iret
;	
	
INT_14h:
	out	14h,al
	iret



INT_15h:
	cmp	ah,0c0h
	je	INT_15h_0C0
	out	0b1h,al
	iret
INT_15h_0C0:
	mov	ax,CodeSeg
	mov	es,ax
	mov	bx,offset INT_15h_Environment - offset start
	mov	ah,0
	iret
INT_15h_Environment:
	db	08h,00h,0fch,00h, 01h,0b4h,40h,00h,00h,00h
	iret



INT_16h:
	out	16h,al
	iret


INT_17h:
	out	17h,al
	iret


INT_18h:
	iret


INT_19h:
	iret


INT_1Ah:
	out	73h,al				;这里用一条IO指令调用对应的模拟程序来实现中断1Ah
	iret


INT_1Bh:
	iret


INT_1Ch:
	iret


INT_1Dh:
	iret


INT_1Eh:
	iret


INT_1Fh:
	iret
	
	
INT_NOP:
	out	0cdh,al
	iret


	

;*******************************************************************************
;BIOSBoot的编写尽量只调用BIOS中断
DataBIOSBoot:
strHi:	
db	"easyVM",0dh,0ah
strBuild:
db	"Last Build 2008/05/31",0dh,0ah
strCopyRight:
db	"Designer & Programmer: "
stryinX:
db	"yinX"
strMail:
db	0dh,0ah,"Contact: "
strLms:
db	"Lms_1986@163.com"
strLfCr:
db	0dh,0ah,0dh,0ah
strVerEnd:

FuncVerInfo:
	mov	bh,0
	mov	ax,CodeSeg
	mov	es,ax
	mov	bp,offset strHi - offset start
	mov	cx,offset stryinX - offset strHi
	mov	dx,0
	mov	al,1
	mov	bl,7
	mov	ah,13h
	int	10h
	mov	bp,offset stryinX - offset start
	mov	cx,offset strMail - offset stryinX
	mov	dx,200h+23
	mov	al,1
	mov	bl,09fh
	mov	ah,13h
	int	10h
	mov	bp,offset strMail - offset start
	mov	cx,offset strLms - offset strMail
	mov	dx,200h+27
	mov	al,1
	mov	bl,7
	mov	ah,13h
	int	10h
	mov	bp,offset strLms - offset start
	mov	cx,offset strLfCr - offset strLms
	mov	dx,300h+9
	mov	al,1
	mov	bl,1fh
	mov	ah,13h
	int	10h
	mov	bp,offset strLfCr - offset start
	mov	cx,offset strVerEnd - offset strLfCr
	mov	dx,300h+25
	mov	al,1
	mov	bl,07h
	mov	ah,13h
	int	10h
	ret

FuncBIOSBoot:

	;初始化，置各段寄存器的值，置栈空间，复制地址表
	mov	ax,cs	
	mov	ds,ax
	mov	es,ax
	sub	ax,1000h
	mov	ss,ax	
	mov	ax,0ff00h
	mov	sp,ax
	mov	bp,ax
	mov	cx,offset IntTableEnd- offset IntTable
	mov	di,0
	xor	ax,ax
	mov	es,ax
	mov	si,offset IntTable - offset start
	rep movsb
	
	;把中断表其余部分也填满
	mov	cx,0d0h
OtherInt:
	mov	ax,offset INT_NOP - offset start
	stosw
	mov	ax,CodeSeg
	stosw
	loop	OtherInt


	call	FuncVerInfo
	
	mov	ax,0040h
	mov	ds,ax
	mov	bx,0100h	;这个位置放置着启动盘盘符	
	mov	dl,byte ptr [bx];读取启动盘盘符	00 - A:	80 - C:
	
	mov	ax,0
	mov	es,ax
	mov	ah,02h		;读引导扇区
	mov	al,1
	mov	cx,1
	mov	dh,0
	mov	bx,7c00h
	int	13h
	
	;寄存器的初始化
	mov	ax,0
	mov	bx,0100h
	mov	bl,byte ptr [bx]
	xor	bh,bh
	mov	cx,ax
	inc	cx
	mov	dx,bx
	mov	ss,ax
	mov	ds,ax
	mov	es,ax
	mov	sp,ax
	sub	sp,2
	mov	bp,ax
	mov	ax,0aa55h
	
	
	;跳到引导扇区
	db	0eah,00,7ch,00,00
	
	
	
;*************************************************************************
;SYSTEM DATA - DISKETTE PARAMETERS
DisketteParameters:
db	0afh,002h,025h,002h,012h,01bh,0ffh,06ch
db	0f6h,00fh,008h,04fh,000h,004h,000h,000h
DisketteParametersEnd:
	
;*************************************************************************
;中断地址表，要复制到0x0:0x0去
DataSeg	equ	0f000h
IntTable:
dw	offset INT_00h - offset start,	DataSeg, offset INT_01h - offset start,	DataSeg, offset INT_02h - offset start,	DataSeg, offset INT_03h - offset start,	DataSeg
dw	offset INT_04h - offset start,	DataSeg, offset INT_05h - offset start,	DataSeg, offset INT_06h - offset start,	DataSeg, offset INT_07h - offset start,	DataSeg
dw	offset INT_08h - offset start,	DataSeg, offset INT_09h - offset start,	DataSeg, offset INT_0Ah - offset start,	DataSeg, offset INT_0Bh - offset start,	DataSeg
dw	offset INT_0Ch - offset start,	DataSeg, offset INT_0Dh - offset start,	DataSeg, offset INT_0Eh - offset start,	DataSeg, offset INT_0Fh - offset start,	DataSeg
dw	offset INT_10h - offset start,	DataSeg, offset INT_11h - offset start,	DataSeg, offset INT_12h - offset start,	DataSeg, offset INT_13h - offset start,	DataSeg
dw	offset INT_14h - offset start,	DataSeg, offset INT_15h - offset start,	DataSeg, offset INT_16h - offset start,	DataSeg, offset INT_17h - offset start,	DataSeg
dw	offset INT_NOP - offset start,	DataSeg, offset INT_NOP - offset start,	DataSeg, offset INT_1Ah - offset start,	DataSeg, offset INT_NOP - offset start,	DataSeg
dw	offset INT_NOP - offset start,	DataSeg, offset INT_NOP - offset start,	DataSeg, offset DisketteParameters - offset start,DataSeg, offset INT_NOP - offset start, DataSeg

dw	offset INT_NOP - offset start,	DataSeg, offset INT_NOP - offset start,	DataSeg, offset INT_NOP - offset start,	DataSeg, offset INT_NOP - offset start,	DataSeg
dw	offset INT_NOP - offset start,	DataSeg, offset INT_NOP - offset start,	DataSeg, offset INT_NOP - offset start,	DataSeg, offset INT_NOP - offset start,	DataSeg
dw	offset INT_NOP - offset start,	DataSeg, offset INT_NOP - offset start,	DataSeg, offset INT_NOP - offset start,	DataSeg, offset INT_NOP - offset start,	DataSeg
dw	offset INT_NOP - offset start,	DataSeg, offset INT_NOP - offset start,	DataSeg, offset INT_NOP - offset start,	DataSeg, offset INT_NOP - offset start,	DataSeg

IntTableEnd:


;org	0ffff0h
	jmp	offset FuncBIOSBoot
	

End start