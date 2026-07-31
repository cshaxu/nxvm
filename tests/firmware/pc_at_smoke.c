#include <stdio.h>
#include <string.h>
#include "firmware/pc_at.h"
int main(void) { nxvm_firmware f; nxvm_firmware_pc_at_plan p; nxvm_firmware_initialize(&f); if(nxvm_firmware_pc_at_compose(&f,&p)!=NXVM_CORE_STATUS_OK||p.reset_segment!=0xf000u||p.reset_offset!=0xfff0u||p.service_count!=6u||strcmp(nxvm_firmware_service_at(&f,2u)->id,"bios.int10.video")!=0||nxvm_firmware_freeze(&f)!=NXVM_CORE_STATUS_OK)return 1; puts("M5:T4:S2:PC-AT-FIRMWARE:OK"); return 0; }
