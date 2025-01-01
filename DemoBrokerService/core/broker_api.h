#ifndef BROKER_API_H
#define BROKER_API_H

#include <unistd.h>

class BrokerApi
{
public:
    virtual void register_service(pid_t pid, const char* service_name) = 0;
    virtual void unregister_service(pid_t p_id) = 0;
};

#endif // BROKER_API_H