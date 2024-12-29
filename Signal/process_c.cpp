#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <thread>
#include <mutex>
#include <experimental/filesystem>

#include <csignal>
#include <sys/wait.h>

int main(int argc, char** argv)
{
    std::cout << "Process C started with PID: " << getpid() << std::endl;
    int count = 0;
    while (true)
    {
        sleep(1);
        count++;
        pid_t parent_id = getppid();
        kill(parent_id, SIGUSR1);

        if (count > 3)
        {
            break;
        }
    }
    return 0;
}