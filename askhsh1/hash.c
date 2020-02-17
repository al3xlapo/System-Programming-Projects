#include "hash.h"


void initializeHashTable(hashTable** accTable, int HTsize){ //create a hashTable
	int index = 0;
	(*accTable) = malloc(sizeof(hashTable));
	(*accTable)->data = malloc(HTsize * sizeof(bucket));
	(*accTable)->capacity = HTsize; //store its capacity
	for(index=0 ; index<HTsize; index++){
		bucket* bt = &((*accTable)->data[index]);
		createBucket(&bt); //create buckets
	}
}


int hashFunction(hashTable* accTable, int accId){
	
	return (accId % accTable->capacity); // the hash function is x mod (number of buckets in the array)
}


void linearHashing(hashTable* accTable, int accId){
	int h;
	node* search = NULL;
	bucket* toFill; 
	h = hashFunction(accTable, accId);
	toFill = &accTable->data[h]; //the bucket to receive the node pointer
	if( (search = query(accTable->data[h], accId)) == NULL){ //if the node doesn't exist;
		putInBucket(&toFill, accId);
		printf("success: created %d\n", accId);
	}
	else
		printf("failed: %d already exists\n", accId);
}
	
	
