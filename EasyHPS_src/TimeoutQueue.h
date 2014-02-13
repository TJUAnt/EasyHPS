#ifndef _TIMEOUT_QUEUE_H_
#define _TIMEOUT_QUEUE_H_

#include <sys/time.h>

typedef struct tagTimeOut
{
    int thread_id;
	int node_id;
    struct timeval time;
	struct tagTimeOut* next;
    struct tagTimeOut* pre;
}TimeOut;
/*
void init_timeout_queue(int timeout);

void add_timeout_queue(int thread_id);

void remove_timeoutnode_by_id(int timeoutnode_id);

void destroy_timeout_queue();

void check_timeout();
*/
#endif
