#ifndef _SCHED_H_
#define _SCHED_H_

#define STACK_SIZE 512
#define WORD_SIZE 4
#define CPSR 0x13

#define NULL ((void*)0)
typedef enum {READY, SLEEPING, EXECUTING, ZOMBIE} etat;
typedef void (*func_t) ( void*);
typedef struct ctx_s {
        unsigned int sp;
        void *lr;
} ctx_s;
typedef struct pcb_s {
	etat etat;
	struct ctx_s context;
	struct pcb_s * next;
	func_t func_pointer;
	void* args;
}pcb_s;


/*struct ctx_s* current_ctx;*/

void init_ctx(struct ctx_s* ctx, func_t f, unsigned int stack_size);
void __attribute__ ((naked)) switch_to(struct ctx_s* ctx);

void init_pcb(struct pcb_s* pcb, func_t f, void *args, unsigned int stack_size);
void create_process(func_t f, void *args, unsigned int stack_size);
void start_current_process();
void elect();
void ctx_switch_from_irq ();
void start_sched();
void clear_next_process();

void __attribute__ ((naked)) ctx_switch();

#endif
