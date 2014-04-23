@echo off
cd vmachine
tfpack pack vmach qdbios.c qdbios.h qdcga.c qdcga.h qdkeyb.c qdkeyb.h vapi.c vapi.h vcmos.c vcmos.h vcpu.c vcpu.h vcpuins.c vcpuins.h vdma.c vdma.h vfdc.c vfdc.h vfdd.c vfdd.h vglobal.h vmachine.c vmachine.h vpic.c vpic.h vpit.c vpit.h vport.c vport.h vram.c vram.h
move vmach ..
cd system
tfpack pack sys linux.c linux.h
move sys ..\..
cd ..\..\console
tfpack pack cons console.c console.h debug.c debug.h asm86.c asm86.h
move cons ..
cd ..
tfpack pack main main.c makefile
