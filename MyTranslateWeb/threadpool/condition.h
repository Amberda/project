#ifndef __CONDITION_H__
#define __CONDITION_H__

#include <pthread.h>

//同步与互斥
typedef struct condition_t
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
}condition_t;


int condition_init(condition_t* pc);

int condition_lock(condition_t *pc);

int condition_unlock(condition_t *pc);

int condition_wait(condition_t *pc);

int condition_timedwait(condition_t *pc,struct timespec *pt);

int condition_signal(condition_t *pc);

int condition_broadcast(condition_t *pc);

int condition_destroy(condition_t *pc);


#endif //__CONDITION_H__
