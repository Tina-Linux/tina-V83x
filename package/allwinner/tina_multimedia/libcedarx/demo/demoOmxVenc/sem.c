#include <stdlib.h>
#include "sem.h"

OMXSem* t_sem_new (void)
{
    OMXSem *sem = (OMXSem *)malloc(sizeof(OMXSem));

    if(!sem)
        return NULL;

    pthread_cond_init(&sem->condition, NULL);
    pthread_mutex_init(&sem->mutex, NULL);
    sem->counter = 0;

    return sem;
}


void t_sem_free (OMXSem * sem)
{
    pthread_cond_destroy(&sem->condition);
    pthread_mutex_destroy(&sem->mutex);
    free (sem);
}


void t_sem_down (OMXSem * sem)
{
    pthread_mutex_lock (&sem->mutex);

    while (sem->counter == 0)
    {
        pthread_cond_wait(&sem->condition, &sem->mutex);
    }

    sem->counter--;

    pthread_mutex_unlock (&sem->mutex);
}


void t_sem_up (OMXSem * sem)
{
    pthread_mutex_lock (&sem->mutex);

    sem->counter++;
    pthread_cond_signal (&sem->condition);

    pthread_mutex_unlock (&sem->mutex);
}
