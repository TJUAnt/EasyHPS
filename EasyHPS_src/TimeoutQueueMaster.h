#ifndef _TIMEOUT_QUEUE_MASTER_H_
#define _TIMEOUT_QUEUE_MASTER_H_

#include "TimeoutQueue.h"

void Master_init_timeout_queue(int timeout);

void Master_add_timeout_queue(int _thread_id, int _node_id);

int Master_remove_timeoutnode_by_nodeid(int _node_id);

void Master_destroy_timeout_queue();

void* Master_check_timeout_routine(void *);

//void register_task_nodeid_by_threadid(int node_id,int thread_id);

//int get_task_nodeid_by_threadid(int thread_id);
#endif
