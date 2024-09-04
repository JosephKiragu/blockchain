#ifndef PROJECT_H
#define PROJECT_H


#define MAX_TRANSACTIONS 100
#define HASH_SIZE 64 // Consider 32 for balance between performance and security
#define ADDRESS_SIZE 40
#define MINER_REWARD 100.00




#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <assert.h>


// ---------
// |
// || TRANSACTION
// |
// -------

// STRUCT
typedef struct {
	char sender[ADDRESS_SIZE + 1];
	char recipient[ADDRESS_SIZE + 1];
	double amount;
} Transaction;


// ---------
// |
// || BLOCK 
// |
// -------
// 
typedef struct {
	int index;
	time_t time_stamp;
	Transaction transactions[MAX_TRANSACTIONS];
	int transaction_count;
	unsigned long long proof;
	char previous_hash[HASH_SIZE + 1];
}  Block;


// ---------
// |
// || BLOCKCHAIN
// |
// -------
// 
typedef struct {
	Block **blocks;
	int length;
	int capacity;
	int difficulty;
} Blockchain;



// ---------
// |
// || ERROR CODES
// |
// -------
// 
typedef enum {
	ERROR_NONE = 0,
	ERROR_MEMORY_ALLOCATION,
	ERROR_BLOCK_FULL,
	ERROR_INVALID_ARGUMENT,
	ERROR_IMPL_ERROR,
	ERROR_LOGIC_ERROR,
	ERROR_HASH_CALCULATION,
	ERROR_INVALID_CHAIN,
	ERROR_WRITE_FAILED,
	ERROR_BUFFER_OVERFLOW,
	ERROR_FUNC_ERROR,
	ERROR_NULL_PTR

} ErrorCode;


// ---------
// |
// || ERROR DETAILS
// |
// -------
// 

typedef struct {
	ErrorCode error_code;
	char message[256];
} ErrorDetails;





// BLOCK FUNCTIONS

Block* create_block(int index, unsigned long long proof, const char *previous_hash, ErrorDetails* err);
ErrorCode add_transaction_to_block(Block *block, const Transaction* transaction, ErrorDetails* err);
char *calculate_block_hash(const Block *block, ErrorDetails *err);
void free_block(Block *block);


// TRANSACTION FUNCTIONS
Transaction* create_transaction(const char *sender, const char* recipient, double amount, ErrorDetails *err);
void free_transaction(Transaction *transaction);
bool validate_transaction(const Transaction *transaction, ErrorDetails *err); // optional


// BlOCKCHAIN FUNCTIONS
Blockchain *create_blockchain(ErrorDetails *err);
void free_blockchain(Blockchain* chain);
ErrorCode add_block(Blockchain* chain, Block* block, ErrorDetails* err);
int is_chain_valid(const Blockchain* chain);
ErrorCode is_chain_valid_(const Blockchain* chain, ErrorDetails* err);
Block* get_last_block(const Blockchain* chain, ErrorDetails *err);
Block* mine_block(Blockchain *chin, const char* miner_address, ErrorDetails* err);


// MEMORY MANAGEMENT FUNCTIONS
void* reallocate_memory(void *ptr, size_t size, ErrorDetails *err);
void* allocate_memory(size_t size, ErrorDetails* err);

// ERROR FUNCTIONS
void log_error(const ErrorDetails *err);
void handle_error(const ErrorDetails *err);
void set_error(ErrorDetails *err, ErrorCode e_code, const char *message);
ErrorDetails init_err();

// TESTING FUUNCTIONS



// PROOF OF WORK FUNCTIONS
uint64_t proof_of_work(uint64_t last_proof, int difficulty, ErrorDetails *err);
int is_valid_proof(uint64_t last_prooof, uint64_t proof, int difficulty, ErrorDetails *err);
char *generate_hash(const char* input, ErrorDetails* err);


// FUNCTION POINTERS

// Assign a function pointer for the allocator
typedef void* (*AllocatorFunc)(size_t, ErrorDetails*);
typedef void* (*ReallocatorFunc)(void *, size_t, ErrorDetails*);

extern AllocatorFunc allocate_block_memory ;
extern AllocatorFunc allocate_block_chain_memory;
extern AllocatorFunc allocate_hash_string_memory;
extern ReallocatorFunc reallocate_chain_capacity;

#endif