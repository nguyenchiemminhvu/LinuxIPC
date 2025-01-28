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
    DBusMessage* p_msg;
    DBusMessageIter args;
    DBusError d_error;
    int serial = 0;
    char* signal_value = "Hello, World!";

    dbus_error_init(&d_error);

    // Create a signal and check for errors
    p_msg = dbus_message_new_signal(OBJECT_PATH, INTERFACE_NAME, SIGNAL_NAME);
    if (p_msg == NULL)
    {
        fprintf(stderr, "Message Null\n");
        exit(-1);
    }

    // Append arguments onto signal
    dbus_message_iter_init_append(p_msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &signal_value))
    {
        fprintf(stderr, "Out Of Memory!\n");
        exit(-1);
    }

    // Send the signal and flush the connection
    if (!dbus_connection_send(p_con, p_msg, &serial))
    {
        fprintf(stderr, "Out Of Memory!\n");
        exit(-1);
    }
    dbus_connection_flush(p_con);

    printf("Signal Sent\n");

    // Free the message
    dbus_message_unref(p_msg);
}

void listen_signals(DBusConnection* p_con)
{
    DBusMessage* p_msg;
    DBusError d_error;

    dbus_error_init(&d_error);

    // Add a rule for which messages we want to see
    char rule[DBUS_MAXIMUM_MATCH_RULE_LENGTH];
    snprintf(rule, sizeof(rule), "type='signal',interface='%s'", INTERFACE_NAME);
    dbus_bus_add_match(p_con, rule, &d_error);
    dbus_connection_flush(p_con);

    if (dbus_error_is_set(&d_error))
    {
        fprintf(stderr, "Match Error (%s)\n", d_error.message);
        exit(-1);
    }

    printf("Listening for signals\n");

    // Loop listening for signals being emitted
    while (1)
    {
        // Non blocking read of the next available message
        dbus_connection_read_write(p_con, 0);
        p_msg = dbus_connection_pop_message(p_con);

        // Loop again if we haven't read a message
        if (p_msg == NULL)
        {
            usleep(10000);
            continue;
        }

        // Check if the message is a signal from the correct interface and with the correct name
        if (dbus_message_is_signal(p_msg, INTERFACE_NAME, SIGNAL_NAME))
        {
            DBusMessageIter args;
            char* signal_value;

            // Read the parameters
            if (!dbus_message_iter_init(p_msg, &args))
            {
                fprintf(stderr, "Message has no arguments!\n");
            }
            else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
            {
                fprintf(stderr, "Argument is not string!\n");
            }
            else
            {
                dbus_message_iter_get_basic(&args, &signal_value);
                printf("Received Signal with value: %s\n", signal_value);
                dbus_message_unref(p_msg);
                break;
            }
        }
    }
}

void cleanup(DBusConnection* p_con)
{
    if (p_con != NULL)
    {
        dbus_connection_unref(p_con);
    }
    dbus_shutdown();
    (void)unsetenv("DBUS_SESSION_BUS_ADDRESS");
}

int main()
{
    DBusConnection* p_con;
    DBusError d_error;
    int rc;

    dbus_error_init(&d_error);

    // Start a new DBus session bus manually and set the environment variable
    char* dbus_address = getenv("DBUS_SESSION_BUS_ADDRESS");
    if (dbus_address == NULL)
    {
        fprintf(stderr, "DBUS_SESSION_BUS_ADDRESS is not set. Starting a new session bus.\n");
        system("dbus-daemon --session --fork --print-address > /tmp/dbus_address");
        FILE* fp = fopen("/tmp/dbus_address", "r");
        if (fp == NULL)
        {
            fprintf(stderr, "Failed to open /tmp/dbus_address\n");
            exit(-1);
        }
        char address[256];
        if (fgets(address, sizeof(address), fp) != NULL)
        {
            // Remove the newline character at the end of the address string
            address[strcspn(address, "\n")] = '\0';
            setenv("DBUS_SESSION_BUS_ADDRESS", address, 1);
        }
        fclose(fp);
        setenv("DBUS_SESSION_BUS_ADDRESS", address, 1);
        printf("DBUS_SESSION_BUS_ADDRESS=%s\n", address);
    }

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

    cleanup(p_con);

    return 0;
}