#include "operations.h"


void createNodes(hashTable* accTable, nList** l){
	while(*l != NULL){ //for each node given
		linearHashing(accTable, (*l)->accId);
		l = &((*l)->next);
	}
}



int addtran(hashTable* accTable, int acc1, int acc2, float cash){ //link two nodes
	int h1, h2;
	node *nd1, *nd2; 
	h1 = hashFunction(accTable, acc1); //find the bucket of each account id
	h2 = hashFunction(accTable, acc2);
	nd1 = query(accTable->data[h1], acc1); //get the nodes from their respective buckets
	nd2 = query(accTable->data[h2], acc2);
	if(nd1 == NULL || nd2 == NULL){
		printf("failed: one of the nodes doesn't exist\n");
		return 1;
	}
	else{
		insertEdge(&nd1, &nd2, cash);
		printf("success: added transaction %d %d with amount %f\n", acc1, acc2, cash);
		return 0;
	}
}


int deltran(hashTable* accTable, int acc1, int acc2){
	int h1, h2;
	node *nd1, *nd2; 
	h1 = hashFunction(accTable, acc1); //find the bucket of each account id
	h2 = hashFunction(accTable, acc2);
	nd1 = query(accTable->data[h1], acc1); //get the nodes from their respective buckets
	nd2 = query(accTable->data[h2], acc2);
	if(nd1 == NULL || nd2 == NULL){
		printf("failed: one of the nodes doesn't exist\n");
		return 1;
	}
	else{
		deleteEdge(&nd1, &nd2);
		printf("success: deleted transaction %d %d\n", acc1, acc2);
		return 0;
	}	

}

void delNodes(hashTable* accTable, nList** l){
	int h, delPos = 0;
	bucket* bt;
	while(*l != NULL){ //for each node given
		h = hashFunction(accTable, (*l)->accId); //search  for the node in the correct bucket
		bt = &(accTable->data[h]);
		delPos = queryNull(bt, (*l)->accId); //search the bucket for the node
		if(delPos < 0)
			printf("failed: %d doesn't exist\n", (*l)->accId);
		else if(delPos >= 0){ //if success >= 0, it will be the cell whose node we'll delete
			if( delNode(&(bt->dtable[delPos])) == 0 ){ //the node has been deleted
				(bt->used)--;
				bt->dtable[delPos] = NULL;
				//if the node was the last one in the table, decrement the "last" variable
				if( delPos == (bt->last - 1) ) 
					(bt->last)--;
				printf("success: deleted %d\n", (*l)->accId);
			}
			else
				printf("failed: couldn't delete %d\n", (*l)->accId);
			
		}
		l = &((*l)->next);
		delPos = -1;
	}
}


void lookup(hashTable* accTable, int mode, int accId){ 
	int h, sum = 0;
	node* nd;
	h = hashFunction(accTable, accId);
	nd = query(accTable->data[h], accId);
	if(nd == NULL){
		printf("failed: %d doesn't exist\n", accId);
		return;
	}
	edge* out = nd->out;
	edge* in = nd->in;
	switch(mode){ //mode = 0 for sum, 1 for in and 2 for out cash
		case 0:
			sum = sumEdges(out) + sumEdges(in);
			printf("success: sum(%d) = %d\n\n", accId, sum);
			break;
		case 1:
			sum = sumEdges(in);
			printf("success: in(%d) = %d\n\n", accId, sum);
			break;
		case 2:
			sum = sumEdges(out);
			printf("success: out(%d) = %d\n\n", accId, sum);
			break;
	}
}	
	

void triangle(hashTable* accTable, int N, float k){
	int h, steps = 3, i;
	node* nd;
	nList* l = NULL;
	h = hashFunction(accTable, N);
	nd = query(accTable->data[h], N);
	if(nd == NULL){
		printf("failed: %d doesn't exist\n", N);
		return;
	}
	triangleSearch(nd, N, k, steps, &l); //search the graph
	if(!switchS)
		printf("triangle(%d, %.3f) doesn't exist\n", N, k);
	switchS = 0; //reinitialize the switch variable for the next triangle function
	printf("\n");
	deleteL(&l);
	reinitializeNodes(accTable); //sets v (visited) and p (to be printed) variables of each graph node to 0 again
}	


