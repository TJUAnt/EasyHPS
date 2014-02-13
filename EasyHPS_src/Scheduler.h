#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "DAGPattern.h"

typedef struct tag_data_ptr_node
{
    void* data_pointer;
    struct tag_data_ptr_node * next;
}DAG_data;


typedef struct
{
    DAG_data* datalist;
    SizeT block_size;
    SizeT dag_size;
    PosT dag_pos;
    int thread_num;
    int time_out;
    Process_t process;
    enum DAG_pattern_type dag_pattern_type;
}SchedulerArgs;

void Init_EasyTHPS(SchedulerArgs schedulerArgs,int mypid);

void Destroy_EasyTHPS(SchedulerArgs schedulerArgs,int mypid);

void add_DAG_ptr_list(void* );

void init_DAG_ptr_list();

DAG_data* get_DAG_ptr_list();
/*
void EasyPDP_Scheduler(SchedulerArgs schedulerArgs);

void SetSchedulerArg(int x,int y,void* data,int arg1,int arg2,int arg3,int timeout);

SchedulerArgs GetSchedulerArg();
*/
#endif
