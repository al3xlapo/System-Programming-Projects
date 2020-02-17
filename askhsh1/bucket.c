#include "bucket.h"


void createBucket(bucket** data){ 
	(*data)->dtable = malloc(10*sizeof(node*)); //initialize the dynamic table of node pointers
	(*data)->capacity = 10;
	(*data)->used = 0; //the bucket is initially empty
	(*data)->last = 0;
}

void  makeEntry(node** newN, int accId){
	*newN = malloc(sizeof(node));
	(*newN)->accId = accId;
	(*newN)->v = 0;
	(*newN)->p = 0;
	(*newN)->in = NULL;
	(*newN)->out = NULL;
}

void putInBucket(bucket** bt, int accId){
	int tableEntries = (*bt)->capacity;
	int i, added = 0;
	node** newN; //create the new node there
	(*bt)->used++; //about to add an entry
	if((*bt)->used <= (*bt)->capacity){
	//the new node will be added at the first free (NULL pointer) spot in the table
		for(i=0; i < (*bt)->last; i++){  //if there is a free spot before the "last" element in the table, "last" remains
			if( (*bt)->dtable[i] == NULL ){ //the same
				newN = &((*bt)->dtable[i]);
				makeEntry(newN, accId);
				added = 1;
				break;
			}
		}
		if(!added){ //else the new node is added at the end of the existing entries, and "last" is incremented
			newN = &((*bt)->dtable[(*bt)->last]);
			makeEntry(newN, accId);
			(*bt)->last++;
		}
	}
	else{ //the table is full
		(*bt)->dtable = expand((*bt)->dtable, tableEntries);
		(*bt)->capacity *= 2; //the capacity is now double the previous one
		(*bt)->last++;
		newN = &((*bt)->dtable[(*bt)->used - 1]); //now create the node
		makeEntry(newN, accId);
	}
}



node** expand(node** dtable, int capacity){
	int i;
	node** temp;
	temp = realloc(dtable, 2*capacity); //allocate twice the memory already allocated
	if(temp == NULL){ //realloc failed
		free(temp);
		printf("Fault in realloc, exiting\n");
		exit(0);
	}
	else
		dtable = temp; //the dtable now starts at the new address

	return dtable;
}


node* query(bucket bt, int accId){
	int i = 0;
	while(i < bt.last){ //only parse to the last filled table cell
		if(bt.dtable[i] != NULL){ //only check the non-NULL entries in your search
			if( bt.dtable[i]->accId == accId ) //if the node is found return a pointer to that node
				return bt.dtable[i];
		}
		i++;
	}
	return NULL;
}


int queryNull(bucket* bt, int accId){ //find the element to delete, and NULL-ify the correspondent table cell
	int i = 0;
	while(i < bt->last){ //only parse to the last filled table cell
		if(bt->dtable[i] != NULL){ //only check non-NULL entries in your search
			if( bt->dtable[i]->accId == accId ){ //if the node is found return a pointer to that node
				return i; //successful search, return the position of the node to be deleted
			}
		}
		i++;
	}
	return -1; //node didn't exist
}


int deleteBucket(bucket* bt, int index){ //for each dtable cell, delete the node
	int i;
	node** nd;
	for(i=0; i < bt[index].last; i++){
		nd = &bt[index].dtable[i];
		if(*nd != NULL){
			deleteEdges((*nd)->out);
			(*nd)->out = NULL;
			deleteEdges((*nd)->in);
			(*nd)->in = NULL;
			free(*nd); //free every node
			*nd = NULL;
		}
	}
	free(bt[index].dtable);
}


void printBucket(bucket* bt, int index){
	int i;
	node* nd;
	for(i=0; i < bt[index].last; i++){
		nd = bt[index].dtable[i];	
		if(nd != NULL){
			printf("vertex(%d) = ", nd->accId);
			printNeigh(nd->out);
			if(nd->out == NULL)
				printf("\n");
		}
	}
}



void dumpBucket(bucket* bt, int index, nList** l, dList** d){
	int i;
	node* nd;
	for(i=0; i < bt[index].last; i++){
		nd = bt[index].dtable[i];
		if(nd != NULL){
			insertL(l, nd->accId);
			dumpEdge(nd->accId, nd->out, d);
		}
	}
}
			

















	
