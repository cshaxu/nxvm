/* This file is a part of NekoVMac project. */

#include "vmachine.h"
#include "vcpu.h"
#include "vmemory.h"
#include "vdisplay.h"
#include "vbios.h"

void BIOSInit()
{}

void NVMTest()
{
	// POST
	CPUTest();
	MemoryTest();
	nvmprint("\nBIOS Power-On-Self-Test is done.\n");
}