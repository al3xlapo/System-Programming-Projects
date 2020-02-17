#include "board.h"



int pidClient = 0; //global variables to be seen by signal  handlers
int hangup = 0;
int fdSw, fdSr, fdOw, fdOr;
FILE* fs=NULL;

int main(int argc, char** argv){

	
	FILE *fpid = NULL, *fsend = NULL;
	int gl, selectS = 1, selectO = 1, pidS, cid;
	pid_t pid;
	size_t lsize = 200;
	char *path, *selfw, *selfr, *othersw, *othersr, *spid, *msgbufs, *msgbufo, *line, *token, *command, *pidServer,
		 *cname, *msg, *fname, *content, *tempname;

	umask(000);
	msgbufs = malloc((MSGSIZE+1) * sizeof(char));
	msgbufo = malloc((MSGSIZE+1) * sizeof(char));
	line = malloc(lsize * sizeof(char));
	token = malloc(50 * sizeof(char));
	pidServer = malloc(30 * sizeof(char));
	cname = malloc(50 * sizeof(char));
	msg = malloc(200 * sizeof(char));
	command = malloc(30 * sizeof(char));
	fname = malloc(50 * sizeof(char));
	tempname = malloc(50 * sizeof(char));
	
	//check arguments
	if(argc != 2){ 
		printf("Wrong arguments, execution format is ./board <path>\n");
		exit(1);
	}
	mkdir(argv[1], 0777); //if the directory doesn't exist, create it
	chmod(argv[1], 0777);

	

	//the server and client must ignore SINGINT (ctrl + c)
  	sigset_t block_mask, old_mask;
	sigemptyset(&block_mask);
	sigaddset (&block_mask, SIGINT); //add SIGINT to the mask and block it
	if(sigprocmask(SIG_SETMASK, &block_mask, &old_mask) < 0){
		perror("sigprocmask server/client");
		exit(1);
	}
			
	
	//initialize named pipes and files
	//**fdSr is where the server reads for self, fdSw where the server writes for self (and selfr and selfw their fifos)
	//**fdOr is where thre server reads for others, fdOw where the server writes for others (and othersr and othersw their fifos)
	//**similarly fdOr is where boardpost reads, fdSr where client reads etc
	//client writes where server reads and reads where server writes
	

	path = calloc((strlen(argv[1])+1), sizeof(char));	
	strcpy(path, argv[1]);
	selfw =  calloc((strlen(path)+11), sizeof(char));
	selfr = calloc((strlen(path)+11), sizeof(char));
	spid = calloc((strlen(path)+11), sizeof(char));
	othersw = calloc((strlen(path)+11), sizeof(char));
	othersr = calloc((strlen(path)+11), sizeof(char));
	
	strcpy(selfr, path);
	strcpy(othersr, path);
	strcpy(selfw, path);
	strcpy(othersw, path);
	strcpy(spid, argv[1]);	
	
	if(path[strlen(path) - 1] != '/'){
		strcat(selfr, "/_selfr");
		strcat(othersr, "/_othersr");
		strcat(spid, "/_pid");
		strcat(selfw, "/_selfw");
		strcat(othersw, "/_othersw");
	}
	else{
		strcat(selfr, "_selfr");
		strcat(othersr, "_othersr");
		strcat(selfw, "_selfw");
		strcat(othersw, "_othersw");
		strcat(spid, "_pid");
	}
	
	
	if( (fs = fopen(spid, "r")) != NULL ){ //check if _pid file exists. if it does, the server is up
		hangup = 0;
		//set handler to wake up after receiving SIGCONT from server
		struct sigaction wake;
		wake.sa_handler = wakeup;
		wake.sa_flags = 0;
		if(sigaction(SIGCONT, &wake, NULL)){
			perror("sigaction client\n");
			exit(1);
		}
		printf("The server is already running\n");
		//the server is up, so the process won't be forked, the same client code will be used here

		fscanf(fs, "%s", pidServer);
		pidS = atoi(pidServer);

		//set up handler for SIGHUP, when received the client will terminate
		struct sigaction terminate;
		terminate.sa_handler = endClient;
		sigemptyset(&terminate.sa_mask);
		terminate.sa_flags = 0;
		if(sigaction(SIGHUP, &terminate, NULL)){
			perror("sigaction client\n");
			exit(1);
		}

		kill(pidS, SIGUSR1); //let the server know your pid

		//open fifo for writing, after waiting for signal from the server		 
		
		if( (fdSw = open(selfr, O_RDWR | O_NONBLOCK)) == -1 ) {
			perror( "fifo self open error (client)");
			exit(1);
		}
		if( (fdSr = open(selfw, O_RDWR | O_NONBLOCK)) == -1 ) {
			perror( "fifo self open error (client)");
			exit(1);
		}
		
		//send each "line" - command given to the server process
		//the client will stop looping and terminate, if it receives SIGHUP
		while( (gl = getline(&line, &lsize, stdin) ) != -1  && !hangup){
			if(hangup) break;
			if( (strlen(line) >= 1) && line[strlen(line) - 1] == '\n')
				line[strlen(line) - 1] = '\0';
			sscanf(line, "%s", command);
			if(!strcmp(command, "getmessages")){
				printf("\n");
				int writes = write(fdSw, line, lsize);
				usleep(1000);
				
				while(read(fdSr, msgbufs, MSGSIZE+1) > 0)
					printf("%s", msgbufs);
				printf("\n");
			}
			else{
				
				int writes = write(fdSw, line, lsize);
				if (writes > 0)	
	            			fflush(stdout);
			}
			command = calloc(30, sizeof(char));
		}
		printf("terminating client\n");
		free(path); free(spid);  free(pidServer); free(msgbufs);
		free(msgbufo); free(token); free(line); free(cname); free(msg); free(command); free(fname); free(tempname);
		free(othersr); free(othersw); free(selfr); free(selfw);
		close(fdSr);
		close(fdSw);
		fclose(fs);
		exit(0); //end client
	}
	//printf("selfr %s othersr %s selfw %s othersw %s\n", selfr, othersr, selfw, othersw);
	
	if( (pid = fork()) == 0 ){ //if it is the server
		//continue to create fifos and then fork if the _pid file didn't exist
		if( mkfifo(selfr, 0666) == -1 ){
			if ( errno != EEXIST ) {
				perror (" mkfifo _selfr " ) ;
				exit (1) ;
			}
		}
		if( mkfifo(othersr, 0666) == -1 ){
			if ( errno != EEXIST ) {
				perror (" mkfifo _othersr " ) ;
				exit (1) ;
			}
		}
		if( mkfifo(selfw, 0666) == -1 ){
			if ( errno != EEXIST ) {
				perror (" mkfifo _selfw " ) ;
				exit (1) ;
			}
		}
		if( mkfifo(othersw, 0666) == -1 ){
			if ( errno != EEXIST ) {
				perror (" mkfifo _othersw " ) ;
				exit (1) ;
			}
		}
	
		int r = 0, sumr = 0;
		struct sigaction wake;
		wake.sa_handler = wakeup;
		wake.sa_flags = 0;
		if(sigaction(SIGCONT, &wake, NULL)){
			perror("sigaction server\n");
			exit(1);
		}
		//msgbufs = malloc((MSGSIZE+1) * sizeof(char));

		//open fifos and files for reading/writing
		
		if( (fdSr = open(selfr, O_RDWR | O_NONBLOCK)) == -1 ) { //open _selfr
			perror( "fifo self open error (server)");
			exit(1);
		}
		if( (fdOr = open(othersr, O_RDWR | O_NONBLOCK)) == -1 ) { //open _othersr
			perror( "fifo others open error (server)");
			exit(1);
		}
		if( (fdSw = open(selfw, O_RDWR | O_NONBLOCK)) == -1 ) { //open _selfw
			perror( "fifo self open error (server)");
			exit(1);
		}
		if( (fdOw = open(othersw, O_RDWR | O_NONBLOCK)) == -1 ) { //open _othersw
			perror( "fifo others open error (server)");
			exit(1);
		}
		if( (fpid = fopen(spid, "w+")) == NULL ){ //open _pid
			perror("Couldn't create file with server pid\n");
			exit(1);
		}
		
		sprintf(pidServer, "%d", getpid());
		fputs(pidServer, fpid);
		fseek(fpid, SEEK_SET, 0);
			
		//set up handler for SIGTERM sent by clients, it will be used to get client's pid
		struct sigaction touch;
		touch.sa_sigaction = getPid;
		sigemptyset(&touch.sa_mask);
		touch.sa_flags = SA_SIGINFO; //siginfo needed to get the sender's (client's) signal
		if(sigaction(SIGUSR1, &touch, NULL)){
			perror("sigaction server\n");
			exit(1);
		}
		
		//set up mask for pselect(), all signals will be pending until pselect waits again, and handled then
		sigemptyset(&block_mask);
		sigaddset(&block_mask, SIGUSR1); //SIGILL will be blocked and handled when received by server		
		if(sigprocmask(SIG_BLOCK, &block_mask, &old_mask) < 0){ //old_mask now contains only SIGINT
			perror("sigprocmask server");
			exit(1);
		}
		
		//set up pselect() (fd_set rfds is defined globally)
    		struct timeval tv;
		int retval;
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(fdSr, &rfds);
		FD_SET(fdOr, &rfds);

		usleep(100);
		int ppid = getppid();
		channel* c = NULL; //initialize board		

		kill(ppid, SIGCONT); //signal the client that it's ok to open the fifo

		//read data from the client and users (through fifo), process it and perform the actions needed
		while(1){
			//if pselect returns check to see which file descriptor is ready and do what's needed
			retval = pselect(10, &rfds, NULL, NULL, NULL, &old_mask);
			if(retval > 0){
				if(FD_ISSET(fdSr, &rfds)){ //if the client sent a message
					int reads = read(fdSr, msgbufs, MSGSIZE+1);
					if (reads > 0) {
	        	    			sscanf(msgbufs, "%s", command);
						if(!strcmp(command, "createchannel")){
							//skip the command and get the first argument
							token = strtok(msgbufs, " "); 
							if(token!=NULL) 
								token = strtok(NULL, " "); //get channel id
							else{
								printf("wrong command\n");
								continue;
							}  
							cid = atoi(token);
							if(token!=NULL) 
								token = strtok(NULL, " "); //get channel name
							else{
								printf("wrong command\n");
								continue;
							}  
							strcpy(cname, token);
							insertChannel(&c, cid, cname);
						}
						else if(!strcmp(command, "getmessages")){
							//skip the command and get the first argument
							token = strtok(msgbufs, " "); 
							token = strtok(NULL, " "); //get channel id
							cid = atoi(token);
							getMessages(fdSw, c, cid);
							printf("exited getmessages goig for clear channel\n");
							pause();
							clearChannel(&c, cid);
						}
						else if(!strcmp(command, "exit")){
							kill(pidClient, SIGHUP); //terminate client
						}
						else if(!strcmp(command, "shutdown")){
							kill(pidClient, SIGHUP); //terminate client
							break; //and break the loop to terminate yourself
		
						}
						else
							printf("Unknown command\n\n");
						fflush(stdout);
						memset(command, 0, 30);
		        		}
					else if(reads == 0){ //EOF received from client
						printf("Read received EOF\n");
						
					}	
					else{
						printf("Error in read of _self\n");
						break;
					}
				}
				if(FD_ISSET(fdOr, &rfds)){ //if the boardpost sent a message
					int reado = read(fdOr, msgbufo, MSGSIZE+1);
					if (reado > 0){
						sscanf(msgbufo, "%s", command);
						if(!strcmp(command, "list")){
							printChannels(fdOw, c);	
						}
						else if(!strcmp(command, "write")){
							token = strtok(msgbufo, " "); 
							if(token!=NULL) 
								token = strtok(NULL, " "); //get channel id
							else{
								printf("wrong command\n");
								continue;
							} 
							cid = atoi(token);
							if(token!=NULL) 
								token = strtok(NULL, " "); //get channel name
							else{
								printf("wrong command\n");
								continue;
							} 
							strcpy(msg, token);
							storeMsg(&c, cid, msg);
						}
						else if(!strcmp(command, "send")){
							token = strtok(msgbufo, " "); 
							if(token!=NULL) 
								token = strtok(NULL, " "); //get channel id
							else{
								printf("wrong command\n");
								continue;
							}
							cid = atoi(token);
							if(token!=NULL) 
								token = strtok(NULL, " "); //get channel name
							else{
								printf("wrong command\n");
								continue;
							} 
							strcpy(fname, token);
							if(token!=NULL) 
								token = strtok(NULL, " "); //get the size of file data
							else{
								printf("wrong command\n");
								continue;
							} 
							//and allocate a large enough string
							content = malloc(atoi(token) * sizeof(char));
							content[0]='\0';
							pause(); //wait for boardpost to start writing data
							//now read file data and store it in content
							while( (r = read(fdOr, msgbufo, MSGSIZE+1)) > 0){
								if(content[0] == 0)
									strcpy(content, msgbufo);
									//memcpy(content+sumr, msgbufo, r);
								else
									strcat(content, msgbufo);
									//memcpy(content+sumr, msgbufo, r);
								sumr += r;
							}
							storeFile(&c, cid, fname, content, path);
							free(content);
							sumr = 0;
						}
						else
							printf("Unknown command\n\n");
						memset(command, 0, 30);
						fflush(stdout);
					}
					else if(reado == 0){ //EOF received from boardpost	
					}	
					else{
						printf("Error in read of _others\n");
						break;
					}				
	        		}
				FD_SET(fdSr, &rfds);
				FD_SET(fdOr, &rfds);
			}
			else if(retval == 0)
				printf("Select timed out\n");
			else{
				//perror("Error returned from select\n");
				fflush(stdout);
				continue;
			}		
		}

		//server shutdown
		printf("terminating server\n");
		close(fdOr);
		close(fdSr);
		close(fdOw);
		close(fdSw);
		fclose(fpid);
		if(unlink(selfr) || unlink(othersr) || unlink(selfw) || unlink(othersw) || remove(spid)){
			perror("remove");
			exit(1);
		}
		if(c != NULL) deleteBoard(&c);
		free(path); free(spid); free(pidServer); free(msgbufs);
		free(msgbufo); free(token); free(line); free(cname); free(msg); free(command); free(fname);
		free(othersr); free(othersw); free(selfr); free(selfw);
		fflush(stdout);
		exit(0);
	}
	else{ //if it is the client
		//set handler to wake up after receiving SIGCONT from server
		int r, sumr=0;
		struct sigaction wake;
		wake.sa_handler = wakeup;
		wake.sa_flags = 0;
		if(sigaction(SIGCONT, &wake, NULL)){
			perror("sigaction client\n");
			exit(1);
		}
		
		pause(); //wait until the server is set up
		if( (fs = fopen(spid, "r+")) == NULL ){
			printf("Couldn't open _pid file(client)\n");
			exit(1);
		}
		fscanf(fs, "%s", pidServer); //get server pid
		pidS = atoi(pidServer);
		
		kill(pidS, SIGUSR1); //let server know your pid
		
		//set up handler for SIGHUP, when received the client will terminate
		struct sigaction terminate;
		terminate.sa_handler = endClient;
		sigemptyset(&terminate.sa_mask);
		terminate.sa_flags = 0;
		if(sigaction(SIGHUP, &terminate, NULL)){
			perror("sigaction client\n");
			exit(1);
		}
		

		//open fifo for writing					
		if( (fdSw = open(selfr, O_RDWR | O_NONBLOCK)) == -1 ) {
			perror( "fifo self open error (client)");
			exit(1);
		}
		if( (fdSr = open(selfw, O_RDWR | O_NONBLOCK)) == -1 ) {
			perror( "fifo self open error (client)");
			exit(1);
		}
		//send each "line" - command given to the server process
		//the client will stop looping and terminate, if it receives SIGHUP
		while( (gl = getline(&line, &lsize, stdin) ) != -1  && !hangup){
			if(hangup) break;
			if( (strlen(line) >= 1) && line[strlen(line) - 1] == '\n')
				line[strlen(line) - 1] = '\0';
			sscanf(line, "%s", command);
			if(!strcmp(command, "getmessages")){
				printf("\n");
				int writes = write(fdSw, line, lsize);
				usleep(1000);
				while(read(fdSr, msgbufs, MSGSIZE+1) > 0)
					printf("%s", msgbufs);
				printf("\n");
			}
			else{
				int writes = write(fdSw, line, lsize);
			}
			memset(line, 0, lsize);
			memset(command, 0, 30);
			fflush(stdout);
		}
		printf("terminating client\n");
		free(path); free(spid); free(pidServer); free(msgbufs);
		free(msgbufo); free(token); free(line); free(cname); free(msg); free(command); free(fname); free(tempname);
		free(othersr); free(othersw); free(selfr); free(selfw);
		close(fdSr);
		close(fdSw);
		fclose(fs);
		exit(0);
	}
}



void wakeup(int signum){

}


void endClient(int signum){

	hangup = 1;	
}	
	
void getPid(int signum, siginfo_t* siginfo, void* context){

	pidClient = siginfo->si_pid;	
}

