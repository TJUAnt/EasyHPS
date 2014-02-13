#ifndef _TASK_QUEUE_H_
#define _TASK_QUEUE_H_
#include "DAGPattern.h"

typedef struct tagTask
{
    Process_t process;
    int node_id;
    struct tagTask* next;
}Task;

void init_task_queue();

void add_task(int node_id);

int remove_task(int node_id);

void destroy_task_queue();

Task* get_task();

void SetTaskQueueFinish();

void add_computable_task();
#endif
