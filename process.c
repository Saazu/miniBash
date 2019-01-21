#include "process.h"

Directory d = NULL;

int process (const CMD *cmdList) {
	reapZombies();
	int status;
	pid_t pid;

	if (cmdList->type == SIMPLE) {
		status = simpleProc(cmdList);
	}
	else if (cmdList->type == PIPE) {
		status = processPipe(cmdList);
	}
	else if (cmdList->type == SEP_AND) {
		if(!(status = process(cmdList->left)))
			status = process(cmdList->right);
	}
	else if (cmdList->type == SEP_OR) {
		if((status = process(cmdList->left)))
			status = process(cmdList->right);
	}
	else if (cmdList->type == SEP_END) {
		status = process(cmdList->left);
		if (cmdList->right)
			status = process(cmdList->right);
	}
	else if (cmdList->type == SUBCMD) {
		if ((pid = fork()) < 0) {
			status = errno;
			perror("sub fork");
			exit(status);
		}
		if (pid == 0) {
			redirectIn(cmdList);
			redirectOut(cmdList);
			status = process(cmdList->left);
			exit(status);
		}
		else {
			signal(SIGINT, SIG_IGN);
			wait(&status);
			signal(SIGINT, SIG_DFL);
			status = STATUS(status);
		}
	}
	else if (cmdList->type == SEP_BG) {
		handleBackground(cmdList);
		if (cmdList->right)
			status = process(cmdList->right);
	}

	//set status to final value of status(errno)
	char *finalStatus = malloc(sizeof(char) * 20);
	snprintf(finalStatus, sizeof(int)+1, "%d", status);
	setenv("?", finalStatus, 1);
	free(finalStatus);
	return status;
}

int simpleProc (const CMD *cmdList) {
	pid_t pid;
	pid_t child;
	int status = 0; // = EXIT_SUCCESS;;
	//pid = fork();

	if (cmdList && cmdList->type == SIMPLE) {
		if (!strcmp(cmdList->argv[0], "cd") || 
			!strcmp(cmdList->argv[0], "pushd") || 
			!strcmp(cmdList->argv[0], "popd")) {

			int out = dup(1);
			int err = dup(2);
			status = handleDir(cmdList);
			dup2(out, 1);
			dup2(err, 2);
			close(out);
			close(err);
		}
		else if ((pid = fork()) < 0) {
			perror("SIMPLE FAILED");
			status = errno;
			//return status;
			exit(status);
		}

		else if (pid == 0) {
			if (cmdList->fromType != NONE) {
				status = redirectIn(cmdList);
			}

			if (cmdList->toType != NONE) {
				status = redirectOut(cmdList);
			}
			handleLocals(cmdList);
			execvp(cmdList->argv[0], cmdList->argv);
			//return status;
			exit(status);
		}
		else {
			signal(SIGINT, SIG_IGN);
			child = pid;
			while ((pid = wait(&status)) > 0) {
				if (pid == child)
					break;
				fprintf(stderr, "Completed: %d (%d)\n", pid, status);
			}
			signal(SIGINT, SIG_DFL);
			status = STATUS(status);
		}
	}
	return status;
}

int redirectIn (const CMD *cmdList) {

	if (cmdList->fromType == RED_IN) {
		int inFile = open(cmdList->fromFile, O_RDONLY);

		if (inFile < 0) {
			//status = errno;
			perror("RED_IN");
			return errno;
		}
		else {
			dup2(inFile, 0);
			close(inFile);
		}
	}
	else if (cmdList->fromType == RED_IN_HERE) {
		int fd[2];
		pid_t pid;
		//pid = fork();

		if (pipe(fd) || (pid = fork()) < 0) {
			//status = errno;
			perror("RED_HERE");
			return errno;
		}
		else if (pid == 0) {
			close(fd[0]);

			if ( write (fd[1], cmdList->fromFile, strlen(cmdList->fromFile)) < 0) {
				//status = errno;
				perror("Here document write failed");
				/******************/
				exit(1);
			}

			close(fd[1]);
			exit(0);
		} 
		else {
			close(fd[1]);
			dup2(fd[0], 0);
			close(fd[0]);
		}
	}
	
	return 0;
}

// Redirect stdout: NONE (default), RED_OUT (>),
// RED_OUT_APP (>>), or  RED_OUT_ERR (&>)
int redirectOut(const CMD *cmdList) {
	int status;
	int outFile;

	if(cmdList->toType == RED_OUT)
		outFile = open(cmdList->toFile, O_CREAT|O_WRONLY|O_TRUNC, 0777); 
	else
		outFile = open(cmdList->toFile, O_CREAT|O_WRONLY|O_APPEND, 0777);
	
	if(outFile < 0){ 
		//perror("HERE");
		return errno;
	} else {
		//saved = dup(1);
		status = dup2(outFile, 1);
		close(outFile);
	}
	return status ? status : -1;
}

