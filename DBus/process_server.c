#include <dbus/dbus.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_dbus_daemon(void) __attribute__((constructor));

void init_dbus_daemon(void)
{
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
            address[strcspn(address, "\n")] = '\0';
            setenv("DBUS_SESSION_BUS_ADDRESS", address, 1);
        }
        fclose(fp);
        setenv("DBUS_SESSION_BUS_ADDRESS", address, 1);
        printf("DBUS_SESSION_BUS_ADDRESS=%s\n", address);
    }
}

void check_dbus_error(DBusError *error, const char *msg)
{
    if (dbus_error_is_set(error))
    {
        fprintf(stderr, "%s: %s\n", msg, error->message);
        dbus_error_free(error);
        exit(1);
    }
}

void handle_method_call(DBusMessage *msg, DBusConnection *conn)
{
    DBusMessage *reply;
    DBusMessageIter args;
    const char *input;
    int result;

    if (!dbus_message_iter_init(msg, &args) || dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_STRING)
    {
        fprintf(stderr, "Error: Method call has no arguments or argument is not a string\n");
        return;
    }

    dbus_message_iter_get_basic(&args, &input);
    printf("Server received method call with input: '%s'\n", input);

    result = strlen(input);

    reply = dbus_message_new_method_return(msg);
    if (!reply)
    {
        fprintf(stderr, "Error: Out of memory while creating reply\n");
        return;
    }

    dbus_message_iter_init_append(reply, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT32, &result))
    {
        fprintf(stderr, "Error: Out of memory while appending arguments to reply\n");
        dbus_message_unref(reply);
        return;
    }

    if (!dbus_connection_send(conn, reply, NULL))
    {
        fprintf(stderr, "Error: Out of memory while sending reply\n");
    }

    dbus_connection_flush(conn);
    dbus_message_unref(reply);
}

int main()
{
    DBusConnection *conn;
    DBusError error;
    DBusMessage *msg;

    printf("Starting D-Bus server...\n");
    dbus_error_init(&error);

    conn = dbus_bus_get(DBUS_BUS_SESSION, &error);
    check_dbus_error(&error, "Failed to connect to the D-Bus session bus");

    int ret = dbus_bus_request_name(conn, "com.example.MyService", DBUS_NAME_FLAG_REPLACE_EXISTING, &error);
    check_dbus_error(&error, "Failed to request name on the bus");

    if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
    {
        fprintf(stderr, "Error: Not the primary owner of the name\n");
        exit(1);
    }

    printf("Server is running and waiting for method calls...\n");

    while (1)
    {
        dbus_connection_read_write(conn, -1);
        msg = dbus_connection_pop_message(conn);

        if (msg == NULL)
        {
            continue;
        }

        if (dbus_message_is_method_call(msg, "com.example.MyInterface", "GetStringLength"))
        {
            handle_method_call(msg, conn);
        }

        dbus_message_unref(msg);
    }

    return 0;
}