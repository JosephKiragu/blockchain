#include "../include/project.h"

#include "../include/httpserver.h"
#include "../include/concurrent_queue.h"
#include "../include/thread_pool.h"

int completed_tasks = 0;
pthread_mutex_t completed_tasks_lock = PTHREAD_MUTEX_INITIALIZER;

void task_function(void *arg) {
	// initial state:
	// task not executed
	// argument contains task ID

	// transformation:
	int *task_id = (int *)arg;
	printf("executing task %d\n", *task_id);
	// simulate work
	usleep(10000);

	// report task completed
	printf("Task %d completed\n", *task_id);

	// cleaan up


	// desired state:
	// task executed
	// completed tasks count updated
	// task completeion reporoted
	// memory freed
}


int main() {
	// initial state:
	// no thread pool
	// no tasks executed

	// transformation
	ThreadPool pool;
	ErrorDetails err = { ERROR_NONE, ""};
	ErrorCode result;

	// create thread pool
	result = thread_pool_create(&pool, NUM_THREADS, &err); // rem to destroy
	if (result != ERROR_NONE) {
		handle_error(&err);
	}

	int *task_ids = allocate_memory(NUM_TASKS * sizeof(int), &err); // rem to free
		if (err.error_code != ERROR_NONE) {
			thread_pool_destroy(&pool);
			handle_error(&err);
		}

	printf("threads created successfully\n");

	// add tasks to the pool
	for (int i = 0; i < NUM_TASKS; i++) {
		
		task_ids[i] = i;

		result = thread_pool_add_task(&pool, task_function, &task_ids[i], &err);
		if (result != ERROR_NONE) {
			thread_pool_destroy(&pool);
			free(task_ids);
			handle_error(&err);
		}
		printf("task %d added to the pool\n",i);
	}

	// wait fot the tasks to complete
	printf("waiting fot the tasks to complete...\n");
	thread_pool_wait(&pool);

	int completed_tasks;
	if (pthread_mutex_lock(&(pool.completed_tasks_lock)) != 0) {
		fprintf(stderr, "failed to lock compleeted tasks lock\n");
		thread_pool_destroy(&(pool));
		return 1;
	}
	completed_tasks = pool.completed_tasks;
	pthread_mutex_unlock(&(pool.completed_tasks_lock));

	
	// verify all tasks completed successfully
	if (completed_tasks != NUM_TASKS) {
		printf("error: expected %d completed tasks but got %d\n", NUM_TASKS ,completed_tasks);
		thread_pool_destroy(&pool);
		return 1;
	}
	
	printf("all tasks completed successfully\n");

	// clean up
	// free(task_ids); 
	thread_pool_destroy(&pool);
	printf("thread pool destroyed\n");
	return 0;


	}



// z