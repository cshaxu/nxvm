# Makefile Maker [0.3.0045]
# Copyright (c) 2012 Neko. All rights reserved.

CC = gcc
CFLAGS = -g -O
TARGET = a.out

$(TARGET): main.o asm86.o console.o debug.o qdbios.o qdfdd.o qdkeyb.o qdrtc.o qdvga.o vapi.o vcpu.o vcpuins.o vmachine.o vpic.o vport.o vram.o w32adisp.o w32akeyb.o win32app.o
	$(CC) -o $(TARGET) main.o asm86.o console.o debug.o qdbios.o qdfdd.o qdkeyb.o qdrtc.o qdvga.o vapi.o vcpu.o vcpuins.o vmachine.o vpic.o vport.o vram.o w32adisp.o w32akeyb.o win32app.o


main.o: main.c console/console.h
	$(CC) -c $(CFLAGS) main.c

asm86.o: console/asm86.c console/asm86.h
	$(CC) -c $(CFLAGS) console/asm86.c

console.o: console/console.c console/debug.h console/console.h
	$(CC) -c $(CFLAGS) console/console.c

debug.o: console/debug.c console/asm86.h console/debug.h
	$(CC) -c $(CFLAGS) console/debug.c

qdbios.o: vmachine/qdbios.c vmachine/vapi.h vmachine/vglobal.h vmachine/vcpu.h vmachine/vram.h vmachine/qdvga.h vmachine/qdfdd.h vmachine/qdkeyb.h vmachine/qdrtc.h vmachine/qdbios.h
	$(CC) -c $(CFLAGS) vmachine/qdbios.c

qdfdd.o: vmachine/qdfdd.c vmachine/vcpu.h vmachine/vglobal.h vmachine/vram.h vmachine/qdfdd.h
	$(CC) -c $(CFLAGS) vmachine/qdfdd.c

qdkeyb.o: vmachine/qdkeyb.c vmachine/vapi.h vmachine/vglobal.h vmachine/vport.h vmachine/vcpu.h vmachine/vpic.h vmachine/qdbios.h vmachine/qdkeyb.h vmachine/vram.h
	$(CC) -c $(CFLAGS) vmachine/qdkeyb.c

qdrtc.o: vmachine/qdrtc.c vmachine/vcpuins.h vmachine/vglobal.h vmachine/vapi.h vmachine/vport.h vmachine/vcpu.h vmachine/vram.h vmachine/qdrtc.h
	$(CC) -c $(CFLAGS) vmachine/qdrtc.c

qdvga.o: vmachine/qdvga.c vmachine/vapi.h vmachine/vglobal.h vmachine/vcpu.h vmachine/qdvga.h vmachine/vram.h
	$(CC) -c $(CFLAGS) vmachine/qdvga.c

vapi.o: vmachine/vapi.c vmachine/vmachine.h vmachine/vglobal.h vmachine/vport.h vmachine/vcpu.h vmachine/vram.h vmachine/vpic.h vmachine/qdbios.h vmachine/qdfdd.h vmachine/vapi.h vmachine/system/win32app.h
	$(CC) -c $(CFLAGS) vmachine/vapi.c

vcpu.o: vmachine/vcpu.c vmachine/vcpuins.h vmachine/vglobal.h vmachine/vcpu.h vmachine/vapi.h
	$(CC) -c $(CFLAGS) vmachine/vcpu.c

vcpuins.o: vmachine/vcpuins.c vmachine/vapi.h vmachine/vglobal.h vmachine/vport.h vmachine/vcpu.h vmachine/vram.h vmachine/vpic.h vmachine/vcpuins.h vmachine/qdbios.h
	$(CC) -c $(CFLAGS) vmachine/vcpuins.c

vmachine.o: vmachine/vmachine.c vmachine/vmachine.h vmachine/vglobal.h vmachine/vport.h vmachine/vcpu.h vmachine/vram.h vmachine/vpic.h vmachine/qdbios.h vmachine/qdfdd.h vmachine/vapi.h
	$(CC) -c $(CFLAGS) vmachine/vmachine.c

vpic.o: vmachine/vpic.c vmachine/vglobal.h vmachine/vport.h vmachine/vcpu.h vmachine/vpic.h vmachine/vapi.h
	$(CC) -c $(CFLAGS) vmachine/vpic.c

vport.o: vmachine/vport.c vmachine/vport.h vmachine/vglobal.h
	$(CC) -c $(CFLAGS) vmachine/vport.c

vram.o: vmachine/vram.c vmachine/vram.h vmachine/vglobal.h
	$(CC) -c $(CFLAGS) vmachine/vram.c

w32adisp.o: vmachine/system/w32adisp.c vmachine/system/win32app.h vmachine/system/w32adisp.h
	$(CC) -c $(CFLAGS) vmachine/system/w32adisp.c

w32akeyb.o: vmachine/system/w32akeyb.c vmachine/system/win32app.h vmachine/system/w32akeyb.h
	$(CC) -c $(CFLAGS) vmachine/system/w32akeyb.c

win32app.o: vmachine/system/win32app.c vmachine/system/w32adisp.h vmachine/system/w32akeyb.h vmachine/system/win32app.h
	$(CC) -c $(CFLAGS) vmachine/system/win32app.c

clean:
	rm -f $(TARGET)
	rm -f main.o asm86.o console.o debug.o qdbios.o qdfdd.o qdkeyb.o qdrtc.o qdvga.o vapi.o vcpu.o vcpuins.o vmachine.o vpic.o vport.o vram.o w32adisp.o w32akeyb.o win32app.o
