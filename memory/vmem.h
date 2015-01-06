#ifndef _VMEM_H_
#define _VMEM_H_

#include "../os/types.h"

#define INDEX_PAGE_SIZE 4

#define PAGE_SIZE 4096
#define SECON_LVL_TT_COUN 256
#define SECON_LVL_TT_SIZE SECON_LVL_TT_COUN * INDEX_PAGE_SIZE

#define FIRST_LVL_TT_COUN 4096
#define FIRST_LVL_TT_SIZE FIRST_LVL_TT_COUN * INDEX_PAGE_SIZE
#define TOTAL_TT_SIZE FIRST_LVL_TT_SIZE + FIRST_LVL_TT_COUN * SECON_LVL_TT_SIZE 

#define FIRST_LVL_TT_ADDR 0x48000
#define SECON_LVL_TT_ADDR FIRST_LVL_TT_ADDR + FIRST_LVL_TT_SIZE
	
unsigned int init_kern_translation_table(void);
	
#endif
