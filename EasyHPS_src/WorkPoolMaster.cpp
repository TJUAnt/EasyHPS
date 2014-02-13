#include "SchedulerMaster.h"
#include "WorkPoolMaster.h"
#include "TaskQueue.h"
#include "TimeoutQueueMaster.h"
#include "DAGNodeManager.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
static pthread_t* thread_pool;

static pthread_mutex_t master_thread_pool_lock;

static WorkPoolProcess_t masterWorkPoolProcess;

static double* master_ctime;

static double* master_mtime;

static double* master_cctime;

static double time_cal_thread;

static FILE *scheduler_fp;
void output_time()
{
	FILE *fp = fopen("EasyHPS_log.txt","w");
	int node_num,i;
	double sum = 0;
	double output_1,output_2;
	output_1 = output_2 = 0.0;
	MPI_Comm_size(MPI_COMM_WORLD,&node_num);
	node_num--;
	for(i = 0; i < node_num; i++)
	{
		output_2 += master_mtime[i];
		sum += master_ctime[i];
	}
	fprintf(fp,"Message Passing Time: %.6lf(sec)\nCalculate Time(Thread): %.6lf(sec)\n",output_2,time_cal_thread);
	fprintf(fp,"Load balancing\n");
	for(i = 0; i < node_num; i++)
	{
		fprintf(fp,"Slave Process %d: %.3lf%\n",i+1,master_ctime[i]*100.0/(double)sum);
	}
	fflush(fp);
	fclose(fp);
	return;
}

void SetGetMasterWorkPoolProcess()
{
	masterWorkPoolProcess = Master_thread_routine;
	return ;
}

WorkPoolProcess_t GetMasterWorkPoolProcess()
{
	return masterWorkPoolProcess;
}

void Master_init_work_pool(int max_thread_num)
{
	int i;
	scheduler_fp = fopen("EasyHPS_Scheduler_log.txt","w");
	time_cal_thread = 0.0;
	thread_pool = (pthread_t*)malloc(max_thread_num*sizeof(pthread_t));
	master_ctime = (double *)malloc(max_thread_num*sizeof(double));
	master_mtime = (double *)malloc(max_thread_num*sizeof(double));
	master_cctime = (double *)malloc(max_thread_num*sizeof(double));
	for(i = 0; i < max_thread_num; i++)
	{
		int* ii = &i;
		pthread_create(&thread_pool[i], NULL, Master_thread_routine, (void *) i);
		master_ctime[i] = 0.0;
		master_mtime[i] = 0.0;
	}
	pthread_mutex_init(&master_thread_pool_lock,NULL);
	return;
}

void Master_destroy_work_pool(int max_thread_num)
{
	int i;
	for(i = 0; i < max_thread_num; i++)
	{
		pthread_join(thread_pool[i],NULL);
	}
	fflush(scheduler_fp);
	fclose(scheduler_fp);
	free(thread_pool);
	thread_pool = NULL;
	pthread_mutex_destroy(&master_thread_pool_lock);
	return;
}


void Master_cancel_handler(void* arg)
{
	// printf("cancel_handler executing...\n");
	pthread_mutex_unlock ((pthread_mutex_t*)arg);
}

