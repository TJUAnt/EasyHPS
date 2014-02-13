#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include "SchedulerMaster.h"
#include "DAGNodeManager.h"
#include "TimeoutQueueMaster.h"
#include "TaskQueue.h"
#include "WorkPoolMaster.h"
static SchedulerArgs masterSchedulerArgs;

void SetMasterSchedulerArgs(int x,int y,DAG_data *data,int dag_row,int dag_col,int arg2,int arg3,int timeout, enum DAG_pattern_type dag_pattern_type)
{
    masterSchedulerArgs.datalist = data;
    masterSchedulerArgs.dag_size.row = dag_row;
    masterSchedulerArgs.dag_size.col = dag_col;
    masterSchedulerArgs.dag_pos.x = x;
    masterSchedulerArgs.dag_pos.y = y;
    masterSchedulerArgs.block_size.row = arg2;
    masterSchedulerArgs.block_size.col = arg2;
    masterSchedulerArgs.thread_num = arg3;
    masterSchedulerArgs.time_out = timeout;
    masterSchedulerArgs.dag_pattern_type = dag_pattern_type;
    return;
}

SchedulerArgs GetMasterSchedulerArgs()
{
    return masterSchedulerArgs;
}

void EasyHPS_recover()
{
    int node_id;
    DAGPattern dag_pattern = get_dag_pattern();
    FILE *fp = fopen("run_log","r");
    if(fp == NULL){
	fp = fopen("run_log","w+");
	fflush(fp);
	fclose(fp);
    }
    printf("EasyHPS_recover\n");
    fp = fopen("run_log","r");
    while(fscanf(fp,"%d",&node_id)!=EOF)
    {
	printf("%d\n",node_id);
	dag_pattern.dag_pattern_finish[node_id] = 1;
    }
    fflush(fp);
    fclose(fp);
    return;
}

void EasyTHPS_Scheduler_Master(SchedulerArgs schedulerArgs)
{
    struct timeval time_start,time_end;
    gettimeofday(&time_start,NULL);
    Init_EasyTHPS(schedulerArgs,0);
    EasyHPS_recover();
    update_computable_node(-1);
    add_computable_task();
    
    while(1)
    {
        if(is_computable_finished())
        {
            SetTaskQueueFinish();
            break;
	}
        update_dag_pattern();
        add_computable_task();
    }
    Destroy_EasyTHPS(schedulerArgs,0);
    gettimeofday(&time_end,NULL);
    output_time();
    printf("Runtime: %.6lf sec\n",time_end.tv_sec-time_start.tv_sec+(time_end.tv_usec-time_start.tv_usec)/1000000.0);
    return;
}
