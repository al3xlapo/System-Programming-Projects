#include "hash.h"

//condition variables and mutexes
pthread_cond_t nonemptyQ; //queue
pthread_cond_t nonfullQ;
pthread_mutex_t queueLock;

pthread_cond_t nonemptyPool; //thread pool

int* queue; //the queue of socket descriptors
int qSize; //queue max size
int queueCur; //current number of elements in the queue
int NoW; //total worker threads
int thrPool; //number of free worker threads

int mtxNum;
pthread_mutex_t* hashMtx; //hash table mutexes - I'll have one mutex for each group of 3 buckets

hashTable* accTable;


void initializeHashTable(hashTable** accTable){ //create a hashTable
	int index = 0;
	(*accTable) = malloc(sizeof(hashTable));
	(*accTable)->data = malloc(HTsize * sizeof(bucket));
	(*accTable)->capacity = HTsize; //store its capacity
	for(index=0 ; index<HTsize; index++){
		bucket* bt = &((*accTable)->data[index]);
		createBucket(&bt); //create buckets
	}
}


int hashFunction(hashTable* accTable, char* accName){
	
	int digsum = 0, i; //digsum will hold the sum of accName's digits' ASCII codes, and will be used for hashing
	for(i=0; i < strlen(accName); i++)
		digsum += accName[i];
	return (digsum % accTable->capacity); // the hash function is digsum mod (number of buckets in the array)
}


int addAccount(hashTable* accTable, char* accName, int cash, int sock, int delay){
	int h, length;
	node* search = NULL;
	bucket* toFill;
	char msgbuf[MSGSIZE+1], clen;
	h = hashFunction(accTable, accName);
	pthread_mutex_lock(&hashMtx[h/3]); //lock the suitable mutex
	usleep(delay);
	toFill = &accTable->data[h]; //the bucket to receive the node pointer
	if( (search = query(accTable->data[h], accName)) == NULL){ //if the node doesn't exist;
		putInBucket(&toFill, accName, cash);
		sprintf(msgbuf, "Success. Account creation (%s:%d)\n", accName, cash);
		length = strlen(msgbuf)+1;
		clen = length;
		write(sock, &clen, 1);
		write(sock, msgbuf, length);
		pthread_mutex_unlock(&hashMtx[h/3]);
		return 0;
	}
	else{
		sprintf(msgbuf, "Error. Account creation failed (%s:%d)\n", accName, cash);
		length = strlen(msgbuf)+1;
		clen = length;
		write(sock, &clen, 1);
		write(sock, msgbuf, length);
		pthread_mutex_unlock(&hashMtx[h/3]);
		return -1;
	}
}


void mutexLock2(int h1, int h2){

	if(h1/3 == h2/3) //check if they belong to the same mutex group - if so just lock once
		pthread_mutex_lock(&hashMtx[h1/3]);
	else{ //else, always lock the mutex with the "lowest" position in the bucket first
		if(h1 < h2){
			pthread_mutex_lock(&hashMtx[h1/3]);
			pthread_mutex_lock(&hashMtx[h2/3]);
		}
		else{
			pthread_mutex_lock(&hashMtx[h2/3]);
			pthread_mutex_lock(&hashMtx[h1/3]);
		}
	}		
}


void mutexUnlock2(int h1, int h2){

	if(h1/3 == h2/3) //check if they belong to the same mutex group - if so just lock once
		pthread_mutex_unlock(&hashMtx[h1/3]);
	else{ //else, always lock the mutex with the "lowest" position in the bucket first
		if(h1 < h2){
			pthread_mutex_unlock(&hashMtx[h1/3]);
			pthread_mutex_unlock(&hashMtx[h2/3]);
		}
		else{
			pthread_mutex_unlock(&hashMtx[h2/3]);
			pthread_mutex_unlock(&hashMtx[h1/3]);
		}
	}		
}


