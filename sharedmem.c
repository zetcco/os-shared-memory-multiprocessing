#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>

#define BUFFER_SIZE 5

int main() {
	int* shared_buff;

	if ((shared_buff = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED) {
		perror("Allocating shared memeory error");
		exit(1);
	}

	int child_pid;

	child_pid = fork();
	if (child_pid < 0) {
		perror("Fork failed");
		exit(2);
	}

	if (child_pid == 0) {
		for (int i = 0 ; i < BUFFER_SIZE ; i++) {
			printf("Child writing %d to buffer[%d]\n", i*i, i);
			*(shared_buff + i) = (i+1)*(i+1);
			sleep(1);
		}
	} else {
		for (int i = 0 ; i < BUFFER_SIZE ; i++) {
			printf("Parent reading buffer[%d] -> %d\n", i, *(shared_buff + i));
			sleep(2);
		}
	}
}