#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <signal.h>

#define PROJECT_ID 0x5555

#define SHM_MODE 0666
#define SHM_FLAGS (IPC_CREAT | SHM_MODE)
#define SHM_SIZE 32
#define SHM_PATH "/tmp/shm"

key_t shm_key;
int common_idx = 0;

void init_shm_keys() __attribute__((constructor));

void init_shm_keys()
{
    int fd_shm = open(SHM_PATH, O_CREAT | O_RDWR, 0666);
    if (fd_shm == -1)
    {
        perror("open");
        exit(1);
    }
    close(fd_shm);

    shm_key = ftok(SHM_PATH, PROJECT_ID);
    if (shm_key == -1)
    {
        perror("ftok");
        exit(1);
    }
}

void signal_handler(int signum, siginfo_t *info, void *ptr)
{
    if (signum == SIGINT)
    {
        shmctl(shmget(shm_key, SHM_SIZE, SHM_FLAGS), IPC_RMID, NULL);
        unlink(SHM_PATH);
        remove(SHM_PATH);
        printf("Cleanup Shared Memory\n");
        exit(0);
    }
}

int main(int argc, char** argv)
{
    int shm_id = shmget(shm_key, SHM_SIZE, SHM_FLAGS);
    if (shm_id == -1)
    {
        perror("shmget");
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
        char* shm_ptr = (char*)shmat(shm_id, NULL, 0);
        if (shm_ptr == MAP_FAILED)
        {
            perror("shmat");
            exit(1);
        }

        memset(shm_ptr, 0, SHM_SIZE);

        while (common_idx < SHM_SIZE)
        {
            char ch = 'b' + (random() % 26);
            shm_ptr[common_idx] = ch;
            printf("CHILD edited:  %s\n", (char*)shm_ptr);
            common_idx++;
            usleep(100000);
        }
    }
    else
    {
        struct sigaction act;
        act.sa_sigaction = signal_handler;
        act.sa_flags = SA_SIGINFO;
        sigaction(SIGINT, &act, NULL);

        char* shm_ptr = (char*)shmat(shm_id, NULL, 0);
        if (shm_ptr == MAP_FAILED)
        {
            perror("shmat");
            exit(1);
        }

        memset(shm_ptr, 0, SHM_SIZE);

        while (common_idx < SHM_SIZE)
        {
            char ch = 'a' + (random() % 26);
            shm_ptr[common_idx] = ch;
            printf("PARENT edited: %s\n", (char*)shm_ptr);
            common_idx++;
            usleep(100000);
        }

        wait(NULL);
    }

    return 0;
}