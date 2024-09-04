
#include "../include/project.h"
#include "../include/httpserver.h"



HttpServer* create_http_server(int port, Blockchain* blockchain, ErrorDetails *err) {
	// initial state:
	// no seerver object
	// port number and chain provided
	// No socket or address configured
	
	// Transformation
	// step 1: Allocate memory to server
	HttpServer* server = (HttpServer*)allocate_memory(sizeof(HttpServer), err); // remember to close
	if (server == NULL) {
		return NULL;
	}

	// 2. Create the socket
	server->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server->socket_fd == -1) {
		set_error(err, ERROR_IMPL_ERROR, "Failed to create socket");
		free(server);
	}

	// 3. configure the socket
	server->address.sin_family = AF_INET; // setting ipv4 domain
	server->address.sin_addr.s_addr = INADDR_ANY; // setting ip address
	server->address.sin_port = htons(port); // setting port number

	// 4. Bind the socket
	if (bind(server->socket_fd, (struct sockaddr*)&server->address, sizeof(server->address)) < 0) {
		set_error(err, ERROR_IMPL_ERROR, "Failed to bind socket");
		close(server->socket_fd);
		return NULL;
	}

	// 5. Associaate the blockchain
	server->blockchain = blockchain;

	// Desired statee:
	// fully initialized http server
	// socket created and bound
	// address configured
	// blockchain associated
	return server;
}

void free_http_server(HttpServer* server) {
	// initial state: existing http server

	// transformation:
	// check if server exists
	if (server) {
		// close the socket
		close(server->socket_fd);

		// free the memory
		free(server); // free after end of server ops
	}

	// desired state
	// socket closed
	// server object freed
	// no memory leaks
}

ErrorCode start_http_server(HttpServer* server, ErrorDetails* err) {
	// initial state:
	// initialized httpserver object
	// server not yet listening

	// transformation
	// starat listeneing on the socket
	if (listen(server->socket_fd, MAX_CONNECTIONS) < 0) {
		set_error(err, ERROR_IMPL_ERROR, "Failed to listen on socket");
		return ERROR_IMPL_ERROR;
	}

	printf("HTTP server listening on port %d\n", ntohs(server->address.sin_port));

	// enter the main server loop
	while(1) {
		//initialize the client connection
		ClientConnection client;
		socklen_t addrlen = sizeof(server->address);

		// accept the client connection
		client.client_socket = accept(server->socket_fd, (struct sockaddr*)&server->address, &addrlen);
		if (client.client_socket < 0) {
			set_error(err, ERROR_IMPL_ERROR, "Failed to accept client connection");
			continue;
		}

		// set the client address to non-blocking mode
		int flags = fcntl(client.client_socket, F_GETFL, 0);
		fcntl(client.client_socket, F_SETFL, flags | O_NONBLOCK);

		// initialize the client buffers
		client.buffer_size = 0;
		client.buffer_index = 0;

		// handle the client coonnection.
		ErrorCode result = handle_client_connection(&client, server->blockchain, err);
		if (result != ERROR_NONE) {
			log_error(err);
		}

		// close client connection
		close(client.client_socket);
	}

	// desired state:
	// server continously listening and handlingn connections
	// each cliooent connection prpocccessed and closed
	// errors logged annd handled grcefully
}

