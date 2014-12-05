ARMGNU ?= arm-none-eabi

CFLAGS = -I -Wall -nostdlib -fomit-frame-pointer -mno-apcs-frame -nostartfiles -ffreestanding -g -march=armv6z -marm -mthumb-interwork
ASFLAGS = -g -march=armv6z


C_FILES= os/kernel.c os/phyAlloc.c os/hw.c sched/sched.c os/syscall.c
AS_FILES=os/vectors.s

OBJS = $(patsubst %.s,%.o,$(AS_FILES))
OBJS += $(patsubst %.c,%.o,$(C_FILES))

.PHONY: gcc clean

gcc : kernel

clean :
	rm -rf *.o
	rm -rf *.bin
	rm -rf *.hex
	rm -rf *.elf
	rm -rf *.list
	rm -rf *.img
	rm -rf *.bc
	rm -rf *.clang.opt.s
	rm -rf *~

%.o : %.c
	$(ARMGNU)-gcc $(CFLAGS) -c $< -o $@

%.o : %.s
	$(ARMGNU)-as $(ASFLAGS) $< -o $@

kernel : os/memmap $(OBJS)
	$(ARMGNU)-ld $(OBJS) -T os/memmap -o kernel.elf
	$(ARMGNU)-objdump -D kernel.elf > kernel.list
	$(ARMGNU)-objcopy kernel.elf -O binary kernel.img
	$(ARMGNU)-objcopy kernel.elf -O ihex kernel.hex

student : clean
	cd .. && ./build_student_archive.sh
