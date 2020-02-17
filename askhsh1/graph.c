#include "graph.h"

edge* createEdge(edge* e, float cash, node* neigh){
	e = malloc(sizeof(edge)); //create edge
	e->cash = cash;	//initialize it
	e->next = NULL;
	e->neigh = neigh;
}


void insertEdge(node** source, node** target, float cash){ //add an outward edge to the source and an inward to the target
	int exists = 0;
	edge** eIn = &(*target)->in;
	edge** eOut = &(*source)->out; //eO is the list of outward edges of the source
	if(*eOut == NULL)
		 *eOut = createEdge(*eOut, cash, *target);
	else{		 
		//parse to the end of the edge list while checking if the edge we want to add already exists
		while((*eOut)->next != NULL){
			if( (*eOut)->neigh->accId == (*target)->accId ){ //if it already exists
				exists = 1;
				break;
			}
			eOut = &((*eOut)->next);
		}
		if(exists){
			exists = 0;
			(*eOut)->cash += cash; //update the already existing edge
		}
		else{ //check the last edge, if it's different, create the edge at the end of the list
			if( (*eOut)->neigh->accId == (*target)->accId )
				(*eOut)->cash += cash;
			else{
				edge* tempO = *eOut;
				eOut = &((*eOut)->next);
				*eOut = createEdge(*eOut, cash, *target);
				tempO->next = *eOut;
			}
		}
	} 

	if(*eIn == NULL)
		 *eIn = createEdge(*eIn, cash, *source);
	else{		 
		//parse to the end of the edge list while checking if the edge we want to add already exists
		while((*eIn)->next != NULL){
			if( (*eIn)->neigh->accId == (*source)->accId ){ //if it already exists
				exists = 1;
				break;
			}
			eIn = &((*eIn)->next);
		}
		if(exists){
			exists = 0;
			(*eIn)->cash += cash; //update the existing edge
		}
		else{ //check the last edge, if it's different, create the edge at the end of the list
			if( (*eIn)->neigh->accId == (*source)->accId )
				(*eIn)->cash += cash;
			else{
				edge* tempI = *eIn;
				eIn = &((*eIn)->next);
				*eIn = createEdge(*eIn, cash, *source);
				tempI->next = *eIn;
			}
		}
	} 		
}


void deleteEdge(node** source, node** target){
	edge** eIn = &(*target)->in;
	edge** eOut = &(*source)->out;
	edge* temp;
	int found=0;
	while(*eOut != NULL){ //search all the outward edges of the source for an edge with target as neighbour
		if((*eOut)->neigh->accId == (*target)->accId){ //found
			found = 1;
			temp = *eOut;
			*eOut = (*eOut)->next;
			free(temp);
			break;
		}
		else
			eOut = &((*eOut)->next);
	}
	if(found){
		while(*eIn != NULL){ //search all the inward edges of the target for an edge with source as neighbour
			if((*eIn)->neigh->accId == (*source)->accId){ //found
				temp = *eIn;
				*eIn = (*eIn)->next;
				free(temp);
				break;
			}
			else
				eIn = &((*eIn)->next);
		}
	}
}


int sumEdges(edge* e){
	int sum = 0;
	while(e != NULL){
		sum += e->cash;
		e = e->next;
	}
	return sum;
}



int delNode(node** n){
	if((*n)->out == NULL && (*n)->in == NULL){
		free(*n);
		return 0;
	}
	return 1;
}


int switchS = 0;


