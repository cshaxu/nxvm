AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = nkvdm
nkvdm_SOURCES = src/main.c \
	src/exec.c src/exec.h \
	src/msdos.c src/msdos.h \
	src/dasm16s.c src/dasm16s.h \
	../src/machine.c ../src/machine.h \
	../src/utils.c ../src/utils.h \
	../src/debug.c ../src/debug.h \
	../src/xasm32/aasm32.c ../src/xasm32/aasm32.h \
	../src/xasm32/dasm32.c ../src/xasm32/dasm32.h \
	../src/platform/platform.c ../src/platform/platform.h \
	../src/platform/linux/linux.c ../src/platform/linux/linux.h \
	../src/platform/linux/linuxcon.c ../src/platform/linux/linuxcon.h \
	../src/device/vglobal.h \
	../src/device/device.c ../src/device/device.h \
	../src/device/vmachine.c ../src/device/vmachine.h \
	../src/device/vport.c ../src/device/vport.h \
	../src/device/vbios.c ../src/device/vbios.h \
	../src/device/vcmos.c ../src/device/vcmos.h \
	../src/device/vcpu.c ../src/device/vcpu.h \
	../src/device/vcpuins.c ../src/device/vcpuins.h \
	../src/device/vdebug.c ../src/device/vdebug.h \
	../src/device/vdma.c ../src/device/vdma.h \
	../src/device/vfdc.c ../src/device/vfdc.h \
	../src/device/vfdd.c ../src/device/vfdd.h \
	../src/device/vhdc.c ../src/device/vhdc.h \
	../src/device/vhdd.c ../src/device/vhdd.h \
	../src/device/vkbc.c ../src/device/vkbc.h \
	../src/device/vpic.c ../src/device/vpic.h \
	../src/device/vpit.c ../src/device/vpit.h \
	../src/device/vram.c ../src/device/vram.h \
	../src/device/vvadp.c ../src/device/vvadp.h \
	../src/device/qdx/qdx.c ../src/device/qdx/qdx.h \
	../src/device/qdx/qdcga.c ../src/device/qdx/qdcga.h \
	../src/device/qdx/qddisk.c ../src/device/qdx/qddisk.h \
	../src/device/qdx/qdkeyb.c ../src/device/qdx/qdkeyb.h
LIBS = -lpthread -lncurses
