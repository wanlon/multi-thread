#ifndef WTHREAD_H
#define WTHREAD_H

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>

/*
*线程池里所有运行和等待的队列都是一个CThread_worker结构
*由于所有的CThread_worker结构都在队列中，所以是队列中的一个节点
*/
typedef struct worker
{
    /*回调函数，当任务运行时会调用此函数，也可以声明为其他形式*/
    void *(*process)(void *arg);
    /*回调函数的参数*/
    void *arg;              
    struct worker *next;
}CThread_worker;
 
/*线程池的结构*/
typedef struct
{
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_ready;
 
    /*指向任务等待队列的队头*/
    CThread_worker *queue_head;
    
    /*是否销毁线程池*/
    int shutdown;
    
    /*线程ID，使用堆空间来分配内存*/
    pthread_t *threadid;
 
    /*线程池中线程的数目*/
    int max_thread_num;
 
    /*当前等待队列的任务数目*/
    int cur_queue_size;
}CThread_pool;

void pool_init(int max_thread_num);
void *thread_routine(void *arg);
void pool_add_worker(void *(*process)(void *arg), void *arg);
void pool_destroy();

static CThread_pool *pool = NULL;

#endif
