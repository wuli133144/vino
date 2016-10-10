/*
 * HTTP Request Headers
 */
struct http_request_headers {
	char *method;
	char *uri;
	char *query_args;
	char *version;
	char *host;
	char *user_agent;
	char *post_data;
};

struct http_request_headers* parse_headers(int);

struct http_request_headers* parse_method_uri_version(char *, struct http_request_headers*);

struct http_request_headers* parse_post_data(int, struct http_request_headers*);

void read_until_crnl(int);

void http_request_free(struct http_request_headers*);