//execution of pipelines (sequences of simple commands or subcommands separated
//by the pipeline operator |)
int processPipe (const CMD *cmdList) {
	int fd[2];
	pid_t pid1;
	pid_t pid2;
	int left = 0;
	int right = 0;
	int status;
	int status1 = 0;
	int status2 = 0;

	if (pipe(fd) || (pid1 = fork()) < 0) {
		perror("Pipe fork 1");
		status = errno;
		exit(status);
		//return errno;
	}
	else if (pid1 == 0) {
		close(fd[0]);
		dup2(fd[1], 1);
		close(fd[1]);
		left = process(cmdList->left);
		exit(left);
	}
	else {
		close(fd[1]);
	}

	if ((pid2 = fork()) < 0) {
		wait(&status);
		perror("Pipe Fork 2");
		exit(status);
	}
	else if (pid2 == 0){
		dup2(fd[0], 0);
		close(fd[0]);
		right = process(cmdList->right);
		exit(right);
	}
	else{
		close(fd[0]);
	}

	signal(SIGINT, SIG_IGN);
	wait(&status);
	status1 = STATUS(status);

	wait(&status);
	signal(SIGINT, SIG_IGN);
	status2 = STATUS(status);


	return status1 ? status1 : status2;

}

int handleDir (const CMD *cmdList) {
	int status;

	if (!strcmp(cmdList->argv[0], "cd")) {

		if (cmdList->argc <= 0 || cmdList->argc > 2) {
			status = errno;
			perror("cd");
			return status;
		}
		//initialise stack diretory if it is NULL
		if (d == NULL)
			d = stackInit();

		if (cmdList->argc == 1) {
			status = chdir(getenv("HOME"));

			if (d->size == 0) {
				d = stackInit();
				char *cdir = getcwd(0, 0);
				
				//push cwd as head of directory stack
				stackPush(d, cdir);
				free(cdir);
			}
			else {
				//switch head of directory stack to cwd
				stackPop(d);
				char *cdir = getcwd(0, 0);
				stackPush(d, cdir);
				free(cdir);
			}
		}
		else {
			status = chdir(cmdList->argv[1]);
			if (!status) {
				if (d->size == 0) {
					d = stackInit();
					char *cdir = getcwd(0, 0);
				
					//push
					stackPush(d, cdir);
					free(cdir);
				}
				else {
					stackPop(d);
					char *cdir = getcwd(0, 0);
					stackPush(d, cdir);
					free(cdir);
				}
				//printStack(d);
			}
		}
		if (status) {
			perror("cd");
			return status;
		}
		return status;
	}
	//handle pushd [dir name]
	else if (!strcmp(cmdList->argv[0], "pushd")) {

		if (cmdList->argc != 2) {
			fprintf(stderr, "%s\n", "usage: pushd <dirName>");
			return 1;
		}
		//initialise stack diretory if it is NULL
		if (d == NULL)
			d = stackInit();
		//change directory to directory on top of directory stack
		//this is the argument to pushd
		char *pushDir;
		pushDir = getcwd(0, 0);
		status = chdir(cmdList->argv[1]);
		if (status) {
			perror("pushd");
			free(pushDir);
			return status;
		}
		
		//push cwd to top of stack
		stackPush(d, pushDir);
		free(pushDir);
		printf("%s ", getcwd(0, 0));
		printStack(d);
		return status;
	}
	else {
		if (cmdList->argc != 1) {
			fprintf(stderr, "%s\n", "usage: popd");
			return 1;
		}
		if (d->size == 0 || d == NULL) {
			fprintf(stderr, "%s\n", "popd: dir stack empty");
			return 1;
		}
		if (d->size <= 1) {
			fprintf(stderr, "%s\n", "popd: dir stack empty");
			return 1;
		}
		//remove head of dir stack, change directory to new top
		stackPop(d);
		status = chdir(d->head->dir);

		if (status) {
			perror("popd");
			return status;
		}
		printStack(d);
		return status;		
	}
}

void handleLocals(const CMD *cmdList) {
	for (int i = 0; i < cmdList->nLocal; i++ ) {
		setenv(cmdList->locVar[i], cmdList->locVal[i], 1);
	}
}

void reapZombies() {
	int pid;
	int status;

	while((pid = waitpid(-1, &status, WNOHANG)) >0) {
			fprintf(stderr, "Completed: %d (%d)\n", pid, STATUS(status)); 
	}
}

void background(const CMD *cmdList) {
	int pid;
	if((pid = fork()) < 0) {			
		perror("background fork fail");
		exit(1);
	} 
	if(pid == 0) {
		exit(process(cmdList));
		//exit(0);
	} 
	else 
		fprintf(stderr, "Backgrounded: %d\n", pid);
	//return 0;
}

void handleBackground(const CMD *cmdList) {
	//int status; we don't care about you and what happens to you
	if ((cmdList->left->type == SEP_BG || cmdList->left->type == SEP_END) 
			&& cmdList->left->right) {

		CMD *right = cmdList->left->right;
		cmdList->left->right = NULL;

		process(right);
		background(cmdList->left);
	}
	else {
		background(cmdList->left);
	}
}