ErrorCode handle_client_connection(ClientConnection* client, Blockchain* blockchain, 
									ErrorDetails* err) {
	// iniitiaal state:
	// clientconnection object with initialized buffer
	// blockchain object
	// empty error details

	// transformation
	while (1) {
		// read data from client socket
		ssize_t bytes_read = read(client->client_socket, client->buffer + client->buffer_size, 
									BUFFER_SIZE - client->buffer_size);

		// process red operaation result
		if (bytes_read > 0) {
			client->buffer_size += bytes_read;
		} else if (bytes_read == 0) {
			// connetion closed by client
			return ERROR_NONE;
		} else if (errno != EAGAIN && errno != EWOULDBLOCK) {
			set_error(err, ERROR_IMPL_ERROR, "Error reading from client socket");
			return ERROR_IMPL_ERROR;
		}

		// prpocess reeceived dataa
		while(client->buffer_index < client->buffer_size) {
			HttpRequest request;
			HttpResponse response = {0};

			// parse the http request 
			ErrorCode parse_result = parse_http_request(client->buffer, &request, err);

			if(parse_result == ERROR_NONE) {
				// route request and generate respoonse
				ErrorCode route_result = route_request(&request, blockchain, &response, err);
				if (route_result != ERROR_NONE) {
					log_error(err);
					response.status_code = 500;
					response.body = "Internal Server Error";
					response.body_len = strlen(response.body);
				}

				// send http response
				ErrorCode send_result = send_http_response(client->client_socket, &response, err);
				if (send_result != ERROR_NONE) {
					return send_result;
				}

				// update buffer index
				client->buffer_index += strlen(client->buffer + client->buffer_index);
			} else if (parse_result == ERROR_INVALID_ARGUMENT) {
				// Incomplete request, wait for more data
				break;
			} else {
				// Error prsing request
				return parse_result;
			}
		}

		// reset buffer if all data processed
		if (client->buffer_index == client->buffer_size) {
			client->buffer_index = 0;
			client->buffer_size = 0;
		}
	}

	// desired state
	// all client requests parsed
	// responses sent for each valid request
	// errors handled and logged
	// buffer management maintained
}

ErrorCode parse_http_request(const char* raw_request, HttpRequest* request, ErrorDetails* err) {
	// Initial stte:
	// Raw http request string
	// empty Http requesst object
	// eempty errord details


	// trnsformaation:
	// Parse HTTPP method
	if(strncmp(raw_request, "GET ", 4) == 0) {
		request->method = HTTP_GET;
		raw_request += 4;
	} else if (strncmp(raw_request, "POST ",5) == 0) {
		request->method = HTTP_POST;
		raw_request += 5;
	} else {
		request->method = HTTP_UKNOWN;
		set_error(err, ERROR_INVALID_ARGUMENT, "Uknown HTTP method");
		return ERROR_INVALID_ARGUMENT;
	}

	// Parse the path
	const char* path_end = strchr(raw_request, ' ');
	if (path_end == NULL) {
		set_error(err, ERROR_INVALID_ARGUMENT, "Invalid http request format");
		return ERROR_INVALID_ARGUMENT;
	}

	size_t path_length = path_end - raw_request;
	if (path_length >= sizeof(request->path)) {
		set_error(err, ERROR_BUFFER_OVERFLOW,"Path too long");
		return ERROR_BUFFER_OVERFLOW;
	}

	strncpy(request->path, raw_request, path_length);
	request->path[path_length] = '\0';

	// parse the body
	const char* body_start = strstr(raw_request, "\r\n\r\n");
	if (body_start != NULL) {
		body_start += 4;
		strncpy(request->body,  body_start, sizeof(request->body) - 1);
		request->body[sizeof(request->body) - 1] = '\0';
	} else {
		request->body[0] = '\0';
	}

	// Desired state:
	// Parsee HttppRequest object with method, path nd body
	// Any parsing errors caufght and repported
	return ERROR_NONE;
}

ErrorCode route_request(const HttpRequest* request, Blockchain* blockchain, HttpResponse *response, \
						ErrorDetails *err) {
	// Initial state: 
	// Parsed httprequest object
	// Blockchain object
	// Empty http resppponse object
	// empty error details

	// Transformation:
	// Match request path and method to appropriate handler
	if (strcmp(request->path, "/transactions/new") == 0 && request->method == HTTP_POST) {
		return handle_new_transaction(request, blockchain, response, err);
	} else if (strcmp(request->path, "/mine") == 0 && request->method == HTTP_GET) {
		return handle_mine(blockchain, response, err);
	} else if (strcmp(request->path, "/chain") == 0 && request->method == HTTP_GET) {
		return handle_chain(blockchain, response, err);
	} else {
		// handle unmatched routes
		response->status_code = 404;
		response->body ="Not Found";
		response->body_len = strlen(response->body);
		return ERROR_NONE;
	}

	// Desired state:
	// request routed to pppropriate handler
	// response populated with result of handler
	// 404 response for unmatched routes

}

