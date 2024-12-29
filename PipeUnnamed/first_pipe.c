#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char** argv)
{
    int pfds[2];
    if (pipe(pfds) == -1)
    {
        perror("pipe");
        exit(1);
    }

    printf("Write file descriptor: %d\n", pfds[1]);
    write(pfds[1], "test", 5);

    printf("Read file descriptor: %d\n", pfds[0]);
    char buf[5];
    read(pfds[0], buf, 5);
    printf("Read: %s\n", buf);

    return 0;
}