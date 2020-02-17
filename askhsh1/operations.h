#include "hash.h"

//functions

extern int switchS;

void createNodes(hashTable* accTable, nList** l);
int addtran(hashTable* accTable, int acc1, int acc2, float cash);
int deltran(hashTable* accTable, int acc1, int acc2);
void printEdges(hashTable* accTable, int accId);
void delNodes(hashTable* accTable, nList** l);
void lookup(hashTable* accTable, int mode, int accId);
void triangle(hashTable* accTable, int N, float k);
void reinitializeNodes(hashTable* accTable);
void conn(hashTable* accTable, int N1, int N2);
void allcycles(hashTable* accTable, int N);
void traceflow(hashTable* accTable, int N, int L);
int inspect(char* N);
void bye(hashTable* accTable);
void printG(hashTable* accTable);
void dump(hashTable* accTable, FILE* fp);
