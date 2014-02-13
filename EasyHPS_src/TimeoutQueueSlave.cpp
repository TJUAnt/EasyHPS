#include "TimeoutQueueSlave.h"
#include "WorkPoolSlave.h"
#include "DAGNodeManager.h"
#include "TaskQueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <mpi.h>
#include <pthread.h>
static int slave_TimeOutQueue_timeout;
static TimeOut *slavetimeoutqueue;
static pthread_mutex_t slave_timeoutqueue_lock;
static pthread_t time_out_slave_tid;
static FILE *fault_tolerance_fp_slave;
void Slave_init_timeout_queue(int timeout)
{
	fault_tolerance_fp_slave = fopen("EasyHPS_fault_tolerance_slave_log.txt","w+");
	slave_TimeOutQueue_timeout = timeout;
	slavetimeoutqueue = (TimeOut *)malloc(sizeof(TimeOut));
	slavetimeoutqueue->next = NULL;
	slavetimeoutqueue->pre = NULL;
	slavetimeoutqueue->thread_id = -1;
	slavetimeoutqueue->node_id = -1;
	pthread_mutex_init(&slave_timeoutqueue_lock,NULL);
	pthread_create(&time_out_slave_tid,NULL,Slave_check_timeout_routine,NULL);
}
void Slave_add_timeout_queue(int _thread_id, int _node_id)
{
	pthread_mutex_lock(&slave_timeoutqueue_lock);
	TimeOut* timeoutnode = (TimeOut* )malloc(sizeof(TimeOut));
	struct timeval timeouttime;
	gettimeofday(&timeouttime,NULL);
	timeoutnode->time = timeouttime;
	timeoutnode->thread_id = _thread_id;
	timeoutnode->node_id = _node_id;
	timeoutnode->next = slavetimeoutqueue->next;
	timeoutnode->pre = slavetimeoutqueue;
	slavetimeoutqueue->next = timeoutnode;
	if(timeoutnode->next!=NULL)
		timeoutnode->next->pre = timeoutnode;
	pthread_mutex_unlock(&slave_timeoutqueue_lock);
}
int Slave_remove_timeoutnode_by_id(int _node_id)
{
	TimeOut* timeoutptr = slavetimeoutqueue->next;
	pthread_mutex_lock(&slave_timeoutqueue_lock);
	while(timeoutptr != NULL)
	{
		if(timeoutptr->node_id == _node_id)
		{
			timeoutptr->pre->next = timeoutptr->next;
			if(timeoutptr->next!=NULL)
				timeoutptr->next->pre = timeoutptr->pre;
			free(timeoutptr);
			timeoutptr = NULL;
			pthread_mutex_unlock(&slave_timeoutqueue_lock);
			return 0;
		}
		timeoutptr = timeoutptr->next;
	}
	pthread_mutex_unlock(&slave_timeoutqueue_lock);
	return -1;
}

void* Slave_check_timeout_routine(void* ptr)
{
	struct timeval time_now;
	int wait_sec;
	//pthread_mutex_lock(&slave_timeoutqueue_lock);
	TimeOut* timeoutptr = slavetimeoutqueue->next;
	TimeOut* p;
	int t1,t2;
	int thread_id,node_id;
	int mypid;
	MPI_Comm_rank(MPI_COMM_WORLD,&mypid);
	while(timeoutptr!=NULL)
	{
		if(is_computable_finished())
		{
			return NULL;
		}
		pthread_mutex_lock(&slave_timeoutqueue_lock);
		if(timeoutptr == NULL)
		{
			timeoutptr = slavetimeoutqueue->next;
			pthread_mutex_unlock(&slave_timeoutqueue_lock);
			continue;
		}else{
			thread_id = timeoutptr->thread_id;
			node_id = timeoutptr->node_id;
			t1 = timeoutptr->time.tv_sec;
			t2 = timeoutptr->time.tv_usec;
		}
		pthread_mutex_unlock(&slave_timeoutqueue_lock);
		gettimeofday(&time_now,NULL);
		wait_sec = (time_now.tv_sec - t1)*1000000 + time_now.tv_usec - t2;
		if(wait_sec >= slave_TimeOutQueue_timeout)
		{
			fprintf(fault_tolerance_fp_slave,"Slave %d thread %d timeout detected!!\n",mypid,thread_id);
			slave_TimeOutQueue_timeout *= 2;
			if(Slave_remove_timeoutnode_by_id(node_id) == 0)
			{
				add_task(node_id);
				//Slave_restart_thread(timeoutptr->thread_id);
			}
			//Slave_add_timeout_queue(timeoutptr->thread_id);
		}
		timeoutptr = timeoutptr->next;
	}
	//pthread_mutex_unlock(&slave_timeoutqueue_lock);
	return NULL;
}

void Slave_destroy_timeout_queue()
{
	pthread_join(time_out_slave_tid,NULL);
	TimeOut* timeoutptr;
	while((timeoutptr=slavetimeoutqueue->next)!=NULL)
	{
		if(slavetimeoutqueue->next!=NULL)
			slavetimeoutqueue->next = slavetimeoutqueue->next->next;
		free(timeoutptr);
		timeoutptr = NULL;
	}
	free(slavetimeoutqueue);
	slavetimeoutqueue = NULL;
	fflush(fault_tolerance_fp_slave);
	fclose(fault_tolerance_fp_slave);
	pthread_mutex_destroy(&slave_timeoutqueue_lock);
	return;
}
