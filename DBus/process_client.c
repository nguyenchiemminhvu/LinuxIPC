#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void check_dbus_error(DBusError *error, const char *msg)
{
    if (dbus_error_is_set(error))
    {
        fprintf(stderr, "%s: %s\n", msg, error->message);
        dbus_error_free(error);
        exit(1);
    }
}

int main()
{
    DBusConnection *conn;
    DBusError error;
    const char *input = "Hello, D-Bus!";
    int result;

    printf("Starting D-Bus client...\n");

    dbus_error_init(&error);

    conn = dbus_bus_get(DBUS_BUS_SESSION, &error);
    check_dbus_error(&error, "Failed to connect to the D-Bus session bus");

    while (1)
    {
        DBusMessage *msg, *reply;
        DBusMessageIter args;

        msg = dbus_message_new_method_call(
            "com.example.MyService",  // Target service name
            "/com/example/MyObject",  // Object path
            "com.example.MyInterface", // Interface name
            "GetStringLength"         // Method name
        );

        if (!msg)
        {
            fprintf(stderr, "Error: Out of memory while creating message\n");
            exit(1);
        }

        dbus_message_iter_init_append(msg, &args);
        if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &input))
        {
            fprintf(stderr, "Error: Out of memory while appending arguments\n");
            dbus_message_unref(msg);
            exit(1);
        }

        reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &error);
        check_dbus_error(&error, "Failed to send message and wait for reply");

        if (dbus_message_get_args(reply, &error, DBUS_TYPE_INT32, &result, DBUS_TYPE_INVALID))
        {
            printf("Client received reply: The length of the string is %d\n", result);
        }
        else
        {
            check_dbus_error(&error, "Failed to parse reply arguments");
        }

        dbus_message_unref(msg);
        dbus_message_unref(reply);
        usleep(100000);
    }

    dbus_connection_unref(conn);

    return 0;
}