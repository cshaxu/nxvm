/* This file is a part of NXVM project. */

// NOTE: For IBM-PC, 8259A will work only on Fixed Priority Mode with Non-specific EOI.

#include "memory.h"

#include "system/vapi.h"
#include "vglobal.h"
#include "vcpu.h"
#include "vcpuins.h"
#include "vpit.h"
#include "vpic.h"

t_pic vpic1,vpic2;

static void IO_Write_00x0(t_pic *vpic)
{
	// PIC 8259A Command Port
	// Write: ICW1,OCW2,OCW3
	t_nubitcc i;
	if(vcpu.al&0x10) {	// ICW1 (D4=1)
		vpic->icw1 = vcpu.al;
		vpic->init = ICW2;
		//vapiPrint("DEBUG:\tICW1:%x\n",vpic->icw1);
		if(vpic->icw1&0x01) ;	// D0=1, IC4=1
		else vpic->icw4 = 0x00;	// D0=0, IC4=0
		if(vpic->icw1&0x02) ;	// D1=1, SNGL=1, ICW3=0
		else ;					// D1=0, SNGL=0, ICW3=1
		if(vpic->icw1&0x08) ;	// D3=1, LTIM=1, Level Triggered Mode
		else ;					// D3=0, LTIM=0, Edge Triggered Mode
	} else {			// OCW (D4=0)
		if(vcpu.al&0x08) {	// OCW3 (D3=1)
			vpic->ocw3 = vcpu.al;
			//vapiPrint("DEBUG:\tOCW3:%x\n",vpic->ocw3);
			if(vpic->ocw3&0x80) vapiPrint("PIC:\tInvalid OCW3 command.\n");
			else {
				switch(vpic->ocw3&0x60) {	// ESMM (Enable Special Mask Mode)
				case 0x40:	// ESMM=1, SMM=0: Clear Sepcial Mask Mode
					/* Not Implemented */
					break;
				case 0x60:	// ESMM=1, SMM=1: Set Special Mask Mode
					/* Not Implemented */
					break;
				default:	// ESMM=0
					break;
				}
			}
		} else {				// OCW2 (D3=0)
			vpic->ocw2 = vcpu.al;
			//vapiPrint("DEBUG:\tOCW2:%x\n",vpic->ocw2);
			switch(vpic->ocw2&0xe0) {	// D7=R, D6=SL, D5=EOI(End Of Interrupt)
			case 0x80:	// 100: Set  (Rotate Priorities in Auto EOI Mode)
				/* Not Implemented */
				break;
			case 0x00:	// 000: Clear(Rotate Priorities in Auto EOI Mode)
				/* Not Implemented */
				// Why 0x00 equals to 0x20 in easyVM?
				break;
			case 0x20:	// 001: Non-specific EOI Command
						// Set bit of highest priority interrupt in ISR to 0,
						// where IR0 > IR1 > IR2(IR8 > ... > IR15) > IR3 > ... > IR7
				i=0;
				while (!((vpic->isr)&(1<<i)) && (i < 8)) i++;
				if(i < 8) vpic->isr ^= (1<<i);
				if(vpic->slave && !vpic->isr) vpic1.isr ^= 0x04; 
				break;
			case 0x60:	// 011: Specific EOI Command
				i = (vpic->ocw2)&0x07;	// Get L2,L1,L0
				vpic->isr &= ~(1<<i);	// easyVM may have problem with this stmt: "isr &= (1<<i)"@PIC.CPP
				break;
			case 0xa0:	// 101: Rotate Priorities on Non-specific EOI
				/* Not Implemented */
				break;
			case 0xe0:	// 111: Rotate Priority on Specific EOI Command (resets current ISR bit)
				/* Not Implemented */
				break;
			case 0xc0:	// 110: Set Priority (does not reset current ISR bit)
				/* Not Implemented */
				break;
			case 0x40:	// 010: No Operation
				break;
			default:break;}
		}
	}
}
static void IO_Read_00x0(t_pic *vpic)
{
	// PIC 8259A Command Port
	// Read: POLL, IRR, ISR
	t_nubitcc i;
	if((vpic->ocw3)&0x04) {	// P=1 (Poll Command)
		if(vpic->irr) {
			vcpu.al = 0x80;
			i = 0;
			while (!((vpic->isr)&(1<<i)) && (i < 8)) i++;
			vcpu.al |= i;
		} else vcpu.al = 0x00;
	} else {
		switch(vpic->ocw3&0x03) {
		case 0x02:	// RR=1, RIS=0, Read IRR
			vcpu.al = vpic->irr;
			break;
		case 0x03:	// RR=1, RIS=1, Read ISR
			vcpu.al = vpic->isr;
			break;
		default:	// RR=0, No Operation
			vcpu.al = 0x00;
			break;
		}
	}
}
static void IO_Write_00x1(t_pic *vpic)
{
	// PIC 8259A Interrupt Mask Register
	// Write(After ICW1): ICW2, ICW3, ICW4, OCW1

	switch(vpic->init) {
	case ICW2:
		vpic->icw2 = vcpu.al&(~0x07);
		//vapiPrint("DEBUG:\tICW2:%x\n",vpic->icw2);
		if(!((vpic->icw1)&0x02)) vpic->init = ICW3;		// ICW1.D1=0, SNGL=0, ICW3=1
		else if(vpic->icw1&0x01) vpic->init = ICW4;	// ICW1.D0=1, IC4=1
		else vpic->init = OCW1;
		break;
	case ICW3:
		vpic->icw3 = vcpu.al;
		//vapiPrint("DEBUG:\tICW3:%x\n",vpic->icw3);
		if((vpic->icw1)&0x01) vpic->init = ICW4;		// ICW1.D0=1, IC4=1
		else vpic->init = OCW1;
		break;
	case ICW4:
		vpic->icw4 = vcpu.al&0x1f;
		//vapiPrint("DEBUG:\tICW4:%x\n",vpic->icw4);
		if((vpic->icw4)&0x01) ;	// uPM=1, 16-bit 80x86
		else ;			// uPM=0, 8-bit 8080/8085
		if((vpic->icw4)&0x02) ;	// AEOI=1, Automatic End of Interrupt
		else ;			// AEOI=0, Non-automatic End of Interrupt
		if((vpic->icw4)&0x04)
			vpic->slave = 0;		// M/S=1, Master 8259A
		else
			vpic->slave = 1;		// M/S=0, Slave 8259A
		if((vpic->icw4)&0x08) ;	// BUF=1, Buffer
		else ;			// BUF=0, Non-buffer
		if((vpic->icw4)&0x10) ;	// SFNM=1, Special Fully Nested Mode
		else ;			// SFNM=0, Non-special Fully Nested Mode

		if(vpic->slave) {
			vpic->icw3 &= 0x07;		// INT pin of slave connected to IR(vpic->icw3) of master
			if((vpic->icw3)&0x02) ;//vapiPrint("DEBUG:\tSlave connected to IR2.\n");
			else vapiPrint("PIC:\tCommand not supported: ICW3.\n");
		} else {
			if((vpic->icw3)&0x04) ;//vapiPrint("DEBUG:\tBit-2 has slave\n");
			else vapiPrint("PIC:\tCommand not supported: ICW3.\n");
		}
		vpic->init = OCW1;
		break;
	case OCW1:
		//vapiPrint("DEBUG:\tOCW1:%x\n",vpic->ocw1);
		vpic->imr = vcpu.al;
		break;
	default:
		break;	
	}
}
static void IO_Read_00x1(t_pic *vpic)
{
	// PIC 8259A Interrupt Mask Register
	// Read: IMR
	vcpu.al = vpic->imr;
}

