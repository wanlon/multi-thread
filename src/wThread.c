#include <stdio.h>
#include <pthread.h>
#include "wThread.h"


void pool_init(int max_thread_num)
{
    pool = (CThread_pool *)malloc(sizeof(CThread_pool));
    if(pool == NULL) {
        printf("pool_init1\n");    
        exit(-1);
    }
    
    /*初始化线程互斥锁和条件变量*/
    pthread_mutex_init(&(pool->queue_lock), NULL);
    pthread_cond_init(&(pool->queue_ready), NULL);
 
    pool->queue_head = NULL;    
    pool->shutdown = 0;
    pool->max_thread_num = max_thread_num;
    pool->threadid = (pthread_t *)malloc(sizeof(pthread_t) * max_thread_num);
    if(pool->threadid == NULL) {
        printf("pool_init2\n");    
        exit(-1);
    }
    
    /*初始化任务队列为0*/        
    pool->cur_queue_size = 0;
    int i = 0;
 
    /*创建max_thread_num数目的线程*/
    for(i-0; i<max_thread_num; i++) {
        pthread_create(&pool->threadid[i], NULL, thread_routine, NULL);
    }
}

void *thread_routine(void *arg)
{
    printf("starting thread 0x%x\n", pthread_self());
    while(1) {
        /*因为线程中访问到临界资源（任意时刻只允许一个线程访问的资源），所以要上锁*/
        pthread_mutex_lock(&pool->queue_lock);
 
        /*如果任务等待队列为空则线程阻塞，使用了条件变量*/
        while(pool->cur_queue_size == 0 && !pool->shutdown) {
            /*pthread_cond_wait是一个原子操作，等待前会解锁，唤醒后会加锁*/
            printf("thread 0x%x is waiting\n", pthread_self());
            pthread_cond_wait(&(pool->queue_ready), &(pool->queue_lock));
        }
 
        if(pool->shutdown) {
            /*遇到break,continue,return等跳转语句，千万不要忘记先解锁*/
            pthread_mutex_unlock(&(pool->queue_lock));
            printf("thread x%x will exit\n", pthread_self());
            pthread_exit(0);
        }
        
        printf("thread 0x%x is starting to work\n", pthread_self());
        /*从任务队列中取出任务*/
        CThread_worker *worker = pool->queue_head;
        pool->queue_head = worker->next;    
        pool->cur_queue_size--;
        
        /*访问临界资源结束，要解锁，以便其他线程访问*/
        pthread_mutex_unlock(&(pool->queue_lock));
 
        /*执行任务等待队列中的任务*/
        worker->process(worker->arg);
        free(worker);
        worker = NULL;
    }
 
    pthread_exit(NULL);
}

void pool_add_worker(void *(*process)(void *arg), void *arg)
{
    //为新的任务分配内存，然后添加到任务队列中
    CThread_worker *newwork = (CThread_worker *)malloc(sizeof(CThread_worker));
    if(newwork == NULL) {
        printf("pool_add_worker\n");
        exit(-1);
    }
    newwork->process = process;
    newwork->arg = arg;
    newwork->next = NULL;    
    
    /*访问到临界资源要上锁*/
    pthread_mutex_lock(&(pool->queue_lock));
 
    /*将新任务插入到队尾*/
    CThread_worker *temp = pool->queue_head;
    if(temp == NULL) {
        pool->queue_head = newwork;
    } else{
        while(temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newwork;
    }
    /*任务等待队列的任务数加1*/
    pool->cur_queue_size++;
    
    /*解锁*/
    pthread_mutex_unlock(&(pool->queue_lock));
    
    /*发信号唤醒任意一个空闲的线程去处理新加入的任务*/
    pthread_cond_signal(&(pool->queue_ready));
    
}

void pool_destroy()
{
    if(pool->shutdown) {
        return -1; /*防止两次调用*/
    }
 
    pool->shutdown = 1;
 
    /*唤醒所有等待的线程*/
    pthread_cond_broadcast(&(pool->queue_ready));
 
    /*阻塞等待线程退出，否则子线程先退出，主线程没有回收就变成僵尸线程了*/
    int i;
    for(i=0; i<pool->max_thread_num; i++) {
        pthread_join(pool->threadid[i], NULL);
    }
    free(pool->threadid);
    pool->threadid = NULL;
 
    /*销毁等待队列*/
    CThread_worker *temp;
    while(pool->queue_head != NULL) {
        temp = pool->queue_head;
        pool->queue_head = temp->next;
        free(temp);
    }
 
    /*销毁条件互斥锁和条件变量*/
    pthread_mutex_destroy(&(pool->queue_lock));
    pthread_cond_destroy(&(pool->queue_ready));
 
    free(pool);
    /*销毁后指针置空*/
    pool = NULL;
    return 0;
}
