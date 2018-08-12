#include "condition.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>



//int condition_init(condition_t *pc)
//{
//    int ret=0;
//    if((ret=pthread_mutex_init(&pc.mutex,NULL))!=0)
//    {
//        return ret;
//    }
//    if(ret=(pthread_cond_init(&&pc.cond,NULL))!=0)
//    {
//        return ret;
//    }   
//}

int condition_init(condition_t* pc)
{
    int ret=0;
    do
    {
        if((ret=pthread_mutex_init(&(pc->mutex),NULL))!=0) 
            break;
        if((ret=pthread_cond_init(&(pc->cond),NULL))!=0)
            break;
    }while(0);
    return ret;
}

int condition_lock(condition_t *pc)
{
    return pthread_mutex_lock(&(pc->mutex));
}

int condition_unlock(condition_t *pc)
{
    return pthread_mutex_unlock(&(pc->mutex));
}

int condition_wait(condition_t *pc)
{
    return pthread_cond_wait(&(pc->cond),&(pc->mutex));
}

int condition_timedwait(condition_t *pc,struct timespec *pt)
{
    return pthread_cond_timedwait(&(pc->cond),&(pc->mutex),pt);
}

int condition_signal(condition_t *pc)
{
    return pthread_cond_signal(&(pc->cond));
}

int condition_broadcast(condition_t *pc)
{
    return pthread_cond_broadcast(&(pc->cond));
}

int condition_destroy(condition_t *pc)
{
    pthread_mutex_destroy(&(pc->mutex));
    pthread_cond_destroy(&(pc->cond));
    return 0;
}




