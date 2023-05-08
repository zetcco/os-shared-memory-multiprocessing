#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/mman.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define TEST_RUNS 10

/* Semaphore related */
#define SEM_FULL 0
#define SEM_EMPTY 1

void allocate_shared_memory(void **shared_buff, void **front, void **rear);
int get_semaphores(int count);
void init_semaphore(int semid, int semindex, int semval);
int get_semaphore(int semid, int semindex);
void wait(int semid, int semindex);
void signal(int semid, int semindex);

int main() {
	int* shared_mem;
	int* shared_buff;
	int* front;
	int* rear;

	if ((shared_mem = mmap(NULL, sizeof(int)*(BUFFER_SIZE + 2), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED) {
		perror("Allocating shared memeory error");
		exit(1);
	}

	front = shared_mem + 0;
	rear = shared_mem + 1;
	shared_buff = shared_mem + 2;

	*front = 0;
	*rear = 0;

	/* Initialize semaphores */
	int sem_set = get_semaphores(2);
	init_semaphore(sem_set, SEM_EMPTY, BUFFER_SIZE);
	init_semaphore(sem_set, SEM_FULL, 0);

	printf("FULL %d, EMPTY: %d\n", get_semaphore(sem_set, SEM_FULL), get_semaphore(sem_set, SEM_EMPTY));

	/* Start producer/consumer */
	if (fork() == 0) {
		/* Child process */
		int write_val = 0;
		for(int i = 0; i < TEST_RUNS; i++) {
			wait(sem_set, SEM_EMPTY);
			write_val = (i+1)*(i+1);
			printf("%d  \t  -> buffer[%d]\n", write_val, *rear);
			*(shared_buff + *rear) = write_val;
			*rear = (*rear+1)%BUFFER_SIZE;
			//printf("Value of rear: %d\n", *rear);
			signal(sem_set, SEM_FULL);
			//printf("[c] FULL %d, EMPTY: %d\n", get_semaphore(sem_set, SEM_FULL), get_semaphore(sem_set, SEM_EMPTY));
			//if (i%BUFFER_SIZE == 0) sleep(10);
		}
	} else {
		/* Parent process */
		for(int i = 0; i < TEST_RUNS; i++) {
			wait(sem_set, SEM_FULL);
			printf("buffer[%d] -> %d\n", *front, *(shared_buff + *front));
			*front = (*front+1)%BUFFER_SIZE;
			//printf("Value of front: %d\n", *front);
			signal(sem_set, SEM_EMPTY);
			//printf("[p] FULL %d, EMPTY: %d\n", get_semaphore(sem_set, SEM_FULL), get_semaphore(sem_set, SEM_EMPTY));
		}
	}
}

void allocate_shared_memory(void **shared_buff, void **front, void **rear) {
	void *shared_mem;
	if ((shared_mem = mmap(NULL, (BUFFER_SIZE + 2)*sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0)) == MAP_FAILED) {
		perror("Allocating shared memory error");
		exit(1);
	}

	*shared_buff = shared_mem;
	*front = shared_mem + BUFFER_SIZE;
	*rear = shared_mem + BUFFER_SIZE + 1;
}

int get_semaphores(int count) {
	int semaphore_set = semget(IPC_PRIVATE, count, IPC_CREAT | 0600);
	if (semaphore_set == -1) {
		perror("Error allocating semaphore");
		exit(1);
	}
	return semaphore_set;
}

void init_semaphore(int semid, int semindex, int semval) {
	if (semctl(semid, semindex, SETVAL, semval) == -1) {
		perror("Initializing semaphore value error");
		exit(1);
	}
}

int get_semaphore(int semid, int semindex) {
	int semval;
	if ((semval = semctl(semid, semindex, GETVAL)) == -1) {
		perror("Getting semaphore value error");
		exit(1);
	}
	return semval;
}

void wait(int semid, int semindex) {
	struct sembuf sops[1];
	sops[0].sem_num = semindex;
	sops[0].sem_op = -1;
	sops[0].sem_flg = 0;
	if (semop(semid, sops, 1) == -1) {
		perror("Error waiting on semaphore");
		exit(1);
	}
}

void signal(int semid, int semindex) {
	struct sembuf sops[1];
	sops[0].sem_num = semindex;
	sops[0].sem_op = 1;
	sops[0].sem_flg = 0;
	if (semop(semid, sops, 1) == -1) {
		perror("Error waiting on semaphore");
		exit(1);
	}
}