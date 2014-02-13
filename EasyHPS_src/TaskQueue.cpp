#include "TaskQueue.h"
#include "DAGNodeManager.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mpi.h>
static Task* taskqueue;

static pthread_mutex_t taskqueue_lock;

static pthread_cond_t taskqueue_ready; 

static int taskfinish;

void init_task_queue()
{
    int mypid;
    MPI_Comm_rank(MPI_COMM_WORLD,&mypid);
    //    printf("%d init_task_queue\n",mypid);
    taskfinish = 0;
    taskqueue = (Task *)malloc(sizeof(Task));
    taskqueue->next = NULL;
    taskqueue->process = NULL;
    taskqueue->node_id = -1;
    pthread_mutex_init(&taskqueue_lock,NULL);
    pthread_cond_init(&taskqueue_ready,NULL);
}

void destroy_task_queue()
{
    SetTaskQueueFinish();
    Task* taskptr;
    Task* tp = NULL;
    while((taskptr = taskqueue->next) != NULL)
    {
	tp = taskptr;
	taskptr = taskptr->next;
	free(tp);
	tp = NULL;
    }
    free(taskqueue);
    taskqueue = NULL;
    pthread_mutex_destroy(&taskqueue_lock);
}

void add_task(int node_id)
{
    DAGPattern dag_pattern = get_dag_pattern();
    Process_t process = dag_pattern.dag_pattern_element[node_id].process;
    pthread_mutex_lock(&taskqueue_lock);
    Task* task_ptr = (Task *)malloc(sizeof(Task));
    task_ptr->process = process;
    task_ptr->node_id = node_id;
    task_ptr->next = taskqueue->next;
    taskqueue->next = task_ptr;
    pthread_mutex_unlock(&taskqueue_lock);
    return;
}

void add_computable_task()
{
    int computable_node_id;
    while(1)
    {
	computable_node_id = pop_dag_node_computable_stack();
	if(computable_node_id == -1)
	    return;
	add_task(computable_node_id);
    }
    return;
}

/*
   int remove_task(int node_id)
   {
   pthread_mutex_lock(&taskqueue_lock);
   Task* task_ptr = taskqueue->next;
   while(task_ptr!=NULL)
   {
   if(task_ptr->node_id == node_id)
   break;
   task_ptr = task_ptr->next;
   }
   if(task_ptr == NULL){
   pthread_mutex_unlock(&taskqueue_lock);
   return -1;
   }
   task_ptr->pre->next = task_ptr->next;
   if(task_ptr->next != NULL)
   task_ptr->next->pre = task_ptr->pre;
   free(task_ptr);
   task_ptr = NULL;
   pthread_mutex_unlock(&taskqueue_lock);
   return 0;
   }
   */
Task* get_task()
{
    Task* taskptr = NULL;
    pthread_mutex_lock(&taskqueue_lock);
    taskptr = taskqueue->next;
    if(taskptr != NULL)
    {
	taskqueue->next = taskptr->next;
    }
    pthread_mutex_unlock(&taskqueue_lock);
    return taskptr;
}

void SetTaskQueueFinish()
{
    pthread_mutex_lock(&taskqueue_lock);
    taskfinish = 1;
    pthread_cond_broadcast(&taskqueue_ready);
    pthread_mutex_unlock(&taskqueue_lock);
}

