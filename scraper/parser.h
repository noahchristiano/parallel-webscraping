#pragma once
#include <ctype.h>
#include <cstring>
#include <cstdio>

#include <libxml/tree.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

class Parser {
public:
	Parser() {
		xmlInitParser();
	}

	~Parser() {
		// causes kernel panic?
		// xmlCleanupParser();
	}

	char** get_urls(char *body, char *base, int *n) {
		int size = -1;
		char** urls = get_link_text(body, &size);
		normalize(urls, base, size);
		filter(urls, base, size);
		remove_trailing_slash(urls, base, size);
		*n = size;
		return urls;
	}

	// very basic parser
	// discarded links are null
	void normalize(char **urls, char *base, int size) {
		int b_size = strlen(base);

		for (int i = 0; i < size; i++) {
			int len = strlen(urls[i]);
			char *url;
			if (len > 0) {
				if (strncmp(urls[i], "http", 4) == 0) { // http...
					continue;
				}
				else if (urls[i][0] == '/') { // starts with /
					int mid = (base[b_size-1] == '/') ? b_size-1 : b_size; // extra /
					url = (char *) malloc(sizeof(char)*(mid+len+1));
					memcpy(url, base, mid);
					memcpy(&url[mid], urls[i], strlen(urls[i]));
					url[mid+len] = '\0';
					free(urls[i]);
					urls[i] = url;
					continue;
				}
				else if (isalnum(urls[i][0])) { // alphanumeric
					if (base[b_size-1] == '/') { // base has slash
						url = (char *) malloc(sizeof(char)*(b_size+len+1));
						memcpy(url, base, b_size);
						memcpy(&url[b_size], urls[i], strlen(urls[i]));
						url[b_size+len] = '\0';
					}
					else { // base doesn't have slash
						url = (char *) malloc(sizeof(char)*(b_size+len+2));
						memcpy(url, base, b_size);
						url[b_size] = '/';
						memcpy(&url[b_size+1], urls[i], strlen(urls[i]));
						url[b_size+1+len] = '\0';
					}
					free(urls[i]);
					urls[i] = url;
					continue;
				}
			}
			urls[i] = NULL;
		}
	}

	// simple method to get domain from URL
	char * get_domain(char * url) {
		char *t;
		t = strchr(url, '/');
		t = strchr(t+1, '/');
		int start = t-url+1;
		t = strchr(t+1, '/');
		int end = t-url;
		if (!t) {
			end = strlen(url);
		}
		char *domain = (char *) malloc(sizeof(char)*(end-start));
		memcpy(domain, &url[start], end-start+1);
		domain[end-start] = '\0';
		return domain;
	}

	// remove domains != base
	void filter(char **urls, char *base, int size) {
		char *base_domain = get_domain(base);

		for (int i = 0; i < size; i++) {
			if (!urls[i]) continue;

			char *domain = get_domain(urls[i]);
			if (strcmp(base_domain, get_domain(urls[i])) != 0) {
				urls[i] = NULL;
			}
		}
	}

	void remove_trailing_slash(char **urls, char *base, int size) {
		for (int i = 0; i < size; i++) {
			if (!urls[i]) continue;

			int len = strlen(urls[i]);
			if (urls[i][len-1] == '/') {
				urls[i][len-1] = '\0';
			}
		}
	}

	static void xmlGenericErrorFunc(void *ctx, const char *msg, ...) { 
		// printf("%s\n", msg);
	}

	// from xmlsoft http://xmlsoft.org/examples/xpath1.c
	char** get_link_text(const char *body, int *size) {
		char xpath[] = "//body//a"; // /@href";
		const xmlChar *xpathExpr = (xmlChar *) &xpath;

		htmlDocPtr doc;
		xmlXPathContextPtr xpathCtx; 
		xmlXPathObjectPtr xpathObj; 

		/* Load XML document */
		doc = htmlParseDoc((xmlChar *) body, "UTF-8");
		if (!doc) {
			fprintf(stderr, "Error: unable to parse html\n");
			//return 0;
		}

		/* Create xpath evaluation context */
		xpathCtx = xmlXPathNewContext(doc);
		if(!xpathCtx) {
			fprintf(stderr,"Error: unable to create new XPath context\n");
			xmlFreeDoc(doc); 
			//return 0;
		}

		xmlSetGenericErrorFunc(xpathCtx, xmlGenericErrorFunc);

		/* Evaluate xpath expression */
		xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
		if(!xpathObj) {
			fprintf(stderr,"Error: unable to evaluate xpath expression \"%s\"\n", xpathExpr);
			xmlXPathFreeContext(xpathCtx); 
			xmlFreeDoc(doc); 
			//return 0;
		}

		xmlNodeSetPtr nodes = xpathObj->nodesetval;
		*size = nodes->nodeNr;
		char **urls = (char **) calloc(nodes->nodeNr, sizeof(char *));

		xmlNodePtr cur;
		for (int i = 0; i < nodes->nodeNr; i++) {
			cur = nodes->nodeTab[i];
			char * s_url = (char *) xmlGetProp(cur, (xmlChar *) "href");
			int len = strlen(s_url);
			urls[i] = (char *) malloc(sizeof(char)*(len+1));
			memcpy(urls[i], s_url, len);
			urls[i][len] = '\0';
		}

		/* Cleanup */
		xmlXPathFreeObject(xpathObj);
		xmlXPathFreeContext(xpathCtx); 
		xmlFreeDoc(doc); 

		//return 1;
		return urls;
	}
};
