#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>


//auxiliary list

typedef struct nList{
	int accId;
	struct nList* next;
}nList;

void insertL(nList** l, int accId);
void insertStart(nList** l, int accId);
void deleteL(nList** l);
void printListL(nList* l);
void printListT(nList* l, float cash);
void deleteLast(nList** l);
int countList(nList* l);

//dump list

typedef struct dList{
	int N1, N2;
	float cash;
	struct dList* next;
}dList;

void insertD(dList** l, int N1, int N2, float cash);
void deleteD(dList** l);
void printListD(dList* l, FILE* fp);
void printListLD(nList* l, FILE* fp);

