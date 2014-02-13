#include "DAGPattern.h"
#include "Scheduler.h"
#include "SchedulerMaster.h"
#include "SchedulerSlave.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <mpi.h>
DAGPattern base_dag_pattern;// base_dag_pattern variable
DAGPattern dag_pattern;// dag_pattern of all the data
pthread_mutex_t dag_pattern_lock;

DAGPattern get_dag_pattern()
{
    DAGPattern dag_ptr = dag_pattern;
    return dag_ptr;
}

void Init_DAG_lock()
{
    pthread_mutex_init(&dag_pattern_lock,NULL);
}

void SetDAGPattern(DAGElement* dagelementptr, SizeT dag_size, SizeT block_size, SizeT rect_size, PosT dag_pos,enum DAG_pattern_type dag_pattern_type, int * dag_pattern_finish)
{
    dag_pattern.dag_pattern_element = dagelementptr;
    dag_pattern.dag_size = dag_size;
    dag_pattern.block_size = block_size;
    dag_pattern.rect_size = rect_size;
    dag_pattern.dag_pos = dag_pos;
    dag_pattern.dag_pattern_type = dag_pattern_type;
    dag_pattern.dag_pattern_finish = dag_pattern_finish;
    return;
}

void destroy_dag_pattern()
{
    free(dag_pattern.dag_pattern_element);
    pthread_mutex_destroy(&dag_pattern_lock);
    return;
}

/**
 * paramater dag_node_id (start from 0!!);
 * return response data_block;
 */

BlockT *data_mapping_by_node_id(int dag_node_id)
{
    int mypid;
    MPI_Comm_rank(MPI_COMM_WORLD, &mypid);
    BlockT* data_block = (BlockT* )malloc(sizeof(BlockT)) ;
    PosT pos;
    pos.x = dag_node_id / dag_pattern.rect_size.col * dag_pattern.block_size.row + dag_pattern.dag_pos.x;
    pos.y = ( dag_node_id % dag_pattern.rect_size.col ) * dag_pattern.block_size.col + dag_pattern.dag_pos.y;
    data_block->block_pos = pos;
    SizeT bsize;
    if(pos.x + dag_pattern.block_size.row <= dag_pattern.dag_size.row + dag_pattern.dag_pos.x)
	bsize.row = dag_pattern.block_size.row;
    else
    {
	bsize.row = dag_pattern.dag_pos.x + dag_pattern.dag_size.row - pos.x;
    }
    if(pos.y + dag_pattern.block_size.col <= dag_pattern.dag_size.col + dag_pattern.dag_pos.y)
	bsize.col = dag_pattern.block_size.col;
    else{
	bsize.col = dag_pattern.dag_pos.y + dag_pattern.dag_size.col - pos.y;
    }
    data_block->block_size = bsize;
    data_block->node_id = dag_node_id;
    return data_block;
}

Process_t get_process_by_node_id(int dag_node_id)
{   
    return dag_pattern.dag_pattern_element[dag_node_id].process;
}

void send_datablock(int des_pid,BlockT* data_block)
{
    int mypid;
    MPI_Comm_rank(MPI_COMM_WORLD,&mypid);
    int i,x,y,hlen,wlen;
    x = data_block->block_pos.x;
    y = data_block->block_pos.y;
    hlen = data_block->block_size.row;
    wlen = data_block->block_size.col;
    pthread_mutex_lock(&dag_pattern_lock);
    MPI_Send(&x,1,MPI_INT,des_pid,0+mypid,MPI_COMM_WORLD);
    MPI_Send(&y,1,MPI_INT,des_pid,10000+mypid,MPI_COMM_WORLD);
    MPI_Send(&hlen,1,MPI_INT,des_pid,20000+mypid,MPI_COMM_WORLD);
    MPI_Send(&wlen,1,MPI_INT,des_pid,30000+mypid,MPI_COMM_WORLD);
    SchedulerArgs schedulerArgs = GetMasterSchedulerArgs();
    SchedulerArgs slaveschedulerArgs = GetSlaveSchedulerArgs();
    DAG_data* data_ptr_list;
    if(mypid == 0)
	data_ptr_list = schedulerArgs.datalist;
    else
	data_ptr_list = slaveschedulerArgs.datalist;
    data_ptr_list = data_ptr_list->next;
    while(data_ptr_list != NULL){
	int *data_ptr = (int*)data_ptr_list->data_pointer;
	for(i = x;i < x + hlen; i++)
	{
	    MPI_Send(data_ptr+i*schedulerArgs.dag_size.col+y,wlen,MPI_INT,des_pid,40000+(i-x)*100+mypid,MPI_COMM_WORLD);
	}
	data_ptr_list = data_ptr_list->next;
    }
    pthread_mutex_unlock(&dag_pattern_lock);
    return;
}



