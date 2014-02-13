#ifndef _WORK_POOL_SLAVE_H_
#define _WORK_POOL_SLAVE_H_

#include "WorkPool.h"

void SetGetSlaveWorkPoolProcess();

WorkPoolProcess_t GetSlaveWorkPoolProcess();

void* Slave_thread_routine(void *arg);

void Slave_cancel_handler(void *arg);

void Slave_init_work_pool(int max_thread_num);

void Slave_destroy_work_pool(int max_thread_num);

void Slave_restart_thread(int thread_id);

#endif
