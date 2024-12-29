#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>

int main(int argc, char** argv)
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
        sleep(3);
        printf("CHILD Write file descriptor: %d\n", pipefd[1]);
        write(pipefd[1], "test", 5);
        exit(0);
    }
    else
    {
        // Parent process
        if (fcntl(pipefd[0], F_SETFL, O_NONBLOCK) == -1)
        {
            perror("fcntl");
            exit(1);
        }

        char buf[5];
        while (1)
        {
            int ret = read(pipefd[0], buf, 5);
            if (ret == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    perror("PARENT Read");
                    sleep(1);
                    continue;
                }
                else
                {
                    perror("read");
                    exit(1);
                }
            }
            else
            {
                printf("PARENT Read: %s\n", buf);
                break;
            }
        }
    }

    return 0;
}