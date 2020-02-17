#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <libgen.h>


#define MSGSIZE 500


//list of channels, with each channel pointing to a list of its messages

typedef struct message{
	char* msg;
	struct message* next;
}message;

typedef struct file{
	char* name;
	char* content;
	struct file* next;
}file;
	

typedef struct channel{
	int id;
	char* name;
	message* chMessages; //list of messages the channel holds
	file* chFiles;
	struct channel* next;  //pointer to the next channel
}channel;


channel* createChannel(channel* c, int id, char* name);
file* createFile(file* f, char* name, char* content, char* path);
void storeFile(channel** c, int id, char* name, char* content, char* path);
void insertChannel(channel** c, int id, char* name);
void getMessages(int fdSw, channel* c, int id);
void list(channel* c);
channel* findChannel(channel* c, int id);
message* createMessage(message* m, char* msg);
void storeMsg(channel** c, int id, char* msg);
void printChannels(int fdOw, channel* c);
int printMessages(int fdSw, channel*  c, int id);
void deleteBoard(channel** c);
void deleteChannel(channel* c);
void deleteMessages(message* m);
void deleteFiles(file* f);
void getFile(char* fname, char* content);
void clearChannel(channel** c, int id);



