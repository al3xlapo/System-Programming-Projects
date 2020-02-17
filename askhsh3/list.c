#include "list.h"


void insertL(nList** l, char* accName){ //insert an element in an nList
	if(*l == NULL){
		*l = malloc(sizeof(nList));
		strcpy((*l)->accName, accName);
		(*l)->next = NULL;
	}
	else{
		while((*l)->next != NULL)
			l = &((*l)->next);
		nList* temp = *l;
		l = &((*l)->next);
		*l = malloc(sizeof(nList));
		strcpy((*l)->accName, accName);
		(*l)->next = NULL;
		temp->next = *l;
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
		printf("%s, ", l->accName);
		l = l->next;
	}
	printf("%s)\n",l->accName);
}


void insertH(hashList** l, int h){

	if(*l == NULL){
		*l = malloc(sizeof(hashList));
		(*l)->h = h;
		(*l)->next = NULL;
	}
	else{
		while( (*l)->next != NULL && h > (*l)->h ) //find where the new element should go
			l = &((*l)->next);
		if( (*l)->next == NULL && h > (*l)->h ){  //create the new node at the end of the list
			hashList* temp = *l;
			l = &((*l)->next);
			*l = malloc(sizeof(hashList));
			(*l)->h = h;
			(*l)->next = NULL;
			temp->next = *l;
		}
		else{ //create the new node inside the list
			hashList* temp = *l;
			*l = malloc(sizeof(hashList));
			(*l)->next = temp;
			(*l)->h = h;
		}
	}	

}


void deleteH(hashList** l){

	hashList* toDel = *l;
	hashList* next;
	while(toDel != NULL){
		next = toDel->next;
		free(toDel);
		toDel = next;
	}
	*l = NULL;
}


void printListH(hashList* l){
	if(l == NULL) return;
	while(l != NULL){
		printf("%d\n", l->h);
		l = l->next;
	}
}