void send_datablockpos(int des_pid,BlockT* data_block)
{
    int mypid;
    MPI_Comm_rank(MPI_COMM_WORLD,&mypid);
    MPI_Send(&data_block->block_pos.x,1,MPI_INT,des_pid,0+mypid,MPI_COMM_WORLD);
    MPI_Send(&data_block->block_pos.y,1,MPI_INT,des_pid,10000+mypid,MPI_COMM_WORLD);
    MPI_Send(&data_block->block_size.row,1,MPI_INT,des_pid,20000+mypid,MPI_COMM_WORLD);
    MPI_Send(&data_block->block_size.col,1,MPI_INT,des_pid,30000+mypid,MPI_COMM_WORLD);
    MPI_Send(&data_block->node_id,1,MPI_INT,des_pid,40000+mypid,MPI_COMM_WORLD);
    return;
}

void recv_datablock(int src_id)
{
    int i;
    int x,y,hlen,wlen;
    MPI_Status status;
    MPI_Recv(&x,1,MPI_INT,src_id,0+src_id,MPI_COMM_WORLD,&status);
    MPI_Recv(&y,1,MPI_INT,src_id,10000+src_id,MPI_COMM_WORLD,&status);
    MPI_Recv(&hlen,1,MPI_INT,src_id,20000+src_id,MPI_COMM_WORLD,&status);
    MPI_Recv(&wlen,1,MPI_INT,src_id,30000+src_id,MPI_COMM_WORLD,&status);
    SchedulerArgs schedulerArgs = GetMasterSchedulerArgs();
    SchedulerArgs slaveschedulerArgs = GetSlaveSchedulerArgs();
    int mypid;
    DAG_data* data_ptr_list;
    MPI_Comm_rank(MPI_COMM_WORLD,&mypid);
    if(mypid == 0)
	data_ptr_list = schedulerArgs.datalist;
    else
	data_ptr_list = slaveschedulerArgs.datalist;
    data_ptr_list = data_ptr_list->next;
    pthread_mutex_lock(&dag_pattern_lock);
    while(data_ptr_list != NULL){
	int* data_ptr = (int*)data_ptr_list->data_pointer;
	for(i = x; i < x + hlen; i++)
	{	 
	    MPI_Recv(data_ptr+i*schedulerArgs.dag_size.col+y,wlen,MPI_INT,src_id,40000+(i-x)*100+src_id,MPI_COMM_WORLD,&status);
	}
	data_ptr_list = data_ptr_list->next;
    }
    pthread_mutex_unlock(&dag_pattern_lock);
    return;
}

