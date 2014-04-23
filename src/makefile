# Makefile Maker [0.3.0045]
# Copyright (c) 2012 Neko. All rights reserved.

CC = gcc
CFLAGS = -g -O
TARGET = a.out
LIBRARY = -lpthread -lncurses

$(TARGET): main.o console.o vapi.o vcmos.o vcpu.o vcpuins.o vdma.o vfdc.o vfdd.o vhdd.o vkbc.o vmachine.o vpic.o vpit.o vport.o vram.o vvadp.o linux.o qdbios.o qdcga.o qddisk.o qdkeyb.o aasm32.o dasm32.o debug.o record.o
	$(CC) -o $(TARGET) main.o console.o vapi.o vcmos.o vcpu.o vcpuins.o vdma.o vfdc.o vfdd.o vhdd.o vkbc.o vmachine.o vpic.o vpit.o vport.o vram.o vvadp.o linux.o qdbios.o qdcga.o qddisk.o qdkeyb.o aasm32.o dasm32.o debug.o record.o $(LIBRARY)


main.o: main.c console.h
	$(CC) -c $(CFLAGS) main.c

console.o: console.c console.h
	$(CC) -c $(CFLAGS) console.c

vapi.o: vmachine/vapi.c vmachine/vmachine.h vmachine/vglobal.h  vmachine/vport.h vmachine/vram.h vmachine/vcpu.h vmachine/vpic.h vmachine/vcmos.h vmachine/vdma.h vmachine/vfdc.h vmachine/vfdd.h vmachine/vhdd.h vmachine/vpit.h vmachine/vkbc.h vmachine/vvadp.h vmachine/bios/qdbios.h vmachine/vapi.h vmachine/system/win32.h vmachine/system/linux.h
	$(CC) -c $(CFLAGS) vmachine/vapi.c

vcmos.o: vmachine/vcmos.c vmachine/vapi.h vmachine/vglobal.h  vmachine/vport.h vmachine/vcpu.h vmachine/vcmos.h
	$(CC) -c $(CFLAGS) vmachine/vcmos.c

vcpu.o: vmachine/vcpu.c vmachine/vcpuins.h vmachine/vcpu.h vmachine/vglobal.h 
	$(CC) -c $(CFLAGS) vmachine/vcpu.c

vcpuins.o: vmachine/vcpuins.c vmachine/vglobal.h  vmachine/vapi.h vmachine/vport.h vmachine/vram.h vmachine/vpic.h vmachine/bios/qdbios.h vmachine/vcpuapi.h vmachine/vcpu.h vmachine/vcpuins.h
	$(CC) -c $(CFLAGS) vmachine/vcpuins.c

vdma.o: vmachine/vdma.c vmachine/vapi.h vmachine/vglobal.h  vmachine/vport.h vmachine/vcpu.h vmachine/vram.h vmachine/vfdc.h vmachine/vdma.h
	$(CC) -c $(CFLAGS) vmachine/vdma.c

vfdc.o: vmachine/vfdc.c vmachine/vapi.h vmachine/vglobal.h  vmachine/vport.h vmachine/vpic.h vmachine/vdma.h vmachine/vfdd.h vmachine/vfdc.h
	$(CC) -c $(CFLAGS) vmachine/vfdc.c

vfdd.o: vmachine/vfdd.c vmachine/vapi.h vmachine/vglobal.h  vmachine/vdma.h vmachine/vfdd.h
	$(CC) -c $(CFLAGS) vmachine/vfdd.c

vhdd.o: vmachine/vhdd.c vmachine/vdma.h vmachine/vglobal.h  vmachine/vhdd.h
	$(CC) -c $(CFLAGS) vmachine/vhdd.c

vkbc.o: vmachine/vkbc.c vmachine/vport.h vmachine/vglobal.h  vmachine/vkbc.h
	$(CC) -c $(CFLAGS) vmachine/vkbc.c

