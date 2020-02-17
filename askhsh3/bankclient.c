#include "hash.h"


#define MSGSIZE 200


int main(int argc, char** argv){

	FILE *stream, *op = NULL;
	char commandFile[100], *msgbuf, nWrites[10], clen;
	char* hostName = malloc(100 * sizeof(char));
	size_t lsize = 900;
	int i, go, port, sock, gl, writes, reads, length, sleepTime = 0;
	char* line = calloc(lsize, sizeof(char));
	char* token = calloc(30, sizeof(char));
	char* command = calloc(30, sizeof(char));

	while( (go = getopt(argc, argv, "h:p:i:")) != -1){
		switch(go){
			case 'h':
				strcpy(hostName, optarg);
				break;
			case 'p':
				port = atoi(optarg); //port number
				break;
			case 'i':
				strcpy(commandFile, optarg);
				if((op=fopen(commandFile, "r")) == NULL){
					printf("failed: the specified filename for operations file doesn't exist\n");
					return 1;
				}
				break;
		}
	}

	//block ctrl+c, only exit by "exit" command
	sigset_t block_mask;
	sigemptyset(&block_mask);
	sigaddset(&block_mask, SIGINT);
	sigprocmask(SIG_BLOCK, &block_mask, NULL);
	

	//connect to server
	struct sockaddr_in server;
	struct sockaddr* serverptr = (struct sockaddr*)&server;
	struct hostent* host;
	if( (host = gethostbyname(hostName)) == NULL){
		perror("gethostbyname");
		exit(1);
	}
	server.sin_family = AF_INET;
	memcpy(&server.sin_addr, host->h_addr, host->h_length);
	server.sin_port = htons(port);
	if (( sock = socket ( AF_INET , SOCK_STREAM , 0) ) < 0){
		perror("socket") ;
		exit(1);
	}
	if( connect (sock , (struct sockaddr *)&server , sizeof(server)) < 0 ){
		perror("connect");
		exit(1);
	}

	//connected, start receiving commands from stdin or file
	if(op != NULL){ //read from file first if it exists
		stream=op;
		printf("Reading from operations file...\n\n");
	}
	else{
		stream=stdin;
		printf("Waiting for commands...\n\n");
	}
	while( (gl = getline(&line, &lsize, stream) ) != -1 ){
		if( (strlen(line) >= 1) && line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = '\0';
		
		sscanf(line, "%s", command); //find what the command is, and send the actual message
		//ignore unknown commands			
		if( strcmp(command, "add_account") && strcmp(command, "add_transfer") && strcmp(command, "add_multi_transfer") && strcmp(command, "print_balance") && 
			strcmp(command, "print_multi_balance") && strcmp(command, "sleep") && strcmp(command, "exit") )
		{
			printf("Unknown command\n\n");
			continue; //discard unknown commands
		}
		if(sleepTime){ usleep(sleepTime); sleepTime = 0; }
		
		if(!strcmp(command, "add_account")){
			length = strlen(line)+1;
			clen = length;
			if( write(sock, &clen, 1) < 0) //first send the length of the input to the worker
				exit(-1);
			if( (writes = write(sock, line, length)) < 0) //send the command
				exit(-1);
			if( (reads = read(sock, &clen, 1)) < 0) //read and calculate the size of the "answer"
				exit(-1);
			length = clen;
			msgbuf = malloc(length * sizeof(char));
			if( (reads = read(sock, msgbuf, length)) >= 0)
				printf("%s", msgbuf);
		}
		else if(!strcmp(command, "add_transfer")){
			length = strlen(line)+1;
			clen = length;
			if( write(sock, &clen, 1) < 0) //first send the length of the input to the worker
				exit(-1);
			if( (writes = write(sock, line, length)) < 0) //send the command
				exit(-1);
			if( (reads = read(sock, &clen, 1)) < 0) //read and calculate the size of the "answer"
				exit(-1);
			length = clen;
			msgbuf = malloc(length * sizeof(char));
			if( (reads = read(sock, msgbuf, length)) >= 0)
				printf("%s", msgbuf);
		}
		else if(!strcmp(command, "add_multi_transfer")){
			length = strlen(line)+1;
			clen = length;
			if( write(sock, &clen, 1) < 0) //first send the length of the input to the worker
				exit(-1);
			if( (writes = write(sock, line, length)) < 0) //send the command
				exit(-1);
			if( (reads = read(sock, &clen, 1)) < 0) //read and calculate the size of the "answer"
				exit(-1);
			length = clen;
			msgbuf = malloc(length * sizeof(char));
			if( (reads = read(sock, msgbuf, length)) >= 0)
				printf("%s", msgbuf);
		}
		else if(!strcmp(command, "print_balance")){
			if( write(sock, &clen, 1) < 0)
				exit(-1);
			if( (writes = write(sock, line, length)) < 0)
				exit(-1);
			if( (reads = read(sock, &clen, 1)) < 0) //read and calculate the size of the "answer"
				exit(-1);
			length = clen;
			msgbuf = malloc(length * sizeof(char));
			if( (reads = read(sock, msgbuf, length)) >= 0)
				printf("%s", msgbuf);
		}
		else if(!strcmp(command, "print_multi_balance")){
			length = strlen(line)+1;
			clen = length;
			if( write(sock, &clen, 1) < 0) //first send the length of the input to the worker
				exit(-1);
			if( (writes = write(sock, line, length)) < 0) //send the command
				exit(-1);
			if( (reads = read(sock, &clen, 1)) < 0) //read and calculate the size of the "answer"
				exit(-1);
			length = clen;
			msgbuf = malloc(length * sizeof(char));
			if( (reads = read(sock, msgbuf, length)) >= 0)
				printf("%s", msgbuf);
		}
		else if(!strcmp(command, "sleep")){
			sscanf(line, "%*s %d", &sleepTime);
			continue;
		}
		else if(!strcmp(command, "exit")){
			length = strlen(line)+1;
			clen = length;
			if( write(sock, &clen, 1) < 0) //first send the length of the input to the worker
				exit(-1);
			if( (writes = write(sock, line, length)) < 0) //send the command
				exit(-1);
			free(line); free(command); free(token); free(hostName);
			if(op != NULL) fclose(op);
			exit(0);
		}
		free(msgbuf);
		fflush(stdout);
	}

	//if it was reading from input it will exit
	if(!op){
		free(line); free(token); free(command); free(hostName);
		exit(0); 
	}
	//if it was reading from a file, it will start reading from input
	fclose(op);
	stream=stdin;
	printf("Waiting for commands...\n\n");

	while( (gl = getline(&line, &lsize, stream) ) != -1 ){
		printf("got in\n");
		if( (strlen(line) >= 1) && line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = '\0';
		
		sscanf(line, "%s", command); //find what the command is, and send the actual message
		//ignore unknown commands			
		if( strcmp(command, "add_account") && strcmp(command, "add_transfer") && strcmp(command, "add_multi_transfer") && strcmp(command, "print_balance") && 
			strcmp(command, "print_multi_balance") && strcmp(command, "sleep") && strcmp(command, "exit") )
		{
			printf("Unknown command\n\n");
			continue; //discard unknown commands
		}
		if(sleepTime){ usleep(sleepTime); sleepTime = 0; }
		
		if(!strcmp(command, "add_account")){
			length = strlen(line)+1;
			clen = length;
			if( write(sock, &clen, 1) < 0) //first send the length of the input to the worker
				exit(-1);
			if( (writes = write(sock, line, length)) < 0) //send the command
				exit(-1);
			if( (reads = read(sock, &clen, 1)) < 0) //read and calculate the size of the "answer"
				exit(-1);
			length = clen;
			msgbuf = malloc(length * sizeof(char));
			if( (reads = read(sock, msgbuf, length)) >= 0)
				printf("%s", msgbuf);
		}
		else if(!strcmp(command, "add_transfer")){
			length = strlen(line)+1;
			clen = length;
			if( write(sock, &clen, 1) < 0) //first send the length of the input to the worker
				exit(-1);
			if( (writes = write(sock, line, length)) < 0) //send the command
				exit(-1);
			if( (reads = read(sock, &clen, 1)) < 0) //read and calculate the size of the "answer"
				exit(-1);
			length = clen;
			msgbuf = malloc(length * sizeof(char));
			if( (reads = read(sock, msgbuf, length)) >= 0)
				printf("%s", msgbuf);
		}
		else if(!strcmp(command, "add_multi_transfer")){
			length = strlen(line)+1;
			clen = length;
			if( write(sock, &clen, 1) < 0) //first send the length of the input to the worker
				exit(-1);
			if( (writes = write(sock, line, length)) < 0) //send the command
				exit(-1);
			if( (reads = read(sock, &clen, 1)) < 0) //read and calculate the size of the "answer"
				exit(-1);
			length = clen;
			msgbuf = malloc(length * sizeof(char));
			if( (reads = read(sock, msgbuf, length)) >= 0)
				printf("%s", msgbuf);
		}
		else if(!strcmp(command, "print_balance")){
			if( write(sock, &clen, 1) < 0)
				exit(-1);
			if( (writes = write(sock, line, length)) < 0)
				exit(-1);
			if( (reads = read(sock, &clen, 1)) < 0) //read and calculate the size of the "answer"
				exit(-1);
			length = clen;
			msgbuf = malloc(length * sizeof(char));
			if( (reads = read(sock, msgbuf, length)) >= 0)
				printf("%s", msgbuf);
		}
		else if(!strcmp(command, "print_multi_balance")){
			length = strlen(line)+1;
			clen = length;
			if( write(sock, &clen, 1) < 0) //first send the length of the input to the worker
				exit(-1);
			if( (writes = write(sock, line, length)) < 0) //send the command
				exit(-1);
			if( (reads = read(sock, &clen, 1)) < 0) //read and calculate the size of the "answer"
				exit(-1);
			length = clen;
			msgbuf = malloc(length * sizeof(char));
			if( (reads = read(sock, msgbuf, length)) >= 0)
				printf("%s", msgbuf);
		}
		else if(!strcmp(command, "sleep")){
			sscanf(line, "%*s %d", &sleepTime);
			continue;
		}
		else if(!strcmp(command, "exit")){
			length = strlen(line)+1;
			clen = length;
			if( write(sock, &clen, 1) < 0) //first send the length of the input to the worker
				exit(-1);
			if( (writes = write(sock, line, length)) < 0) //send the command
				exit(-1);
			free(line); free(command); free(token); free(hostName);
			exit(0);
		}
		free(msgbuf);
		fflush(stdout);
	}		
}




