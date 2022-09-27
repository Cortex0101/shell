#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define TOKEN_BUF 100
#define bufferSize 1000


char *commandString[] = {
        "exit",
        "cd",
        "help",
        "ws" // not implemented yet

};



int command_cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "Echo: \n I expected an argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("SHELL");
        }
    }
    return 1;
}

int numCommand() {
    return sizeof(commandString) / sizeof(char *);
}


int command_help(char **args)
{

    printf("Group 27 Shell \n");
    printf("just type command's name and then ENTER. :) \n");
    printf("Our commands :\n");
// for loop for at printe vores commands
    int k;
    for (k= 0; k < numCommand(); k++) {
        printf("  %s\n", commandString[k]);
    }

    return 1;
}

int command_exit(char **args)
{
    //exit(0);
    return 0;
}



int (*commandFunc[]) (char **) = {
        &command_exit,
        &command_cd,
        &command_help

};




int command_Run(char **args)
{
    // processor ID
    pid_t processorID;
    int status;
// fork() function to create a child process
    processorID = fork();
    if (processorID == 0) {
        // Child process :)
        if (execvp(args[0], args) == -1) {
            perror("SHELL");
        }
        exit(0);
    } else if (processorID < 0) {
        // forking() ERROR
        perror("SHELL ");
    } else {
        // main process :)
        do {
            // we use waitpid here to suspend the current process until child process changes its state :)
            waitpid(processorID, &status, WUNTRACED);
            // WIFEXITED Query status to see if a child process ended normally
            //WIFSIGNALED Query status to see if a child process ended abnormally
            // we are looping as long as the child process not ends normally nor ends abnormally
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}


int commandExec(char **args)
{
    int i;

    if (args[0] == NULL) {
        // An empty command
        printf("type something please  ");
        return 1;
    }
    // looping through all the commands we made

    for (i = 0; i < numCommand(); i++) {
        if (strcmp(args[0], commandString[i]) == 0) {
            return (*commandFunc[i])(args);
        }
    }

    return command_Run(args);
}


char *readInput(void)
{

    int buf = bufferSize;
    int pos = 0;
    char *buffer = malloc(sizeof(char) * buf);
    int input;

    if (!buffer) {
        fprintf(stderr, "Shell : ERROR Allocation \n");
        exit(0);
    }

    while (1) {
        // read an input
        input = getchar();

        if (input == EOF) {
            exit(0);
        } else if (input == '\n') {
            buffer[pos] = '\0';
            return buffer;
        } else {
            buffer[pos] = input;
        }
        pos++;

        // exceeded buffer re-alloc()
        if (pos >= buf) {
            buf+= bufferSize;
            buffer = realloc(buffer, buf);
            if (!buffer) {
                fprintf(stderr, "Shell : ERROR Allocation \n");
                exit(0);
            }
        }
    }

}

char **spilt(char *splitLine)
{
    int buf = TOKEN_BUF, pos = 0;
    char **tokens = malloc(buf * sizeof(char*));
    char *token, **tokensBackup;

    if (!tokens) {
        fprintf(stderr, "Shell : ERROR Allocation XD\n");
        exit(0);
    }

    token = strtok(splitLine, "\n\t\r");
    while (token != NULL) {
        tokens[pos] = token;
        pos++;

        if (pos >= buf) {
            buf += TOKEN_BUF;
            tokensBackup = tokens;
            tokens = realloc(tokens, buf * sizeof(char*));

            if (!tokens) {
                free(tokensBackup);
                fprintf(stderr, "Shell : ERROR Allocation XD");
                exit(0);
            }
        }
        // strtok() breaks string str into a series of tokens
        token = strtok(NULL, "\n\t\r");
    }
    tokens[pos] = NULL;
    return tokens;
}

// getting input and execute
void inputLoop(void)
{
    char *splitLine;
    char **args;
    int status;

    do {
        printf("Type> ");
        splitLine = readInput();
        args = spilt(splitLine);
        status = commandExec(args);
        // to free the blocks
        free(splitLine);
        free(args);
    } while (status);
}
//main program
int main(int argc, char **argv)

{
    printf("Hello Humans , I am ECHO\n ");
    printf("type Help to get help \n Group 27 \n");
    inputLoop();




    return 1;
}

