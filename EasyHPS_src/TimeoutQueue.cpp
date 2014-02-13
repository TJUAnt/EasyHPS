#include "TimeoutQueue.h"
#include "WorkPool.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>
static int TimeOutQueue_timeout;
static TimeOut *timeoutqueue;
static pthread_mutex_t timeoutqueue_lock;
void init_timeout_queue(int timeout)
{
    TimeOutQueue_timeout = timeout;
    timeoutqueue = (TimeOut *)malloc(sizeof(TimeOut));
    timeoutqueue->next = NULL;
    timeoutqueue->pre = NULL;
    timeoutqueue->thread_id = -1;
    pthread_mutex_init(&timeoutqueue_lock,NULL);
}
void add_timeout_queue(int _thread_id)
{
    pthread_mutex_lock(&timeoutqueue_lock);
    TimeOut* timeoutnode = (TimeOut* )malloc(sizeof(TimeOut));
    struct timeval timeouttime;
    gettimeofday(&timeouttime,NULL);
    timeoutnode->time = timeouttime;
    timeoutnode->thread_id = _thread_id;
    timeoutnode->next = timeoutqueue->next;
    timeoutnode->pre = timeoutqueue;
    timeoutqueue->next = timeoutnode;
    if(timeoutnode->next!=NULL)
        timeoutnode->next->pre = timeoutnode;
    pthread_mutex_unlock(&timeoutqueue_lock);
}

void remove_timeoutnode_by_id(int _thread_id)
{
    TimeOut* timeoutptr = timeoutqueue->next;
    pthread_mutex_lock(&timeoutqueue_lock);
    while(timeoutptr != NULL)
    {
        if(timeoutptr->thread_id == _thread_id)
        {
            timeoutptr->pre->next = timeoutptr->next;
            if(timeoutptr->next!=NULL)
                timeoutptr->next->pre = timeoutptr->pre;
            free(timeoutptr);
            timeoutptr = NULL;
            pthread_mutex_unlock(&timeoutqueue_lock);
            return;
        }
        timeoutptr = timeoutptr->next;
    }
    pthread_mutex_unlock(&timeoutqueue_lock);
    return;
}

void check_timeout()
{
    struct timeval time_now;
    int wait_sec;
    pthread_mutex_lock(&timeoutqueue_lock);
    TimeOut* timeoutptr = timeoutqueue->next;
    TimeOut* p;
    while(timeoutptr != NULL)
    {
        gettimeofday(&time_now,NULL);
        wait_sec = time_now.tv_sec - timeoutptr->time.tv_sec;
        if(wait_sec >= TimeOutQueue_timeout)
        {
            timeoutptr->pre->next = timeoutptr->next;
            if(timeoutptr->next!=NULL)
                timeoutptr->next->pre = timeoutptr->pre;
            remove_timeoutnode_by_id(timeoutptr->thread_id);
            restart_thread(timeoutptr->thread_id);
            add_timeout_queue(timeoutptr->thread_id);
	    p = timeoutptr;
            timeoutptr = timeoutptr->next;
            free(p);
            p = NULL; 
        }else
	{
            timeoutptr = timeoutptr->next;
        }
    }
    pthread_mutex_unlock(&timeoutqueue_lock);
    return;
}

void destroy_timeout_queue()
{
    TimeOut* timeoutptr;
    while((timeoutptr=timeoutqueue->next)!=NULL)
    {
        if(timeoutqueue->next!=NULL)
        timeoutqueue->next = timeoutqueue->next->next;
        free(timeoutptr);
        timeoutptr = NULL;
    }
    free(timeoutqueue);
    timeoutqueue = NULL;
    pthread_mutex_destroy(&timeoutqueue_lock);
    return;
}
