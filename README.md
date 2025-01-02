- [Introduction](#introduction)
  - [File Locking](#file-locking)
  - [Pipe (Anonymous Pipe)](#pipe-anonymous-pipe)
  - [FIFO (Named Pipe)](#fifo-named-pipe)
  - [Signal](#signal)
  - [Semaphore](#semaphore)
  - [Message Queue](#message-queue)
  - [Shared Memory](#shared-memory)
  - [Socket](#socket)
- [References](#references)

# Introduction

The Linux IPC feature provides the methods for multiple processes to exchange data and signals.

This guideline will introduce to you various IPC techniques, including File Locking, Pipe, Signal, Semaphore, Message Queue, Shared Memory, and Socket.

Each technique has its unique use cases and advantages.

## File Locking

File Locking is a mechanism to control access to a file by multiple processes, ensure that only one process can modify the file at a time, prevent data corruption.

This code below demonstrates a simple producer-consumer model using file locking for IPC in Linux:

- The producer (child process) writes numbers to a shared file, while the consumer (parent process) reads and clears the file.
- File locking ensures that only one process accesses the file at a time, preventing race conditions.

**Function to lock file**

```
void lock_file_wait(int fd)
{
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK; // Write lock
    lock.l_whence = SEEK_SET; // Relative to the start of the file
    lock.l_start = 0; // Start from the beginning of the file
    lock.l_len = 0; // Lock the whole file
    lock.l_pid = getpid();
    
    if (fcntl(fd, F_SETLKW, &lock) == -1)
    {
        perror("fcntl");
    }
}
```

```struct flock lock```: Defines the lock.

```fcntl(fd, F_SETLKW, &lock)```: Sets the lock, waiting if necessary.

**Function to unlock file**

```
void unlock_file(int fd)
{
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = getpid();

    if (fcntl(fd, F_SETLK, &lock) == -1)
    {
        perror("fcntl");
    }
}
```

```lock.l_type = F_UNLCK```: Specifies an unlock operation.

```fcntl(fd, F_SETLK, &lock)```: Sets the unlock operation.

**Full example source code** [HERE](https://github.com/nguyenchiemminhvu/LinuxIPC/tree/main/File).

## Pipe (Anonymous Pipe)

A pipe is an unidirectional communication channel that allows data to flow from one process to another.

It is commonly used for simple communication between parent and child processes.

Pipes are easy to use but limited to communication between related processes.

```
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char** argv)
{
    int pfds[2];
    if (pipe(pfds) == -1)
    {
        perror("pipe");
        exit(1);
    }

    printf("Write file descriptor: %d\n", pfds[1]);
    write(pfds[1], "test", 5);

    printf("Read file descriptor: %d\n", pfds[0]);
    char buf[5];
    read(pfds[0], buf, 5);
    printf("Read: %s\n", buf);

    return 0;
}
```

**Find more demonstration** [HERE](https://github.com/nguyenchiemminhvu/LinuxIPC/tree/main/Pipe).

## FIFO (Named Pipe)

Unlike regular pipes, which are anonymous and only exist as long as the processes are running, FIFOs are given a name in the file system and can be accessed by unrelated processes.

**Full example source code** [HERE](https://github.com/nguyenchiemminhvu/LinuxIPC/tree/main/FIFO).

## Signal

Signals are a form of IPC used to notify a process that a specific event has occurred. They are used for handling asynchronous events like interrupts. Common signals include SIGINT (interrupt), SIGKILL (terminate), and SIGALRM (alarm).

I have prepared a sample code that demonstrates Signal technique.

**Full example source code** [HERE](https://github.com/nguyenchiemminhvu/LinuxIPC/tree/main/Signal).

The program manages a set of processes, monitor their heartbeats, and restart them if they become unresponsive.

## Semaphore

Semaphores are synchronization tools used to control access to shared resources. They can be used to signal between processes and ensure that only a certain number of processes can access a resource at the same time.

Set of System V semaphore APIs on Unix-like operating systems:

[semget()](https://man7.org/linux/man-pages/man3/semget.3p.html): Creates a new semaphore set or accesses an existing one.

[semop()](https://man7.org/linux/man-pages/man3/semop.3p.html): Performs operations on semaphores, such as incrementing or decrementing their values.

[semctl()](https://man7.org/linux/man-pages/man3/semctl.3p.html): Performs various control operations on semaphores, such as setting values or removing the semaphore set.

**Find demonstration source code** [HERE](https://github.com/nguyenchiemminhvu/LinuxIPC/tree/main/Semaphore).

## Message Queue

Message queues allow processes to exchange messages in a structured way. Each message is placed in a queue and can be read by another process. This method is useful for complex communication patterns and ensures that messages are delivered in order.

Set of System V message queue APIs on Unix-like operating systems:

[msgget()](https://man7.org/linux/man-pages/man3/msgget.3p.html): Returns an identifier for the queue. This identifier is used for subsequent operations.

[msgsnd()](https://man7.org/linux/man-pages/man3/msgsnd.3p.html): Send a message to the queue.

[msgrcv()](https://man7.org/linux/man-pages/man3/msgrcv.3p.html): Receive a message from the queue.

[msgctl()](https://man7.org/linux/man-pages/man3/msgctl.3p.html): Perform various control operations on the message queue, such as querying its status or removing it.

**Find demonstration source code** [HERE](https://github.com/nguyenchiemminhvu/LinuxIPC/tree/main/MessageQueue).

## Shared Memory

Shared memory is a method where multiple processes can access the same memory space. It is the fastest form of IPC because it avoids the overhead of copying data between processes. However, it requires careful synchronization to prevent data corruption.

```Creation and Access```: Shared memory segments are created using the [shmget()](https://man7.org/linux/man-pages/man3/shmget.3p.html) system call, which returns an identifier for the segment. This identifier is used for subsequent operations.

```Attaching to Memory```: Processes attach to the shared memory segment using the [shmat()](https://man7.org/linux/man-pages/man3/shmat.3p.html) system call, which returns a pointer to the shared memory. This pointer can be used to read from and write to the shared memory.

```Detaching from Memory```: When a process no longer needs access to the shared memory, it can detach using the [shmdt()](https://man7.org/linux/man-pages/man3/shmdt.3p.html) system call.

```Control Operations```: The [shmctl()](https://man7.org/linux/man-pages/man3/shmctl.3p.html) system call is used to perform various control operations on the shared memory segment, such as querying its status, changing its permissions, or removing it.

**Find demonstration source code** [HERE](https://github.com/nguyenchiemminhvu/LinuxIPC/tree/main/SharedMemory).

## Socket

Sockets provide a way for processes to communicate over a network. They can be used for communication between processes on the same machine or different machines.

Using in IPC context, Unix Sockets is powerful mechanism for communication between processes on the same machine. Unix Sockets don't need complex IP and port setup, they use file system paths to establish connections.

**Full Unix-socket client-server example source code** [HERE](https://github.com/nguyenchiemminhvu/LinuxIPC/tree/main/Socket).

# References

[https://github.com/nguyenchiemminhvu/LinuxIPC/tree/main](https://github.com/nguyenchiemminhvu/LinuxIPC/tree/main)

[https://beej.us/guide/bgipc/html/](https://beej.us/guide/bgipc/html/)

[https://opensource.com/article/19/4/interprocess-communication-linux-networking](https://opensource.com/article/19/4/interprocess-communication-linux-networking)

[https://opensource.com/article/19/4/interprocess-communication-linux-storage](https://opensource.com/article/19/4/interprocess-communication-linux-storage)

[https://opensource.com/article/19/4/interprocess-communication-linux-channels](https://opensource.com/article/19/4/interprocess-communication-linux-channels)

[https://www.linkedin.com/pulse/brief-linux-ipc-amit-nadiger/](https://www.linkedin.com/pulse/brief-linux-ipc-amit-nadiger/)
