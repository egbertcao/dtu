#ifndef _DUT_COMMON_H_
#define _DUT_COMMON_H_

typedef struct slave {
	unsigned int protocol;
	unsigned int s_address;
	unsigned int r_address;
	char function[10];
} slave_t;

#define MAX_SLAVE 20

#endif