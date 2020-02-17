#include "boardpost.h"


int main(int argc, char** argv){

	FILE *fpid=NULL, *sendf=NULL;	
	int fdOw, fdOr, gl, writeo, reado, pidServer, id, csize, r;
	size_t lsize = 200;
	char *line=NULL, *metaline=NULL, *othersw=NULL, *othersr=NULL, *path=NULL, *spid=NULL,  pidSer[10], *token=NULL, *fname=NULL, 			*csizeS=NULL, *msgbufo=NULL, *command=NULL;
	
	line = calloc(lsize, sizeof(char));
	token = calloc(30, sizeof(char));
	fname = calloc(50, sizeof(char)); //get filename
	csizeS = calloc(70, sizeof(char));
	msgbufo = calloc((MSGSIZE+1), sizeof(char));
	command = calloc(30, sizeof(char));
	metaline = calloc(lsize+100, sizeof(char));
	
	if(argc != 2){
		printf("Wrong arguments, execution format is ./boardpost <path>\n");
		exit(1);
	}

	//boardpost must ignore SINGINT (ctrl + c)
  	sigset_t block_mask;
	sigemptyset(&block_mask);
	sigaddset (&block_mask, SIGINT);
	if(sigprocmask(SIG_SETMASK, &block_mask, NULL) < 0){
		perror("sigprocmask user");
		exit(1);
	}

	//figure out path and fifo name
	//**boardpost writes where server reads and reads where server writes
	path = calloc((strlen(argv[1])+1), sizeof(char));	
	strcpy(path, argv[1]);
	othersw = calloc((strlen(path)+11), sizeof(char));
	othersr = calloc((strlen(path)+11), sizeof(char));
	spid = calloc((strlen(path)+11), sizeof(char));
	
	strcpy(othersr, path);
	strcpy(othersw, path);
	strcpy(spid, path);
	if(path[strlen(path) - 1] != '/'){
		strcat(othersr, "/_othersr");
		strcat(spid, "/_pid");
		strcat(othersw, "/_othersw");
	}
	else{
		strcat(othersr, "_othersr");
		strcat(othersw, "_othersw");
		strcat(spid, "_pid");
	}
	if( (fpid = fopen(spid, "r")) == NULL ) {
		printf( "\nServer is down\n");
		exit(1);
	}
	if( (fdOr = open(othersw, O_RDWR | O_NONBLOCK)) == -1 ) {
		printf( "Server is down\n");
		exit(1);
	}
	if( (fdOw = open(othersr, O_RDWR | O_NONBLOCK)) == -1 ) { //open _othersw
			printf( "Server is down\n");
			exit(1);
		}
	
	
	fseek(fpid, SEEK_SET, 0);
	fscanf(fpid, "%s", pidSer); //read server's pid from _pid file
	pidServer = atoi(pidSer);
	fclose(fpid);
	

	//send each "line" - command given in stdin to the server process
	while( (gl = getline(&line, &lsize, stdin) ) != -1 ){
		if( (strlen(line) >= 1) && line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = '\0';
		sscanf(line, "%s", command);
		
		if(!strcmp(command, "send")){ //if the command is "send"
			token = strtok(line, " ");
			strcpy(metaline, token);
			strcat(metaline, " ");
			if(token!=NULL) 
				token = strtok(NULL, " ");
			else{
				printf("wrong command\n");
				continue;
			}  
			strcat(metaline, token);
			strcat(metaline, " ");
			id = atoi(token); //get channel id
			if(token!=NULL) 
				token = strtok(NULL, " ");
			else{
				printf("wrong command\n");
				continue;
			}  
			strcat(metaline, token);
			strcat(metaline, " ");
			strcpy(fname, token);
			if((sendf = fopen(fname, "r")) == NULL){
				printf("This file doesn't exist\n");
				free(metaline);
				continue;
			}
			fseek (sendf, 0, SEEK_END); //calculate the size of the file in bytes
			csize = ftell(sendf);
			sprintf(csizeS, "%d", csize);
			strcat(metaline, csizeS); //and add it at the end of the data sent to the server 
			writeo = write(fdOw, metaline, lsize);
			memset(metaline, 0, lsize+100);
			fseek(sendf, 0, SEEK_SET);
			usleep(10000);
			kill(pidServer, SIGCONT); //let server know you're ready to write
			
			while((r = fread(msgbufo, 1, MSGSIZE+1, sendf)) > 0){ //send all file data to the server
				write(fdOw, msgbufo, MSGSIZE+1);
				//msgbufo = calloc((MSGSIZE+1), sizeof(char)); //reinitialize buffer
				fflush(stdout);
			}
			//line = calloc(lsize, sizeof(char));
			memset(command, 0, 30);
			fclose(sendf);
			printf("\n");
		}
		else if(!strcmp(command, "list")){
			printf("\n");
			writeo = write(fdOw, line, lsize);
			usleep(1000);
			while(read(fdOr, msgbufo, MSGSIZE+1) > 0)
				printf("%s", msgbufo);
			printf("\n");
		}					
		else{ //for write command
			writeo = write(fdOw, line, lsize);
			if (writeo > 0)			
     				fflush(stdout);
			printf("\n");
		}
		fflush(stdout);
		
	}
	printf("terminating boardpost\n");
	close(fdOr);
	close(fdOw);
	free(line); free(othersw); free(othersr); free(path); free(spid); free(token); free(fname); free(csizeS); free(msgbufo);
	free(command);
}


