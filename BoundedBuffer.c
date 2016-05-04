#include "BoundedBuffer.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Initializes the buffer with the specified capacity.
 * This function should allocate the buffer, initialize its properties
 * and also initialize its mutex and condition variables.
 * It should set its finished flag to 0.
 */
void bounded_buffer_init(BoundedBuffer *buff, int capacity) {

	buff->size = 0;
	buff->finished = 0;
	buff->head = 0;
	buff->tail = 0;

	buff->capacity = capacity;

	buff->buffer = (char**) malloc(sizeof(char**) * capacity);
	if(buff->buffer == 0){
		perror("memory allocation error");
	} else {
		pthread_mutex_init(&(buff->mutex), NULL);
		pthread_cond_init(&(buff->q_empty), NULL);
		pthread_cond_init(&(buff->q_full), NULL);
	}
}

/*
 * Enqueue a string (char pointer) to the buffer.
 * This function should add an element to the buffer. If the buffer is full,
 * it should wait until it is not full, or until it has finished.
 * If the buffer has finished (either after waiting or even before), it should
 * simply return 0.
 * If the enqueue operation was successful, it should return 1. In this case it
 * should also signal that the buffer is not empty.
 * This function should be synchronized on the buffer's mutex!
 */
int bounded_buffer_enqueue(BoundedBuffer *buff, char *data) {

	// Check if the buffer has finished
	if (buff->finished) { return 0;	}

	// Begin critical section
	pthread_mutex_lock(&(buff->mutex));

	if (buff->finished) { return 0; }

	// If the buffer is full - wait until it isn't full

	// if (buff->size == buff->capacity) { //it's better to use a while loop
	// see the doc about conditions to know the reason

	while (buff->size == buff->capacity) {

		// if (buff->finished) { return 0; } we don't need this line
		// because we are in the critical section (the lock is ours)

		pthread_cond_wait(&(buff->q_empty), &(buff->mutex));
		if (buff->finished) { return 0; }
	}

	// Enqueues the given string
	int pos = buff->tail;
	buff->buffer[pos] = data;
	buff->tail = (buff->tail + 1) % buff->capacity; // don't forget modulo :)
	buff->size++;

	// Signal for any thread waiting for a value
	pthread_cond_signal(&(buff->q_full));

	// Exit critical section
	pthread_mutex_unlock(&(buff->mutex));

	return 1;
}

/*
 * Dequeues a string (char pointer) from the buffer.
 * This function should remove the head element of the buffer and return it.
 * If the buffer is empty, it should wait until it is not empty, or until it has finished.
 * If the buffer has finished (either after waiting or even before), it should
 * simply return NULL.
 * If the dequeue operation was successful, it should signal that the buffer is not full.
 * This function should be synchronized on the buffer's mutex!
 */
char *bounded_buffer_dequeue(BoundedBuffer *buff) {

	char* headElement = NULL;

	// Check if the buffer has finished
	if (buff->finished) { return 0;	}

	// Begin critical section
	pthread_mutex_lock(&(buff->mutex));

	if (buff->finished) { return 0; }

	// If the buffer is empty - wait until it isn't empty

	//if (!buff->size) {
	while (!buff->size) {

		if (buff->finished) { return 0; }
		pthread_cond_wait(&(buff->q_full), &(buff->mutex));
		if (buff->finished) { return 0; }
	}

	// Dequeues the head element
	int pos = buff->head;
	headElement = buff->buffer[pos];
	buff->head = (buff->head  + 1) % buff->capacity;
	buff->size--;

	// Signal for any thread waiting for a value
	pthread_cond_signal(&(buff->q_empty));

	// Exit critical section
	pthread_mutex_unlock(&(buff->mutex));

	return headElement;
}

/*
 * Sets the buffer as finished.
 * This function sets the finished flag to 1 and then wakes up all threads that are
 * waiting on the condition variables of this buffer.
 * This function should be synchronized on the buffer's mutex!
 */
void bounded_buffer_finish(BoundedBuffer *buff) {

	// Begin critical section
	pthread_mutex_lock(&(buff->mutex));

	buff->finished = 1;

	pthread_cond_signal(&(buff->q_empty));
	pthread_cond_signal(&(buff->q_full));

	// Exit critical section
	pthread_mutex_unlock(&(buff->mutex));
}

/*
 * Frees the buffer memory and destroys mutex and condition variables.
 */
void bounded_buffer_destroy(BoundedBuffer *buff) {

	// Frees the buffer memory
	free(buff->buffer);

	// Destroys mutex
	pthread_mutex_destroy(&(buff->mutex));

	// Destroys conditions variables
	pthread_cond_destroy(&(buff->q_full));
	pthread_cond_destroy(&(buff->q_empty));
}
