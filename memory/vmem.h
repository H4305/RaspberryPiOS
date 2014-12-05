#ifndef _VMEM_H_
#define _VMEM_H_

#define PAGE_SIZE 4096
#define SECON_LVL_TT_COUN 256
#define SECON_LVL_TT_SIZE SECON_LVL_TT_COUN * 4

#define FIRST_LVL_TT_COUN 4096
#define FIRST_LVL_TT_SIZE FIRST_LVL_TT_COUN * 4
#define TOTAL_TT_SIZE FIRST_LVL_TT_SIZE + ( FIRST_LVL_TT_SIZE * SECON_LVL_TT_SIZE )

#define FIRST_LVL_TT_ADDR 0x48000
#define SECON_LVL_TT_ADDR FIRST_LVL_TT_ADDR + FIRST_LVL_TT_COUN


uint32_t device_flags = 
	(0 << 0) | // XN
	(1 << 1) | // Always 1
	(1 << 2) | // B
	(0 << 3) | // C
	(00 << 4) | // AP
	(000 << 6) | // TEX
	(0 << 9) | // APX
	(0 << 10) | // S	
	(0 << 11); // NG
	
unsigned int init_kern_translation_table(void);
	
#endif
