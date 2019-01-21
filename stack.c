#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "stack.h"

#include <stdbool.h>

//wrapper for directory stack
Directory stackInit() {
	Directory d = malloc(sizeof(Directory));
	d->head = NULL;
	d->size = 0;

	return d;
}

//push directory/folder name onto stack
void stackPush (Directory d, char *folder) {
	Node n = malloc(sizeof(Node));
	n->dir = malloc(sizeof(char) * (strlen(folder) + 1));
	strcpy(n->dir, folder);
	n->next = d->head;
	d->head = n;
	d->size++;

	//return 0;
}

//remove directory name from stack
int stackPop (Directory d) {
	if (d->size > 0) {
		Node temp;

		char *ret = malloc(sizeof(char) * (strlen(d->head->dir) + 1));
		strcpy(ret, d->head->dir);

		temp = d->head;
		d->head = temp->next;
		free(temp->dir);
		free(temp);

		d->size--;

		return 0;
	}
	//else {
	fprintf(stderr, "%s\n", "popd: dir stack empty");;
	return -1;
	//}
}


//print all directory names in stack
void printStack (Directory d) {

	Node n;
	printf("%s", d->head->dir);
	for (n = d->head->next; n != 0; n = n->next) {
	//for (token = *stk; token != 0; token = token->next) {
		printf(" ");
		printf("%s", n->dir);
	}

	printf("\n");
}

//dont need to free