all: scraper

scraper: scraper.cpp shared_map.h parser.h request.h input.h blocking_queue.h Timer.h
	g++ -g -O3 $< -o $@ -pthread -std=c++11 -lcurl -lxml2 -I /usr/include/libxml2
	