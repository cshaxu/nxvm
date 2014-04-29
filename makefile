# Makefile Maker [0.3.0045]
# Copyright (c) 2012 Neko. All rights reserved.

CC = gcc
CFLAGS = -g -O
TARGET = a.out
LIBRARY = -lpthread -lncurses

$(TARGET): console.o machine.o main.o utils.o device.o vbios.o vcmos.o vcpu.o vcpuins.o vdebug.o vdma.o vfdc.o vfdd.o vhdc.o vhdd.o vkbc.o vmachine.o vpic.o vpit.o vport.o vram.o vvadp.o aasm32.o dasm32.o debug.o record.o qdcga.o qddisk.o qdkeyb.o qdx.o platform.o linux.o linuxcon.o
	$(CC) -o $(TARGET) console.o machine.o main.o utils.o device.o vbios.o vcmos.o vcpu.o vcpuins.o vdebug.o vdma.o vfdc.o vfdd.o vhdc.o vhdd.o vkbc.o vmachine.o vpic.o vpit.o vport.o vram.o vvadp.o aasm32.o dasm32.o debug.o record.o qdcga.o qddisk.o qdkeyb.o qdx.o platform.o linux.o linuxcon.o $(LIBRARY)


console.o: src/console.c src/utils.h src/global.h src/device/vram.h src/device/vglobal.h src/device/vcpu.h src/device/vfdd.h src/device/vhdd.h src/device/vdebug.h src/device/vmachine.h src/debug.h src/device/debug/record.h src/device/device.h src/platform/platform.h src/machine.h src/console.h
	$(CC) -c $(CFLAGS) src/console.c

machine.o: src/machine.c src/platform/platform.h src/device/device.h src/device/vglobal.h src/machine.h src/global.h
	$(CC) -c $(CFLAGS) src/machine.c

main.o: src/main.c src/utils.h src/global.h src/console.h
	$(CC) -c $(CFLAGS) src/main.c

utils.o: src/utils.c src/platform/platform.h src/utils.h src/global.h
	$(CC) -c $(CFLAGS) src/utils.c

device.o: src/device/device.c src/device/vfdd.h src/device/vglobal.h src/device/vcpu.h src/device/vpic.h src/device/vpit.h src/device/vdma.h src/device/vfdc.h src/device/vmachine.h src/device/device.h
	$(CC) -c $(CFLAGS) src/device/device.c

vbios.o: src/device/vbios.c src/device/vram.h src/device/vglobal.h src/device/vmachine.h src/device/vbios.h src/device/vhdd.h
	$(CC) -c $(CFLAGS) src/device/vbios.c

vcmos.o: src/device/vcmos.c src/device/vmachine.h src/device/vglobal.h src/device/vport.h src/device/vcpu.h src/device/vbios.h src/device/vcmos.h
	$(CC) -c $(CFLAGS) src/device/vcmos.c

vcpu.o: src/device/vcpu.c src/device/vmachine.h src/device/vglobal.h src/device/vcpuins.h src/device/vcpu.h
	$(CC) -c $(CFLAGS) src/device/vcpu.c

vcpuins.o: src/device/vcpuins.c src/device/device.h src/device/vglobal.h src/device/vport.h src/device/vram.h src/device/vpic.h src/device/qdx/qdx.h src/device/vcpuapi.h src/device/vcpu.h src/device/vcpuins.h
	$(CC) -c $(CFLAGS) src/device/vcpuins.c

vdebug.o: src/device/vdebug.c src/utils.h src/device/vcpu.h src/device/vcpuins.h src/device/vmachine.h src/device/device.h src/device/vdebug.h
	$(CC) -c $(CFLAGS) src/device/vdebug.c

vdma.o: src/device/vdma.c src/device/vmachine.h src/device/vglobal.h src/device/vport.h src/device/vram.h src/device/vbios.h src/device/vfdc.h src/device/vdma.h
	$(CC) -c $(CFLAGS) src/device/vdma.c

vfdc.o: src/device/vfdc.c src/device/vmachine.h src/device/vglobal.h src/device/vport.h src/device/vbios.h src/device/vpic.h src/device/vdma.h src/device/vfdd.h src/device/vfdc.h
	$(CC) -c $(CFLAGS) src/device/vfdc.c

vfdd.o: src/device/vfdd.c src/utils.h src/device/vmachine.h src/device/vglobal.h src/device/vdma.h src/device/vfdd.h
	$(CC) -c $(CFLAGS) src/device/vfdd.c