int addtran(hashTable* accTable, char* acc1, char* acc2, int cash, int sock, int delay){ //link two nodes
	int h1, h2, rem, length, first12;
	node *nd1, *nd2; 
	char msgbuf[MSGSIZE+1], clen;
	h1 = hashFunction(accTable, acc1); //find the bucket of each account id
	h2 = hashFunction(accTable, acc2);
	mutexLock2(h1, h2);
	usleep(delay);
	nd1 = query(accTable->data[h1], acc1); //get the nodes from their respective buckets
	nd2 = query(accTable->data[h2], acc2);
	if(nd1 == NULL || nd2 == NULL){
		sprintf(msgbuf, "Error. Transfer addition failed (%s:%s:%d[:%d])\n", acc1, acc2, (int)cash, delay);
		length = strlen(msgbuf)+1;
		clen = length;
		write(sock, &clen, 1);
		write(sock, msgbuf, length);
		mutexUnlock2(h1, h2);
		return -1;
	}
	else{
		rem = nd1->balance - cash; //rem is the balance of acc1 after acc1 sends the cash to acc2
		if(rem < 0){ //if acc1 doesn't have enough cash
			sprintf(msgbuf, "Error. Transfer addition failed (%s:%s:%d[:%d])\n", acc1, acc2, (int)cash, delay);
			length = strlen(msgbuf)+1;
			clen = length;
			write(sock, &clen, 1);
			write(sock, msgbuf, length);
			mutexUnlock2(h1, h2);
			return -1;
		} //else
		insertEdge(&nd1, &nd2, cash);
		nd1->balance -= cash; //subtract the amount given from the balance
		nd2->balance += cash; //and add it to the other node
		sprintf(msgbuf, "Success. Transfer addition (%s:%s:%d[:%d])\n", acc1, acc2, (int)cash, delay);
		length = strlen(msgbuf)+1;
		clen = length;
		write(sock, &clen, 1);
		write(sock, msgbuf, length);
		mutexUnlock2(h1, h2);
		return 0;
	}
}


hashList* lockHashList(hashTable* accTable, nList* l, int h1){ //lock all the mutexes corresponding to the data we want to access/change

	hashList* hl = NULL, *temp;
	int count = 0, prev, h2;
	
	//add the hashes to the list
	insertH(&hl, h1);
	while(l != NULL){
		h2 = hashFunction(accTable, l->accName);
		insertH(&hl, h2); //add them to the list
		l = l->next;
	}
	//parse the hashList, locking the mutexes corresponding to each hash bucket only oncess
	temp = hl;
	while(temp != NULL){
		if(!count) //first mutex I'm locking
			pthread_mutex_lock(&hashMtx[temp->h/3]);
		else
			if((temp->h)/3 > prev / 3)
				pthread_mutex_lock(&hashMtx[temp->h/3]);
		prev = temp->h;
		temp = temp->next;
		count++;
	}
	return hl;
}


hashList* lockHashListPrint(hashTable* accTable, nList* l){ //same as above, but with one less argument, to be used in printMultiBalance

	hashList* hl = NULL, *temp;
	int count = 0, prev, h2;
	
	//add the hashes to the list
	while(l != NULL){
		h2 = hashFunction(accTable, l->accName);
		insertH(&hl, h2); //add them to the list
		l = l->next;
	}
	//parse the hashList, locking the mutexes corresponding to each hash bucket only oncess
	temp = hl;
	while(temp != NULL){
		if(!count) //first mutex I'm locking
			pthread_mutex_lock(&hashMtx[temp->h/3]);
		else
			if((temp->h)/3 > prev / 3)
				pthread_mutex_lock(&hashMtx[temp->h/3]);
		prev = temp->h;
		temp = temp->next;
		count++;
	}
	return hl;
}

hashList* unlockHashList(hashList* hl){

	int count = 0, prev;
	hashList *temp = hl;
	while(temp != NULL){
		if(!count) //first mutex I'm locking
			pthread_mutex_unlock(&hashMtx[temp->h/3]);
		else
			if((temp->h)/3 > prev / 3)
				pthread_mutex_unlock(&hashMtx[temp->h/3]);
		prev = temp->h;
		temp = temp->next;
		count++;	
	}
	return hl;	
}



