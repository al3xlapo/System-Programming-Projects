#include "channels.h"


channel* createChannel(channel* c, int id, char* name){
	c = malloc(sizeof(channel)); //create the channel
	c->id = id; //initialize it
	c->name = malloc((strlen(name)+1) * sizeof(char));
	strcpy(c->name, name);
	c->next = NULL;
	c->chMessages = NULL;
	c->chFiles = NULL;
}


void insertChannel(channel** c, int id, char* name){  //add a new channel at the end of the board
	channel* comp = findChannel(*c, id);
	if(comp != NULL){
		printf("A channel with the id given already exists\n");
		return;
	}
	if(*c == NULL)
		*c = createChannel(*c, id, name);
	else{
		while((*c)->next != NULL)
			c = &((*c)->next);
		channel* temp = *c;
		c = &((*c)->next);
		*c = createChannel(*c, id, name);
		temp->next = *c;
	}	
}


void getMessages(int fdSw, channel* c, int id){

	FILE *new;
	int check;
	check = printMessages(fdSw, c, id); //first send all messges and filenames
	if(check){ //there were files in the channel
		printf("Writing files ...\n");
		file* f;
		f = c->chFiles;
		while(f != NULL){ //for each file
			if( (new = fopen(f->name, "r+")) != NULL) { //if a file with the same name already exists
				strcat(f->name, "_0"); //rename the new file so as not to overwrite the previous
				fclose(new);
				new = fopen(f->name, "w");
				fprintf(new, "%s", f->content); //print content in the new file
				fclose(new);
			}
			else{
				new = fopen(f->name, "w");
				fprintf(new, "%s", f->content); //print content in the new file
				fclose(new);
			}
			f = f->next;
		}
		printf("Finished writing files\n");
	}
	
}


void list(channel* c){
	while(c != NULL){
		printf("%s %d\n", c->name, c->id);
		c = c->next;
	}
}


channel* findChannel(channel* c, int id){
	while(c != NULL){
		if(c->id == id)
			return c;
		c = c->next;
	}
	return NULL;
}


message* createMessage(message* m, char* msg){
	m = malloc(sizeof(message));
	m->msg = malloc((strlen(msg)+1) * sizeof(char));
	strcpy(m->msg, msg);
	m->next = NULL;
}


void storeMsg(channel** c, int id, char* msg){ //add the message to the channel's list of messages
	message** m;	
	channel* ct;
	ct = findChannel(*c, id);
	if(ct != NULL){
		m = &(ct->chMessages); //m now points to channel "id"'s list of messages
	}
	else{
		printf("No channel with that id exists\n");
		return;
	}
	if(*m == NULL)  //insert the message to the list of messages of the channel with that id
		*m = createMessage(*m, msg);		
	else{
		while((*m)->next != NULL)
			m = &((*m)->next);
		message* temp = *m;
		m = &((*m)->next);
		*m = createMessage(*m, msg);
		temp->next = *m;
	}	
}


file* createFile(file* f, char* name, char* content, char* path){

	name = basename(name); //strip path, leave just the name of the file
	f = malloc(sizeof(file));
	f->name = malloc(200 * sizeof(char));
	strcpy(f->name, path); //the file is going ot be stored in the server's directory
	strcat(f->name, "/");
	strcat(f->name, name);
	f->content = malloc((strlen(content)+1) * sizeof(char));
	strcpy(f->content, content);
	f->next = NULL;
	return f;
}


void storeFile(channel** c, int id, char* name, char* content, char* path){
	file** f;
	channel* ct = NULL;	
	ct = findChannel(*c, id);
	if(ct == NULL){
		printf("No channel with that id exists\n");
		return;
	}
	f = &(ct->chFiles);
	if(*f == NULL)  //insert the message to the list of messages of the channel with that id
		*f = createFile(*f, name, content, path);		
	else{
		while((*f)->next != NULL)
			f = &((*f)->next);
		file* temp = *f;
		f = &((*f)->next);
		*f = createFile(*f, name, content, path);
		temp->next = *f;
	}
}


