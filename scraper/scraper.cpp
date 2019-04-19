/* -*- mode: c++ -*- */

#include <atomic>
#include <ctime>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <unistd.h>

#include "input.h"
#include "blocking_queue.h"
// #include "lockfree_queue.h"
#include "shared_map.h"
#include "parser.h"
#include "request.h"
#include "Timer.h"

int NUM_THREADS = 1;
int port;
BlockingQueue q;
SharedMap m;
ggc::Timer t("scrape");

typedef struct scrape_args {
	int tid;
	SharedMap *m;
	BlockingQueue *q;
	std::atomic_int *working;
} scrape_args;

void * scrape_task(void * args) {
	scrape_args *a = (scrape_args *) args;
	Request r;
	Parser p;

	char *url;
	while(!a->q->empty() || atomic_load(a->working) > 0) {
		if (a->q->pop(&url)) {
			// printf("%d %d\n", a->q->size(), atomic_load(a->working));
			atomic_fetch_add(a->working, 1);
			webpage w;
			r.get(url, &w);

			int size;
			char **urls = p.get_urls(w.memory, url, &size);

			for (int i = 0; i < size; i++) {
				if (urls[i] != NULL && a->m->check_and_add(urls[i])) {
					// printf("%s\n", urls[i]);
					a->q->push(urls[i]);
				}
			}
			atomic_fetch_sub(a->working, 1);
		}
	}

	pthread_exit(NULL);
}

int scrape(BlockingQueue *q, SharedMap *m) {
	pthread_t thread[NUM_THREADS];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	std::atomic_int working; // threads in progress
	atomic_store(&working, 0);

	int rc;

	// worker threads
	for(int i = 0; i < NUM_THREADS; i++) {
		scrape_args * args = (scrape_args *) malloc(sizeof(scrape_args));
		args->tid = i;
		args->m = m;
		args->q = q;
		args->working = &working;

		rc = pthread_create(&thread[i], &attr, scrape_task, (void *) args);
		if(rc) {
			printf("error: pthread_create %d\n", rc);
			exit(1);
		}
	}

	for(int i = 0; i < NUM_THREADS; i++) {
		rc = pthread_join(thread[i], NULL);
		if(rc) {
			printf("error: pthread_join %d\n", rc);
			exit(1);
		}
	}

	return 0;
}

void intHandler(int sig) {
	t.stop();
	printf("Total time: %llu ms\n", t.duration_ms());
	printf("Links scraped: %d\n", m.size());
	exit(1);
}

// run with ./scrape threads url_file
int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "error, usage: %s threads url_file\n", argv[0]);
		exit(1);
	}

	signal(SIGINT, intHandler);

	if (argc == 3) {
		Input input;

		if (!input.load_file(argv[2])) {
			fprintf(stderr, "error: load_file\n");
			exit(1);
		}
		printf("Loaded '%s', %d urls\n", argv[2], input.size);
		
		for (int i = 0; i < input.size; i++) {
			if (m.check_and_add(input.url[i])) {
				q.push(input.url[i]);
			}
		}
	}

	/*
	port = std::stoi(argv[1]);
	if (port < 0 || port > 65535) {
		fprintf(stderr, "port out of range %d\n", port);
		exit(1);
	}
	*/

	NUM_THREADS = std::stoi(argv[1]);
	NUM_THREADS = (NUM_THREADS > 0) ? NUM_THREADS : 1;

	printf("please ignore erroneous output from libxml...\n");

	t.start();
	scrape(&q, &m);
	t.stop();

	printf("Total time: %llu ms\n", t.duration_ms());
	printf("Links scraped: %d\n", m.size());
	return 0;
}
