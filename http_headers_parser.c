#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "http_headers_parser.h"
#include "rio.h"

/*-------------------------------------------------------*
	parse the headers of HTTP requset
  -------------------------------------------------------*/
struct http_request_headers* parse_headers(int fd) {
	struct http_request_headers *headers = (struct http_request_headers*)malloc(sizeof(struct http_request_headers));
	if(headers == NULL) {
		perror("malloc");
		return NULL;
	}

	char header_buf[BUFSIZ];    /* HTTP header */
	// char host_buf[BUFSIZ];    /* HTTP host */
	// char user_agent_buf[BUFSIZ];    /* HTTP user-agent */
	
	rio_t rio;
	rio_readinitb(&rio, fd);

	if(rio_readlineb(&rio, header_buf, BUFSIZ) != -1) {
		headers = parse_method_uri_version(header_buf, headers);
		if(strcmp(headers->method, "POST") == 0) {
			headers = parse_post_data(fd, headers);
		}
	} else {
		perror("rio_readlineb");
	}
	
	return headers;

}

/*-------------------------------------------------------*
	parse the first line of HTTP request to fetch 
	HTTP method, HTTP uri and HTTP version.
  -------------------------------------------------------*/
struct http_request_headers* parse_method_uri_version(char *header_buf, struct http_request_headers *headers) {

	char method[BUFSIZ], uri_args[BUFSIZ], uri[BUFSIZ], query_args[BUFSIZ], version[BUFSIZ];
	char *pivot = NULL;    /* the location of '?' */
	int uri_len = 0;

	sscanf(header_buf, "%s %s %s\r\n", method, uri_args, version);
	
	headers->method = (char*)malloc(sizeof(char) * (strlen(method) + 1));
	memset(headers->method, 0, strlen(method) + 1);    /* initialize the array to 0 */
	strcpy(headers->method, method);
	
	pivot = strchr(uri_args, '?');    /* pivot point to character '?' */
	if(pivot != NULL) {    /* without any arguments */
		uri_len = (int)(pivot - uri_args);
	} else {
		uri_len = strlen(uri_args);
	}

	strncpy(uri, uri_args, uri_len);
	headers->uri = (char*)malloc(sizeof(char) * (strlen(uri) + 1));
	memset(headers->uri, 0, strlen(uri) + 1);
	strcpy(headers->uri, uri);
	
	if(pivot != NULL) {    /* without any arguments */
		strcpy(query_args, pivot + 1);
		headers->query_args = (char*)malloc(sizeof(char) * (strlen(query_args) + 1));
		memset(headers->query_args, 0, strlen(query_args) + 1);
		strcpy(headers->query_args, query_args);
	} else {
		headers->query_args = NULL;
	}

	headers->version = (char*)malloc(sizeof(char) * (strlen(version) + 1));
	memset(headers->version, 0, strlen(version) + 1);
	strcpy(headers->version, version);
	
	return headers;
}

/*-------------------------------------------------------*
	parse the HTTP request entity to get post parameters.
  -------------------------------------------------------*/
struct http_request_headers* parse_post_data(int fd, struct http_request_headers *request) {
	
	rio_t rio;
	char buf[BUFSIZ];

	rio_readinitb(&rio, fd);
	read_until_crnl(fd);
	rio_readlineb(&rio, buf, BUFSIZ);    /* skip the rest of HTTP headers */

	request->post_data = (char*)malloc(sizeof(char) * (strlen(buf) + 1));
	memset(request->post_data, 0, strlen(buf) + 1);
	strcpy(request->post_data, buf);

	return request;

}

/*-------------------------------------------------------*
	skip other HTTP headers.
  -------------------------------------------------------*/
void read_until_crnl(int fd) {

	char buf[BUFSIZ];
	rio_t rio;
	rio_readinitb(&rio, fd);

	while(rio_readlineb(&rio, buf, BUFSIZ) != -1 && strcmp(buf, "\r\n") != 0
		&& strcmp(buf, "\n") != 0) {

		;
	}

}

/*-------------------------------------------------------*
	free the memory of parsed HTTP headers.
  -------------------------------------------------------*/
void http_request_free(struct http_request_headers *request) {
	if(request->method != NULL) {
		free(request->method);
	}
	if(request->uri != NULL) {
		free(request->uri);
	}
	if(request->query_args != NULL) {
		free(request->query_args);
	}
	if(request->version != NULL) {
		free(request->version);
	}
	if(request->host != NULL) {
		free(request->host);
	}
	if(request->user_agent != NULL) {
		free(request->user_agent);
	}
	if(request->post_data != NULL) {
		free(request->post_data);
	}
}
