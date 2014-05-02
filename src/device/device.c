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
	device.flagRun = True;
	device.flagFlip = True - device.flagFlip;
	while (device.flagRun) {
		if (device.flagReset) {
			vmachineReset();
			device.flagReset = False;
		}
		vmachineRefresh();
	}
}

/* Issues resetting signal to device thread */
void deviceReset() {
	if (device.flagRun) {
		device.flagReset = True;
	} else {
		vmachineReset();
		device.flagReset = False;
	}
}

/* Issues stopping signal to device thread */
void deviceStop()  {device.flagRun = False;}

/* Initializes devices */
void deviceInit() {
	MEMSET(&device, Zero8, sizeof(t_device));
	vmachineInit();
}

/* Finalizes devices */
void deviceFinal() {vmachineFinal();}