int addMultiTran(hashTable* accTable, char* acc1, nList* l, int cash, int sock, int delay){

	nList* first = l;
	node *nd1, *nd2;
	int h1, h2, rem, length;
	hashList* hl=NULL;
	char msgbuf[MSGSIZE+1], clen;
	h1 = hashFunction(accTable, acc1);
	hl = lockHashList(accTable, l, h1); //get the mutexes
	usleep(delay);
	nd1 = query(accTable->data[h1], acc1);
	rem = nd1->balance; //rem will keep track of the remaining balance for each iteration
	if(nd1 == NULL){
		sprintf(msgbuf, "Error. Multi-Transfer addition failed (%s:%d[:%d])\n", acc1, cash, delay);
		length = strlen(msgbuf)+1;
		clen = length;
		write(sock, &clen, 1);
		write(sock, msgbuf, length);
		hl = unlockHashList(hl); //release them
		deleteH(&hl); //delete the hash list
		return -1;
	}
	while(l != NULL){ //for each destination account: check if it exists and if the total sum of money sent is less or equal than acc1's balance
		h2 = hashFunction(accTable, l->accName);
		nd2 = query(accTable->data[h2], l->accName);
		if(nd2 == NULL){
			sprintf(msgbuf, "Error. Multi-Transfer addition failed (%s:%d[:%d])\n", acc1, cash, delay);
			length = strlen(msgbuf)+1;
			clen = length;
			write(sock, &clen, 1);
			write(sock, msgbuf, length);
			hl = unlockHashList(hl);
			deleteH(&hl);
			return -1;
		}
		rem -= cash;
		l = l->next;
	}
	if(rem < 0){
		sprintf(msgbuf, "Error. Multi-Transfer addition failed (%s:%d[:%d])\n", acc1, cash, delay);
		length = strlen(msgbuf)+1;
		clen = length;
		write(sock, &clen, 1);
		write(sock, msgbuf, length);
		unlockHashList(hl);
		deleteH(&hl);
		return -1;
	}
	//everything is ok, do the transfers
	l = first;
	while(l != NULL){
		h2 = hashFunction(accTable, l->accName);
		nd2 = query(accTable->data[h2], l->accName);
		insertEdge(&nd1, &nd2, cash);
		nd1->balance -= cash;
		nd2->balance += cash;
		l = l->next;
	}
	sprintf(msgbuf, "Success. Multi-Transfer addition (%s:%d[:%d])\n", acc1, cash, delay);
	length = strlen(msgbuf)+1;
	clen = length;
	write(sock, &clen, 1);
	write(sock, msgbuf, length);
	unlockHashList(hl);
	deleteH(&hl);
	return 0;
}



void printNodes(hashTable* accTable){

	int i, j;
	for(i=0; i<HTsize; i++){
		bucket* bt = &accTable->data[i];
		for(j=0; j < accTable->capacity; j++){
			node* nd = bt->dtable[j];
			if(nd!=NULL){
				printf("account %s with balance %d\n", nd->accName, nd->balance);
				printEdges(accTable, nd->accName);
			}
		}
	}
}


void printEdges(hashTable* accTable, char* accName){

	int h;
	node *nd;
	h = hashFunction(accTable, accName);
	nd = query(accTable->data[h], accName);
	if(nd == NULL) { printf("The node to be printed doesn't exist\n"); return; }
	edge* in = nd->in;
	printf("FOR %s in edges\n", accName);
	while(in != NULL){
		printf("%f with neigh %s\n", in->cash, in->neigh->accName);
		in = in->next;
	}
}


