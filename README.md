# LinuxIPC

The Linux IPC feature provides the methods for multiple processes to exchange data and signals.

This guideline will introduce to you various IPC techniques, including File Locking, Pipe, Signal, Semaphore, Message Queue, Shared Memory, and Socket.

Each technique has its unique use cases and advantages.

## File Locking

File Locking is a mechanism to control access to a file by multiple processes, ensure that only one process can modify the file at a time, prevent data corruption.

```

```

## Pipe (Anonymous Pipe)

A pipe is an unidirectional communication channel that allows data to flow from one process to another.

It is commonly used for simple communication between parent and child processes.

Pipes are easy to use but limited to communication between related processes.

```

```

## FIFO (Named Pipe)

Unlike regular pipes, which are anonymous and only exist as long as the processes are running, FIFOs are given a name in the file system and can be accessed by unrelated processes.

```

```

## Signal

Signals are a form of IPC used to notify a process that a specific event has occurred. They are used for handling asynchronous events like interrupts. Common signals include SIGINT (interrupt), SIGKILL (terminate), and SIGALRM (alarm).

```

```

## Semaphore

Semaphores are synchronization tools used to control access to shared resources. They can be used to signal between processes and ensure that only a certain number of processes can access a resource at the same time.

```

```

## Message Queue

Message queues allow processes to exchange messages in a structured way. Each message is placed in a queue and can be read by another process. This method is useful for complex communication patterns and ensures that messages are delivered in order.

```

```

## Shared Memory

Shared memory is a method where multiple processes can access the same memory space. It is the fastest form of IPC because it avoids the overhead of copying data between processes. However, it requires careful synchronization to prevent data corruption.

```

```

## Socket

Sockets provide a way for processes to communicate over a network. They can be used for communication between processes on the same machine or different machines.

Using in IPC context, Unix Sockets is powerful mechanism for communication between processes on the same machine. Unix Sockets don't need complex IP and port setup, they use file system paths to establish connections.

```

```

# References

[https://beej.us/guide/bgipc/html/](https://beej.us/guide/bgipc/html/)

[https://opensource.com/article/19/4/interprocess-communication-linux-networking](https://opensource.com/article/19/4/interprocess-communication-linux-networking)

[https://opensource.com/article/19/4/interprocess-communication-linux-storage](https://opensource.com/article/19/4/interprocess-communication-linux-storage)

[https://opensource.com/article/19/4/interprocess-communication-linux-channels](https://opensource.com/article/19/4/interprocess-communication-linux-channels)

[https://www.linkedin.com/pulse/brief-linux-ipc-amit-nadiger/](https://www.linkedin.com/pulse/brief-linux-ipc-amit-nadiger/)
