#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <mqueue.h>
#include <pthread.h>

#define PATH_MSGQ_TIME "/tmp/mq_time"
#define MAX_NUM_MSG 10
#define MAX_MSG_SIZE 1024

void signal_handler(int signum, siginfo_t *info, void *ptr)
{
    if (signum == SIGINT)
    {
        unlink(PATH_MSGQ_TIME);
        exit(0);
    }
}

int main(int argc, char** argv)
{
    // create message queues
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_NUM_MSG;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    mqd_t mq_time = mq_open(PATH_MSGQ_TIME, O_CREAT | O_RDWR, 0666, &attr);
    if (mq_time == -1)
    {
        perror("mq_open PATH_MSGQ_TIME");
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
        // Child (sender)
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
            char time_buf[MAX_MSG_SIZE];
            memset(time_buf, 0, MAX_MSG_SIZE);
            sprintf(time_buf, "%s", ctime(NULL));
            if (mq_send(mq_time, time_buf, MAX_MSG_SIZE, 0) == -1)
            {
                perror("mq_send PATH_MSGQ_TIME");
            }

            sleep(1);
        }
    }
    else
    {
        // Parent (receiver)
        char buffer[MAX_MSG_SIZE];
        unsigned int prio;
        while (1)
        {
            if (mq_receive(mq_time, buffer, MAX_MSG_SIZE, &prio) == -1)
            {
                perror("mq_receive PATH_MSGQ_TIME");
            }
            printf("Received: %s", buffer);
        }

        wait(NULL);
    }
    return 0;
}