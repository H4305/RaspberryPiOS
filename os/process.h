#ifndef _PROCESS_H_
#define _PROCESS_H_

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
	int nb_quantum_wait;
}pcb_s;

typedef struct pcb_waiting_s
{
    struct pcb_waiting_s *next; /* pointeur sur le next de la liste */
    struct pcb_waiting_s *previous; /* pointeur sur le previous de la liste */
    pcb_s * waiting_process; 
    
}pcb_waiting_s;

#endif