ErrorCode send_http_response(int client_socket, const HttpResponse* response, ErrorDetails *err) {
	// Initial state:
	// client socket
	//populated response object
	// empty error details

	// transformation
	// prepare http header
	char header[256];
	int header_len = snprintf(header, sizeof(header),
		"HTTP/1.1 %d %s\r\nContent-Type: application/json\r\nContent-Length: %zu\r\n\r\n",
		response->status_code,
		response->status_code == 200 ? "OK" : "Error",
		response->body_len);
	
	// send the http header
	if (send(client_socket, header, header_len, 0) < 0) {
		set_error(err, ERROR_WRITE_FAILED, "Failed to send response header");
		return ERROR_WRITE_FAILED;
	}

	// send HTTP body
	if (send(client_socket, response->body, response->body_len, 0) < 0) {
		set_error(err, ERROR_WRITE_FAILED, "Failed to send respsonse header");
		return ERROR_WRITE_FAILED;
	}

	// Desires state:
	// Complete HTTP response (header and body) sent to client
	// any send errors caught and reported
	return ERROR_NONE;
}

ErrorCode handle_new_transaction(const HttpRequest* request, Blockchain* blockchain, HttpResponse* response, \
								ErrorDetails* err) {
	// Initial state:
	// httprequest with transaction data
	// blockchaain object
	// empty httpresponse
	// empty error details

	// traansformatioon
	// Parse transaction from JSON
	Transaction* transaction = json_to_transaction(request->body, err); // to be freed
	if (transaction == NULL) {
		response->status_code = 400;
		response->body = "Invalid transaction data";
		response->body_len = strlen(response->body);
		return ERROR_NONE;
	}

	// Get the laast block from the blockchain
	Block* last_block = get_last_block(blockchain, err);
	if (last_block == NULL) {
		free_transaction(transaction);
		response->status_code = 500;
		response->body = "Failed to get the last block";
		response->body_len = strlen(response->body);
		return ERROR_NONE;
	}

	// Add the transaction to the new block
	ErrorCode add_result = add_transaction_to_block(last_block, transaction, err);
	free_transaction(transaction);

	if (add_result != ERROR_NONE) {
		response->status_code = 500;
		response->body = "Failed to add transaction to block";
		response->body_len = strlen(response->body);
		return ERROR_NONE;
	}

	// preppare success response
	response->status_code = 200;
	response->body = "Transaction added successfully";
	response->body_len = strlen(response->body);

	// Desired state:
	// Transaction added to the last block in the blockchain
	//HTTPResspponse populated with result (success or error)
	return ERROR_NONE;
}


ErrorCode handle_mine(Blockchain* blockchain, HttpResponse* response, ErrorDetails* err) {
	// intial state:
	// HttpRequest (unused rem to remove or maybe aaddress should be  aattached to the request)
	// Blockchain object
	//Empty Httpresponse
	// Empty error details

	// Transformation
	// mine block
	printf("\nentering handle_mine function\n");
	Block *new_block = mine_block(blockchain, "Miner-Address", err);
	if (new_block == NULL) {
		printf("failed to mine block: %s\n", err->message);
		response->status_code = 500;
		response->body = "Failed to mine block";
		response->body_len = strlen(response->body);
		return ERROR_NONE;
	}

	// Serialize the new block to JSON
	char* block_json = get_block_json_string(new_block, err);
	if (block_json == NULL) {
		printf("failed to serialize message %s\n", err->message);
		response->status_code = 500;
		response->body = "Failed to serialize the mined block";
		response->body_len = strlen(response->body);
		return ERROR_NONE;
	}

	// Prepare success response
	response->status_code = 200;
	response->body = block_json;
	response->body_len = strlen(block_json);

	// desired state:
	// new block mined and added to blockchain
	// HttpResponsee populted with the serialized new block (or error message)
	printf("exiting handle mine function\n");
	return ERROR_NONE;
}

