#ifndef DEFS_H
#define DEFS_H

#define PROJECT_ID 0x9999

#define BROKER_QUEUE_MAX_SIZE 1024
#define BROKER_MESSAGE_SIZE 4096
#define BROKER_MQ_PATH "/tmp/mq_broker"

struct BrokerMessage
{
    long mtype;
    char mtext[BROKER_MESSAGE_SIZE];
};

#endif // DEFS_H