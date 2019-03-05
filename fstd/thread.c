#include "thread.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
// @todo: implement windows version
#else

void fstd_thread_init(fstd_thread_t *thread, fstd_routine_t routine, void *arg) {
  pthread_create(thread, NULL, routine, arg);
}

void fstd_thread_join(fstd_thread_t thread, void **retval) {
  pthread_join(thread, retval);
}

void fstd_mutex_init(fstd_mutex_t *mutex) { pthread_mutex_init(mutex, NULL); }

void fstd_mutex_destroy(fstd_mutex_t *mutex) { pthread_mutex_destroy(mutex); }

void fstd_mutex_lock(fstd_mutex_t *mutex) { pthread_mutex_lock(mutex); }

void fstd_mutex_unlock(fstd_mutex_t *mutex) { pthread_mutex_unlock(mutex); }

void fstd_cond_init(fstd_cond_t *cond) { pthread_cond_init(cond, NULL); }

void fstd_cond_destroy(fstd_cond_t *cond) { pthread_cond_destroy(cond); }

void fstd_cond_signal(fstd_cond_t *cond) { pthread_cond_signal(cond); }

void fstd_cond_broadcast(fstd_cond_t *cond) { pthread_cond_broadcast(cond); }

void fstd_cond_wait(fstd_cond_t *cond, fstd_mutex_t *mutex) {
  pthread_cond_wait(cond, mutex);
}

#endif

#ifdef __cplusplus
}
#endif
