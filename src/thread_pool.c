#include "../include/thread_pool.h"

static void *thread_function(void *arg) {
	// initial state
	// thread pool exists
	// thread is newly created and idle

	// transformtion
	ThreadPool *pool = (ThreadPool*)arg;
	Task *task;

	while (1) {
		
		// lock the mutex and wait for a task stop signal
		if (pthread_mutex_lock(&(pool->lock)) != 0) {
			fprintf(stderr, "failed to lock mutex in thread function\n");
			return NULL;
		}

		while (pool->task_queue.size == 0 && !pool->shutdown) {
			if (pthread_cond_wait(&(pool->notify), &(pool->lock)) != 0) {
				pthread_mutex_unlock(&(pool->lock));
				fprintf(stderr, "failed to wait on cond var\n");
				return NULL;
			}
		}
		if (pool->shutdown && pool->task_queue.size == 0) {
			pthread_mutex_unlock(&(pool->lock));
			break;
		}

		// dequeue a task
		ErrorCode err = queue_dequeue(&(pool->task_queue), (void **)&task);
		pthread_mutex_unlock(&(pool->lock));

		if (err == ERROR_NONE && task && task->function)  {
			task->function(task->argument);

			if (pthread_mutex_lock(&(pool->completed_tasks_lock)) != 0) {
				fprintf(stderr, "failed to lock completed tasks lock\n");
				return NULL;
		}
			pool->completed_tasks++;
			printf("debug: completed task: %d\n", pool->completed_tasks); // add debug line
			if (pthread_mutex_unlock(&(pool->completed_tasks_lock)) != 0) {
				fprintf(stderr, "failed to lock completed tasks lock \n");
				return NULL;
			}
			free(task);
		}
	}

	// desried state
	/// thread ccontinoously processes tasks until stops
	return NULL;
}




ErrorCode thread_pool_create(ThreadPool *pool, int thread_count, ErrorDetails *err) {
	// initial state
	// uninitiaalized thread pool srtructure

	// transformation
	// validate input
	if (thread_count <= 0) {
		set_error(err, ERROR_INVALID_ARGUMENT, "invalid thread pool create arguments");
		return ERROR_INVALID_ARGUMENT;
	}

	// initialize pool properties
	pool->thread_count = thread_count;
	pool->stop = false;
	pool->completed_tasks = 0;
	pool->shutdown = false;

	// initialize the task queue
	ErrorCode result = queue_init(&(pool->task_queue), err);
	if (result != ERROR_NONE) {
		return result;
	}

	// allocate memory fornthe threads
	pool->threads = allocate_memory(thread_count * sizeof(pthread_t), err); // rem to free
	if (err->error_code != ERROR_NONE) {
		fprintf(stderr, "pool memory filed to be allocated\n");
		return err->error_code;
	}

	// initilize mutex 
	if (pthread_mutex_init(&(pool->lock), NULL) != 0) {
		set_error(err, ERROR_IMPL_ERROR, "mutex failed to intialize");
		free(pool->threads);
		return ERROR_IMPL_ERROR;
	}
	// initialize mutex for completed tasks
	if (pthread_mutex_init(&(pool->completed_tasks_lock), NULL) != 0) {
		set_error(err, ERROR_IMPL_ERROR, "failed to initialize completed_tasks_lock mutex");
		pthread_mutex_destroy(&(pool->lock));
		free(pool->threads);
		return ERROR_IMPL_ERROR;
	}
	// initialize condition variable
	if (pthread_cond_init(&(pool->notify), NULL) != 0) {
		set_error(err, ERROR_IMPL_ERROR, "cond failed to intialize");
		pthread_mutex_destroy(&(pool->lock));
		free(pool->threads);
		return ERROR_IMPL_ERROR;
	}
 
	// creaate worker thread"s
	for (int i = 0; i < thread_count; i++) {
		if(pthread_create(&(pool->threads[i]), NULL, thread_function, (void*)pool) != 0) {
			fprintf(stderr, "failed to create thread %d\n", i);
			set_error(err, ERROR_IMPL_ERROR, "failed to create threads");
			thread_pool_destroy(pool);
			return ERROR_IMPL_ERROR;

		}
	}
	// Desired state
	// fully initialized thread pool with worker threads runnning
	return ERROR_NONE;
}

