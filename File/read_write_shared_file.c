#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/file.h>
#include <signal.h>

const char* shared_file = "/tmp/shared_file.txt";
const int MAX_BUF = 1024;

void signal_handler(int signum, siginfo_t* siginfo, void* context)
{
    if (signum == SIGINT)
    {
        printf("Cleaning up shared file...\n");
        if (remove(shared_file) != 0)
        {
            perror("remove");
        }
        exit(0);
    }
}

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

        int next_number = 0;
        while (1)
        {
            struct flock lock;
            lock.l_type = F_WRLCK; // Write lock
            lock.l_whence = SEEK_SET; // Relative to the start of the file
            lock.l_start = 0; // Start from the beginning of the file
            lock.l_len = 0; // Lock the whole file
            lock.l_pid = getpid();
            
            fcntl(fd, F_SETLKW, &lock);
            if (lock.l_type != F_UNLCK)
            {
                char buf[MAX_BUF];
                sprintf(buf, "%d\n", next_number);
                write(fd, buf, strlen(buf));
                printf("Write %d\n", next_number);
                next_number++;

                lock.l_type = F_UNLCK;
                if (fcntl(fd, F_SETLK, &lock) == -1)
                {
                    perror("fcntl");
                }
            }

            sleep(1);
        }

        close(fd);
        exit(0);
    }
    else
    {
        // Parent process (consumer)
        struct sigaction act;
        act.sa_sigaction = signal_handler;
        act.sa_flags = SA_SIGINFO;
        sigaction(SIGINT, &act, NULL);

        int fd = open(shared_file, O_RDWR | O_CREAT, 0666);
        if (fd < 0)
        {
            perror("PARENT open");
            exit(1);
        }

        while (1)
        {
            struct flock lock;
            lock.l_type = F_WRLCK; // Write lock
            lock.l_whence = SEEK_SET; // Relative to the start of the file
            lock.l_start = 0; // Start from the beginning of the file
            lock.l_len = 0; // Lock the whole file
            lock.l_pid = getpid();
            
            fcntl(fd, F_SETLKW, &lock);
            if (lock.l_type != F_UNLCK)
            {
                lseek(fd, 0, SEEK_SET);

                char buf[MAX_BUF];
                int n = read(fd, buf, MAX_BUF);
                if (n > 0 && n < MAX_BUF)
                {
                    buf[n] = '\0';
                    printf("Read %s", buf);
                }

                if (ftruncate(fd, 0) != 0)
                {
                    perror("ftruncate");
                }

                lock.l_type = F_UNLCK;
                if (fcntl(fd, F_SETLK, &lock) == -1)
                {
                    perror("fcntl");
                }
            }

            sleep(1);
        }

        close(fd);
        wait(NULL);
    }

    return 0;
}