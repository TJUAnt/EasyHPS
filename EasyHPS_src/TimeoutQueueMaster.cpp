#include "DAGNodeManager.h"
#include "TimeoutQueueMaster.h"
#include "WorkPoolMaster.h"
#include "TaskQueue.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <mpi.h>
static int master_TimeOutQueue_timeout;
static TimeOut *mastertimeoutqueue;
static pthread_mutex_t master_timeoutqueue_lock;
static pthread_t time_out_master_tid;
//void register_task_nodeid_by_threadid(int node_id,int thread_id)
//{
//	task_register[thread_id] = node_id;
//}
//int get_task_nodeid_by_threadid(int thread_id)
//{
//	return task_register[thread_id];
//}
static FILE *fault_tolerance_fp_master;
void Master_init_timeout_queue(int timeout)
{
	fault_tolerance_fp_master = fopen("EasyHPS_fault_tolerance_master_log.txt","w");
	master_TimeOutQueue_timeout = timeout;
	mastertimeoutqueue = (TimeOut *)malloc(sizeof(TimeOut));
	mastertimeoutqueue->next = NULL;
	mastertimeoutqueue->pre = NULL;
	mastertimeoutqueue->thread_id = -1;
	mastertimeoutqueue->node_id = -1;
	pthread_mutex_init(&master_timeoutqueue_lock,NULL);
	pthread_create(&time_out_master_tid,NULL,Master_check_timeout_routine,NULL);
}

void print_master_timeout_queue()
{
	TimeOut* ptr = mastertimeoutqueue->next;
	printf("---------------------\n");
	while(ptr!=NULL)
	{
		printf("[%d] ",ptr->node_id);
		ptr = ptr->next;
	}
	printf("\n");
	return;
}
void Master_add_timeout_queue(int _thread_id,int _node_id)
{
	pthread_mutex_lock(&master_timeoutqueue_lock);
	TimeOut* timeoutnode = (TimeOut* )malloc(sizeof(TimeOut));
	struct timeval timeouttime;
	gettimeofday(&timeouttime,NULL);
	timeoutnode->time = timeouttime;
	timeoutnode->node_id = _node_id;
	timeoutnode->thread_id = _thread_id;
	timeoutnode->next = mastertimeoutqueue->next;
	timeoutnode->pre = mastertimeoutqueue;
	mastertimeoutqueue->next = timeoutnode;
	if(timeoutnode->next!=NULL)
		timeoutnode->next->pre = timeoutnode;
	pthread_mutex_unlock(&master_timeoutqueue_lock);
}

int Master_remove_timeoutnode_by_nodeid(int _node_id)
{
	pthread_mutex_lock(&master_timeoutqueue_lock);
	TimeOut* timeoutptr = mastertimeoutqueue->next;
	while(timeoutptr != NULL)
	{
		if(timeoutptr->node_id == _node_id)
		{
			timeoutptr->pre->next = timeoutptr->next;
			if(timeoutptr->next!=NULL)
				timeoutptr->next->pre = timeoutptr->pre;
			free(timeoutptr);
			timeoutptr = NULL;
			pthread_mutex_unlock(&master_timeoutqueue_lock);
			return 0;
		}
		timeoutptr = timeoutptr->next;
	}
	pthread_mutex_unlock(&master_timeoutqueue_lock);
	return -1;
}

void* Master_check_timeout_routine(void *ptr)
{
	struct timeval time_now;
	int wait_sec;
	int* register_table;
	//pthread_mutex_lock(&master_timeoutqueue_lock);
	TimeOut* timeoutptr = mastertimeoutqueue->next;
	int t1,t2;
	int thread_id,node_id;
	while(1)
	{
		//print_master_timeout_queue();
		if(is_computable_finished())
		{
			return NULL;
		}
		pthread_mutex_lock(&master_timeoutqueue_lock);
		if(timeoutptr == NULL){
			timeoutptr = mastertimeoutqueue->next;
			pthread_mutex_unlock(&master_timeoutqueue_lock);
			continue;
		}else
		{
			thread_id = timeoutptr->thread_id;
			node_id = timeoutptr->node_id;
			t1 = timeoutptr->time.tv_sec;
			t2 = timeoutptr->time.tv_usec;
		}	
		pthread_mutex_unlock(&master_timeoutqueue_lock);
		gettimeofday(&time_now,NULL);
		wait_sec = (time_now.tv_sec - t1)*1000000 + time_now.tv_usec - t2;
		if(wait_sec >= master_TimeOutQueue_timeout)
		{
			fprintf(fault_tolerance_fp_master,"Master timeout dectected!! Process %d!, node: %d!\n",thread_id,node_id);
			fprintf(fault_tolerance_fp_master,"Adjust the timeout!\n");
			master_TimeOutQueue_timeout *= 2;
			if(Master_remove_timeoutnode_by_nodeid(node_id) == 0){
				add_task(node_id);   	
			}
		}
		timeoutptr = timeoutptr->next;
	}
	
	//pthread_mutex_unlock(&master_timeoutqueue_lock);
	return NULL;
}

void Master_destroy_timeout_queue()
{
	pthread_join(time_out_master_tid,NULL);
	TimeOut* timeoutptr;
	while((timeoutptr=mastertimeoutqueue->next)!=NULL)
	{
		if(mastertimeoutqueue->next!=NULL)
			mastertimeoutqueue->next = mastertimeoutqueue->next->next;
		free(timeoutptr);
		timeoutptr = NULL;
	}
	free(mastertimeoutqueue);
	mastertimeoutqueue = NULL;
	fflush(fault_tolerance_fp_master);
	fclose(fault_tolerance_fp_master);
	pthread_mutex_destroy(&master_timeoutqueue_lock);
	return;
}
