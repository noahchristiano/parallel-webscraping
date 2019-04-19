all: test

test: test.cpp lockfree_queue.h
	g++ -g -O3 $< -o $@ -pthread -std=c++11 -march=native