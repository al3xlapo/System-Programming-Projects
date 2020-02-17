#include "worker.h"


int main(int argc, char** argv){

	int go, port, i, err, masterSocket, newSocket;
	accTable = NULL;
	initializeHashTable(&accTable);

	//create the hash table mutexes, I've set a mutex for every 3 buckets, so I can get each bucket's mutex(for example for bucket number x) by doing x/3
	mtxNum = HTsize / 3;
	if( mtxNum < (HTsize / 3) ) mtxNum++; //add a mutex if there are still some tables left without one
	hashMtx = malloc(mtxNum * sizeof(pthread_mutex_t)); 

	//parse commandline arguments
	while( (go = getopt(argc, argv, "p:s:q:")) != -1){
		switch(go){
			case 'p':
				port = atoi(optarg); //port number
				break;
			case 's':
				NoW = atoi(optarg); //number of total workers (thread pool size)
				thrPool = NoW; //all workers are available at first
				break;
			case 'q':
				qSize = atoi(optarg); //queue size
				queue = malloc(qSize * sizeof(int));
				queueCur = 0; //the queue is empty at first
				break;
		}
	}

	//initialize condition variables, mutexes
	pthread_mutex_init(&queueLock, NULL);
	pthread_cond_init(&nonemptyQ, NULL);
	pthread_cond_init(&nonfullQ, NULL);
	pthread_cond_init(&nonemptyPool, NULL);
	for(i = 0; i < mtxNum; i++)
		pthread_mutex_init(&hashMtx[i], NULL);

	//create master/administration socket
	struct sockaddr_in server, client;
	socklen_t clientlen = sizeof(client);
	masterSocket = socket(AF_INET, SOCK_STREAM, 0);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(port);
	if( bind(masterSocket, (struct sockaddr*) &server, sizeof(server)) < 0){
		perror("bind");
        	exit(1);
	}
	listen(masterSocket, 5);

	//create threads
	pthread_t* threads = malloc(NoW * sizeof(pthread_t)); //table of thread ids
	for(i=0; i<NoW; i++){
		if( err = pthread_create(&threads[i], NULL, workerThread, NULL) ){
			perror("pthread_create");
			exit(1);
		}
	}
	

	while(1){  //the server is running forever
		//listen at masterSocket for connections and put them in the queue as they come
		if( (newSocket = accept(masterSocket, (struct sockaddr*)&client, &clientlen)) > 0){ 
			pthread_mutex_lock(&queueLock); //get the queue
			while(queueCur >= qSize) //full queue, wait till it's nonfull
				pthread_cond_wait(&nonfullQ, &queueLock);	 
			queue[queueCur] = newSocket;
			queueCur++; //add an element
			pthread_mutex_unlock(&queueLock); //release it
			pthread_cond_broadcast(&nonemptyQ); //the queue isn't empty
		}
		else{ perror("accept"); break; } //quit if there's an error	
	}

	close(masterSocket);
	exit(0);
}

