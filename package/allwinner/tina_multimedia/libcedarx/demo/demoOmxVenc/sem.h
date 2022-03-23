#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef SEM_H
#define SEM_H

#include <pthread.h>


typedef struct OMXSem
{
  pthread_cond_t condition;
  pthread_mutex_t mutex;
  int counter;
}OMXSem;


OMXSem *t_sem_new (void);
void t_sem_free (OMXSem * sem);
void t_sem_down (OMXSem * sem);
void t_sem_up (OMXSem * sem);

#endif /* SEM_H */

#ifdef __cplusplus
}
#endif /* __cplusplus */
