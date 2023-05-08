#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>

#define BUFFER_SIZE 5
#define TEST_RUNS 10

int main() {
	int* shared_mem;
	int* shared_buff;
	int* front;
	int* rear;

	if ((shared_mem = mmap(NULL, BUFFER_SIZE + 2, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED) {
		perror("Allocating shared memeory error");
		exit(1);
	}

	front = shared_mem + 0;
	rear = shared_mem + 1;
	shared_buff = shared_mem + 2;

	*front = 0;
	*rear = 0;

	int child_pid;
	child_pid = fork();
	if (child_pid < 0) {
		perror("Fork failed");
		exit(2);
	}

	if (child_pid == 0) {
		/* Producer */
		int write_val;
		for (int i = 0 ; i < TEST_RUNS ; i++) {
			while ( (*rear + 1)%BUFFER_SIZE == *front ); // Wait if buffer is full
			write_val = (i+1)*(i+1);
			printf("%d  \t  -> buffer[%d]\n", write_val, *rear);
			*(shared_buff + *rear) = write_val;
			*rear = (*rear+1)%BUFFER_SIZE;
		}
	} else {
		/* Consumer */
		for (int i = 0 ; i < TEST_RUNS ; i++) {
			while (*rear == *front);					// Wait if buffer is empty
			printf("buffer[%d] -> %d\n", *front, *(shared_buff + *front));
			*front = (*front+1)%BUFFER_SIZE;
		}
	}
}