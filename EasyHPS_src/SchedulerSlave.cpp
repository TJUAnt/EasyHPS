#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "Scheduler.h"
#include "SchedulerSlave.h"
#include "SchedulerMaster.h"
#include "DAGNodeManager.h"
#include "TimeoutQueueSlave.h"
#include "TaskQueue.h"
#include <mpi.h>
static SchedulerArgs slaveSchedulerArgs;

int dag_flag[80000];
int data_buffer[80000];

static double time_c,time_tt;

void SetSlaveSchedulerArgs(int x,int y,DAG_data *data,int arg1,int arg2,int arg3,int timeout, enum DAG_pattern_type dag_pattern_type)
{
    slaveSchedulerArgs.datalist = data;
    slaveSchedulerArgs.dag_size.row = arg1;
    slaveSchedulerArgs.dag_size.col = arg1;
    slaveSchedulerArgs.dag_pos.x = x;
    slaveSchedulerArgs.dag_pos.y = y;
    slaveSchedulerArgs.block_size.row = arg2;
    slaveSchedulerArgs.block_size.col = arg2;
    slaveSchedulerArgs.thread_num = arg3;
    slaveSchedulerArgs.time_out = timeout;
    slaveSchedulerArgs.dag_pattern_type = dag_pattern_type;
    return;
}

SchedulerArgs GetSlaveSchedulerArgs()
{
    return slaveSchedulerArgs;
}
void EasyTHPS_Scheduler_Slave(SchedulerArgs schedulerArgs)
{
    int signal = 1;
    double time_len,time_cal,thread_other;
    int data_send_cnt,i,ack,nece_cnt;
    BlockT datablock;
    MPI_Status status;
    memset(dag_flag,0,sizeof(dag_flag));
    MPI_Send(&signal,1,MPI_INT,0,0,MPI_COMM_WORLD);
    time_c = time_tt = 0.0;
    struct timeval sta,ed;
    while(1)
    {
	MPI_Recv(&data_send_cnt,1,MPI_INT,0,10,MPI_COMM_WORLD,&status);
	if(data_send_cnt == -1)
	{
	    //show Master pool down
	    break;
	} 
	MPI_Recv(data_buffer,data_send_cnt,MPI_INT,0,20,MPI_COMM_WORLD,&status);

	/* find necessary node_id data */
	nece_cnt = 0;
	for(i = 0; i < data_send_cnt; i++)
	{
	    if(dag_flag[data_buffer[i]] == 0)
		nece_cnt++;
	}
	MPI_Send(&nece_cnt,1,MPI_INT,0,30,MPI_COMM_WORLD);
	for(i = 0; i < data_send_cnt; i++)
	{
	    if(dag_flag[data_buffer[i]] == 0)
	    { 
		MPI_Send(&data_buffer[i],1,MPI_INT,0,1000+i,MPI_COMM_WORLD);
		recv_datablock(0);
	    }
	}
	/* find necessary node_id data */
	datablock = recv_datablockpos(0);
	gettimeofday(&sta,NULL);
	schedulerArgs.dag_pos = datablock.block_pos;
	schedulerArgs.dag_size = datablock.block_size;
	Init_EasyTHPS(schedulerArgs,1);
	update_computable_node(-1);
	add_computable_task();
	while(1)
	{
	    if(is_computable_finished())
	    {
		SetTaskQueueFinish();
		break;
	    }
	    //  Slave_check_timeout_routine();
	    update_dag_pattern();
	    add_computable_task();
	}
	ack = 1;
	gettimeofday(&ed,NULL);
	time_len = ed.tv_sec - sta.tv_sec + (ed.tv_usec - sta.tv_usec)/1000000.0;
	time_cal = get_thread_cal_time(schedulerArgs.thread_num);
	MPI_Send(&ack,1,MPI_INT,0,40,MPI_COMM_WORLD);
	MPI_Send(&time_len,1,MPI_DOUBLE,0,40,MPI_COMM_WORLD);
	MPI_Send(&time_cal,1,MPI_DOUBLE,0,40,MPI_COMM_WORLD);
	//printf("%d\n",datablock.node_id);
	send_datablock(0,&datablock);
	Destroy_EasyTHPS(schedulerArgs,1);
    }
    return;
}
