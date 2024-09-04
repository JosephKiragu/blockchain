#ifndef CONCURRENT_QUEUE_H
#define CONCURRENT_QUEUE_H

#define NUM_THREADS 4
#define ITEMS_PER_THREAD 1000

#include <pthread.h>
#include <stdbool.h>
#include "../include/project.h"



typedef struct QueueNode {
	void* data;
	struct QueueNode* next;
} QueueNode;

typedef struct {
	QueueNode* head;
	QueueNode* tail;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int size;
} ConcurrentQueue;


// concurrent queue functions

ErrorCode queue_init(ConcurrentQueue *queue, ErrorDetails *err);
ErrorCode queue_enqueue(ConcurrentQueue *queue, void* data, ErrorDetails *err);
ErrorCode queue_dequeue(ConcurrentQueue *queue, void** data);
bool queue_is_empty(ConcurrentQueue* queue, ErrorDetails* err);
void queue_destroy(ConcurrentQueue* queue, ErrorDetails* err);




#endif