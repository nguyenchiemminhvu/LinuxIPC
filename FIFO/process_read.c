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
    int fd = open(FIFO_PATH, O_RDONLY);

    char buffer[MESSAGE_SIZE];
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        int read_bytes = read(fd, buffer, sizeof(buffer));
        if (read_bytes == 0)
        {
            printf("EOF\n");
            break;
        }
        else if (read_bytes == -1)
        {
            perror("read");
            break;
        }
        else
        {
            if (read_bytes < MESSAGE_SIZE)
            {
                buffer[read_bytes] = '\0';
                printf("Read %d bytes: %s\n", read_bytes, buffer);
            }
        }
    }

    close(fd);
    return 0;
}