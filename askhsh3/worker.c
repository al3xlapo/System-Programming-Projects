#include "worker.h"


int releaseFlag = 0;
pthread_mutex_t rel = PTHREAD_MUTEX_INITIALIZER; 

void release(int signum){

	pthread_mutex_lock(&rel);
	releaseFlag = 1;
	pthread_mutex_unlock(&rel);
}


void* workerThread(void* arg){
	
	int sock, i, *id = arg, reads, cash, delay, length;
	char  command[100], accName[100], accSource[100], accTarget[100], wrong[30], clen;

	if ( pthread_detach(pthread_self())) { //detach thread
		perror( "pthread_detach") ;
		exit (1) ;
	}
	
	nList* l = NULL;
	char *token, *saveptr;
	token = malloc(100*sizeof(char));

	//set up a signal handler so as when a client exits, the worker won't receive SIGPIPE and crash, but instead will try to get another from the queue
	struct sigaction move;
	move.sa_handler = release;
	move.sa_flags = 0;
	if(sigaction(SIGPIPE, &move, NULL)){
		perror("sigaction worker\n");
		exit(1);
	}

	while(1){
		//first, get the client socket descriptor from the queue
		pthread_mutex_lock(&queueLock); //get the lock
		while(queueCur <= 0){ //wait if the queue is empty
			pthread_cond_wait(&nonemptyQ, &queueLock);
		}
		sock = queue[0];
		for(i=1; i<queueCur; i++){
			queue[i-1] = queue[i];
			queue[i] = 0;
		}
		queueCur--; //consumed a queue element
		pthread_mutex_unlock(&queueLock); //release it
		pthread_cond_broadcast(&nonfullQ); //the queue isn't full
		releaseFlag = 0; //it can enter the loop with the new client

		//got the client descriptor, start receiving commands
		while(!releaseFlag){
			if ( (reads = read(sock, &clen, 1)) < 0 ) //read the size of the command sent by the client
				exit(-1);
			length = clen+1;
			char *msgbuf = calloc((length+1), sizeof(char)); //allocate as much space as it's needed
			
			if ( (reads = read(sock, msgbuf, length)) < 0 ) //read the actual command
				exit(-1);
			sscanf(msgbuf, "%s", command);
			if(!strcmp(command, "add_account")){
				//skip the command and get the first argument
				token = strtok_r(msgbuf, " ", &saveptr);
				token = strtok_r(NULL, " ", &saveptr);
				if(token!=NULL)
					cash = atoi(token); //get cash
				else{
					sprintf(wrong, "Wrong command\n");
					length = strlen(wrong);
					clen = length;
					write(sock, &clen, 1);
					write(sock, wrong, length);
					free(msgbuf);
					continue;
				}  
				token = strtok_r(NULL, " ", &saveptr);
				if(token!=NULL) //get account name
					strcpy(accName, token);
				else{
					sprintf(wrong, "Wrong command\n");
					length = strlen(wrong);
					clen = length;
					write(sock, &clen, 1);
					write(sock, wrong, length);
					free(msgbuf);
					continue;
				}
				token = strtok_r(NULL, " ", &saveptr);
				if(token != NULL)
					delay = atoi(token);
				addAccount(accTable, accName, cash, sock, delay);
			}
			else if(!strcmp(command, "add_transfer")){
				token = strtok_r(msgbuf, " ", &saveptr);
				token = strtok_r(NULL, " ", &saveptr);
				if(token!=NULL)
					cash = atoi(token); //get cash
				else{
					sprintf(wrong, "Wrong command\n");
					length = strlen(wrong)+1;
					clen = length;
					write(sock, &clen, 1);
					write(sock, wrong, length);
					free(msgbuf);
					continue;
				}  
				token = strtok_r(NULL, " ", &saveptr);
				if(token!=NULL) //get source name
						strcpy(accSource, token);
				else{
					sprintf(wrong, "Wrong command\n");
					length = strlen(wrong)+1;
					clen = length;
					write(sock, &clen, 1);
					write(sock, wrong, length);
					free(msgbuf);
					continue;
				}
				token = strtok_r(NULL, " ", &saveptr);
				if(token!=NULL) //get target name
					strcpy(accTarget, token);
				else{
					sprintf(wrong, "Wrong command\n");
					length = strlen(wrong)+1;
					clen = length;
					write(sock, &clen, 1);
					write(sock, wrong, length);
					free(msgbuf);
					continue;
				}
				token = strtok_r(NULL, " ", &saveptr);
					
				if(token != NULL)
					delay = atoi(token);
				addtran(accTable, accSource, accTarget, cash, sock, delay);
			}
			else if(!strcmp(command, "add_multi_transfer")){
				token = strtok_r(msgbuf, " ", &saveptr);
				token = strtok_r(NULL, " ", &saveptr);
				if(token!=NULL)
					cash = atoi(token); //get cash
				else{
					sprintf(wrong, "Wrong command\n");
					length = strlen(wrong)+1;
					clen = length;
					write(sock, &clen, 1);
					write(sock, wrong, length);
					free(msgbuf);
					continue;
				}  
				token = strtok_r(NULL, " ", &saveptr);
				if(token!=NULL) //get source name
					strcpy(accSource, token);
				else{
					sprintf(wrong, "Wrong command\n");
					length = strlen(wrong)+1;
					clen = length;
					write(sock, &clen, 1);
					write(sock, wrong, length);
					free(msgbuf);
					continue;
				}
					token = strtok_r(NULL, " ", &saveptr);
				//get each account name given, stop only if you meet the "delay" argument, or reach the end
				while(token!=NULL && !isdigit(token[0])){ 
					strcpy(accName, token);
					insertL(&l, token); 
					token = strtok_r(NULL, " ", &saveptr);
				}
				if(token != NULL)
					delay = atoi(token);
				addMultiTran(accTable, accSource, l, cash, sock, delay);
				deleteL(&l);
			}
			else if(!strcmp(command, "print_balance")){
				token = strtok_r(msgbuf, " ", &saveptr);
				token = strtok_r(NULL, " ", &saveptr);
				if(token!=NULL)
					strcpy(accName, token); //get account name
				else{
					sprintf(wrong, "Wrong command\n");
					length = strlen(wrong)+1;
					clen = length;
					write(sock, &clen, 1);
					write(sock, wrong, length);
					free(msgbuf);
					continue;
				}  
				printBalance(accTable, accName, sock);
			
			}
			else if(!strcmp(command, "print_multi_balance")){
				token = strtok_r(msgbuf, " ", &saveptr);
				token = strtok_r(NULL, " ", &saveptr);
				while(token!=NULL){ //get each account name given
					strcpy(accName, token);
					insertL(&l, token); 
					token = strtok_r(NULL, " ", &saveptr);
				}
				printMultiBalance(accTable, l, sock);
				deleteL(&l);
			}
			else if(!strcmp(command, "exit")){
				free(msgbuf);
				close(sock);
				break;
			}
			free(msgbuf);
			fflush(stdout);
		}
	}
	return;
}







