#ifndef	HTTP_SERVER_H
#define HTTP_SERVER_H

#include "project.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include "cJSON.h"


#define MAX_CONNECTIONS 10 // there exists an interesting bug when this is chnaged to 100.. INVESTIGATE
#define BUFFER_SIZE 4096

typedef struct {
	int socket_fd;
	struct sockaddr_in address;
	Blockchain* blockchain;
} HttpServer;

typedef struct {
	int client_socket;
	char buffer[BUFFER_SIZE];
	size_t buffer_size;
	size_t buffer_index;
} ClientConnection;

typedef enum {
	HTTP_GET,
	HTTP_POST,
	HTTP_UKNOWN,
} HttpMethod;


typedef struct {
	HttpMethod method;
	char path[256];
	char body[BUFFER_SIZE];
} HttpRequest;


typedef struct {
	int status_code;
	char* body;
	size_t body_len;
} HttpResponse;


// httpserver functions
HttpServer* create_http_server(int port, Blockchain* blockchain, ErrorDetails *err);
void free_http_server(HttpServer* server);
ErrorCode start_http_server(HttpServer* server, ErrorDetails* err);
ErrorCode handle_client_connection(ClientConnection* client, Blockchain* blockchain, ErrorDetails* err);
ErrorCode parse_http_request(const char* raw_request, HttpRequest* request, ErrorDetails* err);
ErrorCode route_request(const HttpRequest* request, Blockchain* blockchain, HttpResponse* response, ErrorDetails* err);
ErrorCode send_http_response(int client_socket, const HttpResponse* response, ErrorDetails* err);

// API endpoint handlers
ErrorCode handle_new_transaction(const HttpRequest* request, Blockchain* blockchain, HttpResponse* response, ErrorDetails* err);
ErrorCode handle_mine(Blockchain* blockchain, HttpResponse* response, ErrorDetails* err);
ErrorCode handle_chain(Blockchain* blockchain, HttpResponse* response, ErrorDetails* err);


// JSON helpers
char* blockchain_to_json(const Blockchain* blockchain, ErrorDetails* err);
cJSON* block_to_json(const Block* block, ErrorDetails* err);
Transaction* json_to_transaction(const char* json, ErrorDetails* err);
char* get_block_json_string (const Block* block, ErrorDetails* err);


// testing functions
ErrorCode simulate_http_request(HttpServer* server, const char* method, const char* path, const char* body, HttpResponse* response, ErrorDetails* err);

#endif