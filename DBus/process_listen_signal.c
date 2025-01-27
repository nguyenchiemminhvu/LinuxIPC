#include <dbus/dbus.h>  

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SERVICE_NAME "com.ncmv.signal_service"
#define OBJECT_PATH "/tmp/ncmv/signal_object"
#define INTERFACE_NAME "com.ncmv.signal_interface"
#define SIGNAL_NAME "sample_signal"

void send_signals(DBusConnection* p_con)
{

}

void listen_signals(DBusConnection* p_con)
{

}

int main()
{
    DBusConnection* p_con;
    DBusError d_error;
    int rc;

    dbus_error_init(&d_error);

    p_con = dbus_bus_get(DBUS_BUS_SESSION, &d_error);
    if (dbus_error_is_set(&d_error))
    {
        fprintf(stderr, "Connection Error (%s)\n", d_error.message);
        dbus_error_free(&d_error);
    }

    if (p_con == NULL)
    {
        fprintf(stderr, "dbus_bus_get() failed\n");
        exit(-1);
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        fprintf(stderr, "fork() failed\n");
        exit(-1);
    }

    if (pid == 0)
    {
        // Child process send signals
        send_signals(p_con);
    }
    else
    {
        // Parent process listen signals
        listen_signals(p_con);
    }

    return 0;
}