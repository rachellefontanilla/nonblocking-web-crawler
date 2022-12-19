#include <stdio.h>
#include <stdlib.h>

/******************************************************************************
 * MY STRUCTURES and TYPEDEFS
 *****************************************************************************/

/* 1. QUEUE (LINKED LIST): ALL urls to visit */

// node struct
typedef struct Node {
    char* node_url;
    struct Node* next;
} Node;

// queue struct
typedef struct Queue {
    struct Node *front;
    struct Node *rear;
} Queue;


/* 2. STACK (LINKED LIST): PNG urls visited OR Page urls visited */
typedef struct Stack {
    struct Node *top;
} Stack;




/******************************************************************************
 * MY FUNCTION PROTOTYPES 
 *****************************************************************************/
/* 1. QUEUE (LINKED LIST): ALL urls to visit */
Node* newNode(char* node_url); // add a new node to queue, containing given url
Queue* createQueue(); // initialize new queue
int isQEmpty(Queue* q); // returns 1 if queue is empty, 0 otherwise
void enQueue(Queue* q, char* node_url); // adds url to end of queue
char* deQueue(Queue* q); // returns url from front of queue
void printQueue(Queue* q); // print all URLs in Queue
void freeQueue(Queue* q); // clears queue by repeatedly calling deQueue


/* 2. STACK (LINKED LIST): PNG urls visited OR Page urls visited */
// new node function same as one for queue
Stack* createStack();
int isSEmpty(Stack* s); // returns 1 if stack is empty, 0 otherwise
int pushStack(Stack* s, char* node_url); // push url to top of stack
char* popStack(Stack* s); // removes url from top of stack
void printStack(Stack* s); // print all URLs in Stack
void freeStack(Stack* s); // clears stack by repeatedly calling popStack