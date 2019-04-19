#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#define URL_SIZE 256 // should be 2000 characters, but that's lots of memory

class Input {
public:
	int size;
	char **url;

	// inspired by Professor Pai's simplegraph.h
	int load_file(const char *f) {
		FILE *fp;

		fp = fopen(f, "r");
		if (fp == NULL) {
			fprintf(stderr, "error: could not open file %s\n", f);
			return 0;
		}

		if (fscanf(fp, "%d", &size) == EOF) {
			fprintf(stderr, "error: couldn't read num_urls\n");
			fclose(fp);
			return 0;
		}

		url = (char **) calloc(size, sizeof(char *));

		char *t_url;
		int rd;
		for (unsigned int i = 0; i < size; i++) {
			t_url = (char *) malloc(sizeof(char)*URL_SIZE);
			rd = fscanf(fp, "%s", t_url);

			if (rd == EOF || rd < 1) {
				fprintf(stderr, "error: %d couldn't read url %d\n", rd, i);
				fclose(fp);
				return 0;
			}

			int len = strlen(t_url) + 1;
			url[i] = (char *) malloc(sizeof(char)*len);
			memcpy(url[i], t_url, len);
			url[i][len-1] = '\0';
		}

		fclose(fp);
		return 1;
	}

	void dump() {
		for (int i = 0; i < size; i++) {
			printf("%s\n", url[i]);
		}
	}
};
