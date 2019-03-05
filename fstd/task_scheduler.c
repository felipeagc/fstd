#include "task_scheduler.h"
#include <stdlib.h>

FSTD_THREAD_LOCAL uint32_t fstd_worker_id = 0;

static inline void
task_init(fstd_task_t *task, fstd_routine_t routine, void *args) {
  task->routine = routine;
  task->args = args;
  task->next = NULL;
}

static inline void
task_append(fstd_task_t *task, fstd_routine_t routine, void *args) {
  if (task->next != NULL) {
    return task_append(task->next, routine, args);
  } else {
    task->next = (fstd_task_t *)malloc(sizeof(fstd_task_t));
    task_init(task->next, routine, args);
  }
}

void *worker_routine(void *args) {
  fstd_worker_t *worker = (fstd_worker_t *)args;

  fstd_worker_id = worker->id;

  fstd_task_t *curr_task = NULL;

  while (1) {
    while (!worker->scheduler->stop && worker->scheduler->task == NULL) {
      fstd_cond_wait(&worker->scheduler->wait_cond, &worker->scheduler->mutex);
    }

    if (worker->scheduler->stop) {
      fstd_mutex_unlock(&worker->scheduler->mutex);
      return NULL;
    }

    curr_task = worker->scheduler->task;
    if (worker->scheduler->task != NULL) {
      worker->scheduler->task = worker->scheduler->task->next;
    }
    fstd_mutex_unlock(&worker->scheduler->mutex);

    if (curr_task != NULL) {
      curr_task->routine(curr_task->args);

      free(curr_task);
    }

    fstd_cond_signal(&worker->scheduler->done_cond);
  }

  return NULL;
}

static inline void
worker_init(fstd_worker_t *worker, fstd_task_scheduler_t *scheduler, uint32_t id) {
  worker->id = id;
  worker->scheduler = scheduler;
  worker->working = false;
  fstd_thread_init(&worker->thread, worker_routine, worker);
  fstd_mutex_init(&worker->mutex);
}

static inline void worker_wait(fstd_worker_t *worker) {
  fstd_thread_join(worker->thread, NULL);
}

static inline void worker_destroy(fstd_worker_t *worker) {
  fstd_mutex_destroy(&worker->mutex);
}

void fstd_scheduler_init(fstd_task_scheduler_t *scheduler, uint32_t num_workers) {
  scheduler->num_workers = num_workers;
  scheduler->workers =
      (fstd_worker_t *)malloc(sizeof(fstd_worker_t) * scheduler->num_workers);
  scheduler->task = NULL;
  scheduler->stop = false;
  fstd_cond_init(&scheduler->wait_cond);
  fstd_cond_init(&scheduler->done_cond);
  fstd_mutex_init(&scheduler->mutex);
  for (uint32_t i = 0; i < scheduler->num_workers; i++) {
    worker_init(&scheduler->workers[i], scheduler, i);
  }
}

void fstd_scheduler_add_task(
    fstd_task_scheduler_t *scheduler, fstd_routine_t routine, void *args) {
  fstd_mutex_lock(&scheduler->mutex);
  if (scheduler->task == NULL) {
    scheduler->task = (fstd_task_t *)malloc(sizeof(fstd_task_t));
    task_init(scheduler->task, routine, args);
  } else {
    task_append(scheduler->task, routine, args);
  }
  fstd_mutex_unlock(&scheduler->mutex);

  fstd_cond_signal(&scheduler->wait_cond);
}

void fstd_scheduler_destroy(fstd_task_scheduler_t *scheduler) {
  while (1) {
    fstd_cond_wait(&scheduler->done_cond, &scheduler->mutex);
    if (scheduler->task == NULL) {
      scheduler->stop = true;
      fstd_mutex_unlock(&scheduler->mutex);
      break;
    }
    fstd_mutex_unlock(&scheduler->mutex);
  }

  fstd_cond_broadcast(&scheduler->wait_cond);

  for (uint32_t i = 0; i < scheduler->num_workers; i++) {
    worker_wait(&scheduler->workers[i]);
  }

  for (uint32_t i = 0; i < scheduler->num_workers; i++) {
    worker_destroy(&scheduler->workers[i]);
  }

  fstd_cond_destroy(&scheduler->wait_cond);
  fstd_cond_destroy(&scheduler->done_cond);
  fstd_mutex_destroy(&scheduler->mutex);

  free(scheduler->workers);
}
