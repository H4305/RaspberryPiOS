ARMGNU ?= arm-none-eabi

CFLAGS = -I -Wall -nostdlib -fomit-frame-pointer -mno-apcs-frame -nostartfiles -ffreestanding -g -march=armv6z -marm -mthumb-interwork
ASFLAGS = -g -march=armv6z

BIN_DIR = bin/


C_FILES= os/kernel.c os/phyAlloc.c os/hw.c os/syscall.c sched/sched.c memory/vmem.c
AS_FILES=os/vectors.s

OBJ = $(patsubst %.s,%.o,$(AS_FILES))
OBJ += $(patsubst %.c,%.o,$(C_FILES))


OBJS = $(addprefix $(BIN_DIR),$(OBJ))
MEMMAP = os/memmap

.PHONY: gcc clean

gcc : kernel

clean :
	rm -rf bin
	rm -rf */*.bin
	rm -rf */*.hex
	rm -rf */*.elf
	rm -rf */*.list
	rm -rf */*.img
	rm -rf */*.bc
	rm -rf */*.clang.opt.s
	rm -rf */*~

$(BIN_DIR)%.o : %.c
	mkdir -p $(dir $@)
	$(ARMGNU)-gcc $(CFLAGS) -c $< -o $@

$(BIN_DIR)%.o : %.s
	mkdir -p $(dir $@)
	$(ARMGNU)-as $(ASFLAGS) $< -o $@

kernel : $(MEMMAP) $(OBJS)
	$(ARMGNU)-ld $(OBJS) -T $(MEMMAP) -o $(BIN_DIR)kernel.elf
	$(ARMGNU)-objdump -D $(BIN_DIR)kernel.elf > $(BIN_DIR)kernel.list
	$(ARMGNU)-objcopy $(BIN_DIR)kernel.elf -O binary $(BIN_DIR)kernel.img
	$(ARMGNU)-objcopy $(BIN_DIR)kernel.elf -O ihex $(BIN_DIR)kernel.hex

student : clean
	cd .. && ./build_student_archive.sh
