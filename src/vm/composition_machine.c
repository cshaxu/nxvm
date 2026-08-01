/* Copyright 2012-2014 Neko. */

/* MACHINE controls machine status. */

#include "vm/machine/device.h"
#include "vm/platform/platform.h"

#include "vm/composition_display.h"

#include "vm/composition_machine.h"

void machineStart() {
    machineReset();
    machineResume();
}

void machineReset() {
    deviceReset();
    if (!device.flagRun) vm_composition_publish_display(1);
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