int printBalance(hashTable* accTable, char* accName, int sock){

	int h, length;
	node *nd;
	char msgbuf[MSGSIZE+1], clen;
	h = hashFunction(accTable, accName);
	pthread_mutex_lock(&hashMtx[h/3]); //lock the suitable mutex
	nd = query(accTable->data[h], accName);
	if(nd == NULL) {
		sprintf(msgbuf, "Error. Balance (%s)\n", accName);
		length = strlen(msgbuf)+1;
		clen = length;
		write(sock, &clen, 1);
		write(sock, msgbuf, length);
		pthread_mutex_unlock(&hashMtx[h/3]); //and release
		return -1; 
	}
	sprintf(msgbuf, "Success. Balance (%s:%d)\n", accName, nd->balance);
	length = strlen(msgbuf)+1;
	clen = length;
	write(sock, &clen, 1);
	write(sock, msgbuf, length);
	pthread_mutex_unlock(&hashMtx[h/3]);
	return 0;
}


int printMultiBalance(hashTable* accTable, nList* l, int sock){

	nList* first = l;
	node *nd;
	hashList* hl=NULL;
	//successLength and errorLength will hold the length the message buffer should have to contain the whole message for error and success respectively
	int h, successLength=1, errorLength=1; 
	char msgbuf[MSGSIZE+1], clen;
	hl = lockHashListPrint(accTable, l);
	while(l != NULL){ //check if every item in the list is a non-NULL node
		h = hashFunction(accTable, l->accName);
		nd = query(accTable->data[h], l->accName);
		if(nd == NULL){ //error in the command
			//first calculate the size of the buffer
			sprintf(msgbuf, "Error. Multi-Balance (");
			errorLength += strlen(msgbuf);
			l = first;
			while(l->next != NULL){
				sprintf(msgbuf, "%s, ", l->accName);
				errorLength += strlen(msgbuf);
				l = l->next;
			}
			sprintf(msgbuf, "%s)\n", l->accName);
			errorLength += strlen(msgbuf);
			
			//fill the buffer and send the data to the client
			char* msgbuffer = malloc(errorLength * sizeof(char));
			
			sprintf(msgbuf, "Error. Multi-Balance (");
			strcpy(msgbuffer, msgbuf);
			l = first;
			while(l->next != NULL){
				sprintf(msgbuf, "%s, ", l->accName);
				strcat(msgbuffer, msgbuf);
				l = l->next;
			}
			sprintf(msgbuf, "%s)\n", l->accName);
			strcat(msgbuffer, msgbuf);
			clen = errorLength;
			write(sock, &clen, 1);
			write(sock, msgbuffer, errorLength);
			free(msgbuffer);
			unlockHashList(hl);
			deleteH(&hl);
			return;
		}
		l = l->next;
	}
	//everything is ok, now parse the whole list again to calculate the length of the msgbuffer
	l = first;
	sprintf(msgbuf, "Success. Multi-Balance (");
	successLength += strlen(msgbuf);
	while(l->next != NULL){
		h = hashFunction(accTable, l->accName);
		nd = query(accTable->data[h], l->accName);
		sprintf(msgbuf, "%s/%d, ", nd->accName, nd->balance);
		successLength += strlen(msgbuf);	
		l = l->next;
	}
	h = hashFunction(accTable, l->accName);
	nd = query(accTable->data[h], l->accName);
	sprintf(msgbuf, "%s/%d)\n", nd->accName, nd->balance);
	successLength += strlen(msgbuf);
	
	//parse it once more to form the message
	char* msgbuffer = malloc(successLength * sizeof(char));
	l = first;
	sprintf(msgbuf, "Success. Multi-Balance (");
	strcpy(msgbuffer, msgbuf);
	while(l->next != NULL){
		h = hashFunction(accTable, l->accName);
		nd = query(accTable->data[h], l->accName);
		sprintf(msgbuf, "%s/%d, ", nd->accName, nd->balance);
		strcat(msgbuffer, msgbuf);	
		l = l->next;
	}
	h = hashFunction(accTable, l->accName);
	nd = query(accTable->data[h], l->accName);
	sprintf(msgbuf, "%s/%d)\n", nd->accName, nd->balance);
	strcat(msgbuffer, msgbuf);
	clen = successLength;
	write(sock, &clen, 1);
	write(sock, msgbuffer, successLength);
	free(msgbuffer);
	unlockHashList(hl);
	deleteH(&hl);
	return;
}



