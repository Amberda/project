#include "threadpool.h"

//线程需要执行的动作
void *run(void *arg)
{
    int i = *(int*)arg;
    free(arg);
    printf("%lu working %d\n",pthread_self(),i);
    sleep(1);
}

int main()
{
    threadpool_t pool;

    //初始化 最大创建3个线程池
    threadpool_init(&pool,3);
    int i;
    //给线程池中添加10个任务
    for(i=0;i<10;i++)
    {
        int  *p=(int*)malloc(sizeof(int));
        *p=i;
        threadpool_add(&pool,run,(void*)p);
    }

    //销毁线程池
    threadpool_destroy(&pool);
    return 0;
}
