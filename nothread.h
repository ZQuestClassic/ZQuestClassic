#if !defined( PTHREAD_H )
#define PTHREAD_H
typedef int pthread_mutex_t;
typedef int pthread_mutexattr_t;
int pthread_mutex_init (pthread_mutex_t *,const pthread_mutexattr_t *) { return 0; }
int pthread_mutex_destroy (pthread_mutex_t *) { return 0; }
int pthread_mutex_lock (pthread_mutex_t *) { return 0; }
int pthread_mutex_timedlock(pthread_mutex_t,const struct timespec *) { return 0; }
int pthread_mutex_trylock (pthread_mutex_t *) { return 0; }
int pthread_mutex_unlock (pthread_mutex_t *) { return 0; }
#endif /* PTHREAD_H */
 
