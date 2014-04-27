/* Copyright 2012-2014 Neko. */

/* MACHINE controls machine status. */

#include "platform/platform.h"
#include "device/device.h"

#include "machine.h"

void machineStart() {
	machineReset();
	machineResume();
}

void machineReset() {deviceReset();}

void machineStop() {deviceStop();}

void machineResume() {platformStart();}

void machineInit() {
	platformInit();
	deviceInit();
}

void machineFinal() {
	deviceFinal();
	platformFinal();
}