void triangleSearch(node* nd, int N, float k, int steps, nList** l){ //steps is initialized at 3 so it can find triangular exchanges 
	edge* out = nd->out;
	edge* in = nd->in;
	nd->v = 1; //this node has been visited
	if(nd->accId != N || steps == 3) //insert the nodes you visit in the list so that you can print them if needed
		insertL(l, nd->accId);  //excluding the target node when steps<3, to prevent duplicates
	if(steps < 0)
		return;
	if(steps == 0 && nd->accId == N){ //we've reached again the node we started from and the cycle is triangular
		if(!switchS){
			printf("success: triangle(%d, %.3f)= \n", N, k);
			switchS = 1;
		}
		printListL(*l);
		return; //the previous calling functions will know it was a success, so that they type their node id
	}	
	while(out != NULL){
		if((out->neigh->v) == 0){ //if the node is unvisited
			triangleSearch(out->neigh, N, k, steps-1, l); //recurve
			out->neigh->v = 0; //unmark it when you're done
			deleteLast(l); //and remove it from the list
		}
		if(out->neigh->accId == N && steps == 1) //only visit the target for a second time if you're about to  
			triangleSearch(out->neigh, N, k, steps-1, l); //complete the triangle
		out = out->next;
	}
}



int connSearch(node* nd, int N1, int N2, nList** l){
	edge* out = nd->out;
	edge* in = nd->in;
	nd->v = 1; //this node has been visited
	insertL(l, nd->accId);
	if(nd->accId == N2){ //target found
		printf("success: conn(%d, %d) = ", N1, N2);
		printListL(*l);
		switchS = 1;
		return 1; //found
	}
	while(out != NULL){ //searh the graph recursively for N2
		if((out->neigh->v) == 0){ //if the node is unvisited
			if( connSearch(out->neigh, N1, N2, l) == 1 ) return 1; //if a path was found stop searching
			out->neigh->v = 0;
			deleteLast(l);
		}
		out = out->next;
	}
	return 0;
}


void allcyclesSearch(node* nd, int N, int steps, nList** l){
	edge* out = nd->out;
	edge* in = nd->in;
	nd->v = 1;
	if(nd->accId != N || steps == 3) //don't keep a duplicate of the first node in the list
		insertL(l, nd->accId);
	if(nd->accId == N && steps <= 0){ //target found
		if(!switchS){
			printf("success: cycles %d = \n", N);
			switchS = 1;
		}
		printListL(*l);
		return;
	}
	while(out != NULL){ //searh the graph recursively for N2
		if((out->neigh->v) == 0){ //if the neighbouring node is unvisited
			allcyclesSearch(out->neigh, N, steps-1, l);
			out->neigh->v = 0; //unmark it when you're done
			deleteLast(l);
		}
		if(out->neigh->accId == N && steps <= 1) //only visit the target for a second time if you're about to  
			allcyclesSearch(out->neigh, N, steps-1, l); //complete a cycle
		out = out->next;
	}
}


void traceflowSearch(node* nd, int N, int L, int l, float cash, nList** list){
	edge* out = nd->out;
	edge* in = nd->in;
	nd->v = 1; //this node has been visited
	insertL(list, nd->accId); //insert the nodes you visit in the list so that you can print them if needed
	if( (nd->accId == N && l < L) || (l < 0) ) //don't follow cycles
		return;
	if(l == 0){
		if(!switchS){
			printf("success: traceflow(%d, %d)= \n", N, L);
			switchS = 1;
		}
		printListT(*list, cash);
		return;
	}
	while(out != NULL){
		if((out->neigh->v) == 0){ //if the node is unvisited
			traceflowSearch(out->neigh, N, L, l-1, cash + out->cash, list); //recurve
			deleteLast(list);
			out->neigh->v = 0;
		}
		out = out->next;
	}
}


void deleteEdges(edge* e){
	edge* temp;
	while(e!=NULL){
		temp=e;
		e = e->next;
		free(temp);
	}
}


void printNeigh(edge* e){
	if(e == NULL) return;
	while(e->next != NULL){
		printf("(%d, %.3f), ", e->neigh->accId, e->cash);
		e = e->next;
	}
	printf("(%d, %.3f)\n", e->neigh->accId, e->cash);
}


void dumpEdge(int N1, edge* e, dList** d){
	if(e == NULL) return;
	while(e != NULL){
		insertD(d, N1, e->neigh->accId, e->cash);
		e = e->next;
	}
}












