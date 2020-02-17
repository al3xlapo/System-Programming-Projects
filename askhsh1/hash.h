#include "bucket.h"

typedef struct hashTable{
	bucket* data; //the table itself
	int capacity;
}hashTable;
	

void initializeHashTable(hashTable** accTable, int HTsize);
int hashFunction(hashTable* accTable, int accId);
void linearHashing(hashTable* accTable, int accId);

