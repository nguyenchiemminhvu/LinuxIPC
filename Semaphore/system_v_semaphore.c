#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#define PROJECT_ID 0x12345678
#define SEM_FLAGS 0666
#define SEM_MODE (IPC_CREAT | IPC_EXCL | SEM_FLAGS)

#define SEM_PATH "/tmp/semaphore"

key_t sem_key;

void init_semaphore_keys() __attribute__((constructor));

void init_semaphore_keys()
{
    int fd = open(SEM_PATH, O_CREAT | O_RDWR, 0666);
    if (fd == -1)
    {
        perror("open SEM_PATH");
        exit(1);
    }
    close(fd);

    sem_key = ftok(SEM_PATH, PROJECT_ID);
    if (sem_key == -1)
    {
        perror("ftok SEM_PATH");
        exit(1);
    }
}

void signal_handler(int signum, siginfo_t *info, void *ptr)
{
    if (signum == SIGINT)
    {
        if (semctl(semget(sem_key, 1, SEM_FLAGS), 0, IPC_RMID) == 0)
        {
            printf("Cleanup semget(sem_key, 1, SEM_FLAGS)\n");
        }

        if (unlink(SEM_PATH) == 0)
        {
            printf("Cleanup %s\n", SEM_PATH);
        }

        exit(0);
    }
}

void sem_wait_for_resourses(int sem_id, int n)
{
    struct sembuf sembuf = {
        .sem_num = 0,
        .sem_op = -n,
        .sem_flg = 0,
    };
    if (semop(sem_id, &sembuf, 1) == -1)
    {
        perror("semop request");
        exit(1);
    }
}

void sem_release_resourses(int sem_id, int n)
{
    struct sembuf sembuf = {
        .sem_num = 0,
        .sem_op = n,
        .sem_flg = 0,
    };
    if (semop(sem_id, &sembuf, 1) == -1)
    {
        perror("semop release");
        exit(1);
    }
}

int main(int argc, char** argv)
{
    int sem_id = semget(sem_key, 1, SEM_MODE);
    if (sem_id == -1)
    {
        perror("semget");
        exit(1);
    }

    // Set the number of semaphore resources to 0
    if (semctl(sem_id, 0, SETVAL, 0) == -1)
    {
        perror("semctl SETVAL");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        exit(1);
    }

    if (pid == 0)
    {
        // Child process (producer)
        while (1)
        {
            printf("CHILD produce 1 product\n");
            sem_release_resourses(sem_id, 1);
            sleep(1);
        }
    }
    else
    {
        // Parent process (consumer)
        struct sigaction act;
        memset(&act, 0, sizeof(act));
        act.sa_sigaction = signal_handler;
        act.sa_flags = SA_SIGINFO;
        if (sigaction(SIGINT, &act, NULL) == -1)
        {
            perror("sigaction");
            exit(1);
        }

        while (1)
        {
            sem_wait_for_resourses(sem_id, 5);
            printf("PARENT make a package with 5 products\n");
        }

        wait(NULL);
    }
    return 0;
}