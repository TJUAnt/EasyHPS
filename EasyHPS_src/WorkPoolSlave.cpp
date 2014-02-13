#include "WorkPoolSlave.h"
#include "TaskQueue.h"
#include "SchedulerSlave.h"
#include "SchedulerMaster.h"
#include "TimeoutQueueSlave.h"
#include "DAGNodeManager.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static pthread_t* thread_pool;

static pthread_mutex_t slave_thread_pool_lock;

static WorkPoolProcess_t slaveWorkPoolProcess;

static double* thread_time;

void SetSlaveWorkPoolProcess()
{
	slaveWorkPoolProcess = Slave_thread_routine;
	return ;
}

WorkPoolProcess_t GetSlaveWorkPoolProcess()
{
	return slaveWorkPoolProcess;
}

double get_thread_cal_time(int max_thread_num)
{
	int i;
	double ret = 0.0;
	for(i = 0; i < max_thread_num; i++)
	{
		ret += thread_time[i];
	}
	return ret;
}
void Slave_init_work_pool(int max_thread_num)
{
	int i;
	thread_pool = (pthread_t*)malloc(max_thread_num*sizeof(pthread_t));
	thread_time = (double *)malloc(max_thread_num*sizeof(double));
	//memset(thread_pool,0,sizeof(thread_pool));
	for(i = 0; i < max_thread_num; i++)
	{
		thread_time[i] = 0.0;
		int *ii = &i;
		pthread_create(&thread_pool[i], NULL, Slave_thread_routine, (void *) i);
	}
	pthread_mutex_init(&slave_thread_pool_lock,NULL);
	return;
}

void Slave_destroy_work_pool(int max_thread_num)
{
	int i;
	for(i = 0; i < max_thread_num; i++)
	{
		pthread_join(thread_pool[i],NULL);
	}
	free(thread_pool);
	thread_pool = NULL;
	free(thread_time);
	thread_time = NULL;
	pthread_mutex_destroy(&slave_thread_pool_lock);
	return;
}

void Slave_cancel_handler(void* arg)
{
	// printf("cancel_handler executing...\n");
	pthread_mutex_unlock ((pthread_mutex_t*)arg);
}

void* Slave_thread_routine(void *arg)
{
	int thread_id =(int)arg;
	int oldtype,dag_node_sum;
	double percent;
	struct timeval st,ed;
	DAGPattern dagpattern = get_dag_pattern();
	dag_node_sum = dagpattern.rect_size.row * dagpattern.rect_size.col;
	while(1)
	{
		Task* task_ptr = get_task();
		if(task_ptr == NULL)
		{
			if(is_computable_finished())
			{
				//                printf("%d exit successfully\n",thread_id);
				pthread_exit(NULL);
			}
			else
			{
			}
		}
		else
		{
			SchedulerArgs schedulerArgs;
			BlockT *data_block;
			/*
			 * Receive thread singal message
			 */
			//MPI_Receive();
			pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
			pthread_cleanup_push(Slave_cancel_handler,(void*) &(slave_thread_pool_lock));
			pthread_mutex_lock(&slave_thread_pool_lock);
			//	    printf("Slave task node id %d.\n",task_ptr->node_id);
			data_block = data_mapping_by_node_id(task_ptr->node_id);
			Slave_add_timeout_queue(thread_id,task_ptr->node_id);
			schedulerArgs = GetMasterSchedulerArgs();

			pthread_mutex_unlock(&slave_thread_pool_lock);
			pthread_cleanup_pop(0);
			pthread_setcanceltype(oldtype, NULL);
			gettimeofday(&st,NULL);
			(*(task_ptr->process))(thread_id, schedulerArgs.dag_size, data_block);
			gettimeofday(&ed,NULL);
			thread_time[thread_id] += ed.tv_sec - st.tv_sec + (ed.tv_usec - st.tv_usec)/1000000.0;
			pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
			pthread_cleanup_push(Slave_cancel_handler,(void*) &(slave_thread_pool_lock));
			pthread_mutex_lock(&slave_thread_pool_lock);

			if(Slave_remove_timeoutnode_by_id(task_ptr->node_id) == 0)
			{
				add_dag_node_finish_stack(task_ptr->node_id);
				percent = get_finish_node_num()/(double)dag_node_sum;
			}
			//	    printf("SlaveThread %d: %.6lf percent finish.\n",thread_id,percent*100);
			pthread_mutex_unlock(&slave_thread_pool_lock);
			pthread_cleanup_pop(0);
			pthread_setcanceltype(oldtype, NULL);
			free(task_ptr);
			task_ptr = NULL;
			free(data_block);
			data_block = NULL;
			//            printf("* *\n");
		}
	}
	//    printf("%d exit successfully!!\n",thread_id);
	return NULL;
}


void Slave_restart_thread(int thread_id)
{
	pthread_mutex_lock(&slave_thread_pool_lock);
	if (pthread_cancel(thread_pool[thread_id]))
	{
		fprintf(stderr, "Cann't cancel thread %d\n", thread_id);
		return;
	}
	int *iid = &thread_id;
	pthread_create(&thread_pool[thread_id],NULL,Slave_thread_routine,(void* )iid);
	pthread_mutex_unlock(&slave_thread_pool_lock);
	return ;
}