vhdc.o: src/device/vhdc.c src/device/vmachine.h src/device/vglobal.h src/device/vbios.h src/device/vhdc.h
	$(CC) -c $(CFLAGS) src/device/vhdc.c

vhdd.o: src/device/vhdd.c src/utils.h src/device/vmachine.h src/device/vglobal.h src/device/vdma.h src/device/vhdd.h
	$(CC) -c $(CFLAGS) src/device/vhdd.c

vkbc.o: src/device/vkbc.c src/device/vmachine.h src/device/vglobal.h src/device/vport.h src/device/vbios.h src/device/vkbc.h
	$(CC) -c $(CFLAGS) src/device/vkbc.c

vmachine.o: src/device/vmachine.c src/device/debug/record.h src/debug.h src/device/vport.h src/device/vglobal.h src/device/vram.h src/device/vcpu.h src/device/vbios.h src/device/vpic.h src/device/vpit.h src/device/vcmos.h src/device/vdma.h src/device/vfdc.h src/device/vfdd.h src/device/vhdc.h src/device/vhdd.h src/device/vkbc.h src/device/vvadp.h src/device/qdx/qdx.h src/device/vmachine.h
	$(CC) -c $(CFLAGS) src/device/vmachine.c

vpic.o: src/device/vpic.c src/device/vmachine.h src/device/vglobal.h src/device/vport.h src/device/vbios.h src/device/vpic.h
	$(CC) -c $(CFLAGS) src/device/vpic.c

vpit.o: src/device/vpit.c src/device/vmachine.h src/device/vglobal.h src/device/vport.h src/device/vbios.h src/device/vpic.h src/device/vpit.h
	$(CC) -c $(CFLAGS) src/device/vpit.c

vport.o: src/device/vport.c src/device/vmachine.h src/device/vglobal.h src/device/vport.h
	$(CC) -c $(CFLAGS) src/device/vport.c

vram.o: src/device/vram.c src/device/vmachine.h src/device/vglobal.h src/device/vport.h src/device/vram.h
	$(CC) -c $(CFLAGS) src/device/vram.c

vvadp.o: src/device/vvadp.c src/device/vmachine.h src/device/vglobal.h src/device/vport.h src/device/vbios.h src/device/vvadp.h
	$(CC) -c $(CFLAGS) src/device/vvadp.c

aasm32.o: src/xasm32/aasm32.c src/utils.h src/xasm32/aasm32.h
	$(CC) -c $(CFLAGS) src/xasm32/aasm32.c

dasm32.o: src/xasm32/dasm32.c src/utils.h src/xasm32/dasm32.h
	$(CC) -c $(CFLAGS) src/xasm32/dasm32.c

debug.o: src/debug.c src/utils.h src/machine.h src/debug.h
	$(CC) -c $(CFLAGS) src/debug.c

record.o: src/device/debug/record.c src/utils.h src/device/debug/record.h
	$(CC) -c $(CFLAGS) src/device/debug/record.c

qdcga.o: src/device/qdx/qdcga.c src/platform/platform.h src/device/qdx/qdx.h src/device/qdx/qdcga.h
	$(CC) -c $(CFLAGS) src/device/qdx/qdcga.c

qddisk.o: src/device/qdx/qddisk.c src/device/qdx/qdx.h src/device/qdx/qddisk.h
	$(CC) -c $(CFLAGS) src/device/qdx/qddisk.c

qdkeyb.o: src/device/qdx/qdkeyb.c src/utils.h src/device/qdx/qdx.h src/device/qdx/qdkeyb.h
	$(CC) -c $(CFLAGS) src/device/qdx/qdkeyb.c

qdx.o: src/device/qdx/qdx.c src/device/qdx/qdcga.h src/device/qdx/qdkeyb.h src/device/qdx/qddisk.h src/device/qdx/qdx.h
	$(CC) -c $(CFLAGS) src/device/qdx/qdx.c

platform.o: src/platform/platform.c src/platform/platform.h src/platform/win32/win32.h src/platform/linux/linux.h
	$(CC) -c $(CFLAGS) src/platform/platform.c

linux.o: src/platform/linux/linux.c src/platform/linux/linux.h src/platform/linux/linuxcon.h
	$(CC) -c $(CFLAGS) src/platform/linux/linux.c

linuxcon.o: src/platform/linux/linuxcon.c src/utils.h src/device/device.h src/platform/linux/linuxcon.h
	$(CC) -c $(CFLAGS) src/platform/linux/linuxcon.c $(LIBRARY)

clean:
	rm -f $(TARGET)
	rm -f *.o
