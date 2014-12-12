#include "isolation.h"

/*
 * Handler called on Interrutp by MMU on access error 
 */
void 
data_handler() {
	
	int error;       // error bits
	data_error error_label;
	
	/* 
	 * Lecture du registre Data Fault 
	 * qui contient la cause de la faute
	 */
	__asm volatile("mrc p15, 0, %0, c5, c0, 0": "=r" (error));
	/* Masque pour récuperer les derniers bits */
	error = error | 16;
	switch(error) {
		case 7 : 
			error_label = data_error.TRANSLATION_FAULT;
			break;
		case 6 :
			error_label = data_error.ACCESS_FAULT;
			break;
		case 15 :
			error_label = data_error.PERMISSION_FAULT;
			break;
		
		default :
			error_label = data_error.NOT_DEFINED;
			break;
	}
	
	exit();
}

