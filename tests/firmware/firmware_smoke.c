#include <stdio.h>
#include "core/machine/firmware_interface.h"
int main(void) {
 core_machine_firmware f; core_machine_firmware_service_descriptor post={"post",CORE_MACHINE_FIRMWARE_SERVICE_POST,20u,0u}; core_machine_firmware_service_descriptor rom={"rom",CORE_MACHINE_FIRMWARE_SERVICE_ROM,10u,0u}; core_machine_firmware_service_descriptor in={"int16",CORE_MACHINE_FIRMWARE_SERVICE_INTERRUPT,30u,0x16u}; core_machine_firmware_service_descriptor duplicate={"other",CORE_MACHINE_FIRMWARE_SERVICE_INTERRUPT,40u,0x16u}; core_machine_firmware_initialize(&f);
 if (core_machine_firmware_register_service(&f,&post) || core_machine_firmware_register_service(&f,&rom) || core_machine_firmware_register_service(&f,&in) || core_machine_firmware_service_at(&f,0u)!=&rom || core_machine_firmware_register_service(&f,&duplicate)!=NTVDM64_STATUS_UNSUPPORTED || core_machine_firmware_freeze(&f) || core_machine_firmware_register_service(&f,&post)!=NTVDM64_STATUS_INVALID_STATE) return 1;
 puts("M5:T4:S1:FIRMWARE:OK");
 return 0; }
