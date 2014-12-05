#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#define SYS_CALL_REBOOT 1
#define SYS_CALL_WAIT 2

void __attribute__ ((naked)) sys_reboot();
void __attribute__ ((naked)) sys_wait(unsigned int nbQuantums);
void __attribute__ ((naked)) SWIHandler();

#endif
