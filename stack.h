#include "/c/cs323/Hwk5/process-stub.h"

struct stack {
  char *dir;
  struct stack *next;
};
typedef struct stack *Node;


struct kStack {
	Node head;
	int size;
};
typedef struct kStack *Directory;

Directory stackInit();

void stackPush (Directory d, char *dir);

int stackPop (Directory d);

void printStack (Directory d);