vmachine.o: vmachine/vmachine.c vmachine/debug/aasm32.h vmachine/debug/dasm32.h vmachine/debug/record.h vmachine/debug/debug.h vmachine/vapi.h vmachine/vglobal.h  vmachine/vmachine.h vmachine/vport.h vmachine/vram.h vmachine/vcpu.h vmachine/vpic.h vmachine/vcmos.h vmachine/vdma.h vmachine/vfdc.h vmachine/vfdd.h vmachine/vhdd.h vmachine/vpit.h vmachine/vkbc.h vmachine/vvadp.h vmachine/bios/qdbios.h
	$(CC) -c $(CFLAGS) vmachine/vmachine.c

vpic.o: vmachine/vpic.c vmachine/vapi.h vmachine/vglobal.h  vmachine/vport.h vmachine/vpic.h
	$(CC) -c $(CFLAGS) vmachine/vpic.c

vpit.o: vmachine/vpit.c vmachine/vapi.h vmachine/vglobal.h  vmachine/vport.h vmachine/vpic.h vmachine/vpit.h
	$(CC) -c $(CFLAGS) vmachine/vpit.c

vport.o: vmachine/vport.c vmachine/vport.h vmachine/vglobal.h 
	$(CC) -c $(CFLAGS) vmachine/vport.c

vram.o: vmachine/vram.c vmachine/vport.h vmachine/vglobal.h  vmachine/vram.h
	$(CC) -c $(CFLAGS) vmachine/vram.c

vvadp.o: vmachine/vvadp.c vmachine/vport.h vmachine/vglobal.h  vmachine/vvadp.h
	$(CC) -c $(CFLAGS) vmachine/vvadp.c

linux.o: vmachine/system/linux.c vmachine/system/linux.h
	$(CC) -c $(CFLAGS) vmachine/system/linux.c $(LIBRARY)

qdbios.o: vmachine/bios/qdbios.c vmachine/bios/qdcga.h vmachine/bios/qdkeyb.h vmachine/bios/qddisk.h vmachine/bios/qdmisc.h vmachine/bios/qdbios.h vmachine/bios/qdrtc.h vmachine/bios/post.h
	$(CC) -c $(CFLAGS) vmachine/bios/qdbios.c

qdcga.o: vmachine/bios/qdcga.c vmachine/bios/qdbios.h vmachine/bios/qdcga.h
	$(CC) -c $(CFLAGS) vmachine/bios/qdcga.c

qddisk.o: vmachine/bios/qddisk.c vmachine/bios/qdbios.h vmachine/bios/qddisk.h
	$(CC) -c $(CFLAGS) vmachine/bios/qddisk.c

qdkeyb.o: vmachine/bios/qdkeyb.c vmachine/bios/qdbios.h vmachine/bios/qdkeyb.h
	$(CC) -c $(CFLAGS) vmachine/bios/qdkeyb.c

aasm32.o: vmachine/debug/aasm32.c vmachine/debug/debug.h vmachine/debug/aasm32.h
	$(CC) -c $(CFLAGS) vmachine/debug/aasm32.c

dasm32.o: vmachine/debug/dasm32.c vmachine/debug/debug.h vmachine/debug/dasm32.h
	$(CC) -c $(CFLAGS) vmachine/debug/dasm32.c

debug.o: vmachine/debug/debug.c vmachine/debug/aasm32.h vmachine/debug/dasm32.h vmachine/debug/debug.h
	$(CC) -c $(CFLAGS) vmachine/debug/debug.c

record.o: vmachine/debug/record.c vmachine/debug/record.h vmachine/debug/dasm32.h
	$(CC) -c $(CFLAGS) vmachine/debug/record.c

clean:
	rm -f $(TARGET)
	rm -f main.o console.o vapi.o vcmos.o vcpu.o vcpuins.o vdma.o vfdc.o vfdd.o vhdd.o vkbc.o vmachine.o vpic.o vpit.o vport.o vram.o vvadp.o linux.o qdbios.o qdcga.o qddisk.o qdkeyb.o aasm32.o dasm32.o debug.o record.o
