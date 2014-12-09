/*
 * syscall.c
 * 
 * Copyright 2014 Danilescu Ludmila 
 * 
 * 
 */
 
#include "syscall.h"
#include "hw.h"
#include "process.h"
#include "../sched/sched.h"

void doSysCallReboot(){

	const int PM_RSTC = 0x2010001c; 
	const int PM_WDOG = 0x20100024;
	const int PM_PASSWORD = 0x5a000000;
	const int PM_RSTC_WRCFG_FULL_RESET = 0x00000020;

	PUT32(PM_WDOG, PM_PASSWORD | 1);
	PUT32(PM_RSTC, PM_PASSWORD | PM_RSTC_WRCFG_FULL_RESET);
	while(1);    

}


void doSysCallWait(nbQuantums){

	struct pcb_s * current_process = getCurrentProcess();
	
	current_process->nb_quantum_wait = nbQuantums + 1;
	current_process->etat = SLEEPING;
	
	add_processus_waiting_list(current_process);
	
	ctx_switch();
	
	//Ca revient ici après que le nbQuantums devient 0 (l'ordonanceur choisi à nouveau le processus endormi -> écrase contexte de a? naked peut etre?
	
}



void __attribute__ ((naked)) sys_reboot(){   // naked -> traitant d'interuption. Fonction sans sauvegarde (restore) des registres
	
	DISABLE_IRQ();	
	
	__asm("mov r0, %0" : : "r"(SYS_CALL_REBOOT));
	__asm("SWI 0" : : : "lr");
	
	ENABLE_IRQ();
}


void __attribute__ ((naked)) sys_wait(unsigned int nbQuantums){	
	
	DISABLE_IRQ();	
	
	__asm("mov r0, %0" : : "r"(SYS_CALL_WAIT));
	__asm("mov r1, %0" : : "r"(nbQuantums));
	__asm("SWI 0" : : : "lr");
	
	ENABLE_IRQ();
}

void __attribute__ ((naked)) SWIHandler(){
	
	unsigned int numSysCall = 10;
	
	__asm("mov %0, r0" : "=r"(numSysCall));
	
	if(numSysCall == 1){
		
		doSysCallReboot();
		
	}else if (numSysCall == 2){
		
		unsigned int nbQuantums;
		__asm("mov %0, r1" : "=r"(nbQuantums));
		doSysCallWait(nbQuantums);
		
	}

}





