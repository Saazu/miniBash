#include "stack.h"

Directory directory;

//push to directory stack
void stackPush(char *dir) {
	Stack *stk = malloc(sizeof(Stack));
	stk->dir = malloc(sizeof(char) * (strlen(dir)+1));
	strcpy(stk->dir, dir);
	stk->next = directory.head;
	directory.head = stk;
	directory.num++;
}

//pop from directory stack
char *stackPop() {
	char *rm = malloc(sizeof(char) * (strlen(directory.head->dir)+1));
	strcpy(rm, directory.head->dir);
	free(directory.head->dir);
	Stack *temp = directory.head->next;
	free(directory.head);
	directory.head = temp;
	directory.num--;

	return rm;
}

void stackPrint(Directory d) {
	Stack *stk;
	for (stk = d; stk != 0; stk = d.head->next) {
		printf("%s\n", stk->dir);
	}
}


/*
typedef struct stack {
  char *dir;
  struct stack *next;
} Stack;


typedef struct directory {
	Stack *head;
	int num;
} Directory;


void stackPush(char *dir);

char *stackPop();

void stackPrint(Stack *stk);


#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

/*  
 * This is the interface for a stack of tokens
 *
 * This is the type of the objects entered in the stack.
 */

