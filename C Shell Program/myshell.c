#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define MAX_ARGS 20
#define BUFSIZE 1024

int parse(char *cmdline, char *args[][MAX_ARGS])
{
  char *array[MAX_ARGS] = {0};
  char *ptr;
  char *ptr2;
  int i = 0;
  int d = 0;
  int j = 0;

  ptr = strtok(cmdline, ";");

  /* if no args */
  if (ptr == NULL || !strcmp(ptr, " ") || !strcmp(ptr, "\n") || !strcmp(ptr, "\t"))
  {
    return 0;
  }

  /* sperating by commands if more than one*/
  while (ptr != NULL)
  {
    array[i] = strdup(ptr);
    ptr = strtok(NULL, ";");
    i++;
  }

  /* placing in double array and parsing again for spaces, newline, and tabs*/
  for (; j < i; j++)
  {
    d = 0;
    ptr2 = strtok(array[j], "\n\t ");
    while (ptr2 != NULL)
    {
      args[j][d] = strdup(ptr2);
      ptr2 = strtok(NULL, "\n\t ");
      d++;
    }
    if (i >= MAX_ARGS || j >= MAX_ARGS)
    {
      printf("Too many arguments!\n");
      exit(1);
    }
  }
  free(ptr);
  free(ptr2);
  return i;
}

void execute(char *args[][MAX_ARGS], int i, int j, int bg_pr[], int *bgptr)
{
  int pid, async = 0;

  if (!strcmp(args[i][0], "quit") || !strcmp(args[i][0], "exit"))
  {exit(0);}

  if (!strcmp(args[i][0], "kill")){
    kill(atoi(args[i][1]), SIGKILL);
    return;
  }
  if (!strcmp(args[i][0], "stop")){
    kill(atoi(args[i][1]), SIGSTOP);
    return;
  }
  if (!strcmp(args[i][0], "resume")){
    kill(atoi(args[i][1]), SIGCONT);
    return;
  }
  
  /* check if async call */
  while (args[i][j] != NULL){j++;}
  if (!strcmp(args[i][j - 1], "&"))
  {
    async = 1;
    args[i][--j] = 0;
  }
  else
    async = 0;
  
  /* start process */
  pid = fork();
  if (pid == 0)
  {
    if (!strcmp(args[i][0], "/bin/ps"))
    {
      if (*bgptr <= 0)
      {
        printf("No background processes to display\n");
        exit(1);
      }
      char str[20] = {0};
      char *dbl[20][20];

      int n, c = 0;

      int piece = sprintf(str, "/bin/ps %d", bg_pr[0]);
      for (; c < *bgptr; c++)
      {
        piece += sprintf(str + piece, " %d", bg_pr[c]);
      }
      
      char *ptr;
      ptr = strtok(str, " ");
      while (ptr != NULL)
      {
        dbl[0][n] = strdup(ptr);
        ptr = strtok(NULL, " ");
        n++;
      }
      execve(dbl[0][0], dbl[0], NULL);
      perror("exec failed");
      exit(-1);
    }
    else
    {
      execve(args[i][0], args[i], NULL);
      /* return only when exec fails */
      perror("exec failed");
      exit(-1);
    }
  }
  else if (pid > 0)
  {
    /* parent process */
    if (!async)
    {
      waitpid(pid, NULL, 0);
    }
    else
    {
      bg_pr[*bgptr] = pid;
      printf("PID %d\n", bg_pr[*bgptr]);
      (*bgptr)++;
    }
  }
  else
  {
    /* error occurred */
    perror("fork failed");
    exit(1);
  }
}

int find(int array[], int size, int value){
    
    int i = 0;
    while(( i < size) && (array[i] != value)) i++;

    return i;
}


int main(int argc, char *argv[])
{
  char cmdline[BUFSIZE] = {0};
  char* args[MAX_ARGS][MAX_ARGS] = {0};
  int bg_pr[MAX_ARGS] = {0};
  
  int pid, ncmds = 0;
  int bg_size = 0;
  int* bgptr = &bg_size;
  
  FILE* input_file;

  if (argc == 2){
    input_file = fopen(argv[1], "r");
    if(input_file == NULL){perror("Error opening file"); exit(-1);}
  } 
  else if (argc == 1){
    input_file = stdin;
  } else{
     printf("To begin program type \"./myshell \" followed by filename or by itself\n");
     return 0;
    }

  for (;;)
  {
    /* attempt to clear bg_pr array */
    if (bg_size > 0){
      pid = waitpid(-1,NULL,WNOHANG);
      if(pid > 0)
      {
        int index = find(bg_pr, bg_size, pid);
        bg_pr[index] = bg_pr[bg_size-1];
        bg_size--;
      }
    }    	   
    
    printf("MCameron> ");

    if (fgets(cmdline, BUFSIZE, input_file) == NULL)
    {
      if (feof(input_file)){exit(0);}
      else{
        perror("fgets failed");
        exit(1);
      }
    }
    
 
    /* clear execution array before every use */
    memset(args, 0, sizeof(args[0][0]) * MAX_ARGS * MAX_ARGS);
    
    int out_count = 0;
    ncmds = parse(cmdline, args);
    if (ncmds == 0) {;}
    else{
      for (; out_count < ncmds; out_count++)
      {
	int in_count = 0;
        execute(args, out_count, in_count, bg_pr, bgptr);
      }
    }
  }
  
  fclose(input_file);
  
  return 0;
}