int printMessages(int fdSw, channel*  c, int id){ //write all the messages/files in the channel with id == cid to fdSw
						 //printMessages will only return 1 if at least one file exists, else it returns 0
	char* msgbufs = malloc((MSGSIZE+1) * sizeof(char));
	message** m = NULL;
	file** f = NULL;
	channel* ct;
	ct = findChannel(c, id);
	if(ct == NULL){
		sprintf(msgbufs, "\nChannel %d doesn't exist\n", id);
		write(fdSw, msgbufs, MSGSIZE+1);
		free(msgbufs);
		return 0;
	}
	if(ct->chMessages == NULL && ct->chFiles == NULL){
		sprintf(msgbufs, "\nChannel %d is empty\n", ct->id);
		write(fdSw, msgbufs, MSGSIZE+1);
		free(msgbufs);
		return 0;
	}	
	else{
		sprintf(msgbufs, "\nChannel %d\n", ct->id);
		write(fdSw, msgbufs, MSGSIZE+1);
		if(ct->chMessages != NULL){
			*m = ct->chMessages;
			while(*m != NULL){
				printf("bomb\n");
				sprintf(msgbufs, "Message %s\n", (*m)->msg);
				write(fdSw, msgbufs, MSGSIZE+1);
				m = &((*m)->next);
			}
		}
		if(ct->chFiles != NULL){
			*f = ct->chFiles;
			while(*f != NULL){
				sprintf(msgbufs, "File %s\n", (*f)->name); //this is going to get printed
				write(fdSw, msgbufs, MSGSIZE+1);
				f = &((*f)->next);
			}
			free(msgbufs);
			return 1;
		}
		free(msgbufs);
		return 0;
	}
}



void printChannels(int fdOw, channel* c){

	char* msgbufo = malloc((MSGSIZE+1) * sizeof(char));
	if(c == NULL){
		sprintf(msgbufo, "There are no channels\n");
		write(fdOw, msgbufo, MSGSIZE+1);
		free(msgbufo);
		return;
	}
	while(c != NULL){
		sprintf(msgbufo, "Channel with id %d and name %s\n", c->id, c->name);
		write(fdOw, msgbufo, MSGSIZE+1);
		c = c->next;
	}
	free(msgbufo);
}


void deleteBoard(channel** c){ //delete all channels in the board
	
	channel* temp;
	while(*c != NULL){
		temp = *c;		
		c = &((*c)->next);
		deleteChannel(temp);
		free(temp->name);
		free(temp);
	}
	*c = NULL;
}


void clearChannel(channel** c, int id){

	channel* ct;
	ct = findChannel(*c, id);
	if(ct == NULL) return;
	message** m;
	file** f;
	if(ct->chMessages != NULL){
		*m = ct->chMessages;
		deleteMessages(*m);
		printf("just exited chMessages\n");
		*m = NULL;
	}
	printf("passed chMessages\n");
	if(ct->chFiles != NULL){
		*f = ct->chFiles;
		deleteFiles(*f);
		printf("passed chFiles\n");
		*f = NULL;
	}
	printf("passed chFiles\n");
}

	
void deleteChannel(channel* c){ //delete all messages in the channel

	message** m;
	file** f;
	if(c->chMessages != NULL){
		*m = c->chMessages;
		deleteMessages(*m); 
		*m = NULL;
	}
	if(c->chFiles != NULL){
		*f = c->chFiles;
		deleteFiles(*f);
		*f = NULL;
	}
}

void deleteMessages(message* m){
	message* temp;
	while(m != NULL){
		temp = m;
		m = m->next;
		free(temp->msg);
		free(temp);
	}
}


void deleteFiles(file* f){
	file* temp;
	while(f != NULL){
		temp = f;
		f = f->next;
		free(temp->name);
		free(temp->content);
		free(temp);
	}
}

