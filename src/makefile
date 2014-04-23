# Makefile Maker [0.3.0043]
# Copyright (c) 2012 Neko. All rights reserved.

CC = gcc
CFLAGS = -g -O
TARGET = a.out

$(TARGET): console.o debug.o dosint.o main.o asm86.o vcmos.o vcpu.o vcpuins.o vdmac.o vfdc.o vfdd.o vmachine.o vpic.o vpit.o vram.o vapi.o win32app.o win32con.o
	$(CC) -o $(TARGET) console.o debug.o dosint.o main.o asm86.o vcmos.o vcpu.o vcpuins.o vdmac.o vfdc.o vfdd.o vmachine.o vpic.o vpit.o vram.o vapi.o win32app.o win32con.o


console.o: console.c vmachine/vmachine.h vmachine/vglobal.h vmachine/coption.h vmachine/vcpu.h vmachine/vram.h vmachine/vpic.h vmachine/vcmos.h vmachine/vpit.h vmachine/vfdd.h vmachine/vfdc.h vmachine/vdmac.h global.h debug.h console.h
	$(CC) -c $(CFLAGS) console.c

debug.o: debug.c vmachine/vmachine.h vmachine/vglobal.h vmachine/coption.h vmachine/vcpu.h vmachine/vram.h vmachine/vpic.h vmachine/vcmos.h vmachine/vpit.h vmachine/vfdd.h vmachine/vfdc.h vmachine/vdmac.h vmachine/vcpuins.h asm86/asm86.h global.h debug.h
	$(CC) -c $(CFLAGS) debug.c

dosint.o: dosint.c global.h dosint.h
	$(CC) -c $(CFLAGS) dosint.c

main.o: main.c vmachine/coption.h global.h console.h
	$(CC) -c $(CFLAGS) main.c

asm86.o: asm86/asm86.c asm86/asm86.h
	$(CC) -c $(CFLAGS) asm86/asm86.c

vcmos.o: vmachine/vcmos.c vmachine/vcpu.h vmachine/vglobal.h vmachine/coption.h vmachine/vcpuins.h vmachine/vcmos.h
	$(CC) -c $(CFLAGS) vmachine/vcmos.c

vcpu.o: vmachine/vcpu.c vmachine/vcpuins.h vmachine/vglobal.h vmachine/coption.h vmachine/vram.h vmachine/vcpu.h
	$(CC) -c $(CFLAGS) vmachine/vcpu.c

vcpuins.o: vmachine/vcpuins.c vmachine/vcpu.h vmachine/vglobal.h vmachine/coption.h vmachine/vram.h vmachine/vcpuins.h vmachine/vpic.h vmachine/system/vapi.h
	$(CC) -c $(CFLAGS) vmachine/vcpuins.c

vdmac.o: vmachine/vdmac.c vmachine/system/vapi.h vmachine/vcpu.h vmachine/vglobal.h vmachine/coption.h vmachine/vcpuins.h vmachine/vdmac.h
	$(CC) -c $(CFLAGS) vmachine/vdmac.c

vfdc.o: vmachine/vfdc.c vmachine/vcpu.h vmachine/vglobal.h vmachine/coption.h vmachine/vcpuins.h vmachine/vpic.h vmachine/vram.h vmachine/vdmac.h vmachine/vfdd.h vmachine/vfdc.h
	$(CC) -c $(CFLAGS) vmachine/vfdc.c

vfdd.o: vmachine/vfdd.c vmachine/vfdd.h vmachine/vglobal.h vmachine/coption.h vmachine/system/vapi.h
	$(CC) -c $(CFLAGS) vmachine/vfdd.c

vmachine.o: vmachine/vmachine.c vmachine/system/vapi.h vmachine/vmachine.h vmachine/vglobal.h vmachine/coption.h vmachine/vcpu.h vmachine/vram.h vmachine/vpic.h vmachine/vcmos.h vmachine/vpit.h vmachine/vfdd.h vmachine/vfdc.h vmachine/vdmac.h
	$(CC) -c $(CFLAGS) vmachine/vmachine.c

vpic.o: vmachine/vpic.c vmachine/system/vapi.h vmachine/vglobal.h vmachine/coption.h vmachine/vcpu.h vmachine/vcpuins.h vmachine/vpit.h vmachine/vpic.h
	$(CC) -c $(CFLAGS) vmachine/vpic.c

vpit.o: vmachine/vpit.c vmachine/system/vapi.h vmachine/vcpu.h vmachine/vglobal.h vmachine/coption.h vmachine/vpic.h vmachine/vcpuins.h vmachine/vpit.h
	$(CC) -c $(CFLAGS) vmachine/vpit.c

vram.o: vmachine/vram.c vmachine/vram.h vmachine/vglobal.h vmachine/coption.h
	$(CC) -c $(CFLAGS) vmachine/vram.c

vapi.o: vmachine/system/vapi.c vmachine/system/vapi.h
	$(CC) -c $(CFLAGS) vmachine/system/vapi.c

win32app.o: vmachine/system/win32app.c vmachine/system/win32app.h
	$(CC) -c $(CFLAGS) vmachine/system/win32app.c

win32con.o: vmachine/system/win32con.c vmachine/system/win32con.h
	$(CC) -c $(CFLAGS) vmachine/system/win32con.c

clean:
	rm -f $(TARGET)
	rm -f console.o debug.o dosint.o main.o asm86.o vcmos.o vcpu.o vcpuins.o vdmac.o vfdc.o vfdd.o vmachine.o vpic.o vpit.o vram.o vapi.o win32app.o win32con.o