void IO_Read_0020()	{IO_Read_00x0(&vpic1);}
void IO_Read_0021()	{IO_Read_00x1(&vpic1);}
void IO_Read_00A0()	{IO_Read_00x0(&vpic2);}
void IO_Read_00A1()	{IO_Read_00x1(&vpic2);}
void IO_Write_0020() {IO_Write_00x0(&vpic1);}
void IO_Write_0021() {IO_Write_00x1(&vpic1);}
void IO_Write_00A0() {IO_Write_00x0(&vpic2);}
void IO_Write_00A1() {IO_Write_00x1(&vpic2);}

t_nubit16 vpicGetIMR()
{return (((t_nubit16)vpic2.imr)<<8)+vpic1.imr;}
t_nubit16 vpicGetISR()
{return (((t_nubit16)vpic2.isr)<<8)+vpic1.isr;}
t_nubit16 vpicGetIRR()
{return (((t_nubit16)vpic2.irr)<<8)+vpic1.irr;}
void vpicSetIMR(t_nubit16 imr)
{
	vpic1.imr = imr&0xff;
	vpic2.imr = imr>>8;
}
void vpicSetISR(t_nubit16 isr)
{
	vpic1.isr = isr&0xff;
	vpic2.isr = isr>>8;
}
void vpicSetIRR(t_nubit16 irr)
{
	vpic1.irr = irr&0xff;
	vpic2.irr = irr>>8;
}

