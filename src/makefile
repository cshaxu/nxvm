# Makefile Maker [0.3.0045]
# Copyright (c) 2012 Neko. All rights reserved.

CC = gcc
CFLAGS = -g -O
TARGET = a.out

$(TARGET): main.o asm86.o console.o debug.o vbios.o vcmos.o vcpu.o vcpuins.o vdma.o vfdc.o vfdd.o vkbc.o vkeyb.o vmachine.o vpic.o vpit.o vram.o vapi.o
	$(CC) -o $(TARGET) main.o asm86.o console.o debug.o vbios.o vcmos.o vcpu.o vcpuins.o vdma.o vfdc.o vfdd.o vkbc.o vkeyb.o vmachine.o vpic.o vpit.o vram.o vapi.o


main.o: main.c console/console.h
	$(CC) -c $(CFLAGS) main.c

asm86.o:
	$(CC) -c $(CFLAGS) asm86/asm86.c

console.o:
	$(CC) -c $(CFLAGS) asm86/console.c

debug.o:
	$(CC) -c $(CFLAGS) asm86/debug.c

vbios.o: vmachine/vbios.c vmachine/vbios.h vmachine/vglobal.h
	$(CC) -c $(CFLAGS) vmachine/vbios.c

vcmos.o: vmachine/vcmos.c vmachine/vcpu.h vmachine/vglobal.h vmachine/vcpuins.h vmachine/vcmos.h
	$(CC) -c $(CFLAGS) vmachine/vcmos.c

vcpu.o: vmachine/vcpu.c vmachine/vcpuins.h vmachine/vglobal.h vmachine/vcpu.h
	$(CC) -c $(CFLAGS) vmachine/vcpu.c

vcpuins.o: vmachine/vcpuins.c vmachine/vcpu.h vmachine/vglobal.h vmachine/vram.h vmachine/vcpuins.h vmachine/vpic.h
	$(CC) -c $(CFLAGS) vmachine/vcpuins.c

vdma.o: vmachine/vdma.c vmachine/vcpu.h vmachine/vglobal.h vmachine/vcpuins.h vmachine/vram.h vmachine/vfdc.h vmachine/vdma.h
	$(CC) -c $(CFLAGS) vmachine/vdma.c

vfdc.o: vmachine/vfdc.c vmachine/vcpu.h vmachine/vglobal.h vmachine/vcpuins.h vmachine/vpic.h vmachine/vram.h vmachine/vdma.h vmachine/vfdd.h vmachine/vfdc.h
	$(CC) -c $(CFLAGS) vmachine/vfdc.c

vfdd.o: vmachine/vfdd.c vmachine/vdma.h vmachine/vglobal.h vmachine/vfdd.h
	$(CC) -c $(CFLAGS) vmachine/vfdd.c

vkbc.o: vmachine/vkbc.c vmachine/vcpuins.h vmachine/vglobal.h vmachine/vcpu.h vmachine/vpic.h vmachine/vkeyb.h vmachine/vkbc.h
	$(CC) -c $(CFLAGS) vmachine/vkbc.c

vkeyb.o: vmachine/vkeyb.c vmachine/vkbc.h vmachine/vglobal.h vmachine/vkeyb.h
	$(CC) -c $(CFLAGS) vmachine/vkeyb.c

vmachine.o: vmachine/vmachine.c vmachine/vmachine.h vmachine/vglobal.h vmachine/vcpu.h vmachine/vram.h vmachine/vpic.h vmachine/vpit.h vmachine/vcmos.h vmachine/vdma.h vmachine/vfdc.h vmachine/vfdd.h vmachine/vkbc.h vmachine/vkeyb.h
	$(CC) -c $(CFLAGS) vmachine/vmachine.c

vpic.o: vmachine/vpic.c vmachine/vglobal.h vmachine/vcpu.h vmachine/vcpuins.h vmachine/vpit.h vmachine/vpic.h
	$(CC) -c $(CFLAGS) vmachine/vpic.c

vpit.o: vmachine/vpit.c vmachine/vcpu.h vmachine/vglobal.h vmachine/vpic.h vmachine/vcpuins.h vmachine/vpit.h
	$(CC) -c $(CFLAGS) vmachine/vpit.c

vram.o: vmachine/vram.c vmachine/vram.h vmachine/vglobal.h
	$(CC) -c $(CFLAGS) vmachine/vram.c

vapi.o: system/vapi.c system/vapi.h
	$(CC) -c $(CFLAGS) system/vapi.c

clean:
	rm -f $(TARGET)
	rm -f main.o asm86.o console.o debug.o vbios.o vcmos.o vcpu.o vcpuins.o vdma.o vfdc.o vfdd.o vkbc.o vkeyb.o vmachine.o vpic.o vpit.o vram.o vapi.o
