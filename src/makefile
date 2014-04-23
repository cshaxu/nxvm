# Makefile Maker [0.3.0045]
# Copyright (c) 2012 Neko. All rights reserved.

CC = gcc
CFLAGS = -g -O
TARGET = a.out

$(TARGET): main.o asm86.o console.o debug.o qdbios.o qdcga.o qdkeyb.o vapi.o vcmos.o vcpu.o vcpuins.o vdma.o vfdc.o vfdd.o vmachine.o vpic.o vpit.o vport.o vram.o w32adisp.o w32akeyb.o w32cdisp.o w32ckeyb.o win32app.o win32con.o
	$(CC) -o $(TARGET) main.o asm86.o console.o debug.o qdbios.o qdcga.o qdkeyb.o vapi.o vcmos.o vcpu.o vcpuins.o vdma.o vfdc.o vfdd.o vmachine.o vpic.o vpit.o vport.o vram.o w32adisp.o w32akeyb.o w32cdisp.o w32ckeyb.o win32app.o win32con.o


main.o: main.c console/console.h
	$(CC) -c $(CFLAGS) main.c

asm86.o: console/asm86.c console/asm86.h
	$(CC) -c $(CFLAGS) console/asm86.c

console.o: console/console.c console/asm86.h console/debug.h console/console.h
	$(CC) -c $(CFLAGS) console/console.c

debug.o: console/debug.c console/asm86.h console/debug.h
	$(CC) -c $(CFLAGS) console/debug.c

qdbios.o: vmachine/qdbios.c vmachine/vapi.h vmachine/vglobal.h vmachine/vcpu.h vmachine/vmachine.h vmachine/vport.h vmachine/vram.h vmachine/vpic.h vmachine/vcmos.h vmachine/vdma.h vmachine/vfdc.h vmachine/vfdd.h vmachine/vpit.h vmachine/qdbios.h vmachine/qdcga.h vmachine/qdkeyb.h
	$(CC) -c $(CFLAGS) vmachine/qdbios.c

qdcga.o: vmachine/qdcga.c vmachine/vapi.h vmachine/vglobal.h vmachine/vcpu.h vmachine/qdcga.h vmachine/vram.h
	$(CC) -c $(CFLAGS) vmachine/qdcga.c

qdkeyb.o: vmachine/qdkeyb.c vmachine/vapi.h vmachine/vglobal.h vmachine/vcpu.h vmachine/vport.h vmachine/vpic.h vmachine/qdbios.h vmachine/qdkeyb.h vmachine/vram.h
	$(CC) -c $(CFLAGS) vmachine/qdkeyb.c

vapi.o: vmachine/vapi.c vmachine/vmachine.h vmachine/vglobal.h vmachine/vport.h vmachine/vram.h vmachine/vcpu.h vmachine/vpic.h vmachine/vcmos.h vmachine/vdma.h vmachine/vfdc.h vmachine/vfdd.h vmachine/vpit.h vmachine/qdbios.h vmachine/vapi.h vmachine/system/win32con.h vmachine/system/win32app.h
	$(CC) -c $(CFLAGS) vmachine/vapi.c

vcmos.o: vmachine/vcmos.c vmachine/vapi.h vmachine/vglobal.h vmachine/vcpu.h vmachine/vport.h vmachine/vcmos.h
	$(CC) -c $(CFLAGS) vmachine/vcmos.c

vcpu.o: vmachine/vcpu.c vmachine/vapi.h vmachine/vglobal.h vmachine/vcpu.h vmachine/vcpuins.h
	$(CC) -c $(CFLAGS) vmachine/vcpu.c

vcpuins.o: vmachine/vcpuins.c vmachine/vapi.h vmachine/vglobal.h vmachine/vcpu.h vmachine/vport.h vmachine/vram.h vmachine/vpic.h vmachine/qdbios.h vmachine/vcpuins.h
	$(CC) -c $(CFLAGS) vmachine/vcpuins.c

