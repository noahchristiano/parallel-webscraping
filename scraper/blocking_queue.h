#include <cstring>
#include <mutex>
#include <queue>
#include <stdlib.h>
#include <string>

class BlockingQueue {
private:
	std::mutex m;
	std::queue<std::string> q;

public:
	void dump() {
		m.lock();
		std::string s;
		for (int m = q.size(); m > 0; m--) {
			s = q.front();
			printf("%s\n", s.c_str());
			q.pop();
			q.push(s);
		}
		m.unlock();
	}

	bool empty() {
		m.lock();
		bool e = q.empty();
		m.unlock();
		return e;
	}

	int size() {
		m.lock();
		unsigned int s = q.size();
		m.unlock();
		return s;
	}

	int push(char *item) {
		m.lock();

		std::string s(item);
		q.push(s);

		m.unlock();
		return 1;
	}

	int pop(char **item) {
		m.lock();
		if (q.size() == 0) {
			m.unlock();
			return 0;
		}

		int len = q.front().size();
		char *t = (char *) malloc(sizeof(char)*(len+1));
		memcpy(t, q.front().c_str(), len);
		t[len] = '\0';
		*item = t;
		q.pop();
		
		m.unlock();
		return 1;
	}

	// for testing
	std::string back() {
		m.lock();
		std::string s = (char *) q.back().c_str();
		m.unlock();
		return s;
	}
};
