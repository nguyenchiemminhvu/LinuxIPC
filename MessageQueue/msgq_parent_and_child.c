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
void clean_ipc_keys() __attribute__((destructor));

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
}

void clean_ipc_keys()
{
    msgctl(msgget(key_random, 0666), IPC_RMID, NULL);
    msgctl(msgget(key_time, 0666), IPC_RMID, NULL);
    msgctl(msgget(key_error, 0666), IPC_RMID, NULL);

    unlink(PATH_MSGQ_RANDOM);
    unlink(PATH_MSGQ_TIME);
    unlink(PATH_MSGQ_ERROR);
}

int main(int argc, char** argv)
{
    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(1);
    }

    if (pid == 0)
    {
        // Child
        printf("Child process: msgq_random = %d, msgq_time = %d, msgq_error = %d\n", key_random, key_time, key_error);
    }
    else
    {
        // Parent
        printf("Parent process: msgq_random = %d, msgq_time = %d, msgq_error = %d\n", key_random, key_time, key_error);

        wait(NULL);
    }
    return 0;
}