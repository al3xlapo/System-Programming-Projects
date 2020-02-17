#include "bucket.h"

#define HTsize 300
#define MSGSIZE 200


typedef struct hashTable{
	bucket* data; //the table itself
	int capacity;
}hashTable;

extern pthread_cond_t nonemptyQ;
extern pthread_cond_t nonfullQ;
extern pthread_cond_t nonemptyPool;
extern pthread_mutex_t queueLock;
extern pthread_mutex_t heapLock;
extern int* queue;
extern int qSize;
extern int queueCur;
extern int NoW;
extern int thrPool;
extern hashTable* accTable;
extern int mtxNum;
extern pthread_mutex_t* hashMtx;
	

void initializeHashTable(hashTable** accTable);
int hashFunction(hashTable* accTable, char* accName);
int addAccount(hashTable* accTable, char* accName, int cash, int sock, int delay);
int addtran(hashTable* accTable, char* acc1, char* acc2, int cash, int sock, int delay);
int addMultiTran(hashTable* accTable, char* acc1, nList* l, int cash, int sock, int delay);
int printBalance(hashTable* accTable, char* accName, int sock);
int printMultiBalance(hashTable* accTable, nList* l, int sock);
void printEdges(hashTable* accTable, char* accName);
void bye(hashTable* accTable);
void mutexLock2(int h1, int h2);
void mutexUnlock2(int h1, int h2);
hashList* lockHashListPrint(hashTable* accTable, nList* l);
hashList* lockHashList(hashTable* accTable, nList* l, int h1);
hashList* unlockHashList(hashList* hl);
