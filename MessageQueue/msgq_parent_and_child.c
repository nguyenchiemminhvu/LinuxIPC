#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PROJECT_ID 0x1234

#define PATH_MSGQ_RANDOM "/tmp/msgq_random"

key_t key_random;

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
}

void clean_ipc_keys()
{
    if (unlink(PATH_MSGQ_RANDOM) == 0)
    {
        printf("Cleanup %s\n", PATH_MSGQ_RANDOM);
    }
}

int main(int argc, char** argv)
{
    int msgq_id = msgget(key_random, IPC_CREAT | 0666);
    if (msgq_id == -1)
    {
        perror("msgget");
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
        // Child
        
    }
    else
    {
        // Parent
        

        wait(NULL);
    }
    return 0;
}