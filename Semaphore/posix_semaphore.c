#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define SEM_PATH "/posix_semaphore"
#define SEM_MODE 0666

void signal_handler(int signum, siginfo_t *info, void *ptr)
{
    if (signum == SIGINT)
    {
        sem_close(sem_open(SEM_PATH, O_CREAT, SEM_MODE, 0));
        sem_unlink(SEM_PATH);
        remove(SEM_PATH);
        printf("Semaphore closed and unlinked\n");
        exit(0);
    }
}

void* producing_thread(void* arg)
{
    sem_t* p_sem = (sem_t*)arg;
    int count = 0;
    while (1)
    {
        printf("Made a product unit\n");
        count++;

        if (count == 5)
        {
            if (sem_post(p_sem) == -1)
            {
                perror("sem_post");
            }
            count = 0;
        }
        sleep(1);
    }
}

int main(int argc, char** argv)
{
    sem_t* p_sem = sem_open(SEM_PATH, O_CREAT | O_EXCL, SEM_MODE, 0);
    if (p_sem == SEM_FAILED)
    {
        if (errno == EEXIST)
        {
            p_sem = sem_open(SEM_PATH, 0, SEM_MODE, 0);
        }
        else
        {
            perror("sem_open");
            exit(1);
        }
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        exit(1);
    }

    if (pid == 0)
    {
        // Child process
        pthread_t tid[3];
        for (int i = 0; i < 3; i++)
        {
            pthread_create(&tid[i], NULL, producing_thread, p_sem);
            pthread_detach(tid[i]);
        }

        while (1)
        {
            pause();
        }
    }
    else
    {
        // Parent process
        struct sigaction act;
        act.sa_sigaction = signal_handler;
        act.sa_flags = SA_SIGINFO;
        sigaction(SIGINT, &act, NULL);

        while (1)
        {
            if (sem_wait(p_sem) == -1)
            {
                perror("sem_wait");
            }

            printf("PARENT consume a product unit\n");
        }
        
        wait(NULL);
    }
    return 0;
}