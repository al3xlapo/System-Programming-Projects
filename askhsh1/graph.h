#include "list.h"


//graph
typedef struct edge edge;
typedef struct node node;


struct node{ //node's id and inbound and outbound edges
	int accId;
	int v;	 //v for visited
	int p;	//p is 1 when this node needs to be printed, because it belongs in a triangle, cycle, etc
	edge* in;
	edge* out;
};


//if the edge is inbound, the "neigh" pointer points to to the node who sends the cash, the start of the edge
//else it points to the node about to receive the cash, the receiver
struct edge{
	float cash;
	node* neigh;
	edge* next; 
};

void insertEdge(node** source, node** target, float cash);
void deleteEdge(node** source, node** target);
edge* createEdge(edge* e, float cash, node* neigh);
int delNode(node** n);
int sumEdges(edge* e);
void triangleSearch(node* nd, int N, float k, int steps, nList** l);
int connSearch(node* nd, int N1, int N2, nList** l);
void allcyclesSearch(node* nd, int N, int steps, nList** l);
void traceflowSearch(node* nd, int N, int L, int l, float cash, nList** list);
void deleteEdges(edge* e);
void printNeigh(edge* e);
void dumpEdge(int N1, edge* e, dList** d);

