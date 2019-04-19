all: test

test: test.cpp blocking_queue.h
	g++ -g -O3 $< -o $@ -pthread -std=c++11 -march=native