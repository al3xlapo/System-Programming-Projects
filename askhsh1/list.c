#include "list.h"

//list

void insertL(nList** l, int accId){ //insert an element in an nList
	if(*l == NULL){
		*l = malloc(sizeof(nList));
		(*l)->accId = accId;
		(*l)->next = NULL;
	}
	else{
		while((*l)->next != NULL)
			l = &((*l)->next);
		nList* temp = *l;
		l = &((*l)->next);
		*l = malloc(sizeof(nList));
		(*l)->accId = accId;
		(*l)->next = NULL;
		temp->next = *l;
	}	
}


void insertStart(nList** l, int accId){
	if(*l == NULL){
		*l = malloc(sizeof(nList));
		(*l)->accId = accId;
		(*l)->next = NULL;
	}
	else{
		nList* temp = *l;
		*l = malloc(sizeof(nList));
		(*l)->accId = accId;
		(*l)->next = temp;
	}
}
		

void deleteL(nList** l){ //delete the whole nList
	nList* toDel = *l;
	nList* next;
	while(toDel != NULL){
		next = toDel->next;
		free(toDel);
		toDel = next;
	}
	*l = NULL;
}

void printListL(nList* l){
	if(l == NULL) return;
	printf("(");
	while(l->next != NULL){
		printf("%d, ", l->accId);
		l = l->next;
	}
	printf("%d)\n",l->accId);
}



void printListT(nList* l, float cash){
	if(l == NULL) return;
	printf("(");
	while(l->next != NULL){
		printf("%d, ", l->accId);
		l = l->next;
	}
	printf("%d, %.3f)\n",l->accId, cash);
}



void deleteLast(nList** l){
	nList* toDel = *l;
	nList* prev;
	if(*l == NULL) return;
	while(toDel->next != NULL){
		prev = toDel;
		toDel = toDel->next;
	}
	free(toDel);
	prev->next = NULL;
}


int countList(nList* l){
	int count = 0;
	while(l != NULL){
		count++;
		l = l->next;
	}
}


//dump list


void insertD(dList** l, int N1, int N2, float cash){ //insert an element in an dList
	if(*l == NULL){
		*l = malloc(sizeof(dList));
		(*l)->N1 = N1;
		(*l)->N2 = N2;
		(*l)->cash = cash;
		(*l)->next = NULL;
	}
	else{
		while((*l)->next != NULL)
			l = &((*l)->next);
		dList* temp = *l;
		l = &((*l)->next);
		*l = malloc(sizeof(dList));
		(*l)->N1 = N1;
		(*l)->N2 = N2;
		(*l)->cash = cash;
		(*l)->next = NULL;
		temp->next = *l;
	}	
}


void printListD(dList* l, FILE* fp){
	while(l != NULL){
		fprintf(fp, "addtran %d %d %f\n", l->N1, l->N2, l->cash);
		l = l->next;
	}
}
	
	
void printListLD(nList* l, FILE* fp){
	if(l == NULL) return;
	while(l->next != NULL){
		fprintf(fp, "%d ", l->accId);
		l = l->next;
	}
	fprintf(fp, "%d\n", l->accId);
}
			



void deleteD(dList** l){ //delete the whole dList
	dList* toDel = *l;
	dList* next;
	while(toDel != NULL){
		next = toDel->next;
		free(toDel);
		toDel = next;
	}
	*l = NULL;
}

