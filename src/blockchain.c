#define INITIAL_CAPACITY 10
#define INITIAL_DIFFICULTY 4

#include "../include/project.h"


AllocatorFunc allocate_block_chain_memory = NULL;
AllocatorFunc allocate_block_memory = NULL;
ReallocatorFunc reallocate_chain_capacity = NULL;

Blockchain *create_blockchain(ErrorDetails *err)
{
	
	allocate_block_chain_memory = allocate_memory; // initializing function pointers
	allocate_block_memory = allocate_memory; // Initializing function pointers

	// Transformation: createe and initialize the blockchain
	Blockchain *chain = (Blockchain *)allocate_block_chain_memory(sizeof(Blockchain), err);

	chain->blocks = (Block **)allocate_block_memory(sizeof(Block *) * INITIAL_CAPACITY, err);

	chain->length = 0;
	chain->capacity = INITIAL_CAPACITY;
	chain->difficulty = INITIAL_DIFFICULTY;

	// create genesis block
	Block *genesis = create_block(0, 1, "0", err);
	if (genesis == NULL)
	{
		goto cleanup;
	}

	chain->blocks[0] = genesis;
	chain->length = 1;
	// Final state: Blockahin with genesis block is creaated
	return chain;

cleanup:
	free(chain->blocks);
	free(chain);
	return NULL;
}

void free_blockchain(Blockchain *chain)
{
	// Initial staate: Blockchain exists with allocted memory

	// Trnsfformtion: Free all blocks and the blockchain itself
	if (chain != NULL)
	{
		for (int i = 0; i < chain->length; i++)
		{
			free_block(chain->blocks[i]);
		}
		free(chain->blocks);
		free(chain);
	}
	// final state: All cllocated memory is freed
}

// Check chain validity
ErrorCode is_chain_valid_(const Blockchain* chain, ErrorDetails* err) {
	// Initial state: Bloockchaian exists and needs vaalidation

	// input validation
	if (chain == NULL || chain->length == 0) {
		set_error(err, ERROR_INVALID_ARGUMENT, "Invalid blockchain provided for validation");
		return ERROR_INVALID_ARGUMENT;
	}

	// Transformation: Validate each blocl in the chain
	for (int i = 1; i < chain->length; i++) {
		Block* current_block = chain->blocks[i];
		Block* previous_block = chain->blocks[i - 1];

		// Check if the previous hash matches
		char *previous_hash = calculate_block_hash(previous_block, err);
		if (previous_hash == NULL) {
			return ERROR_HASH_CALCULATION;
		}

		if (strcmp(current_block->previous_hash, previous_hash) != 0) {
			free(previous_hash);
			set_error(err, ERROR_INVALID_CHAIN, "Block hash mismatch");
			return ERROR_INVALID_CHAIN;
		}
		free(previous_hash);
	}

	return ERROR_NONE;


}

ErrorCode add_block(Blockchain *chain, Block *block, ErrorDetails *err)
{
	// initial state: Blockchain exists and anew block is prpovided
	printf("Entering add_block function\n");
	reallocate_chain_capacity = reallocate_memory;

	// input validation
	if (chain == NULL || block == NULL)
	{
		set_error(err, ERROR_INVALID_ARGUMENT, "Null pointer provided to add_block");
		return ERROR_INVALID_ARGUMENT;
	}
	printf("Current chain length: %d, capacity: %d\n", chain->length, chain->capacity);

	// transformaation: Add the new blocm to the chain
	if (chain->length == chain->capacity)
	{	
		printf("Resizing blockchain capacity\n");
		int new_capacity = chain->capacity * 2;
		Block **new_blocks = (Block **)reallocate_chain_capacity(chain->blocks, sizeof(Block *) * new_capacity, err);
		if (new_capacity > chain->capacity) {
			memset(new_blocks + chain->capacity, 0, (new_capacity - chain->capacity) * sizeof(Block*));
		}
		if (new_blocks == NULL)
		{
			printf("Error in reallocate_chain_capacity: %s\n", err->message);
			return ERROR_MEMORY_ALLOCATION;
		}
		chain->blocks = new_blocks;
		chain->capacity = new_capacity;

		printf("Block added. New chainn length: %d\n", chain->length);
		printf("Exiting add_block_function\n");
	}

	chain->blocks[chain->length] = block;
	chain->length++;

	// final taate: new block is added to the chain
	return ERROR_NONE;
}

Block* get_last_block(const Blockchain* chain, ErrorDetails *err) {
	// initil state: Blockchain exists

	// input valaidtion
	if (chain == NULL || chain->length == 0) {
		set_error(err, ERROR_INVALID_ARGUMENT, "Invalid blockchin provided to get_last_block");
		return NULL;
	}

	// Transformtioon retrive the last block
	return chain->blocks[chain->length - 1];
}


Block* mine_block(Blockchain* chain, const char* miner_address, ErrorDetails* err) {
	// inital state: Blockchaain exists and rrady for mining
	printf("Entering mine_block function\n");
	if (chain == NULL || miner_address == NULL) {
		set_error(err, ERROR_INVALID_ARGUMENT, "Invalid blockchain or miner aaddress");
		return NULL;
	}

	Block *last_block = NULL; // pointer to last block
	char* last_hash = NULL; // pointer to last hash
	Block* new_block = NULL; // pointer to new block
	Transaction *reward_tx = NULL; // Pointer to trasaction
	

	// Transformatioon: Create a new block with a valid proof
	last_block = get_last_block(chain, err);
	printf("Last block retrieved: %p\n", (void*)last_block);
	if (last_block == NULL) {
		set_error(err, ERROR_INVALID_ARGUMENT, "Unable to get last block");
		goto cleanup;
	}
	

	uint64_t last_proof = last_block->proof;
	printf("Last proof: %llu\n", last_proof);
	printf("\nBEFORE PROOF OF WORK\n");
	uint64_t proof = proof_of_work(last_proof, chain->difficulty, err);
	printf("\nAFTER PROOF OF WORK\n");
	printf("New proof found: %llu\n", proof);
	if (err->error_code != ERROR_NONE) {
		printf("Error in proof of work: %s\n", err->message);
		goto cleanup; // error occurred during proof of work
	}
	
	last_hash = calculate_block_hash(last_block, err);
	
	if (last_hash == NULL) {
		printf("Error in calculate_block_hash: %s\n", err->message);
		goto cleanup;
	}
	
	new_block = create_block(chain->length, proof, last_hash, err);
	printf("New Block created: %p\n", (void *)new_block);
	if (new_block == NULL) {
		goto cleanup;
	}

	reward_tx = create_transaction("0", miner_address, 1.0, err);
	printf("Reward transaction created: %p\n", (void*)reward_tx);
	if (reward_tx == NULL) {
		goto cleanup; // Error already set in create transaction
	}

	ErrorCode add_result = add_transaction_to_block(new_block, reward_tx, err);
	printf("Transaction aadded to block, reslt: %d\n", add_result);
	if (add_result != ERROR_NONE) {
		printf("Error in add_transaction_to_block: %s\n", err->message);
		goto cleanup;
	}
	

	printf("Exiting mine_block function\n");
	return new_block;

cleanup:
	if (reward_tx != NULL) {
		free_transaction(reward_tx);
	}
	if(new_block != NULL) {
		free_block(new_block);
	}
	if(last_hash != NULL) {
		free(last_hash);
	}

	return NULL;
}