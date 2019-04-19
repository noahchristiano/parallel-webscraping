#include <atomic>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>

class LockFreeQueue {
struct node_t;

typedef struct pointer_t {
	node_t * ptr;
	unsigned int count;

	pointer_t() noexcept : ptr{nullptr}, count{0} {};
	pointer_t(node_t *ptr) : ptr{ptr}, count{0} {};
	pointer_t(node_t *ptr, unsigned int count) : ptr{ptr}, count{count} {}

	bool operator ==(const pointer_t & other) const {
		return ptr == other.ptr && count == other.count;
	}

	bool operator !=(const pointer_t & other) const {
		return ptr != other.ptr || count != other.count;
	}
} pointer_t;

typedef struct node_t {
	char *val;
	std::atomic<pointer_t> next;

	node_t() : next{pointer_t{}} {}
	node_t(char *val) : val(val), next{pointer_t{}} {}
} node_t;

std::atomic<pointer_t> * head;
std::atomic<pointer_t> * tail;

public:
	LockFreeQueue() {
		node_t * node = (node_t *) malloc(sizeof(node_t));
		head = (std::atomic<pointer_t> *) malloc(sizeof(std::atomic<pointer_t>));
		tail = (std::atomic<pointer_t> *) malloc(sizeof(std::atomic<pointer_t>));
		if(!node || !head || !tail) {
			printf("malloc fail\n");
			exit(1);
		}
		atomic_store(head, pointer_t{node, 0});
		atomic_store(tail, pointer_t{node, 0});
	}

	void dump() {
		if(empty()) {
			printf("empty\n");
			return;
		}

		pointer_t step = atomic_load(head).ptr->next;
		printf("dump: ");
		while(step.ptr != atomic_load(tail).ptr) {
			printf("%s ", step.ptr->val);
			step = atomic_load(&step.ptr->next);
		}

		printf("%s\n", step.ptr->val);
	}

	bool empty() {
		return atomic_load(head).ptr == atomic_load(tail).ptr;
	}

	int push(char *item) {
		node_t * node = (node_t *) malloc(sizeof(node_t));
		if(!node) {
			printf("malloc fail\n");
			exit(1);
		}

		int len = strlen(item);
		node->val = (char *) malloc(sizeof(char)*(len+1));
		memcpy(node->val, item, len);
		node->val[len] = '\0';
		pointer_t t, n;

		while(true) {
			t = atomic_load(tail);
			n = atomic_load(&t.ptr->next);

			if(t == atomic_load(tail)) {
				if(!n.ptr) {
					// try to add to end of list
					if(atomic_compare_exchange_weak(
						&t.ptr->next, &n, pointer_t{node, n.count+1})) {
						// printf("successful push %d\n", item);
						break;
					}
				}
				else {
					// swing to next node
					fflush(stdout);
					atomic_compare_exchange_weak(tail, &t, pointer_t{n.ptr, t.count + 1});
				}
			}
		}
		// swing tail to inserted node
		atomic_compare_exchange_weak(tail, &t, pointer_t{node, t.count + 1});
		return 1;
	}

	int pop(char **item) {
		pointer_t h, t, n;

		while(true) {
			h = atomic_load(head);
			t = atomic_load(tail);
			n = atomic_load(&h.ptr->next);

			if(h == atomic_load(head)) {
				if(h.ptr == t.ptr) { // is queue empty or tail behind?
					if(!n.ptr) { // is queue empty?
						return 0;
					}
					// tail is behind, advance
					atomic_compare_exchange_weak(tail, &t, pointer_t{n.ptr, t.count + 1});
				}
				else {
					int len = strlen(n.ptr->val);
					char *t = (char *) malloc(sizeof(char)*(len+1));
					memcpy(t, n.ptr->val, len);
					t[len] = '\0';
					*item = t;

					// try to swing head to next node
					if(atomic_compare_exchange_weak(head, &h, pointer_t{n.ptr, h.count + 1})) {
						// printf("successful pop %d\n", item);
						break;
					}
				}
			}
		}
		free(h.ptr);
		return 1;
	}
};
