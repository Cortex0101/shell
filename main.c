#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define WRITE 1
#define READ 0

#define N 5

void close_all(int pipes[], int n) {
    for (int i = 0; i < n; ++i) {
        close(pipes[i]);
    }
}

void pipe_all(int pipes[], int num_pipes) {
    for (int i = 0; i < num_pipes; ++i) {
        pipe(pipes + (2 * i));
    }
}

pid_t pid = 0;

/*
child 2 -> 0
child 3 -> 2
child 4 -> 4
child 5 -> 6
*/

void fork_child(char** cmds[], int pipes[], int child) {
    int reads[] = {NULL, NULL, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30};
    int writes[] = {NULL, NULL, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33};
    pid = fork();
    if (pid == 0)
	{
        // 2nd cmd in -> pipe 1 read
	  dup2(pipes[reads[child]], READ);

	  // replace SECOND CMD'S stdout with write end of 2nd pipe

    // 2nd cmd out -> pipe 2 write
	  dup2(pipes[writes[child]], WRITE);
	  

	  close_all(pipes, N * 2);

	  execvp(cmds[child][0], cmds[child]);
	} else if (pid < 0) {
        printf("error");
    } else {
        if (child < N - 1) {
            fork_child(cmds, pipes, child + 1);
        } else {
            if (fork() == 0)
	    {
	      // replace cut's stdin with input read of 2nd pipe

	      dup2(pipes[reads[N]], 0);

	      // close all ends of pipes

	      close_all(pipes, N * 2);

	      execvp(cmds[child][0], cmds[child]);
	    }
        }
    }
}

/**
 * Executes the command "cat scores | grep Villanova | cut -b 1-10".
 * This quick-and-dirty version does no error checking.
 *
 * @author Jim Glenn
 * @version 0.1 10/4/2004
 */

int main(int argc, char **argv)
{
  int status;
  int i;

  // arguments for commands; your parser would be responsible for
  // setting up arrays like these


  char *cat_args[] = {"ls", NULL};
  char *grep_args[] = {"ls", NULL};
  char *ls[] = {"grep", "o", NULL};
  char *cut_args[] = {"grep", "p", NULL};
  char *test[] = {"grep", "m", NULL};
  char *test2[] = {"wc", NULL};
  char* arr[][N + 1] = {cat_args, grep_args, ls, cut_args, test, test, NULL};

  int pipes[(2 * (N - 1))];
  pipe_all(pipes, N - 1);

  // we now have 4 fds:
  // pipes[0] = read end of cat->grep pipe (read by grep)
  // pipes[1] = write end of cat->grep pipe (written by cat)
  // pipes[2] = read end of grep->cut pipe (read by cut)
  // pipes[3] = write end of grep->cut pipe (written by grep)

  // Note that the code in each if is basically identical, so you
  // could set up a loop to handle it.  The differences are in the
  // indicies into pipes used for the dup2 system call
  // and that the 1st and last only deal with the end of one pipe.

  // fork the first child (to execute cat)
  pid = fork();
  if (pid == 0)
    {
      // replace first commands stdout with write part of 1st pipe

      dup2(pipes[1], WRITE);

      // close all pipes (very important!); end we're using was safely copied
      close_all(pipes, N * 2);

      execvp(*cat_args, cat_args);
    }
  else if (pid == -1) {
    printf("error!");
  } else 
    {
        fork_child(arr, pipes, 2);


        /*
    for (int child = 2; child < N; ++child) {
        fork_child(arr[child - 1], pipes, child);
    }
    */
      
  // only the parent gets here and waits for 3 children to finish
  
  close_all(pipes, N * 2);

  for (i = 0; i < N; i++)
    wait(&status);
}
}

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#define PIPE_INPUT 1                              // INPUT_END means where the pipe takes input
#define PIPE_OUTPUT 0                             // OUTPUT_END means where the pipe produces output

#define STDOUT 1
#define STDIN 0

