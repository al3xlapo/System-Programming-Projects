#include "bucket.h"


void createBucket(bucket** data){ 
	(*data)->dtable = malloc(10*sizeof(node*)); //initialize the dynamic table of node pointers
	(*data)->capacity = 10;
	(*data)->used = 0; //the bucket is initially empty
	(*data)->last = 0;
}

void  makeEntry(node** newN, char* accName, int cash){
	*newN = malloc(sizeof(node));
	strcpy((*newN)->accName, accName);
	(*newN)->balance = cash;
	(*newN)->in = NULL;
}

void putInBucket(bucket** bt, char* accName, int cash){
	int tableEntries = (*bt)->capacity;
	int i, added = 0;
	node** newN; //create the new node there
	
	if((*bt)->used < (*bt)->capacity){
		newN = &((*bt)->dtable[(*bt)->used]); //add it to the first free cell in the table of nodes
		makeEntry(newN, accName, cash);
		
	}	
	else{ //the table is full
		(*bt)->dtable = expand((*bt)->dtable, tableEntries);
		(*bt)->capacity *= 2; //the capacity is now double the previous one
		newN = &((*bt)->dtable[(*bt)->used]); //now create the node
		makeEntry(newN, accName, cash);
	}
	(*bt)->used++; //added a new entry
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


node* query(bucket bt, char* accName){
	int i = 0;
	while(i < bt.used){ //only parse to the last filled table cell
		if( !strcmp(bt.dtable[i]->accName, accName) ) //if the node is found return a pointer to that node
			return bt.dtable[i];
		i++;
	}
	return NULL;
}


int queryNull(bucket* bt, char* accName){ //find the element to delete, and NULL-ify the correspondent table cell
	int i = 0;
	while(i < bt->used){ //only parse to the last filled table cell
		if( !strcmp(bt->dtable[i]->accName, accName) ) //if the node is found return a pointer to that node
			return i; //successful search, return the position of the node to be deleted
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
			deleteEdges((*nd)->in);
			(*nd)->in = NULL;
			free(*nd); //free every node
			*nd = NULL;
		}
	}
	free(bt[index].dtable);
}



