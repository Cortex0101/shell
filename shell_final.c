#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#define WRITE 1
#define READ 0
pid_t pid = 0;
int N = 0;

char* get_input(void) {
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

char* get_input2(void) {
    int buf = 1024;
    int pos = 0;
    char* buffer = malloc(sizeof(char) * buf);
    int input;

    if (!buffer) {
        fprintf(stderr, "error failed");
        exit(0);
    }

    while (1) {
        input = getchar();
        
        if (input == EOF) {
            continue;
            exit(0);
        } else if (input == '\n') {
            buffer[pos] = '\0';
            return buffer;
        } else {
            buffer[pos] = input;
        }
        pos++;
    }

}

int split(const char *str, char c, char ***arr) {
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

char *trim(char *str) {
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

///////////////////////////////////////////7

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

void fork_child(char** cmds[], int pipes[], int child) {
    int reads[] = {NULL, NULL, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30};
    int writes[] = {NULL, NULL, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33};
    pid = fork();
    if (pid == 0) {
	  dup2(pipes[reads[child]], READ);
	  dup2(pipes[writes[child]], WRITE);
	  close_all(pipes, N * 2);
	  execvp(cmds[child][0], cmds[child]);
	} else if (pid < 0) {
        printf("error");
    } else {
        if (child < N - 1) {
            fork_child(cmds, pipes, child + 1);
        } else {
            if (fork() == 0) {
	            dup2(pipes[reads[N]], 0);
	            close_all(pipes, N * 2);
	            execvp(cmds[child][0], cmds[child]);
	        }
        }
    }
}

void process_input(char* input) {
    char **commands = NULL;
    int num_commands = split(input, '|', &commands);
    trim_all(&commands, num_commands);
    char** command_and_args = NULL;
    char*** cmd_arr = (char***) malloc(sizeof(char***) * (num_commands + 1));
    cmd_arr[num_commands] = NULL;
    N = num_commands;

    for (int i = 0; i < num_commands; ++i) {
        int command_size = split(commands[i], ' ', &command_and_args);
        trim_all(&command_and_args, command_size);

        cmd_arr[i] = (char**) malloc(sizeof(char**)  * (command_size + 1));
        cmd_arr[command_size] = NULL;
        cmd_arr[i] = command_and_args;
    }

    int i = 0;
    int status = 0;
    int* pipes = (int*) malloc(sizeof(int) * (2 * (N- 1)));
        
        pipe_all(pipes, N - 1);

        pid = fork();
        if (pid == 0) {
            if (N == 1) {
                execvp(cmd_arr[0][0], cmd_arr[0]);
            } else {
                dup2(pipes[1], WRITE);
                close_all(pipes, N * 2);
                execvp(cmd_arr[0][0], cmd_arr[0]);
            }
        } else if (pid == -1) {
            printf("error!");
        } else {
            fork_child(cmd_arr, pipes, 2);
            close_all(pipes, N * 2);
            for (i = 0; i < N; i++) {
                wait(&status);
            }
        }
}



int main(int argc, char **argv) {
    char* stop_token = "stop\n";
    char input[100];
    do {
        if (scanf("%s", input) != 1) {
            char ch;
            while ((ch = getchar()) != '\n') {
                if (ch == EOF) {
                    printf("error reading");
                    exit(1);
                }
            }
        } 
        if (strcmp(input, stop_token) == 0) {
            break;
        }
        process_input(input);
    } while (1);
}

/*
int main(int argc, char **argv) {
  int status;
  int i;

  char *cat_args[] = {"ls", NULL};
  char *grep_args[] = {"ls", NULL};
  char *ls[] = {"grep", "o", NULL};
  char *cut_args[] = {"grep", "p", NULL};
  char *test[] = {"grep", "m", NULL};
  char *test2[] = {"wc", NULL};
  char* arr[][N + 1] = {cat_args, grep_args, ls, cut_args, test, test, NULL};

  int pipes[(2 * (N - 1))];
  pipe_all(pipes, N - 1);

  pid = fork();
  if (pid == 0) {
      dup2(pipes[1], WRITE);
      close_all(pipes, N * 2);
      execvp(*cat_args, cat_args);
  }
  else if (pid == -1) {
    printf("error!");
  } else {
    fork_child(arr, pipes, 2);
    close_all(pipes, N * 2);
    for (i = 0; i < N; i++) {
        wait(&status);
    }
  }
}
*/
