#pragma once

#include <mutex>
#include <string>
#include <unordered_map>

// no progess, in progress, done scraping
enum status { NP, IP, DONE };

class SharedMap {
public:
	std::unordered_map<std::string, int> m;
	std::mutex l;

	int add(char *url) {
		l.lock();
		std::string str(url);
		m.insert({str, NP});
		l.unlock();
		return 0;
	}

	int update(char *url, enum status s) {
		l.lock();
		std::string str(url);
		m.insert({str, s});
		l.unlock();
		return 0;
	}

	int check_status(char *url) {
		l.lock();
		std::string str(url);
		int s = m.at(str);
		l.unlock();
		return s;
	}

	// 1 success, 0 fail
	int check_and_add(char *url) {
		l.lock();
		std::string str(url);
		std::unordered_map<std::string, int>::const_iterator i = m.find(str);
		if (i == m.end()) {
			m.insert({str, NP});
			l.unlock();
			return 1;
		}
		l.unlock();
		return 0;
	}

	int size() {
		l.lock();
		int s = m.size();
		l.unlock();
		return s;
	}
};
