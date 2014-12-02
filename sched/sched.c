#include "sched.h"
#include "phyAlloc.h"
#include "hw.h"

struct pcb_s* idle_process;
struct pcb_s * current_process = NULL;
void * main_lr;

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
	while(1)
		ctx_switch();
}

void ctx_switch_from_irq () {

	DISABLE_IRQ();
	__asm("sub lr, lr, #4");
	__asm("srsdb sp!, #0x13");
	__asm("cps #0x13");
	
	__asm("push {r0-r12}");
	__asm("mov %0, sp" : "=r"(current_process->context.sp));
	
	elect();
	__asm("mov sp, %0" : : "r"(current_process->context.sp));
		
	set_tick_and_enable_timer();
	__asm("pop {r0-r12}");
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
