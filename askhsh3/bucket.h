#include "graph.h"


typedef struct bucket{
	node** dtable; //dynamic table of pointers to graph nodes
	int capacity;
	int last; //last filled node in the bucket
	int used;
}bucket;


void createBucket(bucket** data);
void putInBucket(bucket** bt, char* accName, int cash);
node** expand(node** dtable, int capacity);
node* query(bucket bt, char* accName);
int queryNull(bucket* bt, char* accName);
int deleteBucket(bucket* bt, int index);
