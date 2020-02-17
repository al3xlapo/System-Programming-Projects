#include "list.h"

typedef struct edge edge;
typedef struct node node;


struct node{ //node's id, its balance and transfers(edge*) it has received
	char accName[200];
	int balance;
	edge* in;
};


//"neigh" pointer points to to the node who sends the cash, the start of the edge, cash is the total cash received from that account(neigh)
struct edge{
	float cash;
	node* neigh;
	edge* next; 
};

void insertEdge(node** source, node** target, float cash);
edge* createEdge(edge* e, float cash, node* neigh);
int delNode(node** n);
void deleteEdges(edge* e);
void printNeigh(edge* e);