void conn(hashTable* accTable, int N1, int N2){
	int h1, h2;
	node *nd1, *nd2;
	nList* l = NULL;
	h1 = hashFunction(accTable, N1);
	h2 = hashFunction(accTable, N2);
	nd1 = query(accTable->data[h1], N1);
	nd2 = query(accTable->data[h2], N2);
	if(nd1 == NULL || nd2 == NULL){
		printf("failed: one of the node IDs doesn't exist\n");
		return;
	}
	connSearch(nd1, N1, N2, &l); //search the graph
	if(!switchS)
		printf("conn(%d, %d) not found\n", N1, N2);
	switchS = 0;
	printf("\n");
	reinitializeNodes(accTable); //sets v (visited) and p (to be printed) variables of each graph node to 0 again
	deleteL(&l);
}


void allcycles(hashTable* accTable, int N){
	int h, steps = 3;
	node* nd;
	nList* l = NULL;
	h = hashFunction(accTable, N);
	nd = query(accTable->data[h], N);
	if(nd == NULL){
		printf("failed: %d doesn't exist\n", N);
		return;
	}
	allcyclesSearch(nd, N, steps, &l);
	if(!switchS)
		printf("no cycle found for %d\n", N);
	switchS = 0;
	deleteL(&l);
	reinitializeNodes(accTable);
	printf("\n");
}



void traceflow(hashTable* accTable, int N, int L){
	int h;
	node* nd;
	nList* list = NULL;
	h = hashFunction(accTable, N);
	nd = query(accTable->data[h], N);
	if(nd == NULL){
		printf("failed: %d doesn't exist\n", N);
		return;
	}
	traceflowSearch(nd, N, L, L, 0, &list);
	switchS = 0;
	deleteL(&list);
	printf("\n");
	reinitializeNodes(accTable);

}


void bye(hashTable* accTable){
	int i;
	bucket* bt;
	for(i=0; i < accTable->capacity; i++)
		deleteBucket(accTable->data, i); //delete all buckets
	printf("success: cleaned memory\n");
}


void printG(hashTable* accTable){
	int i;
	for(i=0; i < accTable->capacity; i++)
		printBucket(accTable->data, i);
	printf("\n");
}


void dump(hashTable* accTable, FILE* fp){
	int i;
	nList* l = NULL;
	dList* d = NULL;
	for(i=0; i < accTable->capacity; i++)
		dumpBucket(accTable->data, i, &l, &d);
	printf("\n");
	fprintf(fp, "createnodes ");
	printListLD(l, fp);
	printListD(d, fp);
	deleteL(&l);
	deleteD(&d);
}


void reinitializeNodes(hashTable* accTable){ //sets v (visited) and p (to be printed) variables of each graph node to 0 again
	int i, j;
	bucket bt;
	for(i=0; i < accTable->capacity; i++){
		bt = accTable->data[i];
		for(j=0; j < bt.last; j++) //parse each dynamic table, skipping NULL pointers
			if(bt.dtable[j] != NULL){
				bt.dtable[j]->v = 0;
				bt.dtable[j]->p = 0;
			}
	}
}



int inspect(char* N){
	int i;
	if(strlen(N) != 7)
		return 0;
	for(i=0; i < strlen(N); i++)
		if(!isdigit(N[i]))
			return 0;
	return 1;
}

//auxiliary

void printEdges(hashTable* accTable, int accId){
	int h;
	node *nd;
	h = hashFunction(accTable, accId);
	nd = query(accTable->data[h], accId);
	if(nd == NULL) { printf("The node to be printed doesn't exist\n"); return; }
	edge* out = nd->out;
	edge* in = nd->in;
	printf("FOR %d out edges\n", accId);
	while(out != NULL){
		printf("%f with neigh %d\n", out->cash, out->neigh->accId);
		out = out->next;
	}
	printf("FOR %d in edges\n", accId);
	while(in != NULL){
		printf("%f with neigh %d\n", in->cash, in->neigh->accId);
		in = in->next;
	}
}

	




