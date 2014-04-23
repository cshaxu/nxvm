#Makefile Maker [0.3.0036]
#Copyright (c) 2012 Neko. All rights reserved.

CC = gcc
CFLAGS = -g -O
TARGET = a.out

$(TARGET): console.o debug.o dosint.o main.o asm86.o vcpu.o vcpuins.o vmachine.o vmemory.o vapi.o win32app.o win32con.o
	$(CC) -o $(TARGET) console.o debug.o dosint.o main.o asm86.o vcpu.o vcpuins.o vmachine.o vmemory.o vapi.o win32app.o win32con.o


console.o: ./console.c ./vmachine/vmachine.h ./vmachine/vglobal.h ./vmachine/coption.h ./vmachine/vcpu.h ./vmachine/vmemory.h ./global.h ./debug.h ./console.h
	$(CC) -c $(CFLAGS) ./console.c

debug.o: ./debug.c ./vmachine/vmachine.h ./vmachine/vglobal.h ./vmachine/coption.h ./vmachine/vcpu.h ./vmachine/vmemory.h ./vmachine/vcpuins.h ./asm86/asm86.h ./global.h ./debug.h
	$(CC) -c $(CFLAGS) ./debug.c

dosint.o: ./dosint.c ./global.h ./dosint.h
	$(CC) -c $(CFLAGS) ./dosint.c

main.o: ./main.c ./vmachine/coption.h ./global.h ./console.h
	$(CC) -c $(CFLAGS) ./main.c

asm86.o: ./asm86/asm86.c ./asm86/asm86.h
	$(CC) -c $(CFLAGS) ./asm86/asm86.c

vcpu.o: ./vmachine/vcpu.c ./vmachine/vcpuins.h ./vmachine/vglobal.h ./vmachine/coption.h ./vmachine/vmemory.h ./vmachine/vcpu.h
	$(CC) -c $(CFLAGS) ./vmachine/vcpu.c

vcpuins.o: ./vmachine/vcpuins.c ./vmachine/vcpu.h ./vmachine/vglobal.h ./vmachine/coption.h ./vmachine/vmemory.h ./vmachine/vcpuins.h ./vmachine/system/vapi.h ./vmachine/system/../vglobal.h ./vmachine/system/../coption.h
	$(CC) -c $(CFLAGS) ./vmachine/vcpuins.c

vmachine.o: ./vmachine/vmachine.c ./vmachine/system/vapi.h ./vmachine/system/../vglobal.h ./vmachine/system/../coption.h ./vmachine/vmachine.h ./vmachine/vglobal.h ./vmachine/coption.h ./vmachine/vcpu.h ./vmachine/vmemory.h
	$(CC) -c $(CFLAGS) ./vmachine/vmachine.c

vmemory.o: ./vmachine/vmemory.c ./vmachine/vmemory.h ./vmachine/vglobal.h ./vmachine/coption.h ./vmachine/system/vapi.h ./vmachine/system/../vglobal.h ./vmachine/system/../coption.h
	$(CC) -c $(CFLAGS) ./vmachine/vmemory.c

vapi.o: ./vmachine/system/vapi.c ./vmachine/system/../coption.h ./vmachine/system/vapi.h ./vmachine/system/../vglobal.h
	$(CC) -c $(CFLAGS) ./vmachine/system/vapi.c

win32app.o: ./vmachine/system/win32app.c ./vmachine/system/win32app.h
	$(CC) -c $(CFLAGS) ./vmachine/system/win32app.c

win32con.o: ./vmachine/system/win32con.c ./vmachine/system/win32con.h
	$(CC) -c $(CFLAGS) ./vmachine/system/win32con.c

clean:
	rm -f $(TARGET)
	rm -f console.o debug.o dosint.o main.o asm86.o vcpu.o vcpuins.o vmachine.o vmemory.o vapi.o win32app.o win32con.o