ErrorCode handle_chain(Blockchain* blockchain, HttpResponse* response,ErrorDetails* err) {
	// initial state
	// Httprequest (currently unused in this function)
	// blockhain object
	// errordetails object

	// transformation
	// serialize the blockchain
	char* chain_json = blockchain_to_json(blockchain, err);
	if (chain_json == NULL) {
		response->status_code = 500;
		response->body = "Failed to serialize blockchain";
		response->body_len = strlen(response->body);
		return ERROR_NONE;
	} 

	// Prepare the response
	response->status_code = 200;
	response->body = chain_json;
	response->body_len = strlen(chain_json);

	// desired state:
	// Httpresponse populated with serialized json blockchain
	return ERROR_NONE;
	
}

Transaction* json_to_transaction(const char* json, ErrorDetails *err) {
	// initial stage
	// json string contaaining transaction data
	// error details pointer

	// transformation
	// parse the json string
	cJSON* root = cJSON_Parse(json); // to be freed
	if (root == NULL) {
		const char* error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL) {
			set_error(err, ERROR_INVALID_ARGUMENT, "invalid json transaction: json to transaction function");
		}
		return NULL;
	}

	// allocate transaction memory
	Transaction* transaction = (Transaction *)allocate_memory(sizeof(Transaction), err); // buffer for storing transacrion details to be freed
	if (transaction == NULL) {
		cJSON_Delete(root);
		return NULL;
	}

	//extract sender
	cJSON* sender = cJSON_GetObjectItemCaseSensitive(root, "sender");
	if (!cJSON_IsString(sender) || sender->valuestring == NULL) {
		set_error(err, ERROR_INVALID_ARGUMENT, "error extracting sender from json transaction");
		goto cleanup;
	}

	if (strlen(sender->valuestring) >= ADDRESS_SIZE) {
		set_error(err, ERROR_BUFFER_OVERFLOW, "buffer overflow at sender address");
		goto cleanup;
	}

	strncpy(transaction->sender, sender->valuestring, ADDRESS_SIZE - 1);
	transaction->sender[ADDRESS_SIZE - 1] = '\0'; // Revisit this logic

	// extract recipient
	cJSON* recipient = cJSON_GetObjectItemCaseSensitive(root, "recipient");
	if (!cJSON_IsString(recipient) || recipient->valuestring == NULL) {
		set_error(err, ERROR_INVALID_ARGUMENT, "error extracting recipient");
		goto cleanup;
	}

	// validate input
	if (strlen(sender->valuestring) >= ADDRESS_SIZE - 1) {
		set_error(err, ERROR_BUFFER_OVERFLOW, "buffer flow when extracting transaction address");
	}

	strncpy(transaction->recipient, recipient->valuestring, ADDRESS_SIZE - 1);
	transaction->recipient[ADDRESS_SIZE - 1] = '\0';

	// Extract amount
	cJSON *amount = cJSON_GetObjectItemCaseSensitive(root, "amount");
	if(!cJSON_IsNumber(amount)) {
		set_error(err, ERROR_INVALID_ARGUMENT, "invalid transaction amount");
		goto cleanup;
	}

	transaction->amount = amount->valuedouble;

	// desired state
	// clean up, return transaction object
	cJSON_Delete(root);
	return transaction;




cleanup:
	cJSON_Delete(root);
	free_transaction(transaction);
	return NULL;
}