vdma.o: vmachine/vdma.c vmachine/vapi.h vmachine/vglobal.h vmachine/vcpu.h vmachine/vport.h vmachine/vram.h vmachine/vfdc.h vmachine/vdma.h
	$(CC) -c $(CFLAGS) vmachine/vdma.c

vfdc.o: vmachine/vfdc.c vmachine/vapi.h vmachine/vglobal.h vmachine/vcpu.h vmachine/vport.h vmachine/vpic.h vmachine/vdma.h vmachine/vfdd.h vmachine/vfdc.h
	$(CC) -c $(CFLAGS) vmachine/vfdc.c

vfdd.o: vmachine/vfdd.c vmachine/vdma.h vmachine/vglobal.h vmachine/vfdd.h
	$(CC) -c $(CFLAGS) vmachine/vfdd.c

vmachine.o: vmachine/vmachine.c vmachine/vapi.h vmachine/vglobal.h vmachine/vcpu.h vmachine/vmachine.h vmachine/vport.h vmachine/vram.h vmachine/vpic.h vmachine/vcmos.h vmachine/vdma.h vmachine/vfdc.h vmachine/vfdd.h vmachine/vpit.h vmachine/qdbios.h
	$(CC) -c $(CFLAGS) vmachine/vmachine.c

vpic.o: vmachine/vpic.c vmachine/vapi.h vmachine/vglobal.h vmachine/vcpu.h vmachine/vport.h vmachine/vpic.h
	$(CC) -c $(CFLAGS) vmachine/vpic.c

vpit.o: vmachine/vpit.c vmachine/vapi.h vmachine/vglobal.h vmachine/vcpu.h vmachine/vport.h vmachine/vpic.h vmachine/vpit.h
	$(CC) -c $(CFLAGS) vmachine/vpit.c

vport.o: vmachine/vport.c vmachine/vport.h vmachine/vglobal.h
	$(CC) -c $(CFLAGS) vmachine/vport.c

vram.o: vmachine/vram.c vmachine/vram.h vmachine/vglobal.h
	$(CC) -c $(CFLAGS) vmachine/vram.c

w32adisp.o: vmachine/system/w32adisp.c vmachine/system/win32app.h vmachine/system/w32adisp.h
	$(CC) -c $(CFLAGS) vmachine/system/w32adisp.c

w32akeyb.o: vmachine/system/w32akeyb.c vmachine/system/win32app.h vmachine/system/w32akeyb.h
	$(CC) -c $(CFLAGS) vmachine/system/w32akeyb.c

w32cdisp.o: vmachine/system/w32cdisp.c vmachine/system/win32con.h vmachine/system/w32cdisp.h
	$(CC) -c $(CFLAGS) vmachine/system/w32cdisp.c

w32ckeyb.o: vmachine/system/w32ckeyb.c vmachine/system/win32con.h vmachine/system/w32ckeyb.h
	$(CC) -c $(CFLAGS) vmachine/system/w32ckeyb.c

win32app.o: vmachine/system/win32app.c vmachine/system/w32adisp.h vmachine/system/w32akeyb.h vmachine/system/win32app.h
	$(CC) -c $(CFLAGS) vmachine/system/win32app.c

win32con.o: vmachine/system/win32con.c vmachine/system/w32cdisp.h vmachine/system/w32ckeyb.h vmachine/system/win32con.h vmachine/system/win32app.h
	$(CC) -c $(CFLAGS) vmachine/system/win32con.c

clean:
	rm -f $(TARGET)
	rm -f main.o asm86.o console.o debug.o qdbios.o qdcga.o qdkeyb.o vapi.o vcmos.o vcpu.o vcpuins.o vdma.o vfdc.o vfdd.o vmachine.o vpic.o vpit.o vport.o vram.o w32adisp.o w32akeyb.o w32cdisp.o w32ckeyb.o win32app.o win32con.o
