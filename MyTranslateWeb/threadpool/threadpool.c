#include "threadpool.h"
#include "condition.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

//相当于生产者消费者模型，线程池为消费者，队列为生产者

//初始化线程池
void threadpool_init(threadpool_t *pool,int rheads)
{
    condition_init(&(pool->ready));
    pool->first=NULL;
    pool->last=NULL;
    pool->counter=0;
    pool->idle=0;
    pool->max_threads=rheads;
    pool->quit=0;
}

void* route(void* arg)
{
    threadpool_t* pool=(threadpool_t*)arg;
    int timeout=0;
    while(1)
    {
        timeout=0;
        condition_lock(&pool->ready);
        pool->idle++;
        //任务队列没有任务，而且没有得到退出通知
        while(pool->first==NULL&&pool->quit!=1)
        {
            printf("%lu thread is waittind...\n",pthread_self());
            //condition_wait(&pool->ready);
            struct timespec sp;
            clock_gettime(CLOCK_REALTIME,&sp);
            sp.tv_sec+=2;
            timeout=condition_timedwait(&pool->ready,&sp);
            if(timeout==ETIMEDOUT)
            {    
                timeout=1;
                break;
            }
        }
        pool->idle--;
        //等到任务
        if(pool->first!=NULL)
        {
            condition_unlock(&pool->ready);//防止任务执行时间过长，导致别的任务无法操作队列，先解锁完了之后加锁
            task_t* node=pool->first;
            pool->first=node->next;
            (node->run)(node->arg);//执行回调函数
            condition_lock(&pool->ready);
            free(node);
        }
        //退出标志为1，销毁线程池
        if(pool->quit==1&&pool->first==NULL)
        {
            pool->counter--;
            if(pool->counter==0)
                condition_signal(&pool->ready);
            condition_unlock(&pool->ready);
            break;
        }
        //超时处理
        if(timeout==1&&pool->first==NULL)
        {
            condition_unlock(&pool->ready);
            break;
        }
        condition_unlock(&pool->ready);
    }
    printf("%lu thread exited\n",pthread_self());
}

//往线程池中添加任务
void threadpool_add(threadpool_t *pool,void *(*run)(void*),void *arg)
{
    task_t *node=(task_t *)malloc(sizeof(task_t));
    node->run=run;
    node->arg=arg;
    node->next=NULL;
    
    condition_lock(&pool->ready);
    if(pool->first==NULL)
    {
        pool->first=node;
    }
    else
    {
        pool->last->next=node;
    }
    pool->last=node;

    if(pool->idle>0)
    {
        condition_signal(&pool->ready);
    }
    else if(pool->counter<pool->max_threads)
    {
        pthread_t tid;
        pthread_create(&tid,NULL,route,(void*)pool);
        pool->counter++;
    }
    condition_unlock(&(pool->ready));
}

//销毁线程池
void threadpool_destroy(threadpool_t *pool)
{
    condition_lock(&pool->ready);
    if(pool->quit==1)//可能有多个人同时发销毁通知
    {
        condition_unlock(&pool->ready);
        return;
    }

    pool->quit=1;

    if(pool->idle>0)//让所有空闲的线程都退出
        condition_broadcast(&pool->ready);
    while(pool->counter>0)
    {
        condition_wait(&pool->ready);//在所有进程退出之前，不应该销毁线程
    }

    condition_destroy(&pool->ready);
    condition_unlock(&pool->ready);
}

