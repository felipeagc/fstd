#ifndef FSTD_TASK_SCHEDULER_H
#define FSTD_TASK_SCHEDULER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "thread.h"
#include <stdbool.h>
#include <stdint.h>

extern FSTD_THREAD_LOCAL uint32_t fstd_worker_id;

typedef struct fstd_task_t {
  struct fstd_task_t *next;
  fstd_routine_t routine;
  void *args;
} fstd_task_t;

typedef struct {
  uint32_t id;
  struct fstd_task_scheduler *scheduler;
  bool working;
  fstd_mutex_t mutex;
  fstd_thread_t thread;
} fstd_worker_t;

typedef struct fstd_task_scheduler {
  uint32_t num_workers;
  fstd_worker_t *workers;
  fstd_mutex_t mutex;
  fstd_cond_t wait_cond;
  fstd_cond_t done_cond;
  fstd_task_t *task;
  bool stop;
} fstd_task_scheduler_t;

void fstd_scheduler_init(fstd_task_scheduler_t *scheduler, uint32_t num_workers);

void fstd_scheduler_add_task(
    fstd_task_scheduler_t *scheduler, fstd_routine_t routine, void *args);

void fstd_scheduler_destroy(fstd_task_scheduler_t *scheduler);

#ifdef __cplusplus
}
#endif

#endif
