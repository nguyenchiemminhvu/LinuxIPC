#ifndef BROKER_PROXY_H
#define BROKER_PROXY_H

#include "broker_api.h"

class BrokerProxy : public BrokerApi
{
public:
    virtual void register_service(pid_t pid, const char* service_name);
    virtual void unregister_service(pid_t pid);
};

#endif // BROKER_PROXY_H