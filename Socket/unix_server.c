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
#define MAX_CONNECTION 64
#define BUFFER_SIZE 1024

void signal_handler(int signum)
{
    if (signum == SIGINT)
    {
        unlink(SOCKET_PATH);
        printf("\nServer shut down.\n");
        exit(0);
    }
}

void set_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL);
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

int main()
{
    signal(SIGINT, signal_handler);

    int sock_server = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_server < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    set_non_blocking(sock_server);

    struct sockaddr_un addr_server;
    memset(&addr_server, 0, sizeof(struct sockaddr_un));
    addr_server.sun_family = AF_UNIX;
    strncpy(addr_server.sun_path, SOCKET_PATH, sizeof(addr_server.sun_path) - 1);

    unlink(SOCKET_PATH);

    if (bind(sock_server, (struct sockaddr *)&addr_server, sizeof(addr_server)) == -1)
    {
        perror("bind");
        close(sock_server);
        exit(EXIT_FAILURE);
    }

    if (listen(sock_server, MAX_CONNECTION) == -1)
    {
        perror("listen");
        close(sock_server);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on %s\n", SOCKET_PATH);

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
                        printf("Client %d is connected\n", sock_client);
                        set_non_blocking(sock_client);

                        if (nfds < MAX_CONNECTION)
                        {
                            fds[nfds].fd = sock_client;
                            fds[nfds].events = POLLIN;
                            nfds++;
                        }
                        else
                        {
                            printf("Max clients reached. Closing client %d.\n", sock_client);
                            close(sock_client);
                        }
                    }
                    else
                    {
                        if (errno != EAGAIN && errno != EWOULDBLOCK)
                        {
                            perror("accept");
                        }
                    }
                }
                else
                {
                    char buffer[BUFFER_SIZE] = {0};
                    int recv_bytes = recv(fds[i].fd, buffer, BUFFER_SIZE, 0);

                    if (recv_bytes > 0)
                    {
                        printf("Client %d sent: %s\n", fds[i].fd, buffer);

                        char response[BUFFER_SIZE] = {0};
                        snprintf(response, BUFFER_SIZE, "Server time: %ld", time(NULL));
                        if (send(fds[i].fd, response, strlen(response), 0) <= 0)
                        {
                            perror("send");
                        }
                    }
                    else if (recv_bytes == 0 || (recv_bytes == -1 && errno != EAGAIN && errno != EWOULDBLOCK))
                    {
                        printf("Client %d is disconnected\n", fds[i].fd);
                        close(fds[i].fd);

                        fds[i] = fds[nfds - 1];
                        nfds--;
                        i--;
                    }
                    else
                    {
                        if (errno != EAGAIN && errno != EWOULDBLOCK)
                        {
                            perror("recv");
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