char* my_getline(void) {
    char * line = malloc(100), * linep = line;
    size_t lenmax = 100, len = lenmax;
    int c;

    if(line == NULL)
        return NULL;

    for(;;) {
        c = fgetc(stdin);
        if(c == EOF)
            break;

        if(--len == 0) {
            len = lenmax;
            char * linen = realloc(linep, lenmax *= 2);

            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if((*line++ = c) == '\n')
            break;
    }
    *line = '\0';
    return linep;
}

int split(const char *str, char c, char ***arr)
{
    int count = 1;
    int token_len = 1;
    int i = 0;
    char *p;
    char *t;

    p = str;
    while (*p != '\0')
    {
        if (*p == c)
            count++;
        p++;
    }

    *arr = (char**) malloc(sizeof(char*) * count);
    if (*arr == NULL)
        exit(1);

    p = str;
    while (*p != '\0')
    {
        if (*p == c)
        {
            (*arr)[i] = (char*) malloc( sizeof(char) * token_len );
            if ((*arr)[i] == NULL)
                exit(1);

            token_len = 0;
            i++;
        }
        p++;
        token_len++;
    }
    (*arr)[i] = (char*) malloc( sizeof(char) * token_len );
    if ((*arr)[i] == NULL)
        exit(1);

    i = 0;
    p = str;
    t = ((*arr)[i]);
    while (*p != '\0')
    {
        if (*p != c && *p != '\0')
        {
            *t = *p;
            t++;
        }
        else
        {
            *t = '\0';
            i++;
            t = ((*arr)[i]);
        }
        p++;
    }

    return count;
}

char *trim(char *str)
{
    size_t len = 0;
    char *frontp = str;
    char *endp = NULL;

    if( str == NULL ) { return NULL; }
    if( str[0] == '\0' ) { return str; }

    len = strlen(str);
    endp = str + len;

    /* Move the front and back pointers to address the first non-whitespace
     * characters from each end.
     */
    while( isspace((unsigned char) *frontp) ) { ++frontp; }
    if( endp != frontp )
    {
        while( isspace((unsigned char) *(--endp)) && endp != frontp ) {}
    }

    if( frontp != str && endp == frontp )
            *str = '\0';
    else if( str + len - 1 != endp )
            *(endp + 1) = '\0';

    /* Shift the string so that it starts at str so that if it's dynamically
     * allocated, we can still free it on the returned pointer.  Note the reuse
     * of endp to mean the front of the string buffer now.
     */
    endp = str;
    if( frontp != str )
    {
            while( *frontp ) { *endp++ = *frontp++; }
            *endp = '\0';
    }

    return str;
}

void trim_all(char*** arr, size_t size) {
    for (int i = 0; i < size; i++) {
        (*arr)[i] = trim((*arr)[i]);
    }
        
}

void test_print_arr(char*** arr, size_t size) {
    printf("printing %zu token.\n", size);
    for (int i = 0; i < size; i++)
        printf("string #%d: %s\n", i, (*arr)[i]);
}

pid_t pid;
int fd[2];

void pip_exec(char* cmd[]) {
    pid = fork();
    if (pid < 0) {
        printf("error");
    }
    else if(pid==0)
    {
        dup2(fd[PIPE_OUTPUT], STDIN);
        close(fd[PIPE_OUTPUT]);
        //execlp(scmd, scmd, secarg, (char*) NULL);
        execvp((cmd)[0], cmd);
        fprintf(stderr, "Failed to execute '%s'\n", (cmd)[0]);
        exit(1);
    }
    else
    {
        int status;
        close(fd[PIPE_INPUT]);
        close(fd[PIPE_OUTPUT]);
        waitpid(pid, &status, 0);
    }
    
}

int main (int argc, char ** argv) 
{
    do {
        /*
        char *s = "ls";
        char **commands = NULL;
        int c = split(s, '|', &commands);
        trim_all(&commands, c);
        test_print_arr(&commands, c);
        fork();
        char** command_and_args = NULL;

        for (int i = 0; i < c; ++i) {
            int command_size = split(commands[i], ' ', &command_and_args);
            trim_all(&command_and_args, command_size);












            //char* ar[] = {"wc", "program.c", NULL};
            int status_code = execvp(command_and_args[0], command_and_args);
                if (status_code == -1) {
                    printf("Terminated Incorrectly\n");
                    return 1;
                }
        }
        */
        

    char *firstcmd = "ls";

    char* frsarg = "-l";

    char* scmd = "wc";

    char* secarg = "-l";

    char* first[] = {"echo", "this is a test a", NULL};
    char* second[] = {"wc", NULL, NULL};

pipe(fd);
pid = fork();

if(pid==0)
{
    dup2(fd[PIPE_INPUT], STDOUT);
    close(fd[PIPE_OUTPUT]);
    close(fd[PIPE_INPUT]);
    //execlp(firstcmd, firstcmd, frsarg, (char*) NULL);
    execvp(first[0], first);
    fprintf(stderr, "Failed to execute '%s'\n", first[0]);
    exit(1);
}
else
{ 
    pip_exec(&second);
    //pip_exec(&second, 1);
    /*
    pid = fork();
    if(pid==0)
    {
        dup2(fd[PIPE_OUTPUT], STDIN);
        close(fd[PIPE_INPUT]);
        close(fd[PIPE_OUTPUT]);
        //execlp(scmd, scmd, secarg, (char*) NULL);
        execvp(second[0], second);
        fprintf(stderr, "Failed to execute '%s'\n", scmd);
        exit(1);
    }
    else
    {
        

        int status;
        close(fd[PIPE_OUTPUT]);
        close(fd[PIPE_INPUT]);
        waitpid(pid, &status, 0);
    }
    */
    
   }



    } while (0);
    return 0;
}


        /*
        int pipes[c][2];
        int pipe1[2];
        int pipe2[2];
        int pipe3[2];

        //exit(1);

        if (pipe(pipe1) == -1) {
            printf("failed to pipe firsts!");
                exit(1);
            }

        if (fork() != 123) {
                dup2(pipe1[1], 1);
                // close fds
                close(pipe1[0]);
                close(pipe1[1]);
                // exec
                char** command_and_args = NULL;
                int command_size = split(commands[0], ' ', &command_and_args);
                trim_all(&command_and_args, command_size);
                char* ar[] = {"wc", "program.c", NULL};
                execlp("ps", "ps", "aux", NULL);
                //int status_code = execvp(ar[0], ar);
                //if (status_code == -1) {
                //    printf("Terminated Incorrectly\n");
                //    return 1;
               // }
            }
            else {
                printf("This line will be printed\n");
            }

        if (c > 1) {
            for (int i = 1; i < c - 1; ++i) {
            if (pipe(pipes[i]) == -1) {
                printf("failed to pipe!");
                exit(1);
            }

            if (fork() != 123) {
                dup2(pipes[i][0], 0);
                // output to pipe2
                dup2(pipes[i][1], 1);
                // close fds
                close(pipes[i - 1][0]);
                close(pipes[i - 1][1]);
                close(pipes[i][0]);
                close(pipes[i][1]);
                // exec
                char** command_and_args = NULL;
                int command_size = split(commands[i], ' ', &command_and_args);
                trim_all(&command_and_args, command_size);
                //char* ar[] = {"wc", "program.c", NULL};
                int status_code = execvp(command_and_args[0], command_and_args);
                if (status_code == -1) {
                    printf("Terminated Incorrectly\n");
                    return 1;
                }
            }
            else {
                printf("This line will be printed\n");
            }
        }



        if (fork() != 123) {
                // input from pipe2
                dup2(pipes[c - 1][0], 0);
                // output to stdout (already done)
                // close fds
                close(pipes[c - 1][0]);
                close(pipes[c - 1][1]);
                // exec
                // exec
                char** command_and_args = NULL;
                int command_size = split(commands[c - 1], ' ', &command_and_args);
                trim_all(&command_and_args, command_size);
                //char* ar[] = {"wc", "program.c", NULL};
                int status_code = execvp(command_and_args[0], command_and_args);
                if (status_code == -1) {
                    printf("Terminated Incorrectly\n");
                    return 1;
                }
            }
            else {
                printf("This line will be printed\n");
            }

        }
        */

/*

*/

/*
int main (int argc, char ** argv)
{
    int i;
    char *s = "ls | wc | ls | wc";
    int c = 0;
    char **arr = NULL;

    c = split(s, '|', &arr);

    printf("found %d tokens.\n", c);

    for (i = 0; i < c; i++)
        printf("string #%d: %s\n", i, arr[i]);

    return 0;
}
*/
