# Makefile Maker [0.3.0045]
# Copyright (c) 2012 Neko. All rights reserved.

CC = gcc
CXX = g++
CFLAGS = -g -O
TARGET = bochx
LIBRARY = -lstdc++

$(TARGET): bochsapi.o nxvmapi.o vcpu.o vcpuins.o
	$(CC) -o $(TARGET) bochsapi.o nxvmapi.o vcpu.o vcpuins.o $(LIBRARY)

bochsapi.o: bochsapi.cc bochsapi.h ../src/device/vcpuins.h ../src/device/vcpu.h ../src/device/vglobal.h ../src/device/device.h ../src/utils.h ../src/global.h
	$(CXX) -c $(CFLAGS) bochsapi.cc

nxvmapi.o: nxvmapi.c ../src/device/vcpuins.h ../src/device/vcpu.h ../src/device/vpic.h ../src/device/vram.h ../src/device/vport.h ../src/device/vglobal.h ../src/device/device.h ../src/utils.h ../src/global.h
	$(CC) -c $(CFLAGS) nxvmapi.c

vcpu.o: ../src/device/vcpu.c ../src/device/vcpu.h ../src/device/vcpuins.h ../src/device/vglobal.h ../src/utils.h ../src/global.h
	$(CC) -c $(CFLAGS) ../src/device/vcpu.c

vcpuins.o: ../src/device/vcpuins.c ../src/device/vcpuins.h ../src/device/vcpu.h ../src/device/vpic.h ../src/device/vram.h ../src/device/vport.h ../src/device/vglobal.h ../src/device/device.h ../src/utils.h ../src/global.h
	$(CC) -c $(CFLAGS) ../src/device/vcpuins.c

clean:
	rm -f $(TARGET)
	rm -f *.o
