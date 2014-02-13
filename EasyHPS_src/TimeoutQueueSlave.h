#ifndef _TIMEOUT_QUEUE_SLAVE_H_
#define _TIMEOUT_QUEUE_SLAVE_H_

#include "TimeoutQueue.h"


void Slave_init_timeout_queue(int timeout);

void Slave_add_timeout_queue(int _thread_id, int _node_id);

int Slave_remove_timeoutnode_by_id(int _node_id);

void Slave_destroy_timeout_queue();

void* Slave_check_timeout_routine(void *);
#endif
