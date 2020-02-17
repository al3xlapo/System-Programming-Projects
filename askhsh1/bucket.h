#include "graph.h"

typedef struct bucket{
	node** dtable; //dynamic table of pointers to graph nodes
	int capacity;
	int last; //last filled node in the bucket
	int used;
}bucket;


void createBucket(bucket** data);
void putInBucket(bucket** bt, int accId);
node** expand(node** dtable, int capacity);
node* query(bucket bt, int accId);
int queryNull(bucket* bt, int accId);
int deleteBucket(bucket* bt, int index);
void printBucket(bucket* bt, int index);
void dumpBucket(bucket* bt, int index, nList** l, dList** d);
