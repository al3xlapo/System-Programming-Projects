#include "operations.h"


const char* ccreate = "createnodes";
const char* cdeln = "delnodes";
const char* caddt = "addtran";
const char* cdelt = "deltran";
const char* clook = "lookup";
const char* ctri = "triangle";
const char* cconn = "conn";
const char* callc = "allcycles";
const char* ctrace = "traceflow";
const char* cbye = "bye";
const char* cprint = "print";
const char* cdump = "dump";


int main(int argc, char *argv[]){
	FILE *op = NULL, *stream, *fp;
	int i, go, gl, invalid = 0, N, N1, N2, mode, L;
	float cash, k;
	size_t lsize = 300;
	nList* l = NULL;
	char opfilename[50], filename[50], command[20], smode[20];
	char* token = (char*)malloc(20 * sizeof(char));
	char* line=(char*)malloc(lsize * sizeof(char)); //it will be used to store lines read, be it a command or a line in the file
	node **nd;
	bucket* bt;
	hashTable* accTable = NULL;
	int HTsize = 0;
	

	//parse command arguments, initialize
	while( (go = getopt(argc, argv, "o:b:")) != -1){
		switch(go){
			case 'o':
				strcpy(opfilename, optarg);
				if((op=fopen(opfilename, "r")) == NULL){
					printf("failed: the specified filename for operations file doesn't exist\n");
					return 1;
				}
				break;
			case 'b':
				if( (HTsize = atoi(optarg)) <= 0 ){
					printf("failed: the hashtable must have at least one entry\n");
					return 1;
				}
				initializeHashTable(&accTable, HTsize); //initialize table
				break;
		}
	}

	if(!HTsize) return -1;
	if(op != NULL){ //read from file first if it exists
		stream=op;
		printf("Reading from operations file...\n\n");
	}
	else{
		stream=stdin;
		printf("Waiting for commands...\n\n>");
	}
	

	//operations

	//for each line (command) given check if the first string (the command word) exists
	//if it does use strtok or sscanf to skip it and parse the rest of the line, executing what's needed
	while( (gl = getline(&line, &lsize, stream)) != -1 ){ //reading commands (either from file or stdin);
		sscanf(line, "%s", command);
		if( strcmp(command, ccreate) && strcmp(command, cdeln) && strcmp(command, cdelt) && strcmp(command, caddt) && 
			strcmp(command, clook) && strcmp(command, ctri) && strcmp(command, cconn) && strcmp(command, callc) &&
			strcmp(command, ctrace) && strcmp(command, cbye) && strcmp(command, cprint) && strcmp(command, cdump))
		{
			printf("Unknown command\n\n");
			if(!op) printf(">");
			continue; //discard unknown commands
		}
		
		if(!strcmp(command, ccreate)){
			token = strtok(line, " "); //skip the first word and get the first argument
			token = strtok(NULL, " ");
			while(token != NULL){ //parse the rest of the line
				if(token[strlen(token)-1] == '\n') token[strlen(token)-1] = '\0'; //remove \n character
				if(!inspect(token)) //inspect returns 1 if the ID is 7-digit
					printf("%d isn't a 7-digit number\n", atoi(token));
				else
					insertL(&l, atoi(token));
				token = strtok(NULL, " ");
			}
			createNodes(accTable, &l);
			deleteL(&l); //clear the list for its next use
		}
		else if(!strcmp(command, cdeln)){
			token = strtok(line, " ");
			token = strtok(NULL, " ");
			while(token != NULL){
				if(token[strlen(token)-1] == '\n') token[strlen(token)-1] = '\0'; //remove \n character
				if(!inspect(token))
					printf("%d isn't a 7-digit number\n", atoi(token));
				else
					insertL(&l, atoi(token));
				token = strtok(NULL, " ");
			}
			delNodes(accTable, &l);
			deleteL(&l);
		}
		else if(!strcmp(command, caddt)){
			sscanf(line, "%*s %d %d %f\n", &N1, &N2, &cash);
			addtran(accTable, N1, N2, cash);
		}
		else if(!strcmp(command, cdelt)){
			sscanf(line, "%*s %d %d\n", &N1, &N2);
			deltran(accTable, N1, N2);
		}
		else if(!strcmp(command, clook)){
			mode = 0;
			if( sscanf(line, "%*s %s %d\n", smode, &N) != 2 ){
				printf("Wrong command form\n");
				if(!op) printf(">");
				continue;
			}
			if(strcmp(smode, "sum") == 0) mode = 0;
			else if(strcmp(smode, "in") == 0) mode = 1;
			else if(strcmp(smode, "out") == 0) mode = 2;
			lookup(accTable, mode, N);
		}
		else if(!strcmp(command, ctri)){
			if( sscanf(line, "%*s %d %f\n", &N, &k) != 2 ){
				printf("Wrong command form\n");
				if(!op) printf(">");
				continue;
			}
			triangle(accTable, N, k);
		}
		else if(!strcmp(command, cconn)){
			if( sscanf(line, "%*s %d %d\n", &N1, &N2) != 2 ){
				printf("Wrong command form\n");
				if(!op) printf(">");
				continue;
			}
			conn(accTable, N1, N2);
		}
		else if(!strcmp(command, callc)){
			if( sscanf(line, "%*s %d\n", &N) != 1 ){
				printf("Wrong command form\n");
				if(!op) printf(">");
				continue;
			}
			allcycles(accTable, N);	
		}
		else if(!strcmp(command, ctrace)){
			if( sscanf(line, "%*s %d %d\n", &N, &L) != 2 ){
				printf("Wrong command form\n");
				if(!op) printf(">");
				continue;
			}
			traceflow(accTable, N, L);	
		}
		else if(!strcmp(command, cbye)){
			bye(accTable);
			free(accTable->data);
			accTable->data = NULL;
			free(accTable);
			accTable = NULL;
			initializeHashTable(&accTable, HTsize);
		}
		else if(!strcmp(command, cprint)){
			printG(accTable);
		}
		else if(!strcmp(command, cdump)){
			if( sscanf(line, "%*s %s\n", filename) != 1 ){
				printf("Wrong command form\n");
				if(!op) printf(">");
				continue;
			}
			if(filename[strlen(filename) - 1] == 'c' || filename[strlen(filename) - 1] == 'h'){
				printf("failed: can't overwrite .c or .h files\n\n");
				if(!op) printf(">");
				continue;
			}
			if((fp=fopen(filename, "w")) == NULL){
				printf("failed: couldn't create dump file\n\n");
				if(!op) printf(">");
				continue;
			}
			dump(accTable, fp);
			fclose(fp);
		}
		if(!op) printf(">");
	}
	if(!op){ //if it was reading from input it will just finish
		bye(accTable);
		free(accTable->data);
		free(accTable);
		deleteL(&l);
		free(token);
		free(line);
		return 0;
	}
	//else if it was reading from operations, it will start reading from input
	fclose(op);
	stream=stdin;
	printf("Waiting for commands...\n\n>");
	
	//reading commands from stdin now, if an operations file was given before
	while( (gl = getline(&line, &lsize, stream)) != -1 ){ 
		sscanf(line, "%s", command);
		if( strcmp(command, ccreate) && strcmp(command, cdeln) && strcmp(command, cdelt) && strcmp(command, caddt) && 
			strcmp(command, clook) && strcmp(command, ctri) && strcmp(command, cconn) && strcmp(command, callc) &&
			strcmp(command, ctrace) && strcmp(command, cbye) && strcmp(command, cprint) && strcmp(command, cdump))
		{
			printf("Unknown command\n\n");
			if(!op) printf(">");
			continue; //discard unknown commands
		}
		
		if(!strcmp(command, ccreate)){
			token = strtok(line, " "); //skip the first word and get the first argument
			token = strtok(NULL, " ");
			while(token != NULL){ //parse the rest of the line
				if(token[strlen(token)-1] == '\n') token[strlen(token)-1] = '\0'; //remove \n character
				if(!inspect(token)) //inspect returns 1 if the ID is 7-digit
					printf("%d isn't a 7-digit number\n", atoi(token));
				else
					insertL(&l, atoi(token));
				token = strtok(NULL, " ");
			}
			createNodes(accTable, &l);
			deleteL(&l); //clear the list for its next use
		}
		else if(!strcmp(command, cdeln)){
			token = strtok(line, " ");
			token = strtok(NULL, " ");
			while(token != NULL){
				if(token[strlen(token)-1] == '\n') token[strlen(token)-1] = '\0'; //remove \n character
				if(!inspect(token))
					printf("%d isn't a 7-digit number\n", atoi(token));
				else
					insertL(&l, atoi(token));
				token = strtok(NULL, " ");
			}
			delNodes(accTable, &l);
			deleteL(&l);
		}
		else if(!strcmp(command, caddt)){
			sscanf(line, "%*s %d %d %f\n", &N1, &N2, &cash);
			addtran(accTable, N1, N2, cash);
		}
		else if(!strcmp(command, cdelt)){
			sscanf(line, "%*s %d %d\n", &N1, &N2);
			deltran(accTable, N1, N2);
		}
		else if(!strcmp(command, clook)){
			mode = 0;
			if( sscanf(line, "%*s %s %d\n", smode, &N) != 2 ){
				printf("Wrong command form\n");
				printf(">");
				continue;
			}
			if(strcmp(smode, "sum") == 0) mode = 0;
			else if(strcmp(smode, "in") == 0) mode = 1;
			else if(strcmp(smode, "out") == 0) mode = 2;
			lookup(accTable, mode, N);
		}
		else if(!strcmp(command, ctri)){
			if( sscanf(line, "%*s %d %f\n", &N, &k) != 2 ){
				printf("Wrong command form\n");
				printf(">");
				continue;
			}
			triangle(accTable, N, k);
		}
		else if(!strcmp(command, cconn)){
			if( sscanf(line, "%*s %d %d\n", &N1, &N2) != 2 ){
				printf("Wrong command form\n");
				printf(">");
				continue;
			}
			conn(accTable, N1, N2);
		}
		else if(!strcmp(command, callc)){
			if( sscanf(line, "%*s %d\n", &N) != 1 ){
				printf("Wrong command form\n");
				printf(">");
				continue;
			}
			allcycles(accTable, N);	
		}
		else if(!strcmp(command, ctrace)){
			if( sscanf(line, "%*s %d %d\n", &N, &L) != 2 ){
				printf("Wrong command form\n");
				printf(">");
				continue;
			}
			traceflow(accTable, N, L);	
		}
		else if(!strcmp(command, cbye)){
			bye(accTable);
			free(accTable->data);
			accTable->data = NULL;
			free(accTable);
			accTable = NULL;
			initializeHashTable(&accTable, HTsize);
		}
		else if(!strcmp(command, cprint)){
			printG(accTable);
		}
		else if(!strcmp(command, cdump)){
			if( sscanf(line, "%*s %s\n", filename) != 1 ){
				printf("Wrong command form\n");
				printf(">");
				continue;
			}
			if(filename[strlen(filename) - 1] == 'c' || filename[strlen(filename) - 1] == 'h'){
				printf("failed: can't overwrite .c or .h files\n\n");
				printf(">");
				continue;
			}
			if((fp=fopen(filename, "w")) == NULL){
				printf("failed: couldn't create dump file\n\n");
				printf(">");
				continue;
			}
			dump(accTable, fp);
			fclose(fp);
		}
		printf(">");
	}
	bye(accTable); //end
	deleteL(&l);
	free(accTable->data);
	free(accTable);
	free(token);
	free(line);
	return 0;
}
