#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

const char* shared_file = "/tmp/shared_file.txt";
const int MAX_BUF = 1024;

int main(int argc, char** argv)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        exit(1);
    }

    if (pid == 0)
    {
        // Child process (producer)
        int fd = open(shared_file, O_RDWR | O_CREAT, 0666);
        if (fd < 0)
        {
            perror("CHILD open");
            exit(1);
        }

        while (1)
        {
            sleep(1);
        }

        close(fd);
    }
    else
    {
        // Parent process (consumer)
        int fd = open(shared_file, O_RDWR | O_CREAT, 0666);
        if (fd < 0)
        {
            perror("PARENT open");
            exit(1);
        }

        while (1)
        {
            sleep(1);
        }

        close(fd);
    }
    return 0;
}