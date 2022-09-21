#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

void prompt() {

}

void read_command(const char* command, const char** parameters) {

}

int main() {
    const char* command = "test()";
    const char* parameters = "blah";

    while (1) {
        prompt();
        read_command(command, &parameters);

        if (fork() != 0) {
            // parent code
            waitpid(-1, &status, 0);
        } else {
            execve(command, parameters, 0);
        }
    }

    return 0;
}
