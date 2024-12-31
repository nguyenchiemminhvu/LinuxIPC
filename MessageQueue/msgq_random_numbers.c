#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define PROJECT_ID 0x1234

#define PATH_MSGQ_RANDOM "/tmp/msgq_random"

struct random_msgbuf
{
    long mtype;
    long mvalue;
};

key_t key_random;

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
}

void clean_ipc_keys()
{
    if (msgctl(msgget(key_random, IPC_CREAT | IPC_EXCL), IPC_RMID, NULL) == 0)
    {
        printf("Cleanup msgget(key_random, IPC_CREAT | IPC_EXCL)\n");
    }

    if (unlink(PATH_MSGQ_RANDOM) == 0)
    {
        printf("Cleanup %s\n", PATH_MSGQ_RANDOM);
    }
}

void signal_handler(int signum, siginfo_t *info, void *ptr)
{
    if (signum == SIGINT)
    {
        clean_ipc_keys();
        exit(0);
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
        // Child (sender)
        printf("CHILD: msgq_id = %d\n", msgq_id);

        while (1)
        {
            struct random_msgbuf msgbuf;
            msgbuf.mtype = 1;
            msgbuf.mvalue = random();
            if (msgsnd(msgq_id, &msgbuf, sizeof(struct random_msgbuf) - sizeof(long), 0) == -1)
            {
                perror("msgsnd");
                exit(1);
            }
            printf("CHILD: Sent %ld\n", msgbuf.mvalue);

            struct msqid_ds buf;
            if (msgctl(msgq_id, IPC_STAT, &buf) == -1)
            {
                perror("msgctl IPC_STAT");
                exit(1);
            }
            printf("CHILD: msg_qnum = %ld\n", buf.msg_qnum);

            sleep(1);
        }
    }
    else
    {
        // Parent (receiver)
        printf("PARENT: msgq_id = %d\n", msgq_id);

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
            struct random_msgbuf msgbuf;
            if (msgrcv(msgq_id, &msgbuf, sizeof(struct random_msgbuf) - sizeof(long), 0, 0) == -1)
            {
                perror("msgrcv");
                exit(1);
            }
            printf("PARENT: Received %ld\n", msgbuf.mvalue);

            sleep(1);
        }

        wait(NULL);
    }
    return 0;
}