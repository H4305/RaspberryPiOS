#include "../sched/sched.h"
#include "../memory/vmem.h"
#include "hw.h"

struct ctx_s ctx_ping;
struct ctx_s ctx_pong;
struct ctx_s ctx_init;

void funcA(){
	int cptA = 0;
	while ( 1 ) {
		cptA ++;
	}
}

void funcB() {
	int cptB = 1;
	while ( 1 ) {
		cptB += 2 ;
	}
}
// ------------------------------------------------------------------------
int kmain (void){
	
	initVmem();
	
	sys_wait(5);
	return	0;
}

int initSched() {
	init_hw();
	create_process(funcB, NULL, STACK_SIZE);
	create_process(funcA, NULL, STACK_SIZE);
	start_sched();
}

int initVmem() {
	unsigned int translation;
	init_kern_translation_table();
	configure_mmu_C();
	//start_mmu_C();
	// Kernel pages
	translation = tool_translate(48000);
	translation = tool_translate(256);
	// I/O pages
	translation = tool_translate(536875008);
	translation = tool_translate(553648127);
	// Default pages
	translation = tool_translate(5242880); // should be 0
}
