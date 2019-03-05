#ifndef FSTD_THREAD_H
#define FSTD_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
#define FSTD_THREAD_LOCAL __declspec(thread)
#elif defined(__GNUC__)
#define FSTD_THREAD_LOCAL __thread
#endif

typedef void *(*fstd_routine_t)(void *);

#if defined(_WIN32)
// @todo: implement windows version
#else

#include <pthread.h>

// Pthreads version
typedef pthread_t fstd_thread_t;

typedef pthread_mutex_t fstd_mutex_t;

typedef pthread_cond_t fstd_cond_t;
#endif

void fstd_thread_init(fstd_thread_t *thread, fstd_routine_t routine, void *arg);
void fstd_thread_join(fstd_thread_t thread, void **retval);

void fstd_mutex_init(fstd_mutex_t *mutex);
void fstd_mutex_destroy(fstd_mutex_t *mutex);
void fstd_mutex_lock(fstd_mutex_t *mutex);
void fstd_mutex_unlock(fstd_mutex_t *mutex);

void fstd_cond_init(fstd_cond_t *cond);
void fstd_cond_destroy(fstd_cond_t *cond);
void fstd_cond_signal(fstd_cond_t *cond);
void fstd_cond_broadcast(fstd_cond_t *cond);
void fstd_cond_wait(fstd_cond_t *cond, fstd_mutex_t *mutex);

#ifdef __cplusplus
}
#endif

#endif