ErrorCode thread_pool_add_task(ThreadPool *pool, void (*function)(void *), void *argument, ErrorDetails *err) {
	// initial stte
	// exisiting thread pool
	// new taask to be added


	// transformation
	// validte the input
	if (!pool || !function ) {
		fprintf(stderr, "invalid arguments in thread pool dd task\n");
		return ERROR_INVALID_ARGUMENT;
	}

	Task *task = (Task *)allocate_memory(sizeof(Task), err); // vriable for holding thr task to be completed
	if (err->error_code != ERROR_NONE) {
		fprintf(stderr, "failed to allocate task memory\n");
		return ERROR_MEMORY_ALLOCATION;
	}
	task->function = function;
	task->argument = argument;


	// lock the mutex
	int lock_result = pthread_mutex_lock(&(pool->lock));
	if (lock_result != 0) {
		char error_msg[256];
		strerror_r(lock_result, error_msg, sizeof(error_msg));
		fprintf(stderr, "failed to lock mutex inside thread pool add task with error: %s\n", error_msg);
		set_error(err, ERROR_IMPL_ERROR, error_msg);
		return ERROR_IMPL_ERROR;
	}

	if (pool->stop) {
		if (pthread_mutex_unlock(&(pool->lock)) != 0) {
			fprintf(stderr, "mutex failed to unlock");
			return ERROR_IMPL_ERROR;
		}
		set_error(err, ERROR_IMPL_ERROR,"thread pool failed is stopping insisde thread pool add task");
		return ERROR_IMPL_ERROR;
	}



	// enqueue the task
	ErrorCode result = queue_enqueue(&(pool->task_queue), task, err);
	if (result == ERROR_NONE) {
		pthread_cond_signal(&(pool->notify));
	} else {
		free(task);
	}
	pthread_mutex_unlock(&(pool->lock));
		

	// desired staate:
	// new task added to the tsks queue
	// waiting threads notified
	return result;
}

void thread_pool_wait(ThreadPool *pool) {
	// initial state
	// threadpool potentially proceesing tasks

	// transformation
	// validaate input
	// time_t start_time = time(NULL);
	if (!pool) {
		fprintf(stderr, "invaalid input\n");
		return;
	}
	// lock the mutex
	while (1) {
		// aattempt to lock the mutex
		if (pthread_mutex_lock(&(pool->lock)) != 0) {
			fprintf(stderr, "mutex fsiled to lock inside thred_pool_wait\n");
			return;
		}
		int completed_tasks;
		if (pthread_mutex_lock(&(pool->completed_tasks_lock)) != 0) {
			fprintf(stderr, "failed to lock completed tasks lock\n");
			pthread_mutex_unlock(&(pool->lock));
			return;
		}
		completed_tasks = pool->completed_tasks;
		pthread_mutex_unlock(&(pool->completed_tasks_lock));
		printf("debug: waiting. queue size: %d, completed tasks: %d\n", pool->task_queue.size, completed_tasks);
		
		if (pool->task_queue.size == 0 && completed_tasks == NUM_TASKS) {
			pool->shutdown = true; // signal threads to exit
			pthread_cond_broadcast(&(pool->notify)); // wake all threads
			pthread_mutex_unlock(&(pool->lock));
			break;
		}
		// unlock mtex to allow otheer threds to access the shared resources
		pthread_mutex_unlock(&(pool->lock));
		// if (time(NULL) - start_time > 60) {
		// 	printf("timeoeut waiting for tasks to complete\n");
		// 	break;
		// }

		// sleep for a short period to avoid busy waiting
		usleep(1000);

	}
	// wait for all htreads to complpete
	for (int i = 0; i < pool->thread_count; i++) {
		pthread_join(pool->threads[i], NULL);
	}

	printf("all tasks completed\n"); // debug

}


void thread_pool_destroy(ThreadPool *pool) {
	// initial atste:
	// existing thread pool with potential ongoing tasks

	// transformation
	// check if pool exists
	if (!pool) {
		fprintf(stderr, "invalid argument\n");
		return;
	}

	// // wait for threads to complete
	// thread_pool_wait(pool);

	// Set shutdown and wake up all threaads
	pthread_mutex_lock(&(pool->lock));
	pool->shutdown = true;
	pthread_cond_broadcast(&(pool->notify));
	pthread_mutex_unlock(&(pool->lock));

	// join all threads waiting for them to compplete
	for (int i = 0; i < pool->thread_count; i++) {
		if(pthread_join(pool->threads[i], NULL) != 0) {
			fprintf(stderr, "failed to joinn thread %d\n", i);
		}
	}


	

	
	// clean up resources
	if(pthread_mutex_destroy(&(pool->lock)) != 0) {
		fprintf(stderr, "mutex failed to destoy\n");
	}
	if (pthread_mutex_destroy(&(pool->completed_tasks_lock)) != 0) {
		fprintf(stderr, "mutex failed to destory\n");
	}
	if (pthread_cond_destroy(&(pool->notify)) != 0) {
		fprintf(stderr, "cond vrible failed to signal\n");
	}
	// free thread array
	free(pool->threads);

	// clean up remaining tassks in the queue
	Task *task;
	while (queue_dequeue((&pool->task_queue), (void *)&task) == ERROR_NONE) {
		free(task);
	}

	// clean up resources
	ErrorDetails err = {ERROR_NONE, ""};
	queue_destroy(&(pool->task_queue), &err);
	if (err.error_code != ERROR_NONE) {
		fprintf(stderr, "failed to destroy task queue: %s\n", err.message);
	}

	// desired state
	// all threads stopped
	// all resources freeed
	// thread pool ready for deallocation
}

void thread_pool_get_completed_tasks(ThreadPool *pool, int *completed) {
	pthread_mutex_lock(&(pool->completed_tasks_lock));
	*completed = pool->completed_tasks;
	pthread_mutex_unlock(&(pool->completed_tasks_lock));
}