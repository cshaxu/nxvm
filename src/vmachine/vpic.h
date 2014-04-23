/* This file is a part of NXVM project. */

// Programmable Interrupt Controller: Intel 8259A (Master+Slave)

#ifndef NXVM_VPIC_H
#define NXVM_VPIC_H

#include "vglobal.h"


typedef enum {ICW1,ICW2,ICW3,ICW4,OCW1} PICInitStatus;

typedef struct {
	t_nubit8 irr;	// Interrupt Request Register
	t_nubit8 imr;	// Interrupt Mask Register
	t_nubit8 isr;	// In Service Register
	t_nubit8 icw1,icw2,icw3,icw4,ocw1,ocw2,ocw3;
	PICInitStatus init;
	t_bool slave;
} t_pic;

extern t_pic vpicmaster,vpicslave; // 0x08; 

t_nubit16 vpicGetIMR();
t_nubit16 vpicGetISR();
t_nubit16 vpicGetIRR();
void vpicSetIMR(t_nubit16 imr);
void vpicSetISR(t_nubit16 isr);
void vpicSetIRR(t_nubit16 irr);

t_bool vpicIsINTR();
t_nubit8 vpicGetINTR();
void vpicRespondINTR(t_nubit8 intr);
void vpicSetIRQ(t_nubit8 irqid);

void PICInit();
void PICTerm();


#endif