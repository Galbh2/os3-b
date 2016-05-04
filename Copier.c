#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Copier.h"
#include "BoundedBuffer.h"

#define READ_BUFF_SIZE 4096
#define STDIN_READ_BUFF_SIZE 80
#define MAX_PATH_LENGTH 1024
#define COPY_BUFFER_SIZE 1024
#define FILE_QUEUE_SIZE 10

#define FILE_ACCESS_RW 0666
#define PATH_SEPARATOR "/"
#define PATH_SEPARATOR_CHAR '/'

typedef struct {
	BoundedBuffer *buff;
	char *dest;
} CopierData;

typedef struct {
	BoundedBuffer *buff;
	char *pipe;
} ListenerData;

/*
 * Listener thread starting point.
 * Creates a named pipe (the name should be supplied by the main function) and waits for
 * a connection on it. Once a connection has been received, reads the data from it and
 * parses the file names out of the data buffer. Each file name is copied to a new string
 * and then enqueued to the files queue.
 * If the enqueue operation fails (returns 0), the copier is application to exit and therefore
 * the Listener thread should stop. Before stopping, it should remove the pipe file and
 * free the memory of the filename it failed to enqueue.
 */
void *run_listener(void *param) {
	ListenerData *data;
	BoundedBuffer *buff;
	int fd, num, len, start, status;
	char filename[READ_BUFF_SIZE];
	char *fn_ptr;

	// Get the pointer to the parameters struct
	data = (ListenerData*)param;

	// Create the pipe
	mknod(data->pipe, S_IFIFO | FILE_ACCESS_RW, 0);

	while (1) {
		// Wait for a connection on the pipe
		fd = open(data->pipe, O_RDONLY);

		// Read data from pipe
		while ((num = read(fd, filename, READ_BUFF_SIZE)) > 0) {
			// Parse read data into file names, enqueue them, etc...
			// TODO: Complete this part
			// TODO: Make sure the thread exits at some point
		}
		close(fd);
	}
}

/*
 * Returns the name of a given file without the directory path.
 */
char *get_file_name(char *path) {
	int len;
	int i;

	len = strlen(path);

	for (i = len - 1; i >= 0; i--) {
		if (path[i] == PATH_SEPARATOR_CHAR) {
			return &(path[i + 1]);
		}
	}
	return path;
}

/*
 * A file copy function. Copies file from src to dest using a buffer.
 */
void copy_file(char *src, char *dest) {

}

/*
 * Copier thread starting point.
 * The copier reads file names from the files queue, one by one, and copies them to the
 * destination directory as given to the main function. Then it should free the memory of
 * the dequeued file name string (it was allocated by the Listener thread).
 * If the dequeue operation fails (returns NULL), it means that the application is trying
 * to exit and therefore the thread should simply terminate.
 */
void *run_copier(void *param) {

}


void printBuffer(BoundedBuffer* b) {

	if (b->size > 0) {
		int i;
		for (i = 0; i < b->capacity; i++) {
			printf("%s\n", (b->buffer)[i]);
		}
	}
}

/*
 * Main function.
 * Reads command line arguments in the format:
 * 		./Copier pipe_name destination_dir
 * Where pipe_name is the name of FIFO pipe that the Listener should create and
 * destination_dir is the path to the destination directory that the copier should
 * copy files into.
 * This function should create the files queue and prepare the parameters to the Listener and
 * Copier threads. Then, it should create these threads.
 * After threads are created, this function should control them as follows:
 * it should read input from user, and if the input line is EXIT_CMD (defined above), it should
 * set the files queue as "finished". This should make the threads terminate (possibly only
 * when the next connection is received).
 * At the end the function should join the threads and exit.
 */
int main(int argc, char *argv[]) {

	BoundedBuffer boundedBuffer;
	BoundedBuffer* p_boundedBuffer = &boundedBuffer;
	int capacity = 2;

	bounded_buffer_init(p_boundedBuffer, capacity);

	char* s1 = "hello";
	char* s2 = "world";
	char* s3 = "bye";


	bounded_buffer_enqueue(p_boundedBuffer, s1);
	bounded_buffer_enqueue(p_boundedBuffer, s2);

	printBuffer(p_boundedBuffer);

	printf("\n\n");

	bounded_buffer_dequeue(p_boundedBuffer);

	bounded_buffer_enqueue(p_boundedBuffer, s3);

	printBuffer(p_boundedBuffer);

	printf("\n\n");




	return 0;
}
