#include "sched.h"
#include "../os/phyAlloc.h"
#include "../os/hw.h"

#define FIFO

struct pcb_s* idle_process;
struct pcb_s * current_process = NULL;
struct pcb_waiting_s * current_process_waiting = NULL;

void * main_lr;


struct pcb_s * getCurrentProcess() {
	return current_process;
}

void add_processus_waiting_list(struct pcb_s * process) {
	struct pcb_waiting_s * new;
	new = (struct pcb_waiting_s*) phyAlloc_alloc(sizeof(struct pcb_waiting_s));
	
	if(current_process_waiting==NULL) {
		current_process_waiting = new;
		current_process_waiting->next = new;
		current_process_waiting->previous = new;
		current_process_waiting->waiting_process = process;
	}else 
	{
		new->next = current_process_waiting->next;
		current_process_waiting->next = new;
		
		new->previous = current_process_waiting;
		new->next->previous = new;
		
		current_process_waiting = current_process_waiting->next; //Je passe au nouveau processus en attente
		current_process_waiting->waiting_process = process;
	}
	
}

void decrement_wait_quantum() {
	
	pcb_waiting_s * first_process_waiting = current_process_waiting;
	
	if(first_process_waiting->next == first_process_waiting) {
		int nb_quantum_wait = (first_process_waiting->waiting_process->nb_quantum_wait - 1);
		first_process_waiting->waiting_process->nb_quantum_wait = nb_quantum_wait;
		
		if(nb_quantum_wait == 0) {
			//Remove element from list
			first_process_waiting->waiting_process->etat = READY;
			phyAlloc_free(current_process_waiting, sizeof(struct pcb_waiting_s)); //Free de la mémoire 
			current_process_waiting = NULL; //Liste Vide
			
		}
	}else {
		
		while(current_process_waiting->next != first_process_waiting)
		{
			int nb_quantum_wait = (current_process_waiting->waiting_process->nb_quantum_wait - 1);
			current_process_waiting->waiting_process->nb_quantum_wait = nb_quantum_wait;
		
			if(nb_quantum_wait == 0) {
				//Remove element from list
				current_process_waiting->previous->next = current_process_waiting->next;
				current_process_waiting->next->previous = current_process_waiting->previous;
				
				first_process_waiting->waiting_process->etat = READY;
				phyAlloc_free(current_process_waiting, sizeof(struct pcb_waiting_s)); //Free de la mémoire 
							
			}
			
			current_process_waiting = current_process_waiting->next;
		}
		
		int nb_quantum_wait = (current_process_waiting->waiting_process->nb_quantum_wait - 1);
		current_process_waiting->waiting_process->nb_quantum_wait = nb_quantum_wait;
	
		if(nb_quantum_wait == 0) {
			//Remove element from list
			current_process_waiting->previous->next = current_process_waiting->next;
			current_process_waiting->next->previous = current_process_waiting->previous;
			
			first_process_waiting->waiting_process->etat = READY;
			phyAlloc_free(current_process_waiting, sizeof(struct pcb_waiting_s)); //Free de la mémoire 		
		}
	}//Fin else
	
	
	
}

void init_ctx(struct ctx_s* ctx, func_t f, unsigned int stack_size) {
	ctx->sp = ((unsigned int) phyAlloc_alloc(stack_size) + STACK_SIZE);
	ctx->sp -= WORD_SIZE;
	*((int*) ctx->sp) = CPSR;
	ctx->sp -= WORD_SIZE;
	*((int*) ctx->sp) = (unsigned int)&start_current_process;
	ctx->sp -= 13 * WORD_SIZE;
	//ctx->lr = &start_current_process;
}

void init_pcb(struct pcb_s* pcb,func_t f, void* args, unsigned int stack_size) {
	pcb->etat = READY;
	init_ctx(&pcb->context, f, stack_size);
	pcb->func_pointer = f;
	pcb->args = args;
}

void create_process(func_t f, void *args, unsigned int stack_size) {
	struct pcb_s * new;
	new = (struct pcb_s*) phyAlloc_alloc(sizeof(struct pcb_s));
	
	if (current_process == NULL) {
		current_process = new;
		current_process->next = new;
	}
	else if(current_process == idle_process) {
		current_process->next = new;
		new->next = new;
	}
	else {
		new->next = current_process->next;
		current_process->next = new;
	}
	init_pcb(new, f, args, stack_size);	
}

void start_current_process() {
	current_process->func_pointer(current_process->args);
	
	current_process->etat = ZOMBIE;
	ctx_switch();
}

void elect() {
	if(current_process->etat == EXECUTING) {
		current_process->etat = READY;
	}
	
	struct pcb_s * next = current_process->next;
	while(next->etat == ZOMBIE) {
		DISABLE_IRQ();
		clear_next_process();
		next = current_process->next;
		current_process = next;
		set_tick_and_enable_timer();
		ENABLE_IRQ();
	}
	
	current_process = next;
	
	if(current_process->etat == READY) {
		current_process->etat = EXECUTING;
	}
}

void idle() {
	while(1);
}

void ctx_switch_from_irq () {
	DISABLE_IRQ();
#ifndef FIFO 	
	__asm("sub lr, lr, #4");
#endif
	__asm("srsdb sp!, #0x13");
#ifndef FIFO
	__asm("cps #0x13");
	__asm("push {r0-r12}");
	__asm("mov %0, sp" : "=r"(current_process->context.sp));
	
	elect();
	__asm("mov sp, %0" : : "r"(current_process->context.sp));
#endif
	set_tick_and_enable_timer();
#ifdef FIFO
	__asm("pop {r0-r12}");
#endif
	ENABLE_IRQ();
	__asm("rfeia sp!");
}	

void start_sched() {
	create_process(NULL, NULL, 0);
	idle_process = current_process->next;
	current_process->next = idle_process->next;
	current_process = idle_process;
	set_tick_and_enable_timer();
	ENABLE_IRQ();
}

void __attribute__ ((naked)) ctx_switch() {
	__asm("push {r0-r12}");
	__asm("mov %0, sp" : "=r"(current_process->context.sp));
	__asm("mov %0, lr" : "=r"(current_process->context.lr));
	elect();
	__asm("mov sp, %0" : : "r"(current_process->context.sp));
	__asm("mov lr, %0" : : "r"(current_process->context.lr));
	__asm("pop {r0-r12}");
	__asm("bx lr");
}

void clear_next_process() {
	struct pcb_s * kill_me = current_process->next;
	current_process->next = kill_me->next;
	
	if (current_process == kill_me) {
		current_process = idle_process;
		idle_process->next = idle_process;
	}
	
	//zero-fill
	
	kill_me->context.sp -= STACK_SIZE + 13*4 + 1*4;
	phyAlloc_free(kill_me->context.sp, STACK_SIZE);
	phyAlloc_free(kill_me, sizeof(struct pcb_s));
}
