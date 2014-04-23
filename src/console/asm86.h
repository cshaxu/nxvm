/* This file is a part of Neko's ASM86 Project. */

/*
	Neko Confidential
	Copyright (c) 2012 Neko. All rights reserved.
	Project Period:	01/25/2012 - 02/05/2012
	Current Build:	0x002d (03/15/2012)
*/

#ifndef NXVM_ASM86_H
#define NXVM_ASM86_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	int flag;	/* 0 = es; 1 = cs; 2 = ss; 3 = ds; 4 = no operand; */
	int seg;	/* when flag = 0,1,2,3: 0 = seglabel; 1 = instruction; */
	unsigned short mod,rm,imm;
	int len;	/* length of target value */
} Operand;

/* returns length of instruction */
int assemble(const char *asmStmt,unsigned short locCS,
	void *locMemory,unsigned short locSegment,unsigned short locOffset);
/* returns length of instruction  */
int disassemble(char *dasmStmt,Operand *resOperand,
	const void *locMemory,const unsigned short locSegment,const unsigned short locOffset);

#ifdef __cplusplus
}/*cends*/
#endif

#endif