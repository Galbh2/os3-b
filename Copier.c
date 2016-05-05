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
	char* path;

	//TODO check what are the unused variables

	// Get the pointer to the parameters struct
	data = (ListenerData*)param;

	// Get the pointer to the BoundedBuffer

	buff = data->buff;

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

			char* token;
			token = strtok(filename, "\n");

			while (token != NULL) {

				//memory allocation for the new path
				path = (char*) malloc(sizeof(char) * strlen(token));
				strcpy(path, token);
				token = strtok(NULL, "\n");
				printf("%s", token);

				// checks if the program wants to exit
				// free memory, close files and remove the pipe
				if (!bounded_buffer_enqueue(buff, path)) {
					free(path);
					close(fd);
					remove(data->pipe);
					return NULL;
					pthread_exit(NULL);
				}

			}
		}
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

	//open files and check for errors
	FILE* srcFile = fopen(src, "rb");
	FILE* destFile;

	if (!srcFile) {
		perror("error while opening files...");
		return;
	} else {
		destFile = fopen(dest, "wb");
		 if (!destFile) {
			 perror("can't write to dest...");
			 return;
		 }
	}

	// memory allocation for the buffer
	unsigned char* buffer = (unsigned char*) malloc(COPY_BUFFER_SIZE);

	if (!buffer) {
		perror("error while allocating memory for buffer");
		return;
	}

	// read and write the file
	while (!feof(srcFile)) {
		fread(buffer, COPY_BUFFER_SIZE, 1, srcFile);
		fwrite(buffer, COPY_BUFFER_SIZE, 1, destFile);
	}


	// closing the files and release memory
	fclose(srcFile);
	fclose(destFile);
	free(buffer);
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

	// gets the thread data

	CopierData* copierData = (CopierData*) param;
	BoundedBuffer* buff = (BoundedBuffer*) copierData->buff;
	char* src;

	// checks if the program wants to exit
	while ( (src = bounded_buffer_dequeue(buff)) != NULL) {
		//TODO check for error while malloc
		char* name = get_file_name(src);
		char* dest = (char*) malloc(sizeof(char) * strlen(copierData->dest) + strlen(name));
		strcpy(dest, copierData->dest);
		strcat(dest, name);

		copy_file(src, dest);

		free(dest);
	}

	pthread_exit(NULL);
}

/**
 * Helper function for printing a queue
 */
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

	//TODO add command line arguments

	pthread_t listener, copier;

	// creates the buffer
	int capacity = 2;

	// init data for the threads
	BoundedBuffer buff;
	BoundedBuffer* p_boundedBuffer = &buff;
	bounded_buffer_init(p_boundedBuffer, capacity);

	ListenerData listenerDate;
	listenerDate.buff = p_boundedBuffer;
	listenerDate.pipe = "./the_pipe";

	CopierData copierData;
	copierData.buff = p_boundedBuffer;
	copierData.dest = "./copies/";

	// runs the threads
	pthread_create(&listener, NULL, run_listener, (void*)(&listenerDate));
	pthread_create(&copier, NULL, run_copier, (void*)(&copierData));

	// Join threads
	pthread_join(listener, NULL);
	pthread_join(copier, NULL);

	// Free allocated resources
	bounded_buffer_destroy(p_boundedBuffer);

	return 0;
}
