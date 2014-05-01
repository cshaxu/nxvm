/* Copyright 2012-2014 Neko. */

/*
 * DEVICE provides the device interface to drive the device thread
 * and access the virtual devices.
 */

#include "../utils.h"

#include "vmachine.h"
#include "device.h"

t_device device;

/* Starts device thread */
void deviceStart() {
	device.flagRun = 1;
	device.flagFlip = 1 - device.flagFlip;
	while (device.flagRun) {
		if (device.flagReset) {
			vmachineReset();
			device.flagReset = 0;
		}
		vmachineRefresh();
	}
}

/* Issues resetting signal to device thread */
void deviceReset() {
	if (device.flagRun) {
		device.flagReset = 1;
	} else {
		vmachineReset();
		device.flagReset = 0;
	}
}

/* Issues stopping signal to device thread */
void deviceStop()  {device.flagRun = 0;}

/* Initializes devices */
void deviceInit() {
	MEMSET(&device, 0x00, sizeof(t_device));
	vmachineInit();
}

/* Finalizes devices */
void deviceFinal() {vmachineFinal();}