static t_nubit8 GetIntID(t_nubit8 reg)
{
	t_nubit8 id = 0;
	while(!((reg>>id)&0x01) && (id < 0x08)) id++;
	return id;
}
t_bool vpicIsINTR()
{
	t_nubit8 isid1,irid1,isid2,irid2;
	/* Device INT Begin */
	vpitIntTick();
	/* Device INT End */
	if(vpic2.irr&(~vpic2.imr)) vpic1.irr |= 0x04;
	irid1 = GetIntID(vpic1.irr&(~vpic1.imr));
	isid1 = GetIntID(vpic1.isr);
	//vapiPrint("irid1 = %d, isid1 = %d\n",irid1,isid1);
	if(irid1 < isid1) return 1;
	else if(irid1 == isid1 && irid1 == 2) {
		irid2 = GetIntID(vpic2.irr&(~vpic2.imr));
		isid2 = GetIntID(vpic2.isr);
		if(irid2 < isid2) return 1;
		else return 0;
	} else return 0;
}
t_nubit8 vpicGetINTR()
{
	t_nubit8 irid1,irid2;
	irid1 = GetIntID(vpic1.irr&(~vpic1.imr));
	if(irid1 == 2) {
		// IRQ2: IRQ8~IRQ15
		vpic1.isr |= 0x04;
		irid2 = GetIntID(vpic2.irr&(~vpic2.imr));
		vpic2.isr |= 1<<irid2;
		vpic2.irr |= 1<<irid2;
		if(GetIntID(vpic2.irr&(~vpic2.imr)) == 0x08) vpic1.irr ^= 0x04;
		irid2 |= vpic2.icw2;
		return irid2;
	} else {
		// IRQ0~IRQ1, IRQ3~IRQ7
		vpic1.isr |= 1<<irid1;
		vpic1.irr ^= 1<<irid1;
		irid1 |= vpic1.icw2;
		return irid1;
	}
}
/*void vpicRespondINTR()
{
	t_nubit8 i = 0;
	while(!((vpic1.isr>>i)&0x01) && (i < 0x08)) i++;
	if(i == 2) {
		i = 0;
		while(!((vpic2.isr>>i)&0x01) && (i < 0x08)) i++;
		vpic2.isr &= ~(1<<i);
		vpic2.irr &= ~(1<<i);
		i = 0;
		while(!((vpic2.isr>>i)&0x01) && (i < 0x08)) i++;
		if(i == 0x08) {
			vpic1.isr &= ~0x04;
			vpic1.irr &= ~0x04;
		}
	} else {
		vpic1.isr &= ~(1<<i);
		vpic1.irr &= ~(1<<i);
	}
}*/
void vpicSetIRQ(t_nubit8 irqid)
{
	switch(irqid) {
	case 0x00:	vpic1.irr |= 0x01;break;
	case 0x01:	vpic1.irr |= 0x02;break;
//	case 0x02:	vpic1.irr |= 0x04;break;
	case 0x03:	vpic1.irr |= 0x08;break;
	case 0x04:	vpic1.irr |= 0x10;break;
	case 0x05:	vpic1.irr |= 0x20;break;
	case 0x06:	vpic1.irr |= 0x40;break;
	case 0x07:	vpic1.irr |= 0x80;break;
	case 0x08:	vpic2.irr |= 0x01;vpic1.irr |= 0x04;break;
	case 0x09:	vpic2.irr |= 0x02;vpic1.irr |= 0x04;break;
	case 0x0a:	vpic2.irr |= 0x04;vpic1.irr |= 0x04;break;
	case 0x0b:	vpic2.irr |= 0x08;vpic1.irr |= 0x04;break;
	case 0x0c:	vpic2.irr |= 0x10;vpic1.irr |= 0x04;break;
	case 0x0d:	vpic2.irr |= 0x20;vpic1.irr |= 0x04;break;
	case 0x0e:	vpic2.irr |= 0x40;vpic1.irr |= 0x04;break;
	case 0x0f:	vpic2.irr |= 0x80;vpic1.irr |= 0x04;break;
	default:break;}
}

#define mov(n) (vcpu.al=n)
#define out(n) FUNEXEC(vcpuinsOutPort[n])

void PICInit()
{
	memset(&vpic1,0,sizeof(t_pic));
	memset(&vpic2,0,sizeof(t_pic));
	vpic1.init = vpic2.init = ICW1;
	vcpuinsInPort[0x0020] = (t_faddrcc)IO_Read_0020;
	vcpuinsInPort[0x0021] = (t_faddrcc)IO_Read_0021;
	vcpuinsInPort[0x00a0] = (t_faddrcc)IO_Read_00A0;
	vcpuinsInPort[0x00a1] = (t_faddrcc)IO_Read_00A1;
	vcpuinsOutPort[0x0020] = (t_faddrcc)IO_Write_0020;
	vcpuinsOutPort[0x0021] = (t_faddrcc)IO_Write_0021;
	vcpuinsOutPort[0x00a0] = (t_faddrcc)IO_Write_00A0;
	vcpuinsOutPort[0x00a1] = (t_faddrcc)IO_Write_00A1;
	// Initialization Start
	/* Should be done by BIOS */
	mov(0x11);	// ICW1: 0001 0001
	out(0x20);
	mov(0x08);	// ICW2: 0000 1000
	out(0x21);
	mov(0x04);	// ICW3: 0000 0100
	out(0x21);
	mov(0x05);	// ICW4: 0000 0101 (Why everyone says it's 0x01???)
	out(0x21);
	mov(0x11);	// ICW1: 0001 0001
	out(0xa0);
	mov(0x70);	// ICW2: 0111 0000
	out(0xa1);
	mov(0x02);	// ICW3: 0000 0010
	out(0xa1);
	mov(0x01);	// ICW4: 0000 0001 (Why not 0000 00101 for Master??)
	out(0xa1);
	// Initialization Ends
}
void PICTerm() {}