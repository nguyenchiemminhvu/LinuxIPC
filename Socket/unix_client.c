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

int main(int argc, char** argv)
{
    int sock_server = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_server < 0)
    {
        perror("socket");
        exit(1);
    }

    struct sockaddr_un addr_server;
    memset(&addr_server, 0, sizeof(struct sockaddr_un));
    addr_server.sun_family = AF_UNIX;
    strncpy(&addr_server.sun_path, SOCKET_PATH, sizeof(addr_server.sun_path) - 1);

    if (connect(sock_server, (struct sockaddr*)&addr_server, sizeof(addr_server)) == -1)
    {
        perror("connect");
        close(sock_server);
        exit(1);
    }

    // Client Loop
    while (1)
    {
        char request_buf[BUFFER_SIZE];
        memset(request_buf, 0, BUFFER_SIZE);
        printf("Enter request: ");
        fgets(request_buf, BUFFER_SIZE - 1, stdin);
        request_buf[strcspn(request_buf, "\r\n")] = 0;

        int sent_bytes = send(sock_server, request_buf, strlen(request_buf), 0);
        if (sent_bytes <= 0)
        {
            perror("send");
            continue;
        }
        else
        {
            if (strcmp(request_buf, "quit") == 0)
            {
                break;
            }

            char response_buf[BUFFER_SIZE];
            memset(response_buf, 0, BUFFER_SIZE);

            int recv_bytes = recv(sock_server, response_buf, BUFFER_SIZE, 0);
            if (recv_bytes <= 0)
            {
                perror("recv");
                break;
            }

            printf("Server response: %s\n", response_buf);
        }
    }

    close(sock_server);
    return 0;
}