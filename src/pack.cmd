@echo off
cd vmachine
tfpack pack vmachine.pack vvadp.c vvadp.h vkbc.c vkbc.h vapi.c vapi.h vcmos.c vcmos.h vcpu.c vcpu.h vcpuins.c vcpuins.h vdma.c vdma.h vfdc.c vfdc.h vfdd.c vfdd.h vhdd.c vhdd.h vglobal.h vmachine.c vmachine.h vpic.c vpic.h vpit.c vpit.h vport.c vport.h vram.c vram.h
move vmachine.pack ..
cd system
tfpack pack system.pack linux.c linux.h
move system.pack ..\..
cd ..\bios
tfpack pack bios.pack post.h qdbios.c qdbios.h qdmisc.h qdrtc.h qdkeyb.c qdkeyb.h qdcga.c qdcga.h qddisk.c qddisk.h
move bios.pack ..\..
cd ..\debug
tfpack pack debug.pack debug.c debug.h aasm.c aasm.h dasm.c dasm.h
move debug.pack ..\..
cd ..\..\console
tfpack pack console.pack console.c console.h
move console.pack ..
cd ..
tfpack pack main.pack main.c makefile
