#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int main()
{
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        exit(1);
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(1);
    }

    if (pid == 0)
    {
        // Child process
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipefd[1]
        close(pipefd[1]);
        execlp("ls", "ls", NULL);
    }
    else
    {
        // Parent process
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO); // Redirect stdin to pipefd[0]
        close(pipefd[0]);
        execlp("wc", "wc", "-l", NULL);
    }

    return 0;
}