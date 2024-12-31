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

#define SOCKET_PATH "/tmp/unix_socket"
#define MAX_CONNECTION 64
#define BUFFER_SIZE 1024

void signal_handler(int signum, siginfo_t *info, void *ptr)
{
    if (signum == SIGINT)
    {
        unlink(SOCKET_PATH);
        exit(0);
    }
}

void set_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, &flags);
    if (flags == -1)
    {
        perror("fcntl F_GETFL");
        return;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("fcntl F_SETFL O_NONBLOCK");
    }
}

int main(int argc, char** argv)
{
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = signal_handler;
    act.sa_flags = SA_SIGINFO;
    if (sigaction(SIGINT, &act, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    int sock_server = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_server < 0)
    {
        perror("socket");
        exit(1);
    }

    set_non_blocking(sock_server);

    struct sockaddr_un addr_server;
    memset(&addr_server, 0, sizeof(struct sockaddr_un));
    addr_server.sun_family = AF_UNIX;
    strncpy(&addr_server.sun_path, SOCKET_PATH, sizeof(addr_server.sun_path) - 1);

    unlink(SOCKET_PATH);
    if (bind(sock_server, (struct sockaddr*)&addr_server, sizeof(addr_server)) == -1)
    {
        perror("bind");
        close(sock_server);
        exit(1);
    }

    if (listen(sock_server, MAX_CONNECTION) == -1)
    {
        perror("listen");
        close(sock_server);
        exit(1);
    }

    // Server Loop
    struct pollfd fds[MAX_CONNECTION];
    memset(fds, 0, sizeof(struct pollfd) * MAX_CONNECTION);
    fds[0].fd = sock_server;
    fds[0].events = POLLIN;

    int nfds = 1;

    while (1)
    {
        int act = poll(fds, nfds, -1);
        if (act < 0)
        {
            perror("poll");
            break;
        }

        for (int i = 0; i < nfds; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == sock_server)
                {
                    int sock_client = accept(sock_server, NULL, NULL);
                    if (sock_client > 0)
                    {
                        fds[nfds].fd = sock_client;
                        fds[nfds].events = POLLIN;
                        nfds++;
                    }
                }
                else
                {
                    char request_buf[BUFFER_SIZE];
                    memset(request_buf, 0, BUFFER_SIZE);

                    int recv_bytes = recv(fds[i].fd, request_buf, BUFFER_SIZE, 0);
                    if (recv_bytes <= 0 || strcmp(request_buf, "quit") == 0)
                    {
                        perror("recv");

                        close(fds[i].fd);
                        fds[i].fd = fds[nfds - 1].fd;
                        fds[i].events = fds[nfds - 1].events;
                        nfds--;
                        i--;
                    }
                    else
                    {
                        printf("Receive request: %s\n", request_buf);

                        char response_buf[BUFFER_SIZE];
                        memset(response_buf, 0, BUFFER_SIZE);
                        sprintf(response_buf, "%d", time(NULL));

                        int sent_bytes = send(fds[i].fd, response_buf, strlen(response_buf), 0);
                        if (sent_bytes <= 0)
                        {
                            perror("send");
                        }
                    }
                }
            }
        }
    }

    close(sock_server);
    unlink(SOCKET_PATH);
    return 0;
}