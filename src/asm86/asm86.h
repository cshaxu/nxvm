/* This file is a part of Neko's ASM86 Project. */

#ifndef NKASM86_ASM86_H
#define NKASM86_ASM86_H

typedef struct {
	int flag;	// 0 = es; 1 = cs; 2 = ss; 3 = ds; 4 = no operand;
	int seg;	// when flag = 0,1,2,3: 0 = seglabel; 1 = instruction;
	unsigned short mod,rm,imm;
	int len;	// length of target value
} Operand;

int assemble(const char *asmStmt,unsigned short locCS,
	unsigned int locMemory,
	unsigned short locSegment,
	unsigned short locOffset);	// returns length of instruct
int disassemble(char *dasmStmt,Operand *resOperand,
	const unsigned int locMemory,
	const unsigned short locSegment,
	const unsigned short locOffset);	// returns length of instruct

#endif