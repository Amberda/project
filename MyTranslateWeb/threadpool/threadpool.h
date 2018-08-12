#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include "condition.h"


//任务结构体，将所有的任务放入线程池的等待队列
typedef struct task
{
    void *(*run)(void *);//回调的任务
    void *arg;//回调函数的参数
    struct task *next;//下一个节点的指针
}task_t;

typedef struct threadpool
{
    condition_t ready;//同步和互斥
    task_t *first;    //任务队列队头
    task_t *last;     //任务队列队尾
    int counter;     //当前线程池中有多少个线程
    int idle;         //多少个空闲线程
    int max_threads;   //线程池允许创建的线程最大个数
    int quit;         //销毁线程池的时候置1

}threadpool_t;

//初始化线程池
void threadpool_init(threadpool_t *pool,int rheads);

//往线程池中添加任务
void threadpool_add(threadpool_t *pool,void *(*run)(void*),void *arg);

//销毁线程池
void threadpool_destroy(threadpool_t *pool);

#endif //__THREADPOOL_H__
