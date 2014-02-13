#include "WorkPool.h"
#include "TaskQueue.h"
#include "TimeoutQueue.h"
#include "Scheduler.h"
#include "DAGNodeManager.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
pthread_t* thread_pool;

void init_work_pool(int max_thread_num, int mypid)
{
    int i;
    WorkPoolProcess workPoolProcess;
    if(mypid == 0)
    {
	workPoolProcess = GetMasterWorkPoolProcess();
        Init_Master_Work_Pool_lock();
    }
    else
    {
        workPoolProcess = GetSlaveWorkPoolProcess();
        Init_Slave_Work_Pool_lock();
    }
    thread_pool = (pthread_t*)malloc(max_thread_num*sizeof(pthread_t));
    memset(thread_pool,0,sizeof(thread_pool));
    for(i = 0; i < max_thread_num; i++)
    {
        pthread_create(&thread_pool[i], NULL, workPoolProcess, (void *) i);
    }
    return;
}

void destroy_work_pool(int max_thread_num)
{
    int i;
    for(i = 0; i < max_thread_num; i++)
    {
    //    printf("---------%d--------\n",i);
        if(thread_pool[i] != 0)
            pthread_join(thread_pool[i],NULL);
    }
//    free(thread_pool);
    pthread_mutex_destroy(&thread_pool_lock);
    return;
}
void cancel_handler(void* arg)
{
    // printf("cancel_handler executing...\n");
    pthread_mutex_unlock ((pthread_mutex_t*)arg);
}
void* thread_routine(void *arg)
{
    int thread_id = (int)arg;
    int oldtype,dag_node_sum;
    double percent;
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
            BlockT *data_block;
            //MPI_Receive();
            pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
	    pthread_cleanup_push(cancel_handler,(void*) &(thread_pool_lock));
            pthread_mutex_lock(&thread_pool_lock);
	    data_block = data_mapping_by_node_id(task_ptr->node_id);
            add_timeout_queue(thread_id);
            SchedulerArgs schedulerArgs = GetSchedulerArg();
            
            (*(task_ptr->process))(thread_id, schedulerArgs.dag_size, data_block);
            
            
            //MPI_Receive();receive ack signal
            //MPI_Receive();receive compute data;
            remove_timeoutnode_by_id(thread_id);
            add_dag_node_finish_stack(task_ptr->node_id);
            percent = get_finish_node_num()/(double)dag_node_sum;
	    printf("Thread %d: %.6lf percent finish.\n",thread_id,percent*100);      
	    pthread_mutex_unlock(&thread_pool_lock);
            pthread_cleanup_pop(0);	
	    pthread_setcanceltype(oldtype, NULL);
	    free(data_block);
            free(task_ptr);  
//            printf("* *\n");
	}
    }
//    printf("%d exit successfully!!\n",thread_id);
    return NULL;
}
void restart_thread(int thread_id, WorkPoolProcess_t workPoolProcess)
{
    pthread_mutex_lock(&thread_pool_lock);
    if (pthread_cancel(thread_pool[thread_id]))
    {
        fprintf(stderr, "Cann't cancel thread %d\n", thread_id);
        return;
    }
    pthread_create(&thread_pool[thread_id],NULL,workPoolProcess,(void* )thread_id);
    pthread_mutex_unlock(&thread_pool_lock);
    return ;
}
*/
