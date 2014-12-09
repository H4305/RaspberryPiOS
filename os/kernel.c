#include "../sched/sched.h"
#include "hw.h"

struct ctx_s ctx_ping;
struct ctx_s ctx_pong;
struct ctx_s ctx_init;

void funcA(){
	int cptA = 0;
	while ( cptA < 5 ) {
		cptA ++;
	}
	int value = cptA;
}

void funcB() {
	int cptB = 1;
	while (cptB < 10) {
		cptB += 2 ;
	}
	int value = cptB;
}
// ------------------------------------------------------------------------
int kmain (void){
	init_hw();
	create_process(funcB, NULL, STACK_SIZE);
	create_process(funcA, NULL, STACK_SIZE);
	start_sched();
	//ctx_switch();
	/*
		Pas atteignable vues nos 2 fonctions
	*/

	sys_wait(5);
	return	0;
}
