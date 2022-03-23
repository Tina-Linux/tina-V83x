#ifndef DTV_SEMAPHORE_H
#define DTV_SEMAPHORE_H

#include <pthread.h>
typedef struct dtv_sem_t
{
	pthread_cond_t	condition;
	pthread_mutex_t mutex;
	unsigned int	semval;
}dtv_sem_t;

int32_t dtv_sem_init(dtv_sem_t* tsem, uint32_t val);
void dtv_sem_deinit(dtv_sem_t* tsem);
void dtv_sem_down(dtv_sem_t* tsem);
void dtv_sem_up(dtv_sem_t* tsem);
void dtv_sem_reset(dtv_sem_t* tsem);
void dtv_sem_wait(dtv_sem_t* tsem);
void dtv_sem_signal(dtv_sem_t* tsem);

#define dtv_mutex_lock(x)		pthread_mutex_lock(x)
#define dtv_mutex_unlock(x)	pthread_mutex_unlock(x)

#define dtv_cond_wait_while_exp(tsem, expression) \
	pthread_mutex_lock(&tsem.mutex); \
	while (expression) { \
		pthread_cond_wait(&tsem.condition, &tsem.mutex); \
	} \
	pthread_mutex_unlock(&tsem.mutex);

#define dtv_cond_wait_if_exp(tsem, expression) \
	pthread_mutex_lock(&tsem.mutex); \
	if (expression) { \
		pthread_cond_wait(&tsem.condition, &tsem.mutex); \
	} \
	pthread_mutex_unlock(&tsem.mutex);
#endif//DTV_SEMAPHORE_H
