#include <sys/wait.h>             /* Needed to use wait() */
#include <stdio.h>                  
#include <stdlib.h>
#include <unistd.h>               /* UNIX and POSIX constants and functions (fork, pipe) */
#include <string.h>               /* Needed to use strlen() */

int main(int argc, char *argv[])
{
 int pipefd[2];
 pid_t cpid;
 char buf;
 if (argc != 2) 
  {
   fprintf(stderr, "Usage: %s \n", argv[0]);
   exit(EXIT_FAILURE);
  }
 if (pipe(pipefd) == -1)          /* An error has occurred. */
  {
   fprintf(stderr, "%s", "The call to pipe() has failed.\n");           
   exit(EXIT_FAILURE);
  }
 cpid = fork();                   /* fork() returns the child process's PID  */
 if (cpid == -1)                  /* An error has occurred. */
  {
   fprintf(stderr, "%s", "The call to fork() has failed.\n");
   exit(EXIT_FAILURE);
  }
 if (cpid == 0) 
  {                               /* Child reads from pipe */
   printf("I am the child.\n");
   close(pipefd[1]);              /* Close unused write end */
   printf("The child is about to read from the pipe.\n");
   while (read(pipefd[0], &buf, 1) > 0)
     write(STDOUT_FILENO, &buf, 1);
   write(STDOUT_FILENO, "\n", 1);
   close(pipefd[0]);
   printf("The child has just echoed from the pipe to standard output.\n");
   _exit(EXIT_SUCCESS);
  } 
 else 
  {                               /* Parent writes argv[1] to pipe */
   printf("I am the parent.\n");
   close(pipefd[0]);              /* Close unused read end */
   write(pipefd[1], argv[1], strlen(argv[1]));
   close(pipefd[1]);              /* Closing creates the EOF marker. */
   printf("The parent has just written data into the pipe.\n");
   printf("The parent will now wait for the child to terminate.\n");
   wait(NULL);                    /* Parent waits for the child to terminate */
   exit(EXIT_SUCCESS);
  }
 return 0;
}