cJSON* block_to_json(const Block* block, ErrorDetails* err) {
	//initial state: 
	// empty block,
	// empty error details,
	// no json representation

	// transformation: create root json for the block object
	cJSON* root = cJSON_CreateObject();
	if (root == NULL) {
		set_error(err, ERROR_MEMORY_ALLOCATION, "error creating cjson root");
		return NULL;
	}

	// add block metadata
	if (!cJSON_AddNumberToObject(root, "index", block->index) ||
		!cJSON_AddNumberToObject(root, "timestamp", block->time_stamp) || 
		!cJSON_AddNumberToObject(root, "proof", block->proof) || 
		!cJSON_AddStringToObject(root, "previous_hash", block->previous_hash)) {

		set_error(err, ERROR_MEMORY_ALLOCATION, "error allocating memory for block json");
		goto cleanup;
		}

	// create transactioons array
	cJSON* transactions = cJSON_AddArrayToObject(root, "transactions"); // to be cleaned
	if (transactions == NULL) {
		set_error(err, ERROR_MEMORY_ALLOCATION, "error allocating memory for json block");
		goto cleanup;
	}

	// aadd each transaction to the array
	for (int i = 0 ; i < block->transaction_count; i++) {
		cJSON* transaction = cJSON_CreateObject();
		if (transaction == NULL) {
			set_error(err, ERROR_MEMORY_ALLOCATION, "error allocating memory to json object");
			goto cleanup;
		}

		if (!cJSON_AddStringToObject(transaction, "sender", block->transactions[i].sender) ||
			!cJSON_AddStringToObject(transaction, "recipient", block->transactions[i].recipient) ||
			!cJSON_AddNumberToObject(transaction, "amount", block->transactions[i].amount)) {
			set_error(err, ERROR_MEMORY_ALLOCATION, "error adding transaction to json");
			cJSON_Delete(transaction);
			goto cleanup;
		}

		if (!cJSON_AddItemToArray(transactions, transaction)) {
			set_error(err, ERROR_MEMORY_ALLOCATION, "error creating transaction memory");
			cJSON_Delete(transaction);
			goto cleanup;
		}
	}

	// desired state:
	// cjson obkect representation of the block
	// NULL terminaated and error set if serialization fails
	return root;


cleanup:
	cJSON_Delete(root);
	return NULL;
}

// helper functionn to set jsson string from cjson object
char* get_block_json_string (const Block* block, ErrorDetails* err) {
	cJSON* json_object = block_to_json(block, err);

	if (json_object == NULL) {
		return NULL;
	}
	char* json_str = cJSON_Print(json_object);
	cJSON_Delete(json_object);
	
	if (json_str == NULL) {
		set_error(err, ERROR_MEMORY_ALLOCATION, "error extracting json string of block");
		return NULL;
	}

	return json_str;	

}


char* blockchain_to_json(const Blockchain* blockchain, ErrorDetails *err) {
		// initiala statae:
		// blockchain object
		// emppty error detailss
		// no jsosn representtiion

		// transformtion:

		// creaate root json object
		cJSON* root = cJSON_CreateObject();
		if (root == NULL) {
			set_error(err, ERROR_MEMORY_ALLOCATION, "error alloctinng memory inside blockchain to json f");
			return NULL;
		}

		// add blockchain length
		if (!cJSON_AddNumberToObject(root, "length", blockchain->length)) {
			set_error(err, ERROR_MEMORY_ALLOCATION, "failed to create cjson length object");
			goto cleanup;
		}

		// create blocks aarray
		cJSON* blocks = cJSON_AddArrayToObject(root, "blocks");
		if (blocks == NULL) {
			set_error(err, ERROR_MEMORY_ALLOCATION, "error creating cjson array to store blocks");
			goto cleanup;
		}

		for (int i = 0; i < blockchain->length; i++) {
			cJSON* block_json = block_to_json(blockchain->blocks[i], err);
			if (block_json == NULL) {
				set_error(err, ERROR_MEMORY_ALLOCATION, "error creating memoery for cjson block object");
				goto cleanup;
			}

			if(!cJSON_AddItemToArray(blocks, block_json)) {
				set_error(err, ERROR_MEMORY_ALLOCATION, "error adding block to blocks json");
				cJSON_Delete(block_json);
				goto cleanup;
			}
		}

		// convert json to string
		char* json_str = cJSON_Print(root);
		if (json_str == NULL) {
			set_error(err, ERROR_MEMORY_ALLOCATION, "error converting blockchain json object to string");
			goto cleanup;
		}

		// c;eanup and return
		cJSON_Delete(root);
		return json_str;


cleanup:
	cJSON_Delete(root);
	return NULL;
}