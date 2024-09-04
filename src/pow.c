#include "../include/project.h"

AllocatorFunc allocate_hash_string_memory = NULL;

uint64_t proof_of_work(uint64_t last_proof, int difficulty, ErrorDetails* err) {
	// initiaal state: ;ast proff and difficulty are provided
	uint64_t proof = 0;
	printf("\nBEFORE IS VALID PROOF");
	// transformation: find a valid proof
	while (!is_valid_proof(last_proof, proof, difficulty, err)) {
		if (err->error_code != ERROR_NONE) {
			printf("error in is_valid_proof: %s\n", err->message);
			return 0; // Errpr occurred during validation
		}
		proof++;
		// adding  a check to prevent infinite looping
		if (proof == UINT64_MAX) {
			set_error(err, ERROR_LOGIC_ERROR, "proof of work failed to find a solution");
			return 0;
		}
		// add a debug print after every million iterations
		if(proof % 1000000 == 0) {
			printf("proof attempt: %llu\n", proof);
		} 
	}
	printf("\nAFTER IS VALID PROOF\n");
	// final state: Valid proof us found
	return proof;
}


int is_valid_proof (uint64_t last_proof, uint64_t proof, int difficulty, ErrorDetails *err) {
	// intial state: Last proof , current proof and diffuculty are provided
	char buffer[512]; // increased buffer size to cccomodate larger numbers
	int bytes_written = snprintf(buffer, sizeof(buffer), "%llu%llu", last_proof, proof);
	if (bytes_written < 0 || bytes_written >= (int)sizeof(buffer)) {
		set_error(err, ERROR_BUFFER_OVERFLOW, "buffer overflow in is valid proof");
		return 0;
	}

	// input validation for dicciculty
	if (difficulty <= 0 || difficulty > SHA256_DIGEST_LENGTH * 2) {
		set_error(err, ERROR_LOGIC_ERROR, "invalid difficulty test");
		return 0;
	}

	// Tranformation: Genrate hash and check if it meets the require dfificulty
	unsigned char hash[SHA256_DIGEST_LENGTH];
	char hash_string[SHA256_DIGEST_LENGTH * 2 + 1];
	
	if (!SHA256((unsigned char*)buffer, strlen(buffer), hash)) {
		set_error(err, ERROR_IMPL_ERROR, "SHA256 failed inside is_valid_proof");
		return 0;
	}
	
	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++){
		sprintf(&hash_string[i * 2], "%02x", hash[i]);
	}
	hash_string[SHA256_DIGEST_LENGTH * 2] = '\0';



	for (int i = 0 ; i < difficulty; i++) {
		if (hash_string[i] != '0') {
			return 0;
		}
	}

	// final stte: vaalidity of the proof is determined
	return 1;
}


// Helper function to generte hash
char * generate_hash(const char* input, ErrorDetails *err) {
	// initial staate: proof not generateds, input string provvided
	unsigned char hash[SHA256_DIGEST_LENGTH];
	allocate_hash_string_memory = allocate_memory;
	char *hash_string = allocate_hash_string_memory(SHA256_DIGEST_LENGTH * 2 + 1, err);
	if (hash_string == NULL) {
		return NULL; // error set in allocate memory
	}

	// Transformatiooon: Genrate SHA256 hsh
	if(!SHA256((unsigned char*)input, strlen(input), hash)) {
		set_error(err, ERROR_IMPL_ERROR, "sha256 failed in generate hash");
		free(hash_string);
		return NULL;
	}

	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
		sprintf(&hash_string[i * 2], "%02x", hash[i]);
	}
	hash_string[SHA256_DIGEST_LENGTH * 2] = '\0';

	// final state: Hash atrong has been created
	return hash_string;
}

