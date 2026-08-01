/* Copyright 2012-2014 Neko. */

/* MACHINE controls machine status. */

#include "vm/machine/device.h"
#include "platform/vm/platform.h"

#include "vm/machine/machine.h"

void machineStart() {
    machineReset();
    machineResume();
}

void machineReset() {
    deviceReset();
}

void machineStop() {
    deviceStop();
}

void machineResume() {
    platformStart();
}

void machineInit() {
    platformInit();
    deviceInit();
}

void machineFinal() {
    deviceFinal();
    platformFinal();
}
