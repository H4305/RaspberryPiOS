#include "vmem.h"

uint32 first_tt_flags =
	(1 << 0) | //1
	(0 << 1) | //0
	(0 << 2) | //SBZ
	(0 << 3) | //NS
	(0 << 4) | //SBZ
	(0000 << 5) | //Domain
	(0 << 9); //P - not supported

uint32 second_tt_flags = 
	(0 << 0) | // XN
	(1 << 1) | // Always 1
	(0 << 2) | // B
	(0 << 3) | // C
	(00 << 4) | // AP
	(001 << 6) | // TEX
	(0 << 9) | // APX
	(0 << 10) | // S	
	(0 << 11); // NG

uint32 device_flags = 
	(0 << 0) | // XN
	(1 << 1) | // Always 1
	(1 << 2) | // B
	(0 << 3) | // C
	(00 << 4) | // AP
	(000 << 6) | // TEX
	(0 << 9) | // APX
	(0 << 10) | // S	
	(0 << 11); // NG

// Available frames table
uint8* frames_table = (uint8*) VMEM_ALLOC_T_START;

// Initializes the translation table
unsigned int init_kern_translation_table(void) {
	
	uint32* ftt_a = (uint32 *)FIRST_LVL_TT_ADDR;
	uint32 page_i = 0;
	unsigned int i;
	
	// Iterate through first level table indexes
	for(i = 0; i < FIRST_LVL_TT_COUN; i ++) {
		uint32* stt_a = (uint32 *)(SECON_LVL_TT_ADDR + (i << 10));
		ftt_a[i] = (uint32)
		first_tt_flags |
		((uint32)stt_a & 0xFFFFFC00);

		uint32 j;
		// Iterate through second level table indexes
		for( j = 0;j < SECON_LVL_TT_COUN; j ++) {
			uint32 val = 0;
			
			if(page_i * PAGE_SIZE < 0x500000) {
				val = second_tt_flags |
				(page_i << 12);
			}
			if(page_i * PAGE_SIZE > 0x20000000 &&
			   page_i * PAGE_SIZE < 0x20FFFFFF) {
				val = device_flags |
				(page_i << 12);
			}
			stt_a[j] = val;
			page_i++;
		}
	}
	return 0;
}

// Initializes the page frames availability table
bool avail_frames_init() {
	for(uint32 i = 0; i < VMEM_TOTAL_PAGES; i ++) {
		vmem_table[i] = 0;
	}
	return true;
}

// Allocates a frame and returns the adress
uint8* vMem_alloc(uint32 pages) {
	for(uint64 i = 0; i < FRAMES_T_PAGES; i ++) {
		if(frames_table[i] == 0) { // page available
			//Check if it fits
			bool fit = true;
			for(uint64 j = 0; j < pages; j ++) {
				if(frames_table[i + j] != 0) {
					fit = false;
					i = i+j;
					break;
				}
			}
			// If it fits, allocate it
			if(fit) {
				for(uint64 j = 0; j < pages; j ++) {
					frames_table[i + j] = 1;
				}
				return (uint8 *)(i * PAGE_SIZE);
			}
		}
	}
	return false;
}

// Free allocated pages
bool vMem_free(uint8* ptr, uint32 pages) {
	uint32 page = (uint32)(ptr)/PAGE_SIZE;
	for(uint32 i = 0; i < pages; i ++) {
		vmem_table[page + i] = 0;
	}
	return true;
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
	register unsigned int pt_addr = ((unsigned int)FIRST_LVL_TT_ADDR);
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

unsigned int tool_translate(unsigned int va) {
	unsigned int pa; /* the result */

	/* 1st and 2nd table addresses */
	unsigned int table_base;
	unsigned int second_level_table;

	/* Indexes */
	unsigned long first_level_index;
	unsigned long second_level_index;
	unsigned long page_index;

	/* Descriptors */
	unsigned int first_level_descriptor;
	unsigned int* first_level_descriptor_address;
	unsigned int second_level_descriptor;
	unsigned int* second_level_descriptor_address;
	__asm("mrc p15, 0, %[tb], c2, c0, 0" : [tb] "=r"(table_base));
	table_base = table_base & 0xFFFFC000;

	/* Indexes*/
	first_level_index = (va >> 20);
	second_level_index = ((va << 12) >> 24);
	page_index = (va & 0x00000FFF);

	/* First level descriptor */
	first_level_descriptor_address = (unsigned int*) (table_base |
	(first_level_index << 2));
	first_level_descriptor = *(first_level_descriptor_address);

	/* Second level descriptor */
	second_level_table = first_level_descriptor & 0xFFFFFC00;
	second_level_descriptor_address = (unsigned int*) (second_level_table
	| (second_level_index << 2));
	second_level_descriptor = *second_level_descriptor_address;

	/* Physical address */
	pa = (second_level_descriptor & 0xFFFFF000) | page_index;
	return pa;
}
