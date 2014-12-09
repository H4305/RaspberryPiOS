#ifndef _SCHED_H_
#define _SCHED_H_

#include "../os/process.h"

#define STACK_SIZE 512
#define WORD_SIZE 4
#define CPSR 0x13

#define NULL ((void*)0)



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

//To use it in syscall
struct pcb_s * getCurrentProcess();


#endif