void write_datablock(int node_id)
{
    FILE *fp;
    int i,j;
    SchedulerArgs schedulerArgs = GetMasterSchedulerArgs();
    SchedulerArgs slaveschedulerArgs = GetSlaveSchedulerArgs();
    int mypid;
    DAG_data* data_ptr_list;
    MPI_Comm_rank(MPI_COMM_WORLD,&mypid);
    if(mypid == 0)
	data_ptr_list = schedulerArgs.datalist;
    else
	data_ptr_list = slaveschedulerArgs.datalist;
    BlockT* data_block = data_mapping_by_node_id(node_id);
    int x = data_block->block_pos.x;
    int y = data_block->block_pos.y;
    int row = data_block->block_size.row;
    int col = data_block->block_size.col;
    char filename[15] = "mem/";
    int cnt = 0;
    int tmp_node_id;
    data_ptr_list = data_ptr_list->next;
    while(data_ptr_list != NULL)
    {
	int p = 4;
	int *data_ptr = (int*)data_ptr_list->data_pointer;
	filename[p++] = 'a' + cnt;
	tmp_node_id = data_block->node_id;
	do
	{
	    filename[p++] = (tmp_node_id & 0xF) + 'a';
	    tmp_node_id >>= 4;
	}while(tmp_node_id!=0);  
	filename[p] = 0;
	fp = fopen(filename,"w");
	for(i = x; i < x + row; i++)
	{
	    for(j = y; j < y + col; j++)
	    {
		fprintf(fp,"%d ",(data_ptr)[i*schedulerArgs.dag_size.col+j]);
	    }
	}
	data_ptr_list = data_ptr_list->next;
	cnt++;
	fflush(fp);
	fclose(fp);
    }
    return;
}

void read_datablock(int node_id)
{
    int i,j;
    SchedulerArgs schedulerArgs = GetMasterSchedulerArgs();
    SchedulerArgs slaveschedulerArgs = GetSlaveSchedulerArgs();
    int mypid;
    DAG_data* data_ptr_list;
    MPI_Comm_rank(MPI_COMM_WORLD,&mypid);
    if(mypid == 0)
	data_ptr_list = schedulerArgs.datalist;
    else
	data_ptr_list = slaveschedulerArgs.datalist;
    BlockT* data_block = data_mapping_by_node_id(node_id);
    int x = data_block->block_pos.x;
    int y = data_block->block_pos.y;
    int row = data_block->block_size.row;
    int col = data_block->block_size.col;
    char filename[15] = "mem/";
    int cnt = 0;
    int tmp_node_id;
    data_ptr_list = data_ptr_list->next;
    FILE *fp; 
    while(data_ptr_list != NULL)
    {
	int p = 4;
	int *data_ptr = (int*)data_ptr_list->data_pointer;
	tmp_node_id = data_block->node_id;
	filename[p++] = 'a' + cnt;
	do	
	{
	    filename[p++] = (tmp_node_id & 0xF) + 'a';
	    tmp_node_id >>= 4;
	}while(tmp_node_id != 0);	
	filename[p] = 0;
	fp = fopen(filename,"r");
	//fprintf(stderr,"%s\n",filename);
	for(i = x; i < x + row; i++)
	{
	    for(j = y; j < y + col; j++)
	    {
		fscanf(fp,"%d ",data_ptr + i*schedulerArgs.dag_size.col+j);
	    }
	} 
	cnt++;
	data_ptr_list = data_ptr_list->next;
	fflush(fp);
	fclose(fp);
    } 
    return;
}

BlockT recv_datablockpos(int src_id)
{
    BlockT data_block;
    MPI_Status status;
    MPI_Recv(&data_block.block_pos.x,1,MPI_INT,src_id,0+src_id,MPI_COMM_WORLD,&status);
    MPI_Recv(&data_block.block_pos.y,1,MPI_INT,src_id,10000+src_id,MPI_COMM_WORLD,&status);
    MPI_Recv(&data_block.block_size.row,1,MPI_INT,src_id,20000+src_id,MPI_COMM_WORLD,&status);
    MPI_Recv(&data_block.block_size.col,1,MPI_INT,src_id,30000+src_id,MPI_COMM_WORLD,&status);
    MPI_Recv(&data_block.node_id,1,MPI_INT,src_id,40000+src_id,MPI_COMM_WORLD,&status);
    return data_block;
}

