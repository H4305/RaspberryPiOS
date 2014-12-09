#include "vmem.h"

uint32_t first_tt_flags =
	(1 << 0) | //1
	(0 << 1) | //0
	(0 << 2) | //SBZ
	(0 << 3) | //NS
	(0 << 4) | //SBZ
	(0000 << 5) | //Domain
	(0 << 9); //P - not supported

uint32_t second_tt_flags = 
	(0 << 0) | // XN
	(1 << 1) | // Always 1
	(0 << 2) | // B
	(0 << 3) | // C
	(00 << 4) | // AP
	(001 << 6) | // TEX
	(0 << 9) | // APX
	(0 << 10) | // S	
	(0 << 11); // NG

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



 
unsigned int init_kern_translation_table(void) {
	int i = 0;
	unsigned int* first_tt_addr = (unsigned int *) FIRST_LVL_TT_ADDR;
	unsigned int* second_tt_addr = (unsigned int *) SECON_LVL_TT_ADDR;
	
	for(i = 0; i < FIRST_LVL_TT_COUN; i++) {
		first_tt_addr[i] = (unsigned int *) first_tt_flags |
		(SECON_LVL_TT_ADDR + i * SECON_LVL_TT_SIZE << 10);
	}
	
	for(i = 0; i < FIRST_LVL_TT_COUN * SECON_LVL_TT_COUN; i++) {
		if (i < 0x500000) {
			second_tt_addr[i] = (unsigned int *) second_tt_flags |
			(i << 12) 
		}
	}
	return 0;
}

void start_mmu_C() {
register unsigned int control;
__asm("mcr p15, 0, %[zero], c1, c0, 0" : : [zero] "r"(0)); //Disable cache
__asm("mcr p15, 0, r0, c7, c7, 0"); //Invalidate cache (data and instructions) */
__asm("mcr p15, 0, r0, c8, c7, 0"); //Invalidate TLB entries
/* Enable ARMv6 MMU features (disable sub-page AP) */
control = (1<<23) | (1 << 15) | (1 << 4) | 1;
/* Invalidate the translation lookaside buffer (TLB) */
__asm volatile("mcr p15, 0, %[data], c8, c7, 0" : : [data] "r" (0));
/* Write control register */
__asm volatile("mcr p15, 0, %[control], c1, c0, 0" : : [control] "r" (control));
}

void configure_mmu_C() {
register unsigned int pt_addr = ((unsigned int)FIRST_LVL_TT_POS);
/* Translation table 0 */
__asm volatile("mcr p15, 0, %[addr], c2, c0, 0" : : [addr] "r" (pt_addr));
/* Translation table 1 */
__asm volatile("mcr p15, 0, %[addr], c2, c0, 1" : : [addr] "r" (pt_addr));
/* Use translation table 0 for everything */
__asm volatile("mcr p15, 0, %[n], c2, c0, 2" : : [n] "r" (0));
/* Set Domain 0 ACL to "Manager", not enforcing memory permissions
* Every mapped section/page is in domain 0
*/
__asm volatile("mcr p15, 0, %[r], c3, c0, 0" : : [r] "r" (0x3));
}
