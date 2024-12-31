#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#define PROJECT_ID 0x4321

#define PATH_MSGQ_TIME "/tmp/msgq_time"
#define PATH_MSGQ_ERROR "/tmp/msgq_error"
#define MSGQ_TYPE_TIME 1
#define MSGQ_TYPE_ERROR 2
#define MAX_MSG_SIZE 256

struct msgbuf_time
{
    long mtype;
    time_t mtime;
};

struct msgbuf_error
{
    long mtype;
    char mtext[MAX_MSG_SIZE];
};

key_t key_time;
key_t key_error;

void init_ipc_keys() __attribute__((constructor));

void init_ipc_keys()
{
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
    if (msgctl(msgget(key_time, IPC_CREAT | IPC_EXCL), IPC_RMID, NULL) == 0)
    {
        printf("Cleanup msgget(key_time, IPC_CREAT | IPC_EXCL)\n");
    }

    if (unlink(PATH_MSGQ_TIME) == 0)
    {
        printf("Cleanup %s\n", PATH_MSGQ_TIME);
    }

    if (msgctl(msgget(key_error, IPC_CREAT | IPC_EXCL), IPC_RMID, NULL) == 0)
    {
        printf("Cleanup msgget(key_error, IPC_CREAT | IPC_EXCL)\n");
    }

    if (unlink(PATH_MSGQ_ERROR) == 0)
    {
        printf("Cleanup %s\n", PATH_MSGQ_ERROR);
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

void* thread_time_func(void* arg)
{
    int msgq_time_id = msgget(key_time, IPC_CREAT | 0666);
    if (msgq_time_id == -1)
    {
        perror("msgget key_time");
        return NULL;
    }

    struct msgbuf_time msg_time;
    while (1)
    {
        if (msgrcv(msgq_time_id, &msg_time, sizeof(msg_time.mtime), MSGQ_TYPE_TIME, 0) == -1)
        {
            perror("msgrcv key_time");
            exit(1);
        }

        printf("Received time: %s\n", ctime(&msg_time.mtime));
    }

    return NULL;
}

void* thread_error_func(void* arg)
{
    int msgq_error_id = msgget(key_error, IPC_CREAT | 0666);
    if (msgq_error_id == -1)
    {
        perror("msgget key_error");
        return NULL;
    }

    struct msgbuf_error msg_error;
    while (1)
    {
        if (msgrcv(msgq_error_id, &msg_error, sizeof(msg_error.mtext), MSGQ_TYPE_ERROR, 0) == -1)
        {
            perror("msgrcv key_error");
            exit(1);
        }

        printf("Received error: %s\n", msg_error.mtext);
    }

    return NULL;
}

int main(int argc, char** argv)
{
    int msgq_time_id = msgget(key_time, IPC_CREAT | 0666);
    int msgq_error_id = msgget(key_error, IPC_CREAT | 0666);

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

        int count = 0;
        while (1)
        {
            // send time to message queue
            struct msgbuf_time msg_time;
            msg_time.mtype = MSGQ_TYPE_TIME;
            msg_time.mtime = time(NULL);
            if (msgsnd(msgq_time_id, &msg_time, sizeof(msg_time.mtime), 0) == -1)
            {
                perror("msgsnd key_time");
            }

            // send error to message queue
            if (count % 5 == 0)
            {
                struct msgbuf_error msg_error;
                msg_error.mtype = MSGQ_TYPE_ERROR;
                memset(msg_error.mtext, 0, MAX_MSG_SIZE);
                snprintf(msg_error.mtext, MAX_MSG_SIZE, "Error: %d", rand());
                if (msgsnd(msgq_error_id, &msg_error, sizeof(msg_error.mtext), 0) == -1)
                {
                    perror("msgsnd key_error");
                }
            }

            count++;
            sleep(1);
        }
    }
    else
    {
        // Parent (receiver)
        pthread_t thread_time;
        pthread_t thread_error;

        if (pthread_create(&thread_time, NULL, thread_time_func, NULL) != 0)
        {
            perror("pthread_create thread_time");
        }

        if (pthread_create(&thread_error, NULL, thread_error_func, NULL) != 0)
        {
            perror("pthread_create thread_error");
        }

        pthread_detach(thread_time);
        pthread_detach(thread_error);

        wait(NULL);
    }
    return 0;
}