void write_dag_finish(int node_id)
{
    DAGPattern dag_pattern = get_dag_pattern();
    dag_pattern.dag_pattern_finish[node_id] = 1;
    FILE *fp = fopen("run_log","a+");
    fprintf(fp,"%d\n",node_id);
    fflush(fp);
    fclose(fp);
    return;
}
void* Master_thread_routine(void *arg)
{
	struct timeval sta1,sta2,ed1,ed2;
	BlockT *data_block;
	int thread_id = (int)arg;
	int target_processid = thread_id + 1;
	int oldtype,dag_node_sum,i,data_send_cnt;
	double percent,timelen,time_cal,thread_other_val;
	int signal;
	int data_recv_cnt,nece_node_id,ack;
	MPI_Status status;
	DAGPattern dagpattern = get_dag_pattern();
	dag_node_sum = dagpattern.rect_size.row * dagpattern.rect_size.col;
	MPI_Recv(&signal,1,MPI_INT,target_processid,0,MPI_COMM_WORLD,&status);
	while(1)
	{
		Task* task_ptr = get_task();
		if(task_ptr == NULL)
		{
			if(is_computable_finished())
			{
				data_send_cnt = -1;
				MPI_Send(&data_send_cnt,1,MPI_INT,target_processid,10,MPI_COMM_WORLD);
				data_block = NULL;
				pthread_exit(NULL);
			}
			else
			{
			}
		}
		else
		{
			// BlockT *data_block;
			/*
			 * Receive thread singal message
			 */
			data_block = data_mapping_by_node_id(task_ptr->node_id);
			//fprintf(stderr,"%d %d\n",task_ptr->node_id,dagpattern.dag_pattern_finish[task_ptr->node_id]);
			if(dagpattern.dag_pattern_finish[task_ptr->node_id] == 1)
			{
//				fprintf(stderr,"read!!\n");
				read_datablock(task_ptr->node_id);	
				add_dag_node_finish_stack(task_ptr->node_id);
				percent = get_finish_node_num()/(double)dag_node_sum;
				printf("Process %d: %.6lf percent finish.\n",thread_id,percent*100);
				fprintf(scheduler_fp,"Process %d: %.6lf percent finish.\n",thread_id,percent*100);
				continue;
			}
			Master_add_timeout_queue(thread_id,task_ptr->node_id);
			//register_task_nodeid_by_threadid(task_ptr->node_id,thread_id);
			/*-------------------------------------------------------------------------------------------------------*/
			DAGPattern dag_pattern = get_dag_pattern();
			pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
			pthread_cleanup_push(Master_cancel_handler,(void*) &(master_thread_pool_lock));
			pthread_mutex_lock(&master_thread_pool_lock);
			data_send_cnt = dag_pattern.dag_pattern_element[task_ptr->node_id].data_pre_cnt;
			gettimeofday(&sta2,NULL);

			MPI_Send(&data_send_cnt,1,MPI_INT,target_processid,10,MPI_COMM_WORLD);
			MPI_Send(dag_pattern.dag_pattern_element[task_ptr->node_id].data_prefix_id,data_send_cnt,MPI_INT,target_processid,20,MPI_COMM_WORLD);

			MPI_Recv(&data_recv_cnt,1,MPI_INT,target_processid,30,MPI_COMM_WORLD,&status);
			for(i = 0; i < data_recv_cnt; i++)
			{
				MPI_Recv(&nece_node_id,1,MPI_INT,target_processid,1000+i,MPI_COMM_WORLD,&status);
				data_block = data_mapping_by_node_id(nece_node_id);
				send_datablock(target_processid,data_block);
			}
			data_block = data_mapping_by_node_id(task_ptr->node_id);
			send_datablockpos(target_processid,data_block);

			pthread_mutex_unlock(&master_thread_pool_lock);
			pthread_cleanup_pop(0);
			pthread_setcanceltype(oldtype, NULL);
			/**
			 * Send compute data_block and necessary data
			 */
			/**
			 * receive ack signal
			 */
			MPI_Recv(&ack,1,MPI_INT,target_processid,40,MPI_COMM_WORLD,&status);
			pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
			pthread_cleanup_push(Master_cancel_handler,(void*) &(master_thread_pool_lock));
			pthread_mutex_lock(&master_thread_pool_lock);
			MPI_Recv(&timelen,1,MPI_DOUBLE,target_processid,40,MPI_COMM_WORLD,&status);
			MPI_Recv(&time_cal,1,MPI_DOUBLE,target_processid,40,MPI_COMM_WORLD,&status);
			recv_datablock(target_processid);

				
			gettimeofday(&ed2,NULL);
			master_ctime[thread_id] += ed2.tv_sec - sta2.tv_sec + (ed2.tv_usec - sta2.tv_usec)/1000000.0;
			master_mtime[thread_id] += ed2.tv_sec - sta2.tv_sec + (ed2.tv_usec - sta2.tv_usec)/1000000.0;
			master_mtime[thread_id] -= timelen;
			master_cctime[thread_id] += timelen;
			time_cal_thread += time_cal;
			if( Master_remove_timeoutnode_by_nodeid(task_ptr->node_id) == 0 )
			{
				add_dag_node_finish_stack(task_ptr->node_id);
				percent = get_finish_node_num()/(double)dag_node_sum;
				printf("Process %d: %.6lf percent finish.\n",thread_id,percent*100);
				fprintf(scheduler_fp,"Process %d: %.6lf percent finish.\n",thread_id,percent*100);
				write_datablock(task_ptr->node_id);
				write_dag_finish(task_ptr->node_id);
			}
			pthread_mutex_unlock(&master_thread_pool_lock);
			pthread_cleanup_pop(0);
			pthread_setcanceltype(oldtype, NULL);
			free(data_block);
			free(task_ptr);
			task_ptr = NULL;
		}
	}
	free(data_block);
	data_block = NULL;
	return NULL;
}

void Master_restart_thread(int thread_id)
{
	pthread_mutex_lock(&master_thread_pool_lock);
	if (pthread_cancel(thread_pool[thread_id]))
	{
		fprintf(stderr, "Cann't cancel thread %d\n", thread_id);
		pthread_mutex_unlock(&master_thread_pool_lock);
		return;
	}
	int *iid = &thread_id;
	pthread_create(&thread_pool[thread_id],NULL,Master_thread_routine,(void* )iid);
	pthread_mutex_unlock(&master_thread_pool_lock);
	return ;
}

