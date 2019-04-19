#include <cstring>
#include <mutex>
#include <stdlib.h>

typedef struct node {
	node *next;
	char *val;

	bool operator ==(const node & other) const {
		return val == other.val;
	}

	bool operator !=(const node & other) const {
		return val != other.val;
	}
} node;

class BlockingQueue {
private:
	std::mutex m;
	unsigned int qsize;
	node *head;
	node *tail;
	int *items;

public:
	int initialize() {
		m.lock();
		qsize = 0;
		head = (node *) malloc(sizeof(node));
		head->val = (char *) malloc(sizeof(char)*1); // placeholder
		tail = head;
		m.unlock();
	}

	void dump() {
		m.lock();
		node *t = tail;

		while(t != head) {
			printf("%s\n", t->val);
			t = t->next;
		}
		m.unlock();
	}

	bool empty() {
		m.lock();
		bool e = head->next == NULL;
		m.unlock();
		return e;
	}

	int size() {
		m.lock();
		unsigned int s = qsize;
		m.unlock();
		return s;
	}

	int push(char *item) {
		m.lock();

		node *n = (node *) malloc(sizeof(node));
		int len = strlen(item);
		n->val = (char *) malloc(sizeof(char)*(len+1));
		memcpy(n->val, item, len);
		n->val[len] = '\0';

		tail->next = n;
		tail = n;
		
		qsize++;
		m.unlock();
		return 1;
	}

	int pop(char **item) {
		m.lock();
		if (head->next == NULL) {
			m.unlock();
			return 0;
		}
		
		node *n = head->next;

		int len = strlen(n->val);
		char *t_char = (char *) malloc(sizeof(char)*(len+1));
		memcpy(t_char, n->val, len);
		t_char[len] = '\0';
		*item = t_char;

		head->next = head->next->next;
		qsize--;
		m.unlock();
		return 1;
	}
};
