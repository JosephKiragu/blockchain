#ifndef THREAD_POOL_H
#define	THREAD_POOL_H

#include <pthread.h>
#include <stdbool.h>

#include "project.h"
#include "concurrent_queue.h"

#define NUM_TASKS 100

typedef struct {
	void (*function)(void *);
	void *argument;
} Task;

typedef struct {
	pthread_t *threads;
	int thread_count;
	ConcurrentQueue task_queue;
	bool stop;
	pthread_mutex_t lock;
	pthread_cond_t notify;
	// new members
	int completed_tasks;
	pthread_mutex_t completed_tasks_lock;
	bool shutdown;
} ThreadPool;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

ErrorCode thread_pool_create(ThreadPool *pool, int thread_count, ErrorDetails *err);
ErrorCode thread_pool_add_task(ThreadPool *pool, void (*function)(void *), void *argument, ErrorDetails *err);
void thread_pool_get_completed_tasks(ThreadPool *pool, int *completed);
void thread_pool_wait(ThreadPool *pool);
void thread_pool_destroy(ThreadPool *pool);


#endif