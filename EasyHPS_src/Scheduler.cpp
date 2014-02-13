#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Scheduler.h"
#include "DAGNodeManager.h"
#include "WorkPoolMaster.h"
#include "WorkPoolSlave.h"
#include "TimeoutQueueMaster.h"
#include "TimeoutQueueSlave.h"
#include "TaskQueue.h"
#include "DAGPatternMaster.h"
#include "DAGPatternSlave.h"

//static SchedulerArgs schedulerArgs;

static DAG_data* dag_data_ptr_list;

void init_DAG_ptr_list()
{
	dag_data_ptr_list = (DAG_data*)malloc(sizeof(DAG_data));
	dag_data_ptr_list->next = NULL;
}

DAG_data * get_DAG_ptr_list()
{
	return dag_data_ptr_list;
}

void add_DAG_ptr_list(void *dptr)
{
	DAG_data * dtmp = (DAG_data*)malloc(sizeof(DAG_data));
	dtmp->data_pointer = dptr;
	dtmp->next = dag_data_ptr_list->next;
	dag_data_ptr_list->next = dtmp;
}

void Init_EasyTHPS(SchedulerArgs schedulerArgs,int mypid)
{
    init_dag_node_finish_stack();
    init_dag_node_computable_stack();
    init_task_queue();
    if(mypid == 0)
    {
//		init_register_table();
//       printf("Init_EasyTHPS_Master\n");
        Master_init_dag_pattern(schedulerArgs.dag_pos,schedulerArgs.dag_size,schedulerArgs.block_size,schedulerArgs.process,schedulerArgs.dag_pattern_type);

        Master_init_timeout_queue(schedulerArgs.time_out);
		Master_init_work_pool(schedulerArgs.thread_num);
    }
    else
    {
//        printf("Init_EasyTHPS_Slave\n");
        Slave_init_dag_pattern(schedulerArgs.dag_pos,schedulerArgs.dag_size,schedulerArgs.block_size,schedulerArgs.process,schedulerArgs.dag_pattern_type);
        Slave_init_timeout_queue(schedulerArgs.time_out);
        Slave_init_work_pool(schedulerArgs.thread_num);
    }
    return;
}
void Destroy_EasyTHPS(SchedulerArgs schedulerArgs,int mypid)
{
    if(mypid == 0)
    {
        Master_destroy_work_pool(schedulerArgs.thread_num);
        Master_destroy_timeout_queue();
    }
    else
    {
        Slave_destroy_work_pool(schedulerArgs.thread_num);
        Slave_destroy_timeout_queue();
    }
    destroy_task_queue();
    destroy_dag_pattern();
    destroy_dag_node_finish_stack();
    destroy_dag_node_computable_stack();
}
