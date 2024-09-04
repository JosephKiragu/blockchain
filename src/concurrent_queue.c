#include "../include/concurrent_queue.h"

ErrorCode queue_init(ConcurrentQueue *queue, ErrorDetails *err) {
	// initial state: queue empty
	
	// transformation: initialize queue
	queue->head = queue->tail = NULL;
	queue->size = 0;

	if (pthread_mutex_init(&queue->mutex, NULL) != 0) {
		set_error(err, ERROR_IMPL_ERROR, "failed to iinitialize mutex");
		return ERROR_IMPL_ERROR;
	}

	if (pthread_cond_init(&queue->cond, NULL) != 0) {
		pthread_mutex_destroy(&queue->mutex);
		set_error(err, ERROR_IMPL_ERROR, "failed to initialize condition vaariable for mutex");
		return ERROR_IMPL_ERROR;
	}

	return ERROR_NONE;
	
}

ErrorCode queue_enqueue(ConcurrentQueue *queue, void* data, ErrorDetails *err) {
	if (!queue || !data || !err) {
		printf("null ptr detected in queue init\n");
		return ERROR_NULL_PTR;
	}

	QueueNode* new_node = (QueueNode*)allocate_memory(sizeof(QueueNode), err);
	if (err->error_code != ERROR_NONE) return err->error_code;

	new_node->data = data;
	new_node->next = NULL;

	// lock mutext to ensure exclusive access to the hred resourcess
	if (pthread_mutex_lock(&queue->mutex) != 0) {
		printf("mutex failied to lock in queue init\n");
		return ERROR_IMPL_ERROR;
	}

	if (queue->tail == NULL) {
		queue->head = queue->tail = new_node;
	} else {
		queue->tail->next = new_node;
		queue->tail = new_node;
	}

	queue->size++;

	if (pthread_cond_signal(&queue->cond) != 0) {
		printf("mutex condition failed to initializ iniside enqueue\n");
		pthread_mutex_unlock(&queue->mutex);
		return ERROR_IMPL_ERROR;
		}

	if (pthread_mutex_unlock(&queue->mutex) != 0) {
	 	printf("mutex failed to unlock inside enqueue\n");
		return ERROR_IMPL_ERROR;
	}
	return ERROR_NONE;
}


ErrorCode queue_dequeue(ConcurrentQueue *queue, void** data) {
	//initial state:
	// queue exists with potential ekements
	// mutex is unlocked
	// data pointer is unaasigned

	// transformation
	// lock the mutex
	if (!queue || !data) {
		printf("invalid data input for queue dequeue\n");
		return ERROR_INVALID_ARGUMENT;
	}
	if (pthread_mutex_lock(&queue->mutex) != 0) {
		printf("mutex failed to lock\n");
		return ERROR_IMPL_ERROR;
	}

	// wait for an element if the queue iss empty
	while (queue->head == NULL) {
		if (pthread_cond_wait(&queue->cond, &queue->mutex) != 0) {
			printf("mutex conditioon failed to initialize\n");
			pthread_mutex_unlock(&queue->mutex);
			return ERROR_IMPL_ERROR;
		}
	}
	// remove the node element
	QueueNode* node = queue->head;
	*data = node->data;
	queue->head = node->next;

	if (queue->head == NULL) {
		queue->tail = NULL;
	}

	// update the tail if necessary
	queue->size--;

	// unlock the mutex
	if (pthread_mutex_unlock(&queue->mutex) != 0) {
		printf("mutext failed to unlock");
		return ERROR_IMPL_ERROR;
	}

	// free the dequeued node
	free(node);

	// dessired state:
	// first element removed fronm the queue;
	// data pointer aigned to the dequeued element
	// queue size is deccreased
	// mutex unlcked
	// memory of dequeued node freed
	return ERROR_NONE;

}

bool queue_is_empty(ConcurrentQueue* queue, ErrorDetails* err) {
	// innitial state
	// queue ecists
	// mutex is unliocked
	// empty statuss uknowne
	bool is_empty = true;

	// transformation: locl the mutex
	if (pthread_mutex_lock(&queue->mutex) != 0) {
		set_error(err, ERROR_IMPL_ERROR, "mutex failed to lock insie queue is empty");
		printf("mutex failed to lock insie queue is empty\n");
		return false; // return false since we cant determine the empty status
	}

	// check if quueue is empty
	is_empty = (queue->head == NULL);

	// unlock the mutex
	if (pthread_mutex_unlock(&queue->mutex) != 0) {
		set_error(err, ERROR_IMPL_ERROR, "mutex failed to lock inside is queue empty");
		return false; 
	}

	// dsrired state:
	// empty status determined
	// mutext unlocked
	return is_empty;

}

void queue_destroy(ConcurrentQueue* queue, ErrorDetails* err) {
	// initial state:
	// queue exists with potential elements
	// mutex and condition variables set

	// transformation: lock the mutex
	if (pthread_mutex_lock(&queue->mutex) != 0) {
		set_error(err, ERROR_IMPL_ERROR, "mutex failed to lock inside queue destroy");
		return;
	}

	// free all modes in the queue
	while (queue->head != NULL) {
		QueueNode* node = queue->head;
		queue->head = node->next;
		free(node->data);
		free(node);
	}

	// unlock the mutex
	if(pthread_mutex_unlock(&queue->mutex) != 0) {
		set_error(err, ERROR_IMPL_ERROR,"mutex failed to unlock");
		return;
	}
	// destroy the muitex and condition variable
	if(pthread_mutex_destroy(&queue->mutex) != 0) {
		set_error(err, ERROR_IMPL_ERROR, "mutext failed to be destroyed");
		return;
	}
	if(pthread_cond_destroy(&queue->cond) != 0) {
		set_error(err, ERROR_IMPL_ERROR, "mutex condition failed to be destrooyed");
		return;
	}

	// desired state
	// all nodes freed
	// mutex and condition variaable destroyed
	// queue structure ready for deallocation (doesnt happen here)
}
