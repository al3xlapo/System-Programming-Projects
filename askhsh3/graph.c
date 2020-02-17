#include "graph.h"


edge* createEdge(edge* e, float cash, node* neigh){
	e = malloc(sizeof(edge)); //create edge
	e->cash = cash;	//initialize it
	e->next = NULL;
	e->neigh = neigh;
}


void insertEdge(node** source, node** target, float cash){ //add an outward edge to the source and an inward to the target
	int exists = 0;
	edge** eIn = &(*target)->in;

	if(*eIn == NULL) //if there aren't any edges(transfers) yet
		 *eIn = createEdge(*eIn, cash, *source);
	else{		 
		//parse to the end of the edge list while checking if the edge we want to add already exists
		while((*eIn)->next != NULL){
			if( !strcmp((*eIn)->neigh->accName, (*source)->accName) ){ //if it already exists
				exists = 1;
				break;
			}
			eIn = &((*eIn)->next);
		}
		if(exists){
			exists = 0;
			(*eIn)->cash += cash; //update the existing edge
		}
		else{ //check the last edge, if it's different, create the edge at the end of the list
			if( !strcmp((*eIn)->neigh->accName, (*source)->accName) )
				(*eIn)->cash += cash;
			else{
				edge* tempI = *eIn;
				eIn = &((*eIn)->next);
				*eIn = createEdge(*eIn, cash, *source);
				tempI->next = *eIn;
			}
		}
	} 		
}


int delNode(node** n){
	if((*n)->in == NULL){
		free(*n);
		return 0;
	}
	return 1;
}


void deleteEdges(edge* e){
	edge* temp;
	while(e!=NULL){
		temp=e;
		e = e->next;
		free(temp);
	}
}

