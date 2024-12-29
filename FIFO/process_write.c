#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

const char* FIFO_PATH = "/tmp/sample_fifo";
const int MESSAGE_SIZE = 1024;

int main(int argc, char** argv)
{
    int fifo_status = mkfifo(FIFO_PATH, 0666);
    if (fifo_status == -1 && errno != EEXIST)
    {
        perror("mkfifo");
        return 1;
    }

    int fd = open(FIFO_PATH, O_WRONLY);
    char buffer[MESSAGE_SIZE];
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        printf("Enter a message: ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\r\n")] = '\0';

        int write_bytes = write(fd, buffer, strlen(buffer));
        if (write_bytes == -1)
        {
            perror("write");
            break;
        }
    }

    close(fd);
    return 0;
}