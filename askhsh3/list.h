#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>
#include <signal.h>

//auxiliary list

typedef struct nList{
	char accName[200];
	struct nList* next;
}nList;

void insertL(nList** l, char* accName);
void insertStart(nList** l, char* accName);
void deleteL(nList** l);
void printListL(nList* l);
void printListT(nList* l, float cash);
void deleteLast(nList** l);
int countList(nList* l);

//hashList, to be used for locking and unlocking mutexes in add_multi_tran and print_multi_balance
//it is a list sorted in ascending order of hash numbers, eg 1->3->4->37-> ... 

typedef struct hashList{
	int h;
	struct hashList* next;
}hashList;

//no need for sorting function, insertH will make sure each new element is inserted in the correct spot
void insertH(hashList** l, int h);
void deleteH(hashList** l);
void printListH(hashList* l);
