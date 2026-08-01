/* Copyright 2012-2014 Neko. */

/* MACHINE controls machine status. */

#include "vm/machine/device.h"
#include "core/product/wait.h"
#include "vm/platform/platform.h"

#include "vm/composition_display.h"

#include "vm/composition_machine.h"

static void vm_composition_wait(void *context, uint32_t milliseconds)
{
    (void)context;
    platformSleep(milliseconds);
}

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
    core_product_wait_bind(vm_composition_wait, NULL);
    deviceInit();
}

void machineFinal() {
    deviceFinal();
    core_product_wait_bind(NULL, NULL);
    platformFinal();
}
