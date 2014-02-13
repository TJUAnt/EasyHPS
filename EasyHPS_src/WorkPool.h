#ifndef _WORK_POOL_H_
#define _WORK_POOL_H_

typedef void *(*WorkPoolProcess_t)(void *arg);

void init_work_pool(int max_thread_num,int mypid);

void destroy_work_pool(int max_thread_num);

void restart_thread(int thread_id);

#endif
