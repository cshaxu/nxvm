@echo off
cd vmachine
tfpack pack vmach vvadp.c vvadp.h vkbc.c vkbc.h vapi.c vapi.h vcmos.c vcmos.h vcpu.c vcpu.h vcpuins.c vcpuins.h vdma.c vdma.h vfdc.c vfdc.h vfdd.c vfdd.h vhdd.c vhdd.h vglobal.h vmachine.c vmachine.h vpic.c vpic.h vpit.c vpit.h vport.c vport.h vram.c vram.h
move vmach ..
cd system
tfpack pack sys linux.c linux.h
move sys ..\..
cd ..\bios
tfpack pack bio qdbios.c qdbios.h qdmisc.c qdmisc.h qdkeyb.c qdkeyb.h qdcga.c qdcga.h qddisk.c qddisk.h qdrtc.c qdrtc.h
move bio ..\..
cd ..\..\console
tfpack pack cons console.c console.h debug.c debug.h asm86.c asm86.h
move cons ..
cd ..
tfpack pack main main.c makefile
