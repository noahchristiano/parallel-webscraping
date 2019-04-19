#include <atomic>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <string>
#include <unistd.h>
#include "blocking_queue.h"

int NUM_THREADS = 1;

typedef struct task_args {
	int tid;
	BlockingQueue *q;
} task_args;

void * task(void * args) {
	struct task_args * a = (task_args *) args;
	/*
	for(int i = 0; i < NUM_THREADS-a->tid; i++) {
		a->q->push(a->tid);
		printf("push %d\n", a->tid);
	}
	*/
	char * s = (char *) malloc(sizeof(char)*11);
	sprintf(s, "%d", a->tid);
	a->q->push(s);
	printf("push %s\n", s);
	pthread_exit(NULL);
}

bool threaded(BlockingQueue *q) {
	pthread_t thread[NUM_THREADS];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	int rc;
	for(int i = 0; i < NUM_THREADS; i++) {
		struct task_args * a = (struct task_args *) malloc(sizeof(struct task_args));
		a->tid = i;
		a->q = q;

		rc = pthread_create(&thread[i], &attr, task, (void *) a);
		if(rc) {
			printf("error: pthread_create %d\n", rc);
			exit(1);
		}
	}

	//usleep(500000);
	q->dump();

	for(int i = 0; i < NUM_THREADS; i++) {
		rc = pthread_join(thread[i], NULL);
		if(rc) {
			printf("error: pthread_join %d\n", rc);
			exit(1);
		}

		char *k;
		for(int j = 0; j < 2; j++) {
			if(q->pop(&k)) {
				printf("pop %s\n", k);
			}
			else {
				printf("nothing to pop\n");
			}
		}
	}

	q->dump();
}

int main() {
	NUM_THREADS = 10;
	BlockingQueue q;

	threaded(&q);
	return 0;
}
