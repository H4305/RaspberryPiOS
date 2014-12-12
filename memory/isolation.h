#ifndef _ISOLATION_H
#define _ISOLATION_H

typedef enum {TRANSLATION_FAULT, ACCESS_FAULT, PERMISSION_FAULT, NOT_DEFINED} data_error;

void data_handler();


#endif
