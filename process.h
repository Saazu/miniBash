#include "/c/cs323/Hwk5/process-stub.h"
#include "stack.h"

//handle directory change and directory stack manipulation
int handleDir (const CMD *cmdList);

//execute simple commands
int simpleProc (const CMD *cmdList);

//remove all zombie processes
void reapZombies();

//execute program in background
void background(const CMD *cmdList);

//assign local names to local variables
void handleLocals(const CMD *cmdList);

int redirectIn (const CMD *cmdList);

int redirectOut(const CMD *cmdList);

//execution of pipelines (sequences of simple commands or subcommands separated
//by the pipeline operator |)
int processPipe (const CMD *cmdList);

void handleBackground(const CMD *cmdList);