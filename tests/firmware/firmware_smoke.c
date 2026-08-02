#include <stdio.h>
#include "core/machine/firmware_interface.h"
int main(void) {
 nxvm_firmware f; nxvm_firmware_service_descriptor post={"post",NXVM_FIRMWARE_SERVICE_POST,20u,0u}; nxvm_firmware_service_descriptor rom={"rom",NXVM_FIRMWARE_SERVICE_ROM,10u,0u}; nxvm_firmware_service_descriptor in={"int16",NXVM_FIRMWARE_SERVICE_INTERRUPT,30u,0x16u}; nxvm_firmware_service_descriptor duplicate={"other",NXVM_FIRMWARE_SERVICE_INTERRUPT,40u,0x16u}; nxvm_firmware_initialize(&f);
 if (nxvm_firmware_register_service(&f,&post) || nxvm_firmware_register_service(&f,&rom) || nxvm_firmware_register_service(&f,&in) || nxvm_firmware_service_at(&f,0u)!=&rom || nxvm_firmware_register_service(&f,&duplicate)!=NXVM_CORE_STATUS_UNSUPPORTED || nxvm_firmware_freeze(&f) || nxvm_firmware_register_service(&f,&post)!=NXVM_CORE_STATUS_INVALID_STATE) return 1;
 puts("M5:T4:S1:FIRMWARE:OK");
 return 0; }
