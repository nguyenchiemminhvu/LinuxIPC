#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <poll.h>
#include <signal.h>
#include <errno.h>

#define SOCKET_PATH "/tmp/unix_socket"
#define BUFFER_SIZE 32

void signal_handler(int signum)
{
    if (signum == SIGINT || signum == SIGCHLD)
    {
        unlink(SOCKET_PATH);
        exit(0);
    }
}

int main()
{
    int fd_pair[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd_pair) == -1)
    {
        perror("socketpair");
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
        signal(SIGCHLD, signal_handler);

        char buf[BUFFER_SIZE];
        while (1)
        {
            memset(buf, 0, BUFFER_SIZE);
            int read_bytes = recv(fd_pair[1], buf, BUFFER_SIZE, 0);
            if (read_bytes <= 0)
            {
                break;
            }

            int n = strlen(buf);
            for (int i = 0; i < n; i++)
            {
                if (buf[i] >= 'a' && buf[i] <= 'z')
                {
                    buf[i] = (buf[i] & (~32));
                }
                else
                {
                    buf[i] = (buf[i] | 32);
                }
            }

            send(fd_pair[1], buf, n, 0);

            usleep(100000);
        }
    }
    else
    {
        signal(SIGINT, signal_handler);

        char buf[BUFFER_SIZE];
        memset(buf, 0, BUFFER_SIZE);
        for (int i = 0; i < BUFFER_SIZE - 1; i++)
        {
            buf[i] = 'a' + (i % 26);
            printf("PARENT made string:  %s\n", buf);
            send(fd_pair[0], buf, BUFFER_SIZE, 0);

            char response[BUFFER_SIZE];
            memset(response, 0, BUFFER_SIZE);
            recv(fd_pair[0], response, BUFFER_SIZE, 0);
            printf("CHILD remade string: %s\n", response);

            strcpy(buf, response);
        }

        close(fd_pair[0]);
        close(fd_pair[1]);
        unlink(SOCKET_PATH);
        kill(pid, SIGCHLD);
        wait(NULL);
    }

    return 0;
}