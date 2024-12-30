#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "defs.h"

key_t key_random;
key_t key_time;
key_t key_error;

void init_ipc_keys() __attribute__((constructor));

void init_ipc_keys()
{
    int fd_random = open(PATH_MSGQ_RANDOM, O_CREAT | O_RDWR, 0666);
    if (fd_random == -1)
    {
        perror("open PATH_MSGQ_RANDOM");
        exit(1);
    }
    close(fd_random);

    key_random = ftok(PATH_MSGQ_RANDOM, PROJECT_ID);
    if (key_random == -1)
    {
        perror("ftok PATH_MSGQ_RANDOM");
        exit(1);
    }

    int fd_time = open(PATH_MSGQ_TIME, O_CREAT | O_RDWR, 0666);
    if (fd_time == -1)
    {
        perror("open PATH_MSGQ_TIME");
        exit(1);
    }
    close(fd_time);

    key_time = ftok(PATH_MSGQ_TIME, PROJECT_ID);
    if (key_time == -1)
    {
        perror("ftok PATH_MSGQ_TIME");
        exit(1);
    }

    int fd_error = open(PATH_MSGQ_ERROR, O_CREAT | O_RDWR, 0666);
    if (fd_error == -1)
    {
        perror("open PATH_MSGQ_ERROR");
        exit(1);
    }
    close(fd_error);

    key_error = ftok(PATH_MSGQ_ERROR, PROJECT_ID);
    if (key_error == -1)
    {
        perror("ftok PATH_MSGQ_ERROR");
        exit(1);
    }

    printf("MSGQ keys: random = %d, time = %d, error = %d\n", key_random, key_time, key_error);
}

int main(int argc, char** argv)
{
    return 0;
}