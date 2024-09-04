#include "../include/project.h"

Transaction* create_transaction(const char *sender, const char* recipient, double amount, ErrorDetails *err) {
	// Initial state : No transaction exists

	// ionpuut validation
	if (sender == NULL || recipient == NULL || amount < 0) {
		if (err != NULL) {
			err->error_code = ERROR_INVALID_ARGUMENT;
			snprintf(err->message, sizeof(err->message), "Invalid transaction parameter");
			log_error(err);
		}
		return NULL;
	}

	// Transformation: Allocate memory and initialzie transaction
	Transaction *transaction = (Transaction*) allocate_memory(sizeof(Transaction), err);
	if (transaction == NULL) {
		return NULL; // errror details set
	} 

	//  Desired state: Fully initialized transaction
	strncpy(transaction->sender, sender, ADDRESS_SIZE);
	transaction->sender[ADDRESS_SIZE] = '\0';
	strncpy(transaction->recipient, recipient, ADDRESS_SIZE);
	transaction->recipient[ADDRESS_SIZE] = '\0';
	transaction->amount = amount;

	return transaction;

}

// Function to free a transation

void free_transaction(Transaction *transaction) {
	// initial state: Transaaction exiwts in memory

	// Transaction": Free the memory
	if (transaction != NULL) {
		free(transaction);
	}

	// memory freed
}

// OPTIONAL FUNCTION TO VALIDATE A TRANSACTION
bool validate_transaction(const Transaction *transaction, ErrorDetails *err) {
	// Initial state: Transaction exists and needs validation

	// TRANSFORMATION: apply validation
	if (transaction == NULL ){
		if (err!= NULL) {
			err->error_code = ERROR_INVALID_ARGUMENT;
			snprintf(err->message, sizeof(err->message), "Transaction pointer is null ");
			log_error(err);
		}
		return false;
	}

	if (transaction->amount <= 0) {
		if (err != NULL) {
			err->error_code = ERROR_INVALID_ARGUMENT;
			snprintf(err->message, sizeof(err->message), "Transaction amount must be greater than 0");
		}
		return false;
	}

	// Add more validation checks as needed(e.g sender and recipient addresss formart)

	// Desired state: Traansaction validtity determined
	return true;
}