#include "linkedList.h"

/* ----------------------------------- */
/* 1. QUEUE (LINKED LIST): ALL urls to visit */

Node* newNode(char* node_url) {
    // create new node
    Node* node = (Node*)malloc(sizeof(Node));

    // allocate memory for url
    node->node_url = (char*)malloc(strlen(node_url) + 1);
    // copy given url to queue
    strcpy(node->node_url, node_url);

    node->next = NULL;
    return node;
};

// create Queue
Queue* createQueue() {
    // allocate memory for queue
    Queue* q = NULL;

    q = (Queue*)malloc(sizeof(Queue));
    if(q == NULL){
        // throw error if unsuccessful
        perror("malloc");
    } else {
        // initialize values of queue
        q->front = NULL;
        q->rear = NULL;
    }
    return q;
};

// returns 1 is queue is empty
// 0 otherwise (if queue is NULL or non-empty)
int isQEmpty(Queue* q){
    if ( q == NULL ) {
        return 0;
    } else {
        return(q->front == NULL);
    }
}

void enQueue(Queue* q, char* node_url) {
    // new node with given url
    Node* node = newNode(node_url);

 
    // queue is empty
    if (q->rear == NULL) {
        // set both rear and front to same new node
        q->front = q->rear = node;
    } else { // queue is not empty
        // add node to end of queue (rear's next)
        // update rear to point to the new node
        q->rear->next = node;
        q->rear = node;
    }
 
};


char* deQueue(Queue* q) {
    // queue is empty
    if (q->front == NULL){
        return "empty";
    } else { // queue is not empty
        // remove front node from queue by setting it to next node
        Node* node = q->front;
        q->front = q->front->next;
    
        // if front node becomes NULL (end of queue), 
        // change rear to be NULL as well
        if (q->front == NULL){
            q->rear = NULL;
        }

        // return just url
        char* ret_url = node->node_url;
        // free node memory
        free(node);
        return ret_url;
    }
 
}

void printQueue(Queue* q){
    if (isQEmpty(q)){
        // queue is empty
        // printf("Empty Queue\n");
    } else { 
        // queue is not empty
        // printf("URLs in Queue:\n");

        Node* curr_node = q->front;
        while (curr_node != NULL){
            // printf("%s\n", curr_node->url);
            curr_node = curr_node->next;
        }
        // printf("\n");
    }
    
}

void freeQueue(Queue* q){
    if (q != NULL){
        while (!isQEmpty(q)){
            // queue is not empty
            // deQueue frees memory of node
            // this frees memory of node url
            char* temp = (deQueue(q));
            free(temp);
        }
        free(q);
    }
}



/* ----------- 2. STACK (LINKED LIST): PNG urls visited OR Page urls visited ---------------- */
Stack* createStack(){
    // create stack
    Stack* s = NULL;
    
    // try to allocate memory for stack
    s = (Stack*)malloc(sizeof(s));
    if(s == NULL){
        // throw error if unsuccessful
        perror("malloc");
    } else {
        // if malloc successful
        // initialize values of stack
        s->top = NULL;
    }
    return s;

}

// returns 1 if empty else 0
int isSEmpty(Stack* s){
    if ( s == NULL ) {
        return 0;
    } else {
        return(s->top == NULL);
    }
}


int pushStack(Stack* s, char* url){
    if ( s == NULL ) {
        return -1;
    } else {
        // new node with given url
        Node* node = newNode(url);

        // point new node to current top of stack
        node->next = s->top;

        // update top pointer to new node
        s->top = node;
        return 1;
    }
}


char* popStack(Stack* s){
    // stack is empty
    if (isSEmpty(s)){
        return("empty stack");
    } else { // stack is not empty
        // remove top node from stack by setting it to next node
        Node* curr = s->top;
        s->top = s->top->next;

        // return just url
        char* ret_url = curr->node_url;
        // free memory
        free(curr);
        curr = NULL;
        return(ret_url);
    }
}

void printStack(Stack* s){
    if (isSEmpty(s)){
        // stack is empty
        // printf("Empty Stack\n");
    } else { 
        // stack is not empty
        // printf("URLs in Stack:\n");

        // iterate
        Node* curr_node = s->top;
        while (curr_node != NULL){
            // printf("%s\n", curr_node->url);
            curr_node = curr_node->next;
        }
        // printf("\n");
    }
}

void freeStack(Stack* s){
    if (s != NULL){
        while (!isSEmpty(s)){
            // stack is not empty
            // popStack frees memory of node
            // this frees memory of node url
            char* temp = popStack(s);
            free(temp);
        }
        free(s);
    }
}