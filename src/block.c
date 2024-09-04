#include "../include/project.h"


// function to create a anew block
Block* create_block(int index, unsigned long long proof, const char *previous_hash, ErrorDetails* err) {
    // initial state: No block esists

    // Transformation: Allocaate memory and initialize block
    Block *block = allocate_block_memory(sizeof(Block), err);
    if (block == NULL) {
        return NULL;
    }

    // Desired state: fully initialized block
    block->index = index;
    block->time_stamp = time(NULL);
    block->transaction_count = 0;
    block->proof = proof;
    strncpy(block->previous_hash, previous_hash, HASH_SIZE);
    block->previous_hash[HASH_SIZE] = '\0';

    return block;

}

// Function to add a strnsction block
ErrorCode add_transaction_to_block(Block *block, const Transaction* transaction, ErrorDetails* err) {
    // intial state:m Blockl exists with spome or no transaction

    // transformation: Add new transction if space is available
    if (block->index < MAX_TRANSACTIONS) {
        memcpy(&block->transactions[block->transaction_count], transaction, sizeof(*transaction));
        block->transaction_count++;
        return ERROR_NONE;
    }
    if (err != NULL) {
        err->error_code = ERROR_BLOCK_FULL;
        snprintf(err->message, sizeof(err->message), "Block is full, cannot add more transactions");

    }
    return ERROR_BLOCK_FULL;
}


// Function to calculate block hash (placeholder)
char* calculate_block_hash(const Block *block, ErrorDetails *err) {
	//Initiaal state: block exists and needs too be hashed
	allocate_hash_string_memory = allocate_memory;
	// Input validation
	if (block == NULL) {
		set_error(err, ERROR_INVALID_ARGUMENT, "Null block pointeer provided to calculate block hash");
		return NULL;
	}
	// debugging block details
	printf("Block details:\n");
	printf("Index: %d\n",block->index );
	printf("Timestamp: %lld\n", (long long)block->time_stamp);
	printf("Transaction count: %d\n", block->transaction_count);
	printf("Proof: %llu\n", block->proof);
	printf("Previous hash: %s\n", block->previous_hash);

	if (block->transaction_count < 0 || block->transaction_count > MAX_TRANSACTIONS) {
		set_error(err, ERROR_LOGIC_ERROR, "Invalid transaction count");
		return NULL;
	}

	// Tramsformation: Serialize block and data
	// 1. Serialize block data
	char *serialized = NULL;
	size_t serialized_length = 0; // store length of serialized dataa
	FILE *memstream = open_memstream(&serialized, &serialized_length); // serialized to be freed
	
	if (memstream == NULL) {
		set_error(err, ERROR_MEMORY_ALLOCATION, "Failed to create a memory stream");
		goto cleanup;
	}

	printf("Inside calculate block hash function before memstream serialization \n");
	// Serializing block data with error checking
	if (fprintf(memstream, "%d", block->index) < 0 || 
		fprintf(memstream, "%lld", (long long)block->time_stamp ) < 0) {
			set_error(err, ERROR_WRITE_FAILED, "Failed to write block data to memory stream");
			goto cleanup;
		}
	printf("Serializing transactions. Count : %d\n", block->transaction_count);
	for (int i = 0; i < block->transaction_count; i++){
		if(i > MAX_TRANSACTIONS) {
			set_error(err, ERROR_BUFFER_OVERFLOW, "Transaction count exceeds MAX_TRANSACTIONNS");
			goto cleanup;
		}
		if (fprintf(memstream, "%s%s%.2f",
					block->transactions[i].sender,
					block->transactions[i].recipient,
					block->transactions[i].amount) < 0) {
			set_error(err, ERROR_WRITE_FAILED, "failed to write transaction data to memstream");
			goto cleanup;
					}
	}

	if (fprintf(memstream, "%llu", block->proof) < 0 ||
		fprintf(memstream, "%s", block->previous_hash) < 0) {
			set_error(err, ERROR_WRITE_FAILED, "Failed to write block proof or previous hash to memory stream");
			goto cleanup;
		}
	fclose(memstream);
	memstream = NULL;
	printf("Inside calculate block hash function after memstream serialization \n");
	printf("Serialization completed. Length: %zu\n", serialized_length);
	// 2. Generate SHA256 hash
	unsigned char  hash[SHA256_DIGEST_LENGTH];
	SHA256((unsigned char*)serialized, serialized_length, hash);
	
	// 3. Convert hash to hexdecimal
	char *hash_string = allocate_hash_string_memory(SHA256_DIGEST_LENGTH * 2 + 1, err);
	printf("Before allocating hash string.\n");
	if (hash_string == NULL) {
		printf("Allocate hash string error: %s", err->message);
		goto cleanup;// Erroor  set in allocate memory
	}

	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
		sprintf(&hash_string[i * 2], "%02x", hash[i]);
	}

	free(serialized);
	printf("Exiting calculate_block_hash function.\n");
	return hash_string;

cleanup:
	if (memstream != NULL) {
		fclose(memstream);
	}

	if (serialized != NULL) {
		free(serialized);
	}
	return NULL;

}

// Function to free a block
void free_block(Block *block) {
    if (block != NULL) {
        free(block);
    }
    // Desired state: Block memory is freed
}