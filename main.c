#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

void read_command(const char* command, const char** parameters) {
    if (strcmp(command, "fork()") == 0) {
        //fork();
    }
}

void prompt() {
    printf("Please input a command");
    char command[20];
    scanf("%s", &command);
    read_command(command, "blah");
}

int main() {
    const char* command = "test()";
    const char* parameters = "blah";

    while (1) {
        prompt();
        read_command(command, &parameters);

        /*
        if (fork() != 0) {
            // parent code
            waitpid(-1, &status, 0);
        } else {
            execve(command, parameters, 0);
        }
         */
    }

    return 0;
}
