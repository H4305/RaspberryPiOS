#include "../sched/sched.h"
#include "hw.h"

struct ctx_s ctx_ping;
struct ctx_s ctx_pong;
struct ctx_s ctx_init;

void funcA(){
	int cptA = 0;
	while ( 1 ) {
		cptA += 1;
		if(cptA == 2) {
			sys_wait(2);
		}
		ctx_switch();
	}
}

void funcB() {
	int cptB = 1;
	while ( 1 ) {
		cptB += 2 ;
		ctx_switch();
	}
}
// ------------------------------------------------------------------------
int kmain (void){
	init_hw();
	create_process(funcB, NULL, STACK_SIZE);
	create_process(funcA, NULL, STACK_SIZE);
	start_sched();
	
	ctx_switch();
	/*
		Pas atteignable vues nos 2 fonctions
	*/

	return	0;
}
