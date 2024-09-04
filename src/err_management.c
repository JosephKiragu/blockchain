#include <stdio.h>
#include <stdlib.h>
#include "../include/project.h"


void set_error(ErrorDetails *err, ErrorCode e_code, const char *message) {
	if (err == NULL || message == NULL || strcmp(message, "")) {
		printf("Invalid set error parameters");
		return;
	}
		
	err->error_code = e_code;
	snprintf(err->message, sizeof(err->message), "%s", message);
	fprintf(stderr, "Error Code: %d\nError Message: %s\n", err->error_code, err->message);
	return;
}

void log_error(const ErrorDetails *err) {
	FILE *log_file = fopen("log_file.txt", "a");
	if (log_file != NULL) {
		fprintf(log_file, "Error Code: %d\nError Message: %s\n", err->error_code, err->message);
		fclose(log_file);
	}

}


// function to allocate memory with error handling
void *allocate_memory(size_t size, ErrorDetails *err){
	void*ptr = malloc(size);
	if (ptr == NULL) {
		if(err != NULL) {
			err->error_code = ERROR_MEMORY_ALLOCATION;
			snprintf(err->message, sizeof(err->message), "Memory allocation failed");
		}
		return NULL;
	}
	return ptr;
}

// function to reallocate memory with error haandling
void *reallocate_memory(void *ptr, size_t size, ErrorDetails *err) {
	void *new_ptr = realloc(ptr, size);
	if (new_ptr == NULL) {
		if (err != NULL) {
			set_error(err, ERROR_MEMORY_ALLOCATION, "Error reaallocating memory");
		}
		return NULL;
	}
	return new_ptr;
}



// Function to handle errors
void handle_error(const ErrorDetails *err) {
	if (err != NULL && err->error_code != ERROR_NONE) {
		fprintf(stderr, "Error: %s", err->message);
		exit(EXIT_FAILURE);
	}
}

ErrorDetails init_err() {
	ErrorDetails err = {ERROR_NONE, ""};
	return err;
}
