#ifndef _WORK_POOL_MASTER_H_
#define _WORK_POOL_MASTER_H_

#include "WorkPool.h"

void SetGetMasterWorkPoolProcess();

WorkPoolProcess_t GetMasterWorkPoolProcess();

void Master_init_work_pool(int max_thread_num);

void Master_destroy_work_pool(int max_thread_num);

void output_time();

void* Master_thread_routine(void *arg);

void Master_cancel_handler(void *arg);

void Master_restart_thread(int thread_id);
#endif
