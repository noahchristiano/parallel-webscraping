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

		char *t = (char *) q.front().c_str();
		*item = t;
		q.pop();
		
		m.unlock();
		return 1;
	}
};
