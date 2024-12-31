#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <signal.h>
#include <fcntl.h>

#define SHM_PATH "/dev/shm_posix"
#define SHM_SIZE 32
#define SHM_MODE 0666
#define SHM_FLAGS (O_CREAT | O_RDWR)

int common_idx = 0;

void signal_handler(int signum, siginfo_t* info, void* ptr)
{
    if (signum == SIGINT)
    {
        shm_unlink(SHM_PATH);
        remove(SHM_PATH);
        printf("Cleanup %s\n", SHM_PATH);
        exit(0);
    }
}

int main(int argc, char** argv)
{
    int shm_id = shm_open(SHM_PATH, SHM_FLAGS, SHM_MODE);
    if (shm_id == -1)
    {
        perror("shm_open");
        exit(1);
    }

    if (ftruncate(shm_id, SHM_SIZE) == -1)
    {
        perror("ftruncate");
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
        // Child process
        char* shm_ptr = (char*)mmap(NULL, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_id, 0);
        if (shm_ptr == MAP_FAILED)
        {
            perror("mmap");
            exit(1);
        }

        while (common_idx < SHM_SIZE)
        {
            char ch = 'a' + (common_idx % 26);
            shm_ptr[common_idx] = ch;
            printf("CHILD edited:  %s\n", shm_ptr);
            common_idx++;
            usleep(100000);
        }

        munmap(shm_ptr, SHM_SIZE);
    }
    else
    {
        // Parent process
        struct sigaction act;
        act.sa_sigaction = signal_handler;
        act.sa_flags = SA_SIGINFO;
        sigaction(SIGINT, &act, NULL);

        char* shm_ptr = (char*)mmap(NULL, SHM_SIZE, PROT_WRITE, MAP_SHARED, shm_id, 0);
        if (shm_ptr == MAP_FAILED)
        {
            perror("mmap");
            exit(1);
        }

        memset(shm_ptr, 0, SHM_SIZE);

        while (common_idx < SHM_SIZE)
        {
            char ch = 'A' + (common_idx % 26);
            shm_ptr[common_idx] = ch;
            printf("PARENT edited: %s\n", shm_ptr);
            common_idx++;
            usleep(100000);
        }

        munmap(shm_ptr, SHM_SIZE);

        wait(NULL);
    }

    return 0;
}